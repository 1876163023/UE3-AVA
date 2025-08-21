#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"
#include "ScenePrivate.h"

IMPLEMENT_CLASS(AEnvCubeActor);

#define ENVCUBE_SIZE 32

/// RenderThread에 선택 시키는 것은 location!, 그걸 fetch해 오는 op는 따로; 속도 때문에:)
const FTexture* GEnvCube_ActiveTexture = NULL;
UBOOL GEnvCube_Dirty = FALSE;
static INT GEnvCubeList_DirtyCount = 0, GEnvCubeList_UpdateCount = 0;
FVector GEnvCube_Location;

struct FEnvCubeEntry
{
	FVector					Location;
	const FTexture*			Texture;
};

//UBOOL GHack_SyncEveryDraw = TRUE;

static TArray<FEnvCubeEntry> GEnvCubes;

void AEnvCubeActor::Init()
{
	if( StaticMesh && GEngine && GEngine->EnvCubeActorMaterial )
	{
		// assign the material instance to the static mesh plane
		if( StaticMesh->Materials.Num() == 0 )
		{
			// add a slot if needed
			StaticMesh->Materials.Add();
		}
		// only one material entry
		StaticMesh->Materials(0) = GEngine->EnvCubeActorMaterial;
	}	
}

void AEnvCubeActor::PostLoad()
{
	Super::PostLoad();

	Init();	

	EnvCube_MarkDirty();	
}

void AEnvCubeActor::Spawned()
{
	Super::Spawned();

	Init();	

	EnvCube_MarkDirty();
}

void AEnvCubeActor::FinishDestroy()
{
	Super::FinishDestroy();

	EnvCube_MarkDirty();
}

void EnvCube_ResetCache()
{
	GEnvCube_ActiveTexture = NULL;
	GEnvCube_Dirty = FALSE;
	GEnvCube_Location = FVector( HALF_WORLD_MAX, HALF_WORLD_MAX, HALF_WORLD_MAX );
}

void EnvCube_SetLocation( FVector NewLocation )
{
	if (NewLocation == GEnvCube_Location)
		return;

	GEnvCube_Location = NewLocation;
	GEnvCube_Dirty = TRUE;
}

const FTexture* EnvCube_Resolve()
{
	check( IsInRenderingThread() );

	if (GEnvCube_Dirty) 
	{
		FLOAT MinDist = FLT_MAX;
		GEnvCube_ActiveTexture = NULL;

		for (INT i=0; i<GEnvCubes.Num(); ++i)
		{
			FLOAT SizeSq = (GEnvCubes(i).Location - GEnvCube_Location).SizeSquared();

			if (SizeSq < MinDist)
			{
				MinDist = SizeSq;				
				GEnvCube_ActiveTexture = GEnvCubes(i).Texture;
			}
		}			

		GEnvCube_Dirty = FALSE;
	}

	return GEnvCube_ActiveTexture;
}

static INT EnvCube_RegisterIndex = -1;
UBOOL GEnvCube_IsRequired;

void EnvCube_PrepareDraw()
{
	GEnvCube_IsRequired = FALSE;
	EnvCube_RegisterIndex = -1;
}

void EnvCube_Reserve( const FShaderParameter& Parameter, UINT ElementIndex /*= 0*/ )
{
	if(Parameter.IsBound())
	{
		check(ElementIndex < Parameter.GetNumRegisters());				
		EnvCube_RegisterIndex = Parameter.GetBaseRegisterIndex() + ElementIndex;		

		GEnvCube_IsRequired = TRUE;
	}	
	else
	{
		EnvCube_RegisterIndex = -1;
	}
}

void EnvCube_Update()
{
	/*if (GHack_SyncEveryDraw)
		FlushRenderingCommands();*/

	if (GEnvCubeList_DirtyCount == GEnvCubeList_UpdateCount)
		return;		

	FlushRenderingCommands();

	GEnvCubeList_UpdateCount = GEnvCubeList_DirtyCount;
	
	GEnvCubes.Empty();	

	if (GWorld)
	{
		for (FActorIterator It; It; ++It)
		{
			AActor* Actor = *It;

			AEnvCubeActor* EnvCube = Cast<AEnvCubeActor>( Actor );

			if (EnvCube)
			{					
				EnvCube->Texture = EnvCube_GetCubeTex( EnvCube );

				if (EnvCube->Texture != NULL)
				{					
					FEnvCubeEntry NewEntry;

					NewEntry.Location = EnvCube->Location;
					NewEntry.Texture = EnvCube->Texture ? EnvCube->Texture->Resource : NULL;

					GEnvCubes.AddItem( NewEntry );
				}					
			}		
		}
	}	
}

void EnvCube_Bind( FCommandContextRHI* Context, FPixelShaderRHIParamRef PixelShader, const FPrimitiveSceneInfo* PrimitiveSceneInfo )
{
	if (EnvCube_RegisterIndex < 0) return;	

	if (!PrimitiveSceneInfo->EnvCube)
	{
		FVector NewLocation = PrimitiveSceneInfo->Component->Bounds.Origin;
		EnvCube_SetLocation( NewLocation );
		PrimitiveSceneInfo->EnvCube = EnvCube_Resolve();
	}	

	const FTexture* Texture = PrimitiveSceneInfo->EnvCube;
	if (!Texture)
	{
		Texture = GWhiteTextureCube;
	}
	
	if (Texture)
	{
		Texture->LastRenderTime = GCurrentTime;
		RHISetSamplerState(Context,PixelShader,EnvCube_RegisterIndex,Texture->SamplerStateRHI,Texture->TextureRHI);	
	}	
}

void EnvCube_MarkDirty()
{
	GEnvCubeList_DirtyCount++;	
}