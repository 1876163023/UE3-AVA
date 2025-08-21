#include "avaLaunch.h"
#include "EngineAnimClasses.h"
#include "EngineAIClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineDecalClasses.h"
#include "EngineDSPClasses.h"
#include "EngineSequenceClasses.h"
#include "GameFrameworkClasses.h"
#if FINAL_RELEASE
#include "EngineUserInterfaceClasses.h"
#endif
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"
#if FINAL_RELEASE
#include "EngineParticleClasses.h"
#endif
#include "avaGameClasses.h"

static const INT Width = 101;
static const INT Height = 117;

static void DrawTile( FCanvas* Canvas, USurface* Surface, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& DrawColor )
{
	FLOAT mW = Surface->GetSurfaceWidth();
	FLOAT mH = Surface->GetSurfaceHeight();
	U /= mW; UL /= mW;
	V /= mH; VL /= mH;

	UTexture* Texture = Cast<UTexture>(Surface);
	if ( Texture != NULL )
	{
		FTexture* RawTexture = Texture->Resource;
		::DrawTile(Canvas, X, Y, XL, YL, U, V, UL, VL, DrawColor, RawTexture);
	}
	else
	{
		UMaterialInstance* Material = Cast<UMaterialInstance>(Surface);
		if ( Material != NULL )
		{
			::DrawTile(Canvas,X, Y, XL, YL, U, V, UL, VL, Material->GetInstanceInterface(0));
		}
	}
}

struct FAvaRatingInfo 
{
public :
	UTexture2D*		Texture;
	INT				Counter;
	FLOAT			StartTime;	
	FLOAT			TriggeredTime;
	FLOAT			Duration;
	UBOOL			bEnabled;

	FAvaRatingInfo()
		: Texture(NULL), Counter(0), StartTime(-1), TriggeredTime(-1), Duration(-1), bEnabled(FALSE)
	{
	}

	void Initialize()
	{		
		Texture = LoadObject<UTexture2D>( NULL, TEXT("avaRating.Rating"), NULL, LOAD_None, NULL );

		Texture->AddToRoot();
	}

	void Uninitialize()
	{
		//Texture->RemoveFromRoot();
	}

	void DrawRating( FCanvas* Canvas, INT X, INT Y, INT TileX, INT TileY, FLOAT Alpha )
	{		
		if (Texture != NULL)
		{
			DrawTile( Canvas, Texture, X, Y, Width, Height, TileX * 128, TileY * 128, Width, Height, FLinearColor( 1, 1, 1, Alpha ) );
		}		
	}

	void DrawLine( FCanvas* Canvas, INT X, INT Y, const INT* Items, INT HoriAdvance, INT VertAdvance, FLOAT Alpha )
	{
		for (;*Items >= 0; Items+=2)
		{			
			DrawRating( Canvas, X, Y, Items[0], Items[1], Alpha );

			X += HoriAdvance;
			Y += VertAdvance;
		}
	}

	void Render( FCanvas* Canvas, FLOAT Alpha )
	{
		const UBOOL bTeen = !GetAvaNetHandler()->IsPlayerAdult();

		if ( GetAvaNetHandler()->GetCurrentChannelMaskLevel() >= 2 )
			return;

		static const INT Padding = 10;
		static const INT Spacing = 10;
		static const INT StringPadding1 = 25;
		static const INT StringPadding2 = 5;

		const INT SizeX = Canvas->GetRenderTarget()->GetSizeX();
		const INT SizeY = Canvas->GetRenderTarget()->GetSizeY();

		INT TopY = Padding;
		INT BottomY = SizeY - Padding - Height;

		// 폭력성 0,1 공포 1,1 언어의 부적절성 2,1
		// 15세 이용가 1,0 18세 이용가 2,0		

		INT Tops[] = { bTeen ? 1 : 2, 0, -1 };
		INT Bottoms[] = { 0, 0, /*0, 1, 1, 1,*/ -1,-1 };

		DrawLine( Canvas, SizeX - Padding - Width, TopY, Tops, -( Width + Padding ), 0, Alpha );
		DrawLine( Canvas, Padding, BottomY, Bottoms, ( Width + Padding ), 0, Alpha );

		AavaGame* DefaultAvaGame = AavaGame::StaticClass()->GetDefaultObject<AavaGame>();
		// 이 게임은 몇세 이용가 입니다.
		// 만 몇세 미만의 청소년은 이용할 수 없습니다.
		INT XL, YL, YPos;
		const INT Hour = Counter - 1;
		if ( Hour == 0 )
		{
			FString Message1 = bTeen ? *DefaultAvaGame->GameRatingInformationTeen1 : *DefaultAvaGame->GameRatingInformation1;
			FString Message2 = bTeen ? *DefaultAvaGame->GameRatingInformationTeen2 : *DefaultAvaGame->GameRatingInformation2;

			StringSize( GEngine->LargeFont, XL, YL, *Message1 );
			//DrawString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color, FLOAT XScale=1.0, FLOAT YScale=1.0);
			YPos = TopY + Height + StringPadding1;
			DrawString( Canvas,	SizeX - XL - StringPadding1, YPos, *Message1, GEngine->LargeFont, FLinearColor( 0.627f, 0.654f, 0.678f, Alpha ) );
			YPos += YL + StringPadding2;
			StringSize( GEngine->LargeFont, XL, YL, *Message2 );
			DrawString( Canvas,	SizeX - XL - StringPadding1, YPos, *Message2, GEngine->LargeFont, FLinearColor( 0.627f, 0.654f, 0.678f, Alpha ) );
		}

		// 연령에 상관없이 메세지가 보여야함
		//if (!bTeen)
		//	return;

		FString Message = FString::Printf( *DefaultAvaGame->SafeGameRecommendation, Hour );
		FString MessageHR;

		if (Hour == 0)
		{
			Message = DefaultAvaGame->SafeGameRecommendation_0HR;
		}
		else if (Hour < 1)
		{
			return;
		}
		if (Hour < 3)
		{				
		}		
		else if (Hour == 3)
		{
			MessageHR = DefaultAvaGame->SafeGameRecommendation_3HR;
		}
		else if (Hour == 4)
		{
			MessageHR = DefaultAvaGame->SafeGameRecommendation_4HR;
		}
		else if (Hour == 5)
		{
			MessageHR = DefaultAvaGame->SafeGameRecommendation_5HR;
		}
		else if (Hour == 6)
		{
			MessageHR = DefaultAvaGame->SafeGameRecommendation_6HR;
		}
		else if (Hour < 12)
		{
			MessageHR = DefaultAvaGame->SafeGameRecommendation_7HR;
		}
		else if (Hour < 24)
		{
			MessageHR = DefaultAvaGame->SafeGameRecommendation_12HR;
		}
		else 
		{
			MessageHR = DefaultAvaGame->SafeGameRecommendation_24HR;
		}

		StringSize( GEngine->MediumFont, XL, YL, *Message );

		DrawString( Canvas,
			Padding,
			SizeY - Padding * 3 - Height - YL * 2,
			*Message,
			GEngine->MediumFont,
			FLinearColor( 1, 1, 1, Alpha )
			);

		DrawString( Canvas,
			Padding,
			SizeY - Padding * 2 - Height - YL,
			*MessageHR,
			GEngine->MediumFont,
			FLinearColor( 1, 1, 1, Alpha )
			);
	}

	void Tick( FCanvas* Canvas )
	{
		if (!bEnabled)
			return;

		const FLOAT CurrentTime = appSeconds();

		if (StartTime < 0)
		{
			StartTime = CurrentTime;
		}	

		// 새로 trigger될 조건을 판별
		if (CurrentTime > TriggeredTime + Duration)
		{
			if (Counter == 0 || CurrentTime > TriggeredTime + 3600 /*1 Hour */)
			{
				Duration = 3.0f;
				Counter++;
				TriggeredTime = CurrentTime;
			}
		}

		if (CurrentTime > TriggeredTime + Duration)
			return;

		FLOAT u = (CurrentTime - TriggeredTime) / Duration;

		static const FLOAT Fade = 0.125f;
		FLOAT Alpha = 1.0f;

		if (u < Fade)
		{
			Alpha = u/Fade;
		}
		else if (u > 1-Fade)
		{
			Alpha = (1-u) / Fade;
		}

		Render( Canvas, Alpha );		
	}
} GAvaRatingInfo;

void appRatingInfo_Initialize()
{
	if (GIsEditor || !GIsGame) return;

	GAvaRatingInfo.Initialize();
}

void appRatingInfo_Render( FCanvas* Canvas )
{
	if (GIsEditor || !GIsGame) return;

	GAvaRatingInfo.Tick( Canvas );
}

void appRatingInfo_Uninitialize()
{
	if (GIsEditor || !GIsGame) return;

	GAvaRatingInfo.Uninitialize();
}

void appRatingInfo_Enable( UBOOL bEnable )
{
	if (GIsEditor || !GIsGame) return;

	GAvaRatingInfo.bEnabled = bEnable;	
}