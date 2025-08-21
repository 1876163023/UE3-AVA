/*=============================================================================
	UnStaticMesh.cpp: Static mesh class implementation.
	Copyright 1997-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"
#include "EnginePhysicsClasses.h"
#include "UnStaticMeshLegacy.h"

IMPLEMENT_CLASS(AStaticMeshActorBase);
IMPLEMENT_CLASS(AStaticMeshActor);
IMPLEMENT_CLASS(AStaticMeshCollectionActor);
IMPLEMENT_CLASS(ADynamicSMActor);
IMPLEMENT_CLASS(UStaticMesh);

/*-----------------------------------------------------------------------------
FStaticMeshTriangleBulkData (FStaticMeshTriangle version of bulk data)
-----------------------------------------------------------------------------*/

/**
 * Returns size in bytes of single element.
 *
 * @return Size in bytes of single element
 */
INT FStaticMeshTriangleBulkData::GetElementSize() const
{
	return sizeof(FStaticMeshTriangle);
}

/**
 * Serializes an element at a time allowing and dealing with endian conversion and backward compatiblity.
 *
 * @warning BulkSerialize: FStaticMeshTriangle is serialized as memory dump
 * See TArray::BulkSerialize for detailed description of implied limitations.
 * 
 * @param Ar			Archive to serialize with
 * @param Data			Base pointer to data
 * @param ElementIndex	Element index to serialize
 */
void FStaticMeshTriangleBulkData::SerializeElement( FArchive& Ar, void* Data, INT ElementIndex )
{
	FStaticMeshTriangle& StaticMeshTriangle = *((FStaticMeshTriangle*)Data + ElementIndex);
	if( Ar.Ver() < VER_REPLACED_LAZY_ARRAY_WITH_UNTYPED_BULK_DATA )
	{
		Ar << StaticMeshTriangle.Vertices[0];
		Ar << StaticMeshTriangle.Vertices[1];
		Ar << StaticMeshTriangle.Vertices[2];
		Ar << StaticMeshTriangle.NumUVs;

		for( INT UVIndex=0; UVIndex<StaticMeshTriangle.NumUVs; UVIndex++ )
		{
			Ar << StaticMeshTriangle.UVs[0][UVIndex];
			Ar << StaticMeshTriangle.UVs[1][UVIndex];
			Ar << StaticMeshTriangle.UVs[2][UVIndex];
		}

		Ar << StaticMeshTriangle.Colors[0];
		Ar << StaticMeshTriangle.Colors[1];
		Ar << StaticMeshTriangle.Colors[2];

		Ar << StaticMeshTriangle.MaterialIndex;
		Ar << StaticMeshTriangle.SmoothingMask;
	}
	else
	{
		Ar << StaticMeshTriangle.Vertices[0];
		Ar << StaticMeshTriangle.Vertices[1];
		Ar << StaticMeshTriangle.Vertices[2];

		if( Ar.Ver() < VER_FIXED_TRIANGLE_BULK_DATA_SERIALIZATION )
		{
			for( INT UVIndex=0; UVIndex<8; UVIndex++ )
			{
				Ar << StaticMeshTriangle.UVs[0][UVIndex];
				Ar << StaticMeshTriangle.UVs[1][UVIndex];
				Ar << StaticMeshTriangle.UVs[2][UVIndex];
			}
		}
		else
		{
			for( INT VertexIndex=0; VertexIndex<3; VertexIndex++ )
			{
				for( INT UVIndex=0; UVIndex<8; UVIndex++ )
				{
					Ar << StaticMeshTriangle.UVs[VertexIndex][UVIndex];
				}
			}
		}

		Ar << StaticMeshTriangle.Colors[0];
		Ar << StaticMeshTriangle.Colors[1];
		Ar << StaticMeshTriangle.Colors[2];
		Ar << StaticMeshTriangle.MaterialIndex;
		Ar << StaticMeshTriangle.SmoothingMask;
		Ar << StaticMeshTriangle.NumUVs;
	}
}

/**
 * Returns whether single element serialization is required given an archive. This e.g.
 * can be the case if the serialization for an element changes and the single element
 * serialization code handles backward compatibility.
 */
UBOOL FStaticMeshTriangleBulkData::RequiresSingleElementSerialization( FArchive& Ar )
{
	if( Ar.Ver() < VER_FIXED_TRIANGLE_BULK_DATA_SERIALIZATION )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/** The implementation of the static mesh position-only vertex data storage type. */
class FPositionVertexData :
	public FStaticMeshVertexDataInterface,
	public TResourceArray<FPositionVertex,TRUE,VERTEXBUFFER_ALIGNMENT>
{
public:

	typedef TResourceArray<FPositionVertex,TRUE,VERTEXBUFFER_ALIGNMENT> ArrayType;

	virtual void ResizeBuffer(UINT NumVertices)
	{
		if((UINT)ArrayType::Num() < NumVertices)
		{
			// Enlarge the array.
			ArrayType::Add(NumVertices - ArrayType::Num());
		}
		else if((UINT)ArrayType::Num() > NumVertices)
		{
			// Shrink the array.
			ArrayType::Remove(NumVertices,ArrayType::Num() - NumVertices);
		}
	}
	virtual UINT GetStride() const
	{
		return sizeof(FPositionVertex);
	}
	virtual BYTE* GetDataPointer()
	{
		return (BYTE*)&(*this)(0);
	}
	virtual FResourceArrayInterface* GetResourceArray()
	{
		return this;
	}
	virtual void Serialize(FArchive& Ar)
	{
		TResourceArray<FPositionVertex,TRUE,VERTEXBUFFER_ALIGNMENT>::BulkSerialize(Ar);
	}
};



FPositionVertexBuffer::FPositionVertexBuffer():
VertexData(NULL),
Data(NULL)
{}

FPositionVertexBuffer::~FPositionVertexBuffer()
{
	CleanUp();
}

/** Delete existing resources */
void FPositionVertexBuffer::CleanUp()
{
	if (VertexData)
	{
		delete VertexData;
		VertexData = NULL;
	}
}

/**
* Initializes the buffer with the given vertices, used to convert legacy layouts.
* @param InVertices - The vertices to initialize the buffer with.
*/
void FPositionVertexBuffer::Init(const FLegacyStaticMeshVertexBuffer& InVertexBuffer)
{
	NumVertices = InVertexBuffer.GetNumVertices();

	// Allocate the vertex data storage type.
	AllocateData();

	// Allocate the vertex data buffer.
	VertexData->ResizeBuffer(NumVertices);
	Data = VertexData->GetDataPointer();

	// Copy the vertices into the buffer.
	for(UINT VertexIndex = 0;VertexIndex < InVertexBuffer.GetNumVertices();VertexIndex++)
	{
		VertexPosition(VertexIndex) = InVertexBuffer.VertexPosition(VertexIndex);
	}
}

/**
* Initializes the buffer with the given vertices, used to convert legacy layouts.
* @param InVertices - The vertices to initialize the buffer with.
*/
void FPositionVertexBuffer::Init(const TArray<FStaticMeshBuildVertex>& InVertices)
{
	NumVertices = InVertices.Num();

	// Allocate the vertex data storage type.
	AllocateData();

	// Allocate the vertex data buffer.
	VertexData->ResizeBuffer(NumVertices);
	Data = VertexData->GetDataPointer();

	// Copy the vertices into the buffer.
	for(INT VertexIndex = 0;VertexIndex < InVertices.Num();VertexIndex++)
	{
		VertexPosition(VertexIndex) = InVertices(VertexIndex).Position;
	}
}

/** Serializer. */
FArchive& operator<<(FArchive& Ar,FPositionVertexBuffer& VertexBuffer)
{
	Ar << VertexBuffer.Stride << VertexBuffer.NumVertices;

	if(Ar.IsLoading())
	{
		// Allocate the vertex data storage type.
		VertexBuffer.AllocateData();
	}

	// Serialize the vertex data.
	VertexBuffer.VertexData->Serialize(Ar);

	// Make a copy of the vertex data pointer.
	VertexBuffer.Data = VertexBuffer.VertexData->GetDataPointer();

	return Ar;
}

/**
* Specialized assignment operator, only used when importing LOD's.  
*/
void FPositionVertexBuffer::operator=(const FPositionVertexBuffer &Other)
{
	//VertexData doesn't need to be allocated here because Build will be called next,
	VertexData = NULL;
}

void FPositionVertexBuffer::InitRHI()
{
	check(VertexData);
	FResourceArrayInterface* ResourceArray = VertexData->GetResourceArray();
	if(ResourceArray->GetResourceDataSize())
	{
		// Create the vertex buffer.
		VertexBufferRHI = RHICreateVertexBuffer(ResourceArray->GetResourceDataSize(),ResourceArray,FALSE);
	}
}

void FPositionVertexBuffer::AllocateData()
{
	// Clear any old VertexData before allocating.
	CleanUp();

	VertexData = new FPositionVertexData;
	// Calculate the vertex stride.
	Stride = VertexData->GetStride();
}

/*-----------------------------------------------------------------------------
FStaticMeshVertexBuffer
-----------------------------------------------------------------------------*/

/** The implementation of the static mesh vertex data storage type. */
template<UINT NumTexCoords>
class TStaticMeshVertexData :
	public FStaticMeshVertexDataInterface,
	public TResourceArray<TStaticMeshFullVertex<NumTexCoords>,TRUE,VERTEXBUFFER_ALIGNMENT>
{
public:

	typedef TResourceArray<TStaticMeshFullVertex<NumTexCoords>,TRUE,VERTEXBUFFER_ALIGNMENT> ArrayType;

	virtual void ResizeBuffer(UINT NumVertices)
	{
		if((UINT)ArrayType::Num() < NumVertices)
		{
			// Enlarge the array.
			ArrayType::Add(NumVertices - ArrayType::Num());
		}
		else if((UINT)ArrayType::Num() > NumVertices)
		{
			// Shrink the array.
			ArrayType::Remove(NumVertices,ArrayType::Num() - NumVertices);
		}
	}
	virtual UINT GetStride() const
	{
		return sizeof(TStaticMeshFullVertex<NumTexCoords>);
	}
	virtual BYTE* GetDataPointer()
	{
		return (BYTE*)&(*this)(0);
	}
	virtual FResourceArrayInterface* GetResourceArray()
	{
		return this;
	}
	virtual void Serialize(FArchive& Ar)
	{
		TResourceArray<TStaticMeshFullVertex<NumTexCoords>,TRUE,VERTEXBUFFER_ALIGNMENT>::BulkSerialize(Ar);
	}
};

FStaticMeshVertexBuffer::FStaticMeshVertexBuffer():
	VertexData(NULL),
	Data(NULL)
{}

FStaticMeshVertexBuffer::~FStaticMeshVertexBuffer()
{
	CleanUp();
}

/** Delete existing resources */
void FStaticMeshVertexBuffer::CleanUp()
{
	if (VertexData)
	{
		delete VertexData;
		VertexData = NULL;
	}
}

/**
* Initializes the buffer with the given vertices.
* @param InVertexBuffer - The legacy vertex buffer to copy data from.
*/
void FStaticMeshVertexBuffer::Init(const class FLegacyStaticMeshVertexBuffer& InVertexBuffer)
{
	NumTexCoords = InVertexBuffer.GetNumTexCoords();
	NumVertices = InVertexBuffer.GetNumVertices();

	// Allocate the vertex data storage type.
	AllocateData();

	// Allocate the vertex data buffer.
	VertexData->ResizeBuffer(NumVertices);
	Data = VertexData->GetDataPointer();

	// Copy the vertices into the buffer.
	for(UINT VertexIndex = 0;VertexIndex < InVertexBuffer.GetNumVertices();VertexIndex++)
	{
		VertexShadowExtrusionPredicate(VertexIndex) = InVertexBuffer.VertexShadowExtrusionPredicate(VertexIndex);
		VertexTangentX(VertexIndex) = InVertexBuffer.VertexTangentX(VertexIndex);
		VertexTangentY(VertexIndex) = InVertexBuffer.VertexTangentY(VertexIndex);
		VertexTangentZ(VertexIndex) = InVertexBuffer.VertexTangentZ(VertexIndex);
		//VertexColor(VertexIndex) = InVertexBuffer.VertexColor(VertexIndex);
		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			VertexUV(VertexIndex,UVIndex) = InVertexBuffer.VertexUV(VertexIndex,UVIndex);
		}
	}
}

/**
* Initializes the buffer with the given vertices.
* @param InVertices - The vertices to initialize the buffer with.
* @param InNumTexCoords - The number of texture coordinate to store in the buffer.
*/
void FStaticMeshVertexBuffer::Init(const TArray<FStaticMeshBuildVertex>& InVertices,UINT InNumTexCoords)
{
	NumTexCoords = InNumTexCoords;
	NumVertices = InVertices.Num() * 2;

	// Allocate the vertex data storage type.
	AllocateData();	

	// Allocate the vertex data buffer.
	VertexData->ResizeBuffer(NumVertices);
	Data = VertexData->GetDataPointer();

	// Copy the vertices into the buffer.
	for(INT VertexIndex = 0;VertexIndex < InVertices.Num();VertexIndex++)
	{
		const FStaticMeshBuildVertex& SourceVertex = InVertices(VertexIndex);
		for(INT Extruded = 0;Extruded < 2;Extruded++)
		{
			const UINT DestVertexIndex = (Extruded ? InVertices.Num() : 0) + VertexIndex ;
			VertexShadowExtrusionPredicate(DestVertexIndex) = Extruded ? 1.0f : 0.0f;
			VertexTangentX(DestVertexIndex) = SourceVertex.TangentX;
			VertexTangentY(DestVertexIndex) = SourceVertex.TangentY;
			VertexTangentZ(DestVertexIndex) = SourceVertex.TangentZ;
			for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
			{
				VertexUV(DestVertexIndex,UVIndex) = SourceVertex.UVs[UVIndex];
			}
		}
	}
}

/**
* Removes the cloned vertices used for extruding shadow volumes.
* @param NumVertices - The real number of static mesh vertices which should remain in the buffer upon return.
*/
void FStaticMeshVertexBuffer::RemoveShadowVolumeVertices(UINT InNumVertices)
{
	check(VertexData);
	VertexData->ResizeBuffer(InNumVertices);
	NumVertices = InNumVertices;

	// Make a copy of the vertex data pointer.
	Data = VertexData->GetDataPointer();
}

/** Serializer. */
FArchive& operator<<(FArchive& Ar,FStaticMeshVertexBuffer& VertexBuffer)
{
	Ar << VertexBuffer.NumTexCoords << VertexBuffer.Stride << VertexBuffer.NumVertices;

	if(Ar.IsLoading())
	{
		// Allocate the vertex data storage type.
		VertexBuffer.AllocateData();
	}

	// Serialize the vertex data.
	VertexBuffer.VertexData->Serialize(Ar);

	// Make a copy of the vertex data pointer.
	VertexBuffer.Data = VertexBuffer.VertexData->GetDataPointer();

	return Ar;
}

/**
* Specialized assignment operator, only used when importing LOD's.  
*/
void FStaticMeshVertexBuffer::operator=(const FStaticMeshVertexBuffer &Other)
{
	//VertexData doesn't need to be allocated here because Build will be called next,
	VertexData = NULL;
}

void FStaticMeshVertexBuffer::InitRHI()
{
	check(VertexData);
	FResourceArrayInterface* ResourceArray = VertexData->GetResourceArray();
	if(ResourceArray->GetResourceDataSize())
	{
		// Create the vertex buffer.
		VertexBufferRHI = RHICreateVertexBuffer(ResourceArray->GetResourceDataSize(),ResourceArray,FALSE);
	}
}

void FStaticMeshVertexBuffer::AllocateData()
{
	//<@ ava specific ; 2006/8/25 changmin
	// lightmap coord generate ½Ã, ±âÁ¸ÀÇ vertex data¸¦ ¾ø¾Ö°í, ´Ù½Ã »ý¼ºÇØ¾ß ÇÏ´Â °æ¿ì°¡ ÀÖ´Ù.

	// Clear any old VertexData before allocating.
	CleanUp();

	//>@ ava
	check(!VertexData);

	// Clear any old VertexData before allocating.
	switch(NumTexCoords)
	{
	case 0: VertexData = new TStaticMeshVertexData<0>; break;
	case 1: VertexData = new TStaticMeshVertexData<1>; break;
	case 2: VertexData = new TStaticMeshVertexData<2>; break;
	case 3: VertexData = new TStaticMeshVertexData<3>; break;
	case 4: VertexData = new TStaticMeshVertexData<4>; break;
	default: appErrorf(TEXT("Invalid number of texture coordinates"));
	};

	// Calculate the vertex stride.
	Stride = VertexData->GetStride();
}

/*-----------------------------------------------------------------------------
FStaticMeshRenderData
-----------------------------------------------------------------------------*/

FStaticMeshRenderData::FStaticMeshRenderData()
{
}


/**
 * Special serialize function passing the owning UObject along as required by FUnytpedBulkData
 * serialization.
 *
 * @param	Ar		Archive to serialize with
 * @param	Owner	UObject this structure is serialized within
 */
void FStaticMeshRenderData::Serialize( FArchive& Ar, UObject* Owner )
{
	if( Ar.Ver() < VER_REPLACED_LAZY_ARRAY_WITH_UNTYPED_BULK_DATA )
	{
		RawTriangles.SerializeLikeLazyArray( Ar, Owner );
	}
	else
	{
		RawTriangles.Serialize( Ar, Owner );
	}
	Ar << Elements;

	if(Ar.Ver() < VER_STATICMESH_VERTEXBUFFER_MERGE && Ar.IsLoading())
	{
		TArray<FStaticMeshBuildVertex> Vertices;

		if(Ar.Ver() < VER_USE_UMA_RESOURCE_ARRAY_MESH_DATA)
		{
			// load old vertex data
			TArray<FLegacyStaticMeshVertex> OldVertices;
			OldVertices.BulkSerialize( Ar );

			// Allocate the vertices.
			Vertices.Empty(OldVertices.Num());
			Vertices.AddZeroed(OldVertices.Num());

			// Copy the legacy vertex data into the vertex array.
			for(INT VertexIndex = 0;VertexIndex < Vertices.Num();VertexIndex++)
			{
				Vertices(VertexIndex).Position = OldVertices(VertexIndex).Position;
				Vertices(VertexIndex).TangentX = OldVertices(VertexIndex).TangentX;
				Vertices(VertexIndex).TangentY = OldVertices(VertexIndex).TangentY;
				Vertices(VertexIndex).TangentZ = OldVertices(VertexIndex).TangentZ;
			}
		}
		else
		{
			// Load legacy position/tangent vertex buffers.
			FLegacyStaticMeshPositionVertexBuffer LegacyPositionVertexBuffer;
			FLegacyStaticMeshTangentVertexBuffer LegacyTangentVertexBuffer;
			Ar << LegacyPositionVertexBuffer;
			Ar << LegacyTangentVertexBuffer;

			// Allocate the build vertices.
			Vertices.Empty(LegacyTangentVertexBuffer.Tangents.Num() / 3);
			Vertices.AddZeroed(LegacyTangentVertexBuffer.Tangents.Num() / 3);

			// Copy the legacy vertex data into the vertex array.
			for(INT VertexIndex = 0;VertexIndex < Vertices.Num();VertexIndex++)
			{
				Vertices(VertexIndex).Position = LegacyPositionVertexBuffer.Positions(VertexIndex).Position;
				Vertices(VertexIndex).TangentX = LegacyTangentVertexBuffer.Tangents(VertexIndex * 3 + 0);
				Vertices(VertexIndex).TangentY = LegacyTangentVertexBuffer.Tangents(VertexIndex * 3 + 1);
				Vertices(VertexIndex).TangentZ = LegacyTangentVertexBuffer.Tangents(VertexIndex * 3 + 2);
			}
		}

		// Load legacy UV buffers.
		TArray<FLegacyStaticMeshUVBuffer> LegacyUVBuffers;
		Ar << LegacyUVBuffers;

		// Copy the legacy UV data into the vertex array.
		const UINT NumTexCoords = Min<UINT>(LegacyUVBuffers.Num(),MAX_TEXCOORDS);
		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			for(INT VertexIndex = 0;VertexIndex < Vertices.Num();VertexIndex++)
			{
				Vertices(VertexIndex).UVs[UVIndex] = LegacyUVBuffers(UVIndex).UVs(VertexIndex);
			}
		}

		// Initialize the vertex buffer using the vertices extracted from the legacy data.
		VertexBuffer.Init(Vertices,NumTexCoords);
		PositionVertexBuffer.Init(Vertices);
		NumVertices = Vertices.Num();
	} 
	else if (Ar.IsLoading() && Ar.Ver() < VER_SEPARATED_STATIC_MESH_POSITIONS)
	{
		FLegacyStaticMeshVertexBuffer LegacyVertexBuffer;
		Ar << LegacyVertexBuffer << NumVertices;
		VertexBuffer.Init(LegacyVertexBuffer);
		PositionVertexBuffer.Init(LegacyVertexBuffer);
	}
	else
	{
		Ar << PositionVertexBuffer << VertexBuffer << NumVertices;
	}	

	Ar << IndexBuffer;
	Ar << WireframeIndexBuffer;
	Edges.BulkSerialize( Ar );
	Ar << ShadowTriangleDoubleSided;

 
 	//!{ 2006-05-30	Çã Ã¢ ¹Î
 	if( Ar.LicenseeVer() >= VER_AVA_UNWRAP_STATICMESH_LIGHTMAP_COORDS )
 	{
 		Ar << LightMapWidth;
 		Ar << LightMapHeight;
 	}
 	//!} 2006-05-30	Çã Ã¢ ¹Î
 
 	//!{ 2006-06-27	Çã Ã¢ ¹Î
 	if( Ar.LicenseeVer() >= VER_AVA_STATICMESH_EXPORT_RADIOSITY_GEOMETRY )
 	{
 		BulkExportElements.Serialize( Ar, Owner );
 		BulkConvexPolygons.Serialize( Ar, Owner );
 		BulkConvexPlanes.Serialize( Ar, Owner );
 		BulkConvexTangentXs.Serialize( Ar, Owner );
 		BulkConvexTangentYs.Serialize( Ar, Owner );
 		BulkConvexToTris.Serialize( Ar, Owner );
 		BulkConvexPolygonIndexBuffer.Serialize( Ar, Owner );
 		BulkConvexPolygonVertexBuffer.Serialize( Ar, Owner );
 	}
 	//!} 2006-06-27	Çã Ã¢ ¹Î
}

INT FStaticMeshRenderData::GetTriangleCount() const
{
	INT NumTriangles = 0;
	for(INT ElementIndex = 0;ElementIndex < Elements.Num();ElementIndex++)
	{
		NumTriangles += Elements(ElementIndex).NumTriangles;
	}
	return NumTriangles;
}

/**
 * Initializes the LOD's render resources.
 * @param Parent Parent mesh
 */
void FStaticMeshRenderData::InitResources(UStaticMesh* Parent)
{
	// Initialize the vertex and index buffers.
	BeginInitResource(&IndexBuffer);
	if( WireframeIndexBuffer.Indices.Num() )
	{
		BeginInitResource(&WireframeIndexBuffer);
	}	
	BeginInitResource(&VertexBuffer);
	BeginInitResource(&PositionVertexBuffer);

	// Initialize the static mesh's vertex factory.

	TSetResourceDataContext<FLocalVertexFactory> VertexFactoryData(&VertexFactory);
	VertexFactoryData->PositionComponent = FVertexStreamComponent(
		&PositionVertexBuffer,
		STRUCT_OFFSET(FPositionVertex,Position),
		PositionVertexBuffer.GetStride(),
		VET_Float3
		);
	VertexFactoryData->TangentBasisComponents[0] = FVertexStreamComponent(
		&VertexBuffer,
		STRUCT_OFFSET(TStaticMeshFullVertex<MAX_TEXCOORDS>,TangentX),
		VertexBuffer.GetStride(),
		VET_PackedNormal
		);
	VertexFactoryData->TangentBasisComponents[1] = FVertexStreamComponent(
		&VertexBuffer,
		STRUCT_OFFSET(TStaticMeshFullVertex<MAX_TEXCOORDS>,TangentY),
		VertexBuffer.GetStride(),
		VET_PackedNormal
		);
	VertexFactoryData->TangentBasisComponents[2] = FVertexStreamComponent(
		&VertexBuffer,
		STRUCT_OFFSET(TStaticMeshFullVertex<MAX_TEXCOORDS>,TangentZ),
		VertexBuffer.GetStride(),
		VET_PackedNormal
		);

	VertexFactoryData->TextureCoordinates.Empty();
	for(UINT UVIndex = 0;UVIndex < VertexBuffer.GetNumTexCoords();UVIndex++)
	{
		VertexFactoryData->TextureCoordinates.AddItem(FVertexStreamComponent(
			&VertexBuffer,
			STRUCT_OFFSET(TStaticMeshFullVertex<MAX_TEXCOORDS>,UVs) + sizeof(FVector2D) * UVIndex,
			VertexBuffer.GetStride(),
			VET_Float2
			));
	}

	if(Parent->LightMapCoordinateIndex >= 0 && (UINT)Parent->LightMapCoordinateIndex < VertexBuffer.GetNumTexCoords())
	{
		VertexFactoryData->ShadowMapCoordinateComponent = FVertexStreamComponent(
			&VertexBuffer,
			STRUCT_OFFSET(TStaticMeshFullVertex<MAX_TEXCOORDS>,UVs) + sizeof(FVector2D) * Parent->LightMapCoordinateIndex,
			VertexBuffer.GetStride(),
			VET_Float2
			);
	}

	VertexFactoryData.Commit();
	BeginInitResource(&VertexFactory);

	// Initialize the static mesh's shadow vertex factory.
	if( UEngine::ShadowVolumesAllowed() )
	{
		TSetResourceDataContext<FLocalShadowVertexFactory> ShadowVertexFactoryData(&ShadowVertexFactory);
		ShadowVertexFactoryData->PositionComponent = FVertexStreamComponent(&PositionVertexBuffer,STRUCT_OFFSET(FPositionVertex,Position),PositionVertexBuffer.GetStride(),VET_Float3);
		ShadowVertexFactoryData->ExtrusionComponent = FVertexStreamComponent(&VertexBuffer,STRUCT_OFFSET(TStaticMeshFullVertex<MAX_TEXCOORDS>,ShadowExtrusionPredicate),VertexBuffer.GetStride(),VET_Float1);
		ShadowVertexFactoryData.Commit();
		BeginInitResource(&ShadowVertexFactory);
	}

	INC_DWORD_STAT_BY( STAT_StaticMeshVertexMemory, VertexBuffer.GetStride() * VertexBuffer.GetNumVertices() + PositionVertexBuffer.GetStride() * PositionVertexBuffer.GetNumVertices());
	INC_DWORD_STAT_BY( STAT_StaticMeshIndexMemory, (IndexBuffer.Indices.Num() + WireframeIndexBuffer.Indices.Num()) * 2 );
}

/**
 * Releases the LOD's render resources.
 */
void FStaticMeshRenderData::ReleaseResources()
{
	DEC_DWORD_STAT_BY( STAT_StaticMeshVertexMemory, VertexBuffer.GetStride() * VertexBuffer.GetNumVertices() + PositionVertexBuffer.GetStride() * PositionVertexBuffer.GetNumVertices());
	DEC_DWORD_STAT_BY( STAT_StaticMeshIndexMemory, (IndexBuffer.Indices.Num() + WireframeIndexBuffer.Indices.Num()) * 2 );

	// Release the vertex and index buffers.
	BeginReleaseResource(&IndexBuffer);
	BeginReleaseResource(&WireframeIndexBuffer);
	BeginReleaseResource(&VertexBuffer);
	BeginReleaseResource(&PositionVertexBuffer);

	// Release the vertex factories.
	BeginReleaseResource(&VertexFactory);
	BeginReleaseResource(&ShadowVertexFactory);
}

/*-----------------------------------------------------------------------------
UStaticMesh
-----------------------------------------------------------------------------*/

/**
 * Initializes the static mesh's render resources.
 */
void UStaticMesh::InitResources()
{
	for(INT LODIndex = 0;LODIndex < LODModels.Num();LODIndex++)
	{
		LODModels(LODIndex).InitResources(this);
	}
}

/**
 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
 *
 * @return size of resource as to be displayed to artists/ LDs in the Editor.
 */
INT UStaticMesh::GetResourceSize()
{
	FArchiveCountMem CountBytesSize( this );
	INT ResourceSize = CountBytesSize.GetNum();
	return ResourceSize;
}


/**
 * Releases the static mesh's render resources.
 */
void UStaticMesh::ReleaseResources()
{
	for(INT LODIndex = 0;LODIndex < LODModels.Num();LODIndex++)
	{
		LODModels(LODIndex).ReleaseResources();
	}

	// insert a fence to signal when these commands completed
	ReleaseResourcesFence.BeginFence();
}


//
//	UStaticMesh::StaticConstructor
//
void UStaticMesh::StaticConstructor()
{
	new(GetClass()->HideCategories) FName(NAME_Object);

	new(GetClass(),TEXT("UseSimpleLineCollision"),RF_Public)		UBoolProperty(CPP_PROPERTY(UseSimpleLineCollision),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("UseSimpleBoxCollision"),RF_Public)			UBoolProperty(CPP_PROPERTY(UseSimpleBoxCollision),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("UseSimpleRigidBodyCollision"),RF_Public)	UBoolProperty(CPP_PROPERTY(UseSimpleRigidBodyCollision),TEXT(""),CPF_Edit);	
	new(GetClass(),TEXT("ForceDoubleSidedShadowVolumes"),RF_Public)	UBoolProperty(CPP_PROPERTY(DoubleSidedShadowVolumes),TEXT(""),CPF_Edit);

	new(GetClass(),TEXT("LightMapResolution"),RF_Public)			UIntProperty(CPP_PROPERTY(LightMapResolution),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("LightMapCoordinateIndex"),RF_Public)		UIntProperty(CPP_PROPERTY(LightMapCoordinateIndex),TEXT(""),CPF_Edit);
	new(GetClass(),TEXT("LODDistanceRatio"),RF_Public)				UFloatProperty(CPP_PROPERTY(LODDistanceRatio),TEXT(""),CPF_Edit);

	//!{ 2006-05-30	Çã Ã¢ ¹Î
	new(GetClass(),TEXT("LuxelSize"),RF_Public)						UIntProperty(CPP_PROPERTY(LuxelSize),TEXT(""),CPF_Edit);
	//!} 2006-05-30	Çã Ã¢ ¹Î

	/**
	 * The following code creates a dynamic array of structs, where the struct contains a dynamic array of MaterialInstances...In unrealscript, this declaration
	 * would look something like this:
	 *
	 *	struct StaticMeshLODInfo
	 *	{
	 *		var() editfixedsize native array<MaterialInstance> EditorMaterials;
	 *	};
	 * 
	 *	var() editfixedsize native array<StaticMeshLODInfo> LODInfo;
	 *
	 */

	//////////////////////////////////////////////////////////////////////////
	// First create StaticMeshLODElement struct
	UScriptStruct* LODElementStruct = new(GetClass(),TEXT("StaticMeshLODElement"),RF_Public|RF_Transient|RF_Native) UScriptStruct(NULL);
	new(LODElementStruct,TEXT("Material"),RF_Public)			UObjectProperty(EC_CppProperty,0,TEXT(""),CPF_Edit,UMaterialInstance::StaticClass());
	new(LODElementStruct,TEXT("bEnableCollision"),RF_Public)	UBoolProperty(EC_CppProperty,sizeof(UMaterialInstance*),TEXT(""),CPF_Edit | CPF_Native);

	// We're finished adding properties to the FStaticMeshLODElement struct - now we link the struct's properties (which sets the PropertiesSize for the struct) and initialize its defaults
	LODElementStruct->SetPropertiesSize(sizeof(FStaticMeshLODElement));
	LODElementStruct->AllocateStructDefaults();
	FArchive ArDummy0;
	LODElementStruct->Link(ArDummy0,0);

	//////////////////////////////////////////////////////////////////////////
	// Then create the StaticMeshLODInfo struct
	UScriptStruct* LODStruct = new(GetClass(),TEXT("StaticMeshLODInfo"),RF_Public|RF_Transient|RF_Native) UScriptStruct(NULL);

	// Next, create the dynamic array of LODElements - use the struct as the outer for the new array property so that the array is contained by the struct
	UArrayProperty*	ElementsProp = new(LODStruct,TEXT("Elements"),RF_Public) UArrayProperty(EC_CppProperty,0,TEXT(""),CPF_Edit | CPF_EditFixedSize | CPF_Native);

	// Dynamic arrays have an Inner property which corresponds to the array type.
	ElementsProp->Inner	= new(ElementsProp,TEXT("StructProperty1"),RF_Public) UStructProperty(EC_CppProperty,0,TEXT(""),CPF_Edit,LODElementStruct);

	// Link defaults
	LODStruct->SetPropertiesSize(sizeof(FStaticMeshLODInfo));
	LODStruct->AllocateStructDefaults();
	FArchive ArDummy1;
	LODStruct->Link(ArDummy1,0);

	//////////////////////////////////////////////////////////////////////////
	// Finally add array property to the StaticMesh class itself.

	// Next, create the dynamic array of StaticMeshLODInfo structs...same procedure as creating the Materials array above, except that this time, we use the class as the Outer for
	// the array, so that the property becomes a member of the class
	UArrayProperty*	InfoProp = new(GetClass(),TEXT("LODInfo"),RF_Public) UArrayProperty(CPP_PROPERTY(LODInfo),TEXT(""),CPF_Edit | CPF_EditFixedSize | CPF_Native);
	InfoProp->Inner = new(InfoProp,TEXT("StructProperty0"),RF_Public) UStructProperty(EC_CppProperty,0,TEXT(""),CPF_Edit,LODStruct);

	// Add physics body setup
	new(GetClass(),TEXT("BodySetup"),RF_Public)							UObjectProperty(CPP_PROPERTY(BodySetup),TEXT(""),CPF_Edit | CPF_EditInline, URB_BodySetup::StaticClass());

	UClass* Class = GetClass();
	Class->EmitObjectReference( STRUCT_OFFSET( UStaticMesh, BodySetup ) ); //@todo rtgc: is this needed seeing that BodySetup is exposed above?
}

/**
 * Initializes property values for intrinsic classes.  It is called immediately after the class default object
 * is initialized against its archetype, but before any objects of this class are created.
 */
void UStaticMesh::InitializeIntrinsicPropertyValues()
{
	UseSimpleLineCollision		= TRUE;
	UseSimpleBoxCollision		= TRUE;
	UseSimpleRigidBodyCollision = TRUE;	
	LODDistanceRatio			= 1.0;
	LODMaxRange = 2000;

	//!{ 2006-05-30	Çã Ã¢ ¹Î
	LuxelSize					= 16;
	//!} 2006-05-30	Çã Ã¢ ¹Î
}

/**
 * Callback used to allow object register its direct object references that are not already covered by
 * the token stream.
 *
 * @param ObjectArray	array to add referenced objects to via AddReferencedObject
 */
void UStaticMesh::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects( ObjectArray );
	for( INT LODIndex=0; LODIndex<LODModels.Num(); LODIndex++ )
	{
		const FStaticMeshRenderData& LODRenderData = LODModels(LODIndex);
		for( INT ElementIndex=0; ElementIndex<LODRenderData.Elements.Num(); ElementIndex++ ) 
		{
			AddReferencedObject( ObjectArray, LODRenderData.Elements(ElementIndex).Material );
		}
	}
}

void UStaticMesh::PreEditChange(UProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	// Release the static mesh's resources.
	ReleaseResources();

	// Flush the resource release commands to the rendering thread to ensure that the edit change doesn't occur while a resource is still
	// allocated, and potentially accessing the UStaticMesh.
	ReleaseResourcesFence.Wait();
}

void UStaticMesh::PostEditChange(UProperty* PropertyThatChanged)
{
	// Ensure that LightMapResolution is either 0 or a power of two.
	if( LightMapResolution > 0 )
	{
		LightMapResolution = 1 << appCeilLogTwo( LightMapResolution );
	}
	else
	{
		LightMapResolution = 0;
	}

	UBOOL	bNeedsRebuild = FALSE;

 	//!{ 2006-06-27	Çã Ã¢ ¹Î
 	if( PropertyThatChanged->GetFName() == FName(TEXT("LuxelSize")) )
 	{
 		bNeedsRebuild = 1;
 
 		if( LuxelSize < 0 )
 			LuxelSize = 1;
 
 		LuxelSize = 1 << appCeilLogTwo( LuxelSize );
 
 		bNeedsRebuild = ModifyLuxelSize( LuxelSize );
 	}
 
 	// Radiosity LightMap coords °¡ ÀÖ´Ù¸é, Lightmap Coordinate ´Â Ç×»ó 1ÀÌ´Ù..
 	if( LODModels.Num() )
 	{
 		if( LODModels(0).BulkConvexPolygons.GetElementCount() > 0 )
 		{
 			LightMapCoordinateIndex = 1;
 		}
 	}
 	//!} 2006-06-27	Çã Ã¢ ¹Î

	// If any of the elements have had collision added or removed, rebuild the static mesh.
	// If any of the elements have had materials altered, update here
	for(INT i=0;i<LODModels.Num();i++)
	{
		FStaticMeshRenderData&	RenderData				= LODModels(i);

		for(INT ElementIndex = 0;ElementIndex < RenderData.Elements.Num();ElementIndex++)
		{	
			UMaterialInstance*	EditorMat				= LODInfo(i).Elements(ElementIndex).Material;
			const UBOOL			bEditorEnableCollision	= LODInfo(i).Elements(ElementIndex).bEnableCollision;

			// Copy from UI expose array to 'real' array.
			FStaticMeshElement&	MeshElement				= RenderData.Elements(ElementIndex);

			MeshElement.Material		= EditorMat;
			MeshElement.EnableCollision = bEditorEnableCollision;
			if(MeshElement.OldEnableCollision != MeshElement.EnableCollision)
			{
				bNeedsRebuild = TRUE;
				MeshElement.OldEnableCollision = MeshElement.EnableCollision;
			}

			// This only NULLs out direct references to materials.  We purposefully do not NULL
			// references to material instances that refer to decal materials, and instead just
			// warn about them during map checking.  The reason is that the side effects of NULLing
			// refs to instances pointing to decal materials would be strange (e.g. changing an
			// instance to refer to a decal material instead would then cause references to the
			// instance to break.
			UMaterialInstance*& ElementMaterial = MeshElement.Material;
			if ( ElementMaterial && ElementMaterial->IsA( UDecalMaterial::StaticClass() ) )
			{
				ElementMaterial = NULL;
			}
		}
	}

	if(bNeedsRebuild)
	{
		Build();
	}
	else
	{
        // Reinitialize the static mesh's resources.		
		InitResources();

		FStaticMeshComponentReattachContext(this);		
	}
	
	Super::PostEditChange(PropertyThatChanged);
}

void UStaticMesh::BeginDestroy()
{
	Super::BeginDestroy();
	ReleaseResources();

	// Free any physics-engine per-poly meshes.
	ClearPhysMeshCache();
}

UBOOL UStaticMesh::IsReadyForFinishDestroy()
{
	return ReleaseResourcesFence.GetNumPendingFences() == 0;
}

//
//	UStaticMesh::Rename
//

UBOOL UStaticMesh::Rename( const TCHAR* InName, UObject* NewOuter, ERenameFlags Flags )
{
	// Rename the static mesh
    return Super::Rename( InName, NewOuter, Flags );
}


/**
 * Called after duplication & serialization and before PostLoad. Used to e.g. make sure UStaticMesh's UModel
 * gets copied as well.
 */
void UStaticMesh::PostDuplicate()
{
	Super::PostDuplicate();
}

/**
 *	UStaticMesh::Serialize
 */
void UStaticMesh::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << Bounds;
	Ar << BodySetup;

	if(Ar.Ver() < VER_REMOVE_STATICMESH_COLLISIONMODEL)
	{
		UModel* DummyModel;
		Ar << DummyModel;
	}

	Ar << kDOPTree;

	if( Ar.IsLoading() )
	{
		Ar << InternalVersion;
	}
	else if( Ar.IsSaving() )
	{
		InternalVersion = STATICMESH_VERSION;
		Ar << InternalVersion;
	}

	LODModels.Serialize( Ar, this );
	
	Ar << LODInfo;
	
	Ar << ThumbnailAngle;

	if (Ar.Ver() >= VER_STATICMESH_THUMBNAIL_DISTANCE)
	{
		Ar << ThumbnailDistance;
	}

	// Repair bad collision data if the package is old
	if (Ar.Ver() < VER_REPAIR_STATICMESH_COLLISION && Ar.IsLoading())
	{
		TArray<FkDOPBuildCollisionTriangle> kDOPBuildTriangles;
		kDOPBuildTriangles.Empty(kDOPTree.Triangles.Num());
		// Iterate through the collision indices rebuilding the collision triangle data
		for (INT Index = 0; Index < kDOPTree.Triangles.Num(); Index++)
		{
			// Add the triangle to the list to build from
			new(kDOPBuildTriangles)FkDOPBuildCollisionTriangle(kDOPTree.Triangles(Index).v1,
				kDOPTree.Triangles(Index).v2,kDOPTree.Triangles(Index).v3,
				kDOPTree.Triangles(Index).MaterialIndex,
				LODModels(0).PositionVertexBuffer.VertexPosition(kDOPTree.Triangles(Index).v1),
				LODModels(0).PositionVertexBuffer.VertexPosition(kDOPTree.Triangles(Index).v2),
				LODModels(0).PositionVertexBuffer.VertexPosition(kDOPTree.Triangles(Index).v3));
		}
		// Now rebuild with the correct centroid data
		kDOPTree.Build(kDOPBuildTriangles);
	}

	// Reset clobbered default values.
	if( Ar.IsLoading() && Ar.Ver() < VER_STATICMESH_PROPERTY_FIXUP_2 )
	{
		UseSimpleBoxCollision		= TRUE;
		UseSimpleRigidBodyCollision = TRUE;
		LODDistanceRatio			= 1.0;
	}

	if( Ar.IsCountingMemory() )
	{
		Ar << LODMaxRange;
		Ar << PhysMeshScale3D;

		//TODO: Count these members when calculating memory used
		//Ar << kDOPTreeType;
		//Ar << PhysMesh;
		//Ar << ReleaseResourcesFence;
	}

#if !CONSOLE
	// Strip away loaded Editor-only data if we're a client and never care about saving.
	if( Ar.IsLoading() && GIsClient && !GIsEditor && !GIsUCC )
	{
		// Console platform is not a mistake, this ensures that as much as possible will be tossed.
		StripData( UE3::PLATFORM_Console );
	}
#endif
}

//
//	UStaticMesh::PostLoad
//
void UStaticMesh::PostLoad()
{
	Super::PostLoad();

	if(InternalVersion < STATICMESH_VERSION)
	{	
		Build();
	}

	if( !UEngine::ShadowVolumesAllowed() )
	{
		RemoveShadowVolumeData();
	}

	if( !GIsUCC && !HasAnyFlags(RF_ClassDefaultObject) )
	{
		InitResources();
	}
	check(LODModels.Num() == LODInfo.Num());
}

/**
 * Used by various commandlets to purge Editor only data from the object.
 */
void UStaticMesh::StripData(UE3::EPlatformType TargetPlatform)
{
	Super::StripData(TargetPlatform);

	// RawTriangles is only used in the Editor and for rebuilding static meshes.
	for( INT i=0; i<LODModels.Num(); i++ )
	{
		FStaticMeshRenderData& LODModel = LODModels(i);
		LODModel.RawTriangles.RemoveBulkData();
		LODModel.WireframeIndexBuffer.Indices.Empty();

 		//!{ 2006-06-27	Çã Ã¢ ¹Î
 		LODModels(i).BulkExportElements.RemoveBulkData();
 		LODModels(i).BulkConvexPolygons.RemoveBulkData();
 		LODModels(i).BulkConvexPlanes.RemoveBulkData();
 		LODModels(i).BulkConvexTangentXs.RemoveBulkData();
 		LODModels(i).BulkConvexTangentYs.RemoveBulkData();
 		LODModels(i).BulkConvexToTris.RemoveBulkData();
 		LODModels(i).BulkConvexPolygonIndexBuffer.RemoveBulkData();
 		LODModels(i).BulkConvexPolygonVertexBuffer.RemoveBulkData();
 		//!} 2006-06-27	Çã Ã¢ ¹Î
	}

	// remove shadow volume data based on engine config setting
	if( !UEngine::ShadowVolumesAllowed() )
	{
		RemoveShadowVolumeData();
	}
}

//
//	UStaticMesh::GetDesc
//

/** 
 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
 */
FString UStaticMesh::GetDesc()
{
	//@todo: Handle LOD descs here?
	return FString::Printf( TEXT("%d Tris, %d Verts"), LODModels(0).RawTriangles.GetElementCount(), LODModels(0).NumVertices );
}

/** 
 * Returns detailed info to populate listview columns
 */
FString UStaticMesh::GetDetailedDescription( INT Index )
{
	FString Description = TEXT( "" );
	switch( Index )
	{
	case 0:
		Description = FString::Printf( TEXT( "%d Triangles" ), LODModels(0).RawTriangles.GetElementCount() );
		break;
	case 1: 
		Description = FString::Printf( TEXT( "%d Vertices" ), LODModels(0).NumVertices );
		break;
	}
	return( Description );
}

/** 
* Removes all vertex data needed for shadow volume rendering 
*/
void UStaticMesh::RemoveShadowVolumeData()
{
#if !CONSOLE
	for( INT LODIdx=0; LODIdx < LODModels.Num(); LODIdx++ )
	{
		FStaticMeshRenderData& LODModel = LODModels(LODIdx);
		LODModel.Edges.Empty();
		LODModel.ShadowTriangleDoubleSided.Empty();
		LODModel.VertexBuffer.RemoveShadowVolumeVertices(LODModel.NumVertices);
	}
#endif
}

/*-----------------------------------------------------------------------------
UStaticMeshComponent
-----------------------------------------------------------------------------*/

void UStaticMeshComponent::InitResources()
{
	// Create the light-map resources.
	for(INT LODIndex = 0;LODIndex < LODData.Num();LODIndex++)
	{
		if(LODData(LODIndex).LightMap != NULL)
		{
			LODData(LODIndex).LightMap->InitResources();
		}
	}
}

void UStaticMeshComponent::PostEditUndo()
{
	// The component's light-maps are loaded from the transaction, so their resources need to be reinitialized.
	InitResources();

	Super::PostEditUndo();
}

/**
 * Called after all objects referenced by this object have been serialized. Order of PostLoad routed to 
 * multiple objects loaded in one set is not deterministic though ConditionalPostLoad can be forced to
 * ensure an object has been "PostLoad"ed.
 */
void UStaticMeshComponent::PostLoad()
{
	Super::PostLoad();

	// Initialize the resources for the freshly loaded component.
	InitResources();

	// Perform some postload fixups/ optimizations if we're running the game.
	if( GIsGame && StaticMesh && !HasAnyFlags( RF_ClassDefaultObject ) )
	{
		// Figure out whether this static mesh only has sections with unlit materials.
		UBOOL bHasOnlyUnlitMaterials = TRUE;
		for( INT LODIndex=0; LODIndex<StaticMesh->LODModels.Num() && bHasOnlyUnlitMaterials; LODIndex++ )
		{
			for( INT ElementIndex=0; ElementIndex<StaticMesh->LODModels(LODIndex).Elements.Num(); ElementIndex++ )
			{
				UMaterialInstance*	MaterialInstance	= StaticMesh->LODModels(LODIndex).Elements(ElementIndex).Material;
				UMaterial*			Material			= MaterialInstance ? MaterialInstance->GetMaterial() : NULL;
				// a NULL material will use the default material, which is lit
				if( !Material || Material->LightingModel != MLM_Unlit )
				{
					bHasOnlyUnlitMaterials = FALSE;
				}
			}
		}

		// Also check the component materials array for overrides.
		for( INT MaterialIndex=0; MaterialIndex<Materials.Num() && bHasOnlyUnlitMaterials; MaterialIndex++ )
		{
			UMaterialInstance*	MaterialInstance	= Materials(MaterialIndex);
			UMaterial*			Material			= MaterialInstance ? MaterialInstance->GetMaterial() : NULL;
			if( Material && Material->LightingModel != MLM_Unlit )
			{
				bHasOnlyUnlitMaterials = FALSE;
			}
		}

		// This mesh only uses unlit materials so there is no benefit to accepting any lights.
		if( bHasOnlyUnlitMaterials )
		{
			bAcceptsLights = FALSE;
		}
	}
}

/** Change the StaticMesh used by this instance. */
void UStaticMeshComponent::SetStaticMesh(UStaticMesh* NewMesh)
{
	// Do nothing if we are already using the supplied static mesh
	if(NewMesh == StaticMesh)
	{
		return;
	}

	// Don't allow changing static meshes if the owner is "static".
	if( !Owner || !Owner->bStatic )
	{
		// Terminate rigid-body data for this StaticMeshComponent
		TermComponentRBPhys(NULL);

		// Force the recreate context to be destroyed by going out of scope after setting the new static-mesh.
		{
			FComponentReattachContext ReattachContext(this);
			StaticMesh = NewMesh;
		}

		// Re-init the rigid body info.
		UBOOL bFixed = true;
		if(!Owner || Owner->Physics == PHYS_RigidBody)
		{
			bFixed = false;
		}
		InitComponentRBPhys(bFixed);
	}
}

/** Script version of SetStaticMesh. */
void UStaticMeshComponent::execSetStaticMesh _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UStaticMesh, NewMesh);
	P_FINISH;

	SetStaticMesh(NewMesh);
}

/**
 * Returns whether this primitive only uses unlit materials.
 *
 * @return TRUE if only unlit materials are used for rendering, false otherwise.
 */
UBOOL UStaticMeshComponent::UsesOnlyUnlitMaterials() const
{
	if( StaticMesh )
	{
		// Figure out whether any of the sections has a lit material assigned.
		UBOOL bUsesOnlyUnlitMaterials = TRUE;
		for( INT ElementIndex=0; ElementIndex<StaticMesh->LODModels(0).Elements.Num(); ElementIndex++ )
		{
			UMaterialInstance*	MaterialInstance	= GetMaterial(ElementIndex);
			UMaterial*			Material			= MaterialInstance ? MaterialInstance->GetMaterial() : NULL;

			if( !Material || Material->LightingModel != MLM_Unlit )
			{
				bUsesOnlyUnlitMaterials = FALSE;
				break;
			}
		}
		return bUsesOnlyUnlitMaterials;
	}
	else
	{
		return FALSE;
	}
}

/**
 * Returns the lightmap resolution used for this primivite instnace in the case of it supporting texture light/ shadow maps.
 * 0 if not supported or no static shadowing.
 *
 * @param Width		[out]	Width of light/shadow map
 * @param Height	[out]	Height of light/shadow map
 */
void UStaticMeshComponent::GetLightMapResolution( INT& Width, INT& Height ) const
{
	if( StaticMesh )
	{
		// Use overriden per component lightmap resolution.
		if( bOverrideLightMapResolution )
		{
			Width	= OverriddenLightMapResolution;
			Height	= OverriddenLightMapResolution;
		}
		// Use the lightmap resolution defined in the static mesh.
		else
		{
			Width	= StaticMesh->LightMapResolution;
			Height	= StaticMesh->LightMapResolution;
		}
	}
	// No associated static mesh!
	else
	{
		Width	= 0;
		Height	= 0;
	}
}

/**
 * Returns the light and shadow map memory for this primite in its out variables.
 *
 * Shadow map memory usage is per light whereof lightmap data is independent of number of lights, assuming at least one.
 *
 * @param [out] LightMapMemoryUsage		Memory usage in bytes for light map (either texel or vertex) data
 * @param [out]	ShadowMapMemoryUsage	Memory usage in bytes for shadow map (either texel or vertex) data
 */
void UStaticMeshComponent::GetLightAndShadowMapMemoryUsage( INT& LightMapMemoryUsage, INT& ShadowMapMemoryUsage ) const
{
	// Zero initialize.
	ShadowMapMemoryUsage	= 0;
	LightMapMemoryUsage		= 0;

	// Cache light/ shadow map resolution.
	INT LightMapWidth		= 0;
	INT	LightMapHeight		= 0;
	GetLightMapResolution( LightMapWidth, LightMapHeight );

	// Determine whether static mesh/ static mesh component has static shadowing.
	if( HasStaticShadowing() && StaticMesh )
	{
		// Determine whether we are using a texture or vertex buffer to store precomputed data.
		if( LightMapWidth > 0 && LightMapHeight > 0
		&&	StaticMesh->LightMapCoordinateIndex >= 0 
		&&	(UINT)StaticMesh->LightMapCoordinateIndex < StaticMesh->LODModels(0).VertexBuffer.GetNumTexCoords() )
		{
			// Stored in texture.
			const FLOAT MIP_FACTOR = 1.33f;
			ShadowMapMemoryUsage	= appTrunc( MIP_FACTOR * LightMapWidth * LightMapHeight ); // G8
			LightMapMemoryUsage		= appTrunc( NUM_LIGHTMAP_COEFFICIENTS * MIP_FACTOR * LightMapWidth * LightMapHeight / 2 ); // DXT1
		}
		else
		{
			// Stored in vertex buffer.
			ShadowMapMemoryUsage	= sizeof(FLOAT) * StaticMesh->LODModels(0).NumVertices;
			LightMapMemoryUsage		= sizeof(FQuantizedLightSample) * StaticMesh->LODModels(0).NumVertices;
		}
	}
}

INT UStaticMeshComponent::GetNumElements() const
{
	//@HACK ; deif ; for SMC which has no valid static mesh
	//check(StaticMesh->LODModels.Num() >= 1);
	return StaticMesh ? StaticMesh->LODModels(0).Elements.Num() : 0;
}

/**
 *	UStaticMeshComponent::GetMaterial
 * @param MaterialIndex Index of material
 * @return Material instance for this component at index
 */
UMaterialInstance* UStaticMeshComponent::GetMaterial(INT MaterialIndex) const
{
	// Call GetMateiral using the base (zeroth) LOD.
	return GetMaterial( MaterialIndex, 0 );
}

/**
*	UStaticMeshComponent::GetMaterial
* @param MaterialIndex Index of material
* @param LOD Lod level to query from
* @return Material instance for this component at index
*/
UMaterialInstance* UStaticMeshComponent::GetMaterial(INT MaterialIndex, INT LOD) const
{
	// If we have a base materials array, use that
	if(MaterialIndex < Materials.Num() && Materials(MaterialIndex))
	{
		return Materials(MaterialIndex);
	}
	// Otherwise get from static mesh lod
	else if(StaticMesh && MaterialIndex < StaticMesh->LODModels(LOD).Elements.Num() && StaticMesh->LODModels(LOD).Elements(MaterialIndex).Material)
	{
		return StaticMesh->LODModels(LOD).Elements(MaterialIndex).Material;
	}
	else
		return NULL;
}

UStaticMeshComponent::UStaticMeshComponent()
{
}

void UStaticMeshComponent::AddReferencedObjects( TArray<UObject*>& ObjectArray )
{
	Super::AddReferencedObjects(ObjectArray);
	for(INT LODIndex = 0;LODIndex < LODData.Num();LODIndex++)
	{
		if(LODData(LODIndex).LightMap != NULL)
		{
			LODData(LODIndex).LightMap->AddReferencedObjects(ObjectArray);
		}
	}
}

//!{ 2006-05-09	Çã Ã¢ ¹Î
struct FLightMapSampleData2D
{
public:
	friend FArchive& operator<<( FArchive& Ar, FLightMapSampleData2D& LightMap)
	{
		return Ar << LightMap.Data << LightMap.SizeX << LightMap.SizeY;
	}

private:
	TArray<FLightSample2> Data;
	UINT SizeX;
	UINT SizeY;
};
//!} 2006-05-09	Çã Ã¢ ¹Î

//!{ 2006-05-09	Çã Ã¢ ¹Î

/**
* The raw data which is used to construct a 1D light-map.
*/
struct FLightMapSampleData1D
{
public:
	//!{ 2006-05-08	Çã Ã¢ ¹Î
	friend FArchive& operator<<(FArchive& Ar,FLightMapSampleData1D& LightMap)
	{
		return Ar << LightMap.Data << LightMap.Size;
	}
	//!} 2006-05-08	Çã Ã¢ ¹Î

private:

	/** The incident light samples for a 1D array of points. */
	TArray<FLightSample2> Data;

	/** The number of elements in the light-map. */
	UINT Size;
};
//!} 2006-05-09	Çã Ã¢ ¹Î

void UStaticMeshComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if( Ar.Ver() < VER_RENDERING_REFACTOR )
	{
		TIndirectArray<FLegacyStaticMeshLightSerializer> LegacyStaticLights;
		LegacyStaticLights.Serialize( Ar, this );
	}

 
 	//!{ 2006-06-26	Çã Ã¢ ¹Î 	
 	if( Ar.LicenseeVer() >= VER_AVA_SAMPLE_AMBIENT && Ar.LicenseeVer() < VER_AVA_REMOVE_SAMPLE_AMBIENT )
 	{
		FLightMapSampleData2D LegacyTextureLightMap;
		FLightMapSampleData1D LegacyVertexLightMap;

 		Ar << LegacyTextureLightMap;
 		Ar << LegacyVertexLightMap;
 	}
 	//!} 2006-06-26	Çã Ã¢ ¹Î


	if(Ar.Ver() >= VER_LIGHTMAP_NON_UOBJECT)
	{
		Ar << LODData;
	}
}

/**
 * Returns whether native properties are identical to the one of the passed in component.
 *
 * @param	Other	Other component to compare against
 *
 * @return TRUE if native properties are identical, FALSE otherwise
 */
UBOOL UStaticMeshComponent::AreNativePropertiesIdenticalTo( UComponent* Other ) const
{
	UBOOL bNativePropertiesAreIdentical = Super::AreNativePropertiesIdenticalTo( Other );
	UStaticMeshComponent* OtherSMC = CastChecked<UStaticMeshComponent>(Other);

	if( bNativePropertiesAreIdentical )
	{
		// Components are not identical if they have lighting information.
		if( LODData.Num() || OtherSMC->LODData.Num() )
		{
			bNativePropertiesAreIdentical = FALSE;
		}
	}
	
	return bNativePropertiesAreIdentical;
}

/** 
 * Called when any property in this object is modified in UnrealEd
 *
 * @param	PropertyThatChanged		changed property
 */
void UStaticMeshComponent::PostEditChange( UProperty* PropertyThatChanged )
{
	// Ensure that OverriddenLightMapResolution is either 0 or a power of two.
	if( OverriddenLightMapResolution > 0 )
	{
		OverriddenLightMapResolution = 1 << appCeilLogTwo( OverriddenLightMapResolution );
	}
	else
	{
		OverriddenLightMapResolution = 0;
	}

	// This only NULLs out direct references to materials.  We purposefully do not NULL
	// references to material instances that refer to decal materials, and instead just
	// warn about them during map checking.  The reason is that the side effects of NULLing
	// refs to instances pointing to decal materials would be strange (e.g. changing an
	// instance to refer to a decal material instead would then cause references to the
	// instance to break.
	for( INT MaterialIndex = 0 ; MaterialIndex < Materials.Num() ; ++MaterialIndex )
	{
		UMaterialInstance*&	MaterialInstance = Materials(MaterialIndex);
		if( MaterialInstance && MaterialInstance->IsA( UDecalMaterial::StaticClass() ) )
		{
			MaterialInstance = NULL;
		}
	}

	// Ensure properties are in sane range.
	SubDivisionStepSize = Clamp( SubDivisionStepSize, 1, 128 );
	MaxSubDivisions		= Clamp( MaxSubDivisions, 2, 50 );
	MinSubDivisions		= Clamp( MinSubDivisions, 2, MaxSubDivisions );

	Super::PostEditChange( PropertyThatChanged );
}

void UStaticMeshComponent::CheckForErrors()
{
	Super::CheckForErrors();

	// Get the mesh owner's name.
	FString OwnerName(GNone);
	if ( Owner )
	{
		OwnerName = Owner->GetName();
	}

	// Warn about direct or indirect references to decal materials.
	for( INT MaterialIndex = 0 ; MaterialIndex < Materials.Num() ; ++MaterialIndex )
	{
		UMaterialInstance* MaterialInstance = Materials(MaterialIndex);
		if ( MaterialInstance )
		{
			if( MaterialInstance->IsA( UDecalMaterial::StaticClass() ) )
			{
				GWarn->MapCheck_Add(MCTYPE_WARNING, Owner, *FString::Printf(TEXT("%s::%s : Decal material %s is applied to a static mesh"), *GetName(), *OwnerName, *MaterialInstance->GetName() ), MCACTION_NONE, TEXT("DecalMaterialStaticMesh"));
			}

			const UMaterial* ReferencedMaterial = MaterialInstance->GetMaterial();
			if( ReferencedMaterial && ReferencedMaterial->IsA( UDecalMaterial::StaticClass() ) )
			{
				GWarn->MapCheck_Add(MCTYPE_WARNING, Owner, *FString::Printf(TEXT("%s::%s : Material instance %s refers to a decal material (%s) but is applied to a static mesh"), *GetName(), *OwnerName, *MaterialInstance->GetName(), *ReferencedMaterial->GetName() ), MCACTION_NONE, TEXT("DecalMaterialInstanceStaticMesh"));
			}
		}
	}
}

//
//	UStaticMeshComponent::UpdateBounds
//

void UStaticMeshComponent::UpdateBounds()
{
	if(StaticMesh)
	{
		// Graphics bounds.
		Bounds = StaticMesh->Bounds.TransformBy(LocalToWorld);
		
		// Add bounds of collision geometry (if present).
		if(StaticMesh->BodySetup)
		{
			FMatrix Transform;
			FVector Scale3D;
			GetTransformAndScale(Transform, Scale3D);

			Bounds = LegacyUnion(Bounds,FBoxSphereBounds(StaticMesh->BodySetup->AggGeom.CalcAABB(Transform, Scale3D)));
		}

		// Takes into account that the static mesh collision code nudges collisions out by up to 1 unit.
		Bounds.BoxExtent += FVector(1,1,1);
		Bounds.SphereRadius += 1.0f;
	}
	else
	{
		Super::UpdateBounds();
	}
}

//
//	UStaticMeshComponent::IsValidComponent
//

UBOOL UStaticMeshComponent::IsValidComponent() const
{
	return StaticMesh != NULL && StaticMesh->LODModels.Num() && StaticMesh->LODModels(0).NumVertices > 0 && Super::IsValidComponent();
}

void UStaticMeshComponent::Attach()
{
	// Check that the static-mesh hasn't been changed to be incompatible with the cached light-map.
	for(INT i=0;i<LODData.Num();i++)
	{
		if(LODData(i).LightMap)
		{
			const FLightMap1D* LightMap1D = LODData(i).LightMap->GetLightMap1D();
			if(i >= StaticMesh->LODModels.Num() || LightMap1D && LightMap1D->NumSamples() != StaticMesh->LODModels(i).NumVertices)
			{
				// If the vertex light-map doesn't have the same number of elements as the static mesh has vertices, discard the light-map.
				LODData(i).LightMap = NULL;
			}
		}
	}

	// Change the tick group based upon blocking or not
	TickGroup = TickGroup < TG_PostAsyncWork ? (BlockActors ? TG_PreAsyncWork : TG_DuringAsyncWork) : TG_PostAsyncWork;

	// Check to see if we are attached to a static mesh actor
	bIsOwnerAStaticMeshActor = Cast<AStaticMeshActor>(Owner) != NULL;

	Super::Attach();
}

/* ==========================================================================================================
AStaticMeshCollectionActor
========================================================================================================== */
/* === AActor interface === */
/**
* Updates the CachedLocalToWorld transform for all attached components.
*/
void AStaticMeshCollectionActor::UpdateComponentsInternal( UBOOL bCollisionUpdate/*=FALSE*/ )
{
	checkf(!HasAnyFlags(RF_Unreachable), TEXT("%s"), *GetFullName());
	checkf(!HasAnyFlags(RF_ArchetypeObject|RF_ClassDefaultObject), TEXT("%s"), *GetFullName());
	checkf(!ActorIsPendingKill(), TEXT("%s"), *GetFullName());

	// Local to world used for attached non static mesh components.
	const FMatrix& ActorToWorld = LocalToWorld();

	for(INT ComponentIndex = 0;ComponentIndex < Components.Num();ComponentIndex++)
	{	
		UActorComponent* ActorComponent = Components(ComponentIndex);
		if( ActorComponent != NULL )
		{
			UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(ActorComponent);
			if( MeshComponent )
			{
				// never reapply the CachedParentToWorld transform for our StaticMeshComponents, since it will never change.
				MeshComponent->UpdateComponent(GWorld->Scene, this, MeshComponent->CachedParentToWorld);
			}
			else
			{
				ActorComponent->UpdateComponent(GWorld->Scene, this, ActorToWorld);
			}
		}
	}
}


/* === UObject interface === */
/**
* Serializes the LocalToWorld transforms for the StaticMeshComponents contained in this actor.
*/
void AStaticMeshCollectionActor::Serialize( FArchive& Ar )
{
	Super::Serialize(Ar);

	if (!HasAnyFlags(RF_ClassDefaultObject) && Ar.GetLinker() != NULL )
	{
		if ( Ar.IsLoading() )
		{
			FMatrix IdentityMatrix;
			for ( INT CompIndex = 0; CompIndex < StaticMeshComponents.Num(); CompIndex++ )
			{
				if ( StaticMeshComponents(CompIndex) != NULL )
				{
					Ar << StaticMeshComponents(CompIndex)->CachedParentToWorld;
				}
				else
				{
					// even if we had a NULL component for whatever reason, we still need to read the matrix data
					// from the stream so that we de-serialize the same amount of data that was serialized.
					Ar << IdentityMatrix;
				}
			}

			Components = (TArrayNoInit<UActorComponent*>&)StaticMeshComponents;
			StaticMeshComponents.Empty();
		}
		else if ( Ar.IsSaving() )
		{
			// serialize the default matrix for any components which are NULL so that we are always guaranteed to
			// de-serialize the correct amount of data
			FMatrix IdentityMatrix(FMatrix::Identity);
			for ( INT CompIndex = 0; CompIndex < StaticMeshComponents.Num(); CompIndex++ )
			{
				if ( StaticMeshComponents(CompIndex) != NULL )
				{
					Ar << StaticMeshComponents(CompIndex)->CachedParentToWorld;
				}
				else
				{
					Ar << IdentityMatrix;
				}
			}
		}
	}
}


// EOF




