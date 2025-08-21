/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "vrad.h"
//#include "vmpi.h"
#ifdef MPI
#include "messbuf.h"
static MessageBuffer mb;
#endif

#include "mpidist.h"
#include "ava_mpirad.h"

#define	HALFBIT

#define PLANE_TEST_EPSILON  0.01 // patch must be this much in front of the plane to be considered "in front"

extern char		source[MAX_PATH];
extern char		vismatfile[_MAX_PATH];
extern char		incrementfile[_MAX_PATH];
extern qboolean	incremental;

extern int	total_transfer;
extern int max_transfer;
/*
===================================================================

VISIBILITY MATRIX

Determine which patches can see each other
Use the PVS to accelerate if available
===================================================================
*/

#define TEST_EPSILON	0.1

dleaf_t* PointInLeaf (int iNode, Vector const& point)
{
	if ( iNode < 0 )
		return &dleafs[ (-1-iNode) ];

	dnode_t *node = &dnodes[iNode];
	dplane_t *plane = &dplanes[ node->planenum ];

	float dist = DotProduct (point, plane->normal) - plane->dist;
	if ( dist > TEST_EPSILON )
	{
		return PointInLeaf( node->children[0], point );
	}
	else if ( dist < -TEST_EPSILON )
	{
		return PointInLeaf( node->children[1], point );
	}
	else
	{
		dleaf_t *pTest = PointInLeaf( node->children[0], point );
		if ( pTest->cluster != -1 )
			return pTest;

		return PointInLeaf( node->children[1], point );
	}
}


int ClusterFromPoint( Vector const& point )
{
	dleaf_t *leaf = PointInLeaf( 0, point );
	
	return leaf->cluster;
}

void PvsForOrigin (Vector& org, byte *pvs)
{
	int visofs;
	int cluster;

	if (!visdatasize)
	{
		memset (pvs, 255, (dvis->numclusters+7)/8 );
		return;
	}

	cluster = ClusterFromPoint( org );
	if ( cluster < 0 )
	{
		visofs = -1;
	}
	else
	{
		visofs = dvis->bitofs[ cluster ][DVIS_PVS];
	}

	if (visofs == -1)
		Error ("visofs == -1");

	DecompressVis (&dvisdata[visofs], pvs);
}


void TestPatchToPatch( int ndxPatch1, int ndxPatch2, int head, transfer_t *transfers, int iThread )
{
	Vector tmp;

	//
	// get patches
	//
	if( ndxPatch1 == patches.InvalidIndex() || ndxPatch2 == patches.InvalidIndex() )
		return;

	patch_t *patch = &patches.Element( ndxPatch1 );
	patch_t *patch2 = &patches.Element( ndxPatch2 );

	if (patch2->child1 != patches.InvalidIndex() )
	{
		// check to see if we should use a child node instead

		VectorSubtract( patch->origin, patch2->origin, tmp );
		// SQRT( 1/4 )
		// FIXME: should be based on form-factor (ie. include visible angle, etc)
		if ( DotProduct(tmp, tmp) * 0.0625 < patch2->area )
		{
			TestPatchToPatch( ndxPatch1, patch2->child1, head, transfers, iThread );
			TestPatchToPatch( ndxPatch1, patch2->child2, head, transfers, iThread );
			return;
		}
	}

	// check vis between patch and patch2
	// if bit has not already been set
	//  && v2 is not behind light plane
	//  && v2 is visible from v1

	if ( DotProduct (patch2->origin, patch->normal) > patch->planeDist + PLANE_TEST_EPSILON )
	{
		// push out origins from face so that don't intersect their owners
		Vector p1, p2;
		VectorAdd( patch->origin, patch->normal, p1 );
		VectorAdd( patch2->origin, patch2->normal, p2 );
		// FIXME: make a TetsLine that knows the two faces and planes and won't collide with them.
		if( TestLine (p1, p2, head, iThread) == CONTENTS_EMPTY )
		{
			MakeTransfer( ndxPatch1, ndxPatch2, transfers );
		}
	}

}


/*
==============
TestPatchToFace

Sets vis bits for all patches in the face
==============
*/
void TestPatchToFace (unsigned patchnum, int facenum, int head, transfer_t *transfers, int iThread )
{
	if( faceParents.Element( facenum ) == patches.InvalidIndex() || patchnum == patches.InvalidIndex() )
		return;

	patch_t	*patch = &patches.Element( patchnum );
	patch_t *patch2 = &patches.Element( faceParents.Element( facenum ) );

	// if emitter is behind that face plane, skip all patches

	patch_t *pNextPatch;

	if ( patch2 && DotProduct(patch->origin, patch2->normal) > patch2->planeDist + 1.01 )
	{
		// we need to do a real test
		for( ; patch2; patch2 = pNextPatch )
		{
			// next patch
			pNextPatch = NULL;
			if( patch2->ndxNextParent != patches.InvalidIndex() )
			{
				pNextPatch = &patches.Element( patch2->ndxNextParent );
			}

			/*
			// skip patches too far away
			VectorSubtract( patch->origin, patch2->origin, tmp );
			if (DotProduct( tmp, tmp ) > 1024 * 1024 * 8)
				continue;
				*/
			

			int ndxPatch2 = patch2 - patches.Base();
			TestPatchToPatch( patchnum, ndxPatch2, head, transfers, iThread );
		}
	}
}

struct ClusterDispList_t
{
	CUtlVector<int>	dispFaces;
};

static CUtlVector<ClusterDispList_t> g_ClusterDispFaces;

//-----------------------------------------------------------------------------
// Helps us find all displacements associated with a particular cluster
//-----------------------------------------------------------------------------
//void AddDispsToClusterTable( void )
//{
//	g_ClusterDispFaces.SetCount( g_ClusterLeaves.Count() );
//
//	//
//	// add displacement faces to the cluster table
//	//
//	for( int ndxFace = 0; ndxFace < numfaces; ndxFace++ )
//	{
//		// search for displacement faces
//		if( dfaces[ndxFace].dispinfo == -1 )
//			continue;
//
//		//
//		// get the clusters associated with the face
//		//
//		if( facePatches.Element( ndxFace ) != facePatches.InvalidIndex() )
//		{
//			patch_t *pNextPatch = NULL;
//			for( patch_t *pPatch = &patches.Element( facePatches.Element( ndxFace ) ); pPatch; pPatch = pNextPatch )
//			{
//				// next patch
//				pNextPatch = NULL;
//				if( pPatch->ndxNext != patches.InvalidIndex() )
//				{
//					pNextPatch = &patches.Element( pPatch->ndxNext );
//				}
//
//				if( pPatch->clusterNumber != patches.InvalidIndex() )
//				{
//					int ndxDisp = g_ClusterDispFaces[pPatch->clusterNumber].dispFaces.Find( ndxFace );
//					if( ndxDisp == -1 )
//					{
//						ndxDisp = g_ClusterDispFaces[pPatch->clusterNumber].dispFaces.AddToTail();
//						g_ClusterDispFaces[pPatch->clusterNumber].dispFaces[ndxDisp] = ndxFace;
//					}
//				}
//			}
//		}
//	}
//}


/*
==============
BuildVisRow

Calc vis bits from a single patch
==============
*/
void BuildVisRow (int patchnum, byte *pvs, int head, transfer_t *transfers, int iThread )
{
	int		j, k, l, leafIndex;
	patch_t	*patch;
	dleaf_t	*leaf;

	//!{ 2006-05-03	허 창 민

	//byte	face_tested[MAX_MAP_FACES];
	//byte	disp_tested[MAX_MAP_FACES];

	CUtlVector<byte> face_tested;
	CUtlVector<byte> disp_tested;
	face_tested.SetSize( numfaces );
	disp_tested.SetSize( numfaces );

	patch = &patches.Element( patchnum );

	//memset( face_tested, 0, numfaces ) ;
	//memset( disp_tested, 0, numfaces );

	memset( face_tested.Base(), 0, numfaces ) ;
	memset( disp_tested.Base(), 0, numfaces );

	//!} 2006-05-03	허 창 민

	for (j=0; j<dvis->numclusters; j++)
	{
		if ( ! ( pvs[(j)>>3] & (1<<((j)&7)) ) )
		{
			continue;		// not in pvs
		}

		for ( leafIndex = 0; leafIndex < g_ClusterLeaves[j].leafCount; leafIndex++ )
		{
			leaf = dleafs + g_ClusterLeaves[j].leafs[leafIndex];

			for (k=0 ; k<leaf->numleaffaces; k++)
			{
				l = dleaffaces[leaf->firstleafface + k];
				// faces can be marksurfed by multiple leaves, but
				// don't bother testing again
				if (face_tested[l])
				{
					continue;
				}
				face_tested[l] = 1;

				// don't check patches on the same face
				if (patch->faceNumber == l)
					continue;
				TestPatchToFace (patchnum, l, head, transfers, iThread );
			}
		}

		int dispCount = g_ClusterDispFaces[j].dispFaces.Size();
		for( int ndxDisp = 0; ndxDisp < dispCount; ndxDisp++ )
		{
			int ndxFace = g_ClusterDispFaces[j].dispFaces[ndxDisp];
			if( disp_tested[ndxFace] )
				continue;

			disp_tested[ndxFace] = 1;

			// don't check patches on the same face
			if( patch->faceNumber == ndxFace )
				continue;

			TestPatchToFace( patchnum, ndxFace, head, transfers, iThread );
		}
	}


	// Msg("%d) Transfers: %5d\n", patchnum, patch->numtransfers);
}

//!{ 2006-03-22	허 창 민
void BuildVisRow2 (int patchnum, transfer_t *transfers, int iThread )
{
	int		j;
	patch_t	*patch;
	patch = &patches.Element( patchnum );
	for( j=0; j < numfaces; j++)
	{
		if( patch->faceNumber == j )
		{
			continue;
		}
		TestPatchToFace( patchnum, j, 0, transfers, iThread );
	}
}
//!} 2006-03-22	허 창 민



/*
===========
BuildVisLeafs

  This is run by multiple threads
===========
*/

transfer_t* BuildVisLeafs_Start()
{
	return (transfer_t *)calloc( 1,  MAX_PATCHES * sizeof( transfer_t ) );
}


// If PatchCB is non-null, it is called after each row is generated (used by MPI).
void BuildVisLeafs_Cluster( 
	int threadnum,
	transfer_t *transfers, 
	int iCluster, 
	void (*PatchCB)(int iThread, int patchnum, patch_t *patch)
	)
{
	//<@ ava specific ; 2007. 2. 21 changmin
	// 우리는 이거 안 쓸 겁니다.
	//byte	pvs[(MAX_MAP_CLUSTERS+7)/8];
	//patch_t	*patch;
	//int		head;
	//unsigned	patchnum;

	//
	//DecompressVis( &dvisdata[ dvis->bitofs[ iCluster ][DVIS_PVS] ], pvs);
	//head = 0;

	//// light every patch in the cluster
	//if( clusterChildren.Element( iCluster ) != clusterChildren.InvalidIndex() )
	//{
	//	patch_t *pNextPatch;
	//	for( patch = &patches.Element( clusterChildren.Element( iCluster ) ); patch; patch = pNextPatch )
	//	{
	//		//
	//		// next patch
	//		//
	//		pNextPatch = NULL;
	//		if( patch->ndxNextClusterChild != patches.InvalidIndex() )
	//		{
	//			pNextPatch = &patches.Element( patch->ndxNextClusterChild );
	//		}
	//		
	//		patchnum = patch - patches.Base();

	//		// build to all other world clusters
	//		BuildVisRow (patchnum, pvs, head, transfers, threadnum );
	//		
	//		// do the transfers
	//		MakeScales( patchnum, transfers );

	//		// Let MPI aggregate the data if it's being used.
	//		if ( PatchCB )
	//			PatchCB( threadnum, patchnum, patch );
	//	}
	//}
	//>@ ava
}

//!{ 2006-03-22	허 창 민
// If PatchCB is non-null, it is called after each row is generated (used by MPI).
void BuildVisLeafs2_Cluster( 
						   int threadnum,
						   transfer_t *transfers, 
						   int iFace, 
						   void (*PatchCB)(int iThread, int patchnum, patch_t *patch)
						   )
{
	patch_t*	patch;
	patch_t*	pNextPatch;
	unsigned	patchnum;

	// facePatch index가 invalid한 경우에 대한 처리
	if (facePatches.Element( iFace ) != facePatches.InvalidIndex()) 
	{
		// light every patch in the Face
		patch = &patches.Element( facePatches.Element( iFace ) );

		for( ; patch ; patch = pNextPatch  )
		{
			pNextPatch = NULL;
			if( patch->ndxNext != patches.InvalidIndex() )
			{
				pNextPatch = &patches.Element( patch->ndxNext );
			}

			if( patch->child1 == patches.InvalidIndex() )
			{
				patchnum = patch - patches.Base();

				BuildVisRow2( patchnum, transfers, threadnum );

				MakeScales( patchnum, transfers );
			}
		}
	}
}
//!} 2006-03-22	허 창 민


void BuildVisLeafs_End( transfer_t *transfers )
{
	free( transfers );
}


void BuildVisLeafs( int threadnum, void *pUserData )
{
	transfer_t *transfers = BuildVisLeafs_Start();
	
	while ( 1 )
	{
		//
		// build a minimal BSP tree that only
		// covers areas relevent to the PVS
		//
		// JAY: Now this returns a cluster index
		int iCluster = GetThreadWork();
		if ( iCluster == -1 )
			break;

		BuildVisLeafs_Cluster( threadnum, transfers, iCluster, NULL );
	}
	
	BuildVisLeafs_End( transfers );
}
//!{ 2006-03-22	허 창 민
void BuildVisLeafs2( int threadnum, void *pUserData )
{
	transfer_t *transfers = BuildVisLeafs_Start();

	while ( 1 )
	{
		//
		// build a minimal BSP tree that only
		// covers areas relevent to the PVS
		//
		// JAY: Now this returns a cluster index
		int iFace = GetThreadWork();
		if ( iFace == -1 )
			break;

		BuildVisLeafs2_Cluster( threadnum, transfers, iFace, NULL );
	}

	BuildVisLeafs_End( transfers );
}
//!} 2006-03-22	허 창 민

void BuildVisLeafs3( UINT iThread, UINT iFace, LPVOID pUserData )
{
	if ((int)iThread < 0)
		return;

	BuildVisLeafs2_Cluster( iThread, (transfer_t*)pUserData, iFace, NULL );
}

/*
==============
BuildVisMatrix
==============
*/
void BuildVisMatrix (void)
{
	if ( g_bUseMPI )
	{
		Threads threads;
		threads.SetNumThreads( numthreads );
		class VisMatrixMPIInterface : public MPIInterface
		{
		public :			
			virtual UINT GetBufferCount( UINT iFace ) 
			{
				UINT nPatches = 0;

				// facePatch index가 invalid한 경우에 대한 처리
				if (facePatches.Element( iFace ) != facePatches.InvalidIndex())
				{
					patch_t*	patch;
					patch_t*	pNextPatch;
					unsigned	patchnum;

					// light every patch in the Face
					patch = &patches.Element( facePatches.Element( iFace ) );

					for( ; patch ; patch = pNextPatch  )
					{
						pNextPatch = NULL;
						if( patch->ndxNext != patches.InvalidIndex() )
						{
							pNextPatch = &patches.Element( patch->ndxNext );
						}

						if( patch->child1 == patches.InvalidIndex() )
						{
							patchnum = patch - patches.Base();												

							nPatches++;
						}					
					}									
				}

				return nPatches * 2;
			}

			struct Data
			{
				UINT iPatch, nTransfers;
			} m_data;

			virtual void PrepareResultBuffer( UINT iFace, UINT iBuffer, LPVOID* ppBuffer, UINT* Size ) 
			{
				UINT Index = iBuffer / 2;
				UINT Detail = iBuffer % 2;

				*Size = 0;
				*ppBuffer = NULL;				

				if (facePatches.Element( iFace ) != facePatches.InvalidIndex())
				{
					patch_t*	patch;
					patch_t*	pNextPatch;
					unsigned	patchnum;

					// light every patch in the Face
					patch = &patches.Element( facePatches.Element( iFace ) );					

					for( ; patch ; patch = pNextPatch  )
					{
						pNextPatch = NULL;
						if( patch->ndxNext != patches.InvalidIndex() )
						{
							pNextPatch = &patches.Element( patch->ndxNext );
						}

						if( patch->child1 == patches.InvalidIndex() )
						{
							patchnum = patch - patches.Base();												

							if (Index-- == 0)
							{
								switch (Detail)
								{
								case 0 :
									m_data.iPatch = patchnum;
									m_data.nTransfers = patch->numtransfers;

									*ppBuffer = &m_data;
									*Size = sizeof(Data);
									break;
								case 1 :
									*ppBuffer = patch->transfers;
									*Size = sizeof(transfer_t) * patch->numtransfers;
									break;
								}
								return;
							}
						}					
					}
				}
			}						

			virtual void DiscardResultBuffer( UINT iBuffer ) 
			{
			}

			virtual void UpdateResult( UINT iWork, UINT iBuffer, LPVOID pBuffer, UINT Size ) 
			{					
				UINT Detail = iBuffer % 2;

				switch (Detail)
				{
				case 0 :
					memcpy( &m_data, pBuffer, sizeof(Data) );
					break;

				case 1 :
					{
						patch_t& p = patches.Element( m_data.iPatch );
						p.numtransfers = m_data.nTransfers;
						if (p.numtransfers)
						{
							//ASSERT( sizeof(transfer_t) * p.numtransfers == Size );
							p.transfers = (transfer_t*)malloc( Size );
							memcpy( p.transfers, pBuffer, Size );

							total_transfer += p.numtransfers;

							if (p.numtransfers > max_transfer)
							{
								max_transfer = p.numtransfers;
							}
						}
					}
					break;
				}
			}
		};

		transfer_t *transfers = BuildVisLeafs_Start();
		
		threads.SetNumThreads( numthreads );
		threads.MPI_Run( "BuildVisMatrix", numfaces, BuildVisLeafs3, transfers, &VisMatrixMPIInterface(), true );
	}
	else 
	{
		//!{ 2006-03-22	허 창 민
		//RunThreadsOn (dvis->numclusters, true, BuildVisLeafs);
		RunThreadsOn (numfaces, true, BuildVisLeafs2);
		//!} 2006-03-22	허 창 민

	}
}

void FreeVisMatrix (void)
{

}
