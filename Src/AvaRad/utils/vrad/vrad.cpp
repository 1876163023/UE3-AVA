/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

// vrad.c

#include "mpi.h"
#include "vrad.h"
#include "lightmap.h"
#include "vstdlib/strtools.h"
#include "mpidist.h"
#include "ava_mpirad.h"
#include "ava_radworld.h"

#define ALLOWOPTIONS (0 || _DEBUG)

//{{ scene
//!{ 2006-03-20	허 창 민
IVRadDLL2* s_pVRadDll2;
//!} 2006-03-20	허 창 민


/*

NOTES
-----

every surface must be divided into at least two patches each axis

*/

//patch_t		patches[MAX_PATCHES];
//unsigned		num_patches;
//patch_t		*face_patches[MAX_MAP_FACES];
//patch_t		*face_parents[MAX_MAP_FACES];
//patch_t		*cluster_children[MAX_MAP_CLUSTERS];
//Vector		emitlight[MAX_PATCHES];
//Vector		addlight[MAX_PATCHES];

CUtlVector<patch_t>		patches;			
CUtlVector<int>			facePatches;		// constains all patches, children first
CUtlVector<int>			faceParents;		// contains only root patches, use next parent to iterate
CUtlVector<int>			clusterChildren;
CUtlVector<Vector>		emitlight;
CUtlVector<bumplights_t> addlight;

int g_nDXLevel = 90;

int num_sky_cameras;
sky_camera_t sky_cameras[MAX_MAP_AREAS];
int area_sky_cameras[MAX_MAP_AREAS];

//!{ 2006-05-03	허 창 민
//entity_t	*face_entity[MAX_MAP_FACES];
//Vector		face_offset[MAX_MAP_FACES];		// for rotating bmodels

CUtlVector<entity_t*>	face_entity;
CUtlVector<Vector>		face_offset;
//!} 2006-05-03	허 창 민

int			fakeplanes;


unsigned	numbounce = 100; // 25; /* Originally this was 8 */

int			sampleskylevel = 0;	// 0 = none, 1 = 162 sphere sampling, 2 = 1024 cosine weighted hemisphere sampling
float		hdr_tonemapscale = 0;
qboolean	g_bUseRaytrace = false;

float		maxchop = 64;
float		minchop = 64;
qboolean	dumppatches;
bool	    bDumpNormals = false;
bool		bRed2Black = false;

int			TestLine (Vector& start, Vector& stop);

int			junk;

Vector		ambient( 0, 0, 0 );
float		maxlight = (256 * 4 - 1);	// 4x overbright

float		lightscale = 1.0;
float		dlight_threshold = 0.1;  // was DIRECT_LIGHT constant

char		source[MAX_PATH] = "";
char		platformPath[MAX_PATH] = "";

char		global_lights[MAX_PATH] = "";
char		designer_lights[MAX_PATH] = "";
char		level_lights[MAX_PATH] = "";

char		vismatfile[_MAX_PATH] = "";
char		incrementfile[_MAX_PATH] = "";

//IIncremental *g_pIncremental = 0;
bool		g_bInterrupt = false;	// Wsed with background lighting in WC. Tells VRAD
									// to stop lighting.

float		gamma = 0.5;
float		indirect_sun = 1.0;
float		reflectivityScale = 1.0;
qboolean	do_extra = true;
bool		debug_extra = false;
qboolean	do_fast = false;
qboolean	do_centersamples = false;
qboolean	do_dispblend = true;
int			extrapasses = 4;
float		smoothing_threshold = 0.7071067; // cos(45.0*(M_PI/180)) 
// Cosine of smoothing angle(in radians)
float		coring = 1.0;	// Light threshold to force to blackness(minimizes lightmaps)
qboolean	texscale = true;
int			dlight_map = 0; // Setting to 1 forces direct lighting into different lightmap than radiosity

float		luxeldensity = 1.0;
unsigned	num_degenerate_faces;

qboolean	g_bLowPriority = false;
qboolean	g_bLogHashData = false;

double		g_flStartTime;

CUtlVector<byte> g_FacesVisibleToLights;


//!{ 2006-05-12	허 창 민
int	vertex_chunk_size = 10;	// default chunk
int ambientcube_chunk_size = 10;
//!} 2006-05-12	허 창 민



/*
===================================================================

MISC

===================================================================
*/
int		leafparents[MAX_MAP_LEAFS];
int		nodeparents[MAX_MAP_NODES];

/*
=======================================================================

MAKE FACES

=======================================================================
*/

/*
=============
WindingFromFace
=============
*/
winding_t	*WindingFromFace (dface_t *f, Vector& origin )
{
	int			i;
	int			se;
	dvertex_t	*dv;
	int			v;
	winding_t	*w;

	w = AllocWinding (f->numedges);
	w->numpoints = f->numedges;

	for (i=0 ; i<f->numedges ; i++)
	{
		se = dsurfedges[f->firstedge + i];
		if (se < 0)
			v = dedges[-se].v[1];
		else
			v = dedges[se].v[0];

		dv = &dvertexes[v];
		VectorAdd (dv->point, origin, w->p[i]);
	}

	RemoveColinearPoints (w);

	return w;
}

/*
=============
BaseLightForFace
=============
*/
void BaseLightForFace( dface_t *f, Vector& light, float *parea, Vector& reflectivity )
{
	texinfo_t	*tx;
	dtexdata_t	*texdata;

	//
	// check for light emited by texture
	//
	tx = &texinfo[f->texinfo];
	texdata = &dtexdata[tx->texdata];

	light = texdata->brightness;	


	//<@ changmin; 2006. 10. 10
	// texture size를 light와 연계시키지 않을 것입니다.
	//*parea = texdata->height * texdata->width;
	*parea = 1.0f;
	//>@ 

	VectorScale( texdata->reflectivity, reflectivityScale, reflectivity );
	
	// always keep this less than 1 or the solution will not converge
	for ( int i = 0; i < 3; i++ )
	{
		if ( reflectivity[i] > 0.99 )
			reflectivity[i] = 0.99;
	}
}

qboolean IsSky (dface_t *f)
{
	texinfo_t	*tx;

	tx = &texinfo[f->texinfo];
	if (tx->flags & SURF_SKY)
		return true;
	return false;
}

#ifdef STATIC_FOG
/*=============
IsFog
=============*/
qboolean IsFog( dface_t *f )
{
	texinfo_t	*tx;

	tx = &texinfo[f->texinfo];

    // % denotes a fog texture
    if( tx->texture[0] == '%' )
		return true;

	return false;
}
#endif

/*
=============
MakePatchForFace
=============
*/
float	totalarea;
void MakePatchForFace (int fn, winding_t *w)
{
	//!{ 2006-05-03	허 창 민
	//dface_t     *f = dfaces + fn;
	dface_t     *f = &dfaces[fn];
	//!} 2006-05-03	허 창 민
	float	    area;
	patch_t		*patch;
	Vector		centroid(0,0,0);
	int			i, j;
	texinfo_t	*tx;

    // get texture info
    tx = &texinfo[f->texinfo];

	// No patches at all for fog!
#ifdef STATIC_FOG
	if ( IsFog( f ) )
		return;
#endif

	// the sky needs patches or the form factors don't work out correctly
	// if (IsSky( f ) )
	// 	return;	

	area = WindingArea (w);
	if (area <= 0)
	{
		num_degenerate_faces++;
		Msg("degenerate face %d area %f\n",fn, area);
		return;
	}	

	totalarea += area;

	// get a patch
	int ndxPatch = patches.AddToTail();
	patch = &patches[ndxPatch];
	memset( patch, 0, sizeof( patch_t ) );
	patch->ndxNext = patches.InvalidIndex();
	patch->ndxNextParent = patches.InvalidIndex();
	//patch->ndxNextClusterChild = patches.InvalidIndex();	// ava specific ; 우리는 이거 안 씁니다.
	patch->child1 = patches.InvalidIndex();
	patch->child2 = patches.InvalidIndex();
	patch->parent = patches.InvalidIndex();
	patch->needsBumpmap = tx->flags & SURF_BUMPLIGHT ? true : false;

	// link and save patch data
	patch->ndxNext = facePatches.Element( fn );
	facePatches[fn] = ndxPatch;
//	patch->next = face_patches[fn];
//	face_patches[fn] = patch;

	// compute a separate scale for chop - since the patch "scale" is the texture scale
	// we want textures with higher resolution lighting to be chopped up more
	float chopscale[2];
	chopscale[0] = chopscale[1] = 16.0f;
    if ( texscale )
    {
        // Compute the texture "scale" in s,t
        for( i=0; i<2; i++ )
        {
            patch->scale[i] = 0.0f;
			chopscale[i] = 0.0f;
            for( j=0; j<3; j++ )
			{
                patch->scale[i] += 
					tx->textureVecsTexelsPerWorldUnits[i][j] * 
					tx->textureVecsTexelsPerWorldUnits[i][j];
                chopscale[i] += 
					tx->lightmapVecsLuxelsPerWorldUnits[i][j] * 
					tx->lightmapVecsLuxelsPerWorldUnits[i][j];
			}
            patch->scale[i] = sqrt( patch->scale[i] );
			chopscale[i] = sqrt( chopscale[i] );
        }
    }
    else
	{
		patch->scale[0] = patch->scale[1] = 1.0f;
	}
	patch->area = area;
 	patch->sky = IsSky( f );

	// chop scaled up lightmaps coarser
	patch->luxscale = ((chopscale[0]+chopscale[1])/2);
	patch->chop = maxchop;

#ifdef STATIC_FOG
    patch->fog = FALSE;
#endif

	patch->winding = w;

	patch->plane = &dplanes[f->planenum];

	// make a new plane to adjust for origined bmodels
	if (face_offset[fn][0] || face_offset[fn][1] || face_offset[fn][2] )
	{	
		dplane_t	*pl;

		// origin offset faces must create new planes
		if (numplanes + fakeplanes >= MAX_MAP_PLANES)
			Error ("numplanes + fakeplanes >= MAX_MAP_PLANES");
		pl = &dplanes[numplanes + fakeplanes];
		fakeplanes++;

		*pl = *(patch->plane);
		pl->dist += DotProduct (face_offset[fn], pl->normal);
		patch->plane = pl;
	}

	patch->faceNumber = fn;
	WindingCenter (w, patch->origin);

	// Save "center" for generating the face normals later.
	VectorSubtract( patch->origin, face_offset[fn], face_centroids[fn] ); 

	VectorCopy( patch->plane->normal, patch->normal );
	VectorAdd (patch->origin, patch->normal, patch->origin);

	WindingBounds (w, patch->face_mins, patch->face_maxs);
	VectorCopy( patch->face_mins, patch->mins );
	VectorCopy( patch->face_maxs, patch->maxs );

	BaseLightForFace( f, patch->baselight, &patch->basearea, patch->reflectivity );

	//!{ 2006-03-21	허 창 민
	// emissive patch 지원 / 이 결과는 texlight를 override 한다.
	AvaFace* face = &s_pVRadDll2->FaceArray_[fn];
	if( !VectorCompare( face->Emission_, vec3_origin ) )
	{
		VectorCopy( face->Emission_, patch->baselight );
		patch->basearea = patch->area;
	}
	//!} 2006-03-21	허 창 민

	// Chop all texlights very fine.
	if ( !VectorCompare( patch->baselight, vec3_origin ) )
	{
		//patch->chop = do_extra ? minchop / 2 : minchop;
	}

	// get rid of do extra functionality on displacement surfaces
	if( ValidDispFace( f ) )
	{
		patch->chop = maxchop;
	}
}

/*
=======================================================================

SUBDIVIDE

=======================================================================
*/


//-----------------------------------------------------------------------------
// Purpose: does this surface take/emit light
//-----------------------------------------------------------------------------
bool Nolight( patch_t *patch )
{
	//!{ 2006-05-03	허 창 민
	//dface_t *f = dfaces + patch->faceNumber;
	dface_t *f = &dfaces[patch->faceNumber];
	//!} 2006-05-03	허 창 민
	texinfo_t *tx = &texinfo[f->texinfo];
	if (tx->flags & SURF_NOLIGHT)
		return true;

	return false;
}

int CreateChildPatch( int nParentIndex, winding_t *pWinding, float flArea, const Vector &vecCenter )
{
	int nChildIndex = patches.AddToTail();

	patch_t *child = &patches[nChildIndex];
	patch_t *parent = &patches[nParentIndex];

	// copy all elements of parent patch to children
	*child = *parent;

	// Set up links
	child->ndxNext = patches.InvalidIndex();
	child->ndxNextParent = patches.InvalidIndex();
	child->child1 = patches.InvalidIndex();
	child->child2 = patches.InvalidIndex();
	child->parent = nParentIndex;
	child->winding = pWinding;
	child->area = flArea;

	VectorCopy( vecCenter, child->origin );
	if ( ValidDispFace( &dfaces[child->faceNumber] ) )
	{
		// shouldn't get here anymore!!
		Msg( "SubdividePatch: Error - Should not be here!\n" );
		StaticDispMgr()->GetDispSurfNormal( child->faceNumber, child->origin, child->normal, true );
	}
	else
	{
		GetPhongNormal( child->faceNumber, child->origin, child->normal );
	}

	child->planeDist = child->plane->dist;
	WindingBounds(child->winding, child->mins, child->maxs);

	if ( !VectorCompare( child->baselight, vec3_origin ) )
	{
		// don't check edges on surf lights
		return nChildIndex;
	}

	// Subdivide patch towards minchop if on the edge of the face
	Vector total;
	VectorSubtract( child->maxs, child->mins, total );
	VectorScale( total, child->luxscale, total );
	if ( child->chop > minchop && (total[0] < child->chop) && (total[1] < child->chop) && (total[2] < child->chop) )
	{
		for ( int i=0; i<3; ++i )
		{
			if ( (child->face_maxs[i] == child->maxs[i] || child->face_mins[i] == child->mins[i] )
				&& total[i] > minchop )
			{
				child->chop = max( minchop, child->chop / 2 );
				break;
			}
		}
	}

	return nChildIndex;
}



//-----------------------------------------------------------------------------
// Purpose: subdivide the "parent" patch
//-----------------------------------------------------------------------------
void SubdividePatch( int ndxPatch )
{
	winding_t *w, *o1, *o2;
	Vector	total;
	Vector	split;
	vec_t	dist;
	vec_t	widest = -1;
	int		i, widest_axis = -1;
	bool	bSubdivide = false;

	// get the current patch
	patch_t *patch = &patches.Element( ndxPatch );
	if( !patch )
		return;

	// never subdivide sky patches
	if( patch->sky )
		return;

	// get the patch winding
	w = patch->winding;

	// subdivide along the widest axis
	VectorSubtract (patch->maxs, patch->mins, total);
	VectorScale( total, patch->luxscale, total );
	for (i=0 ; i<3 ; i++)
	{
		if ( total[i] > widest )
		{
			widest_axis = i;
			widest = total[i];
		}
		if ( total[i] >= patch->chop && total[i] >= minchop )
		{
			bSubdivide = true;
		}
	}

	if ( (!bSubdivide) && widest_axis != -1)
	{
		// make more square
		if (total[widest_axis] > total[(widest_axis + 1) % 3] * 2
		&&	total[widest_axis] > total[(widest_axis + 2) % 3] * 2)
		{
			if (patch->chop > minchop)
			{
				bSubdivide = true;
				patch->chop = max( minchop, patch->chop / 2 );
			}
		}
	}

	if( !bSubdivide )
	{
		return;
	}

	// split the winding
	VectorCopy (vec3_origin, split);
	split[widest_axis] = 1;
	dist = (patch->mins[widest_axis] + patch->maxs[widest_axis])*0.5f;
	ClipWindingEpsilon (w, split, dist, ON_EPSILON, &o1, &o2);

	// calculate the area of the patches to see if they are "significant"
	Vector center1;
	Vector center2;
	float area1 = WindingAreaAndBalancePoint( o1, center1 );
	float area2 = WindingAreaAndBalancePoint( o2, center2 );

	if( area1 == 0 || area2 == 0 )
	{
		Msg( "zero area child patch\n" );
		return;
	}

	// create new child patches
	int ndxChild1Patch = CreateChildPatch( ndxPatch, o1, area1, center1 );
	int ndxChild2Patch = CreateChildPatch( ndxPatch, o2, area2, center2 );

	// FIXME: This could go into CreateChildPatch if child1, child2 were stored in the patch as child[0], child[1]
	patch = &patches.Element( ndxPatch );
	patch->child1 = ndxChild1Patch;
	patch->child2 = ndxChild2Patch;		

	SubdividePatch( ndxChild1Patch );
	SubdividePatch( ndxChild2Patch );
}

//=====================================================================

/*
=============
MakeScales

  This is the primary time sink.
  It can be run multi threaded.
=============
*/
int	total_transfer;
int max_transfer;


void MakeTransfer( int ndxPatch1, int ndxPatch2, transfer_t *all_transfers )
//void MakeTransfer (patch_t *patch, patch_t *patch2, transfer_t *all_transfers )
{
	Vector	delta;
	vec_t	dist, scale;
	float	trans;
	transfer_t *transfer;

	//
	// get patches
	//
	if( ndxPatch1 == patches.InvalidIndex() || ndxPatch2 == patches.InvalidIndex() )
		return;

	patch_t *patch = &patches.Element( ndxPatch1 );
	patch_t *patch2 = &patches.Element( ndxPatch2 );

	transfer = &all_transfers[patch->numtransfers];

	// calculate transferemnce
	VectorSubtract (patch2->origin, patch->origin, delta);
	dist = VectorNormalize (delta);
	scale = -DotProduct (delta, patch2->normal);

	// bumpmapped patches will recompute this in the function that integrates the transfers
	if( !patch->needsBumpmap )
	{
		scale *= DotProduct( delta, patch->normal );
	}

	// patch normals may be > 90 due to smoothing groups
	if (scale <= 0)
	{
		//Msg("scale <= 0\n");
		return;
	}

	trans = (patch2->area*scale);

	// hack for patch areas that area <= 0 (degenerate)
	if (trans <= 0)
	{
		//Msg("trans <= 0 : %f %f %f\n", trans, patch2->area, scale );
		return;
	}

	float dist_scale = dist*dist;
	trans = trans / dist_scale;	

	if (trans <= 1e-5f)
	{
		//Msg("trans2 <= 0\n");
		return;
	}	

	transfer->patch = patch2 - patches.Base();

	// FIXME: why is this not trans?
	transfer->transfer = trans;

	patch->numtransfers++;
}


void MakeScales ( int ndxPatch, transfer_t *all_transfers )
{
	int		j;
	float	total;
	transfer_t	*t, *t2;
	total = 0;

	if( ndxPatch == patches.InvalidIndex() )
		return;
	patch_t *patch = &patches.Element( ndxPatch );

	// copy the transfers out
	if (patch->numtransfers)
	{
		if (patch->numtransfers > max_transfer)
		{
			max_transfer = patch->numtransfers;
		}


		patch->transfers = ( transfer_t* )calloc (1, patch->numtransfers * sizeof(transfer_t));
		if (!patch->transfers)
			Error ("Memory allocation failure");

		// get total transfer energy
		t2 = all_transfers;

		// overflow check!
		for (j=0 ; j<patch->numtransfers ; j++, t2++)
		{
			total += t2->transfer;
		}

		// the total transfer should be PI, but we need to correct errors due to overlaping surfaces
		if (total > M_PI)
			total = 1.0f/total;
		else	
			total = 1.0f/(M_PI);				

		t = patch->transfers;
		t2 = all_transfers;
		for (j=0 ; j<patch->numtransfers ; j++, t++, t2++)
		{
			t->transfer = t2->transfer*total;
			t->patch = t2->patch;
		}
		if (patch->numtransfers > max_transfer)
		{
			max_transfer = patch->numtransfers;
		}
	}
	else
	{
		// Error - patch has no transfers
		// patch->totallight[2] = 255;
	}

	ThreadLock ();
	total_transfer += patch->numtransfers;
	ThreadUnlock ();
}


/*
=============
WriteWorld
=============
*/
void WriteWorld (char *name)
{
	unsigned	j;
	FILE		*out;
	patch_t		*patch;

	out = fopen (name, "w");
	if (!out)
		Error ("Couldn't open %s", name);

	unsigned int uiPatchCount = patches.Size();
	for (j=0; j<uiPatchCount; j++)
	{
		patch = &patches.Element( j );

		// skip parent patches
		if (patch->child1 != patches.InvalidIndex() )
			continue;

		//if( patch->clusterNumber == -1 )
		//{
		//	Vector vGreen;
		//	VectorClear( vGreen );
		//	vGreen[1] = 256.0f;
		//	WriteWinding( out, patch->winding, vGreen );
		//}
		//else
		{
			Vector light = patch->totallight.light[0] + patch->directlight;
			WriteWinding( out, patch->winding, light );
		}
	}

	g_pFileSystem->Close (out);
}


void WriteWinding (FileHandle_t out, winding_t *w, Vector& color )
{
	int			i;

	CmdLib_FPrintf (out, "%i\n", w->numpoints);
	for (i=0 ; i<w->numpoints ; i++)
	{
		//!{ 2006-06-09	허 창 민
		/*
		CmdLib_FPrintf (out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
			w->p[i][0],
			w->p[i][1],
			w->p[i][2],
			color[ 0 ] / 256,
			color[ 1 ] / 256,
			color[ 2 ] / 256 );
		*/

		CmdLib_FPrintf (out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n",
			w->p[i][0],
			w->p[i][1],
			w->p[i][2],
			color[ 0 ],
			color[ 1 ],
			color[ 2 ]);
		//!} 2006-06-09	허 창 민
	}
	CmdLib_FPrintf (out, "\n");
}


void WriteNormal( FileHandle_t out, Vector const &nPos, Vector const &nDir, 
				  float length, Vector const &color )
{
	CmdLib_FPrintf( out, "2\n" );
	CmdLib_FPrintf( out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n", 
		nPos.x, nPos.y, nPos.z,
		color.x / 256, color.y / 256, color.z / 256 );
	CmdLib_FPrintf( out, "%5.2f %5.2f %5.2f %5.3f %5.3f %5.3f\n", 
		nPos.x + ( nDir.x * length ), 
		nPos.y + ( nDir.y * length ), 
		nPos.z + ( nDir.z * length ),
		color.x / 256, color.y / 256, color.z / 256 );
	CmdLib_FPrintf( out, "\n" );
}



/*
=============
CollectLight
=============
*/
// patch's totallight += new light received to each patch
// patch's emitlight = addlight (newly received light from GatherLight)
// patch's addlight = 0
// pull received light from children.
void CollectLight( Vector& total )
{
	int i, j;
	patch_t	*patch;

	VectorFill( total, 0 );

	// process patches in reverse order so that children are processed before their parents
	unsigned int uiPatchCount = patches.Size();
	for( i = uiPatchCount - 1; i >= 0; i-- )
	{
		patch = &patches.Element( i );
		const int normalCount = patch->needsBumpmap ? NUM_BUMP_VECTS+1 : 1;
		// sky's never collect light, it is just dropped
		if (patch->sky)
		{
			VectorFill( emitlight[ i ], 0 );
		}
		else if ( patch->child1 == patches.InvalidIndex() )
		{
			// This is a leaf node.
			for( j = 0; j < normalCount; ++j )
			{
				VectorAdd( patch->totallight.light[j], addlight[i].light[j], patch->totallight.light[j] );
			}
			VectorCopy( addlight[i].light[0], emitlight[i] );
			VectorAdd( total, emitlight[i], total );
		}
		else
		{
			// This is an interior node.
			// Pull received light from children.
			float s1, s2;
			patch_t *child1;
			patch_t *child2;

			child1 = &patches[patch->child1];
			child2 = &patches[patch->child2];

			// BUG: This doesn't do anything?
			if ((int)patch->area != (int)(child1->area + child2->area))
			{
				s1 = 0;
			}

			s1 = child1->area / (child1->area + child2->area);
			s2 = child2->area / (child1->area + child2->area);

			// patch->totallight = s1 * child1->totallight + s2 * child2->totallight
			for( j = 0; j < normalCount; ++j )
			{
				VectorScale( child1->totallight.light[j], s1, patch->totallight.light[j] );
				VectorMA( patch->totallight.light[j], s2, child2->totallight.light[j], patch->totallight.light[j] );
			}

			// patch->emitlight = s1 * child1->emitlight + s2 * child2->emitlight
			VectorScale( emitlight[patch->child1], s1, emitlight[i] );
			VectorMA( emitlight[i], s2, emitlight[patch->child2], emitlight[i] );
		}
		for( j = 0; j < NUM_BUMP_VECTS+1; ++j)
		{
			VectorFill( addlight[i].light[j], 0 );
		}
	}
}

/*
=============
GatherLight

Get light from other patches
  Run multi-threaded
=============
*/

#ifdef _WIN32
#pragma warning (disable:4701)
#endif

void GatherLight (int threadnum, void *pUserData)
{
	int			i, j, k;
	transfer_t	*trans;
	int			num;
	patch_t		*patch;
	Vector		sum, v;

	while (1)
	{
		j = GetThreadWork ();
		if (j == -1)
			break;

		patch = &patches[j];

		trans = patch->transfers;
		num = patch->numtransfers;

		if( patch->needsBumpmap )
		{
			Vector delta;
			Vector bumpSum[NUM_BUMP_VECTS+1];
			Vector normals[NUM_BUMP_VECTS+1];

			GetPhongNormal( patch->faceNumber, patch->origin, normals[0] );

			texinfo_t *pTexinfo = &texinfo[dfaces[patch->faceNumber].texinfo];
			// use facenormal along with the smooth normal to build the three bump map vectors
			Vector sAxis(pTexinfo->textureVecsTexelsPerWorldUnits[0][0],
				pTexinfo->textureVecsTexelsPerWorldUnits[0][1],
				pTexinfo->textureVecsTexelsPerWorldUnits[0][2]
				);
			Vector tAxis(pTexinfo->textureVecsTexelsPerWorldUnits[1][0],
				pTexinfo->textureVecsTexelsPerWorldUnits[1][1],
				pTexinfo->textureVecsTexelsPerWorldUnits[1][2]
				);
			GetBumpNormals( sAxis, 
							tAxis,
							patch->normal, 
							normals[0],
							&normals[1] );

			for ( i = 0; i < NUM_BUMP_VECTS+1; i++ )
			{
				VectorFill( bumpSum[i], 0 );
			}

			float dot;
			for (k=0 ; k<num ; k++, trans++)
			{
				patch_t *patch2 = &patches[trans->patch];

				// get bump normals
				VectorSubtract (patch2->origin, patch->origin, delta);
				VectorNormalize (delta);
				for(i=0; i<3; i++)
				{
					v[i] = emitlight[trans->patch][i] * patch2->reflectivity[i];
				}
				VectorScale( v, trans->transfer, v );

				Vector bumpTransfer;
				for ( i = 0; i < NUM_BUMP_VECTS+1; i++ )
				{
					dot = DotProduct( delta, normals[i] );
					if ( dot <= 0 )
					{
						// Assert( i > 0 ); // if this hits, then the transfer shouldn't be here.  It doesn't face the flat normal of this face!
						continue;
					}
					bumpTransfer = v * dot;
					VectorAdd( bumpSum[i], bumpTransfer, bumpSum[i] );
				}
			}
			for ( i = 0; i < NUM_BUMP_VECTS+1; i++ )
			{
				VectorCopy( bumpSum[i], addlight[j].light[i] );
			}
		}
		else
		{
			VectorFill( sum, 0 );

			for (k=0 ; k<num ; k++, trans++)
			{
				for(i=0; i<3; i++)
				{
					v[i] = emitlight[trans->patch][i] * patches[trans->patch].reflectivity[i];
				}
				VectorScale( v, trans->transfer, v );
				VectorAdd( sum, v, sum );
			}
			VectorCopy( sum, addlight[j].light[0] );
		}
	}
}

#ifdef _WIN32
#pragma warning (default:4701)
#endif
/*
=============
BounceLight
=============
*/
void BounceLight (void)
{
	unsigned i;
	Vector	added;
	char		name[64];
	qboolean	bouncing = numbounce > 0;

	unsigned int uiPatchCount = patches.Size();
	for (i=0 ; i<uiPatchCount; i++)
	{
		// totallight has a copy of the direct lighting.  Move it to the emitted light and zero it out (to integrate bounces only)
		VectorCopy( patches[i].totallight.light[0], emitlight[i] );

		// NOTE: This means that only the bounced light is integrated into totallight!
		VectorFill( patches[i].totallight.light[0], 0 );
	}

	if (g_bUseMPI && g_bMPIMaster)
	{
		Msg( "[[PROGRESS:Bouncing %d %d]]\n", 0, numbounce ? numbounce : 1 );
	}

	i = 0;
	while ( bouncing )
	{
		// transfer light from to the leaf patches from other patches via transfers
		// this moves shooter->emitlight to receiver->addlight
		unsigned int uiPatchCount = patches.Size();
		RunThreadsOn (uiPatchCount, true, GatherLight);
		// move newly received light (addlight) to light to be sent out (emitlight)
		// start at children and pull light up to parents
		// light is always received to leaf patches
		CollectLight( added );		
		Msg( "[[PROGRESS:Bouncing %d %d]]\n", i, numbounce ? numbounce : 1 );
		qprintf ("\tBounce #%i added RGB(%.0f, %.0f, %.0f)\n", i+1, added[0], added[1], added[2] );
		if ( i+1 == numbounce || (added[0] < 1/255.0 && added[1] < 1/255.0 && added[2] < 1/255.0) )
			bouncing = false;

		i++;
		if ( dumppatches && !bouncing && i != 1)
		{
			sprintf (name, "bounce%i.txt", i);
			WriteWorld (name);
		}
	}

	if (g_bUseMPI && g_bMPIMaster)
	{
		Msg( "[[PROGRESS:Bouncing %d %d]]\n", i, numbounce ? numbounce : 1 );
	}
}

void MakeAllScales (void)
{
	// determine visibility between patches
	BuildVisMatrix ();
	// release visibility matrix
	FreeVisMatrix ();
	Msg("transfers %d, max %d\n", total_transfer, max_transfer );
	qprintf ("transfer lists: %5.1f megs\n", (float)total_transfer * sizeof(transfer_t) / (1024*1024));
}

bool RadWorld_Go()
{
	// build initial facelights
	if (g_bUseMPI) 
	{
		AVA_MPI_BuildFacelights();
	}
	else 
	{
		RunThreadsOnIndividual (numfaces, true, BuildFacelights);
	}

	if( g_bUseMPI )
	{
		AVA_MPI_BuildVertexlights();
	}
	else
	{
		RunThreadsOnIndividual( AVA_StaticMeshManager::GetNumMeshes(), true, AVA_BuildVertexLights );
	}

	// Figure out the offset into lightmap data for each face.
	if (!g_bUseMPI || g_bMPIMaster)
	{
		PrecompLightmapOffsets();

		//!{ 2006-08-18	허 창 민
		// 2006/8/18 : shadowmap 기능 제거
		PrecompShadowmapOffsets();
		//!} 2006-08-18	허 창 민
	}

	// free up the direct lights now that we have facelights
	DeleteDirectLights();

	if ( dumppatches )
	{
		WriteWorld( "bounce0.txt" );
	}
	if (numbounce > 0)
	{
		// allocate memory for emitlight/addlight
		emitlight.SetSize( patches.Size() );
		memset( emitlight.Base(), 0, patches.Size() * sizeof( Vector ) );
		addlight.SetSize( patches.Size() );
		memset( addlight.Base(), 0, patches.Size() * sizeof( bumplights_t ) );
		MakeAllScales ();
		BounceLight ();
	}

	if (!g_bUseMPI || g_bMPIMaster)
	{
		//
		// displacement surface luxel accumulation (make threaded!!!)
		//
		StaticDispMgr()->StartTimer( "Build Patch/Sample Hash Table(s)....." );
		StaticDispMgr()->InsertSamplesDataIntoHashTable();
		StaticDispMgr()->InsertPatchSampleDataIntoHashTable();
		StaticDispMgr()->EndTimer();

		// blend bounced light into direct light and save
		RunThreadsOnIndividual (numfaces, true, FinalLightFace);
		Msg("FinalLightFace Done\n");
		fflush(stdout);
	}

	AVA_MPI_DistributeLightingData();

	if( g_bUseMPI )
	{
		AVA_MPI_BuildVertexAmbientlights();
	}
	else
	{
		RunThreadsOnIndividual( GetNumVertexLightingJobs(), true, ComputeVertexAmbientLights );
	}

	if( g_bUseMPI )
	{
		AVA_MPI_BuildAmbientcubes();
	}
	else
	{
		RunThreadsOnIndividual( GetNumAmbientCubeJobs(), true, GatherAmbientCube );
	}

	if( !g_bUseMPI || g_bMPIMaster )
	{			
		if( Tonemap::DoTonemap( hdr_tonemapscale ) )
		{
			RunThreadsOnIndividual(numfaces, true, TonemapLightFace );
			Msg("TonemapLightFace Done\n"); fflush(stdout);

			RunThreadsOnIndividual(GetNumVertexLightingJobs(), true, TonemapLightVertices );
			Msg("TonemapLightVertex Done\n"); fflush(stdout);

			RunThreadsOnIndividual(GetNumAmbientCubeJobs(), true, TonemapAmbientCubes );
			Msg("TonemapAmbientCubes Done\n"); fflush(stdout);
		}
	}
	return true;
}

//<@ GI Rendering 2007. 5. 2
void Vec3toColorRGBExp32( Vector& v, colorRGBExp32 *c );

void AVA_UpdateVertexLighting( bool bDirectLighting )
{
	// prepare next bouncing
	for( int vertexIndex = 0; vertexIndex < num_blackmesh_vertexes; ++vertexIndex )
	{
		blackmesh_bouncing_data[vertexIndex] = blackmesh_vertexlight_float_vector_data[vertexIndex*4];
	}

	// accumulate and encode vertex light to result buffer
	// direct light는 여기에서 깨진다.
	const int color_size = 4;
	const int vertex_light_size = 16;
	for( int vertexIndex = 0; vertexIndex < num_blackmesh_vertexes; ++vertexIndex )
	{
		for( int bumpIndex = 0; bumpIndex < NUM_BUMP_VECTS + 1; ++bumpIndex )
		{
			const int sampleIndex = vertexIndex * 4 + bumpIndex;

			// world vertex light 와 reasult vertex light 순서가 바뀌면 안된다. blackemsh_vertex_light_float_vector_data를 공유하는데, 변경되기 때문이다.
			// accumulate world vertex light ( include realtime light sytle )
			colorRGBExp32 *worldrgbe = ( colorRGBExp32* ) &blackmesh_worldvertexlightdata[ vertexIndex * vertex_light_size + color_size * bumpIndex ];
			Vector worldvertexLight;
			worldvertexLight.x = TexLightToLinear( worldrgbe->r, worldrgbe->exponent );
			worldvertexLight.y = TexLightToLinear( worldrgbe->g, worldrgbe->exponent );
			worldvertexLight.z = TexLightToLinear( worldrgbe->b, worldrgbe->exponent );
			VectorAdd( worldvertexLight, blackmesh_vertexlight_float_vector_data[sampleIndex], worldvertexLight );
			Vec3toColorRGBExp32( worldvertexLight, worldrgbe );

			// accumulate vertex light ( exclude realtime light style )
			colorRGBExp32 *rgbe = ( colorRGBExp32* ) &blackmesh_vertexlightdata[ vertexIndex * vertex_light_size + color_size * bumpIndex ];
			Vector vertexLight;
			vertexLight.x = TexLightToLinear( rgbe->r, rgbe->exponent );
			vertexLight.y = TexLightToLinear( rgbe->g, rgbe->exponent );
			vertexLight.z = TexLightToLinear( rgbe->b, rgbe->exponent );
			if( bDirectLighting )
			{
				// 여기서 변경.. realtime 성분(sun 성분이 제외된다.)
				VectorSubtract(
					blackmesh_vertexlight_float_vector_data[sampleIndex],
					blackmesh_realtime_vertexlighting[sampleIndex],
					blackmesh_vertexlight_float_vector_data[sampleIndex] );

				blackmesh_vertexlight_float_vector_data[sampleIndex].x =
					(blackmesh_vertexlight_float_vector_data[sampleIndex].x < 0) ? 0.0f : blackmesh_vertexlight_float_vector_data[sampleIndex].x; 
				blackmesh_vertexlight_float_vector_data[sampleIndex].y =
					(blackmesh_vertexlight_float_vector_data[sampleIndex].y < 0) ? 0.0f : blackmesh_vertexlight_float_vector_data[sampleIndex].y; 
				blackmesh_vertexlight_float_vector_data[sampleIndex].z =
					(blackmesh_vertexlight_float_vector_data[sampleIndex].z < 0) ? 0.0f : blackmesh_vertexlight_float_vector_data[sampleIndex].z; 
			}
			VectorAdd( vertexLight, blackmesh_vertexlight_float_vector_data[sampleIndex], vertexLight );
			Vec3toColorRGBExp32( vertexLight, rgbe );
		}
	}
}

// world light value 를 sample buffer로 옮겨야 합니다.
// face			-> bouncedlightdata
// blackmesh	-> blackmesh_bouncing_data
void AVA_PrepareWorldAmbientSampling()
{
	// prepare face bouncing
	memcpy( bouncedlightdata.Base(), dlightdata.Base(), bouncedlightdata.Count() );

	// prepare blackmesh bouncing
	const int color_size		= 4;
	const int vertex_light_size = 16;
	const int bumpIndex			= 0;
	for( int vertexIndex = 0; vertexIndex < num_blackmesh_vertexes; ++vertexIndex )
	{
		colorRGBExp32 *src = ( colorRGBExp32* ) &blackmesh_worldvertexlightdata[ vertexIndex * vertex_light_size + color_size * bumpIndex ];
		Vector *vertexLight = &blackmesh_bouncing_data[vertexIndex];
		vertexLight->x = TexLightToLinear( src->r, src->exponent );
		vertexLight->y = TexLightToLinear( src->g, src->exponent );
		vertexLight->z = TexLightToLinear( src->b, src->exponent );
	}

}

//<@ Secondary Light Source : 2007. 8. 17
void AVA_CreateSecondaryLights()
{
	directlight_t	*dl = NULL;
	float secondarylight_threshold = 100.0f;
	if (s_pVRadDll2->SunLight_.Intensity_.Length() != 0)
	{
		secondarylight_threshold = s_pVRadDll2->SunLight_.SkyMax_;
	}

	int totalsamples = 0;
	int numsecondarylightsources = 0;
	for( int faceIndex = 0; faceIndex < numfaces; ++faceIndex )
	{
		dface_t* face = &dfaces[faceIndex];
		if( texinfo[face->texinfo].flags & SURF_SECONDARYLIGHTSOURCE )
		{
			facelight_t *fl = &facelight[faceIndex];
			Vector* irradianceSamples = fl->light[0][0];

			totalsamples += fl->numsamples;
			for( int sampleIndex = 0; sampleIndex < fl->numsamples; ++sampleIndex )
			{
				if( irradianceSamples[sampleIndex].y > secondarylight_threshold )
				{
					sample_t* sample = &fl->sample[sampleIndex];
					if( sample->area > 1e-6 )
					{
						numsecondarylightsources++;
						++numdlights;
						dl = AllocDLight( sample->pos );
						dl->light.type = emit_surface;
						dl->secondaryfacenum = faceIndex;
						VectorCopy( sample->normal, dl->light.normal );

						VectorScale( irradianceSamples[sampleIndex], sample->area, dl->light.intensity );
						texinfo_t *tex = &texinfo[face->texinfo];
						dtexdata_t *td = &dtexdata[tex->texdata];
						dl->light.intensity.x *= td->reflectivity.x / M_PI;
						dl->light.intensity.y *= td->reflectivity.y / M_PI;
						dl->light.intensity.z *= td->reflectivity.z / M_PI;
					}
				}
			}
		}
	}

	if( !g_bUseMPI || g_bMPIMaster )
	{
		Msg("create %d secondary lights (%f%%)", numsecondarylightsources, (float)numsecondarylightsources/(float)totalsamples * 100.0f );
	}	
}
//>@ Secondary Light Source

bool AVA_RenderWorld()
{
	AVA_InitializeFacesAndFacelights();

	if( g_bUseMPI )
	{
		AVA_MPI_BuildSecondaryFacelights();	// secondary light source를 생성할 face 에 대한 lighting을 먼저 계산합니다.

		AVA_MPI_DistributeSecondaryFacelights();	// lighting 결과를 모든 노드에 전파합니다.

		AVA_CreateSecondaryLights();			// lighting 된 결과에 근거해, secondary light source를 activelights list에 추가합니다.

		AVA_MPI_BuildFacelights2();			// 나머지 face에 대한 lighting을 계산합니다.
	}
	else
	{
		RunThreadsOnIndividual( numfaces, true, AVA_BuildFacelights );
	}

	if( g_bUseMPI )
	{
		AVA_MPI_BuildVertexlights();
	}
	else
	{
		RunThreadsOnIndividual( AVA_StaticMeshManager::GetNumMeshes(), true, AVA_BuildVertexLights );
	}

	PrecompLightmapOffsets();
	PrecompShadowmapOffsets();	// 없앨 기능입니다만...

	// compute diffuse interreflection
	//if( numbounce )
	const bool bBounce = true;	// debug 용
	if( !g_bUseMPI || g_bMPIMaster )
	{
		// lightmap 초기화
		memset( dlightdata.Base(), 0, dlightdata.Count() );
		memset( dlightdata2.Base(), 0, dlightdata2.Count() );

		RunThreadsOnIndividual( numfaces, true, AVA_MakeFaceLightmapFromDirectLightingSamples );

		AVA_UpdateVertexLighting( true );
	}

	for( int bounceCount = 0; bounceCount < (bBounce ? 2 : 0); ++bounceCount )
	{
		if( g_bUseMPI )
		{
			AVA_MPI_DistributeBouncedLight();

			AVA_MPI_GatherFaceAmbientLights();

			AVA_MPI_GatherVertexAmbientLights();

			if( g_bMPIMaster )
			{
				RunThreadsOnIndividual( numfaces, true, AVA_MakeFaceLightmapFromDiffuseReflectionSamples );

				AVA_UpdateVertexLighting( false );
			}
		}
		else
		{
			RunThreadsOnIndividual( numfaces, true, AVA_GatherFaceAmbientlights );

			RunThreadsOnIndividual( GetNumVertexLightingJobs(), true, AVA_GatherVertexAmbientLights );

			RunThreadsOnIndividual( numfaces, true, AVA_MakeFaceLightmapFromDiffuseReflectionSamples );
			
			AVA_UpdateVertexLighting( false );
		}
	}

	// free sample windings..
	// 여기서.. super sampling 할때 sample winding을 사용함...
	if( do_extra )
	{
		for( int facenum = 0; facenum < numfaces; ++facenum )
		{
			facelight_t *fl = &facelight[facenum];
			for (int i = 0; i < fl->numsamples; i++)
			{
				if (fl->sample[i].w)
				{
					FreeWinding( fl->sample[i].w );
					fl->sample[i].w = NULL;
				}
			}
		}
	}

	// world light value 를 sample buffer로 옮겨야 합니다.
	// face			-> bouncedlightdata
	// blackmesh	-> blackmesh_bouncing_data
	if( !g_bUseMPI || g_bMPIMaster )
	{
		AVA_PrepareWorldAmbientSampling();
	}

	if( g_bUseMPI )
	{
		AVA_MPI_DistributeBouncedLight();
	}

	// compute ambient cubes
	if( g_bUseMPI )
	{
		AVA_MPI_BuildAmbientcubes_GI();
	}
	else
	{
		RunThreadsOnIndividual( GetNumAmbientCubeJobs(), true, GatherAmbientCubeGI );
	}

	// tonemap results
	if( !g_bUseMPI || g_bMPIMaster )
	{			
		if( Tonemap::DoTonemap( hdr_tonemapscale ) )
		{
			RunThreadsOnIndividual(numfaces, true, TonemapLightFace );
			RunThreadsOnIndividual(GetNumVertexLightingJobs(), true, TonemapLightVertices );
			RunThreadsOnIndividual(GetNumAmbientCubeJobs(), true, TonemapAmbientCubes );
		}
	}
	return true;
}
//>@ 



// declare the sample file pointer -- the whole debug print system should
// be reworked at some point!! (cab)
FileHandle_t pFileSamples[4];

void VRAD_Finish()
{
	if( dumppatches )
	{
		for( int ndx = 0; ndx < 4; ndx++ )
		{
			g_pFileSystem->Close( pFileSamples[ndx] );
		}
	}
	double end = I_FloatTime ();
	Msg("%5.0f seconds elapsed\n", end-g_flStartTime);
}


// Run startup code like initialize mathlib (called from main() and from the 
// WorldCraft interface into vrad).
void VRAD_Init()
{
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f, false, false, false, false );
	InstallSpewFunction();
}

void AVA_MakePatches()
{
	int		    i, j;
	dface_t	    *f;
	int		    fn;
	winding_t	*w;
	Vector		origin;

	qprintf ("%i faces\n", numfaces);
	nummodels = 1;

	for (i=0 ; i<nummodels ; i++)
	{
		VectorCopy (vec3_origin, origin);
		for(j=0; j < numfaces; ++j)
		{
			fn = j;
			VectorCopy (origin, face_offset[fn]);
			f = &dfaces[fn];
			if( f->dispinfo == -1 )
			{
				w = WindingFromFace (f, origin );
				MakePatchForFace( fn, w );
			}
		}
	}
	if (num_degenerate_faces > 0)
	{
		qprintf("%d degenerate faces\n", num_degenerate_faces );
	}
	qprintf ("%i square feet [%.2f square inches]\n", (int)(totalarea/144), totalarea );
	//// make the displacement surface patches
	StaticDispMgr()->MakePatches();
}

/*
=============
AVA_SubdividePatches
=============
*/
void AVA_SubdividePatches(void)
{
	unsigned i;

	if (numbounce == 0)
	{
		return;
	}

	unsigned int uiPatchCount = patches.Size();
	qprintf ("%i patches before subdivision\n", uiPatchCount);

	for (i = 0; i < uiPatchCount; i++)
	{
		patch_t *pCur = &patches.Element( i );
		pCur->planeDist = pCur->plane->dist;

		pCur->ndxNextParent = faceParents.Element( pCur->faceNumber );
		faceParents[pCur->faceNumber] = pCur - patches.Base();
	}

	for (i=0 ; i< uiPatchCount; i++)
	{
		patch_t *patch = &patches.Element( i );
		patch->parent = -1;
		if ( Nolight(patch) )
		{
			continue;
		}

		if (!do_fast)
		{
			if( dfaces[patch->faceNumber].dispinfo == -1 )
			{
				SubdividePatch( i );
			}
			else
			{
				StaticDispMgr()->SubdividePatch( i );
			}
		}
	}

	// fixup next pointers
	for (i = 0; i < (unsigned)numfaces; i++)
	{
		facePatches[i] = facePatches.InvalidIndex();
	}

	uiPatchCount = patches.Size();
	for (i = 0; i < uiPatchCount; i++)
	{
		patch_t *pCur = &patches.Element( i );
		pCur->ndxNext = facePatches.Element( pCur->faceNumber );
		facePatches[pCur->faceNumber] = pCur - patches.Base();
	}
	qprintf ("%i patches after subdivision\n", uiPatchCount);
}

int ParseCommandLine( int argc, char **argv, bool *onlydetail )
{
	*onlydetail = false;

	int i;
	// parsing parameters
	for( i=1 ; i<argc ; i++ )
	{
		if (!strcmp(argv[i],"-dump"))
		{
			dumppatches = true;

			for( int ndx = 0; ndx < 4; ndx++ )
			{
				char filename[MAX_PATH];
				sprintf( filename, "samples%i.txt", ndx );
				pFileSamples[ndx] = g_pFileSystem->Open( filename, "w" );
				if( !pFileSamples[ndx] )
					return -1;
			}
		}
		else if( !strcmp( argv[i], "-dxlevel" ) )
		{
			g_nDXLevel = atoi( argv[i+1] );
			Msg( "DXLevel = %d\n", g_nDXLevel );
			++i;
		}
		else if( !strcmp( argv[i], "-red2black" ) )
		{
			bRed2Black = true;
		}
		else if( !strcmp( argv[i], "-matpath" ) )
		{
			strcpy(qproject, argv[i+1]);
			++i;
		}
		else if( !strcmp( argv[i], "-dumpnormals" ) )
		{
			bDumpNormals = true;
		}
		else if (!strcmp(argv[i],"-bounce"))
		{
			if ( ++i < argc )
			{
#if ALLOWOPTIONS
				numbounce = atoi (argv[i]);
				if ( numbounce < 0 )
				{
					Warning("Error: expected non-negative value after '-bounce'\n" );
					return 1;
				}
#else
				if (atoi (argv[i]) == 0)
				{
					numbounce = 0;
				}
#endif
			}
			else
			{
				Warning("Error: expected a value after '-bounce'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-verbose"))
		{
			verbose = true;
		}
		else if (!strcmp(argv[i],"-terse"))
		{
			verbose = false;
		}
		else if (!strcmp(argv[i],"-threads"))
		{
			if ( ++i < argc )
			{
				numthreads = atoi (argv[i]);
				if ( numthreads <= 0 )
				{
					Warning("Error: expected positive value after '-threads'\n" );
					return 1;
				}
			}
			else
			{
				Warning("Error: expected a value after '-threads'\n" );
				return 1;
			}
		}
#if ALLOWOPTIONS
		else if (!strcmp(argv[i],"-maxchop"))
		{
			if ( ++i < argc )
			{
				maxchop = (float)atof (argv[i]);
				if ( maxchop < 2 )
				{
					Warning("Error: expected positive value after '-maxchop'\n" );
					return 1;
				}
			}
			else
			{
				Warning("Error: expected a value after '-maxchop'\n" );
				return 1;
			}
		}
#endif
#if ALLOWOPTIONS
		else if (!strcmp(argv[i],"-chop"))
		{
			if ( ++i < argc )
			{
				minchop = (float)atof (argv[i]);
				if ( minchop < 1 )
				{
					Warning("Error: expected positive value after '-chop'\n" );
					return 1;
				}
				if ( minchop < 32 )
				{
					Warning("WARNING: Chop values below 32 are not recommended.  Use -extra instead.\n");
				}
			}
			else
			{
				Warning("Error: expected a value after '-chop'\n" );
				return 1;
			}
		}
#endif
#if ALLOWOPTIONS
		else if (!strcmp(argv[i],"-scale"))
		{
			if ( ++i < argc )
			{
				lightscale = (float)atof (argv[i]);
			}
			else
			{
				Warning("Error: expected a value after '-scale'\n" );
				return 1;
			}
		}
#endif
#if ALLOWOPTIONS
		else if (!strcmp(argv[i],"-ambient"))
		{
			if ( i+3 < argc )
			{
				ambient[0] = (float)atof (argv[++i]) * 128;
				ambient[1] = (float)atof (argv[++i]) * 128;
				ambient[2] = (float)atof (argv[++i]) * 128;
			}
			else
			{
				Warning("Error: expected three color values after '-ambient'\n" );
				return 1;
			}
		}
#endif
		// UNDONE: JAY: Is this necessary anymore?
		else if( !strcmp(argv[i], "-proj") )
		{
			if ( ++i < argc && *argv[i] )
				strcpy( qproject, argv[i] );
			else
			{
				Warning("Error: expected path name after '-proj'\n" );
				return 1;
			}
		}
#if ALLOWOPTIONS
		else if ( !strcmp(argv[i], "-maxlight") )
		{
			if ( ++i < argc && *argv[i] )
			{
				maxlight = (float)atof (argv[i]) * 256;
				if ( maxlight <= 0 )
				{
					Warning("Error: expected positive value after '-maxlight'\n" );
					return 1;
				}
			}
			else
			{
				Warning("Error: expected a value after '-maxlight'\n" );
				return 1;
			}
		}
#endif
		else if ( !strcmp(argv[i], "-lights" ) )
		{
			if ( ++i < argc && *argv[i] )
			{
				strcpy( designer_lights, argv[i] );
			}
			else
			{
				Warning("Error: expected a filepath after '-lights'\n" );
				return 1;
			}
		}
#if ALLOWOPTIONS
		else if (!strcmp(argv[i],"-dlight"))
		{
			if ( ++i < argc )
			{
				dlight_threshold = (float)atof (argv[i]);
			}
			else
			{
				Warning("Error: expected a value after '-dlight'\n" );
				return 1;
			}
		}
#endif
		else if (!strcmp(argv[i],"-extra"))
		{
			do_extra = true;
		}
		else if (!strcmp(argv[i],"-noextra"))
		{
			do_extra = false;
		}
		else if (!strcmp(argv[i],"-debugextra"))
		{
			debug_extra = true;
		}
		else if (!strcmp(argv[i],"-fast"))
		{
			do_fast = true;
		}
		else if (!strcmp(argv[i],"-centersamples"))
		{
			do_centersamples = true;
		}
		else if( !strcmp( argv[i], "-dispblend" ) )
		{
			do_dispblend = false;
		}
#if ALLOWOPTIONS
		else if (!strcmp(argv[i],"-sky"))
		{
			if ( ++i < argc )
			{
				indirect_sun = (float)atof (argv[i]);
			}
			else
			{
				Warning("Error: expected a value after '-sky'\n" );
				return 1;
			}
		}
#endif
		else if (!strcmp(argv[i],"-smooth"))
		{
			if ( ++i < argc )
			{
				smoothing_threshold = (float)cos(atof(argv[i])*(M_PI/180.0));
			}
			else
			{
				Warning("Error: expected an angle after '-smooth'\n" );
				return 1;
			}
		}
#if ALLOWOPTIONS
		else if (!strcmp(argv[i],"-coring"))
		{
			if ( ++i < argc )
			{
				coring = (float)atof( argv[i] );
			}
			else
			{
				Warning("Error: expected a light threshold after '-coring'\n" );
				return 1;
			}
		}
#endif
#if ALLOWOPTIONS
		else if (!strcmp(argv[i],"-notexscale"))
		{
			texscale = false;
		}
#endif
		else if (!strcmp(argv[i],"-dlightmap"))
		{
			dlight_map = 1;
		}
		else if (!strcmp(argv[i],"-luxeldensity"))
		{
			if ( ++i < argc )
			{
				luxeldensity = (float)atof (argv[i]);
				if (luxeldensity > 1.0)
					luxeldensity = 1.0 / luxeldensity;
			}
			else
			{
				Warning("Error: expected a value after '-luxeldensity'\n" );
				return 1;
			}
		}
		else if( !stricmp( argv[i], "-low" ) )
		{
			g_bLowPriority = true;
		}
		else if( !stricmp( argv[i], "-loghash" ) )
		{
			g_bLogHashData = true;
		}
		else if( !stricmp( argv[i], "-onlydetail" ) )
		{
			*onlydetail = true;
		}
		else if ( stricmp( argv[i], "-StopOnExit" ) == 0 )
		{
			g_bStopOnExit = true;
		}
		else if ( !Q_strncasecmp( argv[i], "-mpi", 4 ) || !Q_strncasecmp( argv[i-1], "-mpi", 4 ) )
		{
			if ( stricmp( argv[i], "-mpi" ) == 0 )
				g_bUseMPI = true;

			// Any other args that start with -mpi are ok too.
			if ( i == argc - 1 )
				break;
		}
		//!{ 2006-07-14	허 창 민
		else if (!strcmp(argv[i],"-samplesky"))
		{
			if( ++i < argc )
			{
				sampleskylevel = atoi (argv[i]);

				if (sampleskylevel < 0 || sampleskylevel > 2 )
				{
					Warning("Error: expected value after '-sampleskylevel' 0, 1, 2\n" );
					return 1;
				}
			}
			else
			{
				Warning("Error: expected a value after '-sampleskylevel'\n" );
				return 1;
			}
		}
		else if (!strcmp(argv[i],"-hdrtonemapscale"))
		{
			if( ++i < argc )
			{
				hdr_tonemapscale = atof (argv[i]);

				if (hdr_tonemapscale < 0 || hdr_tonemapscale > 20 )
				{
					Warning("Error: expected value after '-hdrtonemapscale' 0..20\n" );
					return 1;
				}
			}
			else
			{
				Warning("Error: expected a value after '-hdrtonemapscale'\n" );
				return 1;
			}
		}
		else if( !strcmp(argv[i], "-raytrace") )
		{
			g_bUseRaytrace = true;
		}
		//!} 2006-07-14	허 창 민
		else
		{
			break;
		}
	}
	return i;
}

void AVA_LoadWorld( char const *pFilename )
{
	ThreadSetDefault ();
	g_flStartTime = I_FloatTime ();
	if( g_bLowPriority )
	{
		SetLowPriority();
	}
	if ( !g_bUseMPI )
	{
		// Setup the logfile.
		char logFile[512];
		_snprintf( logFile, sizeof(logFile), "%s.log", source );
		SetSpewFunctionLogFile( logFile );
	}

	// load geometry data
	if (!s_pVRadDll2->Load( pFilename ))
	{
		Error( "Failed to load %s", pFilename );
	}
	// 호출 순서에 유의
	// dplanes / dvertexes / texinfo / dtexdata / dfaces / dedges / g_dispinfo 정보 생성
	AVA_BuildRadWorld();

	// lightmap extents를 계산해 준다.
	UpdateAllFaceLightmapExtents();

	// displacement ( terrain ) 의 collision model도 생성한다.
	StaticDispMgr()->Init();

	// collision model을 만든다.
	CreateOpcodeModels();

	// alloc for patches
	facePatches.SetSize( numfaces );
	faceParents.SetSize( numfaces );
	for( int ndx = 0; ndx < numfaces; ndx++ )
	{
		facePatches[ndx] = facePatches.InvalidIndex();
		faceParents[ndx] = faceParents.InvalidIndex();
	}
}

void AVA_StartWorld()
{
	AVA_MakePatches();
	PairEdges();
	AVA_SubdividePatches();
	CreateDirectLights();
}

int AVA_RunVRAD( int argc, char **argv )
{
	// for debugging
	//MessageBox( NULL, "Hello", "World", MB_OK );

	MPI::COMM_WORLD.Barrier();

	verbose = true;  // Originally FALSE
	bool onlydetail = false;
	int i = ParseCommandLine( argc, argv, &onlydetail );
	if (i != argc - 1)
	{
		Error ( "usage: vrad [-dump] [-inc] [-bounce n] [-threads n] [-verbose] [-terse] [-proj file] [-maxlight n] [-threads n] [-lights file] [-extra] [-smooth n] [-dlightmap] [-fast] [-blendsamples] [-lowpriority] [-StopOnExit] [-mpi] bspfile" );
	}

	MPI::COMM_WORLD.Barrier();

	// delete previous output file
	if( !g_bUseMPI || g_bMPIMaster )
	{
		TCHAR buf[1024];
		strcpy( buf, argv[i] );
		strcat( buf, TEXT(".out") );
		unlink( buf );
	}

	Pbrt::Initialize();

	InitOpcode();

	AVA_LoadWorld( argv[i] );

	if( g_bUseRaytrace )
	{
		AVA_StartWorld();
		patches.Purge();		// direclight를 생성한 이후는 patch정보가 필요없다..
		AVA_RenderWorld();
	}
	else
	{
		AVA_StartWorld();
		RadWorld_Go();
	}

	Msg("Ready to Finish\n");
	fflush(stdout);

	Msg("Saving result\n"); 
	{
		TCHAR buf[1024];
		strcpy( buf, argv[i] );
		strcat( buf, TEXT(".out") );
		if (!g_bUseMPI || g_bMPIMaster)
		{
			s_pVRadDll2->Save( buf );
		}

	}	
	Msg("Save completed\n"); 

	MPI::COMM_WORLD.Barrier();

	VRAD_Finish();	

	FinalizeOpcode();

	return 0;
}

int AVA_VRAD_Main(int argc, char** argv, IVRadDLL2* pVRad2 )
{
	VRAD_Init();

	s_pVRadDll2 = pVRad2;

	return AVA_RunVRAD( argc, argv );
}