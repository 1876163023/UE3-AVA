/*=============================================================================
	DynamicMeshBuilder.cpp: Dynamic mesh builder implementation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

UBOOL GDoNotUseHarewareDynamicBuffers = FALSE;
UBOOL GUseEventQuery = TRUE;

extern void RHIIssueEventQuery(FCommandContextRHI* Context,FOcclusionQueryRHIParamRef OcclusionQuery);
extern FOcclusionQueryRHIRef RHICreateEventQuery();
extern UBOOL RHIGetEventQueryResult(FOcclusionQueryRHIParamRef OcclusionQuery);

/** The index buffer type used for dynamic meshes. */
class FDynamicMeshIndexBuffer : public FDynamicPrimitiveResource, public FIndexBuffer
{
public:

	TArray<INT> Indices;

	// FRenderResource interface.
	virtual void InitRHI()
	{
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(INT),Indices.Num() * sizeof(INT),NULL,FALSE);

		// Write the indices to the index buffer.
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI,0,Indices.Num() * sizeof(INT));
		appMemcpy(Buffer,&Indices(0),Indices.Num() * sizeof(INT));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}

	// FDynamicPrimitiveResource interface.
	virtual void InitPrimitiveResource()
	{
		Init();
	}

	virtual void ReleasePrimitiveResource()
	{
		Release();
		delete this;
	}
};

/** The vertex buffer type used for dynamic meshes. */
class FDynamicMeshVertexBuffer : public FDynamicPrimitiveResource, public FVertexBuffer
{
public:

	TArray<FDynamicMeshVertex> Vertices;	

	// FResourceResource interface.
	virtual void InitRHI()
	{
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FDynamicMeshVertex),NULL,FALSE);

		// Copy the vertex data into the vertex buffer.
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI,0,Vertices.Num() * sizeof(FDynamicMeshVertex));
		appMemcpy(VertexBufferData,&Vertices(0),Vertices.Num() * sizeof(FDynamicMeshVertex));
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}

	// FDynamicPrimitiveResource interface.
	virtual void InitPrimitiveResource()
	{
		Init();
	}

	virtual void ReleasePrimitiveResource()
	{
		Release();
		delete this;
	}
};

/** The vertex factory type used for dynamic meshes. */
class FDynamicMeshVertexFactory : public FDynamicPrimitiveResource, public FLocalVertexFactory
{
public:

	/** Initialization constructor. */
	FDynamicMeshVertexFactory(const FDynamicMeshVertexBuffer* VertexBuffer)
	{
		// Initialize the vertex factory's stream components.
		TSetResourceDataContext<FLocalVertexFactory> SetDataContext(this);
		SetDataContext->PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,Position,VET_Float3);
		SetDataContext->TextureCoordinates.AddItem(
			FVertexStreamComponent(VertexBuffer,STRUCT_OFFSET(FDynamicMeshVertex,TextureCoordinate),sizeof(FDynamicMeshVertex),VET_Float2)
			);
		SetDataContext->TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,TangentX,VET_PackedNormal);
		SetDataContext->TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,TangentY,VET_PackedNormal);
		SetDataContext->TangentBasisComponents[2] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,TangentZ,VET_PackedNormal);
	}

	// FDynamicPrimitiveResource interface.
	virtual void InitPrimitiveResource()
	{
		Init();
	}

	virtual void ReleasePrimitiveResource()
	{
		Release();
		delete this;
	}
};

FDynamicMeshBuilder::FDynamicMeshBuilder()
{
	VertexBuffer = new FDynamicMeshVertexBuffer;
	IndexBuffer = new FDynamicMeshIndexBuffer;
}

FDynamicMeshBuilder::~FDynamicMeshBuilder()
{
	delete VertexBuffer;
	delete IndexBuffer;
}

INT FDynamicMeshBuilder::AddVertex(
	const FVector& InPosition,
	const FVector2D& InTextureCoordinate,
	const FVector& InTangentX,
	const FVector& InTangentY,
	const FVector& InTangentZ
	)
{
	INT VertexIndex = VertexBuffer->Vertices.Num();
	FDynamicMeshVertex* Vertex = new(VertexBuffer->Vertices) FDynamicMeshVertex;
	Vertex->Position = InPosition;
	Vertex->TextureCoordinate = InTextureCoordinate;
	Vertex->TangentX = InTangentX;
	Vertex->TangentY = InTangentY;
	Vertex->TangentZ = InTangentZ;
	return VertexIndex;
}

/** Adds a vertex to the mesh. */
INT FDynamicMeshBuilder::AddVertex(const FDynamicMeshVertex &InVertex)
{
	INT VertexIndex = VertexBuffer->Vertices.Num();
	FDynamicMeshVertex* Vertex = new(VertexBuffer->Vertices) FDynamicMeshVertex(InVertex);

	return VertexIndex;
}

/** Adds a triangle to the mesh. */
void FDynamicMeshBuilder::AddTriangle(INT V0,INT V1,INT V2)
{
	IndexBuffer->Indices.AddItem(V0);
	IndexBuffer->Indices.AddItem(V1);
	IndexBuffer->Indices.AddItem(V2);
}

/** Adds many vertices to the mesh. Returns start index of verts in the overall array. */
INT FDynamicMeshBuilder::AddVertices(const TArray<FDynamicMeshVertex> &InVertices)
{
	INT StartIndex = VertexBuffer->Vertices.Num();
	VertexBuffer->Vertices.Append(InVertices);
	return StartIndex;
}

/** Add many indices to the mesh. */
void FDynamicMeshBuilder::AddTriangles(const TArray<INT> &InIndices)
{
	IndexBuffer->Indices.Append(InIndices);
}

void FDynamicMeshBuilder::Draw(FPrimitiveDrawInterface* PDI,const FMatrix& LocalToWorld,const FMaterialInstance* MaterialInstance,BYTE DepthPriorityGroup)
{
	// Only draw non-empty meshes.
	if(VertexBuffer->Vertices.Num() > 0 && IndexBuffer->Indices.Num() > 0)
	{
		// Register the dynamic resources with the PDI.
		PDI->RegisterDynamicResource(VertexBuffer);
		PDI->RegisterDynamicResource(IndexBuffer);

		// Create the vertex factory.
		FDynamicMeshVertexFactory* VertexFactory = new FDynamicMeshVertexFactory(VertexBuffer);
		PDI->RegisterDynamicResource(VertexFactory);

		// Draw the mesh.
		FMeshElement Mesh;
		Mesh.IndexBuffer = IndexBuffer;
		Mesh.VertexFactory = VertexFactory;
		Mesh.MaterialInstance = MaterialInstance;
		Mesh.LocalToWorld = LocalToWorld;
		Mesh.WorldToLocal = LocalToWorld.Inverse();
		// previous l2w not used so treat as static		
		Mesh.FirstIndex = 0;
		Mesh.NumPrimitives = IndexBuffer->Indices.Num() / 3;
		Mesh.MinVertexIndex = 0;
		Mesh.MaxVertexIndex = VertexBuffer->Vertices.Num() - 1;
		Mesh.ReverseCulling = LocalToWorld.Determinant() < 0.0f ? TRUE : FALSE;
		Mesh.Type = PT_TriangleList;
		Mesh.DepthPriorityGroup = DepthPriorityGroup;
		PDI->DrawMesh(Mesh);

		// Clear the resource pointers so they cannot be overwritten accidentally.
		VertexBuffer = NULL;
		IndexBuffer = NULL;
	}
}


#define VERTEXBUFFERSIZE_KB 512
#define INDEXBUFFERSIZE_KB 256
#define VERTEXLIGHTMAPBUFFERSIZE_KB (VERTEXBUFFERSIZE_KB / 2)
#define VertexBufferSize (1024 * VERTEXBUFFERSIZE_KB)
#define IndexBufferSize (1024 * INDEXBUFFERSIZE_KB)
#define VertexLightmapBufferSize (1024 * VERTEXLIGHTMAPBUFFERSIZE_KB)

struct FDynamicVertexBuffer : FVertexBuffer
{
	virtual void InitDynamicRHI()
	{
		VertexBufferRHI = RHICreateVertexBuffer( VertexBufferSize, NULL, TRUE );
	}

	// FRenderResource interface.
	virtual void ReleaseDynamicRHI()
	{
		VertexBufferRHI.Release();
	}
};

struct FDynamicVertexLightmapBuffer : FVertexBuffer
{
	virtual void InitDynamicRHI()
	{
		VertexBufferRHI = RHICreateVertexBuffer( VertexLightmapBufferSize, NULL, TRUE );
	}

	// FRenderResource interface.
	virtual void ReleaseDynamicRHI()
	{
		VertexBufferRHI.Release();
	}
};

struct FDynamicIndexBuffer : FIndexBuffer
{
	virtual void InitDynamicRHI()
	{
		IndexBufferRHI = RHICreateIndexBuffer( sizeof(WORD), IndexBufferSize, NULL, TRUE );
	}

	// FRenderResource interface.
	virtual void ReleaseDynamicRHI()
	{
		IndexBufferRHI.Release();
	}
};

struct FDynamicIndexBuffer32 : FIndexBuffer
{
	virtual void InitDynamicRHI()
	{
		IndexBufferRHI = RHICreateIndexBuffer( sizeof(DWORD), IndexBufferSize, NULL, TRUE );
	}

	// FRenderResource interface.
	virtual void ReleaseDynamicRHI()
	{
		IndexBufferRHI.Release();
	}
};

#define NUM_ROUNDROBIN_INIT_NUM				2
#define NUM_ROUNDROBIN_MAX_DYNAMICBUFFERS	8

template <class T>
class TRoundRobin
{
public :
	template <class T>
	struct TBuffer
	{
	public :
		TBuffer()
			: bBusy(FALSE)
		{
		}

		T Resource;
		FOcclusionQueryRHIRef Query;
		UBOOL bBusy;

		void InitDynamicRHI()
		{
			bBusy = FALSE;

			Resource.InitDynamicRHI();

			Query = RHICreateEventQuery();
		}

		void ReleaseDynamicRHI()
		{
			Resource.ReleaseDynamicRHI();

			Query = NULL;
		}

		void Issue()
		{
			if (!GUseEventQuery) 
				return;

			bBusy = TRUE;

			RHIIssueEventQuery( NULL, Query );
		}

		UBOOL CheckHealth()
		{
			if (!bBusy) return TRUE;

			if (RHIGetEventQueryResult( Query ))
			{
				bBusy = FALSE;
				return TRUE;
			}

			return FALSE;
		}
	};	

	TRoundRobin()
		: Index(0)
	{
	}

	void InitDynamicRHI()
	{
		Index = 0;		

		const INT Num = GDoNotUseHarewareDynamicBuffers ? NUM_ROUNDROBIN_INIT_NUM : 1;
		
		for (INT i=0; i<Num; ++i)
		{
			Buffer.AddItem( new TBuffer<T>() );
			Buffer(i)->InitDynamicRHI();			
		}
	}

	void ReleaseDynamicRHI()
	{
		for (INT i=0; i<Buffer.Num(); ++i)
		{
			Buffer(i)->ReleaseDynamicRHI();
			delete Buffer(i);			
		}

		Buffer.Empty();
	}

	void Advance()
	{
		// 일단 원래 거가 바쁘니까;
		Buffer(Index)->Issue();

		for (;;)
		{
			const INT Num = Buffer.Num();
			for (INT i=0; i<Num; ++i)
			{
				// 같은 건 안쓴다!!
				INT Candidate = (i + Index + 1) % Num;
				if (Buffer(Candidate)->CheckHealth())
				{
					Index = Candidate;
					return;
				}
			}

			if (Buffer.Num() < NUM_ROUNDROBIN_MAX_DYNAMICBUFFERS)
			{
				TBuffer<T>* NewBuffer = new TBuffer<T>;
				NewBuffer->InitDynamicRHI();
				Index = Buffer.AddItem( NewBuffer );

				debugf( NAME_PerfWarning, TEXT("Dynamic buffer size was increased to %d" ), Buffer.Num() );

				return;
			}			
		}		
	}

	T* operator -> ()
	{
		return &Buffer(Index)->Resource;
	}

	operator T* ()
	{
		return &Buffer(Index)->Resource;
	}

	T& operator [] ( const INT X )
	{
		return Buffer[X]->Resource;
	}

	T& operator * ()
	{
		return Buffer(Index)->Resource;
	}

	UBOOL CanAppend( UBOOL bShouldDiscard )
	{
		if (!GDoNotUseHarewareDynamicBuffers)
			return !bShouldDiscard;

		if (!bShouldDiscard)
			return TRUE;
		
		Advance();
		return TRUE;
	}

	TArray< TBuffer<typename T>* > Buffer;
	INT Index;
};

class DynamicMeshBuffers : public FRenderResource
{
public :
	TRoundRobin<FDynamicVertexBuffer>			VertexBuffer;
	TRoundRobin<FDynamicVertexLightmapBuffer>	VertexLightmapBuffer;	
	TRoundRobin<FDynamicIndexBuffer>			IndexBuffer16;
	TRoundRobin<FDynamicIndexBuffer32>			IndexBuffer32;
	FIndexBuffer*								IndexBuffer;	
	
	UINT VertexBufferPosition, IndexBufferPosition;
	UINT Index16Pos, Index32Pos;

	UBOOL bIsLocked;

	DynamicMeshBuffers()
		: bIsLocked(FALSE), VertexBufferPosition(0), IndexBufferPosition(0), IndexBuffer( NULL ), Index16Pos(0), Index32Pos(0)
	{}

	~DynamicMeshBuffers()
	{
		check( !bIsLocked );
	}

	virtual void InitDynamicRHI()
	{
		VertexBuffer.InitDynamicRHI();
		IndexBuffer16.InitDynamicRHI();
		IndexBuffer32.InitDynamicRHI();
		VertexLightmapBuffer.InitDynamicRHI();				
	}

	virtual void ReleaseDynamicRHI()
	{		
		VertexBuffer.ReleaseDynamicRHI();
		IndexBuffer16.ReleaseDynamicRHI();
		IndexBuffer32.ReleaseDynamicRHI();
		VertexLightmapBuffer.ReleaseDynamicRHI();		
	}

	void InitForNewFrame()
	{
		VertexBufferPosition = VertexBufferSize;
		IndexBufferPosition = IndexBufferSize;
		Index16Pos = IndexBufferSize;
		Index32Pos = IndexBufferSize;
	}

	void Lock( FDynamicMeshContext& Context, UINT VertexSize, UINT NumVertices, UINT IndexSize, UINT NumIndices, UBOOL bUseVertexLightmap )
	{
		check( IndexSize == sizeof(WORD) || IndexSize == sizeof(DWORD));
		check( !bIsLocked );		

		Context.bUseDynamicVertexBuffer = ( VertexSize > 0 );

		const UINT VertexColorSize = sizeof(FQuantizedLightSample);

		const UINT VertexCapacity = Context.bUseDynamicVertexBuffer ? 
			(bUseVertexLightmap ? 
				Min(VertexBufferSize / VertexSize, VertexLightmapBufferSize / VertexColorSize) 
				: VertexBufferSize / VertexSize) 
			: 0;		
		const UINT IndexCapacity = ( IndexBufferSize / IndexSize );

		if (NumIndices > IndexCapacity || NumVertices > VertexCapacity)
		{
			Context.VertexBuffer = NULL;
			Context.IndexBuffer = NULL;
			Context.MaxVertices = 0;
			Context.MaxIndices = 0;

			return;
		}

		if( NumIndices > 0 )
		{
			if ( IndexSize == sizeof(WORD) )
			{
				IndexBuffer = IndexBuffer16;
			}
			else
			{
				IndexBuffer = IndexBuffer32;
			}
		}
		else
		{
			IndexBuffer = NULL;
		}		
		
		// vertex lightmap info.				

		if( Context.bUseDynamicVertexBuffer )
		{
			Context.VertexOffset = (VertexBufferPosition + VertexSize - 1) / VertexSize;
			VertexBufferPosition = Context.VertexOffset * VertexSize;
			Context.MaxVertices	 = ( VertexCapacity >= Context.VertexOffset) ? ( VertexCapacity - Context.VertexOffset) : 0;
		}
		else
		{
			// init min/max
			Context.MinVertexIndex = UINT_MAX;
			Context.MaxVertexIndex = 0;
		}

		// index buffer info.		
		//Context.IndexOffset	= (IndexBufferPosition + IndexSize - 1) / IndexSize;
		Context.IndexOffset = (IndexSize == sizeof(WORD) ) ? (Index16Pos + IndexSize - 1) / IndexSize : ( Index32Pos + IndexSize- 1 ) / IndexSize;
		Context.MaxIndices	= ( IndexCapacity >= Context.IndexOffset) ? ( IndexCapacity - Context.IndexOffset) : 0;		

		/* insufficient? */
		const UBOOL bDiscardVertexBuffer = Context.MaxVertices < NumVertices;
		if (bDiscardVertexBuffer)
		{
			VertexBufferPosition = 0;			
			Context.VertexOffset = 0;			
			Context.MaxVertices = VertexCapacity;			
			check(Context.MaxVertices >= NumVertices);
		}		

		/* insufficient? */
		const UBOOL bDiscardIndexBuffer = Context.MaxIndices < NumIndices;
		if (bDiscardIndexBuffer)
		{			
			IndexBufferPosition = 0;
			if( IndexSize == sizeof(WORD) )
			{
				Index16Pos = 0;
			}
			else
			{
				Index32Pos = 0;
			}
			Context.IndexOffset = 0;
			Context.MaxIndices = IndexCapacity;
			check(Context.MaxIndices >= NumIndices);
		}				

		Context.VertexSize = VertexSize;
		Context.IndexSize = IndexSize;

		// initialize batch info
		Context.NumVertices = 0;
		Context.NumIndices = 0;

		Context.VertexBuffer = NULL;
		if( Context.bUseDynamicVertexBuffer )
		{
			Context.VertexBuffer = RHILockVertexBuffer( VertexBuffer->VertexBufferRHI, Context.VertexOffset * VertexSize, Context.MaxVertices * VertexSize, 
				VertexBuffer.CanAppend( bDiscardVertexBuffer ) );
		}		

		Context.IndexBuffer = NULL;
		if (NumIndices > 0)
		{
			Context.IndexBuffer = RHILockIndexBuffer( IndexBuffer->IndexBufferRHI, Context.IndexOffset * IndexSize, Context.MaxIndices * IndexSize, 
				IndexSize == sizeof(WORD) ? 
					IndexBuffer16.CanAppend( bDiscardIndexBuffer ) :
					IndexBuffer32.CanAppend( bDiscardIndexBuffer ) );
		}

		Context.VertexLightmapBuffer = NULL;
		if (bUseVertexLightmap)
		{
			Context.VertexLightmapBuffer = RHILockVertexBuffer( VertexLightmapBuffer->VertexBufferRHI, Context.VertexOffset * VertexColorSize, Context.MaxVertices * VertexColorSize, 
				VertexLightmapBuffer.CanAppend( bDiscardVertexBuffer ) );
		}

		bIsLocked = TRUE;
	}

	void Unlock( FDynamicMeshContext& Context )
	{
		check( bIsLocked );

		if( Context.bUseDynamicVertexBuffer )
		{
			VertexBufferPosition += Context.NumVertices * Context.VertexSize;
			RHIUnlockVertexBuffer( VertexBuffer->VertexBufferRHI );
		}

		//IndexBufferPosition += Context.NumIndices * Context.IndexSize;
		if( Context.IndexSize == sizeof(WORD) )
		{
			Index16Pos += Context.NumIndices * Context.IndexSize;
		}
		else
		{
			Index32Pos += Context.NumIndices * Context.IndexSize;
		}

		if (Context.IndexBuffer != NULL)
		{
			RHIUnlockIndexBuffer( IndexBuffer->IndexBufferRHI );						
		}

		if (Context.VertexLightmapBuffer != NULL)
		{
			RHIUnlockVertexBuffer( VertexLightmapBuffer->VertexBufferRHI );			
		}

		bIsLocked = FALSE;
	}
};

TGlobalResource<DynamicMeshBuffers> GDynamicMeshBuffers;

// decal batch는 다른 batch와 섞이면... 깜박이는 현상이 나타난다...
// 일단 한 프레임 내에서 decal batch가 시작되면, buffer를 flush하여, 임시로 해결한다.
// 2007.6.26 changmin
UBOOL GFirstDecalProcessed = FALSE;
void AllocDynamicMesh( FDynamicMeshContext& Context, UINT VertexSize, UINT NumVertices, UINT IndexSize, UINT NumIndices, UBOOL bUseVertexLightmap )
{
	const UBOOL bFirstDecalInThisFrame = bUseVertexLightmap && !GFirstDecalProcessed;
	if( bFirstDecalInThisFrame )
	{
		GDynamicMeshBuffers.InitForNewFrame();
		GFirstDecalProcessed = TRUE;
	}
	GDynamicMeshBuffers.Lock( Context, VertexSize, NumVertices, IndexSize, NumIndices, bUseVertexLightmap );
}

void CommitDynamicMesh( FDynamicMeshContext& Context )
{
	GDynamicMeshBuffers.Unlock( Context );
}


void InitDynamicMeshForNewFrame()
{
	GFirstDecalProcessed = FALSE;
	GDynamicMeshBuffers.InitForNewFrame();
}

FVertexBuffer* DynamicMesh_GetVertexBuffer()
{
	return GDynamicMeshBuffers.VertexBuffer;
}

void FDynamicMeshContext::GenerateMeshElement( FMeshElement& MeshElement, FVertexFactory& VertexFactory )
{
	if( bUseDynamicVertexBuffer )
	{			
		VertexFactory.Init();		
		VertexFactory.SwitchVertexBuffer( *GDynamicMeshBuffers.VertexBuffer );
		
		MeshElement.VertexFactory = &VertexFactory;
		MeshElement.BaseVertexIndex = VertexOffset;
		MeshElement.MinVertexIndex = 0;
		MeshElement.MaxVertexIndex = NumVertices - 1;
	}
	else
	{
		MeshElement.VertexFactory = &VertexFactory;
		MeshElement.MinVertexIndex = MinVertexIndex;
		MeshElement.MaxVertexIndex = MaxVertexIndex;
	}
	
	MeshElement.Type		= PT_TriangleList;
	MeshElement.IndexBuffer = GDynamicMeshBuffers.IndexBuffer;
	MeshElement.FirstIndex	= IndexOffset;
	MeshElement.NumPrimitives = NumIndices / 3;

	if (VertexLightmapBuffer && MeshElement.LCI)
	{
		/// link to rhi :)
		FLightMap1D* Lightmap1D = const_cast<FLightMap1D*>( MeshElement.LCI->GetLightMap()->GetLightMap1D() );
		
		check( Lightmap1D != NULL );

		Lightmap1D->VertexBufferRHI = GDynamicMeshBuffers.VertexLightmapBuffer->VertexBufferRHI;
	}
}

void FDynamicMeshContext::DrawIndexedPrimitive( FCommandContextRHI* Context, UINT PrimitiveType )
{
	RHISetStreamSource( Context, 0, GDynamicMeshBuffers.VertexBuffer->VertexBufferRHI, VertexSize );

	INT NumPrimitives = 0;


	switch (PrimitiveType)
	{
	case PT_TriangleList :
		NumPrimitives = NumIndices / 3;
		break;
	case PT_TriangleFan :
		NumPrimitives = NumIndices - 2;
		break;
	case PT_TriangleStrip :
		NumPrimitives = NumIndices - 2;
		break;
	case PT_LineList :
		NumPrimitives = NumIndices / 2;
		break;
	case PT_QuadList :
		NumPrimitives = NumIndices / 4;
		break;
	}

	RHIDrawIndexedPrimitive( 
		Context, 
		GDynamicMeshBuffers.IndexBuffer->IndexBufferRHI, 
		PrimitiveType, 
		VertexOffset, 
		0,
		NumVertices,
		IndexOffset,
		NumPrimitives);
}

UBOOL IsAffordableForDynamicMesh( INT VertexChunkSize, INT IndexChunkSize, INT VertexLightmapChunkSize )
{
	return VertexChunkSize <= VertexBufferSize && IndexChunkSize <= IndexBufferSize && VertexLightmapChunkSize <= VertexLightmapBufferSize;
}

