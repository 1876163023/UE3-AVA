#include "UnrealEd.h"
#include "../Src/EnvCubePrivate.h"

#define ENVCUBE_VIEWPORT_SIZEX 1024
#define ENVCUBE_VIEWPORT_SIZEY 1024

extern UBOOL GDoNotApplyCullDistances;
extern UBOOL GBakingEnvCubes;

FMatrix CalcCubeFaceViewMatrix( ECubeFace Face, FVector WorldLocation )
{
	FMatrix Result(FMatrix::Identity);

	static const FVector XAxis(1.f,0.f,0.f);
	static const FVector YAxis(0.f,1.f,0.f);
	static const FVector ZAxis(0.f,0.f,1.f);

	// vectors we will need for our basis
	FVector vUp(YAxis);
	FVector vDir;

	switch( Face )
	{
	case CubeFace_PosX:
		//vUp = YAxis;
		vDir = XAxis;
		break;
	case CubeFace_NegX:
		//vUp = YAxis;
		vDir = -XAxis;
		break;
	case CubeFace_PosY:
		vUp = -ZAxis;
		vDir = YAxis;
		break;
	case CubeFace_NegY:
		vUp = ZAxis;
		vDir = -YAxis;
		break;
	case CubeFace_PosZ:
		//vUp = YAxis;
		vDir = ZAxis;
		break;
	case CubeFace_NegZ:
		//vUp = YAxis;
		vDir = -ZAxis;
		break;
	}

	// derive right vector
	FVector vRight( vUp ^ vDir );
	// create matrix from the 3 axes
	Result = FBasisVectorMatrix( vRight, vUp, vDir, -WorldLocation );	

	return Result;
}

struct FEnvCubeShotInfo
{
	INT X, Y;
	INT Size;
	AEnvCubeActor* Actor;
	ECubeFace FaceIndex;
	FVector Location;
	UTexture2D* TargetTexture;

	void Initialize( AEnvCubeActor* InActor, ECubeFace InFaceIndex )
	{
		Actor = InActor;

		check( Actor != NULL );

		TargetTexture = Actor->Texture->GetFace(InFaceIndex);
		Size = Actor->TextureSize;
		Location = Actor->Location;
		FaceIndex = InFaceIndex;
	}

	FSceneView* CreateView( FSceneViewFamilyContext& ViewFamily )
	{
		FLOAT NearPlane = 20.0f;

		// projection matrix based on the fov,near/far clip settings
		// each face always uses a 90 degree field of view
		FPerspectiveMatrix ProjMatrix(
			90.f * (FLOAT)PI / 360.0f,
			(FLOAT)Size,
			(FLOAT)Size,
			NearPlane
			);

		TArray<FPrimitiveSceneInfo*> HiddenPrimitives;

		for (INT i=0; i<Actor->Filter.Num(); ++i)
		{
			AActor* HiddenActor = Actor->Filter(i);

			for (INT CompIdx=0; CompIdx<HiddenActor->Components.Num(); ++CompIdx)
			{
				UPrimitiveComponent* PrimComponent = Cast<UPrimitiveComponent>(HiddenActor->Components(CompIdx));
				if( PrimComponent &&
					PrimComponent->SceneInfo )
				{
					HiddenPrimitives.AddItem(PrimComponent->SceneInfo);
				}
			}
		}

		// create a view for the capture
		return new FSceneView(
			&ViewFamily,
			NULL,
			NULL,
			NULL,			
			NULL,
			NULL,
			NULL,
			X,
			Y,
			Size,
			Size,
			CalcCubeFaceViewMatrix((ECubeFace)FaceIndex, Location ),
			ProjMatrix,
			/* Capture actor에서는 foreground dpg가 없어야~~~ */
			ProjMatrix,
			90,
			/* 없어야~~~ 끝 */
			FLinearColor(0.f,0.f,0.f,0.f),
			FLinearColor(0.f,0.f,0.f,0.f),
			FLinearColor::White,
			HiddenPrimitives
			);
	}

	void UpdateTexture( const TArray<FColor>& TempBitmap, INT ViewportWidth )
	{			
		check( TargetTexture );

		const INT	NumTexels			= Size * Size;
		FColor*		Dst					= (FColor*)TargetTexture->Mips(0).Data.Lock(LOCK_READ_WRITE);
		FColor*		Src					= (FColor*)&TempBitmap(0);

		for( INT y=0; y<Size; y++ )
		{
			for( INT x=0; x<Size; x++ )
			{					
				Dst[y*Size+x] = Src[(y+Y)*ViewportWidth+x+X];
			}
		}

		TargetTexture->Mips(0).Data.Unlock();

		if (TargetTexture->GetOuter())
		{
			TargetTexture->GetOuter()->MarkPackageDirty();
		}
		else
		{
			TargetTexture->MarkPackageDirty();
		}

		TargetTexture->AddressX = TA_Clamp;
		TargetTexture->AddressY = TA_Clamp;

		TargetTexture->UpdateResource();
	}
};

void EnvCube_PrepareTextureFor( AEnvCubeActor* EnvCube )
{	
	EnvCube->Texture = EnvCube_GetCubeTex( EnvCube );

	if (EnvCube->Texture != NULL)
	{
		UTextureCube* Cube = EnvCube->Texture;
		UBOOL bDirty = FALSE;
		for (INT CubeIndex=0; CubeIndex<6; ++CubeIndex)
		{
			UTexture2D* Texture = Cube->GetFace(CubeIndex);

			if (Texture->SizeX != EnvCube->TextureSize ||
				Texture->SizeY != EnvCube->TextureSize)
			{
				bDirty = TRUE;
				Texture->Init(EnvCube->TextureSize, EnvCube->TextureSize, PF_A8R8G8B8);
			}
		}

		if (bDirty)
		{
			Cube->Validate();
		}

		return;
	}

	UTextureCube* Cube = CastChecked<UTextureCube>( UObject::StaticConstructObject(UTextureCube::StaticClass(),EnvCube->TexturePackage ? EnvCube->TexturePackage : EnvCube->GetOutermost(),*EnvCube_GetCubeTexName( EnvCube ),RF_Public|RF_Standalone|RF_Transactional|RF_LoadForClient|RF_LoadForEdit));

	for (INT CubeIndex=0; CubeIndex<6; ++CubeIndex)
	{
		UTexture2D* Texture = CastChecked<UTexture2D>( UObject::StaticConstructObject(UTexture2D::StaticClass(),EnvCube->TexturePackage ? EnvCube->TexturePackage : EnvCube->GetOutermost(),*EnvCube_GetCubeFaceTexName( EnvCube, CubeIndex ),RF_Public|RF_Standalone|RF_Transactional|RF_LoadForClient|RF_LoadForEdit));		

		/* Linear */
		Texture->SRGB = FALSE;
		Texture->RGBL = TRUE;
		Texture->CompressionNoMipmaps = TRUE;
		Texture->Init(EnvCube->TextureSize, EnvCube->TextureSize, PF_A8R8G8B8);				

		Cube->SetFace( CubeIndex, Texture );
	}	

	/* Linear */
	Cube->SRGB = FALSE;	
	Cube->RGBL = TRUE;
	Cube->Validate();
	Cube->NeverStream = TRUE;
	EnvCube->Texture = Cube;	
}

void EnvCube_Render()
{
	TArray<AEnvCubeActor*> EnvCubes;

	for (FActorIterator It; It; ++It)
	{
		AActor* Actor = *It;

		AEnvCubeActor* EnvCube = Cast<AEnvCubeActor>( Actor );

		if (EnvCube)
		{
			EnvCube_PrepareTextureFor( EnvCube );						

			EnvCube->MarkPackageDirty();

			EnvCubes.AddItem( EnvCube );
		}		
	}

	GWorld->MarkPackageDirty();

	/* 아무 것도 없으면 pass */
	if (EnvCubes.Num() == 0)
		return;

	/* Cull distance 적용 끕시다 */
	ENQUEUE_UNIQUE_RENDER_COMMAND(
		FResetCullDistances,		
	{
		GBakingEnvCubes = TRUE;
		GDoNotApplyCullDistances = TRUE;
	});	

	/* Viewport를 만들어야지 */
	UClient* Client = GEngine->Client;
	FViewportClient NullViewportClient;		
	FViewportFrame* ViewportFrame = Client->CreateViewportFrame(
		&NullViewportClient,
		TEXT("Rendering EnvCubes"),
		ENVCUBE_VIEWPORT_SIZEX,
		ENVCUBE_VIEWPORT_SIZEY,
		FALSE
		);	

	FViewport* Viewport = ViewportFrame->GetViewport();	

	// sort memory usage from large to small
	IMPLEMENT_COMPARE_POINTER( AEnvCubeActor, EnvCubeRender, { 
		return B->TextureSize - A->TextureSize;
	}
	);

	Sort<USE_COMPARE_POINTER( AEnvCubeActor, EnvCubeRender )>( &EnvCubes( 0 ), EnvCubes.Num() );	

	FEnvCubeShotInfo ShotInfo;
	ShotInfo.X = 0;
	ShotInfo.Y = 0;

	TArray<FEnvCubeShotInfo> ShotInfos;
	INT LineHeight = 0;

	for (INT i=0; i<EnvCubes.Num(); ++i)
	{
		INT CurrentSize = EnvCubes(i)->TextureSize;

		for (INT FaceIndex=0; FaceIndex<CubeFace_MAX; ++FaceIndex)
		{			
			if (ShotInfo.X + CurrentSize > ENVCUBE_VIEWPORT_SIZEX)
			{
				ShotInfo.X = 0;
				ShotInfo.Y += LineHeight;

				LineHeight = 0;

				if (ShotInfo.Y + CurrentSize > ENVCUBE_VIEWPORT_SIZEY)
				{
					// push page splitter
					ShotInfo.Size = -1;
					ShotInfos.AddItem( ShotInfo );
					ShotInfo.Y = 0;
				}
			}

			// 딱 맞는 경우에만 처리
			if (ShotInfo.X + CurrentSize <= ENVCUBE_VIEWPORT_SIZEX &&
				ShotInfo.Y + CurrentSize <= ENVCUBE_VIEWPORT_SIZEY)
			{
				ShotInfo.Initialize( EnvCubes(i), (ECubeFace)FaceIndex );
				
				ShotInfos.AddItem( ShotInfo );

				LineHeight = Max( LineHeight, CurrentSize );

				ShotInfo.X += CurrentSize;
			}
		}
	}	
	
	while (ShotInfos.Num() > 0)
	{		
		/* open RHI! */
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			BeginDrawingCommand,
			FViewport*,Viewport,Viewport,
		{
			RHIBeginDrawingViewport(Viewport->GetViewportRHI());
			Viewport->RenderTargetSurfaceRHI = RHIGetViewportBackBuffer(Viewport->GetViewportRHI());
		});

		FCanvas Canvas(Viewport,NULL);

		// Create a temporary canvas if there isn't already one.
		UCanvas* CanvasObject = FindObject<UCanvas>(UObject::GetTransientPackage(),TEXT("CanvasObject"));
		if( !CanvasObject )
		{
			CanvasObject = ConstructObject<UCanvas>(UCanvas::StaticClass(),UObject::GetTransientPackage(),TEXT("CanvasObject"));
			CanvasObject->AddToRoot();
		}
		CanvasObject->Canvas = &Canvas;	

		FSceneViewFamilyContext ViewFamily( Viewport, GWorld->Scene, SHOW_DefaultGame & ~(SHOW_PostProcess), GWorld->GetTimeSeconds(), GWorld->GetRealTimeSeconds(), NULL, TRUE, TRUE, TRUE );			

		// create a view for the capture
		FSceneView* View = new FSceneView(
			&ViewFamily,
			NULL,
			NULL,
			NULL,			
			NULL,
			NULL,
			NULL,
			0,
			0,
			ENVCUBE_VIEWPORT_SIZEX,
			ENVCUBE_VIEWPORT_SIZEY,
			FMatrix::Identity,
			FMatrix::Identity,		
			FMatrix::Identity,
			90,
			/* 없어야~~~ 끝 */
			FLinearColor(0.f,1.f,0.f,1.f),
			FLinearColor(0.f,0.f,0.f,0.f),
			FLinearColor::White,
			TArray<FPrimitiveSceneInfo*>()
			);
		// add the view to the family
		ViewFamily.Views.AddItem(View);	

		for (INT i=0; i<ShotInfos.Num(); ++i)
		{
			// page splitter
			if (ShotInfos(i).Size < 0)
				break;

			FSceneView* View = ShotInfos(i).CreateView( ViewFamily );

			ViewFamily.Views.AddItem( View );
		}

		// Update level streaming.
		GWorld->UpdateLevelStreaming( &ViewFamily );

		BeginRenderingViewFamily(&Canvas,&ViewFamily);	

		Canvas.Flush();

		// Read the contents of the viewport into an temp array.
		TArray<FColor> TempBitmap;
		UBOOL IsOk = Viewport->ReadPixels(TempBitmap);

		/* flush RHI! */
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			EndDrawingCommand,
			FViewport*,Viewport,Viewport,
		{
			RHIEndDrawingViewport(Viewport->GetViewportRHI(),TRUE);
		});

		INT LastItemIndex = 0;

		for (INT i=0; i<ShotInfos.Num(); ++i)
		{
			LastItemIndex++;

			// page splitter
			if (ShotInfos(i).Size < 0)
				break;

			ShotInfos(i).UpdateTexture( TempBitmap, Viewport->GetSizeX() );
		}

		ShotInfos.Remove( 0, LastItemIndex );
	}	

	for (INT i=0; i<EnvCubes.Num(); ++i)
	{
		UTextureCube* CubeTexture = EnvCubes(i)->Texture;
		check( CubeTexture );

		CubeTexture->UpdateResource();
	}

	/* Viewport 없앰 */
	Client->CloseViewport( Viewport );	

	/* Cull distance 적용 다시 restore */
	ENQUEUE_UNIQUE_RENDER_COMMAND(
		FSetCullDistances,		
	{
		GDoNotApplyCullDistances = FALSE;
		GBakingEnvCubes = FALSE;
	});	

	EnvCube_MarkDirty();
}