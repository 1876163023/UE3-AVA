#pragma once

struct FBulletTrailRenderData
{
	FBulletTrailRenderData( FBulletTrailRenderContext* Context )
		: NumVertices(0), NumIndices(0), Segments( NULL )
	{
		TArray<FBulletTrailSegment>& SrcSegments = Context->Segments;
		TArray<WORD>& SrcIndices = Context->Indices;

		NumSegments = SrcSegments.Num();
		NumVertices = NumSegments * 4;
		NumIndices = SrcIndices.Num();

		if ( NumIndices > 0)
		{
			Segments = (FBulletTrailSegment*)appRealloc( Segments, sizeof(FBulletTrailSegment) * NumSegments );

			appMemcpy( Segments, &SrcSegments(0), sizeof(FBulletTrailSegment) * NumSegments );

			UMaterialInstance* MaterialInstance = Context->Component->Material;			
			
			if (MaterialInstance)
			{
				MaterialResource = MaterialInstance->GetInstanceInterface( FALSE );
			}
			else
			{
				MaterialResource = NULL;
			}
		}			
	}

	void UpdateVertices( const FSceneView* View )
	{
	}	

	~FBulletTrailRenderData()
	{
		appFree( Segments );
	}
	
	FBulletTrailSegment*						Segments;
	INT											NumVertices;
	INT											NumIndices;
	INT											NumSegments;
	const FMaterialInstance*					MaterialResource;

	DWORD GetMemoryFootprint( void ) const 
	{ 
		return sizeof( *this );
	}
};

