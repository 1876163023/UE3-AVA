/*=============================================================================
AvaStaticMeshLight.cpp: Static mesh Radiosity lighting code.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnRaster.h"
#include "AvaRadiosityLightmapRasterizer.h"

//!{ 2006-05-02	허 창 민

// 이것은 AvaCode를 분리해 내기 위해서 UnStaticMehsLight.cpp에서 복사해 온 것이다.
// 엔진이 갱신되었을 때, 기능상 동일하도록 Check 하여야 한다.

/**
* The interpolated vectors for the static-mesh lighting rasterizer.
*/
struct FStaticMeshLightMapRasterInterpolant2
{
	FVector Vertex;
	FVector TangentLightVector;

	FStaticMeshLightMapRasterInterpolant2(const FVector& InVertex,const FVector& InTangentLightVector):
	Vertex(InVertex),
		TangentLightVector(InTangentLightVector)
	{}

	friend FStaticMeshLightMapRasterInterpolant2 operator+(const FStaticMeshLightMapRasterInterpolant2& A,const FStaticMeshLightMapRasterInterpolant2& B)
	{
		return FStaticMeshLightMapRasterInterpolant2(
			A.Vertex + B.Vertex,
			A.TangentLightVector + B.TangentLightVector
			);
	}

	friend FStaticMeshLightMapRasterInterpolant2 operator-(const FStaticMeshLightMapRasterInterpolant2& A,const FStaticMeshLightMapRasterInterpolant2& B)
	{
		return FStaticMeshLightMapRasterInterpolant2(
			A.Vertex - B.Vertex,
			A.TangentLightVector - B.TangentLightVector
			);
	}

	friend FStaticMeshLightMapRasterInterpolant2 operator*(const FStaticMeshLightMapRasterInterpolant2& A,FLOAT B)
	{
		return FStaticMeshLightMapRasterInterpolant2(
			A.Vertex * B,
			A.TangentLightVector * B
			);
	}

	friend FStaticMeshLightMapRasterInterpolant2 operator/(const FStaticMeshLightMapRasterInterpolant2& A,FLOAT B)
	{
		FLOAT InvB = 1.0f / B;
		return FStaticMeshLightMapRasterInterpolant2(
			A.Vertex * InvB,
			A.TangentLightVector * InvB
			);
	}
};

//
//	FStaticMeshLightingRasterizer - Rasterizes static mesh lighting into a lightmap.
//

class FStaticMeshLightingRasterPolicy2
{
public:

	typedef FStaticMeshLightMapRasterInterpolant2 InterpolantType;

	struct FLightSample
	{
		UINT Visibility;
		UINT Coverage;
		FVector LightDirection;
		FLinearColor LightColor;
	};

	UStaticMeshComponent*	Component;
	TArray<FLightSample>	LightSamples;
	UINT					SizeX;
	UINT					SizeY;
	UBOOL					IsShadowCast;
	/** Whether to perform low quality sampling/ duplicating values. */
	UBOOL					bLowQualitySampling;
	/** Stride to use if low quality sampling is enabled. */
	UINT					GlobalSampleStride;

	ULightComponent*		Light;
	UPointLightComponent*	PointLight;
	FPlane					LightPosition;
	FLOAT					LightRadiusSquared;

	//!{ 2006-04-27	허 창 민
	UBOOL					bLightVisible;
	//!} 2006-04-27	허 창 민

	FStaticMeshLightingRasterPolicy2(UStaticMeshComponent* InComponent,ULightComponent* InLight,const FPlane& InLightPosition,UINT InSizeX,UINT InSizeY,const FLightingBuildOptions& BuildOptions)
		: Component(InComponent)
		, SizeX(InSizeX)
		, SizeY(InSizeY)
		, bLowQualitySampling(!BuildOptions.bPerformFullQualityBuild)
		, GlobalSampleStride(2)
		, Light(InLight)
		, PointLight(Cast<UPointLightComponent>(InLight))
		, LightPosition(InLightPosition)
		, LightRadiusSquared(0)
		//!{ 2006-04-27	허 창 민
		, bLightVisible(FALSE)
		//!} 2006-04-27	허 창 민
	{
		LightSamples.AddZeroed(SizeX * SizeY);
		IsShadowCast = Light->IsShadowCast(Component);

		if(PointLight)
		{
			LightRadiusSquared = Square(PointLight->Radius);
		}
	}

	/**
	* Returns a filtered shadow-map visibility percent.
	* @param	X - The X coordinate to read from.
	* @param	Y - The Y coordinate to read from.
	* @param	OutVisibility - On return, contains the filtered visibility value.
	* @return	True if the sample mapped to a point on the surface.
	*/
	UBOOL FilteredShadowSample(UINT X,UINT Y,FLOAT& OutVisibility) const
	{
		static UINT FilterSizeX = 5;
		static UINT FilterSizeY = 5;
		static UINT FilterMiddleX = (FilterSizeX - 1) / 2;
		static UINT FilterMiddleY = (FilterSizeY - 1) / 2;
		static UINT Filter[5][5] =
		{
			{ 58,  85,  96,  85, 58 },
			{ 85, 123, 140, 123, 85 },
			{ 96, 140, 159, 140, 96 },
			{ 85, 123, 140, 123, 85 },
			{ 58,  85,  96,  85, 58 }
		};

		UINT Visibility = 0;
		UINT Coverage = 0;
		for(UINT FilterY = 0;FilterY < FilterSizeX;FilterY++)
		{
			for(UINT FilterX = 0;FilterX < FilterSizeY;FilterX++)
			{
				INT	SubX = (INT)X - FilterMiddleX + FilterX,
					SubY = (INT)Y - FilterMiddleY + FilterY;
				if(SubX >= 0 && SubX < (INT)SizeX && SubY >= 0 && SubY < (INT)SizeY)
				{
					if( LightSamples(SubX + SubY * SizeX).Coverage > 0 )
					{
						Visibility += Filter[FilterX][FilterY] * LightSamples(SubX + SubY * SizeX).Visibility;
						Coverage += Filter[FilterX][FilterY] * LightSamples(SubX + SubY * SizeX).Coverage;
					}
					
				}
			}
		}

		if(LightSamples(X + Y * SizeX).Coverage > 0)
		{
			OutVisibility = (FLOAT)Visibility / (FLOAT)Coverage;
			return TRUE;
		}
		else
		{
			OutVisibility = 0.0f;
			return FALSE;
		}
	}

	/**
	* Returns a shadow-map visibility percent.
	* @param	X - The X coordinate to read from.
	* @param	Y - The Y coordinate to read from.
	* @param	OutVisibility - On return, contains the filtered visibility value.
	* @return	True if the sample mapped to a point on the surface.
	*/
	UBOOL GetShadowSample(UINT X,UINT Y,FLOAT& OutVisibility) const
	{
		UINT Visibility = 0;
		UINT Coverage = 0;

		Visibility = LightSamples( X + Y * SizeX).Visibility;
		Coverage = LightSamples( X + Y * SizeX ).Coverage;

		
		if( Coverage > 0)
		{
			OutVisibility = (FLOAT)Visibility / (FLOAT)Coverage;
			return TRUE;
		}
		else
		{
			OutVisibility = 0.0f;
			return FALSE;
		}
	}

protected:
	INT GetMinX() const { return 0; }
	INT GetMaxX() const { return SizeX - 1; }
	INT GetMinY() const { return 0; }
	INT GetMaxY() const { return SizeY - 1; }
	
	void ProcessPixel(INT X,INT Y,const FStaticMeshLightMapRasterInterpolant2& Interpolant,UBOOL BackFacing)
	{
		UINT SampleStride = 0;

		if( bLowQualitySampling )
		{
			// Calculate sample stride.
			SampleStride = Min3<UINT>( GlobalSampleStride, SizeX, SizeY );
			if( (X + SampleStride >= SizeX)
				||	(Y + SampleStride >= SizeY) )
			{
				SampleStride = 1;
			}

			// Early out for non- top-left pixels in a 2x2 quad if low quality sampling is enabled.
			if( bLowQualitySampling && ((X % SampleStride) || (Y % SampleStride)) )
			{
				return;
			}
		}

		const UINT			Coverage		= LightSamples(Y * SizeX + X).Coverage + 1;
		const FVector&		LightDirection	= Interpolant.TangentLightVector.SafeNormal();
		const FLinearColor&	LightColor		= Light->GetDirectIntensity(Interpolant.Vertex);

		// Propagate values across pixels when low quality sampling is enabled.
		if( bLowQualitySampling )
		{
			for( UINT OffsetY=0; OffsetY<SampleStride; OffsetY++ )
			{
				for( UINT OffsetX=0; OffsetX<SampleStride; OffsetX++ )
				{
					LightSamples((Y + OffsetY) * SizeX + X + OffsetX).LightDirection	= LightDirection;
					LightSamples((Y + OffsetY) * SizeX + X + OffsetX).LightColor		= LightColor;
					LightSamples((Y + OffsetY) * SizeX + X + OffsetX).Coverage			= Coverage;

				}
			}
		}
		else
		{
			LightSamples(Y * SizeX + X).LightDirection	= LightDirection;
			LightSamples(Y * SizeX + X).LightColor		= LightColor;
			LightSamples(Y * SizeX + X).Coverage		= Coverage;
		}

		if( !BackFacing )
		{
			FVector LightVector = (FVector)LightPosition - LightPosition.W * Interpolant.Vertex;

			if(PointLight && LightVector.SizeSquared() > LightRadiusSquared)
			{
				return;
			}

			if(IsShadowCast)
			{
				FVector			Start		= Interpolant.Vertex + LightVector.SafeNormal() * 0.25f;
				FVector			End			= Interpolant.Vertex + LightVector;
				FCheckResult	Hit(1);
				DWORD			TraceFlags	= TRACE_Level|TRACE_Actors|TRACE_ShadowCast|TRACE_StopAtAnyHit;

				if(!GWorld->SingleLineCheck(Hit,NULL,End,Start,TraceFlags,FVector(0,0,0),Light))
				{
					return;
				}

				if(Component->CastShadow && !Component->LineCheck(Hit,End,Start,FVector(0,0,0),TraceFlags))
				{
					return;
				}
			}

			const UINT Visibility = LightSamples(Y * SizeX + X).Visibility + 1;

			//!{ 2006-04-27	허 창 민
			bLightVisible = TRUE;
			//!} 2006-04-27	허 창 민

			// Propagate values across quad when low quality sampling is enabled.
			if( bLowQualitySampling )
			{
				for( UINT OffsetY=0; OffsetY<SampleStride; OffsetY++ )
				{
					for( UINT OffsetX=0; OffsetX<SampleStride; OffsetX++ )
					{
						LightSamples((Y + OffsetY) * SizeX + X + OffsetX).Visibility = Visibility;
					}
				}
			}
			else
			{
				LightSamples(Y * SizeX + X).Visibility = Visibility;
			}
		}
	}
};

//!} 2006-05-02	허 창 민

void UStaticMeshComponent::GetRadiosityLightMapResolution(INT* Width, INT* Height) const
{
	if( StaticMesh )
	{
		// Use overriden for convert radiosity lightmap to radiosity vertex lighting
		if( bOverrideLightMapResolution && OverriddenLightMapResolution == 0 )
		{
			*Width	= OverriddenLightMapResolution;
			*Height	= OverriddenLightMapResolution;
		}
		// Use the lightmap resolution of LODMOdel(0)
		else if( StaticMesh->LODModels.Num() )
		{
			*Width	= StaticMesh->LODModels(0).LightMapWidth;
			*Height	= StaticMesh->LODModels(0).LightMapHeight;
		}
	}
	// No associated static mesh!
	else
	{
		*Width	= 0;
		*Height	= 0;
	}
}

UBOOL UStaticMeshComponent::IsRadiosityTextureMapLighting( const FStaticMeshRenderData& RenderData ) const
{
	UBOOL	bUseTextureMap = FALSE;
	INT		LightMapWidth	= 0;
	INT		LightMapHeight	= 0;

	GetRadiosityLightMapResolution( &LightMapWidth, &LightMapHeight );

	if( (LightMapWidth > 0)
		&& (LightMapHeight > 0) 
		&&	StaticMesh->LightMapCoordinateIndex < (INT)RenderData.VertexBuffer.GetNumTexCoords()
		&&  RenderData.BulkConvexPolygons.GetElementCount() > 0 )
	{
		bUseTextureMap = TRUE;
	}
	else
	{
		bUseTextureMap = FALSE;
	}

	return bUseTextureMap;
}

/**
* Caches lighting information
* @param	BuildOptions		Build options passed on from UnrealEd( user selected )
* @param	RadiosityAdapter	Get Radiosity Lighting Result From this
*/

void UStaticMeshComponent::CacheRadiosityLighting( const FLightingBuildOptions& BuildOptions, AvaRadiosityAdapter& RadiosityAdapter )
{
#if !(CONSOLE || FINAL_RELEASE)
	InvalidateLightingCache();

	if( StaticMesh && HasStaticShadowing() && bAcceptsLights )
	{
		// The FComponentRecreateContext sets Scene to NULL, so we need to keep track of it here.
		if( StaticMesh->LODModels.Num() == 0 )
		{
			return;
		}

		TArray<ULightComponent*> RadiosityLights;
		for(TObjectIterator<ULightComponent> LightIt;LightIt;++LightIt)
		{
			const UBOOL bLightIsInWorld = LightIt->GetOwner() && GWorld->ContainsActor(LightIt->GetOwner());
			if (bLightIsInWorld && LightIt->UseDirectLightMap)
			{
				RadiosityLights.AddItem( *LightIt );
			}
		}

		FComponentReattachContext ReattachContext(this);

		// Figure out whether we are storing the lighting / shadowing information in a texture or vertex buffer.
		// texture map을 사용하는 경우, render data 에 저장된 lightmap resolution은 불변이다.
		// 즉, radiosity lighting일 경우, lightmap resolution은 override 할 수 없다.
		UBOOL bUseTextureMap = IsRadiosityTextureMapLighting( StaticMesh->LODModels(0) );

		if( bUseTextureMap )
		{
			// LOD Model 0 에 대해서만 한다.
			if( LODData.Num() == 0 )
			{
				LODData.AddZeroed();
			}

			FLightMapData2D*				LightMapData2D	= NULL;
			FLightMapData2D*				LightMapData2D_2= NULL;
			FLightMapData1D*				LightMapData1D	= NULL;
			FStaticMeshComponentLODInfo&	LOD				= LODData(0);
			FStaticMeshRenderData&			RenderData		= StaticMesh->LODModels(0);
			FLOAT							DeterminantSign	= LocalToWorld.Determinant() > 0.0f ? 1.0f : -1.0f;

			//@Note
			// LightGuids은 Unreal의 AttachLight의 속성을 위해 사용하는 것이니,
			// Unreal이 원하는 대로 계산해주어야 한다.			

			// light map을 만든다.
			ComputeRadiosityTextureMapLighting( LightMapData2D, LightMapData2D_2, RenderData, LOD, &RadiosityAdapter );

			// create and encode the light-maps
			if( LightMapData2D && LightMapData2D_2 )
			{				
				LightMapData2D->Lights.Append( RadiosityLights );

				LOD.LightMap = FLightMap2D::AllocateLightMap(
					GetOutermost(),
					LightMapData2D,
					LightMapData2D_2,
					FLightMap2D::NeedsBumpedLightmap(RenderData.Elements.Num() == 1 ? GetMaterial(0) : NULL),
					Bounds,
					RadiosityAdapter.bSupportsCascadedShadow_
					);

				delete LightMapData2D;
				delete LightMapData2D_2;
			}
		}
		else
		{
			// LOD Model 0 에 대해서만 Radiosity Vertex Light Map을 가져온다.
			if( LODData.Num() == 0 )
			{
				LODData.AddZeroed();
			}

			FLightMapData1D*				LightMapData1D	= NULL;
			FLightMapData1D*				LightMapData1DWithoutSun = NULL;
			FStaticMeshComponentLODInfo&	LOD				= LODData(0);
			FStaticMeshRenderData&			RenderData		= StaticMesh->LODModels(0);
			FLOAT							DeterminantSign	= LocalToWorld.Determinant() > 0.0f ? 1.0f : -1.0f;			

			// get sun visibility ; 2007. 11 .13 changmin
			if( ExportId != -1 )
				bCastSunShadow = RadiosityAdapter.GetSunVisibility(ExportId);

			// VertexLightMap 초기화는 이 함수 안에서 한다.
			ComputeRadiosityVertexMapLighting( LightMapData1D, LightMapData1DWithoutSun, RenderData, &RadiosityAdapter );			

			if( LightMapData1D && LightMapData1DWithoutSun )
			{								
				LightMapData1D->Lights.Append( RadiosityLights );
				LOD.LightMap = new FLightMap1D( this, *LightMapData1D, *LightMapData1DWithoutSun, RadiosityAdapter.bSupportsCascadedShadow_ );
				delete LightMapData1D;
				delete LightMapData1DWithoutSun;
				LightMapData1D = NULL;
				LightMapData1DWithoutSun = NULL;
			}

			// lod 0번의 lighting data 를 approximate한다.
			for(int k=1;k<StaticMesh->LODModels.Num();k++)
			{
				if( LODData.Num() <= k )
				{
					LODData.AddZeroed();
				}

				FLightMapData1D*				LightMapData1D	= NULL;
				FStaticMeshComponentLODInfo&	LOD				= LODData(k);
				FStaticMeshRenderData&			RenderData		= StaticMesh->LODModels(k);

				// 
				// Create LodVertex to BaseVertex Map By Distance( LodVertex, BaseVertex )
				//
				TMap<INT, INT> LodVertexToBaseVertexMap;

				FStaticMeshRenderData& BaseRenderData = StaticMesh->LODModels(0);

				for( INT VertexIndex = 0; VertexIndex < (INT)RenderData.NumVertices; ++VertexIndex )
				{
					const FVector& LodPos = RenderData.PositionVertexBuffer.VertexPosition(VertexIndex);
					const FVector& LodNormal = RenderData.VertexBuffer.VertexTangentZ(VertexIndex);

					check(BaseRenderData.NumVertices > 0 );

					FLOAT MinDistanceSquared = WORLD_MAX;
					INT MinBaseIndex = 0;

					for( INT BaseVertexIndex = 0; BaseVertexIndex < (INT)BaseRenderData.NumVertices; ++BaseVertexIndex )
					{
						const FVector& BasePosition = BaseRenderData.PositionVertexBuffer.VertexPosition(BaseVertexIndex);
						const FVector& BaseNormal = BaseRenderData.VertexBuffer.VertexTangentZ(BaseVertexIndex);

						if( (BaseNormal | LodNormal) > 0.866f )
						{
							FLOAT DistanceSquared = (LodPos - BasePosition).SizeSquared();
							if( DistanceSquared < MinDistanceSquared )
							{
								MinDistanceSquared = DistanceSquared;
								MinBaseIndex = BaseVertexIndex;
							}
						}
					}
					// add minimum distanced base vertex index to map
					LodVertexToBaseVertexMap.Set( VertexIndex, MinBaseIndex );
				}

				// create LOD model vertex light map
				ComputeRadiosityVertexMapLightingForLOD( LightMapData1D, LightMapData1DWithoutSun, RenderData, &RadiosityAdapter, LodVertexToBaseVertexMap );


				if( LightMapData1D && LightMapData1DWithoutSun )
				{					
					LightMapData1D->Lights.Append( RadiosityLights );
					LOD.LightMap = new FLightMap1D( this, *LightMapData1D, *LightMapData1DWithoutSun, RadiosityAdapter.bSupportsCascadedShadow_ );
					delete LightMapData1D;
					delete LightMapData1DWithoutSun;
					LightMapData1D = NULL;
					LightMapData1DWithoutSun = NULL;
				}
			}
		}
		

		// 개발시 사용된 데이타에 대해서는 LODData와 LODModel 의 숫자를 맞추어야 할 지도 모른다.
		// 그러나, 배포된 Editor만 사용된 사용자에 대해서는 LODModel의 숫자가 1 이어야 한다.
		// 이 부분이 실행되어서는 안된다.
		for( int k=1; k < StaticMesh->LODModels.Num(); ++k )
		{
			if( k >= LODData.Num() )
			{
				LODData.AddZeroed();
			}
		}
	}
#endif
}

/**
* Compute Radiosity Texture Map Lighting
*
* @param	LightMapData2D		Reference to lightmap data pointer if lightmaps are used
* @param	RenderData			Passed in for convenience
* @param	RadiosityAdapter	Has radiosity result data
*/
void UStaticMeshComponent::ComputeRadiosityTextureMapLighting( 	
	FLightMapData2D*&				LightMapData2D, 
	FLightMapData2D*&				LightMapData2D_2,
	FStaticMeshRenderData&			RenderData,
	FStaticMeshComponentLODInfo&	LOD,
	AvaRadiosityAdapter*			RadiosityAdapter
	)
{
#if !(CONSOLE || FINAL_RELEASE)
	if( FirstExportedFaceNumber == -1 )
	{
		return;
	}

	const INT LightMapWidth		= RenderData.LightMapWidth;
	const INT LightMapHeight	= RenderData.LightMapHeight;

	// Radiosity Lightmap 가져오기 : ExportSurface에서 Export한 순서와 같게 Iterate해야 합니다.
	// FirstExportedFace != -1일 때 Export되었고, Radiosity Lightmap이 있습니다
	INT iExportedFace = FirstExportedFaceNumber;
	
	if( !LightMapData2D )
	{
		// Create new light-map data buffer.
		LightMapData2D = new FLightMapData2D( LightMapWidth, LightMapHeight );
	}

	if( !LightMapData2D_2 )
	{
		LightMapData2D_2 = new FLightMapData2D( LightMapWidth, LightMapHeight );
	}

	// lock & unlock match 에 신경쓸 것
	const AvaStaticMeshExportElement* RawExportElements = (AvaStaticMeshExportElement*)RenderData.BulkExportElements.Lock( LOCK_READ_ONLY );
	const AvaConvexPolygon* RawPolygons = (AvaConvexPolygon*)RenderData.BulkConvexPolygons.Lock( LOCK_READ_ONLY );

	// create light maps
	for(INT ElementIndex = 0;ElementIndex < RenderData.Elements.Num();ElementIndex++)
	{
		const FStaticMeshElement& Element = RenderData.Elements(ElementIndex);

		const AvaStaticMeshExportElement& ExportElement = RawExportElements[ElementIndex];

		for( INT polyIndex = 0; polyIndex < ExportElement.NumPolygons_; ++polyIndex )
		{
			check( iExportedFace < RadiosityAdapter->FaceInfos_.Num() );
			
			// get radiosity lightmap
			AvaFaceInfo& F = RadiosityAdapter->FaceInfos_(iExportedFace++);

			AvaLightmapInfo& LightmapInfo		= F.LightmapInfo;
			
			const INT Width		= LightmapInfo.Width;
			const INT Height	= LightmapInfo.Height;

			if (LightmapInfo.Lightmap == NULL || LightmapInfo.LightmapWithoutSun == NULL )
			{				
				continue;
			}

			// copy radiosity light map
			const INT BaseX = RawPolygons[ExportElement.PolygonStart_ + polyIndex].BaseX_;
			const INT BaseY = RawPolygons[ExportElement.PolygonStart_ + polyIndex].BaseY_;

			INT CopyWidth = Width;
			if( BaseX + Width > LightMapWidth )
			{
				CopyWidth = LightMapWidth - BaseX;
			}
			INT CopyHeight = Height;
			if( BaseY + Height > LightMapHeight )
			{
				CopyHeight = LightMapHeight - BaseY;
			}

			// 모든 light가 light map에 들어간다.
			for( INT Y = 0; Y < CopyHeight; ++Y )
			{
				for( INT X = 0; X < CopyWidth; ++X )
				{
					FLightSample& DestSample = (*LightMapData2D)( BaseX + X, BaseY + Y);
					FLightSample& DestSample_2 = (*LightMapData2D_2)( BaseX + X, BaseY + Y);

					for( INT iBump = 0; iBump < NUM_AVA_LIGHTMAPS; ++iBump )
					{
						FLinearColor LightMapColor(0.0f, 0.0f, 0.0f);

						LightmapInfo.GetSample( X, Y, (iBump+1), &LightMapColor.R, &LightMapColor.G, &LightMapColor.B );	
						DestSample.AddLight( LightMapColor, iBump );

						LightmapInfo.GetSampleWithoutSun( X, Y, (iBump+1), &LightMapColor.R, &LightMapColor.G, &LightMapColor.B );
						DestSample_2.AddLight( LightMapColor, iBump );
					}
					DestSample.bIsMapped = TRUE;
					DestSample_2.bIsMapped = TRUE;
				}
			}
		}
	}

	RenderData.BulkExportElements.Unlock();
	RenderData.BulkConvexPolygons.Unlock();
#endif
}

/**
 * Compute Radiosity Vertex Map Lighting
 *
 * @param	LightMapData1D		Reference to lightmap data pointer if lightmaps are used
 * @param	RenderData			Passed in for convenience
 * @param	RadiosityAdapter	Has radiosity result data
 */
void UStaticMeshComponent::ComputeRadiosityVertexMapLighting(
										FLightMapData1D*& LightMapData1D,
										FLightMapData1D*& LightMapData1DWithoutSun,
										FStaticMeshRenderData& RenderData,
										AvaRadiosityAdapter* RadiosityAdapter )
{
#if !(CONSOLE || FINAL_RELEASE)
	INT iExportedVertex = FirstExportedVertexNumber;

	check( iExportedVertex != -1 );

	const int VertexLightSize = 16;	// rgbe per bump vectors and normal

	unsigned char* VertexLightData = &RadiosityAdapter->BlackMeshVertexLightMap_[iExportedVertex * VertexLightSize ];
	unsigned char* VertexLightDataWithoutSun = &RadiosityAdapter->BlackMeshVertexLightMapWithoutSun_[iExportedVertex * VertexLightSize ];
	const int ColorSize = 4; // RGBE

	// 라이트 맵이 항상 필요하다.
	if( !LightMapData1D )
	{
		LightMapData1D = new FLightMapData1D( RenderData.NumVertices );
	}
	if( !LightMapData1DWithoutSun )
	{
		LightMapData1DWithoutSun = new FLightMapData1D( RenderData.NumVertices );
	}

	for( UINT VertexIndex = 0; VertexIndex < RenderData.NumVertices; ++VertexIndex )
	{
		// with sun light
		const unsigned char* VertexLightBump0 = &VertexLightData[VertexIndex * VertexLightSize + (ColorSize * 0) ];
		const unsigned char* VertexLightBump1 = &VertexLightData[VertexIndex * VertexLightSize + (ColorSize * 1) ];
		const unsigned char* VertexLightBump2 = &VertexLightData[VertexIndex * VertexLightSize + (ColorSize * 2) ];
		const unsigned char* VertexLightBump3 = &VertexLightData[VertexIndex * VertexLightSize + (ColorSize * 3) ];

		// without sun light
		const unsigned char* VertexLightBump4 = &VertexLightDataWithoutSun[VertexIndex * VertexLightSize + (ColorSize * 0) ];
		const unsigned char* VertexLightBump5 = &VertexLightDataWithoutSun[VertexIndex * VertexLightSize + (ColorSize * 1) ];
		const unsigned char* VertexLightBump6 = &VertexLightDataWithoutSun[VertexIndex * VertexLightSize + (ColorSize * 2) ];
		const unsigned char* VertexLightBump7 = &VertexLightDataWithoutSun[VertexIndex * VertexLightSize + (ColorSize * 3) ];
		
		FVector LinearVertexLightBump0;
		FVector LinearVertexLightBump1;
		FVector LinearVertexLightBump2;
		FVector LinearVertexLightBump3;

		FVector LinearVertexLightBump4;
		FVector LinearVertexLightBump5;
		FVector LinearVertexLightBump6;
		FVector LinearVertexLightBump7;
		
		ConvertRGBEToLinear( VertexLightBump0, &LinearVertexLightBump0 );
		ConvertRGBEToLinear( VertexLightBump1, &LinearVertexLightBump1 );
		ConvertRGBEToLinear( VertexLightBump2, &LinearVertexLightBump2 );
		ConvertRGBEToLinear( VertexLightBump3, &LinearVertexLightBump3 );

		ConvertRGBEToLinear( VertexLightBump4, &LinearVertexLightBump4 );
		ConvertRGBEToLinear( VertexLightBump5, &LinearVertexLightBump5 );
		ConvertRGBEToLinear( VertexLightBump6, &LinearVertexLightBump6 );
		ConvertRGBEToLinear( VertexLightBump7, &LinearVertexLightBump7 );
	
		// 라이트맵 데이타를 채운다.
		FLightSample& Dest = (*LightMapData1D)(VertexIndex);
		Dest.AddLight( FLinearColor( LinearVertexLightBump1.X, LinearVertexLightBump1.Y, LinearVertexLightBump1.Z ), 0 );
		Dest.AddLight( FLinearColor( LinearVertexLightBump2.X, LinearVertexLightBump2.Y, LinearVertexLightBump2.Z ), 1 );
		Dest.AddLight( FLinearColor( LinearVertexLightBump3.X, LinearVertexLightBump3.Y, LinearVertexLightBump3.Z ), 2 );

		// without sun light
		FLightSample& Dest2 = (*LightMapData1DWithoutSun)(VertexIndex);
		Dest2.AddLight( FLinearColor( LinearVertexLightBump5.X, LinearVertexLightBump5.Y, LinearVertexLightBump5.Z ), 0 );
		Dest2.AddLight( FLinearColor( LinearVertexLightBump6.X, LinearVertexLightBump6.Y, LinearVertexLightBump6.Z ), 1 );
		Dest2.AddLight( FLinearColor( LinearVertexLightBump7.X, LinearVertexLightBump7.Y, LinearVertexLightBump7.Z ), 2 );
	}	
#endif
}

/**
* Compute Radiosity Vertex Map Lighting for lad
*
* @param	LightMapData1D				Reference to lightmap data pointer if lightmaps are used
* @param	RenderData					Passed in for convenience
* @param	RadiosityAdapter			Has radiosity result data
* @param	LodVertexToBaseVertexMap	lod vertex index to closest base vertex index
*/
void UStaticMeshComponent::ComputeRadiosityVertexMapLightingForLOD(
	FLightMapData1D*& LightMapData1D,
	FLightMapData1D*& LightMapData1DWithoutSun,
	FStaticMeshRenderData& RenderData,
	AvaRadiosityAdapter* RadiosityAdapter,
	const TMap<INT,INT>& LodVertexToBaseVertexMap)
{
#if !(CONSOLE || FINAL_RELEASE)
	INT iExportedVertex = FirstExportedVertexNumber;

	check( iExportedVertex != -1 );

	const int VertexLightSize = 16;	// rgbe per bump vectors and normal

	unsigned char* VertexLightData = &RadiosityAdapter->BlackMeshVertexLightMap_[iExportedVertex * VertexLightSize ];
	unsigned char* VertexLightDataWithoutSun = &RadiosityAdapter->BlackMeshVertexLightMapWithoutSun_[iExportedVertex * VertexLightSize ];

	const int ColorSize = 4; // RGBE

	// 라이트 맵이 항상 필요하다.
	if( !LightMapData1D )
	{
		LightMapData1D = new FLightMapData1D( RenderData.NumVertices );
	}

	if( !LightMapData1DWithoutSun )
	{
		LightMapData1DWithoutSun = new FLightMapData1D( RenderData.NumVertices );
	}

	for( UINT VertexIndex = 0; VertexIndex < RenderData.NumVertices; ++VertexIndex )
	{
		// base model 의 vertex light 값을 가져온다.
		INT BaseVertexIndex = *(LodVertexToBaseVertexMap.Find(VertexIndex));
		
		const unsigned char* VertexLightBump0 = &VertexLightData[BaseVertexIndex * VertexLightSize + (ColorSize * 0) ];
		const unsigned char* VertexLightBump1 = &VertexLightData[BaseVertexIndex * VertexLightSize + (ColorSize * 1) ];
		const unsigned char* VertexLightBump2 = &VertexLightData[BaseVertexIndex * VertexLightSize + (ColorSize * 2) ];
		const unsigned char* VertexLightBump3 = &VertexLightData[BaseVertexIndex * VertexLightSize + (ColorSize * 3) ];

		const unsigned char* VertexLightBump4 = &VertexLightDataWithoutSun[BaseVertexIndex * VertexLightSize + (ColorSize * 0) ];
		const unsigned char* VertexLightBump5 = &VertexLightDataWithoutSun[BaseVertexIndex * VertexLightSize + (ColorSize * 1) ];
		const unsigned char* VertexLightBump6 = &VertexLightDataWithoutSun[BaseVertexIndex * VertexLightSize + (ColorSize * 2) ];
		const unsigned char* VertexLightBump7 = &VertexLightDataWithoutSun[BaseVertexIndex * VertexLightSize + (ColorSize * 3) ];

		FVector LinearVertexLightBump0;
		FVector LinearVertexLightBump1;
		FVector LinearVertexLightBump2;
		FVector LinearVertexLightBump3;

		FVector LinearVertexLightBump4;
		FVector LinearVertexLightBump5;
		FVector LinearVertexLightBump6;
		FVector LinearVertexLightBump7;

		ConvertRGBEToLinear( VertexLightBump0, &LinearVertexLightBump0 );
		ConvertRGBEToLinear( VertexLightBump1, &LinearVertexLightBump1 );
		ConvertRGBEToLinear( VertexLightBump2, &LinearVertexLightBump2 );
		ConvertRGBEToLinear( VertexLightBump3, &LinearVertexLightBump3 );

		ConvertRGBEToLinear( VertexLightBump4, &LinearVertexLightBump4 );
		ConvertRGBEToLinear( VertexLightBump5, &LinearVertexLightBump5 );
		ConvertRGBEToLinear( VertexLightBump6, &LinearVertexLightBump6 );
		ConvertRGBEToLinear( VertexLightBump7, &LinearVertexLightBump7 );

		// 라이트맵 데이타를 채운다.
		{
			FLightSample& Dest = (*LightMapData1D)(VertexIndex);
			Dest.AddLight( FLinearColor( LinearVertexLightBump1.X, LinearVertexLightBump1.Y, LinearVertexLightBump1.Z ), 0 );
			Dest.AddLight( FLinearColor( LinearVertexLightBump2.X, LinearVertexLightBump2.Y, LinearVertexLightBump2.Z ), 1 );
			Dest.AddLight( FLinearColor( LinearVertexLightBump3.X, LinearVertexLightBump3.Y, LinearVertexLightBump3.Z ), 2 );
		}
		{
			FLightSample& Dest = (*LightMapData1DWithoutSun)(VertexIndex);
			Dest.AddLight( FLinearColor( LinearVertexLightBump5.X, LinearVertexLightBump5.Y, LinearVertexLightBump5.Z ), 0 );
			Dest.AddLight( FLinearColor( LinearVertexLightBump6.X, LinearVertexLightBump6.Y, LinearVertexLightBump6.Z ), 1 );
			Dest.AddLight( FLinearColor( LinearVertexLightBump7.X, LinearVertexLightBump7.Y, LinearVertexLightBump7.Z ), 2 );
		}

	}
#endif
}
