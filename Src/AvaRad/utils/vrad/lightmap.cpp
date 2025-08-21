/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/
#define USE_HEAP 1

#include "vrad.h"
#include "lightmap.h"
#include "radial.h"
#include <bumpvects.h>
#include "utlvector.h"
//#include "vmpi.h"

//!{ 2006-03-21	Çã Ã¢ ¹Î
#include "ivraddll.h"
#include "ava_mpirad.h"
//!} 2006-03-21	Çã Ã¢ ¹Î

enum
{
	AMBIENT_ONLY = 0x1,
	NON_AMBIENT_ONLY = 0x2,
};

#define SMOOTHING_GROUP_HARD_EDGE	0xff000000

//==========================================================================//
// CNormalList.
//==========================================================================//

// This class keeps a list of unique normals and provides a fast 
class CNormalList
{
public:
						CNormalList();
	
	// Adds the normal if unique. Otherwise, returns the normal's index into m_Normals.
	int					FindOrAddNormal( Vector const &vNormal );


public:
	
	CUtlVector<Vector>	m_Normals;


private:

	// This represents a grid from (-1,-1,-1) to (1,1,1).
	enum {NUM_SUBDIVS = 8};
	CUtlVector<int>	m_NormalGrid[NUM_SUBDIVS][NUM_SUBDIVS][NUM_SUBDIVS];
};


int g_iCurFace;


//!{ 2006-05-03	Çã Ã¢ ¹Î
//edgeshare_t	edgeshare[MAX_MAP_EDGES];
//Vector	face_centroids[MAX_MAP_EDGES];

//int vertexref[MAX_MAP_VERTS];
//int *vertexface[MAX_MAP_VERTS];
//faceneighbor_t faceneighbor[MAX_MAP_FACES];

CUtlVector<edgeshare_t>	edgeshare;
CUtlVector<Vector>		face_centroids;

CUtlVector<int> vertexref;
CUtlVector<int*> vertexface;
CUtlVector<faceneighbor_t> faceneighbor;
//!} 2006-05-03	Çã Ã¢ ¹Î

static directlight_t *gSkylight = NULL;
directlight_t *gAmbient = NULL;

//!{ 2006-05-03	Çã Ã¢ ¹Î
// ½ºÅÃ¿¡´Â ³Ê¹« Ä¿¼­.. ÁÙÀÓ..

static __declspec(thread) char* tls_sampleData;
static __declspec(thread) void* tls_hasProcessedSamples;
static __declspec(thread) void* tls_gradient;
static __declspec(thread) void* tls_sampleIntensity;

static __declspec(thread) size_t tlsz_hasProcessedSamples;
static __declspec(thread) size_t tlsz_gradient;
static __declspec(thread) size_t tlsz_sampleIntensity;

__forceinline void* tls_realloc( void*& ptr, size_t& size, size_t new_size )
{
	if (new_size <= size) return ptr;

	new_size = ((new_size / 4096) + 1) * 4096;

//	printf( "Realloc %d to %d\n", size, new_size );	

	return ptr = realloc( ptr, size = new_size );
}



//==========================================================================//
// CNormalList implementation.
//==========================================================================//

CNormalList::CNormalList() : m_Normals( 128 )
{
	for( int i=0; i < sizeof(m_NormalGrid)/sizeof(m_NormalGrid[0][0][0]); i++ )
	{
		(&m_NormalGrid[0][0][0] + i)->SetGrowSize( 16 );
	}
}

int CNormalList::FindOrAddNormal( Vector const &vNormal )
{
	int gi[3];

	// See which grid element it's in.
	for( int iDim=0; iDim < 3; iDim++ )
	{
		gi[iDim] = (int)( ((vNormal[iDim] + 1.0f) * 0.5f) * NUM_SUBDIVS - 0.000001f );
		gi[iDim] = min( gi[iDim], NUM_SUBDIVS );
		gi[iDim] = max( gi[iDim], 0 );
	}

	// Look for a matching vector in there.
	CUtlVector<int> *pGridElement = &m_NormalGrid[gi[0]][gi[1]][gi[2]];
	for( int i=0; i < pGridElement->Size(); i++ )
	{
		int iNormal = pGridElement->Element(i);

		Vector *pVec = &m_Normals[iNormal];
		//if( pVec->DistToSqr(vNormal) < 0.00001f )
		if( *pVec == vNormal )
			return iNormal;
	}

	// Ok, add a new one.
	pGridElement->AddToTail( m_Normals.Size() );
	return m_Normals.AddToTail( vNormal );
}




// FIXME: HACK until the plane normals are made more happy
void GetBumpNormals( const float* sVect, const float* tVect, const Vector& flatNormal, 
					 const Vector& phongNormal, Vector bumpNormals[NUM_BUMP_VECTS] )
{
	Vector stmp( sVect[0], sVect[1], sVect[2] );
	Vector ttmp( tVect[0], tVect[1], tVect[2] );
	GetBumpNormals( stmp, ttmp, flatNormal, phongNormal, bumpNormals );
}

void GetBumpNormals( const float* sVect, const float* tVect, const Vector& flatNormal, 
					const Vector& phongNormal, NVector bumpNormals[NUM_BUMP_VECTS] )
{
	Vector stmp( sVect[0], sVect[1], sVect[2] );
	Vector ttmp( tVect[0], tVect[1], tVect[2] );
	GetBumpNormals( stmp, ttmp, flatNormal, phongNormal, bumpNormals );
}

int EdgeVertex( dface_t *f, int edge )
{
	int k;

	if (edge < 0)
		edge += f->numedges;
	else if (edge >= f->numedges)
		edge = edge % f->numedges;

	k = dsurfedges[f->firstedge + edge];
	if (k < 0)
	{
		// Msg("(%d %d) ", dedges[-k].v[1], dedges[-k].v[0] );
		return dedges[-k].v[1];
	}
	else
	{
		// Msg("(%d %d) ", dedges[k].v[0], dedges[k].v[1] );
		return dedges[k].v[0];
	}
}


/*
============
PairEdges
============
*/
void PairEdges (void)
{
	int		        i, j, k, n, m;
	dface_t	        *f;
	int             numneighbors;
    int             tmpneighbor[64];
	faceneighbor_t  *fn;

	// count number of faces that reference each vertex
	for (i=0; i<numfaces ; i++)
	{
		f = &dfaces.Element(i);
        for (j=0 ; j<f->numedges ; j++)
        {
            // Store the count in vertexref
            vertexref[EdgeVertex(f,j)]++;
        }
	}

	// allocate room
	for (i = 0; i < numvertexes; i++)
	{
		// use the count from above to allocate a big enough array
		vertexface[i] = ( int* )calloc( vertexref[i], sizeof( vertexface[0] ) );
		// clear the temporary data
		vertexref[i] = 0;
	}

	// store a list of every face that uses a perticular vertex
	//!{ 2006-05-03	Çã Ã¢ ¹Î
	//for (i=0, f = dfaces ; i<numfaces ; i++, f++)
	for (i=0; i<numfaces ; i++)
	{
		f = &dfaces.Element(i);
	//!} 2006-05-03	Çã Ã¢ ¹Î
        for (j=0 ; j<f->numedges ; j++)
        {
            n = EdgeVertex(f,j);
            
            for (k = 0; k < vertexref[n]; k++)
            {
                if (vertexface[n][k] == i)
                    break;
            }
            if (k >= vertexref[n])
            {
                // add the face to the list
                vertexface[n][k] = i;
                vertexref[n]++;
            }
        }
	}

	// calc normals and set displacement surface flag
	for (i=0; i<numfaces ; i++)
	{
		fn = &faceneighbor[i];

		//!{ 2006-05-03	Çã Ã¢ ¹Î
		f = &dfaces.Element(i);
		//!} 2006-05-03	Çã Ã¢ ¹Î

		// get face normal
		VectorCopy( dplanes[f->planenum].normal, fn->facenormal );

		// set displacement surface flag
		fn->bHasDisp = false;
		if( ValidDispFace( f ) )
		{
			fn->bHasDisp = true;
		}
	}

	// find neighbors
	for (i=0 ; i<numfaces ; i++)
	{
		numneighbors = 0;
		fn = &faceneighbor[i];

		//!{ 2006-05-03	Çã Ã¢ ¹Î
		f = &dfaces.Element(i);
		//!} 2006-05-03	Çã Ã¢ ¹Î


        // allocate room for vertex normals
        fn->normal = ( Vector* )calloc( f->numedges, sizeof( fn->normal[0] ) );
     		
        // look up all faces sharing vertices and add them to the list
        for (j=0 ; j<f->numedges ; j++)
        {
            n = EdgeVertex(f,j);
            
            for (k = 0; k < vertexref[n]; k++)
            {
                double	cos_normals_angle;
                Vector  *pNeighbornormal;
                
                // skip self
                if (vertexface[n][k] == i)
                    continue;

				// if this face doens't have a displacement -- don't consider displacement neighbors
				if( ( !fn->bHasDisp ) && ( faceneighbor[vertexface[n][k]].bHasDisp ) )
					continue;

                pNeighbornormal = &faceneighbor[vertexface[n][k]].facenormal;
                cos_normals_angle = DotProduct( *pNeighbornormal, fn->facenormal );
					
				// add normal if >= threshold or its a displacement surface (this is only if the original
				// face is a displacement)
				if ( fn->bHasDisp )
				{
					// Always smooth with and against a displacement surface.
					VectorAdd( fn->normal[j], *pNeighbornormal, fn->normal[j] );
				}
				else
				{
					// No smoothing - use of method (backwards compatibility).
					if ( ( f->smoothingGroups == 0 ) && ( dfaces[vertexface[n][k]].smoothingGroups == 0 ) )
					{
						if ( cos_normals_angle >= smoothing_threshold )
						{
							VectorAdd( fn->normal[j], *pNeighbornormal, fn->normal[j] );
						}
						else
						{
							// not considered a neighbor
							continue;
						}
					}
					else
					{
						unsigned int smoothingGroup = ( f->smoothingGroups & dfaces[vertexface[n][k]].smoothingGroups );

						// Hard edge.
						if ( ( smoothingGroup & SMOOTHING_GROUP_HARD_EDGE ) != 0 )
							continue;

						if ( smoothingGroup != 0 )
						{
							VectorAdd( fn->normal[j], *pNeighbornormal, fn->normal[j] );
						}
						else
						{
							// not considered a neighbor
							continue;
						}
					}
				}

				// look to see if we've already added this one
				for (m = 0; m < numneighbors; m++)
				{
					if (tmpneighbor[m] == vertexface[n][k])
						break;
				}
				
				if (m >= numneighbors)
				{
					// add to neighbor list
					tmpneighbor[m] = vertexface[n][k];
					numneighbors++;
					if ( numneighbors > ARRAYSIZE(tmpneighbor) )
					{
						Error("Stack overflow in neighbors\n");
					}
				}
            }
        }

        if (numneighbors)
        {
            // copy over neighbor list
            fn->numneighbors = numneighbors;
            fn->neighbor = ( int* )calloc( numneighbors, sizeof( fn->neighbor[0] ) );
            for (m = 0; m < numneighbors; m++)
            {
                fn->neighbor[m] = tmpneighbor[m];
            }
        }
        
		// fixup normals
        for (j = 0; j < f->numedges; j++)
        {
            VectorAdd( fn->normal[j], fn->facenormal, fn->normal[j] );
            VectorNormalize( fn->normal[j] );
        }
    }
}


void SaveVertexNormals( void )
{
	faceneighbor_t *fn;
	int i, j;
	dface_t *f;
	CNormalList normalList;

	g_numvertnormalindices = 0;

	for( i = 0 ;i<numfaces ; i++ )
	{
		fn = &faceneighbor[i];
		f = &dfaces[i];

		for( j = 0; j < f->numedges; j++ )
		{
			Vector vNormal; 
			if( fn->normal )
			{
				vNormal = fn->normal[j];
			}
			else
			{
				// original faces don't have normals
				vNormal.Init( 0, 0, 0 );
			}
			
			if( g_numvertnormalindices == MAX_MAP_VERTNORMALINDICES )
			{
				Error( "g_numvertnormalindices == MAX_MAP_VERTNORMALINDICES" );
			}
			
			//!{ 2006-05-04	Çã Ã¢ ¹Î
			//g_vertnormalindices[g_numvertnormalindices] = (unsigned short)normalList.FindOrAddNormal( vNormal );
			g_vertnormalindices[g_numvertnormalindices] = (int)normalList.FindOrAddNormal( vNormal );
			//!} 2006-05-04	Çã Ã¢ ¹Î
			g_numvertnormalindices++;
		}
	}

	if( normalList.m_Normals.Size() > MAX_MAP_VERTNORMALS )
	{
		Error( "g_numvertnormals > MAX_MAP_VERTNORMALS" );
	}

	// Copy the list of unique vert normals into g_vertnormals.
	g_numvertnormals = normalList.m_Normals.Size();
	//!{ 2006-05-03	Çã Ã¢ ¹Î
	//memcpy( g_vertnormals, normalList.m_Normals.Base(), sizeof(g_vertnormals[0]) * normalList.m_Normals.Size() );
	g_vertnormals.CopyArray( normalList.m_Normals.Base(), normalList.m_Normals.Size() );
	//!} 2006-05-03	Çã Ã¢ ¹Î
}

/*
=================================================================

  LIGHTMAP SAMPLE GENERATION

=================================================================
*/


//-----------------------------------------------------------------------------
// Purpose: Spits out an error message with information about a lightinfo_t.
// Input  : s - Error message string.
//			l - lightmap info struct.
//-----------------------------------------------------------------------------
void ErrorLightInfo(const char *s, lightinfo_t *l)
{
	texinfo_t *tex = &texinfo[l->face->texinfo];
	winding_t *w = WindingFromFace(&dfaces[l->facenum], l->modelorg);

	//
	// Show the face center and material name if possible.
	//
	if (w != NULL)
	{
		// Don't exit, we'll try to recover...
		Vector vecCenter;
		WindingCenter(w, vecCenter);
//		FreeWinding(w);

		Warning("%s at (%g, %g, %g)", s, (double)vecCenter.x, (double)vecCenter.y, (double)vecCenter.z, TexDataStringTable_GetString( dtexdata[tex->texdata].nameStringTableID ) );
	}
	//
	// If not, just show the material name.
	//
	else
	{
		Warning("%s at (degenerate face)", TexDataStringTable_GetString( dtexdata[tex->texdata].nameStringTableID ));
	}
}


/*
================
CalcFaceVectors

Fills in texorg, worldtotex. and textoworld
================
*/
void CalcFaceVectors (lightinfo_t *l)
{
	texinfo_t	*tex;
	int			i, j;
	Vector	    textureSpaceNormal;
	Vector	    luxelSpaceNormal;
	vec_t	    textureDistScale, luxelDistScale;
	vec_t	    dist, len;

	tex = &texinfo[l->face->texinfo];
	
    // convert from float to double
	for (i=0 ; i<2 ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			l->worldToTextureSpace[i][j] = tex->textureVecsTexelsPerWorldUnits[i][j];
			l->worldToLuxelSpace[i][j] = tex->lightmapVecsLuxelsPerWorldUnits[i][j];
		}
	}

    // calculate a normal to the texture plane.  points can be moved along this
    // without changing their S/T
	textureSpaceNormal[0] = 
		tex->textureVecsTexelsPerWorldUnits[1][1] * tex->textureVecsTexelsPerWorldUnits[0][2] - 
		tex->textureVecsTexelsPerWorldUnits[1][2] * tex->textureVecsTexelsPerWorldUnits[0][1];
	textureSpaceNormal[1] = 
		tex->textureVecsTexelsPerWorldUnits[1][2] * tex->textureVecsTexelsPerWorldUnits[0][0] - 
		tex->textureVecsTexelsPerWorldUnits[1][0] * tex->textureVecsTexelsPerWorldUnits[0][2];
	textureSpaceNormal[2] = 
		tex->textureVecsTexelsPerWorldUnits[1][0] * tex->textureVecsTexelsPerWorldUnits[0][1] - 
		tex->textureVecsTexelsPerWorldUnits[1][1] * tex->textureVecsTexelsPerWorldUnits[0][0];
	VectorNormalize (textureSpaceNormal);

    // calculate a normal to the lightmap plane.  points can be moved along this
    // without changing their S/T
	luxelSpaceNormal[0] = 
		tex->lightmapVecsLuxelsPerWorldUnits[1][1] * tex->lightmapVecsLuxelsPerWorldUnits[0][2] - 
		tex->lightmapVecsLuxelsPerWorldUnits[1][2] * tex->lightmapVecsLuxelsPerWorldUnits[0][1];
	luxelSpaceNormal[1] = 
		tex->lightmapVecsLuxelsPerWorldUnits[1][2] * tex->lightmapVecsLuxelsPerWorldUnits[0][0] - 
		tex->lightmapVecsLuxelsPerWorldUnits[1][0] * tex->lightmapVecsLuxelsPerWorldUnits[0][2];
	luxelSpaceNormal[2] = 
		tex->lightmapVecsLuxelsPerWorldUnits[1][0] * tex->lightmapVecsLuxelsPerWorldUnits[0][1] - 
		tex->lightmapVecsLuxelsPerWorldUnits[1][1] * tex->lightmapVecsLuxelsPerWorldUnits[0][0];
	VectorNormalize (luxelSpaceNormal);

    // flip it towards the texture plane normal
	textureDistScale = DotProduct (textureSpaceNormal, l->facenormal);
	if (fabs(textureDistScale) < 1e-4)
	{
		//ErrorLightInfo("\ntaxis", l);

		// Try to recover...
		if (textureDistScale < 0)
			textureDistScale = -1e-4;
		else
			textureDistScale = 1e-4;
	}

	if (textureDistScale < 0)
	{
		textureDistScale = -textureDistScale;
		VectorSubtract (vec3_origin, textureSpaceNormal, textureSpaceNormal);
	}	

    // flip it towards the lightmap plane normal
	luxelDistScale = DotProduct (luxelSpaceNormal, l->facenormal);
	if (fabs(luxelDistScale) < 1e-4)
	{
		ErrorLightInfo("\nlaxis", l);

		// Try to recover...
		if (luxelDistScale < 0)
			luxelDistScale = -1e-4;
		else
			luxelDistScale = 1e-4;
	}

	if (luxelDistScale < 0)
	{
		luxelDistScale = -luxelDistScale;
		VectorSubtract (vec3_origin, luxelSpaceNormal, luxelSpaceNormal);
	}	

    // distscale is the ratio of the distance along the texture normal to
    // the distance along the plane normal
	textureDistScale = 1.0f / textureDistScale;
	luxelDistScale = 1.0f / luxelDistScale;

	for (i=0 ; i<2 ; i++)
	{
		// texture space
		len = (float)VectorLength (l->worldToTextureSpace[i]);
		dist = DotProduct (l->worldToTextureSpace[i], l->facenormal);
		dist *= textureDistScale;
		VectorMA (l->worldToTextureSpace[i], -dist, textureSpaceNormal, l->textureToWorldSpace[i]);
		VectorScale (l->textureToWorldSpace[i], (1/len)*(1/len), l->textureToWorldSpace[i]);

		// luxel space
		len = (float)VectorLength (l->worldToLuxelSpace[i]);
		dist = DotProduct (l->worldToLuxelSpace[i], l->facenormal);
		dist *= luxelDistScale;
		VectorMA (l->worldToLuxelSpace[i], -dist, luxelSpaceNormal, l->luxelToWorldSpace[i]);
		VectorScale (l->luxelToWorldSpace[i], (1/len)*(1/len), l->luxelToWorldSpace[i]);
	}


	for (i=0 ; i<3 ; i++)
	{
	    // calculate textureOrigin on the texture plane
		l->textureOrigin[i] = -tex->textureVecsTexelsPerWorldUnits[0][3] * l->textureToWorldSpace[0][i] - 
						       tex->textureVecsTexelsPerWorldUnits[1][3] * l->textureToWorldSpace[1][i];
		// calculate lightmapOrigin on the lightmap plane
		l->luxelOrigin[i] = -tex->lightmapVecsLuxelsPerWorldUnits[0][3] * l->luxelToWorldSpace[0][i] - 
						        tex->lightmapVecsLuxelsPerWorldUnits[1][3] * l->luxelToWorldSpace[1][i];
	}

    // project back to the face plane
	dist = DotProduct (l->textureOrigin, l->facenormal) - l->facedist;
	dist *= textureDistScale;
	VectorMA (l->textureOrigin, -dist, textureSpaceNormal, l->textureOrigin);

	dist = DotProduct (l->luxelOrigin, l->facenormal) - l->facedist;
	dist *= luxelDistScale;
	VectorMA (l->luxelOrigin, -dist, luxelSpaceNormal, l->luxelOrigin);

	// compensate for org'd bmodels
	VectorAdd (l->textureOrigin, l->modelorg, l->textureOrigin);

	// compensate for org'd bmodels
	VectorAdd (l->luxelOrigin, l->modelorg, l->luxelOrigin);
}


winding_t *LightmapCoordWindingForFace( lightinfo_t *l )
{
	int			i;
	winding_t	*w;

	w = WindingFromFace( l->face, l->modelorg );

	for (i = 0; i < w->numpoints; i++)
	{
		Vector2D coord;
		WorldToLuxelSpace( l, w->p[i], coord );
		w->p[i].x = coord.x;
		w->p[i].y = coord.y;
		w->p[i].z = 0;
	}

	return w;
}


void WriteCoordWinding (FILE *out, lightinfo_t *l, winding_t *w, Vector& color )
{
	int			i;
	Vector		pos;

	fprintf (out, "%i\n", w->numpoints);
	for (i=0 ; i<w->numpoints ; i++)
	{
		LuxelSpaceToWorld( l, w->p[i][0], w->p[i][1], pos );
		fprintf (out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
			pos[0],
			pos[1],
			pos[2],
			color[ 0 ] / 256,
			color[ 1 ] / 256,
			color[ 2 ] / 256 );
	}
	fprintf (out, "\n");
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DumpFaces( lightinfo_t *pLightInfo, int ndxFace )
{
	static	FileHandle_t out;

	// get face data
	faceneighbor_t *fn = &faceneighbor[ndxFace];
	Vector &centroid = face_centroids[ndxFace];

	// disable threading (not a multi-threadable function!)
	ThreadLock();
	
	if( !out )
	{
		// open the file
		out = g_pFileSystem->Open( "face.txt", "w" );
		if( !out )
			return;
	}
	
	//
	// write out face
	//
	CmdLib_FPrintf( out, "face number %d\n", ndxFace );
	for( int ndxEdge = 0; ndxEdge < pLightInfo->face->numedges; ndxEdge++ )
	{
//		int edge = dsurfedges[pLightInfo->face->firstedge+ndxEdge];

		Vector p1, p2;
		VectorAdd( dvertexes[EdgeVertex( pLightInfo->face, ndxEdge )].point, pLightInfo->modelorg, p1 );
		VectorAdd( dvertexes[EdgeVertex( pLightInfo->face, ndxEdge+1 )].point, pLightInfo->modelorg, p2 );
		
		Vector &n1 = fn->normal[ndxEdge];
		Vector &n2 = fn->normal[(ndxEdge+1)%pLightInfo->face->numedges];
		
		CmdLib_FPrintf( out, "3\n");
		
		CmdLib_FPrintf(out, "%f %f %f %f %f %f\n", p1[0], p1[1], p1[2], n1[0] * 0.5 + 0.5, n1[1] * 0.5 + 0.5, n1[2] * 0.5 + 0.5 );
		
		CmdLib_FPrintf(out, "%f %f %f %f %f %f\n", p2[0], p2[1], p2[2], n2[0] * 0.5 + 0.5, n2[1] * 0.5 + 0.5, n2[2] * 0.5 + 0.5 );
		
		CmdLib_FPrintf(out, "%f %f %f %f %f %f\n", centroid[0] + pLightInfo->modelorg[0], 
			                                centroid[1] + pLightInfo->modelorg[1], 
											centroid[2] + pLightInfo->modelorg[2], 
			                                fn->facenormal[0] * 0.5 + 0.5, 
											fn->facenormal[1] * 0.5 + 0.5, 
											fn->facenormal[2] * 0.5 + 0.5 );
		
		CmdLib_FPrintf(out, "\n");
	}
	
	// enable threading
	ThreadUnlock();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool BuildFaceSamplesAndLuxels_DoFast( lightinfo_t *pLightInfo, facelight_t *pFaceLight )
{
	// lightmap size
	int width = pLightInfo->face->m_LightmapTextureSizeInLuxels[0]+1;
	int height = pLightInfo->face->m_LightmapTextureSizeInLuxels[1]+1;

	// ratio of world area / lightmap area
	texinfo_t *pTex = &texinfo[pLightInfo->face->texinfo];
	pFaceLight->worldAreaPerLuxel = 1.0 / ( sqrt( DotProduct( pTex->lightmapVecsLuxelsPerWorldUnits[0], 
															  pTex->lightmapVecsLuxelsPerWorldUnits[0] ) ) * 
											sqrt( DotProduct( pTex->lightmapVecsLuxelsPerWorldUnits[1], 
															  pTex->lightmapVecsLuxelsPerWorldUnits[1] ) ) );

	//
	// quickly create samples and luxels (copy over samples)
	//
	pFaceLight->numsamples = width * height;
	pFaceLight->sample = ( sample_t* )calloc( pFaceLight->numsamples, sizeof( *pFaceLight->sample ) );
	if( !pFaceLight->sample )
		return false;

	pFaceLight->numluxels = width * height;
	pFaceLight->luxel = ( Vector* )calloc( pFaceLight->numluxels, sizeof( *pFaceLight->luxel ) );
	if( !pFaceLight->luxel )
		return false;

	sample_t *pSamples = pFaceLight->sample;
	Vector	 *pLuxels = pFaceLight->luxel;

	for( int t = 0; t < height; t++ )
	{
		for( int s = 0; s < width; s++ )
		{
			pSamples->s = s;
			pSamples->t = t;
			pSamples->coord[0] = s;
			pSamples->coord[1] = t;
			// unused but initialized anyway
			pSamples->mins[0] = s - 0.5;
			pSamples->mins[1] = t - 0.5;
			pSamples->maxs[0] = s + 0.5;
			pSamples->maxs[1] = t + 0.5;
			pSamples->area = pFaceLight->worldAreaPerLuxel;
			LuxelSpaceToWorld( pLightInfo, pSamples->coord[0], pSamples->coord[1], pSamples->pos );
			VectorCopy( pSamples->pos, *pLuxels );

			pSamples++;
			pLuxels++;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool BuildSamplesAndLuxels_DoFast( lightinfo_t *pLightInfo, facelight_t *pFaceLight, int ndxFace )
{
	// build samples for a "face"
	if( pLightInfo->face->dispinfo == -1 )
	{
		return BuildFaceSamplesAndLuxels_DoFast( pLightInfo, pFaceLight );
	}
	// build samples for a "displacement"
	else
	{
		return StaticDispMgr()->BuildDispSamplesAndLuxels_DoFast( pLightInfo, pFaceLight, ndxFace );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------



//char sampleData[sizeof(sample_t)*SINGLEMAP*2];



//!} 2006-05-03	Çã Ã¢ ¹Î

bool BuildFaceSamples( lightinfo_t *pLightInfo, facelight_t *pFaceLight )
{
	// lightmap size
	int width = pLightInfo->face->m_LightmapTextureSizeInLuxels[0]+1;
	int height = pLightInfo->face->m_LightmapTextureSizeInLuxels[1]+1;

	// ratio of world area / lightmap area
	texinfo_t *pTex = &texinfo[pLightInfo->face->texinfo];
	pFaceLight->worldAreaPerLuxel = 1.0 / ( sqrt( DotProduct( pTex->lightmapVecsLuxelsPerWorldUnits[0], 
															  pTex->lightmapVecsLuxelsPerWorldUnits[0] ) ) * 
											sqrt( DotProduct( pTex->lightmapVecsLuxelsPerWorldUnits[1], 
															  pTex->lightmapVecsLuxelsPerWorldUnits[1] ) ) );

#if( USE_HEAP )
	char* sampleData = tls_sampleData;
#else
	char sampleData[sizeof(sample_t)*SINGLEMAP];
#endif

	sample_t *samples = (sample_t*)sampleData;
	sample_t *pSamples = samples;

	// lightmap space winding
	winding_t *pLightmapWinding = LightmapCoordWindingForFace( pLightInfo ); 

	//
	// build vector pointing along the lightmap cutting planes
	//
	Vector sNorm( 1.0f, 0.0f, 0.0f );
	Vector tNorm( 0.0f, 1.0f, 0.0f );

	// sample center offset
	float sampleOffset = ( do_centersamples ) ? 0.5 : 1.0;

	//
	// clip the lightmap "spaced" winding by the lightmap cutting planes
	//
	winding_t *pWindingT1, *pWindingT2;
	winding_t *pWindingS1, *pWindingS2;
	float dist;

	for( int t = 0; t < height && pLightmapWinding; t++ )
	{
		dist = t + sampleOffset;
		
		// lop off a sample in the t dimension
		// hack - need a separate epsilon for lightmap space since ON_EPSILON is for texture space
		ClipWindingEpsilon( pLightmapWinding, tNorm, dist, ON_EPSILON / 16.0f, &pWindingT1, &pWindingT2 );

		for( int s = 0; s < width && pWindingT2; s++ )
		{
			dist = s + sampleOffset;

			// lop off a sample in the s dimension, and put it in ws2
			// hack - need a separate epsilon for lightmap space since ON_EPSILON is for texture space
			ClipWindingEpsilon( pWindingT2, sNorm, dist, ON_EPSILON / 16.0f, &pWindingS1, &pWindingS2 );

			//
			// s2 winding is a single sample worth of winding
			//
			if( pWindingS2 )
			{
				// save the s, t positions
				pSamples->s = s;
				pSamples->t = t;

				// get the lightmap space area of ws2 and convert to world area
				// and find the center (then convert it to 2D)
				Vector center;
				pSamples->area = WindingAreaAndBalancePoint(  pWindingS2, center ) * pFaceLight->worldAreaPerLuxel;
				pSamples->coord[0] = center.x; 
				pSamples->coord[1] = center.y;

				// find winding bounds (then convert it to 2D)
				Vector minbounds, maxbounds;
				WindingBounds( pWindingS2, minbounds, maxbounds );
				pSamples->mins[0] = minbounds.x; 
				pSamples->mins[1] = minbounds.y;
				pSamples->maxs[0] = maxbounds.x; 
				pSamples->maxs[1] = maxbounds.y;

				// convert from lightmap space to world space
				LuxelSpaceToWorld( pLightInfo, pSamples->coord[0], pSamples->coord[1], pSamples->pos );

				if (dumppatches || (do_extra && pSamples->area < pFaceLight->worldAreaPerLuxel - EQUAL_EPSILON))
				{
					//
					// convert the winding from lightmaps space to world for debug rendering and sub-sampling
					//
					Vector worldPos;
					for( int ndxPt = 0; ndxPt < pWindingS2->numpoints; ndxPt++ )
					{
						LuxelSpaceToWorld( pLightInfo, pWindingS2->p[ndxPt].x, pWindingS2->p[ndxPt].y, worldPos );
						VectorCopy( worldPos, pWindingS2->p[ndxPt] );
					}
					pSamples->w = pWindingS2;
				}
				else
				{
					// winding isn't needed, free it.
					pSamples->w = NULL;
					FreeWinding( pWindingS2 );
				}

				pSamples++;
			}

			//
			// if winding T2 still exists free it and set it equal S1 (the rest of the row minus the sample just created)
			//
			if( pWindingT2 )
			{
				FreeWinding( pWindingT2 );
			}

			// clip the rest of "s"
			pWindingT2 = pWindingS1;
		}

		//
		// if the original lightmap winding exists free it and set it equal to T1 (the rest of the winding not cut into samples) 
		//
		if( pLightmapWinding )
		{
			FreeWinding( pLightmapWinding );
		}

		if( pWindingT2 )
		{
			FreeWinding( pWindingT2 );
		}

		pLightmapWinding = pWindingT1;
	}

	//
	// copy over samples
	//
	pFaceLight->numsamples = pSamples - samples;
	pFaceLight->sample = ( sample_t* )calloc( pFaceLight->numsamples, sizeof( *pFaceLight->sample ) );
	if( !pFaceLight->sample )
	{
		Msg( "out of memory : alloc samples for face %d\n", pLightInfo->face - dfaces.Base() );
		return false;
	}
	memcpy( pFaceLight->sample, samples, pFaceLight->numsamples * sizeof( *pFaceLight->sample ) );

	// supply a default sample normal (face normal - assumed flat)
	for( int ndxSample = 0; ndxSample < pFaceLight->numsamples; ndxSample++ )
	{
		pFaceLight->sample[ndxSample].normal = pLightInfo->facenormal;
	}

	// statistics - warning?!
	if( pFaceLight->numsamples == 0 )
	{
		//!{ 2006-05-03	Çã Ã¢ ¹Î
		Msg( "no samples %d\n", pLightInfo->face - dfaces.Base() );
		//!} 2006-05-03	Çã Ã¢ ¹Î
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Free any windings used by this facelight. It's currently assumed they're not needed again
//-----------------------------------------------------------------------------
void FreeSampleWindings( facelight_t *fl )
{
	int i;
	for (i = 0; i < fl->numsamples; i++)
	{
		if (fl->sample[i].w)
		{
			FreeWinding( fl->sample[i].w );
			fl->sample[i].w = NULL;
		}
	}
}



//-----------------------------------------------------------------------------
// Purpose: build the sample data for each lightmapped primitive type
//-----------------------------------------------------------------------------
bool BuildSamples( lightinfo_t *pLightInfo, facelight_t *pFaceLight, int ndxFace )
{
	// build samples for a "face"
	if( pLightInfo->face->dispinfo == -1 )
	{
		return BuildFaceSamples( pLightInfo, pFaceLight );
	}
	// build samples for a "displacement"
	else
	{
		return StaticDispMgr()->BuildDispSamples( pLightInfo, pFaceLight, ndxFace );
	}
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool BuildFaceLuxels( lightinfo_t *pLightInfo, facelight_t *pFaceLight )
{
	// lightmap size
	int width = pLightInfo->face->m_LightmapTextureSizeInLuxels[0]+1;
	int height = pLightInfo->face->m_LightmapTextureSizeInLuxels[1]+1;

	// calcuate actual luxel points
	pFaceLight->numluxels = width * height;
	pFaceLight->luxel = ( Vector* )calloc( pFaceLight->numluxels, sizeof( *pFaceLight->luxel ) );
	if( !pFaceLight->luxel )
	{
		return false;
	}

	for( int t = 0; t < height; t++ )
	{
		for( int s = 0; s < width; s++ )
		{
			LuxelSpaceToWorld( pLightInfo, s, t, pFaceLight->luxel[s+t*width] );
		}
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: build the luxels (find the luxel centers) for each lightmapped
//          primitive type
//-----------------------------------------------------------------------------
bool BuildLuxels( lightinfo_t *pLightInfo, facelight_t *pFaceLight, int ndxFace )
{
	// build luxels for a "face"
	if( pLightInfo->face->dispinfo == -1 )
	{
		return BuildFaceLuxels( pLightInfo, pFaceLight );
	}
	// build luxels for a "displacement"
	else
	{
		return StaticDispMgr()->BuildDispLuxels( pLightInfo, pFaceLight, ndxFace );
	}
}


//-----------------------------------------------------------------------------
// Purpose: for each face, find the center of each luxel; for each texture
//          aligned grid point, back project onto the plane and get the world
//          xyz value of the sample point
// NOTE: ndxFace = facenum
//-----------------------------------------------------------------------------
void CalcPoints( lightinfo_t *pLightInfo, facelight_t *pFaceLight, int ndxFace )
{
	// debugging!
	if( dumppatches ) 
	{
		DumpFaces( pLightInfo, ndxFace );
	}

	// quick and dirty!
	if( do_fast )
	{
		if( !BuildSamplesAndLuxels_DoFast( pLightInfo, pFaceLight, ndxFace ) )
		{
			Msg( "Face %d: (Fast)Error Building Samples and Luxels\n", ndxFace );
		}
		return;
	}

	// build the samples
	if( !BuildSamples( pLightInfo, pFaceLight, ndxFace ) )
	{
		Msg( "Face %d: Error Building Samples\n", ndxFace );
	}

	// build the luxels
	if( !BuildLuxels( pLightInfo, pFaceLight, ndxFace ) )
	{
		Msg( "Face %d: Error Building Luxels\n", ndxFace );
	}
}


//==============================================================

directlight_t	*activelights;
directlight_t	*freelights;

//!{ 2006-05-03	Çã Ã¢ ¹Î
//facelight_t		facelight[MAX_MAP_FACES];
CUtlVector<facelight_t> facelight;
//!} 2006-05-03	Çã Ã¢ ¹Î

int				numdlights;

/*
==================
FindTargetEntity
==================
*/
entity_t *FindTargetEntity (char *target)
{
	int		i;
	char	*n;

	for (i=0 ; i<num_entities ; i++)
	{
		n = ValueForKey (&entities[i], "targetname");
		if (!strcmp (n, target))
			return &entities[i];
	}

	return NULL;
}



/*
=============
AllocDLight
=============
*/

int GetVisCache( int lastoffset, int cluster, byte *pvs );
void SetDLightVis( directlight_t *dl, int cluster );
void MergeDLightVis( directlight_t *dl, int cluster );

directlight_t *AllocDLight( Vector& origin )
{
	directlight_t *dl;

	dl = ( directlight_t* )calloc(1, sizeof(directlight_t));
	dl->index = numdlights++;

	VectorCopy( origin, dl->light.origin );

	dl->facenum = -1;
	dl->secondaryfacenum = -1;

	dl->next = activelights;
	activelights = dl;

	return dl;
}


void FreeDLights()
{
	gSkylight = NULL;
	gAmbient = NULL;

	directlight_t *pNext;
	for( directlight_t *pCur=activelights; pCur; pCur=pNext )
	{
		pNext = pCur->next;
		free( pCur );
	}
	activelights = 0;
}


void SetDLightVis( directlight_t *dl, int cluster )
{
	if (dl->pvs == NULL)
	{
		dl->pvs = (byte *)calloc( 1, (dvis->numclusters / 8) + 1 );
	}

	GetVisCache( -1, cluster, dl->pvs );
}

void MergeDLightVis( directlight_t *dl, int cluster )
{
	if (dl->pvs == NULL)
	{
		SetDLightVis( dl, cluster );
	}
	else
	{
		byte		pvs[MAX_MAP_CLUSTERS/8];
		GetVisCache( -1, cluster, pvs );

		// merge both vis graphs
		for (int i = 0; i < (dvis->numclusters / 8) + 1; i++)
		{
			dl->pvs[i] |= pvs[i];
		}
	}
}


/*
=============
LightForKey
=============
*/
int LightForKey (entity_t *ent, char *key, Vector& intensity )
{
	char *pLight;

	pLight = ValueForKey( ent, key );

	return LightForString( pLight, intensity );
}

int LightForString( char *pLight, Vector& intensity )
{
	double r, g, b, scaler;
	int argCnt;

	VectorFill( intensity, 0 );

	// scanf into doubles, then assign, so it is vec_t size independent
	r = g = b = scaler = 0;
	argCnt = sscanf ( pLight, "%lf %lf %lf %lf", &r, &g, &b, &scaler );

	intensity[0] = pow( r / 255.0, 2.2 ) * 255; // convert to linear

	if( argCnt == 1 )
	{
		// The R,G,B values are all equal.
		intensity[1] = intensity[2] = intensity[0]; 
	}
	else if ( argCnt == 3 || argCnt == 4 )
	{
		// Save the other two G,B values.
		intensity[1] = pow( g / 255.0, 2.2 ) * 255;
		intensity[2] = pow( b / 255.0, 2.2 ) * 255;

		// Did we also get an "intensity" scaler value too?
		if ( argCnt == 4 )
		{
			// Scale the normalized 0-255 R,G,B values by the intensity scaler
			VectorScale( intensity, scaler / 255.0, intensity );
		}
	}
	else
	{
		return false;
	}

	// scale up source lights by scaling factor
	VectorScale( intensity, lightscale, intensity );
	return true;
}



//-----------------------------------------------------------------------------
// Various parsing methods
//-----------------------------------------------------------------------------

static void ParseLightSpot( entity_t* e, directlight_t* dl )
{
	dl->light.type = emit_spotlight;

	dl->light.stopdot = FloatForKey (e, "_inner_cone");
	if (!dl->light.stopdot)
		dl->light.stopdot = 10;

	dl->light.stopdot2 = FloatForKey (e, "_cone");
	if (!dl->light.stopdot2) 
		dl->light.stopdot2 = dl->light.stopdot;
	if (dl->light.stopdot2 < dl->light.stopdot)
		dl->light.stopdot2 = dl->light.stopdot;

	// This is a point light if stop dots are 180...
	if ((dl->light.stopdot == 180) && (dl->light.stopdot2 == 180))
	{
		dl->light.stopdot = dl->light.stopdot2 = 0;
		dl->light.type = emit_point;
		dl->light.exponent = 0;
	}
	else
	{
		// Clamp to 90, that's all DX8 can handle! 
		if (dl->light.stopdot > 90)
		{
			Warning("WARNING: light_spot at (%i %i %i) has inner angle larger than 90 degrees! Clamping to 90...\n",
				(int)dl->light.origin[0], (int)dl->light.origin[1], (int)dl->light.origin[2]);
			dl->light.stopdot = 90;
		}

		if (dl->light.stopdot2 > 90)
		{
			Warning("WARNING: light_spot at (%i %i %i) has outer angle larger than 90 degrees! Clamping to 90...\n",
				(int)dl->light.origin[0], (int)dl->light.origin[1], (int)dl->light.origin[2]);
			dl->light.stopdot2 = 90;
		}

		dl->light.stopdot2 = (float)cos(dl->light.stopdot2/180*M_PI);
		dl->light.stopdot = (float)cos(dl->light.stopdot/180*M_PI);
		dl->light.exponent = FloatForKey (e, "_exponent");
	}

	dl->light.constant_attn = FloatForKey (e, "_constant_attn");
	dl->light.linear_attn = FloatForKey (e, "_linear_attn");
	dl->light.quadratic_attn = FloatForKey (e, "_quadratic_attn");

	dl->light.radius = FloatForKey (e, "_distance");

	// clamp values to >= 0
	if (dl->light.constant_attn < EQUAL_EPSILON)
		dl->light.constant_attn = 0;

	if (dl->light.linear_attn < EQUAL_EPSILON)
		dl->light.linear_attn = 0;

	if (dl->light.quadratic_attn < EQUAL_EPSILON)
		dl->light.quadratic_attn = 0;

	if (dl->light.constant_attn < EQUAL_EPSILON && dl->light.linear_attn < EQUAL_EPSILON && dl->light.quadratic_attn < EQUAL_EPSILON)
		dl->light.constant_attn = 1;

	// scale intensity for unit 100 distance
	float ratio = (dl->light.constant_attn + 100 * dl->light.linear_attn + 100 * 100 * dl->light.quadratic_attn);
	if (ratio > 0)
	{
		VectorScale( dl->light.intensity, ratio, dl->light.intensity );
	}
}

static void ParseLightEnvironment( entity_t* e, directlight_t* dl )
{
	if (gSkylight)
	{
		MergeDLightVis( gSkylight, ClusterFromPoint(dl->light.origin) );

		MergeDLightVis( gAmbient, ClusterFromPoint(dl->light.origin) );

		// flag hacked in sky/ambient lights to be locally ignored.  
		// These go away once engine can take vis data
		dl->light.type = emit_skylight;
		dl->light.flags = 0xFFFF;

		directlight_t *al = AllocDLight( dl->light.origin );
		al->light.flags = 0xFFFF;
		al->light.type = emit_skyambient;
		VectorCopy( gAmbient->light.intensity, al->light.intensity );
	}
	else
	{
		gSkylight	= dl;

		dl->light.type = emit_skylight;

		gAmbient = AllocDLight( dl->light.origin );
		gAmbient->light.type = emit_skyambient;

		if (!LightForKey( e, "_ambient", gAmbient->light.intensity ))
		{
			VectorScale( dl->light.intensity, 0.5, gAmbient->light.intensity );
		}
	}
}

static void ParseLightPoint( entity_t* e, directlight_t* dl )
{
	dl->light.type = emit_point;

	dl->light.constant_attn = FloatForKey (e, "_constant_attn");
	dl->light.linear_attn = FloatForKey (e, "_linear_attn");
	dl->light.quadratic_attn = FloatForKey (e, "_quadratic_attn");

	dl->light.radius = FloatForKey (e, "_distance");

	// clamp values to >= 0
	if (dl->light.constant_attn < EQUAL_EPSILON)
		dl->light.constant_attn = 0;

	if (dl->light.linear_attn < EQUAL_EPSILON)
		dl->light.linear_attn = 0;

	if (dl->light.quadratic_attn < EQUAL_EPSILON)
		dl->light.quadratic_attn = 0;

	if (dl->light.constant_attn < EQUAL_EPSILON && dl->light.linear_attn < EQUAL_EPSILON && dl->light.quadratic_attn < EQUAL_EPSILON)
		dl->light.constant_attn = 1;

	// scale intensity for unit 100 distance
	float ratio = (dl->light.constant_attn + 100 * dl->light.linear_attn + 100 * 100 * dl->light.quadratic_attn);
	if (ratio > 0)
	{
		VectorScale( dl->light.intensity, ratio, dl->light.intensity );
	}
}

/*
=============
CreateDirectLights
=============
*/
#define DIRECT_SCALE (100.0*100.0)
void CreateDirectLights (void)
{
	unsigned        i;
	patch_t	        *p;
	directlight_t	*dl;
	Vector	        dest;

	numdlights = 0;

	FreeDLights();

	//
	// surfaces
	//
	unsigned int uiPatchCount = patches.Size();
	for (i=0; i< uiPatchCount; i++)
	{
		p = &patches.Element( i );

		// skip parent patches
		if (p->child1 != patches.InvalidIndex() )
			continue;

		if (p->basearea < 1e-6)
			continue;

		if( VectorAvg( p->baselight ) >= dlight_threshold )
		{
			dl = AllocDLight( p->origin );

			dl->light.type = emit_surface;
			VectorCopy (p->normal, dl->light.normal);

			// scale intensity by number of texture instances
			VectorScale( p->baselight, lightscale * p->area * p->scale[0] * p->scale[1] / p->basearea, dl->light.intensity );
		}
	}

	//<@ ava specific ;
	extern float hdr_tonemapscale;
	float hdr_scaler = 1.0f;

	if (hdr_tonemapscale != 0)
		hdr_scaler = 1.0f / hdr_tonemapscale;
	//>@ ava

	
	//!{ 2006-04-06	Çã Ã¢ ¹Î
	for( int iLight = 0; iLight < s_pVRadDll2->NumPointLights_; ++iLight)
	{
		AvaPointLight& Light = s_pVRadDll2->PointLightArray_[iLight];

		const int light_style = Light.bLightMapLight_ ? 0 : 1;

		dl = AllocDLight( Light.Position_ );

		dl->light.style				= light_style;
		dl->light.type				= emit_ue3point;
		dl->light.intensity			= Light.Intensity_ * hdr_scaler;
		dl->light.radius			= Light.Radius_;
		dl->light.exponent			= Light.Exponent_;
		dl->light_id				= Light.LightId_;

		// clamp values to >= 0
		if (dl->light.constant_attn < EQUAL_EPSILON)
			dl->light.constant_attn = 0;

		if (dl->light.linear_attn < EQUAL_EPSILON)
			dl->light.linear_attn = 0;

		if (dl->light.quadratic_attn < EQUAL_EPSILON)
			dl->light.quadratic_attn = 0;

		if (dl->light.constant_attn < EQUAL_EPSILON && dl->light.linear_attn < EQUAL_EPSILON && dl->light.quadratic_attn < EQUAL_EPSILON)
			dl->light.constant_attn = 1;

		// scale intensity for unit 100 distance
		float ratio = (dl->light.constant_attn + 100 * dl->light.linear_attn + 100 * 100 * dl->light.quadratic_attn);
		if (ratio > 0)
		{
			VectorScale( dl->light.intensity, ratio, dl->light.intensity );
		}
	}

	//!{ 2006-04-06	Çã Ã¢ ¹Î
	for( int iLight = 0; iLight < s_pVRadDll2->NumSpotLights_; ++iLight)
	{
		AvaSpotLight& Light = s_pVRadDll2->SpotLightArray_[iLight];

		const int light_style = Light.bLightMapLight_ ? 0 : 1;

		
		dl = AllocDLight( Light.Position_ );

		dl->light.style				= light_style;
		dl->light.type				= emit_ue3spot;
		dl->light.intensity			= Light.Intensity_ * hdr_scaler;
		dl->light.radius			= Light.Radius_;
		dl->light.exponent			= Light.Exponent_;
		dl->light_id				= Light.LightId_;

		float ClampedInnerConeAngle = clamp(Light.InnerConeAngle_,0.0f,89.0f) * (FLOAT)M_PI / 180.0f,
			ClampedOuterConeAngle = clamp(Light.OuterConeAngle_ * (FLOAT)M_PI / 180.0f,ClampedInnerConeAngle + 0.001f,89.0f * (FLOAT)M_PI / 180.0f + 0.001f),
			OuterCone = cosf(ClampedOuterConeAngle),
			InnerCone = cosf(ClampedInnerConeAngle);

		dl->light.stopdot			= InnerCone;
		dl->light.stopdot2			= OuterCone;
		dl->light.normal			= Light.Direction_;

		// clamp values to >= 0
		if (dl->light.constant_attn < EQUAL_EPSILON)
			dl->light.constant_attn = 0;

		if (dl->light.linear_attn < EQUAL_EPSILON)
			dl->light.linear_attn = 0;

		if (dl->light.quadratic_attn < EQUAL_EPSILON)
			dl->light.quadratic_attn = 0;

		if (dl->light.constant_attn < EQUAL_EPSILON && dl->light.linear_attn < EQUAL_EPSILON && dl->light.quadratic_attn < EQUAL_EPSILON)
			dl->light.constant_attn = 1;

		// scale intensity for unit 100 distance
		float ratio = (dl->light.constant_attn + 100 * dl->light.linear_attn + 100 * 100 * dl->light.quadratic_attn);
		if (ratio > 0)
		{
			VectorScale( dl->light.intensity, ratio, dl->light.intensity );
		}
	}

	if (s_pVRadDll2->SunLight_.Intensity_.Length() != 0)
	{
		const int light_style = s_pVRadDll2->SunLight_.bLightMapLight_ ? 0 : 1;

		bool result = Pbrt::LoadEnvironmentMap( s_pVRadDll2->SunLight_.SkyEnvMapFileName_,
			0.0f,								// exposure
			1.0f,								// sun scale
			s_pVRadDll2->SunLight_.SkyMax_,		// sky max
			s_pVRadDll2->SunLight_.SkyScale_,	// sky scale // max¸¦ ³ÑÁö ¾ÊÀ»¶§, scale°ªÀ» °öÇÏ¿© °è»êÇÑ´Ù.
			s_pVRadDll2->SunLight_.Yaw_ );

		const bool bUseHdrEnvironmentMap = result;

		if( bUseHdrEnvironmentMap )
		{
			directlight_t *al = AllocDLight( Vector( 0, 0, 0 ));
			al->light.style = light_style;
			al->light.flags = 0;
			al->light.type = emit_hdrEnvmapSkylight;
			al->light.intensity = Vector(0, 0, 0);
			gAmbient = al;

			//Tonemap::UseLinearAdaption( s_pVRadDll2->SunLight_.bUseLinearAdaption_ );
		}
		else
		{
			dl = AllocDLight( Vector( 0, 0, 0 ) );
			dl->light.style				= light_style;
			dl->light.intensity			= s_pVRadDll2->SunLight_.Intensity_;

			// flag hacked in sky/ambient lights to be locally ignored.  
			// These go away once engine can take vis data
			dl->light.type = emit_skylight;
			dl->light.flags = 0;
			dl->light.normal = s_pVRadDll2->SunLight_.Direction_;
			dl->light_id = s_pVRadDll2->SunLight_.LightId_;

			if( light_style == 0 )
			{
				gSkylight = dl;
			}

			if (s_pVRadDll2->SunLight_.Ambient_.LengthSqr() > 0)
			{
				directlight_t *al = AllocDLight( Vector( 0, 0, 0 ));
				al->light.style = 0;
				al->light.flags = 0;
				al->light.type = emit_skyambient;
				al->light.intensity = s_pVRadDll2->SunLight_.Ambient_;
				gAmbient = al;
			}		
		}
		

	}
	//!} 2006-04-06	Çã Ã¢ ¹Î

	qprintf ("%i direct lights\n", numdlights);
}

/*
=============
DeleteDirectLights
=============
*/

void DeleteDirectLights(void)
{
	//<@ Secondary Light Source : 2007. 8. 17 changmin
	// secondary light sourceÀÇ °³¼ö´Â... ¸¹À» °ÍÀÌ´Ù...
	return;
	//>@ Secondary Light Source
	directlight_t		*dl;

	for (dl = activelights; dl != NULL; dl = dl->next )
	{
		dworldlight_t *wl = &dworldlights[numworldlights++];

		if (numworldlights > MAX_MAP_WORLDLIGHTS)
		{
			Error("too many lights %d / %d\n", numworldlights, MAX_MAP_WORLDLIGHTS );
		}
		wl->cluster	= dl->light.cluster;
		wl->type	= dl->light.type;
		wl->style	= dl->light.style;
		VectorCopy( dl->light.origin, wl->origin );
		// FIXME: why does vrad want 0 to 255 and not 0 to 1??
		VectorScale( dl->light.intensity, (1.0 / 255.0), wl->intensity );
		VectorCopy( dl->light.normal, wl->normal );
		wl->stopdot	= dl->light.stopdot;
		wl->stopdot2 = dl->light.stopdot2;
		wl->exponent = dl->light.exponent;
		wl->radius = dl->light.radius;
		wl->constant_attn = dl->light.constant_attn;
		wl->linear_attn = dl->light.linear_attn;
		wl->quadratic_attn = dl->light.quadratic_attn;
	}
}

/*
=============
GatherSampleLight
=============
*/
#define NUMVERTEXNORMALS	162
#define NORMALFORMFACTOR	40.156979 // accumuated dot products for hemisphere
Vector	r_avertexnormals[NUMVERTEXNORMALS] = {
#include "anorms.h"
};

#define TEST_HEMISPHERE_DIRECTION 1

#include <vector>
using namespace std;
vector<Pbrt::Sample_t*> GPbrtCachedSamples;

static struct PbrtJanitor
{
	~PbrtJanitor()
	{
		vector<Pbrt::Sample_t*>::iterator it = GPbrtCachedSamples.begin(), itEnd = GPbrtCachedSamples.end();

		for (;it!=itEnd;++it)
		{
			free( *it );
		}
	}
} __PbrtJanitor;

bool UseCosineWeightedMethod( Vector const& pos, NVector const& normal, const NVector* extra_normals, Vector* skyIrradiance, const int sampleCount )
{
	Pbrt::Sample_t *usingSamples = NULL;
	while (normal.index >= GPbrtCachedSamples.size())
	{
		GPbrtCachedSamples.insert( GPbrtCachedSamples.end(), 1024, NULL );
	}	
	usingSamples = GPbrtCachedSamples[ normal.index ];
	if (usingSamples == NULL)
	{
		Pbrt::Sample_t* samples = usingSamples = GPbrtCachedSamples[ normal.index ] = (Pbrt::Sample_t*)malloc( sizeof(Pbrt::Sample_t) * Pbrt::MAX_SAMPLES );
		Pbrt::CosineWeightedHemiSphereSample( normal, Pbrt::MAX_SAMPLES, samples );
		for (int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
		{
			Pbrt::SampleSkyFromEnvironmentMap( samples[sampleIndex].wi, &samples[sampleIndex].radiance );
		}		
	}

	Vector wi;
	float pdf;
	bool isLightRelevant = false;

	// for sky sampling
	for (int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
	{
		wi = usingSamples[sampleIndex].wi;
		pdf = usingSamples[sampleIndex].pdf;

		// back face
		float dot = DotProduct( normal, wi );

		if ( dot <= EQUAL_EPSILON )
			continue;

		//<@ ava specific ; 2007. 1. 29 changmin
		// skip sampling ground in sample simple sky mode
		if( sampleskylevel == 1 )
		{
			static Vector groundVector( 0.0f, 0.0f, -1.0f );
			if ( DotProduct( wi, groundVector ) > 0.0f )
			{
				continue;
			}
		}
		//>@ ava

		Vector delta;
		VectorScale( wi, MAX_TRACE_LENGTH, delta );
		VectorAdd( pos, delta, delta );

		Vector adjusted_pos = pos;
		adjusted_pos += wi;

		if ( TestLine (adjusted_pos, delta, 0, 0) != CONTENTS_EMPTY )
		{
			continue;
		}

		isLightRelevant = true;

		if( pdf > 0.0f )
		{
			const Vector& skySample = usingSamples[sampleIndex].radiance;

			VectorMA( skyIrradiance[0], dot / pdf, skySample, skyIrradiance[0] );

			if (extra_normals)
			{
				float dot1 = DotProduct( extra_normals[0], wi );
				float dot2 = DotProduct( extra_normals[1], wi );
				float dot3 = DotProduct( extra_normals[2], wi );

				if (dot1<0) dot1 = 0;
				if (dot2<0) dot2 = 0;
				if (dot3<0) dot3 = 0;

				float dotSum = dot1 + dot2 + dot3;				

				float basis1Weight = dot1 / dotSum;
				float basis2Weight = dot2 / dotSum;
				float basis3Weight = dot3 / dotSum;

				VectorMA( skyIrradiance[1], basis1Weight * dot / pdf, skySample, skyIrradiance[1] );
				VectorMA( skyIrradiance[2], basis2Weight * dot / pdf, skySample, skyIrradiance[2] );
				VectorMA( skyIrradiance[3], basis3Weight * dot / pdf, skySample, skyIrradiance[3] );				
			}
		}
	}

	VectorScale( skyIrradiance[0], 1.0f / (float) sampleCount, skyIrradiance[0] );
	if( extra_normals )
	{
		VectorScale( skyIrradiance[1], 1.0f / (float) sampleCount, skyIrradiance[1] );
		VectorScale( skyIrradiance[2], 1.0f / (float) sampleCount, skyIrradiance[2] );
		VectorScale( skyIrradiance[3], 1.0f / (float) sampleCount, skyIrradiance[3] );
	}

	return isLightRelevant;
}

bool UseBrightnessWeightedMethod( Vector const& pos, NVector const& normal, const NVector* extra_normals, Vector* skyIrradiance, const int sampleCount )
{
	// for sun sampling
	Pbrt::EnvmapSample_t* envSamples;

	Pbrt::GetEnvironmentMapSamples( &envSamples );

	Vector wi;
	float pdf;
	float dot;
	Vector delta;

	bool isLightRelevant = false;

	for( int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex )
	{
		wi = envSamples[sampleIndex].wi;
		pdf = envSamples[sampleIndex].pdf;

		dot = DotProduct( normal, wi );

		// back face
		if( dot < EQUAL_EPSILON )
		{
			continue;
		}

		//<@ ava specific ; 2007. 1. 29 changmin
		// skip sampling ground in sample simple sky mode
		if( sampleskylevel == 1 )
		{
			static Vector groundVector( 0.0f, 0.0f, -1.0f );
			if ( DotProduct( wi, groundVector ) > 0.0f )
			{
				continue;
			}
		}
		//>@ ava

		// check occlude
		VectorScale( wi, MAX_TRACE_LENGTH, delta );
		VectorAdd( pos, delta, delta );

		Vector adjusted_pos = pos;
		adjusted_pos += wi;

		if ( TestLine (adjusted_pos, delta, 0, 0) != CONTENTS_EMPTY )
		{
			continue;
		}

		isLightRelevant = true;

		if( pdf > 0.f )
		{
			const Vector& skySample = envSamples[sampleIndex].scaledRadiance;

			VectorMA( skyIrradiance[0], dot / pdf, skySample, skyIrradiance[0] );

			if (extra_normals)
			{
				float dot1 = DotProduct( extra_normals[0], wi );
				float dot2 = DotProduct( extra_normals[1], wi );
				float dot3 = DotProduct( extra_normals[2], wi );

				if( dot1 < 0.0f ) dot1 = 0.0f;
				if( dot2 < 0.0f ) dot2 = 0.0f;
				if( dot3 < 0.0f ) dot3 = 0.0f;

				float dotSum = dot1 + dot2 + dot3;

				float basis1Weight = dot1 / dotSum;
				float basis2Weight = dot2 / dotSum;
				float basis3Weight = dot3 / dotSum;

				if( basis1Weight > 0.0f )
				{
					VectorMA( skyIrradiance[1], basis1Weight * dot / pdf, skySample, skyIrradiance[1] );
				}

				if( basis2Weight > 0.0f )
				{
					VectorMA( skyIrradiance[2], basis2Weight * dot / pdf, skySample, skyIrradiance[2] );
				}

				if( basis3Weight > 0.0f )
				{
					VectorMA( skyIrradiance[3], basis3Weight * dot / pdf, skySample, skyIrradiance[3] );
				}
			}
		}
	}

	VectorScale( skyIrradiance[0], 1.0f / (float) sampleCount, skyIrradiance[0] );

	if( extra_normals )
	{
		VectorScale( skyIrradiance[1], 1.0f / (float) sampleCount, skyIrradiance[1] );
		VectorScale( skyIrradiance[2], 1.0f / (float) sampleCount, skyIrradiance[2] );
		VectorScale( skyIrradiance[3], 1.0f / (float) sampleCount, skyIrradiance[3] );
	}

	return isLightRelevant;
}

// returns dot product with normal and delta
// dl - light
// pos - position of sample
// normal - surface normal of sample
// delta - returned direction to light source
// falloff - amount of light falloff
float GatherSampleLight(
	directlight_t *dl,
	int facenum, 
	Vector const& pos,
	NVector const& normal,
	Vector& delta, 
	float *scale,
	int iThread,
	const NVector* extra_normals /*= NULL*/,
	float* extra_dots /*= NULL */,
	Vector* extra_skys /*= NULL*/,
	Vector* extra_skys2 /*= NULL */)
{
	float			dot = 0.f, dot2 = 0.f;
	float			dist = 0.f;

	bool isLightRelevant = false;

	//!{ 2006-07-26	Çã Ã¢ ¹Î
	if( dl->light.type == emit_hdrEnvmapSkylight )
	{
		// FIXME: temp hacked in dup skylights until engine updated
		if (dl->light.flags)
			return 0.0;

		if ( sampleskylevel == 0 )
			return 0.0f;

		if( sampleskylevel > 0 )
		{
			const Vector& sunDirection = Pbrt::GetSunDirection();
			float sunDot = DotProduct( normal, sunDirection );
			// sun is in back face
			if( sunDot < EQUAL_EPSILON )
			{
				INT sampleCount = (sampleskylevel == 1) ? 128 : Pbrt::MAX_SAMPLES;
				isLightRelevant = UseCosineWeightedMethod( pos, normal, extra_normals, extra_skys, sampleCount );
				if( dl->light.style == 1 && extra_skys2 )
				{
					memcpy( extra_skys2, extra_skys, sizeof(Vector) * 4 );
				}
			}
			else
			{
				// check sun visibility
				VectorScale( sunDirection, MAX_TRACE_LENGTH, delta );
				VectorAdd( pos, delta, delta );
				if ( TestLine (pos, delta, 0, iThread) != CONTENTS_EMPTY )
				{
					INT sampleCount = (sampleskylevel == 1) ? 128 : Pbrt::MAX_SAMPLES;
					isLightRelevant = UseCosineWeightedMethod( pos, normal, extra_normals, extra_skys, sampleCount );
					if( dl->light.style == 1 && extra_skys2 )
					{
						memcpy( extra_skys2, extra_skys, sizeof(Vector) * 4 );
					}
				}
				else
				{
					INT sampleCount = (sampleskylevel == 1) ? 128 : Pbrt::MAX_SUN_SAMPLES;
					isLightRelevant = UseBrightnessWeightedMethod(  pos, normal, extra_normals, extra_skys, sampleCount );
					// sample sky
					if( dl->light.style == 1 && extra_skys2 )
					{
						gSunVisible = true;
						UseCosineWeightedMethod( pos, normal, extra_normals, extra_skys2, sampleCount );
					}
				}
			}
		}
		if( isLightRelevant )
		{
			return 1.0f;
		}
		else
		{
			return 0.0f;
		}
	}
	//!} 2006-07-26	Çã Ã¢ ¹Î
	// skylights work fundamentally differently than normal lights
	else if (dl->light.type == emit_skylight)
	{
		// FIXME: temp hacked in dup skylights until engine updated
		if (dl->light.flags)
			return 0.0;

		if ( sampleskylevel == 0 )
			return 0.0f;

		// make sure the angle is okay
		dot = -DotProduct( normal, dl->light.normal );
		if (dot <= EQUAL_EPSILON)
			return 0.0;

		// search back to see if we can hit a sky brush
		VectorScale( dl->light.normal, -MAX_TRACE_LENGTH, delta );
		VectorAdd( pos, delta, delta );

		if ( TestLine (pos, delta, 0, iThread) != CONTENTS_EMPTY )
			return 0.0;	// occluded		
		
		VectorNormalize(delta);
		*scale = 1.0;
		return dot;
	} 
	else if (dl->light.type == emit_skyambient )
	{
		// FIXME: temp hacked in dup skylights until engine updated
		if (dl->light.flags)
			return 0.0;

		if ( sampleskylevel == 0 )
			return 0.0f;

		float ambient_intensity = 0;

		float extra_sky_intensity[3] = { 0, 0, 0 };

		int j;

		if( sampleskylevel > 0 )
		{
			//!{ 2006-07-11	Çã Ã¢ ¹Î
			const INT SampleCount = sampleskylevel == 1 ? 128 : Pbrt::MAX_SAMPLES;

			Pbrt::Sample_t* cachedSamples = Pbrt::GetCachedSample( normal );
			Pbrt::Sample_t samples[Pbrt::MAX_SAMPLES];

			if( cachedSamples == NULL  )
			{
				Pbrt::CosineWeightedHemiSphereSample( normal, Pbrt::MAX_SAMPLES, samples );
			}
			
			Pbrt::Sample_t* usingSamples = cachedSamples ? cachedSamples : samples;

			Vector wi;
			float pdf;

			// for sky sampling
			for (j = 0; j < SampleCount; j++)
			{
				wi = usingSamples[j].wi;
				pdf = usingSamples[j].pdf;

				// back face
				dot = DotProduct( normal, wi );

				if ( dot <= EQUAL_EPSILON )
					continue;

				VectorScale( wi, MAX_TRACE_LENGTH, delta );
				VectorAdd( pos, delta, delta );

				if ( TestLine (pos, delta, 0, iThread) != CONTENTS_EMPTY )
				{
					continue;
				}

				if( pdf > 0.0f )
				{
					ambient_intensity += ( dot / pdf );

					if (extra_normals)
					{
						float dot1 = DotProduct( extra_normals[0], wi );
						float dot2 = DotProduct( extra_normals[1], wi );
						float dot3 = DotProduct( extra_normals[2], wi );

						float dotSum = dot1 < 0.0 ? 0.0f : dot1;
						dotSum = dot2 < 0.0f ? dotSum : dotSum + dot2;
						dotSum = dot3 < 0.0f ? dotSum : dotSum + dot3;

						float basis1Weight = dot1 / dotSum;
						float basis2Weight = dot2 / dotSum;
						float basis3Weight = dot3 / dotSum;
						
						if( dot1 > EQUAL_EPSILON )
						{
							extra_sky_intensity[0] += ( basis1Weight * dot  / pdf );
						}

						if( dot2 > EQUAL_EPSILON )
						{
							extra_sky_intensity[1] += ( basis2Weight * dot / pdf );
						}

						if( dot3 > EQUAL_EPSILON )
						{
							extra_sky_intensity[2] += ( basis3Weight * dot / pdf );
						}
					}
				}
			}

			delta = normal;
			*scale = 1.0;	// no fall off
			
			if (extra_normals)
			{
				extra_dots[0] = ( extra_sky_intensity[0] / (float)SampleCount );
				extra_dots[1] = ( extra_sky_intensity[1] / (float)SampleCount );
				extra_dots[2] = ( extra_sky_intensity[2] / (float)SampleCount );
			}

			return ( ambient_intensity / (float)(SampleCount) );
		}
	}
	else
	{
		Vector	src;

		if (dl->facenum == -1)
		{
			VectorCopy( dl->light.origin, src );
		}
		else
		{
			src.Init( 0, 0, 0 );
		}

		VectorSubtract (src, pos, delta);
		dist = VectorNormalize (delta);
		dot = DotProduct (delta, normal);
		if (dot <= EQUAL_EPSILON)
			return 0.0;	// behind sample surface

		if (dist < 1.0)
			dist = 1.0;


		switch (dl->light.type)
		{
			//!{ 2006-04-06	Çã Ã¢ ¹Î
			case emit_ue3point:
				{
					float dist_scale = dist / dl->light.radius;					
					*scale = powf( max( 1.0f - dist_scale * dist_scale, 0.0f ), dl->light.exponent );					
				}
				break;
			case emit_ue3spot :
				{
					dot2 = -DotProduct (delta, dl->light.normal);
					if (dot2 <= dl->light.stopdot2)
						return 0.0; // outside light cone

					float dist_scale = dist / dl->light.radius;					
					*scale = powf( max( 1.0f - dist_scale * dist_scale, 0.0f ), dl->light.exponent );										

					if (dot2 <= dl->light.stopdot)
					{
						dot2 = ((dot2) - dl->light.stopdot2) / (dl->light.stopdot - dl->light.stopdot2);
						*scale *= dot2 * dot2;
					}					
				}
				break;
			//!} 2006-04-06	Çã Ã¢ ¹Î

			case emit_point:
				*scale = 1.0 / (dl->light.constant_attn + dl->light.linear_attn * dist + dl->light.quadratic_attn * dist * dist);
				break;

			case emit_surface:
				//<@ Secondary Light Source
				// facenum == -2 ÀÌ¸é vertex lighting
				if( facenum == dl->secondaryfacenum )
				{
					return 0.0f;
				}
				//>@ Secondary Light Source
				dot2 = -DotProduct (delta, dl->light.normal);
				if (dot2 <= EQUAL_EPSILON)
					return 0.0; // behind light surface
				*scale = dot2 / (dist * dist);

				VectorSubtract( src, delta, src );

				break;

			case emit_spotlight:
				dot2 = -DotProduct (delta, dl->light.normal);
				if (dot2 <= dl->light.stopdot2)
					return 0.0; // outside light cone

				*scale = dot2 / (dl->light.constant_attn + dl->light.linear_attn * dist + dl->light.quadratic_attn * dist * dist);
				if (dot2 <= dl->light.stopdot) // outside inner cone
				{
					if ((dl->light.exponent == 0.0f) || (dl->light.exponent == 1.0f))
						*scale *= (dot2 - dl->light.stopdot2) / (dl->light.stopdot - dl->light.stopdot2);
					else
						*scale *= pow((dot2 - dl->light.stopdot2) / (dl->light.stopdot - dl->light.stopdot2), dl->light.exponent);
				}
				break;

			default:
				Error ("Bad dl->light.type");
				*scale = 0.0;
				break;
		}

		if ( TestLine (pos, src, 0, iThread) != CONTENTS_EMPTY )
			return 0.0;	// occluded
	}
	return dot;
}



/*
=============
AddSampleToPatch

Take the sample's collected light and
add it back into the apropriate patch
for the radiosity pass.
=============
*/
void AddSampleToPatch (sample_t *s, Vector& light, int facenum)
{
	patch_t	*patch;
	Vector	mins, maxs;
	int		i;

	if (numbounce == 0)
		return;
	if( VectorAvg( light ) < 0.01f)
		return;

	//
	// fixed the sample position and normal -- need to find the equiv pos, etc to set up 
	// patches
	//
	if( facePatches.Element( facenum ) == facePatches.InvalidIndex() )
		return;

	bool bDisp = ( dfaces[facenum].dispinfo != -1 );

	patch_t *pNextPatch = NULL;
	for( patch = &patches.Element( facePatches.Element( facenum ) ); patch; patch = pNextPatch )
	{
		// next patch
		pNextPatch = NULL;
		if( patch->ndxNext != patches.InvalidIndex() )
		{
			pNextPatch = &patches.Element( patch->ndxNext );
		}

		if (patch->sky)
			continue;

		// skip patches with children
		if ( patch->child1 != patches.InvalidIndex() )
		 	continue;

		// see if the point is in this patch (roughly)
		if( !bDisp )
		{
			WindingBounds (patch->winding, mins, maxs);
		}
		else
		{
			mins = patch->mins;
			maxs = patch->maxs;
		}

		for (i=0 ; i<3 ; i++)
		{
			if (mins[i] > s->pos[i] + 2)
				goto nextpatch;
			if (maxs[i] < s->pos[i] - 2)
				goto nextpatch;
		}

		// add the sample to the patch
		patch->samplearea += s->area;
		VectorMA( patch->samplelight, s->area, light, patch->samplelight );
		return;

nextpatch:;
	}
	// don't worry if some samples don't find a patch
}


void GetPhongNormal( int facenum, Vector const& spot, Vector& phongnormal)
{
	int	j;
	dface_t		*f = &dfaces[facenum];
//	dplane_t	*p = &dplanes[f->planenum];
	Vector		facenormal, vspot;

	VectorCopy( dplanes[f->planenum].normal, facenormal );
	VectorCopy( facenormal, phongnormal );

	if ( smoothing_threshold != 1 )
	{
		faceneighbor_t *fn = &faceneighbor[facenum];

		// Calculate modified point normal for surface
		// Use the edge normals iff they are defined.  Bend the surface towards the edge normal(s)
		// Crude first attempt: find nearest edge normal and do a simple interpolation with facenormal.
		// Second attempt: find edge points+center that bound the point and do a three-point triangulation(baricentric)
		// Better third attempt: generate the point normals for all vertices and do baricentric triangulation.

		for (j=0 ; j<f->numedges ; j++)
		{
			Vector	v1, v2;
			//int e = dsurfedges[f->firstedge + j];
			//int e1 = dsurfedges[f->firstedge + ((j+f->numedges-1)%f->numedges)];
			//int e2 = dsurfedges[f->firstedge + ((j+1)%f->numedges)];

			//edgeshare_t	*es = &edgeshare[abs(e)];
			//edgeshare_t	*es1 = &edgeshare[abs(e1)];
			//edgeshare_t	*es2 = &edgeshare[abs(e2)];
			// dface_t	*f2;
			float		a1, a2, aa, bb, ab;
			int			vert1, vert2;

			Vector& n1 = fn->normal[j];
			Vector& n2 = fn->normal[(j+1)%f->numedges];

			/*
			if (VectorCompare( n1, fn->facenormal ) 
				&& VectorCompare( n2, fn->facenormal) )
				continue;
			*/

			vert1 = EdgeVertex( f, j );
			vert2 = EdgeVertex( f, j+1 );

			Vector& p1 = dvertexes[vert1].point;
			Vector& p2 = dvertexes[vert2].point;

			// Build vectors from the middle of the face to the edge vertexes and the sample pos.
			VectorSubtract( p1, face_centroids[facenum], v1 );
			VectorSubtract( p2, face_centroids[facenum], v2 );
			VectorSubtract( spot, face_centroids[facenum], vspot );
			aa = DotProduct( v1, v1 );
			bb = DotProduct( v2, v2 );
			ab = DotProduct( v1, v2 );
			a1 = (bb * DotProduct( v1, vspot ) - ab * DotProduct( vspot, v2 )) / (aa * bb - ab * ab);
			a2 = (DotProduct( vspot, v2 ) - a1 * ab) / bb;

			// Test center to sample vector for inclusion between center to vertex vectors (Use dot product of vectors)
			if ( a1 >= 0.0 && a2 >= 0.0)
			{
				// calculate distance from edge to pos
				Vector	temp;
				float scale;
				
				// Interpolate between the center and edge normals based on sample position
				scale = 1.0 - a1 - a2;
				VectorScale( fn->facenormal, scale, phongnormal );
				VectorScale( n1, a1, temp );
				VectorAdd( phongnormal, temp, phongnormal );
				VectorScale( n2, a2, temp );
				VectorAdd( phongnormal, temp, phongnormal );
				VectorNormalize( phongnormal );

				/*
				if (a1 > 1 || a2 > 1 || a1 + a2 > 1)
				{
					Msg("\n%.2f %.2f\n", a1, a2 );
					Msg("%.2f %.2f %.2f\n", v1[0], v1[1], v1[2] );
					Msg("%.2f %.2f %.2f\n", v2[0], v2[1], v2[2] );
					Msg("%.2f %.2f %.2f\n", vspot[0], vspot[1], vspot[2] );
					exit(1);

					a1 = 0;
				}
				*/
				/*
				phongnormal[0] = (((j + 1) & 4) != 0) * 255;
				phongnormal[1] = (((j + 1) & 2) != 0) * 255;
				phongnormal[2] = (((j + 1) & 1) != 0) * 255;
				*/
				return;
			}
		}
	}
}





int GetVisCache( int lastoffset, int cluster, byte *pvs )
{
	// get the PVS for the pos to limit the number of checks
    if ( !visdatasize )
    {       
        memset (pvs, 255, (dvis->numclusters+7)/8 );
        lastoffset = -1;
    }
    else 
    {
		if (cluster < 0)
		{
			// Error, point embedded in wall
			// sampled[0][1] = 255;
			memset (pvs, 255, (dvis->numclusters+7)/8 );
			lastoffset = -1;
		}
		else
		{
			int thisoffset = dvis->bitofs[ cluster ][DVIS_PVS];
			if ( thisoffset != lastoffset )
			{ 
				if ( thisoffset == -1 )
				{
					Error ("visofs == -1");
				}

				DecompressVis (&dvisdata[thisoffset], pvs);
			}
			lastoffset = thisoffset;
		}
    }
	return lastoffset;
}


void BuildPatchLights( int facenum );

void DumpSamples( int ndxFace, facelight_t *pFaceLight )
{
	ThreadLock();

	dface_t *pFace = &dfaces[ndxFace];
	if( pFace )
	{
		for( int ndxStyle = 0; ndxStyle < 4; ndxStyle++ )
		{
			if( pFace->styles[ndxStyle] != 255 )
			{
				

				for( int ndxSample = 0; ndxSample < pFaceLight->numsamples; ndxSample++ )
				{
					sample_t *pSample = &pFaceLight->sample[ndxSample];

					WriteWinding( pFileSamples[ndxStyle], pSample->w, pFaceLight->light[ndxStyle][0][ndxSample] );
					if( bDumpNormals )
					{
						WriteNormal( pFileSamples[ndxStyle], pSample->pos, pSample->normal, 15.0f, pSample->normal * 255.0f );
					}
				}
			}
		}
	}

	ThreadUnlock();
}


//-----------------------------------------------------------------------------
// Allocates light sample data
//-----------------------------------------------------------------------------
static inline void AllocateLightstyleSamples( facelight_t* fl, int styleIndex, int numnormals )
{	
	for (int n = 0; n < numnormals; ++n)
	{
		fl->light[styleIndex][n] = ( Vector* )calloc(fl->numsamples, sizeof(Vector));
	}
}


//-----------------------------------------------------------------------------
// Used to find an existing lightstyle on a face
//-----------------------------------------------------------------------------
static inline int FindLightstyle( dface_t* f, int lightstyle )
{
 	for (int k = 0; k < MAXLIGHTMAPS; k++)
	{
		if (f->styles[k] == lightstyle)
			return k;
	}

	return -1;
}

static int FindOrAllocateLightstyleSamples( dface_t* f, facelight_t	*fl, int lightstyle, int numnormals )
{
	// Search the lightstyles associated with the face for a match
	int k;
 	for (k = 0; k < MAXLIGHTMAPS; k++)
	{
		if (f->styles[k] == lightstyle)
			break;

		// Found an empty entry, we can use it for a new lightstyle
		if (f->styles[k] == 255)
		{
			AllocateLightstyleSamples( fl, k, numnormals );
			f->styles[k] = lightstyle;
			break;
		}
	}

	// Check for overflow
	if (k >= MAXLIGHTMAPS)
		return -1;

	return k;
}

//-----------------------------------------------------------------------------
// Compute the illumination point + normal for the sample
//-----------------------------------------------------------------------------
static void ComputeIlluminationPointAndNormals( lightinfo_t const& l, 
	Vector const& samplePosition, NVector const& sampleNormal, SampleInfo_t* pInfo )
{
	// FIXME: move sample point off the surface a bit, this is done so that
	// light sampling will not be affected by a bug	where raycasts will
	// intersect with the face being lit. We really should just have that
	// logic in GatherSampleLight
	VectorAdd( samplePosition, l.facenormal, pInfo->m_Point );

	if( pInfo->m_IsDispFace )
    {
		// FIXME: un-work around the bug fix workaround above; displacements
		// correctly deal with self-shadowing issues
		pInfo->m_Point = samplePosition;
		pInfo->m_PointNormal[0] = sampleNormal;

		if( pInfo->m_NormalCount > 1 )
		{
			// use facenormal along with the smooth normal to build the three bump map vectors
			GetBumpNormals( pInfo->m_pTexInfo->textureVecsTexelsPerWorldUnits[0], 
					        pInfo->m_pTexInfo->textureVecsTexelsPerWorldUnits[1], l.facenormal, 
					        sampleNormal, &pInfo->m_PointNormal[1] );
		}
	}
	else if (!l.isflat)
	{
		// If the face isn't flat, use a phong-based normal instead
   		GetPhongNormal( pInfo->m_FaceNum, samplePosition, pInfo->m_PointNormal[0] );

		if( pInfo->m_NormalCount > 1 )
		{
			// use facenormal along with the smooth normal to build the three bump map vectors
			GetBumpNormals( pInfo->m_pTexInfo->textureVecsTexelsPerWorldUnits[0], 
				pInfo->m_pTexInfo->textureVecsTexelsPerWorldUnits[1], l.facenormal, 
				pInfo->m_PointNormal[0], &pInfo->m_PointNormal[1] );
		}
    }
	pInfo->m_PointNormal[0].getIndex();
}


inline byte PVSCheck( const byte *pvs, int iCluster )
{
	if ( iCluster >= 0 )
	{
		return pvs[iCluster >> 3] & ( 1 << ( iCluster & 7 ) );
	}
	else
	{
		// PointInLeaf still returns -1 for valid points sometimes and rather than 
		// have black samples, we assume the sample is in the PVS.
		return 1;
	}
}


//-----------------------------------------------------------------------------
// Iterates over all lights and computes lighting at a sample point
//-----------------------------------------------------------------------------
static void GatherSampleLightAtPoint( SampleInfo_t& info, int sampleIdx )
{
	Vector delta;
	float falloff, dot;

	// Iterate over all direct lights and add them to the particular sample
	for (directlight_t *dl = activelights; dl != NULL; dl = dl->next)
	{
		float	bump_dots[3];
		Vector	skyIrradiance[4];
		memset( skyIrradiance, 0, sizeof( Vector ) * 4 );

		Vector skyIrradiance2[4];
		memset( skyIrradiance2, 0, sizeof( Vector ) * 4 );

		dot = GatherSampleLight( dl, info.m_FaceNum, info.m_Point, 
			info.m_PointNormal[0], delta, &falloff, info.m_iThread,
			info.m_PointNormal + 1, bump_dots , skyIrradiance, skyIrradiance2 );

		// NOTE: Notice here that if the light is on the back side of the face
		// (tested by checking the dot product of the face normal and the light position)
		// we don't want it to contribute to *any* of the bumped lightmaps. It glows
		// in disturbing ways if we don't do this.
		if (dot <= 0)
		{
			continue;
		}

		// Figure out the lightstyle for this particular sample 
		int lightStyleIndex = FindOrAllocateLightstyleSamples( info.m_pFace, info.m_pFaceLight, dl->light.style, info.m_NormalCount );

		if (lightStyleIndex < 0)
		{
			if (info.m_WarnFace != info.m_FaceNum)
			{
				Warning ("\nWARNING: Too many light styles on a face (%.0f,%.0f,%.0f)\n", info.m_Point[0], info.m_Point[1], info.m_Point[2] );
				info.m_WarnFace = info.m_FaceNum;
			}
			continue;
		}

		// pLightmaps is an array of the lightmaps for each normal direction,
		// here's where the result of the sample gathering goes
		Vector** pLightmaps = info.m_pFaceLight->light[lightStyleIndex];

		// Compute the contributions to each of the bumped lightmaps
		// The first sample is for non-bumped lighting.
		// The other sample are for bumpmapping.
		if( dl->light.type == emit_hdrEnvmapSkylight )
		{
			if( dl->light.style == 0 )
			{
				VectorAdd( pLightmaps[0][sampleIdx], skyIrradiance[0], pLightmaps[0][sampleIdx] );
			}
			else if( dl->light.style == 1 )	// style 1 ÀÎ °æ¿ì, ( sun + sky ) - sky 
			{
				// sun = (sun + sky) - sky
				VectorSubtract( skyIrradiance[0], skyIrradiance2[0], skyIrradiance2[0]);
				VectorAdd( pLightmaps[0][sampleIdx], skyIrradiance2[0], pLightmaps[0][sampleIdx] );
			}
		}
		else
		{
			VectorMA( pLightmaps[0][sampleIdx], falloff * dot, dl->light.intensity, pLightmaps[0][sampleIdx] );
		}

		Assert( pLightmaps[0][sampleIdx].x >= 0 && pLightmaps[0][sampleIdx].y >= 0 && pLightmaps[0][sampleIdx].z >= 0 );
		Assert( pLightmaps[0][sampleIdx].x < 1e10 && pLightmaps[0][sampleIdx].y < 1e10 && pLightmaps[0][sampleIdx].z < 1e10 );

		for( int n = 1; n < info.m_NormalCount; ++n)
		{
			if( dl->light.type == emit_hdrEnvmapSkylight )
			{
				if( dl->light.style == 0 )
				{
					VectorAdd( pLightmaps[n][sampleIdx], skyIrradiance[n], pLightmaps[n][sampleIdx] );
				}
				else
				{
					// sun = ( sun + sky ) - sky
					VectorSubtract( skyIrradiance[n], skyIrradiance2[n], skyIrradiance2[n]);
					VectorAdd( pLightmaps[n][sampleIdx], skyIrradiance2[n], pLightmaps[n][sampleIdx] );
				}
			}
			else
			{
				float bump_dot = dl->light.type == emit_skyambient ? bump_dots[n-1] : DotProduct( info.m_PointNormal[n], delta );
				if ( bump_dot > 0)
				{
					VectorMA( pLightmaps[n][sampleIdx], falloff * bump_dot, dl->light.intensity, pLightmaps[n][sampleIdx] );
				}			
			}
		}

		//!{ 2006-07-03	Çã Ã¢ ¹Î
		const bool bAddLightToStyle0 = ( dl->light.style == 1 );
		if( bAddLightToStyle0 )
		{
			int lightStyle0Index = FindOrAllocateLightstyleSamples( info.m_pFace, info.m_pFaceLight, 0, info.m_NormalCount );

			if (lightStyle0Index < 0)
			{
				if (info.m_WarnFace != info.m_FaceNum)
				{
					Warning ("\nWARNING: Too many light styles on a face (%.0f,%.0f,%.0f)\n", info.m_Point[0], info.m_Point[1], info.m_Point[2] );
					info.m_WarnFace = info.m_FaceNum;
				}
				continue;
			}

			// pLightmaps is an array of the lightmaps for each normal direction,
			// here's where the result of the sample gathering goes
			Vector** pLightmaps = info.m_pFaceLight->light[lightStyle0Index];

			// Compute the contributions to each of the bumped lightmaps
			// The first sample is for non-bumped lighting.
			// The other sample are for bumpmapping.
			if( dl->light.type == emit_hdrEnvmapSkylight )
			{
				VectorAdd( pLightmaps[0][sampleIdx], skyIrradiance[0], pLightmaps[0][sampleIdx] );
			}
			else
			{
				VectorMA( pLightmaps[0][sampleIdx], falloff * dot, dl->light.intensity, pLightmaps[0][sampleIdx] );
			}

			Assert( pLightmaps[0][sampleIdx].x >= 0 && pLightmaps[0][sampleIdx].y >= 0 && pLightmaps[0][sampleIdx].z >= 0 );
			Assert( pLightmaps[0][sampleIdx].x < 1e10 && pLightmaps[0][sampleIdx].y < 1e10 && pLightmaps[0][sampleIdx].z < 1e10 );

			for( int n = 1; n < info.m_NormalCount; ++n)
			{
				if( dl->light.type == emit_hdrEnvmapSkylight )
				{
					VectorAdd( pLightmaps[n][sampleIdx], skyIrradiance[n], pLightmaps[n][sampleIdx] );
				}
				else
				{
					float bump_dot = dl->light.type == emit_skyambient ? bump_dots[n-1] : DotProduct( info.m_PointNormal[n], delta );
					if ( bump_dot > 0)
					{
						VectorMA( pLightmaps[n][sampleIdx], falloff * bump_dot, dl->light.intensity, pLightmaps[n][sampleIdx] );
					}			
				}
			}
		}
		//!} 2006-07-03	Çã Ã¢ ¹Î
	}	
}



//-----------------------------------------------------------------------------
// Iterates over all lights and computes lighting at a sample point
//-----------------------------------------------------------------------------
static void ResampleLightAtPoint( SampleInfo_t& info, int lightStyleIndex, int flags, Vector* pLightmap )
{
	Vector delta;
	float falloff, dot;

	// Iterate over all direct lights and add them to the particular sample
	for (directlight_t *dl = activelights; dl != NULL; dl = dl->next)
	{
		if ((flags & AMBIENT_ONLY) && (dl->light.type != emit_skyambient))
			continue;

		if ((flags & NON_AMBIENT_ONLY) && (dl->light.type == emit_skyambient))
			continue;

		
		// Only add contributions that match the lightstyle 
		Assert( lightStyleIndex <= MAXLIGHTMAPS );
		Assert( info.m_pFace->styles[lightStyleIndex] != 255 );

		const bool bSampleStyle1 = ( info.m_pFace->styles[lightStyleIndex] == 0 );

		if (dl->light.style != info.m_pFace->styles[lightStyleIndex])
		{
			// style ÀÌ 0¹øÀÏ¶§, 1¹øµµ pass ½ÃÅ°ÀÚ.
			if( !( bSampleStyle1 && dl->light.style == 1 ) )
				continue;
		}
		
		float bump_dots[3];
		Vector skyIrradiance[4];
		Vector skyIrradiance2[4];

		memset( skyIrradiance, 0, sizeof( Vector ) * 4 );
		memset( skyIrradiance2, 0, sizeof( Vector ) * 4 );

		dot = GatherSampleLight( dl, info.m_FaceNum, info.m_Point, 
			info.m_PointNormal[0], delta, &falloff, info.m_iThread,
			info.m_PointNormal + 1, bump_dots, skyIrradiance, skyIrradiance2 );

		// NOTE: Notice here that if the light is on the back side of the face
		// (tested by checking the dot product of the face normal and the light position)
		// we don't want it to contribute to *any* of the bumped lightmaps. It glows
		// in disturbing ways if we don't do this.
		if (dot <= 0)
			continue;

		// Compute the contributions to each of the bumped lightmaps
		// The first sample is for non-bumped lighting.
		// The other sample are for bumpmapping.
		if( dl->light.type == emit_hdrEnvmapSkylight )
		{
			if( info.m_pFace->styles[lightStyleIndex] == 1 )
			{
				// style == 1 ; (sun + sky) - sky
				VectorSubtract( skyIrradiance[0], skyIrradiance2[0], skyIrradiance2[0]);
				VectorAdd( pLightmap[0], skyIrradiance2[0], pLightmap[0]);
			}
			else
			{
				VectorAdd( pLightmap[0], skyIrradiance[0], pLightmap[0] );
			}
		}
		else
		{
			VectorMA( pLightmap[0], falloff * dot, dl->light.intensity, pLightmap[0] );
		}

		// bump light
		for( int n = 1; n < info.m_NormalCount; ++n)
		{
			if( dl->light.type == emit_hdrEnvmapSkylight )
			{
				if( info.m_pFace->styles[lightStyleIndex] == 1 )
				{
					// style == 1 ; (sun + sky) - sky
					VectorSubtract( skyIrradiance[n], skyIrradiance2[n], skyIrradiance2[n]);
					VectorAdd( pLightmap[n], skyIrradiance2[n], pLightmap[n]);
				}
				else
				{
					VectorAdd( pLightmap[n], skyIrradiance[n], pLightmap[n] );
				}
			}
			else
			{
				float bump_dot = dl->light.type == emit_skyambient ? bump_dots[n-1] : DotProduct( info.m_PointNormal[n], delta );
				if (bump_dot > 0)
				{
					VectorMA( pLightmap[n], falloff * bump_dot, dl->light.intensity, pLightmap[n] );
				}		
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Perform supersampling at a particular point
//-----------------------------------------------------------------------------
static int SupersampleLightAtPoint( lightinfo_t& l, SampleInfo_t& info, 
	int sampleIndex, int lightStyleIndex, Vector *pDirectLight )
{
	Vector superSamplePosition;
	Vector2D sampleLightOrigin;
	Vector2D superSampleLightCoord;

	// Check out the sample we're currently dealing with...
	sample_t& sample = info.m_pFaceLight->sample[sampleIndex];

	// Get the position of the original sample in lightmapspace
	WorldToLuxelSpace( &l, sample.pos, sampleLightOrigin );
 	// Msg("coord %f %f\n", coord[0], coord[1] );	

	// Some parameters related to supersampling
	int subsample = 4;	// FIXME: make a parameter
	float cscale = 1.0 / subsample;
	float csshift = -((subsample - 1) * cscale) / 2.0;

	// Clear out the direct light values
	for (int i = 0; i < info.m_NormalCount; ++i )
		pDirectLight[i].Init( 0, 0, 0 );

	int subsampleCount = 0;
	for (int s = 0; s < subsample; ++s)
	{
		for (int t = 0; t < subsample; ++t)
		{
			// make sure the coordinate is inside of the sample's winding and when normalizing
			// below use the number of samples used, not just numsamples and some of them
			// will be skipped if they are not inside of the winding
			superSampleLightCoord[0] = sampleLightOrigin[0] + s * cscale + csshift;
			superSampleLightCoord[1] = sampleLightOrigin[1] + t * cscale + csshift;
			// Msg("subsample %f %f\n", superSampleLightCoord[0], superSampleLightCoord[1] );

			// Figure out where the supersample exists in the world, and make sure
			// it lies within the sample winding
			LuxelSpaceToWorld( &l, superSampleLightCoord[0], superSampleLightCoord[1], superSamplePosition );

			// A winding should exist only if the sample wasn't a uniform luxel, or if dumppatches is true.
			if ( sample.w )
			{
				if( !PointInWinding( superSamplePosition, sample.w ) )
					continue;
			}

			// Compute the super-sample illumination point and normal
			// We're assuming the flat normal is the same for all supersamples
			ComputeIlluminationPointAndNormals( l, superSamplePosition, sample.normal, &info );

			// Resample the non-ambient light at this point...
			ResampleLightAtPoint( info, lightStyleIndex, NON_AMBIENT_ONLY, pDirectLight );

			// Got another subsample
			++subsampleCount;
		}
	}

	return subsampleCount;
}





//-----------------------------------------------------------------------------
// Compute gradients of a lightmap
//-----------------------------------------------------------------------------
static void ComputeLightmapGradients( SampleInfo_t& info, bool const* pHasProcessedSample , float* pIntensity, float* gradient )
{
	int w = info.m_LightmapWidth;
	int h = info.m_LightmapHeight;
	facelight_t* fl = info.m_pFaceLight;

	for (int i=0 ; i<fl->numsamples ; i++)
	{
		// Don't supersample the same sample twice
		if (pHasProcessedSample[i])
			continue;

		gradient[i] = 0.0f;
		sample_t& sample = fl->sample[i];

		// Choose the maximum gradient of all bumped lightmap intensities
		for ( int n = 0; n < info.m_NormalCount; ++n )
		{
			int j = n * info.m_LightmapSize + sample.s + sample.t * w;

			if (sample.t > 0)
			{
				if (sample.s > 0)   gradient[i] = max( gradient[i], fabs( pIntensity[j] - pIntensity[j-1-w] ) );
									gradient[i] = max( gradient[i], fabs( pIntensity[j] - pIntensity[j-w] ) );
				if (sample.s < w-1) gradient[i] = max( gradient[i], fabs( pIntensity[j] - pIntensity[j+1-w] ) );
			}
			if (sample.t < h-1)
			{
				if (sample.s > 0)   gradient[i] = max( gradient[i], fabs( pIntensity[j] - pIntensity[j-1+w] ) );
									gradient[i] = max( gradient[i], fabs( pIntensity[j] - pIntensity[j+w] ) );
				if (sample.s < w-1) gradient[i] = max( gradient[i], fabs( pIntensity[j] - pIntensity[j+1+w] ) );
			}
			if (sample.s > 0)   gradient[i] = max( gradient[i], fabs( pIntensity[j] - pIntensity[j-1] ) );
			if (sample.s < w-1) gradient[i] = max( gradient[i], fabs( pIntensity[j] - pIntensity[j+1] ) );
		}
	}
}



//-----------------------------------------------------------------------------
// ComputeLuxelIntensity...
//-----------------------------------------------------------------------------
static inline void ComputeLuxelIntensity( SampleInfo_t& info, int sampleIdx, 
							Vector **ppLightSamples, float* pSampleIntensity )
{
	// Compute a separate intensity for each
	sample_t& sample = info.m_pFaceLight->sample[sampleIdx];
	int destIdx = sample.s + sample.t * info.m_LightmapWidth;
	for (int n = 0; n < info.m_NormalCount; ++n)
	{
		float intensity = ppLightSamples[n][sampleIdx][0] + ppLightSamples[n][sampleIdx][1] + ppLightSamples[n][sampleIdx][2];

		// convert to a linear perception space
		pSampleIntensity[n * info.m_LightmapSize + destIdx] = pow( intensity / 256.0, 1.0 / 2.2 );
	}
}

//-----------------------------------------------------------------------------
// Compute the maximum intensity based on all bumped lighting
//-----------------------------------------------------------------------------
static void ComputeSampleIntensities( SampleInfo_t& info, Vector **ppLightSamples, float* pSampleIntensity )
{
	for (int i=0; i<info.m_pFaceLight->numsamples; i++)
	{
		ComputeLuxelIntensity( info, i, ppLightSamples, pSampleIntensity );
	}
}


//-----------------------------------------------------------------------------
// Perform supersampling on a particular lightstyle
//-----------------------------------------------------------------------------
static void BuildSupersampleFaceLights( lightinfo_t& l, SampleInfo_t& info, int lightstyleIndex )
{
	Vector pAmbientLight[NUM_BUMP_VECTS+1];
	Vector pDirectLight[NUM_BUMP_VECTS+1];


	// This is used to make sure we don't supersample a light sample more than once
	int processedSampleSize = info.m_LightmapSize * sizeof(bool);
	bool* pHasProcessedSample = (bool*)( tls_realloc( tls_hasProcessedSamples, tlsz_hasProcessedSamples, processedSampleSize ) );

	memset( pHasProcessedSample, 0, processedSampleSize );

	// This is used to compute a simple gradient computation of the light samples
	// We're going to store the maximum intensity of all bumped samples at each sample location
	float* pGradient = (float*)(tls_realloc( tls_gradient, tlsz_gradient, info.m_pFaceLight->numsamples * sizeof(float) ) );
	float* pSampleIntensity = (float*)(tls_realloc( tls_sampleIntensity, tlsz_sampleIntensity, info.m_NormalCount * info.m_LightmapSize * sizeof(float) ));

	// Compute the maximum intensity of all lighting associated with this lightstyle
	// for all bumped lighting
	Vector **ppLightSamples = info.m_pFaceLight->light[lightstyleIndex];
	ComputeSampleIntensities( info, ppLightSamples, pSampleIntensity );

	Vector *pVisualizePass = NULL;
	if (debug_extra)
	{
		int visualizationSize = info.m_pFaceLight->numsamples * sizeof(Vector);
		pVisualizePass = (Vector*)stackalloc( visualizationSize );
		memset( pVisualizePass, 0, visualizationSize ); 
	}

	// What's going on here is that we're looking for large lighting discontinuities
	// (large light intensity gradients) as a clue that we should probably be supersampling
	// in that area. Because the supersampling operation will cause lighting changes,
	// we've found that it's good to re-check the gradients again and see if any other
	// areas should be supersampled as a result of the previous pass. Keep going
	// until all the gradients are reasonable or until we hit a max number of passes
	bool do_anotherpass = true;
	int pass = 1;
	while (do_anotherpass && pass <= extrapasses)
	{
		// Look for lighting discontinuities to see what we should be supersampling
		ComputeLightmapGradients( info, pHasProcessedSample, pSampleIntensity, pGradient );

		do_anotherpass = false;

		// Now check all of the samples and supersample those which we have
		// marked as having high gradients
		for (int i=0 ; i<info.m_pFaceLight->numsamples; ++i)
		{
			// Don't supersample the same sample twice
			if (pHasProcessedSample[i])
				continue;

			// Don't supersample if the lighting is pretty uniform near the sample
			if (pGradient[i] < 0.0625)
				continue;

			// Joy! We're supersampling now, and we therefore must do another pass
			// Also, we need never bother with this sample again
			pHasProcessedSample[i] = true;
			do_anotherpass = true;

			if (debug_extra)
			{
				// Mark the little visualization bitmap with a color indicating
				// which pass it was updated on.
				pVisualizePass[i][0] = (pass & 1) * 255;
				pVisualizePass[i][1] = (pass & 2) * 128;
				pVisualizePass[i][2] = (pass & 4) * 64;
			}

			// Figure out the position + normal direction of the sample under consideration
			sample_t& sample = info.m_pFaceLight->sample[i];
			ComputeIlluminationPointAndNormals( l, sample.pos, sample.normal, &info );

			// Compute the ambient light for each bump direction vector
			for (int j = 0; j < info.m_NormalCount; ++j )
				pAmbientLight[j].Init( 0, 0, 0 );

			ResampleLightAtPoint( info, lightstyleIndex, AMBIENT_ONLY, pAmbientLight );

			// Supersample the non-ambient light for each bump direction vector
			int supersampleCount = SupersampleLightAtPoint( l, info, i, lightstyleIndex, pDirectLight );

			// Because of sampling problems, small area triangles may have no samples.
			// In this case, just use what we already have
			if (supersampleCount > 0)
			{
				// Add the ambient + directional terms togather, stick it back into the lightmap
				for (int n = 0; n < info.m_NormalCount; ++n)
				{
					VectorDivide( pDirectLight[n], supersampleCount, ppLightSamples[n][i] );
					ppLightSamples[n][i] += pAmbientLight[n];
				}

				// Recompute the luxel intensity based on the supersampling
				ComputeLuxelIntensity( info, i, ppLightSamples, pSampleIntensity );
			}

		}

		// We've finished another pass
		pass++;
	}

	if (debug_extra)
	{
		// Copy colors representing which supersample pass the sample was messed with
		// into the actual lighting values so we can visualize it
		for (int i=0 ; i<info.m_pFaceLight->numsamples ; ++i)
		{
			for (int j = 0; j <info.m_NormalCount; ++j)
			{
				VectorCopy( pVisualizePass[i], ppLightSamples[j][i] ); 
			}
		}
	}
}

void InitLightinfo( lightinfo_t *pl, int facenum )
{
	dface_t		*f;

	f = &dfaces[facenum];

	memset (pl, 0, sizeof(*pl));
	pl->facenum = facenum;
    pl->face = f;
	// plane
	VectorCopy (dplanes[f->planenum].normal, pl->facenormal);
	pl->facedist = dplanes[f->planenum].dist;
	// offset
	VectorCopy (face_offset[facenum], pl->modelorg);

	CalcFaceVectors( pl );

	// figure out if the surface is flat
	pl->isflat = true;
	if (smoothing_threshold != 1)
	{
		faceneighbor_t *fn = &faceneighbor[facenum];
		for (int j=0 ; j<f->numedges ; j++)
		{
			float dot = DotProduct( pl->facenormal, fn->normal[j] );
			if (dot < 1.0 - EQUAL_EPSILON)
			{
				pl->isflat = false;
				break;
			}
		}
	}
}

static void InitSampleInfo( lightinfo_t const& l, int iThread, SampleInfo_t& info )
{
	info.m_LightmapWidth  = l.face->m_LightmapTextureSizeInLuxels[0]+1;
	info.m_LightmapHeight = l.face->m_LightmapTextureSizeInLuxels[1]+1;
	info.m_LightmapSize = info.m_LightmapWidth * info.m_LightmapHeight;

	// How many lightmaps are we going to need?
	info.m_pTexInfo = &texinfo[l.face->texinfo];
	info.m_NormalCount = (info.m_pTexInfo->flags & SURF_BUMPLIGHT) ? NUM_BUMP_VECTS + 1 : 1;
	info.m_FaceNum = l.facenum;
	info.m_pFace = l.face;
	info.m_pFaceLight = &facelight[info.m_FaceNum];
	info.m_IsDispFace = ValidDispFace( info.m_pFace );
	info.m_iThread = iThread;
	info.m_WarnFace = -1;

	// initialize normals if the surface is flat
	if (l.isflat)
	{
		VectorCopy( l.facenormal, info.m_PointNormal[0] );

		// use facenormal along with the smooth normal to build the three bump map vectors
		if( info.m_NormalCount > 1 )
		{
			GetBumpNormals( info.m_pTexInfo->textureVecsTexelsPerWorldUnits[0], 
				info.m_pTexInfo->textureVecsTexelsPerWorldUnits[1], l.facenormal, 
				l.facenormal, &info.m_PointNormal[1] );
		}
	}
}

// ¸ðµç face¿Í facelight¸¦ ÃÊ±âÈ­ÇÑ´Ù.
void AVA_InitializeFacesAndFacelights()
{
	// alloc data
	if(!tls_sampleData)
	{
		tls_sampleData = (char*)malloc( sizeof(sample_t)*SINGLEMAP );
	}

	for( int facenum = 0; facenum < numfaces; ++facenum )
	{
		if( g_bInterrupt )
		{
			return;
		}

		dface_t *face = &dfaces[facenum];
		face->lightofs = -1;
		for( int j = 0; j < MAXLIGHTMAPS; ++j )
		{
			face->styles[j] = 255;
		}
		if( texinfo[face->texinfo].flags & TEX_SPECIAL )
		{
			continue;
		}

		facelight_t *fl = &facelight[facenum];
		fl = &facelight[facenum];
		memset( fl->light, 0, sizeof( fl->light ) );

		lightinfo_t lightinfo;
		SampleInfo_t sampleInfo;
		InitLightinfo( &lightinfo, facenum );
		CalcPoints( &lightinfo, fl, facenum );
		InitSampleInfo( lightinfo, 0, sampleInfo );

		// always allocate style 0 lightmap
		face->styles[0] = 0;
		AllocateLightstyleSamples( fl, 0, sampleInfo.m_NormalCount );

		// always allocate style 1 lightmap ( indirect light only light )
		face->styles[1] = 1;
		AllocateLightstyleSamples( fl, 1, sampleInfo.m_NormalCount );
	}

	// free data
	{
		free( tls_sampleData ); tls_sampleData = NULL;
	}
}

void AVA_BuildFacelights( int threadnum, int facenum )
{
	if( threadnum < 0 || facenum < 0 )
	{
		return;
	}
	if( g_bInterrupt )
	{
		return;
	}
	dface_t		*face = &dfaces[facenum];
	facelight_t	*fl = &facelight[facenum];
	if( texinfo[face->texinfo].flags & TEX_SPECIAL )
	{
		return;
	}

	lightinfo_t		lightInfo;
	SampleInfo_t	sampleInfo;
	InitLightinfo( &lightInfo, facenum );
	InitSampleInfo( lightInfo, 0, sampleInfo );

	//<@ 2007. 11. 13
	// compute face sun visibility
	face->bSunVisibility = 0;
	gSunVisible = false;
	//>@

	// sample the lights at each sample location
	const int sampleCount = fl->numsamples;
	for( int i = 0; i < fl->numsamples; ++i )
	{
		// Figure out the position + normal direction of the sample under consideration
		sample_t &sample = fl->sample[i];
		ComputeIlluminationPointAndNormals( lightInfo, sample.pos, sample.normal, &sampleInfo );
		// Fix up the sample normal in the case of smooth faces
		if( !lightInfo.isflat )
		{
			sample.normal = sampleInfo.m_PointNormal[0];
		}
		GatherSampleLightAtPoint( sampleInfo, i );
	}

	if( do_extra && !sampleInfo.m_IsDispFace )
	{
		for( int i = 0; i < MAXLIGHTMAPS; ++i )
		{
			if( face->styles[i] == 255 )
			{
				break;
			}
			BuildSupersampleFaceLights( lightInfo, sampleInfo, i );
		}
	}

	//<@ 2007. 11. 13
	// compute face sun visibility
	if( gSunVisible)
	{
		face->bSunVisibility = 1;
	}
	//>@
}

// 2007. 5. 2

//<@ Secondary Light Source : 2007. 8. 16 changmin
void AVA_BuildSecondaryFacelights( int threadnum, int facenum )
{
	if( threadnum < 0 || facenum < 0 )
	{
		return;
	}
	if( g_bInterrupt )
	{
		return;
	}
	dface_t		*face = &dfaces[facenum];
	facelight_t	*fl = &facelight[facenum];
	if( texinfo[face->texinfo].flags & TEX_SPECIAL )
	{
		return;
	}
	if( !(texinfo[face->texinfo].flags & SURF_SECONDARYLIGHTSOURCE) )
	{
		return;
	}

	lightinfo_t		lightInfo;
	SampleInfo_t	sampleInfo;
	InitLightinfo( &lightInfo, facenum );
	InitSampleInfo( lightInfo, 0, sampleInfo );

	// sample the lights at each sample location
	const int sampleCount = fl->numsamples;
	for( int i = 0; i < fl->numsamples; ++i )
	{
		// Figure out the position + normal direction of the sample under consideration
		sample_t &sample = fl->sample[i];
		ComputeIlluminationPointAndNormals( lightInfo, sample.pos, sample.normal, &sampleInfo );
		// Fix up the sample normal in the case of smooth faces
		if( !lightInfo.isflat )
		{
			sample.normal = sampleInfo.m_PointNormal[0];
		}
		GatherSampleLightAtPoint( sampleInfo, i );
	}

	if( do_extra && !sampleInfo.m_IsDispFace )
	{
		for( int i = 0; i < MAXLIGHTMAPS; ++i )
		{
			if( face->styles[i] == 255 )
			{
				break;
			}
			BuildSupersampleFaceLights( lightInfo, sampleInfo, i );
		}
	}
}
//>@ Secondary Light Source


#define INV_PI2		0.31830988618379067154f

static void GatherDiffuseReflectionAtPoint( const lightinfo_t& lightInfo, SampleInfo_t &info, int sampleIndex )
{
	const Vector	&pos	= info.m_Point;
	const NVector	&normal	= info.m_PointNormal[0];
	const NVector	*world_bump_basis	= info.m_PointNormal + 1;

	Pbrt::Sample_t *usingSamples = NULL;
	while (normal.index >= GPbrtCachedSamples.size())
	{
		GPbrtCachedSamples.insert( GPbrtCachedSamples.end(), 1024, NULL );
	}	
	usingSamples = GPbrtCachedSamples[ normal.index ];
	if (usingSamples == NULL)
	{
		Pbrt::Sample_t* samples = usingSamples = GPbrtCachedSamples[ normal.index ] = (Pbrt::Sample_t*)malloc( sizeof(Pbrt::Sample_t) * Pbrt::MAX_SAMPLES );
		Pbrt::CosineWeightedHemiSphereSample( normal, Pbrt::MAX_SAMPLES, samples );
	}

	Vector ambient_color[4] =
	{
		Vector( 0.0f, 0.0f, 0.0f ),
		Vector( 0.0f, 0.0f, 0.0f ),
		Vector( 0.0f, 0.0f, 0.0f ),
		Vector( 0.0f, 0.0f, 0.0f )
	};

	Vector wi;
	float pdf;
	const int sampleCount = 128;
	for( int i = 0; i < sampleCount; ++i )
	{
		wi = usingSamples[i].wi;
		pdf = usingSamples[i].pdf;

		Vector sample_pos;
		Vector upend;
		//VectorAdd( pos, wi, sample_pos );
		VectorMA( pos, 0.1f, wi, sample_pos );
		VectorMA( sample_pos, MAX_TRACE_LENGTH, wi, upend );

		float flDot[4];
		flDot[0] = DotProduct( normal, wi );

		if( flDot[0] < 0.0f )
		{
			continue;
		}

		flDot[1] = DotProduct( world_bump_basis[0], wi );
		flDot[2] = DotProduct( world_bump_basis[1], wi );
		flDot[3] = DotProduct( world_bump_basis[2], wi );

		Vector sample_color;
		bool is_hit = SampleDiffuseReflectionWithOpcode( sample_pos, upend, &sample_color );

		if( is_hit )
		{
			sample_color.x = max( 0.0f, sample_color.x * INV_PI2);
			sample_color.y = max( 0.0f, sample_color.y * INV_PI2);
			sample_color.z = max( 0.0f, sample_color.z * INV_PI2);
			for( int ibump = 0; ibump < NUM_BUMP_VECTS + 1 ; ++ibump )
			{
				if( flDot[ibump] > 0.0f )
				{
					VectorMA( ambient_color[ibump], flDot[ibump] / pdf, sample_color, ambient_color[ibump] );
				}
			}
		}
	}

	for( int ibump = 0; ibump < NUM_BUMP_VECTS + 1; ++ibump )
	{
		VectorScale( ambient_color[ibump], 1.0f / (float)sampleCount, ambient_color[ibump] );
	}

	// pLightmaps is an array of the lightmaps for each normal direction,
	// here's where the result of the sample gathering goes
	Vector** pLightmaps = info.m_pFaceLight->light[0];	// 0 = lightstyleindex

	// copy ambient
	VectorCopy( ambient_color[0], pLightmaps[0][sampleIndex] );
	for( int n = 1; n < info.m_NormalCount; ++n )
	{
		VectorCopy( ambient_color[n], pLightmaps[n][sampleIndex] );
	}
}

static void ResampleDiffuseReflectionAtPoint( const lightinfo_t& lightInfo, SampleInfo_t& info, int lightStyleIndex, Vector* pLightmap )
{
	const Vector	&pos	= info.m_Point;
	const NVector	&normal	= info.m_PointNormal[0];
	const NVector	*world_bump_basis	= info.m_PointNormal + 1;

	Pbrt::Sample_t *usingSamples = NULL;
	while (normal.index >= GPbrtCachedSamples.size())
	{
		GPbrtCachedSamples.insert( GPbrtCachedSamples.end(), 1024, NULL );
	}	
	usingSamples = GPbrtCachedSamples[ normal.index ];
	if (usingSamples == NULL)
	{
		Pbrt::Sample_t* samples = usingSamples = GPbrtCachedSamples[ normal.index ] = (Pbrt::Sample_t*)malloc( sizeof(Pbrt::Sample_t) * Pbrt::MAX_SAMPLES );
		Pbrt::CosineWeightedHemiSphereSample( normal, Pbrt::MAX_SAMPLES, samples );
	}

	Vector ambient_color[4] =
	{
		Vector( 0.0f, 0.0f, 0.0f ),
		Vector( 0.0f, 0.0f, 0.0f ),
		Vector( 0.0f, 0.0f, 0.0f ),
		Vector( 0.0f, 0.0f, 0.0f )
	};

	Vector wi;
	float pdf;
	const int SAMPLE_COUNT = 128;
	for( int i = 0; i < SAMPLE_COUNT; ++i )
	{
		wi = usingSamples[i].wi;
		pdf = usingSamples[i].pdf;

		Vector sample_pos;
		Vector upend;
		//VectorAdd( pos, wi, sample_pos );
		VectorMA( pos, 0.1f, wi, sample_pos );
		VectorMA( sample_pos, MAX_TRACE_LENGTH, wi, upend );

		float flDot[4];
		flDot[0] = DotProduct( normal, wi );

		if( flDot[0] < 0.0f )
		{
			continue;
		}

		flDot[1] = DotProduct( world_bump_basis[0], wi );
		flDot[2] = DotProduct( world_bump_basis[1], wi );
		flDot[3] = DotProduct( world_bump_basis[2], wi );

		Vector sample_color;
		bool is_hit = SampleDiffuseReflectionWithOpcode( sample_pos, upend, &sample_color );

		if( is_hit )
		{
			sample_color.x = max( 0.0f, sample_color.x * INV_PI2);
			sample_color.y = max( 0.0f, sample_color.y * INV_PI2);
			sample_color.z = max( 0.0f, sample_color.z * INV_PI2);
			for( int ibump = 0; ibump < NUM_BUMP_VECTS + 1 ; ++ibump )
			{
				if( flDot[ibump] > 0.0f )
				{
					VectorMA( ambient_color[ibump], flDot[ibump] / pdf, sample_color, ambient_color[ibump] );
				}
			}
		}
	}

	for( int ibump = 0; ibump < NUM_BUMP_VECTS + 1; ++ibump )
	{
		VectorScale( ambient_color[ibump], 1.0f / (float)SAMPLE_COUNT, ambient_color[ibump] );
	}

	// copy ambient
	VectorAdd( ambient_color[0], pLightmap[0], pLightmap[0] );
	for( int n = 1; n < info.m_NormalCount; ++n )
	{
		VectorAdd( ambient_color[n], pLightmap[n], pLightmap[n] );
	}
}

//-----------------------------------------------------------------------------
// Perform supersampling at a particular point
//-----------------------------------------------------------------------------
static int SupersampleDiffuseReflectionAtPoint( lightinfo_t& l, SampleInfo_t& info, 
								   int sampleIndex, int lightStyleIndex, Vector *pDirectLight )
{
	Vector superSamplePosition;
	Vector2D sampleLightOrigin;
	Vector2D superSampleLightCoord;

	// Check out the sample we're currently dealing with...
	sample_t& sample = info.m_pFaceLight->sample[sampleIndex];

	// Get the position of the original sample in lightmapspace
	WorldToLuxelSpace( &l, sample.pos, sampleLightOrigin );

	// Some parameters related to supersampling
	int subsample = 4;	// FIXME: make a parameter
	float cscale = 1.0 / subsample;
	float csshift = -((subsample - 1) * cscale) / 2.0;

	// Clear out the direct light values
	for (int i = 0; i < info.m_NormalCount; ++i )
	{
		pDirectLight[i].Init( 0, 0, 0 );
	}

	int subsampleCount = 0;
	for (int s = 0; s < subsample; ++s)
	{
		for (int t = 0; t < subsample; ++t)
		{
			// make sure the coordinate is inside of the sample's winding and when normalizing
			// below use the number of samples used, not just numsamples and some of them
			// will be skipped if they are not inside of the winding
			superSampleLightCoord[0] = sampleLightOrigin[0] + s * cscale + csshift;
			superSampleLightCoord[1] = sampleLightOrigin[1] + t * cscale + csshift;

			// Figure out where the supersample exists in the world, and make sure
			// it lies within the sample winding
			LuxelSpaceToWorld( &l, superSampleLightCoord[0], superSampleLightCoord[1], superSamplePosition );

			// A winding should exist only if the sample wasn't a uniform luxel, or if dumppatches is true.
			if ( sample.w )
			{
				if( !PointInWinding( superSamplePosition, sample.w ) )
				{
					continue;
				}
			}

			// Compute the super-sample illumination point and normal
			// We're assuming the flat normal is the same for all supersamples
			ComputeIlluminationPointAndNormals( l, superSamplePosition, sample.normal, &info );

			// Resample the non-ambient light at this point...
			ResampleDiffuseReflectionAtPoint( l, info, lightStyleIndex, pDirectLight );

			// Got another subsample
			++subsampleCount;
		}
	}

	return subsampleCount;
}


//-----------------------------------------------------------------------------
// Perform supersampling on a particular lightstyle
//-----------------------------------------------------------------------------
static void BuildSupersampleFaceAmbientlights( lightinfo_t& l, SampleInfo_t& info, int lightstyleIndex )
{
	Vector pDirectLight[NUM_BUMP_VECTS+1];

	// This is used to make sure we don't supersample a light sample more than once
	int processedSampleSize = info.m_LightmapSize * sizeof(bool);
	bool* pHasProcessedSample = (bool*)( tls_realloc( tls_hasProcessedSamples, tlsz_hasProcessedSamples, processedSampleSize ) );

	memset( pHasProcessedSample, 0, processedSampleSize );

	// This is used to compute a simple gradient computation of the light samples
	// We're going to store the maximum intensity of all bumped samples at each sample location
	float* pGradient = (float*)(tls_realloc( tls_gradient, tlsz_gradient, info.m_pFaceLight->numsamples * sizeof(float) ) );
	float* pSampleIntensity = (float*)(tls_realloc( tls_sampleIntensity, tlsz_sampleIntensity, info.m_NormalCount * info.m_LightmapSize * sizeof(float) ));

	// Compute the maximum intensity of all lighting associated with this lightstyle
	// for all bumped lighting
	Vector **ppLightSamples = info.m_pFaceLight->light[lightstyleIndex];
	ComputeSampleIntensities( info, ppLightSamples, pSampleIntensity );

	Vector *pVisualizePass = NULL;

	// What's going on here is that we're looking for large lighting discontinuities
	// (large light intensity gradients) as a clue that we should probably be supersampling
	// in that area. Because the supersampling operation will cause lighting changes,
	// we've found that it's good to re-check the gradients again and see if any other
	// areas should be supersampled as a result of the previous pass. Keep going
	// until all the gradients are reasonable or until we hit a max number of passes
	bool do_anotherpass = true;
	int pass = 1;
	while (do_anotherpass && pass <= extrapasses)
	{
		// Look for lighting discontinuities to see what we should be supersampling
		ComputeLightmapGradients( info, pHasProcessedSample, pSampleIntensity, pGradient );

		do_anotherpass = false;

		// Now check all of the samples and supersample those which we have
		// marked as having high gradients
		for (int i=0 ; i<info.m_pFaceLight->numsamples; ++i)
		{
			// Don't supersample the same sample twice
			if (pHasProcessedSample[i])
				continue;

			// Don't supersample if the lighting is pretty uniform near the sample
			if (pGradient[i] < 0.0625)
				continue;

			// Joy! We're supersampling now, and we therefore must do another pass
			// Also, we need never bother with this sample again
			pHasProcessedSample[i] = true;
			do_anotherpass = true;

			// Figure out the position + normal direction of the sample under consideration
			sample_t& sample = info.m_pFaceLight->sample[i];
			ComputeIlluminationPointAndNormals( l, sample.pos, sample.normal, &info );

			// Supersample the non-ambient light for each bump direction vector
			int supersampleCount = SupersampleDiffuseReflectionAtPoint( l, info, i, lightstyleIndex, pDirectLight );

			// Because of sampling problems, small area triangles may have no samples.
			// In this case, just use what we already have
			if (supersampleCount > 0)
			{
				// Add the ambient + directional terms togather, stick it back into the lightmap
				for (int n = 0; n < info.m_NormalCount; ++n)
				{
					VectorDivide( pDirectLight[n], supersampleCount, ppLightSamples[n][i] );
				}

				// Recompute the luxel intensity based on the supersampling
				ComputeLuxelIntensity( info, i, ppLightSamples, pSampleIntensity );
			}
		}
		// We've finished another pass
		pass++;
	}
}


// sampling face ambient lights
void AVA_GatherFaceAmbientlights( int threadnum, int facenum )
{
	if( threadnum < 0 || facenum < 0 )
	{
		return;
	}
	if( g_bInterrupt )
	{
		return;
	}

	dface_t		*face = &dfaces[facenum];
	facelight_t	*fl = &facelight[facenum];
	if( texinfo[face->texinfo].flags & TEX_SPECIAL )
	{
		return;
	}

	lightinfo_t		lightInfo;
	SampleInfo_t	sampleInfo;
	InitLightinfo( &lightInfo, facenum );
	InitSampleInfo( lightInfo, 0, sampleInfo );

	// sample the lights at each sample location
	for( int i = 0; i < fl->numsamples; ++i )
	{
		// Figure out the position + normal direction of the sample under consideration
		sample_t &sample = fl->sample[i];
		ComputeIlluminationPointAndNormals( lightInfo, sample.pos, sample.normal, &sampleInfo );
		// Fix up the sample normal in the case of smooth faces
		if( !lightInfo.isflat )
		{
			sample.normal = sampleInfo.m_PointNormal[0];
		}
		GatherDiffuseReflectionAtPoint( lightInfo, sampleInfo, i );
	}

	// super sample
	if( do_extra && !sampleInfo.m_IsDispFace )
	{
		for( int i = 0; i < MAXLIGHTMAPS; ++i )
		{
			if( face->styles[i] == 255 )
			{
				break;
			}

			//<@ realtime light style ; 2007. 10. 23
			// ambient sampling ¾ÈÇÑ´Ù.
			if( face->styles[i] == 1 )
				continue;
			//>@

			BuildSupersampleFaceAmbientlights( lightInfo, sampleInfo, i );
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////


/*
=============
BuildFacelights
=============
*/
extern int spy_info[4][3];

void BuildFacelights (int iThread, int facenum)
{
	int	i, j;

	spy_info[iThread][0] = facenum;

	if (iThread < 0 || facenum < 0 )
	{
		free( tls_sampleData ); tls_sampleData = NULL;
		free( tls_hasProcessedSamples ); tls_hasProcessedSamples = NULL; tlsz_hasProcessedSamples = 0;
		free( tls_gradient ); tls_gradient = NULL; tlsz_gradient = 0;
		free( tls_sampleIntensity ); tls_sampleIntensity = NULL; tlsz_sampleIntensity = 0;	
		return;
	}

	if(!tls_sampleData)
	{
		tls_sampleData = (char*)malloc( sizeof(sample_t)*SINGLEMAP );
		tls_realloc( tls_hasProcessedSamples, tlsz_hasProcessedSamples, 64 * 1024 );		
		tls_realloc( tls_gradient, tlsz_gradient, 64 * 1024 );
		tls_realloc( tls_sampleIntensity, tlsz_sampleIntensity, 64 * 1024 );				
	}

	lightinfo_t	l;
	dface_t *f;
	facelight_t	*fl;
	SampleInfo_t sampleInfo;
	Vector spot;

	if( g_bInterrupt )
	{
		return;
	}

    // some surfaces don't need lightmaps
	f = &dfaces[facenum];
	f->lightofs = -1;
	for (j=0 ; j<MAXLIGHTMAPS ; j++)
	{
		f->styles[j] = 255;
	}

	// check for patches for this face.  If none it must be degenerate.  Ignore.
	if( facePatches.Element( facenum ) == facePatches.InvalidIndex() )
	{
		return;
	}

	fl = &facelight[facenum];

	if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
	{
		return;		// non-lit texture
	}

	memset( fl->light, 0, sizeof(fl->light) );	

	InitLightinfo( &l, facenum );

	CalcPoints( &l, fl, facenum );

	InitSampleInfo( l, iThread, sampleInfo );

	// always allocate style 0 lightmap
	f->styles[0] = 0;
	AllocateLightstyleSamples( fl, 0, sampleInfo.m_NormalCount );

	//<@ 2007. 11. 13
	// compute face sun visibility
	f->bSunVisibility = 0;
	gSunVisible = false;
	//>@

	// sample the lights at each sample location
	for (i=0 ; i<fl->numsamples ; i++)
	{
		// Figure out the position + normal direction of the sample under consideration
		sample_t& sample = fl->sample[i];
		ComputeIlluminationPointAndNormals( l, sample.pos, sample.normal, &sampleInfo );

		// Fix up the sample normal in the case of smooth faces...
		if (!l.isflat)
		{
			// The sample normal is now the phong normal
			sample.normal = sampleInfo.m_PointNormal[0];
		}

		// Iterate over all the lights and add their contribution to this spot
		GatherSampleLightAtPoint( sampleInfo, i );
	}

	// get rid of the -extra functionality on displacement surfaces
	if (do_extra && !sampleInfo.m_IsDispFace)
	{
		// For each lightstyle, perform a supersampling pass
		for ( i = 0; i < MAXLIGHTMAPS; ++i )
		{
			// Stop when we run out of lightstyles
			if (f->styles[i] == 255)
			{
				break;
			}

			BuildSupersampleFaceLights( l, sampleInfo, i );
		}
	}

	//<@ 2007. 11. 13
	// compute face sun visibility
	if( gSunVisible)
	{
		f->bSunVisibility = 1;
	}
	//>@

	if (!g_bUseMPI) 
	{
		//
		// This is done on the master node when MPI is used
		//
		BuildPatchLights( facenum );
	}

	if( dumppatches )
	{
		DumpSamples( facenum, fl );
	}

	FreeSampleWindings( fl );
}	

void BuildPatchLights( int facenum )
{
	int i, k;

	patch_t		*patch;

	dface_t	*f = &dfaces[facenum];
	facelight_t	*fl = &facelight[facenum];

	for( k = 0; k < MAXLIGHTMAPS; k++ )
	{
		if (f->styles[k] == 0)
			break;
	}

	if (k >= MAXLIGHTMAPS)
		return;

	// sample À» leaf patch¿¡´Ù ³Ö´Â´Ù...
	for (i = 0; i < fl->numsamples; i++)
	{
		AddSampleToPatch( &fl->sample[i], fl->light[k][0][i], facenum);
	}

	// check for a valid face
	if( facePatches.Element( facenum ) == facePatches.InvalidIndex() )
		return;

	// push up sampled light to parents (children always exist first in the list)
	patch_t *pNextPatch;
	for( patch = &patches.Element( facePatches.Element( facenum ) ); patch; patch = pNextPatch )
	{
		// next patch
		pNextPatch = NULL;
		if( patch->ndxNext != patches.InvalidIndex() )
		{
			pNextPatch = &patches.Element( patch->ndxNext );
		}

		// skip patches without parents
		if( patch->parent == patches.InvalidIndex() )
			continue;

		patch_t *parent = &patches.Element( patch->parent );

		parent->samplearea += patch->samplearea;
		VectorAdd( parent->samplelight, patch->samplelight, parent->samplelight );
	}

	// average up the direct light on each patch for radiosity
	if (numbounce > 0)
	{
		for( patch = &patches.Element( facePatches.Element( facenum ) ); patch; patch = pNextPatch )
		{
			// next patch
			pNextPatch = NULL;
			if( patch->ndxNext != patches.InvalidIndex() )
			{
				pNextPatch = &patches.Element( patch->ndxNext );
			}

			if (patch->samplearea)
			{ 
				float scale;
				Vector v;
				scale = 1.0 / patch->area;

				VectorScale( patch->samplelight, scale, v );
				VectorAdd( patch->totallight.light[0], v, patch->totallight.light[0] );
				VectorAdd( patch->directlight, v, patch->directlight );
			}
		}
	}

	// pull totallight from children (children always exist first in the list)
	for( patch = &patches.Element( facePatches.Element( facenum ) ); patch; patch = pNextPatch )
	{
		// next patch
		pNextPatch = NULL;
		if( patch->ndxNext != patches.InvalidIndex() )
		{
			pNextPatch = &patches.Element( patch->ndxNext );
		}

		if ( patch->child1 != patches.InvalidIndex() )
		{
			float s1, s2;
			patch_t *child1;
			patch_t *child2;

			child1 = &patches.Element( patch->child1 );
			child2 = &patches.Element( patch->child2 );

			s1 = child1->area / (child1->area + child2->area);
			s2 = child2->area / (child1->area + child2->area);

			VectorScale( child1->totallight.light[0], s1, patch->totallight.light[0] );
			VectorMA( patch->totallight.light[0], s2, child2->totallight.light[0], patch->totallight.light[0] );

			VectorCopy( patch->totallight.light[0], patch->directlight );
		}
	}

	bool needsBumpmap = false;
	if( texinfo[f->texinfo].flags & SURF_BUMPLIGHT )
	{
		needsBumpmap = true;
	}

	// add an ambient term if desired
	if (ambient[0] || ambient[1] || ambient[2])
	{
		for( int j=0; j < MAXLIGHTMAPS && f->styles[j] != 255; j++ )
		{
			if ( f->styles[j] == 0 )
			{
				for (i = 0; i < fl->numsamples; i++)
				{
					VectorAdd( fl->light[j][0][i], ambient, fl->light[j][0][i] ); 
					if( needsBumpmap )
					{
						VectorAdd( fl->light[j][1][i], ambient, fl->light[j][1][i] ); 
						VectorAdd( fl->light[j][2][i], ambient, fl->light[j][2][i] ); 
					}
				}
				break;
			}
		}
	}
}


/*
=============
PrecompLightmapOffsets
=============
*/

void PrecompLightmapOffsets()
{
    int facenum;
    dface_t *f;
    int lightstyles;
    int lightdatasize = 0;
    
    // NOTE: We store avg face light data in this lump *before* the lightmap data itself
	// in *reverse order* of the way the lightstyles appear in the styles array.
    for( facenum = 0; facenum < numfaces; facenum++ )
    {
        f = &dfaces[facenum];

        if ( texinfo[f->texinfo].flags & TEX_SPECIAL)
            continue;		// non-lit texture
        
        //if ( dlight_map != 0 )
        //    f->styles[1] = 0;
        
        for (lightstyles=0; lightstyles < MAXLIGHTMAPS; lightstyles++ )
        {
            if ( f->styles[lightstyles] == 255 )
                break;
        }
        
        if ( !lightstyles )
            continue;

        // Reserve room for the avg light color data
		lightdatasize += lightstyles * 4;

        f->lightofs = lightdatasize;

		bool needsBumpmap = false;
		if( texinfo[f->texinfo].flags & SURF_BUMPLIGHT )
		{
			needsBumpmap = true;
		}

		int nLuxels = (f->m_LightmapTextureSizeInLuxels[0]+1) * (f->m_LightmapTextureSizeInLuxels[1]+1);
		if( needsBumpmap )
		{
			lightdatasize += nLuxels * 4 * lightstyles * ( NUM_BUMP_VECTS + 1 );
		}
		else
		{
	        lightdatasize += nLuxels * 4 * lightstyles;
		}
    }

	dlightdata.SetSize( lightdatasize );
	dlightdata2.SetSize( lightdatasize );
	bouncedlightdata.SetSize( lightdatasize );
}