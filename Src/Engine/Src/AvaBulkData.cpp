/*=============================================================================
AvaBulkData.cpp : Ava에서 사용할 BulkData 정의
Red duck inc.
2006. 06 . 27
=============================================================================*/

#include "EnginePrivate.h"
#include "UnStaticMeshLegacy.h"


INT AvaStaticMeshExportElementBulkData::GetElementSize() const
{
	return sizeof( AvaStaticMeshExportElement );
}

void AvaStaticMeshExportElementBulkData::SerializeElement(FArchive& Ar, void* Data, INT ElementIndex )
{
	AvaStaticMeshExportElement& ExportElement = *((AvaStaticMeshExportElement*)Data + ElementIndex);
	Ar << ExportElement;
}


INT AvaConvexPolygonBulkData::GetElementSize() const
{
	return sizeof( AvaConvexPolygon );
}

void AvaConvexPolygonBulkData::SerializeElement(FArchive& Ar, void* Data, INT ElementIndex )
{
	AvaConvexPolygon& Polygon = *((AvaConvexPolygon*)Data + ElementIndex);
	Ar << Polygon;
}

INT FStaticMeshVertexBulkData::GetElementSize() const
{
	return sizeof( FLegacyStaticMeshVertex );
}

void FStaticMeshVertexBulkData::SerializeElement(FArchive& Ar, void* Data, INT ElementIndex )
{
	FLegacyStaticMeshVertex& Vertex = *((FLegacyStaticMeshVertex*)Data + ElementIndex);
	Ar << Vertex;
}

INT FVectorBulkData::GetElementSize() const
{
	return sizeof( FVector );
}

void FVectorBulkData::SerializeElement(FArchive& Ar, void* Data, INT ElementIndex )
{
	FVector& Vector = *((FVector*)Data + ElementIndex);
	Ar << Vector;
}