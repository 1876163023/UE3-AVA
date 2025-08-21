/*=============================================================================
	DynamicMeshBuilder.h: Dynamic mesh builder definitions.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** The vertex type used for dynamic meshes. */
struct FDynamicMeshVertex
{
	FDynamicMeshVertex() {}
	FDynamicMeshVertex( const FVector& InPosition ):
		Position(InPosition),
		TextureCoordinate(FVector2D(0,0)),
		TangentX(FVector(1,0,0)),
		TangentY(FVector(0,1,0)),
		TangentZ(FVector(0,0,1))
	{}

	FDynamicMeshVertex(const FVector& InPosition,const FVector& InTangentX,const FVector& InTangentY,const FVector& InTangentZ,const FVector2D& InTexCoord):
		Position(InPosition),
		TextureCoordinate(InTexCoord),
		TangentX(InTangentX),
		TangentY(InTangentY),
		TangentZ(InTangentZ)
	{}

	FVector Position;
	FVector2D TextureCoordinate;
	FPackedNormal TangentX;
	FPackedNormal TangentY;
	FPackedNormal TangentZ;
};

/**
 * A utility used to construct dynamically generated meshes, and render them to a FPrimitiveDrawInterface.
 * Note: This is meant to be easy to use, not fast.  It moves the data around more than necessary, and requires dynamically allocating RHI
 * resources.  Exercise caution.
 */
class FDynamicMeshBuilder
{
public:

	/** Initialization constructor. */
	FDynamicMeshBuilder();

	/** Destructor. */
	~FDynamicMeshBuilder();

	/** Adds a vertex to the mesh. */
	INT AddVertex(
		const FVector& InPosition,
		const FVector2D& InTextureCoordinate,
		const FVector& InTangentX,
		const FVector& InTangentY,
		const FVector& InTangentZ
		);

	/** Adds a vertex to the mesh. */
	INT AddVertex(const FDynamicMeshVertex &InVertex);

	/** Adds a triangle to the mesh. */
	void AddTriangle(INT V0,INT V1,INT V2);

	/** Adds many vertices to the mesh. */
	INT AddVertices(const TArray<FDynamicMeshVertex> &InVertices);

	/** Add many indices to the mesh. */
	void AddTriangles(const TArray<INT> &InIndices);

	/**
	 * Draws the mesh to the given primitive draw interface.
	 * @param PDI - The primitive draw interface to draw the mesh on.
	 * @param LocalToWorld - The local to world transform to apply to the vertices of the mesh.
	 * @param MaterialInstance - The material instance to render on the mesh.
	 * @param DepthPriorityGroup - The depth priority group to render the mesh in.
	 */
	void Draw(FPrimitiveDrawInterface* PDI,const FMatrix& LocalToWorld,const FMaterialInstance* MaterialInstance,BYTE DepthPriorityGroup);

private:
	class FDynamicMeshIndexBuffer* IndexBuffer;
	class FDynamicMeshVertexBuffer* VertexBuffer;
};

struct FDynamicMeshContext
{
	void* VertexBuffer;
	void* IndexBuffer;
	void* VertexLightmapBuffer;

	/* const */
	UINT MaxVertices;
	UINT MaxIndices;

	UINT VertexOffset;
	UINT IndexOffset;

	UINT VertexSize;
	UINT IndexSize;

	UINT NumVertices;
	UINT NumIndices;	

	UINT MinVertexIndex;
	UINT MaxVertexIndex;
	UBOOL bUseDynamicVertexBuffer;

	template <typename T>
		FORCEINLINE T& CurrentVertex() 
	{
		return *(T*)(((char*)VertexBuffer) + VertexSize * NumVertices);
	}

	template <typename T>
		FORCEINLINE T& CurrentIndex() 
	{
		return *(T*)(((char*)IndexBuffer) + IndexSize * NumIndices);
	}

	FORCEINLINE void AdvanceVertex()
	{
		checkSlow( NumVertices < MaxVertices );
		NumVertices++;
	}

	FORCEINLINE void AdvanceIndex()
	{
		checkSlow( NumIndices < MaxIndices );
		NumIndices++;
	}

	FORCEINLINE UBOOL CheckAvail( UINT RequestVertices, UINT RequestIndices ) const
	{
		return 
			RequestVertices <= MaxVertices - NumVertices &&
			RequestIndices <= MaxIndices - NumIndices;
	}

	FORCEINLINE UBOOL CheckAvailIndices( UINT RequestIndices ) const
	{
		return RequestIndices <= (MaxIndices - NumIndices);
	}

	void GenerateMeshElement( FMeshElement& MeshElement, FVertexFactory& VertexFactory );

	void DrawIndexedPrimitive( FCommandContextRHI* Context, UINT PrimitiveType = PT_TriangleList );
};

/* user function */
// @param NumVertices	Number of vertices *should* be allocated
// @param NumIndices	Number of indices *should* be allocated
// @remark				on failure, Context.MaxIndices == 0; otherwise Context.MaxIndices >= NumIndices
//						on failure, Context.MaxVertices == 0; otherwise Context.MaxVertices >= NumVertices
//						on failure, Context.VertexBuffer == NULL && Context.IndexBuffer == NULL
void AllocDynamicMesh( FDynamicMeshContext& Context, UINT VertexSize, UINT NumVertices, UINT IndexSize, UINT NumIndices, UBOOL bUseVertexLightmap = FALSE );
void CommitDynamicMesh( FDynamicMeshContext& Context );
UBOOL IsAffordableForDynamicMesh( INT VertexChunkSize, INT IndexChunkSize, INT VertexLightmapChunkSize );
FVertexBuffer* DynamicMesh_GetVertexBuffer();