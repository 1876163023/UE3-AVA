#include "../avaLaunch.h"
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
#if !FINAL_RELEASE
#include "avaGameEditorClasses.h"
#endif
#include "UnTruetypeFont.h"
#include "hostMigration.h"

static UBOOL GDisableLoadingScreenThread;

struct FMessageItem
{
	FString		Message;
	FLOAT		MessageDeliveredTime;
	FName		Event;
};

class FLoadingScreenLog : public FOutputDevice
{
public :	
	FLoadingScreenLog()
	{
		CriticalSection = GSynchronizeFactory->CreateCriticalSection();
	}

	~FLoadingScreenLog()
	{
		delete CriticalSection;
	}

	virtual void Serialize( const TCHAR* V, EName Event ) 
	{
		if (Event == NAME_Error || Event == NAME_Warning)
		{
			FScopeLock ScopedLock(CriticalSection);

			FMessageItem Item;
			Item.Message = FString(V);
			Item.Event = Event;
			Item.MessageDeliveredTime = appSeconds();
			Messages.AddItem( Item );
		}		
	}

	TArray<FMessageItem> Messages;
	FCriticalSection* CriticalSection;
};

FLoadingScreenLog* GLoadingScreenLog = NULL;

static FRotator Rot( 0, 0, 0 );
static FLOAT Tau = 0.2f;

class FProgressContext
{
public :
	FLOAT			LastValue;
	FLOAT			TargetValue;
	FLOAT			SetTime;

	FProgressContext()
	{
		Reset();
	}

	void Reset()
	{
		LastValue = TargetValue = 0;
		SetTime = -1;
	}

	void Update( FLOAT DeltaTime )
	{
		static const FLOAT Speed = 30.0f;

		//debugf( NAME_Log, TEXT("%f %f"), LastValue, TargetValue ); // [-] 20070709 dEAthcURe 느린pc에서 avaGame.exe challenge안됨

		LastValue = Min( LastValue + DeltaTime * Speed, TargetValue );
	}
};

class FLoadingScreenTickable : public FTickableObject, public FDeferredCleanupInterface	
{
public :
	TMap<FName,FProgressContext> UserProgress;
	FProgressContext LoadingProgress;

	FLOAT LastTime;

	UBOOL bEnabled;
	FLOAT FakeTickTime;
	FLOAT StartTime;
	FLOAT AccumulatedTime;
	
	UFont* Font;
	UFont* SmallFont;
	TArray<FMessageItem> Messages;

	typedef TArray<FavaRoomPlayerInfo> PlayerListType;
	
	PlayerListType InternalCopy_RoomStartingPlayerList;

	INT ScanLineY;
	INT CurHmAnim;
	INT MaxHmAnims;
	FLOAT HmAnimTime;
	UBOOL bHmDrawFont;

	virtual void FinishCleanup()
	{
		delete this;		
	}

	void RenderThread_FakeTick( FLOAT Offset )
	{
		FLOAT CurTime = appSeconds();
		FLOAT Elapsed = FakeTickTime >= 0 ? CurTime - FakeTickTime : 1.0f;
		FakeTickTime = CurTime;

		Tick( Elapsed + Offset );
	}

	void GameThread_Enable( UBOOL bInEnable )
	{
		if ( bInEnable )
			bHmDrawFont = FALSE;

		FlushRenderingCommands();

		GameThread_Fetch();

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER( Enable, FLoadingScreenTickable*, This, this, UBOOL, bInEnable, bInEnable,
		{
			This->Enable_RenderThread( bInEnable );			

			This->RenderThread_FakeTick(bInEnable ? 0 : 1.0f);
		});				

		FlushRenderingCommands();
	}

	void Enable_RenderThread( UBOOL bInEnable )
	{
		if (bEnabled ^ bInEnable)
		{
			AccumulatedTime = 0.0f;

			bEnabled = bInEnable;

			if (bEnabled)
			{
				LastTime = -1;
				StartTime = appSeconds();
				Font = GEngine->GetMediumFont();
				SmallFont = GEngine->GetSmallFont();

				FakeTickTime = -1.0f;				
			}
			else
			{
				UserProgress.Empty();
				LoadingProgress.Reset();
			}
		}
	}

	void GameThread_SetProgress( FLOAT InProgress )
	{
		GameThread_Fetch();

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER( Enable, FLoadingScreenTickable*, This, this, FLOAT, InProgress, InProgress,
		{
			This->RenderThread_SetProgress( InProgress );			

			This->RenderThread_FakeTick(0.0f);
		});
	}

	void RenderThread_SetProgress( FLOAT InProgress )
	{
		LoadingProgress.TargetValue = InProgress * 100.0f;
		LoadingProgress.SetTime = appSeconds();
	}

	FLoadingScreenTickable()
		: FTickableObject(TRUE), bEnabled(FALSE), ScanLineY(-300), CurHmAnim(0), MaxHmAnims(4), HmAnimTime(0)
	{
	}

	virtual UBOOL IsTickableWhenPaused(void)
	{
		return TRUE;
	}

	virtual UBOOL IsTickable()
	{
		return TRUE;
	}

	void GameThread_Fetch()
	{		
		check( IsInGameThread() );

		FlushRenderingCommands();		

		InternalCopy_RoomStartingPlayerList.Reset();

		InternalCopy_RoomStartingPlayerList.Append( GetAvaNetHandler()->RoomStartingPlayerList );
	}

	void Render_Background( FCanvas* Canvas )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		check( Viewport && avaViewportClient );

		if ( avaViewportClient->CurrentLoadingInfo.bProgress )
		{
			// 화면을 검은색으로 채운다.
			DrawTile(Canvas, 0, 0, Viewport->GetSizeX(), Viewport->GetSizeY(), 0, 0, 1, 1, FLinearColor::Black,NULL,FALSE);

			FVector2D Offset(0,0);

			// Draw a BackgroundImage
			const FLoadingImageData &Background = avaViewportClient->CurrentLoadingInfo.Background;
			if (Background.Image != NULL)
			{
				UTexture2D* Tex = Background.Image;

				// 가운데 정렬.
				Offset = FVector2D( 0.5f * (Viewport->GetSizeX() - Background.UL), 0.5f * (Viewport->GetSizeY() - Background.VL) );

				DrawTile( 
					Canvas, 
					Offset.X, Offset.Y, Background.UL, Background.VL, 
					Background.U/Tex->SizeX,
					Background.V/Tex->SizeY,
					Background.UL/Tex->SizeX,
					Background.VL/Tex->SizeY,
					FLinearColor::White,
					Tex->Resource,
					FALSE );
			}

			const FLoadingImageData &Progress = avaViewportClient->CurrentLoadingInfo.PostImage1;
			if ( Progress.Image != NULL )
			{
				UTexture2D* Tex = Progress.Image;

				float UL = LoadingProgress.LastValue / 100 * Progress.UL;

				DrawTile( 
					Canvas, 
					Offset.X + Progress.DrawPos.X, Offset.Y + Progress.DrawPos.Y, UL, Progress.VL, 
					Progress.U/Tex->SizeX,
					Progress.V/Tex->SizeY,
					UL/Tex->SizeX,
					Progress.VL/Tex->SizeY,
					FLinearColor::White,
					Tex->Resource,
					TRUE );
			}

			const FLoadingImageData &Tip = avaViewportClient->TipBar;
			if ( Tip.Image != NULL )
			{
				UTexture2D* Tex = Tip.Image;

				FVector2D TipUVRatio((float)Viewport->GetSizeX()/1024, (float)Viewport->GetSizeY()/768);

				DrawTile( 
					Canvas, 
					TipUVRatio.X * Tip.DrawPos.X, TipUVRatio.Y * Tip.DrawPos.Y, Tip.UL * TipUVRatio.X, Tip.VL * TipUVRatio.Y, 
					Tip.U/Tex->SizeX, Tip.V/Tex->SizeY, Tip.UL/Tex->SizeX, Tip.VL/Tex->SizeY,
					FLinearColor::White,Tex->Resource,FALSE );
			}
		}
		else
		{
			// Draw a BackgroundImage
			const FLoadingImageData &Background = avaViewportClient->CurrentLoadingInfo.Background;
			if (Background.Image != NULL)
			{
				UTexture2D* Tex = Background.Image;

				DrawTile( 
					Canvas, 
					0, 0, Viewport->GetSizeX(), Viewport->GetSizeY(), 
					Background.U/Tex->SizeX,
					Background.V/Tex->SizeY,
					Background.UL/Tex->SizeX,
					Background.VL/Tex->SizeY,
					FLinearColor( 1.0f, 1.0f, 1.0f, 1.0f ),
					Tex->Resource,
					FALSE );
			}

			const FLoadingImageData &AVALogo = avaViewportClient->AVALogo;
			if ( AVALogo.Image != NULL )
			{
				UTexture2D* Tex = AVALogo.Image;

				DrawTile( 
					Canvas, 
					AVALogo.DrawPos.X, AVALogo.DrawPos.Y, AVALogo.UL, AVALogo.VL, 
					AVALogo.U/Tex->SizeX,
					AVALogo.V/Tex->SizeY,
					AVALogo.UL/Tex->SizeX,
					AVALogo.VL/Tex->SizeY,
					FLinearColor::White,
					Tex->Resource, TRUE);
			}
		}
	}


	void Render_ProgressBar( FCanvas* Canvas, FLOAT DeltaTime )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		const FLoadingImageData &ProgressBar0 = avaViewportClient->ProgressBar0;
		const FLoadingImageData &ProgressBar1 = avaViewportClient->ProgressBar1;
		const FLoadingImageData &ProgressBar2 = avaViewportClient->ProgressBar2;

		// 256보다도 작다면... 걍 그리지 말아버리자...
		if ( Viewport->GetSizeX() < 256 )
			return ;

		// 데이터가 없어도 취소.
		if ( ProgressBar0.Image == NULL || 
			 ProgressBar1.Image == NULL || 
			 ProgressBar2.Image == NULL )
			 return ;

		// 3개의 이미지는 동일한 크기.
		INT SizeX = ProgressBar0.Image->SizeX;
		INT SizeY = ProgressBar0.Image->SizeY;

		const INT BarWidth = 4;
		const INT BarHeight = 27;

		INT CenterSize = Viewport->GetSizeX() - SizeX * 2;
		INT dummySize = CenterSize % BarWidth;
		INT numBlocks = appFloor((CenterSize-dummySize) / SizeX);
		INT modSizeX = CenterSize - numBlocks * SizeX;

		// left
		DrawTile( Canvas, 0, Viewport->GetSizeY() - SizeY, SizeX, SizeY, 
				  ProgressBar0.U/SizeX, ProgressBar0.V/SizeY, ProgressBar0.UL/SizeX, ProgressBar0.VL/SizeY, 
				  FLinearColor::White, ProgressBar0.Image->Resource );
		// right
		DrawTile( Canvas, Viewport->GetSizeX() - SizeX - dummySize, Viewport->GetSizeY() - SizeY, SizeX, SizeY, 
				  ProgressBar2.U/SizeX, ProgressBar2.V/SizeY, ProgressBar2.UL/SizeX, ProgressBar2.VL/SizeY, 
				  FLinearColor::White, ProgressBar2.Image->Resource );


		// 블럭 개수만큼 그리고
		INT i;
		for(i = 0; i < numBlocks; i++)
		{
			UTexture2D* Tex = ProgressBar1.Image;

			DrawTile( Canvas, SizeX + SizeX * i, Viewport->GetSizeY() - SizeY, SizeX, SizeY, 
					  ProgressBar1.U/SizeX, ProgressBar1.V/SizeY, ProgressBar1.UL/SizeX, ProgressBar1.VL/SizeY, 
					  FLinearColor::White, Tex->Resource );
		}

		// 남은 부분을 그리고.
		if ( modSizeX != 0 )
		{
			UTexture2D* Tex = ProgressBar1.Image;

			DrawTile( Canvas, SizeX + SizeX * i, Viewport->GetSizeY() - SizeY, modSizeX, SizeY, 
				ProgressBar1.U/SizeX, ProgressBar1.V/SizeY, (float)modSizeX/SizeX, ProgressBar1.VL/SizeY, 
				FLinearColor::White, Tex->Resource );
		}

		INT maxBars = 20 * 2 + CenterSize / BarWidth;
		INT numBars = appTrunc(LoadingProgress.LastValue * (maxBars+1) / 100);

		const FVector2D Offset(48, 80);

		FVector2D Pos(0, Viewport->GetSizeY() - SizeY);
		Pos += Offset;

		for(INT i = 0; i < numBars; i++)
		{
			DrawTile( Canvas, Pos.X, Pos.Y, BarWidth - 1, BarHeight, 0, 0, 1, 1, FLinearColor::White );

			Pos.X += BarWidth;
		}
	}

	void Render_Footer( FCanvas* Canvas, FLOAT DeltaTime )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		check( Viewport && avaViewportClient );
		
		FString Message = FString::Printf( TEXT("%s"), *avaViewportClient->NowLoading );

		INT XL, YL;
		StringSize( Font, XL, YL, *Message );
		
		INT Y = (Viewport->GetSizeY() - YL * 2);

		// Now loading
		DrawString( Canvas, Viewport->GetSizeX() - XL * 1.5f, Y, *Message, Font, FLinearColor::White );
		Y -= YL;

		// Render progress bar
		{
			LoadingProgress.Update( DeltaTime );

			INT X = YL * 2;
			INT XL = (Viewport->GetSizeX()) - YL * 4;
			INT Thickness = 3;
			INT Spacing = 1;
			INT N = appTrunc(LoadingProgress.LastValue / 100 * XL / (Thickness + Spacing));
			//INT N = appTrunc(LoadingProgress.TargetValue / 100 * XL / (Thickness + Spacing));

			const FLOAT ElapsedTime = appSeconds() - LoadingProgress.SetTime;
			static const FLinearColor SteadyColor( 0.66f, 0.95f, 0.93f, 0.75f  );
			FLinearColor OngoingColor( 1, 1, 1, 0.75f );

			OngoingColor = Lerp( SteadyColor, OngoingColor, appExp( -ElapsedTime / Tau ) );

			for (INT i=0; i<N; i++)
			{
				DrawTile( Canvas, X, Y, Thickness, YL, 0, 0, 1, 1, OngoingColor );

				X += Thickness;
				X += Spacing;
			}				
		}

		// MOTD
		static const INT Padding = 2;
		Message = avaViewportClient->MOTD[ avaViewportClient->MOTDIndex % ARRAY_COUNT(avaViewportClient->MOTD) ];				
		StringSize( Font, XL, YL, *Message );
		DrawTile( Canvas, 1.0f * (Viewport->GetSizeX() - XL) - Padding, Y - Padding, XL + Padding * 2, YL + Padding * 2, 0, 0, 1, 1, FLinearColor( 0, 0, 0, 1.0f ) );
		DrawString( Canvas, 1.0f * (Viewport->GetSizeX() - XL), Y, *Message, Font, FLinearColor::White );
		Y -= YL;
	}

	void Render_Tip( FCanvas* Canvas )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		check( Viewport && avaViewportClient );

		INT XL, YL;

		FString Message = avaViewportClient->MOTD[ avaViewportClient->MOTDIndex % ARRAY_COUNT(avaViewportClient->MOTD) ];				
		StringSize( Font, XL, YL, *Message );

		static const FVector2D TipUV(75.0f/1024, 678.0f/768);
		DrawString( Canvas, (float)Viewport->GetSizeX() * TipUV.X, (float)Viewport->GetSizeY() * TipUV.Y - YL, 
					*Message, Font, FLinearColor::White );
	}

	void Render_Log( FCanvas* Canvas )
	{
		if (!GLoadingScreenLog) return;
		
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		check( Viewport && avaViewportClient );

		INT XL, YL;
		StringSize( Font, XL, YL, TEXT("Qj") );

		INT Y = (Viewport->GetSizeY() - YL * 5);

		/// Fetch messages from Log			
		{
			FScopeLock ScopedLock( GLoadingScreenLog->CriticalSection );

			Messages.Append( GLoadingScreenLog->Messages );
			GLoadingScreenLog->Messages.Empty();
		}			

		for (INT i=0; i<Messages.Num() && Y > 0; ++i)
		{
			const FMessageItem& Item = Messages(Messages.Num()-i-1);				

			StringSize( SmallFont, XL, YL, *Item.Message);
			DrawString( Canvas, 8, Y, *Item.Message, SmallFont, Item.Event == NAME_Error ? FLinearColor(1,0,0,1.0f) : FLinearColor(1,1,0,1.0f) );

			Y -= YL;
		}
	}		

	void Render_Users( FCanvas* Canvas, FLOAT DeltaTime )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		check( Viewport && avaViewportClient );

		FString Message;
		INT XL, YL;
		StringSize( Font, XL, YL, TEXT("일격에부처님곁으로땡") );

		INT Spacing = 8;
		INT NickWidth = Max( XL, 100 );

		INT BarWidth = NickWidth;

		INT Margin = Viewport->GetSizeX() % 4 + 44;

		INT X = Viewport->GetSizeX() - (BarWidth + Spacing + NickWidth + Margin);
		INT Y = Margin;

		const FLOAT CurrentTime = appSeconds();		
		static const INT Height = Max(YL / 2, 8);
		static const FLinearColor BackgroundColor( 0.36f, 0.95f, 0.43f, 1.0f );
		static const FLinearColor ForegroundColor( 0.66f, 0.95f, 0.93f, 1.0f );
		static const FLinearColor ActiveForegroundColor( 0.66f, 0.95f, 0.93f, 1.0f );
		static const FLinearColor ProgressColor(1.0f,1.0f,1.0f,0.2f);

		for (INT i = 0; i < InternalCopy_RoomStartingPlayerList.Num(); ++i)
		{
			if (InternalCopy_RoomStartingPlayerList(i).LoadingProgress >= 0 && InternalCopy_RoomStartingPlayerList(i).LoadingProgress < 100)
			{
				const FString& NickName = InternalCopy_RoomStartingPlayerList(i).NickName;
				FName& NickFName = InternalCopy_RoomStartingPlayerList(i).NickFName;
				INT NewProgress = InternalCopy_RoomStartingPlayerList(i).LoadingProgress;				

				StringSize( Font, XL, YL, *NickName);
				DrawString( Canvas, X + NickWidth - XL, Y, *NickName, Font, FLinearColor::White );
				
				UBOOL bNewData = TRUE;
				FLOAT ElapsedTime = 0.0f;
				
				FProgressContext* pProgress = UserProgress.Find( NickFName );
				if (pProgress)
				{
					// {{ 20080103 dEAthcURe
					if(pProgress->LastValue > NewProgress) { // 초기화
						pProgress->SetTime = CurrentTime;
						pProgress->LastValue = 0.0f;
					}
					else {
						pProgress->TargetValue = NewProgress;
						ElapsedTime = CurrentTime - pProgress->SetTime;
					}
					// }} 20080103 dEAthcURe

					/*bNewData = pProgress->TargetValue != NewProgress;
					if (!bNewData)
					{
						ElapsedTime = CurrentTime - pProgress->SetTime;
					}*/
				}				

				if (!pProgress) // if (bNewData)
				{
					FProgressContext NewEntry;
					NewEntry.TargetValue = NewProgress;
					NewEntry.SetTime = CurrentTime;

					pProgress = &(UserProgress.Set( NickFName, NewEntry ));
				}

				check( pProgress );

				pProgress->Update( DeltaTime );

				INT XP = X + Spacing + NickWidth;
				INT YP = Y + (YL - Height) / 2;												

				DrawTile( Canvas, XP, YP, 1, Height, 0, 0, 1, 1, ProgressColor );
				DrawTile( Canvas, XP+BarWidth, YP, 1, Height, 0, 0, 1, 1, ProgressColor );
				DrawTile( Canvas, XP, YP, BarWidth, 1, 0, 0, 1, 1, ProgressColor );
				DrawTile( Canvas, XP, YP+Height-1, BarWidth, 1, 0, 0, 1, 1, ProgressColor );

				DrawTile( Canvas, XP, YP, BarWidth * pProgress->LastValue / 100, Height, 0, 0, 1, 1, Lerp( ForegroundColor, ActiveForegroundColor, appExp( -ElapsedTime / Tau ) ) );

				Y += YL;
			}
		}
	}

	void Render_Animation( FCanvas* Canvas )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		check( Viewport && avaViewportClient );

		FLOAT Seconds = appSeconds() - StartTime;

		// Draw LoadingIcons;
		check( avaViewportClient->LoadingIcons.Num() == avaViewportClient->LoadingIconData.Num() );
		for( INT i = 0 ; i < avaViewportClient->LoadingIcons.Num(); i++ )
		{
			const FLoadingIconInfo& IconInfo = avaViewportClient->LoadingIconData(i);
			if( avaViewportClient->LoadingIcons(i) == NULL || avaViewportClient->LoadingIcons(i)->Resource == NULL ||
				IconInfo.FirstIconCoords.IsZero() || 
				IconInfo.IconDimension.X <= 0 || IconInfo.IconDimension.Y <= 0 ||
				IconInfo.AnimationPeriod <= 0.f || 
				IconInfo.AnimationImageCount <= 0 ||
				IconInfo.AnimationImageCount > (IconInfo.IconDrawXY.X * IconInfo.IconDrawXY.Y) )
				continue;

			INT Period = appFloor(IconInfo.AnimationPeriod * 1000.f);
			FLOAT CurrentPos = (appFloor(Seconds*1000.f)  % Period ) / (FLOAT)Period;
			INT IconIndex = Clamp( appFloor( CurrentPos * IconInfo.AnimationImageCount ) ,0, IconInfo.AnimationImageCount - 1);
			FLOAT IconWidth = IconInfo.FirstIconCoords.UL;
			FLOAT IconHeight = IconInfo.FirstIconCoords.VL;
			FTexture* Tex = avaViewportClient->LoadingIcons(i)->Resource;
			FLOAT DrawX = IconInfo.IconDrawXY.X >= 0 ? IconInfo.IconDrawXY.X : Viewport->GetSizeX() + IconInfo.IconDrawXY.X;
			FLOAT DrawY = IconInfo.IconDrawXY.Y >= 0 ? IconInfo.IconDrawXY.Y : Viewport->GetSizeY() + IconInfo.IconDrawXY.Y;

			DrawTile( Canvas, DrawX, DrawY, IconWidth, IconHeight, 
				(IconInfo.FirstIconCoords.U + (IconIndex % IconInfo.IconDimension.X) * IconWidth)/Tex->GetSizeX(),
				(IconInfo.FirstIconCoords.V + (IconIndex / IconInfo.IconDimension.X) * IconHeight)/Tex->GetSizeY(), IconWidth/Tex->GetSizeX(), IconHeight/Tex->GetSizeY(), FLinearColor::White, avaViewportClient->LoadingIcons(i)->Resource);
		}
	}

	void Render_ScanLine( FCanvas* Canvas, FLOAT DeltaTime )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		const FLoadingImageData &ScanLine = avaViewportClient->ScanLine;

		if ( avaViewportClient->ScanLine.Image == NULL )
			return ;

		ScanLineY -= Min(appFloor(DeltaTime * 50), 3);
		if ( ScanLineY < -256 )
			ScanLineY = Viewport->GetSizeY();

		DrawTile(Canvas, 0, ScanLineY, Viewport->GetSizeX(), ScanLine.Image->SizeY, 
			     ScanLine.U, ScanLine.V, ScanLine.UL/ScanLine.Image->SizeX, ScanLine.VL/ScanLine.Image->SizeY, 
				 FLinearColor::White, ScanLine.Image->Resource);
	}

	void Render_Sponsor( FCanvas* Canvas )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		FLOAT X = -1, Y = -1;

		for (INT InfoIndex=0; InfoIndex<avaViewportClient->CurrentSponsorInfos.Num(); ++InfoIndex)
		{
			const FLoadingImageData& Data = avaViewportClient->CurrentSponsorInfos(InfoIndex);
			
			if (Data.Image != NULL)
			{
				if (X < 0)
				{
					X = Viewport->GetSizeX() - Data.DrawPos.X - Data.UL;
					Y = Viewport->GetSizeY() - Data.DrawPos.Y - Data.VL;
				}

				DrawTile(Canvas, 
					X, Y,
					Data.UL, Data.VL, 					
					Data.U/Data.Image->SizeX, Data.V/Data.Image->SizeY, Data.UL/Data.Image->SizeX, Data.VL/Data.Image->SizeY, 
					FLinearColor::White, Data.Image->Resource);

				X -= Data.Image->SizeX - 16;
			}
		}
	}

	void Render_PostImage( FCanvas* Canvas )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		// 프로그레스로 사용된다면 이미 Background에서 출력되었을 것이다.
		if ( avaViewportClient->CurrentLoadingInfo.bProgress )
			return ;

		const FLoadingImageData &ImageData1 = avaViewportClient->CurrentLoadingInfo.PostImage1;
		if ( ImageData1.Image != NULL )
		{
			UTexture2D* Tex = ImageData1.Image;

			// 현재 1024x768의 좌표를 Viewport상의 좌표로 변환.
			float x = (float)Viewport->GetSizeX() * (float)ImageData1.DrawPos.X / 1024.0f;
			float y = (float)Viewport->GetSizeY() * (float)ImageData1.DrawPos.Y / 768.0f;

			DrawTile( 
				Canvas, x, y, ImageData1.UL, ImageData1.VL,
				ImageData1.U/Tex->SizeX, ImageData1.V/Tex->SizeY, ImageData1.UL/Tex->SizeX, ImageData1.VL/Tex->SizeY,
				FLinearColor::White, Tex->Resource);
		}

		const FLoadingImageData &ImageData2 = avaViewportClient->CurrentLoadingInfo.PostImage2;
		if ( ImageData2.Image != NULL )
		{
			UTexture2D* Tex = ImageData2.Image;

			// 현재 1024x768의 좌표를 Viewport상의 좌표로 변환.
			float x = (float)Viewport->GetSizeX() * (float)ImageData2.DrawPos.X / 1024.0f;
			float y = (float)Viewport->GetSizeY() * (float)ImageData2.DrawPos.Y / 768.0f;

			DrawTile( 
				Canvas, x, y, ImageData2.UL, ImageData2.VL,
				ImageData2.U/Tex->SizeX, ImageData2.V/Tex->SizeY, ImageData2.UL/Tex->SizeX, ImageData2.VL/Tex->SizeY,
				FLinearColor::White, Tex->Resource);
		}
	}

	//! Host Migration중에 출력되는 함수.
	void Render_HostMigration( FCanvas* Canvas, FLOAT DeltaTime )
	{
		FViewport* Viewport = GEngine->GameViewport->Viewport;
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		const INT PanelW = 328;
		const INT PanelH = 454;
		FLOAT PanelX = 0.5f * (Viewport->GetSizeX() - PanelW);
		FLOAT PanelY = 0.5f * (Viewport->GetSizeY() - PanelH);

		// 현재 애니메이션 프레임 설정.
		HmAnimTime += DeltaTime;
		if ( HmAnimTime > 0.2f )
		{
			if ( ++CurHmAnim >= MaxHmAnims )
				CurHmAnim = 0;

			HmAnimTime = 0;
		}

		// 애니메이션.
		const FLoadingImageData &HmAnim = avaViewportClient->HmAnim;
		if ( HmAnim.Image != NULL )
		{
			UTexture2D* Tex = HmAnim.Image;
			const FVector2D uvOffsets[4] =
			{
				FVector2D(  0,   0),
				FVector2D(173,   0),
				FVector2D(  0, 108),
				FVector2D(173, 108),
			};

			// 현재 1024x768의 좌표를 Viewport상의 좌표로 변환.
			float x = PanelX + 78;
			float y = PanelY + 39;

			DrawTile( Canvas, x, y, HmAnim.UL, HmAnim.VL,
				(uvOffsets[CurHmAnim].X + HmAnim.U)  / Tex->SizeX, 
				(uvOffsets[CurHmAnim].Y + HmAnim.V)  / Tex->SizeY, 
				HmAnim.UL / Tex->SizeX, 
				HmAnim.VL / Tex->SizeY,
				FLinearColor::White, Tex->Resource);
		}

//! 배경에 남아있는 HmScene덕분에 글자는 찍지 않아도 될 것 같다.
/*
		if ( !bHmDrawFont )
		{
			FString MigrationMsg = avaViewportClient->HostMigrationMessage;
			INT XL, YL;
			StringSize( Font, XL, YL, *MigrationMsg );

			FLOAT X = PanelX;
			FLOAT Y = PanelY + 160 + 4;

			// 배경
//			DrawTile( Canvas, X, Y, XL, YL, 0, 0, 1, 1, FLinearColor( 1, 1, 1, 1.0f ) );

			// 글자
			DrawString( Canvas, X, Y, *MigrationMsg, Font, FLinearColor::White );

			bHmDrawFont = TRUE;
		}
*/
	}

	void Render()
	{
		FViewportClient* ViewportClient = GEngine->GameViewport;
		if (!ViewportClient) 
		{
			return;
		}

		FViewport* Viewport = GEngine->GameViewport->Viewport;
		if (!Viewport) 
		{
			return;
		}

		FViewportRHIRef ViewportRHI = Viewport->GetViewportRHI();
		UavaGameViewportClient* avaViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);

		if( !IsValidRef(ViewportRHI) && !avaViewportClient )
		{			
			return;
		}

		const FLOAT CurrentTime = appSeconds();
		const FLOAT DeltaTime = LastTime < 0 ? 0 : CurrentTime - LastTime;
		LastTime = CurrentTime;

		if (!(( g_hostMigration.state == hmsNewHost ) ||
			  ( g_hostMigration.state == hmsHostRestoring ) ||
			  ( g_hostMigration.state == hmsNewClientPrepare ) ||
			  ( g_hostMigration.state == hmsNewClient ) || // [+] 20070528
			  ( g_hostMigration.state == hmsNewClientLoaded )))
		{
			RHIBeginDrawingViewport(Viewport->GetViewportRHI());				
			RHISetRenderTarget(NULL,Viewport->RenderTargetSurfaceRHI,FSurfaceRHIRef());
			RHISetViewport(NULL,0.0f,0.0f,0.0f,Viewport->GetSizeX(),Viewport->GetSizeY(),1.0f);
			RHIClear(NULL,TRUE,FLinearColor( 0, 0, 0 ), FALSE,1.0f,FALSE,0);
			FCanvas Canvas( Viewport, NULL );

			Canvas.PushRelativeTransform( FRotationMatrix( Rot ) );

			LoadingProgress.Update(DeltaTime);

			Render_Background( &Canvas );
//			Render_Footer( &Canvas, DeltaTime );
			Render_ProgressBar( &Canvas, DeltaTime );
			Render_Tip( &Canvas );
			Render_Log( &Canvas );			
			Render_Users( &Canvas, DeltaTime );
//			Render_Animation( &Canvas );
			Render_ScanLine( &Canvas, DeltaTime );
			Render_PostImage( &Canvas );
			Render_Sponsor( &Canvas );
			
			Canvas.Flush();

			RHIEndDrawingViewport(Viewport->GetViewportRHI(),TRUE,FALSE);				
		}
		else
		{
			RHIBeginDrawingViewport(Viewport->GetViewportRHI());				
			RHISetRenderTarget(NULL,Viewport->RenderTargetSurfaceRHI,FSurfaceRHIRef());
			RHISetViewport(NULL,0.0f,0.0f,0.0f,Viewport->GetSizeX(),Viewport->GetSizeY(),1.0f);

			FCanvas Canvas( Viewport, NULL );

			// HostMigration Animation.
			Render_HostMigration( &Canvas, DeltaTime );

			Canvas.Flush();
			RHIEndDrawingViewport(Viewport->GetViewportRHI(),TRUE,FALSE);				
		}
	}

	virtual void Tick( FLOAT DeltaTime )
	{		
		if (!bEnabled)
			return;

		// {{ 20070202 dEAthcURe
		extern FD3DViewport* GD3DDrawingViewport;
		if(GD3DDrawingViewport) {
			return;
		}
		// }} 20070202 dEAthcURe

		if (!GEngine) return;

		AccumulatedTime += DeltaTime;

		// 일정 시간이 지날 때마다 그린다.(너무 자주 그리지 않기 위해서 사용)
		if (AccumulatedTime > 1/20.0f)
		{
			AccumulatedTime = 0.0f;

			Render();
		}
	}
};

static FLoadingScreenTickable* GLoadingScreenTickableObject = NULL;

static void PrepareTrueTypeFont( UFont* Font, FString Message )
{
	for (INT i=0; i<Message.Len(); ++i)
		Font->PrepareOntheFly(Message[i]);
}

UBOOL GIsLoadingOnProgress = FALSE;

class FLoadingScreen : public FCallbackEventDevice, public FTickableObject
{
public :	
	FLoadingScreen()
		: bThreadIsRunning(FALSE)
	{
	}

	virtual UBOOL IsTickableWhenPaused(void)
	{
		return TRUE;
	}

	virtual UBOOL IsTickable()
	{
		return TRUE;
	}

	virtual void Tick( FLOAT DeltaTime )
	{		
		if (GIsLoadingOnProgress)
		{
			debugf( NAME_Log, TEXT("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% END"));
			GIsLoadingOnProgress = FALSE;
		}		
	}

	virtual void Send( ECallbackEventType InType, const FString& InString, UObject* InObject )	
	{
		if (InType == CALLBACK_PreLoadMap)
		{
			debugf( NAME_Log, TEXT("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% BEGIN"));
			GIsLoadingOnProgress = TRUE;
			Activate( InString );
		}
	}

	virtual void Send( ECallbackEventType InType )	
	{
		if (InType == CALLBACK_ReadyToPlay)
		{
			Deactivate();
		}

		if (InType == CALLBACK_PlayerListUpdated)
		{
			GLoadingScreenTickableObject->GameThread_Fetch();
		}
	}	

	void Activate( const FString& InString )
	{		
		void FlushTrueTypeFontPages();

		FlushTrueTypeFontPages();

		if (GWorld)
		{	
			FlushRenderingCommands();

			// 왜!! 켜져 있어!! -_-
			if( GLoadingScreenTickableObject->bEnabled )
			{
				GLoadingScreenTickableObject->GameThread_Enable( FALSE );			

				FlushRenderingCommands();
			}

			debugf(NAME_Log, TEXT("### avaLoadingScreen::Init" ) );

#if !FINAL_RELEASE
			GLoadingScreenLog = new FLoadingScreenLog;

			GLog->AddOutputDevice( GLoadingScreenLog );
#endif

			// Show mouse cursor.
			//			while( ::ShowCursor(TRUE)<0 );

			UavaGameViewportClient* ViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);
			if (ViewportClient)
				ViewportClient->eventInitialize( InString );			

			UFont* Font = GEngine->GetMediumFont();

			if (ViewportClient)
			{
				PrepareTrueTypeFont( Font, ViewportClient->HostMigrationMessage );
				PrepareTrueTypeFont( Font, ViewportClient->NowLoading );
				PrepareTrueTypeFont( Font, ViewportClient->MOTD[ ViewportClient->MOTDIndex % ARRAY_COUNT(ViewportClient->MOTD) ] );
			}			

			for (INT i = 0; i < GetAvaNetHandler()->RoomStartingPlayerList.Num(); ++i)
			{
				if (GetAvaNetHandler()->RoomStartingPlayerList(i).LoadingProgress >= 0 && GetAvaNetHandler()->RoomStartingPlayerList(i).LoadingProgress < 100)
				{
					PrepareTrueTypeFont( Font, GetAvaNetHandler()->RoomStartingPlayerList(i).NickName );					
				}
			}			

			/*UFont* SmallFont = GEngine->GetTinyFont();
			SmallFont->PrepareOntheFly('X');*/


			UTrueTypeFont* TTF = Cast<UTrueTypeFont>( Font );
			if (TTF)
			{
				check(IsInGameThread());

				// game thread에서만 font를 추가할 수 있으므로 여기서 lock flag를 올리는 것은 안전 :)
				TTF->bLocked = TRUE;
			}

			GLoadingScreenTickableObject->GameThread_Enable( TRUE );			
		}
	}

	void Deactivate()
	{
		if (GLoadingScreenTickableObject->bEnabled)
		{
			GLoadingScreenTickableObject->GameThread_Enable( FALSE );

#if !FINAL_RELEASE
			GLog->RemoveOutputDevice( GLoadingScreenLog );

			delete GLoadingScreenLog;
			GLoadingScreenLog = NULL;
#endif

			UavaGameViewportClient* ViewportClient = Cast<UavaGameViewportClient>(GEngine->GameViewport);
			if (ViewportClient)
				ViewportClient->eventUninitialize();

			UFont* Font = GEngine->GetMediumFont();
			UTrueTypeFont* TTF = Cast<UTrueTypeFont>( Font );
			if (TTF)
			{
				check( IsInGameThread() );

				FlushRenderingCommands();

				TTF->bLocked = FALSE;
			}

			// Hide mouse cursor.
			//			while( ::ShowCursor(FALSE)<0 );

			debugf(NAME_Log, TEXT("### avaLoadingScreen::Free" ) );
		}		
	}

	virtual void Send( ECallbackEventType InType, DWORD InFlag )	{}
	virtual void Send( ECallbackEventType InType, const FVector& InVector ) 
	{
		if (InType == CALLBACK_LoadingProgress)
		{
			GLoadingScreenTickableObject->GameThread_SetProgress( InVector.Y > 0 ? InVector.X / InVector.Y : 0.0f );
		}
	}
	virtual void Send( ECallbackEventType InType, class FEdMode* InMode ) {}
	virtual void Send( ECallbackEventType InType, UObject* InObject ) {}
	virtual void Send( ECallbackEventType InType, class FViewport* InViewport, UINT InMessage) {}	

	UBOOL						bThreadIsRunning;
};

static FLoadingScreen* GLoadingScreen = NULL;

void avaLoadingScreen_Init()
{
	if (GIsUCC)
		return;

	GLoadingScreen = new FLoadingScreen;

	GCallbackEvent->Register( CALLBACK_PreLoadMap, GLoadingScreen );
	GCallbackEvent->Register( CALLBACK_ReadyToPlay, GLoadingScreen );
	GCallbackEvent->Register( CALLBACK_PlayerListUpdated, GLoadingScreen );
	GCallbackEvent->Register( CALLBACK_LoadingProgress, GLoadingScreen );

	ENQUEUE_UNIQUE_RENDER_COMMAND( CreateLoadingScreen, 
	{
		GLoadingScreenTickableObject = new FLoadingScreenTickable;
	}
	);	
}

void avaLoadingScreen_Free()
{
	if (GIsUCC)
		return;

	GCallbackEvent->Unregister( CALLBACK_PreLoadMap, GLoadingScreen );
	GCallbackEvent->Unregister( CALLBACK_ReadyToPlay, GLoadingScreen );	
	GCallbackEvent->Unregister( CALLBACK_PlayerListUpdated, GLoadingScreen );
	GCallbackEvent->Unregister( CALLBACK_LoadingProgress, GLoadingScreen );	

	/*BeginCleanup( GLoadingScreenTickableObject );
	GLoadingScreenTickableObject = NULL;

	FlushRenderingCommands();

	delete GLoadingScreen;*/
}
