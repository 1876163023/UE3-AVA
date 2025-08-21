#include "avaGame.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(AavaHUD)
IMPLEMENT_CLASS(UavaUIRadar)
IMPLEMENT_CLASS(UavaUIProgressBar)
IMPLEMENT_CLASS(UavaUIHealthGauge)
IMPLEMENT_CLASS(UavaUIArmorGauge)
IMPLEMENT_CLASS(UavaUIHelmet)
IMPLEMENT_CLASS(UavaUIDeathLog)
IMPLEMENT_CLASS(UavaUISimpleText)
IMPLEMENT_CLASS(UavaUIClock)
IMPLEMENT_CLASS(UavaUITargetName)
IMPLEMENT_CLASS(UavaUIGameDigits)
IMPLEMENT_CLASS(AavaKProj_SmokeBomb)
IMPLEMENT_CLASS(AavaThrowableWeapon)
IMPLEMENT_CLASS(AavaWeap_BaseMissionObject)
IMPLEMENT_CLASS(AavaWeap_Binocular)
IMPLEMENT_CLASS(AavaUseVolume)
IMPLEMENT_CLASS(UavaUIUseInfo)
IMPLEMENT_CLASS(UavaUIInfoMessage)
IMPLEMENT_CLASS(UavaUIGameInfoMessage)
IMPLEMENT_CLASS(UavaUIAmmoGraph)
IMPLEMENT_CLASS(UUIEvent_AmmoGraph)
IMPLEMENT_CLASS(UavaUIWeaponMenu)
IMPLEMENT_CLASS(UavaUIThrowableIcon)
IMPLEMENT_CLASS(AavaInventoryManager)
IMPLEMENT_CLASS(AavaPickup)
IMPLEMENT_CLASS(UavaUIKOTH3)

IMPLEMENT_CLASS(UavaUISpecialInventory)
IMPLEMENT_CLASS(UavaUIMissionIcon)
IMPLEMENT_CLASS(UavaUIDamageIndicator)
IMPLEMENT_CLASS(UavaUIConsoleMessage)
IMPLEMENT_CLASS(UavaUIPlayersSummary)
IMPLEMENT_CLASS(UavaUIBackGroundImage)
IMPLEMENT_CLASS(UavaUISpectatorInfo)
IMPLEMENT_CLASS(UavaUILocationInfo)
IMPLEMENT_CLASS(UavaUIQuickChatMenu)
IMPLEMENT_CLASS(UavaUIVoteMenu)
IMPLEMENT_CLASS(UavaUIDeathEffect)
IMPLEMENT_CLASS(UavaUIGameString)
IMPLEMENT_CLASS(UavaUIPDAPanel)
IMPLEMENT_CLASS(UavaUIKillMessage)
IMPLEMENT_CLASS(UavaUIGameIcon)
IMPLEMENT_CLASS(UavaUIScreenIndicator)
IMPLEMENT_CLASS(UavaUIClassIcon)
IMPLEMENT_CLASS(UavaUICrossHair)
IMPLEMENT_CLASS(UavaUITouchWeapon)
IMPLEMENT_CLASS(UavaUICurrentWeapon)
IMPLEMENT_CLASS(UavaUINVGGauge)
IMPLEMENT_CLASS(UavaUIAmmoGauge)
IMPLEMENT_CLASS(UavaUIHostLoadingBar)

IMPLEMENT_CLASS(UavaUISimpleProgress)
IMPLEMENT_CLASS(UavaUIGameGauge)
IMPLEMENT_CLASS(UavaUIGamePanel)
IMPLEMENT_CLASS(UavaUIFullScreenMap)
IMPLEMENT_CLASS(UavaUIRTNotice)
IMPLEMENT_CLASS(UavaUIProjectileWeapon)

IMPLEMENT_CLASS(UavaUIOccupationBar)
IMPLEMENT_CLASS(UavaUIVoteMessage)

IMPLEMENT_CLASS(UavaUILevelIcon)
IMPLEMENT_CLASS(UavaUIWeaponGauge)

IMPLEMENT_CLASS(UavaUIMemberInfo)
IMPLEMENT_CLASS(UavaUICustomProgressBar)
IMPLEMENT_CLASS(UavaUITopPlayerInfo)
IMPLEMENT_CLASS(UavaUIGrenadeIndicator)

IMPLEMENT_CLASS(UavaUIWeaponSpread)


#define TEAM_EU 0
#define TEAM_NRF 1

static ULocalPlayer* GetLocalPlayer()
{
	ULocalPlayer* LocalPlayer = NULL;
	if( GEngine && GEngine->GamePlayers.Num() > 0 )
		LocalPlayer = GEngine->GamePlayers(0);
	return LocalPlayer;
}

static UBOOL IsTeamGame()
{
	AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
	if ( GRI != NULL && GRI->GameClass != NULL )
	{
		return GRI->GameClass->GetDefaultObject<AGameInfo>()->bTeamGame;
	}
	return false;
}

static UBOOL IsInSpawnProtection( APawn* Pawn )
{
	if ( Pawn == NULL || !Pawn->IsValid() )
		return FALSE;

	AavaGameReplicationInfo*	GRI	= GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
	AavaGame*					GameInfo = NULL;
	if ( GRI != NULL && GRI->GameClass != NULL )
		GameInfo = GRI->GameClass->GetDefaultObject<AavaGame>();

	if ( GameInfo == NULL )
		return FALSE;

	return ( GWorld->GetTimeSeconds() - Pawn->SpawnTime < GameInfo->SpawnProtectionTime );
}

AavaPlayerController* GetavaPlayerOwner(INT Index=-1)
{
	// Attempt to find the first available
	static AavaPlayerController* Result;
	static FLOAT LastTime = -1;

	FLOAT CurrentTime = GWorld->GetRealTimeSeconds();
	if (LastTime == CurrentTime && !GIsEditor)
	{
		return Result;
	}

	LastTime = CurrentTime;

	if (Index < 0)
	{
		for (INT i=0;i<GEngine->GamePlayers.Num(); i++)
		{
			if ( GEngine->GamePlayers(i) && GEngine->GamePlayers(i)->IsValid() )
			{
				Index = i;
				break;
			}
		}
	}

	// Cast and return

	ULocalPlayer* CurrentPlayer = GEngine->GamePlayers.Num() > 0 ? GEngine->GamePlayers(Index) : NULL;
	if ( CurrentPlayer )
	{
		AavaPlayerController* avaPC = Cast<AavaPlayerController>( CurrentPlayer->Actor );

		Result = avaPC;

		return avaPC;
	}
	return NULL;
}

static UBOOL AmISpectator()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	return ( PlayerOwner != NULL && PlayerOwner->PlayerReplicationInfo != NULL ) ? PlayerOwner->PlayerReplicationInfo->bOnlySpectator : FALSE;
}

static UBOOL HasBroadCastAuthority()
{
	UavaNetHandler* NetHandler = UavaNetHandler::StaticClass()->GetDefaultObject<UavaNetHandler>()->GetAvaNetHandler();
	return ( ( NetHandler != NULL && NetHandler->GetCurrentChannelMaskLevel() >= 2 ) );
}

static APlayerReplicationInfo* GetPRI( APawn* Pawn )
{
	if ( Pawn->PlayerReplicationInfo != NULL )	return Pawn->PlayerReplicationInfo;
	else if ( Pawn->DrivenVehicle != NULL && Pawn->DrivenVehicle->PlayerReplicationInfo != NULL )	return Pawn->DrivenVehicle->PlayerReplicationInfo;
	return NULL;
}

static INT GetTeamNum( APawn* Pawn )
{
	APlayerReplicationInfo* PRI = GetPRI( Pawn );
	if ( !PRI || !PRI->Team )	return 255;
	return PRI->Team->TeamIndex;
}

static INT GetTeamNum( APlayerController* Controller )
{
	APlayerReplicationInfo* PRI = Controller ? Controller->PlayerReplicationInfo : NULL;
	if ( !PRI || !PRI->Team )	return 255;
	return PRI->Team->TeamIndex;
}

static UBOOL IsUsingBinocular( AavaPawn* Pawn )
{
	if ( Pawn == NULL || !Pawn->IsValid() )	
		return FALSE;
	if ( Pawn->bUseBinocular )
		return TRUE;
	AavaWeap_Binocular* Binocular = Cast<AavaWeap_Binocular>( Pawn->CurrentWeapon );
	return ( Binocular != NULL && Binocular->SightMode == 1);
}

AavaPlayerReplicationInfo* GetPlayerViewTargetPRI()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	if ( PlayerOwner != NULL )
	{
		APawn* ViewtargetPawn = Cast< APawn >( PlayerOwner->ViewTarget );
		if ( ViewtargetPawn != NULL )
		{
			if ( ViewtargetPawn->PlayerReplicationInfo != NULL )
				return Cast<AavaPlayerReplicationInfo>(ViewtargetPawn->PlayerReplicationInfo);
			else if ( ViewtargetPawn->DrivenVehicle != NULL )
				return Cast<AavaPlayerReplicationInfo>(ViewtargetPawn->DrivenVehicle->PlayerReplicationInfo);
		}
	}
	return NULL;
}


AavaPawn* GetPlayerViewTarget()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = NULL;
	if ( PlayerOwner != NULL )
	{
		PawnOwner = Cast<AavaPawn>( PlayerOwner->ViewTarget );
		if ( PawnOwner == NULL )
		{
			AavaWeaponPawn* WeaponPawn = Cast<AavaWeaponPawn>( PlayerOwner->ViewTarget );
			if ( WeaponPawn != NULL )
			{
				PawnOwner = Cast<AavaPawn>( WeaponPawn->Driver );
			}
		}
	}
	return PawnOwner;
}

#define SEETHROUGH_MASK(x) (3 << (x+1))
#define SEETHROUGH_TARGETTED	2
#define SEETHROUGH_DETECTED		4

void Hack_SetSeeThroughSettings( FSceneView* View )
{
	AavaPawn* AP = GetPlayerViewTarget();
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	DWORD NewMask = 0;	
	
	if (HasBroadCastAuthority())
	{
		// 망원경 사용중?
		if (AP && IsUsingBinocular( AP ) )
		{
			NewMask |= 
				SEETHROUGH_MASK( SEETHROUGH_DETECTED | SEETHROUGH_TARGETTED ) | 
				SEETHROUGH_MASK( SEETHROUGH_DETECTED );
		}

		if (AmISpectator())
		{
			NewMask |= 
				SEETHROUGH_MASK( SEETHROUGH_DETECTED | SEETHROUGH_TARGETTED ) |
				SEETHROUGH_MASK( SEETHROUGH_TARGETTED );
		}	

		if (HUD && HUD->bSeeThrough)
		{
			NewMask |= 
				SEETHROUGH_MASK( SEETHROUGH_DETECTED | SEETHROUGH_TARGETTED ) |
				SEETHROUGH_MASK( SEETHROUGH_DETECTED ) |
				SEETHROUGH_MASK( SEETHROUGH_TARGETTED ) |
				SEETHROUGH_MASK( 0 );		
		}
	}	
	
	static FLOAT LastRenderTime = -1;
	static DWORD LastMask = 0;
	static FLOAT Alpha = 0;

	FLOAT ElapsedTime = LastRenderTime < 0 ? 0 : View->Family->CurrentRealTime - LastRenderTime;		
	LastRenderTime = View->Family->CurrentRealTime;		

	if (NewMask == 0)
	{
		Alpha = 0;
	}
	else
	{
		Alpha = Clamp( Alpha + ElapsedTime * 3.0f, 0.0f, 1.0f );
	}

	View->SeeThroughGroupMask = NewMask;	

	if (View->SeeThroughGroupMask != 0)
	{
		static const FLinearColor LineColor( 0.5f, 0.5f, 0, 1 );
		static const FLinearColor EnemyColor( 0.1f, 0, 0, 1 );
		static const FLinearColor FriendlyColor( 0, 0, 0.1f, 1 );	
		static const FLinearColor Zero( 0, 0, 0, 0 );	

		View->SeeThroughSilhouetteColor = LineColor;

		if (HUD && HUD->bSeeThrough)
		{
			View->SeeThroughGroupColors.AddItem( FriendlyColor );
			View->SeeThroughGroupColors.AddItem( EnemyColor );
		}
		else if (AP && IsUsingBinocular( AP ) )
		{
			if (GetTeamNum(AP) == 0)
			{
				View->SeeThroughGroupColors.AddItem( Zero );
				View->SeeThroughGroupColors.AddItem( EnemyColor );
			}
			else if (GetTeamNum(AP) == 1)
			{		
				View->SeeThroughGroupColors.AddItem( EnemyColor );
				View->SeeThroughGroupColors.AddItem( Zero );
			}	
			else
			{			
				View->SeeThroughGroupColors.AddItem( FriendlyColor );
				View->SeeThroughGroupColors.AddItem( EnemyColor );
			}
		}
		else
		{
			View->SeeThroughGroupColors.AddItem( Zero );
			View->SeeThroughGroupColors.AddItem( Zero );
		}

		for (INT ColorIndex=0; ColorIndex<View->SeeThroughGroupColors.Num(); ++ColorIndex)
		{
			View->SeeThroughGroupColors(ColorIndex).A = Alpha;
		}		

		View->SeeThroughSilhouetteColor.A = Alpha;
	}
}

AavaWeapon* GetViewTargetCurrentWeapon()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = NULL;
	if ( PlayerOwner != NULL )
	{
		AavaWeaponPawn* WeaponPawn = Cast<AavaWeaponPawn>( PlayerOwner->ViewTarget );
		if ( WeaponPawn == NULL )
		{
			PawnOwner = Cast<AavaPawn>( PlayerOwner->ViewTarget );
			if ( PawnOwner != NULL )
				return PawnOwner->CurrentWeapon;
			else
				return NULL;
		}
		else
		{
			return Cast<AavaWeapon>( WeaponPawn->MyVehicleWeapon );
		}
	}
	return NULL;
}

AavaPawn* GetAvaPawn( AavaPlayerReplicationInfo* avaPRI )
{
	AavaPawn* PawnOwner;
	for( FDynamicActorIterator it ; it ; ++it )
	{
		APawn* Pawn = Cast<APawn>(*it);
		if  ( Pawn == NULL )	continue;
		if ( Pawn->PlayerReplicationInfo == avaPRI )
		{
			AavaWeaponPawn* WeaponPawn = Cast<AavaWeaponPawn>( Pawn );
			if ( WeaponPawn == NULL )
			{
				PawnOwner = Cast<AavaPawn>( Pawn );
			}
			else
			{
				PawnOwner = Cast<AavaPawn>( WeaponPawn->Driver );
			}
			return PawnOwner;
		}
	}
	return NULL;
}

VOID AavaHUD::TickSpecial( FLOAT DeltaTime )
{
	Super::TickSpecial( DeltaTime );

	DrawPawnList.Empty();

	FVector	ViewtargetLocation;

	AavaPawn* avaPawn = GetPlayerViewTarget();
	if ( avaPawn == NULL )
	{
		AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
		if ( PlayerOwner != NULL )
		{
			ViewtargetLocation = PlayerOwner->Location;
		}
		else
		{
			return;
		}
	}
	else
	{
		ViewtargetLocation = avaPawn->Location;
	}
		
	UBOOL	bInsert;
	FDrawPawnData	data;
	for ( INT i = 0 ; i < PostRenderedActors.Num() ; ++ i )
	{
		APawn* P = Cast<APawn>(PostRenderedActors(i));
		if ( P == NULL || P->Health < 0 || P->bDeleteMe )
			continue;
		bInsert			= FALSE;
		data.Pawn		= P;
		data.Distance	= ( P->Location - ViewtargetLocation ).Size();

		for ( int j = 0 ; j < DrawPawnList.Num() ; ++ j )
		{
			if ( DrawPawnList(j).Distance < data.Distance )
			{
				bInsert = TRUE;
				DrawPawnList.InsertItem( data, j );
				break;
			}
		}
		if ( bInsert == FALSE )
		{
			DrawPawnList.AddItem( data );
		}
	}

	// 3 초가 넘으면 DamageIndicator 를 지운다...
	for ( INT i = 0 ; i < DamageIndicatorData.Num() ; ++ i )
	{
		if ( GWorld->GetTimeSeconds() - DamageIndicatorData(i).DamageTime > 3.0f )
			DamageIndicatorData.Remove( i--, 1 );
	}

	// Remove exhaust.
	for(INT i = KillMessageData.Num() - 1; i >= 0 ; i--)
	{
		if( KillMessageData(i).TimeOfDeath + 4.0f <  GWorld->GetTimeSeconds())
		{
			KillMessageData.Remove(i,1);
		}
	}

	for(INT i = MyKillMessageData.Num() - 1; i >= 0 ; i--)
	{
		if( MyKillMessageData(i).TimeOfDeath + 4.0f <  GWorld->GetTimeSeconds())
		{
			MyKillMessageData.Remove(i,1);
		}
	}
}



static void DrawTile( FCanvas* Canvas, USurface* Surface, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& DrawColor, UBOOL bAdjustSurfaceSize = TRUE )
{

	UTexture* Texture = Cast<UTexture>(Surface);

	if ( Texture != NULL || bAdjustSurfaceSize == TRUE )
	{
		FLOAT mW = Surface->GetSurfaceWidth();
		FLOAT mH = Surface->GetSurfaceHeight();
		U /= mW; UL /= mW;
		V /= mH; VL /= mH;
	}

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

/**	2006/12/11 YTS, 돌려찍기위한 DrawTile
*	
*	@param	Canvas - 그릴 대상
*	@param	Surface - 찍으려고 하는 Texture2D
*	@param	AxisX - Tex를 돌릴 회전축의 X좌표
*	@param	AxisY - Tex를 돌릴 회전축의 Y좌표
*	@param	DegAngle - 회전 각도 ( Degree )
*	@param	X - 회전축을 0,0으로 한 상대적인 찍을 위치 X좌표
*	@param	Y - 회전축을 0,0으로 한 상대적인 찍을 위치 Y좌표
*	@param	XL - 화면에 찍을 너비 (Width)
*	@param	YL - 화면에 찍을 높이 (Height)
*	@param	U - TexCoord.U
*	@param	V - TexCoord.V
*	@param	UL - TexCoord.UL
*	@param	VL - TexCoord.VL
*/
static void DrawTileRotate(FCanvas* Canvas, USurface* Surface, FLOAT AxisX, FLOAT AxisY, FLOAT DegAngle, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& DrawColor)
{
	if( Canvas == NULL || Surface == NULL )
		return;

	const FLOAT Angle = DegAngle * 65535.0f / 360.f;
	FMatrix TransMat = FRotationMatrix( FRotator(0,appTrunc(Angle),0) )
		* FTranslationMatrix( FVector(AxisX, AxisY,0) );
	Canvas->PushRelativeTransform(TransMat);
	DrawTile(Canvas, Surface, X, Y, XL, YL, U, V, UL, VL, DrawColor);
	Canvas->PopTransform();
}

static void DrawRect( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, UTexture2D* Tex, const FLinearColor& DrawColor )
{
	checkSlow( Tex != NULL );

	DrawTile( Canvas, Tex, X, Y, XL, YL, 0, 0, Tex->SizeX, Tex->SizeY, DrawColor );
}

static void DrawThickBox( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT Thickness, UTexture2D* Tex, const FLinearColor& DrawColor )
{
	/// vertical 
	DrawRect( Canvas, X, Y + Thickness, Thickness, YL - Thickness * 2, Tex, DrawColor );
	DrawRect( Canvas, X + XL - Thickness, Y + Thickness, Thickness, YL - Thickness * 2, Tex, DrawColor );

	/// horizontal
	DrawRect( Canvas, X, Y, XL, Thickness, Tex, DrawColor );		
	DrawRect( Canvas, X, Y + YL - Thickness, XL, Thickness, Tex, DrawColor );		
}

static UBOOL DrawString( UUIObject* Object, FCanvas* Canvas, FLOAT &Y, UFont* Font, const TCHAR* String, const FLinearColor& DrawColor )
{
	// Precalc the Width since it's used for each char.
	FLOAT Width = Object->RenderBounds[UIFACE_Right] - Object->RenderBounds[UIFACE_Left];
	UBOOL bOutOfBounds=false;

	FLOAT SceneHeight = Object->GetScene()->Position.GetBoundsExtent( Object->GetScene(), UIORIENT_Vertical, EVALPOS_PixelViewport);
	FLOAT FontScaling = Font->GetScalingFactor( SceneHeight );
	INT FontPageIndex = Font->GetResolutionPageIndex( SceneHeight );


	// Deconstruct the string

	// Get a reference to the head of this segment
	const TCHAR* Head = String;

	// We want to track the position of the last space.  When it's time to wrap, we will
	// try to wrap at that position instead

	INT LastSpaceIndex = -1;

	// Reset the length

	INT Len = 0;

	// We will use WidthTally to track how wide this segment is.

	FLOAT WidthTally=0.0f;

	FLOAT XSize=0.0f;
	FLOAT YSize=0.0f;

	while ( *String )
	{

		// Grab the next character.  If it's a space, track it

		TCHAR Ch = *String;
		if ( Ch == TEXT(' ') )
		{
			LastSpaceIndex = Len;
		}
		// Size up the character

		Font->GetCharSize( Ch, XSize, YSize,FontPageIndex);

		// Look to see if this new character would force us to wrap
		if (WidthTally + XSize > Width)
		{
			// If we are not sitting on a space, then move back to the last space

			if ( Ch != TEXT(' ') && LastSpaceIndex >=0 )
			{
				Len = LastSpaceIndex;
				String = Head + Len;
			}

			// Do the next segment
			bOutOfBounds = DrawString( Object, Canvas, Y, Font, ++String, DrawColor);
			break;
		}
		else	// We wouldn't wrap, so increment everything
		{
			WidthTally+=XSize;
			Len++;
			String++;
		}
	}

	// We have deconstructed all of the pieces, start rendering.  But
	// be mindful of the bounds.

	Y -= YSize;
	if ( !bOutOfBounds && Y > Object->RenderBounds[UIFACE_Top] )
	{
		INT X = appTrunc(Object->RenderBounds[UIFACE_Left]);
		for( INT i=0; i < Len && Head[i]; i++ )
		{
			INT Ch = (TCHARU)Font->RemapChar(Head[i]);

			// Process character if it's valid.
			if( Ch < Font->Characters.Num() )
			{
				FFontCharacter& Char = Font->Characters(Ch + FontPageIndex);
				UTexture2D* Tex;
				if( (Tex = Font->GetTexture(Char.TextureIndex))!=NULL )
				{
					INT CU     = Char.StartU;
					INT CV     = Char.StartV;
					INT CUL = Char.USize;
					INT CVL = Char.VSize;

					// Draw.
					DrawTile(Canvas,X,Y,CUL,CVL,(FLOAT)CU / (FLOAT)Tex->SizeX, (FLOAT)CV / (FLOAT)Tex->SizeY, (FLOAT)CUL / (FLOAT)Tex->SizeX,(FLOAT)CVL / (FLOAT)Tex->SizeY, DrawColor,Tex->Resource);
					X+=CUL;
				}
			}
		}
	}

	return bOutOfBounds;
}

//  [10/24/2006 YTS] Widget의 RenderBound와 상관없는 위치에 드로잉.
static void DrawString( FCanvas* Canvas, FLOAT X ,FLOAT Y, UFont* Font, const TCHAR* String, const FLinearColor& DrawColor )
{
	INT XL, YL;
	FRenderParameters TextRenderParams;
	FUIStringNode_Text NodeText(TEXT("TextNode"));
	NodeText.SetRenderText(String);
	FUICombinedStyleData StyleData;
	// 주의. UIStringNode_Text::RenderNode()부분에 특화하여 작성하였음. 
	StyleData.TextColor = DrawColor;
	StyleData.DrawFont = Font;
	StyleData.TextAlignment[UIORIENT_Vertical] = UIFACE_Left;
	NodeText.InitializeStyle(StyleData);

	StringSize(Font, XL, YL, String);

	TextRenderParams.DrawX = X;
	TextRenderParams.DrawY = Y;
	TextRenderParams.DrawXL = XL;
	TextRenderParams.DrawYL = YL;
	TextRenderParams.TextAlignment[UIORIENT_Vertical] = UIALIGN_Left;

	NodeText.Render_Node(Canvas, TextRenderParams);
}

// @deif
// -----
// Player가 하나 이상은 있다는 전제 하에! -_-!
// 그리고 하나만 있다는 전제 하에!! 
// 참조 : UnPlayer.cpp 
//		  [ 다음으로 검색하시오 : // @HACK deif added HUD/UI ]
// 
extern FSceneView* GPlayerSceneView;

//  [10/20/2006 YTS] , Get Picking Info
//	@param RayOrg - the origin of ray.
//	@param RayDir - the direction of ray.
// ex) TraceActors From 'RayOrg' To 'RayOrg + RayDir * Distance'
static void CalcPickRay(INT ScreenX, INT ScreenY, FVector& RayOrg, FVector& RayDir)
{	
	FPlane A = GPlayerSceneView->PixelToScreen( ScreenX, ScreenY, 0.0f );
	FPlane B = GPlayerSceneView->PixelToScreen( ScreenX, ScreenY, 1.0f );

	FVector X = GPlayerSceneView->Deproject( A );
	FVector Y = GPlayerSceneView->Deproject( B );
	
	RayOrg = X;
	RayDir = (Y - X).SafeNormal();
}

static void DrawIconInfo( FCanvas* Canvas, const float X, const float Y, const float CX, const float CY, const FIconCodeInfo* IconInfo, float AddAlpha )
{
	if( IconInfo->Image == NULL )		return;
	FLinearColor LC( IconInfo->Color );
	LC.A *= AddAlpha;
	FTextureCoordinates	ImageCoordinates = IconInfo->Coord;
	DrawTile( Canvas, IconInfo->Image, (INT)X, (INT)Y, CX, CY,
			ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL, LC );
}

static void DrawSlotIcon( FCanvas* Canvas, INT nTeamNum, INT SlotNum, INT ScreenX, INT ScreenY, FLOAT CX, FLOAT CY, FLOAT Alpha, INT Align_Horz, INT Align_Vert )
{
	int	SlotTeam, SlotCnt;
	if ( SlotNum < 0 )	return;
	SlotCnt		= SlotNum % 12;
	// 호림이가 반대로 Image 를 만들었다... Super 땜빵...
	if ( nTeamNum == 0 )		SlotTeam = 1;
	else if ( nTeamNum == 1 )	SlotTeam = 0;
	else						return;

	static const FLOAT	UL = 36.0f;
	static const FLOAT	VL = 36.0f;
	FLinearColor	Color(1,1,1,Alpha);
	FLOAT U  =	SlotCnt * UL;
	FLOAT V  =	SlotTeam * VL;
	if ( Align_Horz == 1 )			ScreenX -= CX/2.0;
	else if ( Align_Horz == 2 )		ScreenX -= CX;
	if ( Align_Vert == 1 )			ScreenY -= CY/2.0;
	else if ( Align_Vert == 2 )		ScreenY -= CY;
	DrawTile( Canvas, AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->SlotNumTexture, ScreenX, ScreenY, CX, CY, U, V, UL, VL, Color );

}

static void DrawLeaderIcon( FCanvas* Canvas, INT ScreenX, INT ScreenY, FLOAT CX, FLOAT CY, FLOAT Alpha, INT Align_Horz, INT Align_Vert )
{
	static const FLOAT U	=	AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->LeaderU;
	static const FLOAT V	=	AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->LeaderV;
	static const FLOAT UL	=	AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->LeaderUL;
	static const FLOAT VL	=	AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->LeaderVL;
	FLinearColor	Color(1,1,1,Alpha);
	if ( Align_Horz == 1 )			ScreenX -= CX/2.0;
	else if ( Align_Horz == 2 )		ScreenX -= CX;
	if ( Align_Vert == 1 )			ScreenY -= CY/2.0;
	else if ( Align_Vert == 2 )		ScreenY -= CY;
	DrawTile( Canvas, AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->LeaderTexture, ScreenX, ScreenY, CX, CY, U, V, UL, VL, Color );
}

static void DrawLevelIcon( FCanvas* Canvas, AavaHUD* HUD, INT ScreenX, INT ScreenY, FLOAT CX, FLOAT CY, FLOAT Alpha, INT Level, INT Align_Horz, INT Align_Vert )
{
	//	Align_Horz		0 : Left	1 : Center	2 : Right
	//	Align_Vert		0 : Top		1 : Center	2 : Bottom
	static const INT	SizeX = 5;
	static const INT	SizeY = 6;
	FLinearColor	Color(1,1,1,Alpha);
	INT OffsetX = (Level /(SizeX * SizeY)) * SizeX + ( Level % SizeX );
	INT OffsetY = (Level / SizeX) % SizeY;
	static const FLOAT	UL = 19.0f;
	static const FLOAT	VL = 19.0f;
	FLOAT U  =	OffsetX * UL;
	FLOAT V  =	OffsetY * VL;
	if ( Align_Horz == 1 )			ScreenX -= CX/2.0;
	else if ( Align_Horz == 2 )		ScreenX -= CX;
	if ( Align_Vert == 1 )			ScreenY -= CY/2.0;
	else if ( Align_Vert == 2 )		ScreenY -= CY;
	DrawTile( Canvas, HUD == NULL ? AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->LvlTexture : HUD->LvlTexture, ScreenX, ScreenY, CX, CY, U, V, UL, VL, Color );
}

//	Align_Horz		0 : Left	1 : Center	2 : Right
//	Align_Vert		0 : Top		1 : Center	2 : Bottom
static void DrawClanIcon( FCanvas* Canvas, INT ScreenX, INT ScreenY, FLOAT CX, FLOAT CY, FLOAT Alpha, INT ClanMarkID, INT Align_Horz, INT Align_Vert )
{
	if ( ClanMarkID < 0 )	return;
	static const	INT		CntX = 26;
	static const	INT		CntY = 26;
	const			INT		ClanMarkTexturePage	=	ClanMarkID / ( CntX * CntY );
	if ( !AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ClanMarkTextures.IsValidIndex( ClanMarkTexturePage ) )	return;
	const			INT		OffsetX	=	ClanMarkID % CntX;
	const			INT		OffsetY	=	ClanMarkID / CntX;
	static const	FLOAT	UL	=	19.0f;
	static const	FLOAT	VL	=	19.0f;
	const			FLOAT	U	=	OffsetX * UL;
	const			FLOAT	V	=	OffsetY * VL;
	const			FLinearColor	Color(1,1,1,Alpha);

	if ( Align_Horz == 1 )			ScreenX	-= CX/2.0;
	else if ( Align_Horz == 2 )		ScreenX -= CX;
	if ( Align_Vert == 1 )			ScreenY -= CY/2.0;
	else if ( Align_Vert == 2 )		ScreenY -= CY;
	DrawTile( Canvas, AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ClanMarkTextures(ClanMarkTexturePage), ScreenX, ScreenY, CX, CY, U, V, UL, VL, Color );
}

// Radar 상에서의 위치를 구해준다.
// @param resultPos - 레이더에 찍힐 실제 위치
// @param scaledPos - 레이더에 찍힐 위치를 중심을 기준으로 Scaling한것 (물체가 레이더 외부에 위치할때 테두리에 바로 찍지 않고 조금 안쪽에 찍히도록 하기 위함)
//					bOutSide가 참일때만 Scaling한다.
// @param bOutSide - 물체가 레이더 외부에 있음을 의미
UBOOL UavaUIRadar::CalcPosInRadar( const FVector& pos, const FRotator& ViewRotator, FVector2D& resultPos, FVector2D& scaledPos,UBOOL& bOutside, FLOAT& DegAngle, UBOOL bClamp, UBOOL bAlwaysClamp )
{	
	FVector tempPos, OrgPos;	

	tempPos = Scale * FRotationMatrix(ViewRotator).TransformNormal(pos);
	OrgPos = tempPos;

	FLOAT XL = MapSize[0];
	FLOAT YL = MapSize[1];
	FLOAT HalfXL = XL / 2.0f;
	FLOAT HalfYL = YL / 2.0f;
	FLOAT ScaleX = 1.0f;
	FLOAT ScaleY = 1.0f;
	DegAngle = (appAtan2(tempPos.Y, tempPos.X) + PI) * 180.f / PI;

	if( bShowOutIcon )
	{
		ScaleX = 0.9f;
		ScaleY = 0.9f;
	}

	if( RadarShape == UIRADARSHAPE_CIRCLE )
	{
		FLOAT Rsq = Square( XL / 2.0f );
		bOutside = tempPos.SizeSquared() > Rsq;

		if( bAlwaysClamp || (bOutside && bClamp) )
		{
			tempPos.Normalize();
			OrgPos.Normalize();

			tempPos *= (HalfXL * ScaleX);
			OrgPos *= HalfXL;
		}
		else if (bOutside)
			return FALSE;
	}
	else if (RadarShape == UIRADARSHAPE_RECTANGLE )
	{

		bOutside = ! (Abs(tempPos.X) <= (HalfXL * ScaleX) && 
			Abs(tempPos.Y) <= (HalfYL * ScaleY));

		if (bAlwaysClamp || bOutside && bClamp)
		{
			tempPos.Normalize();
			OrgPos.Normalize();

			if( Abs(tempPos.X / tempPos.Y) > (HalfXL/HalfYL) )
			{
				tempPos *= (HalfXL * ScaleX / Abs(tempPos.X));
				OrgPos *= (HalfXL / Abs(OrgPos.X));
			}
			else
			{
				tempPos *= (HalfYL * ScaleX / Abs(tempPos.Y));
				OrgPos *= (HalfYL / Abs(OrgPos.Y));
			}
		}
		else if (bOutside)
		{
			return FALSE;
		}
	}

	scaledPos.X = appTrunc( tempPos.Y + RenderBounds[UIFACE_Left] + MapSize[0] / 2 + MapOffset[0] );
	scaledPos.Y = appTrunc( -tempPos.X + RenderBounds[UIFACE_Top] + MapSize[0] / 2 + MapOffset[1] );
	resultPos.X = appTrunc( OrgPos.Y + RenderBounds[UIFACE_Left] + MapSize[0] / 2 + MapOffset[0] );
	resultPos.Y = appTrunc( -OrgPos.X + RenderBounds[UIFACE_Top] + MapSize[0] / 2 + MapOffset[1]);

	return TRUE;
}

static FIconCodeInfo* GetIconInfo( INT nCode )
{
	if ( nCode < 0 || nCode >= AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->Icons.Num() )	return NULL;
	return &AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->Icons( nCode );
}

const FIconCodeInfo* UavaUIRadar::GetRadarIconInfo( const AavaHUD* HUD, INT nCode )
{
	if ( HUD == NULL )	return NULL;
	if ( nCode >= HUD->Icons.Num() || nCode < 0 )	return NULL;
	return &HUD->Icons( nCode );
}

void UavaUIRadar::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );

	FString PropThatChangedStr = PropertyThatChanged->GetName();
	if (appStricmp(*PropThatChangedStr, TEXT("MapMaterial")) == 0)
	{
		Map = NULL;	
	}
}

static UBOOL UnscaledWorldToScreen( const FVector& Location, FVector2D& ScreenLoc, UBOOL bShouldKeptInScreen )
{
	if ( !GPlayerSceneView->WorldToPixel( Location, ScreenLoc ) )
		return FALSE;

	if (bShouldKeptInScreen)
	{
		if (!( ScreenLoc.X > GPlayerSceneView->X && ScreenLoc.X < GPlayerSceneView->X + GPlayerSceneView->SizeX && ScreenLoc.Y > GPlayerSceneView->Y && ScreenLoc.Y < GPlayerSceneView->Y + GPlayerSceneView->SizeY ))
			return FALSE;
	}

	if (GSystemSettings->NeedsUpscale())
	{
		FLOAT ScaleFactor = Clamp( GSystemSettings->ScreenPercentage / 100.f, 0.0f, 1.f );

		FLOAT CenterX = GPlayerSceneView->X + GPlayerSceneView->SizeX * 0.5f;
		FLOAT CenterY = GPlayerSceneView->Y + GPlayerSceneView->SizeY * 0.5f;

		// (x - CenterX) / ScaleFactor + CenterX		

		ScreenLoc.X = (ScreenLoc.X - CenterX) / ScaleFactor + CenterX;
		ScreenLoc.Y = (ScreenLoc.Y - CenterY) / ScaleFactor + CenterY;
	}	

	return TRUE;
}

void UavaUIRadar::DrawVehicleInHUD( FCanvas* Canvas, AavaVehicle* Vehicle, AavaPawn* PawnOwner, const AavaPlayerController* PlayerOwner, const FVector& view_Location, const FRotator& view_Rotator,const BYTE OwnerTeam )
{
	const FLOAT					FadeoutTime		= 0.1f;
	const FLOAT					FadeoutDuration	= 0.5f;
	AWorldInfo*					WorldInfo		= GWorld->GetWorldInfo();
	const FLOAT					Visibility		= 1.0f - Clamp( (WorldInfo->TimeSeconds - Vehicle->LastRenderTime - FadeoutTime) / (FadeoutDuration), 0.0f, 1.0f );
	FString						Message;
	FVector2D					ScreenLoc;
	FVector2D					ScreenDownLoc;
	FLinearColor				DrawColor;
	FVector						DrawLoc;

	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	
	if ( Vehicle == NULL )						return;
	if ( HUD == NULL )							return;

	if ( ( PlayerOwner->Rotation.Vector() | ( Vehicle->Location - view_Location ) ) <= 0 )	return;	// 화면 뒤에 있음...
	if ( Visibility <= 0 )						return;

	if ( Vehicle->DrawHUDBoneName != NAME_None )	DrawLoc	=	Vehicle->Mesh->GetBoneLocation( Vehicle->DrawHUDBoneName );
	else											DrawLoc =	Vehicle->Location + Vehicle->DrawHUDOffset;

	if ( !UnscaledWorldToScreen( DrawLoc , ScreenLoc, TRUE ) )
		return;

	if ( OwnerTeam )	DrawColor = HUD->FriendlyColor;
	else				DrawColor = HUD->EnemyColor;

	FLOAT		TargetScale		= 1.0;
	INT			XL, YL = 10;

	ScreenLoc.Y -= YL;
	if ( Visibility > 0 )
	{
		// 이름과 Health 를 찍어준다...
		int DrawX, DrawY;
		Message = FString::Printf( TEXT( "%s" ), *Vehicle->VehicleNameString );
		StringSize( Font, XL, YL, *Message );
		DrawColor.A *= Visibility;
		DrawX = ScreenLoc.X - XL/2;
		DrawY = ScreenLoc.Y;
		DrawStringCentered( Canvas, ScreenLoc.X, ScreenLoc.Y, *Message, Font, DrawColor );
		FLinearColor LC(0,0,0,Visibility);
		DrawRect( Canvas, DrawX , DrawY + int(YL) + 2, Vehicle->HealthBarLength, 3, DefaultTexture, LC );	
		LC = FLinearColor(0,1,0,Visibility);
		DrawRect( Canvas, DrawX , DrawY + int(YL) + 2, Vehicle->HealthBarLength * ( Vehicle->Health / (FLOAT)Vehicle->GetClass()->GetDefaultObject<AavaVehicle>()->Health ), 3, DefaultTexture, LC );			
	}
}

void UavaUIRadar::DrawPawnInHUD( FCanvas* Canvas, AavaPawn* Pawn, AavaPawn* PawnOwner, AavaPlayerController* PlayerOwner, const FVector& view_Location, const FRotator& view_Rotator, const BYTE OwnerTeam )
{
	const FLOAT					FadeoutTime		= 0.1f;
	const FLOAT					FadeoutDuration	= 0.5f;
	AWorldInfo*					WorldInfo		= GWorld->GetWorldInfo();
	
	FLOAT						Visibility		= 1.0f - Clamp( (WorldInfo->TimeSeconds - Pawn->LastRenderTime - FadeoutTime) / (FadeoutDuration), 0.0f, 1.0f );
	const BOOL					bUseBinocular	= IsUsingBinocular( PawnOwner );
	FString						Message;
	AavaPlayerReplicationInfo*	PRI = NULL;
	FVector2D					ScreenLoc;
	FVector2D					ScreenDownLoc;
	BOOL						bSameTeam;
	BOOL						bLeader;
	FLinearColor				DrawColor;
	FVector						HeadBoneLoc;

	FVector						ViewLocation = view_Location;

	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if ( Pawn == NULL )			return;
	Pawn->bIsInScreen	= FALSE;

	struct FScopedSetSeeThrough
	{
		AavaPawn* Pawn;
		UBOOL bDetected_Initial;

		FScopedSetSeeThrough( AavaPawn* InPawn )
			: Pawn( InPawn )
		{			
			bDetected_Initial = Pawn->bDetected;
		}

		~FScopedSetSeeThrough()
		{
			if (Pawn->bDetected ^ bDetected_Initial)
			{
				Pawn->eventUpdateSeeThrough();
			}
		}
	} Scoped_UpdateSetSeeThrough( Pawn );

	if ( PawnOwner != NULL )
		ViewLocation += FVector( 0, 0, PawnOwner->bIsCrouched ? PawnOwner->BaseEyeHeightWhenCrouched : PawnOwner->BaseEyeHeight );
	PRI = Cast<AavaPlayerReplicationInfo>( GetPRI( Pawn ) );
	if ( PRI == NULL || ( PRI->bBot == TRUE && PlayerOwner->bDisplayBotInfo == FALSE ) || HUD == NULL || ( PlayerOwner->Rotation.Vector() | ( Pawn->Location - view_Location ) ) <= 0  )
	{
		Pawn->bDetected		= FALSE;
		return;
	}
	
	// 화면상의 좌표를 얻어온다...
	static FName HeadBoneName( TEXT("Bip01_Head" ) );
	HeadBoneLoc = Pawn->Mesh->GetBoneLocation( HeadBoneName ) ;
	bSameTeam = PlayerOwner->eventIsSameTeamByIndex( GetTeamNum(Pawn) );

	if ( bSameTeam )	
	{
		if ( !Pawn->eventIsBIA() )
		{
			DrawColor =	HUD->FriendlyColor;
		}
		else
		{
			DrawColor = HUD->BIAColor;
			Visibility = Clamp( Visibility, 0.35f, 1.0f );	
		}
	}
	else
	{
		DrawColor = HUD->EnemyColor;
	}

	// SlotNum 의 표시는 가리더라도 항상 해주도록 한다...
	if ( bDrawOnlySlotNum == true )
	{
		Visibility = Clamp( Visibility, 0.35f, 1.0f );
	}

	if ( Visibility > 0 || Pawn->bTargetted )
	{		
		if ( !UnscaledWorldToScreen( HeadBoneLoc + FVector(0,0,15) , ScreenLoc, TRUE ) )
		{
			Pawn->bDetected		= FALSE;
			return;
		}
	}

	FLOAT		TargetScale		= 1.0;
	INT			XL, YL = 10;

	//@NOTE deif - 3D에서 convert된 좌표를 base-line으로 사용한다. (upper left corner가 아닌)
	ScreenLoc.Y -= YL;
	BOOL	bDetected = bSameTeam ? TRUE : FALSE;
	bLeader			  = ( bSameTeam && PRI != NULL && PRI->bSquadLeader ) ? TRUE : FALSE;
	if ( Visibility > 0 )
	{
		// 이름과 Health 를 찍어준다...		
		if ( !bSameTeam )	// 같은 Team 이 아니면 좀 더 Tight 하게 Check 한다....
		{
			FVector			ViewPos		= HeadBoneLoc;
			FVector			ViewStart	= ViewLocation;
			FCheckResult	Hit(1.f);
			GWorld->SingleLineCheck(Hit, PawnOwner,	ViewPos, ViewStart, TRACE_ProjTargets|TRACE_Tesselation|TRACE_ComplexCollision );
			if ( Pawn == Cast<APawn>( Hit.Actor ) )
			{
				bDetected = TRUE;
			}
		}

		if ( bDetected )
		{
			Pawn->bIsInScreen = TRUE;
			if( bSameTeam || bUseBinocular )
			{
				int DrawX, DrawY;


				DrawX = ScreenLoc.X;
				DrawY = ScreenLoc.Y;

				if ( bDrawOnlySlotNum == FALSE )
				{
					Message = FString::Printf( TEXT( "%s" ), *(PRI->PlayerName) );
					StringSize( Font, XL, YL, *Message );
					DrawColor.A *= Visibility;
					DrawX -=  XL/2;
					if ( bLeader == TRUE )	
					{
						DrawLeaderIcon( Canvas, DrawX - int(YL), DrawY, YL, YL, Visibility, 2, 0 );
					}
					DrawLevelIcon( Canvas, HUD, DrawX, DrawY, YL, YL, Visibility, Max(PRI->Level - 1,0), 2, 0 );
					DrawStringCentered( Canvas, ScreenLoc.X, ScreenLoc.Y, *Message, Font, DrawColor );

					BOOL	bDisplayHP = TRUE;
					if ( WorldInfo->TimeSeconds - Pawn->LastPainTime < 1.0 )
					{
						bDisplayHP = ( int( ( WorldInfo->TimeSeconds - Pawn->LastPainTime ) * 1000 ) / 100 ) % 2;
					}
					// HP를 찍어준다.....

					if ( bDisplayHP )
					{
						FLinearColor LC(0,0,0,Visibility);
						DrawRect( Canvas, DrawX - int(YL), DrawY + int(YL) + 2, 52, 3, DefaultTexture, LC );	
						LC = FLinearColor(0,1,0,Visibility);
						DrawRect( Canvas, DrawX - int(YL), DrawY + int(YL) + 2, 52 * ( Pawn->Health / (FLOAT)Pawn->HealthMax ), 3, DefaultTexture, LC );	
					}
				}
				else
				{
					DrawSlotIcon( Canvas, PRI->Team != NULL ? PRI->Team->TeamIndex : 255, PRI->SlotNum, DrawX, DrawY, SlotNumSize.X, SlotNumSize.Y, Visibility, 1, 1 );
				}
			}			
		}
	}

	if ( bSameTeam && Visibility <= 0 )
		bDetected = FALSE;

	if ( bDetected == TRUE )
	{
		if ( Pawn->bDetected == FALSE ) 
		{
			Pawn->bDetected			= TRUE;
			Pawn->LastDetectedTime	= WorldInfo->TimeSeconds;
		}
	}
	else
	{
		Pawn->bDetected = FALSE;
	}

	FLOAT	Opacity = 1.0;
	if ( ( Pawn->bTargetted && ( !bSameTeam || ( AmISpectator() && HasBroadCastAuthority() ) ) ) || ( bUseBinocular && bDetected ) )
	{
		if ( Pawn->bTargetted )
		{
			// Target 되었고 화면상에 보인다.
			if ( Pawn->bDetected )		Opacity	= 1.0;
			// Target 되었고 화면상에 보이지 않는다...
			else						Opacity = 1.0f - Clamp( (WorldInfo->TimeSeconds - Pawn->LastDetectedTime - FadeoutTime) / (FadeoutDuration), 0.0f, 0.5f );
		}
		else
		{
			if ( Pawn->bDetected )	// Target 되지 않았지만 화면상에 보인다. (쌍안경을 이용하는 경우...)
			{
				Opacity = 1.0f * ( int( (WorldInfo->TimeSeconds - Pawn->LastDetectedTime) * 1000 / 500 ) % 2 );
			}
		}

		if ( Pawn->bTargetted )
		{
			TargetScale = Clamp( TargetMaxScale * (TargetDuration - ( WorldInfo->TimeSeconds - Pawn->TargettedTime )) / TargetDuration, 1.0f, TargetMaxScale );
		}
		DrawColor.A *= Opacity;
		// Binocular 를 사용중일때는 Class 를 상징하는 도형을 그려준다...
		if ( !UnscaledWorldToScreen( Pawn->Location - Pawn->CylinderComponent->CollisionHeight * FVector(0,0,0.5), ScreenDownLoc, TRUE ) )	return;
		
		INT IconDataIndex = ( 0 <= PRI->CurrentSpawnClassID && PRI->CurrentSpawnClassID <= 2 ) ? PRI->CurrentSpawnClassID : -1;
		INT IconCode = ClassCodes.IsValidIndex(IconDataIndex) ? ClassCodes(IconDataIndex) : -1;
		const FIconCodeInfo* IconInfo = GetRadarIconInfo(HUD,IconCode);
		if ( IconInfo != NULL )
		{
			FLOAT	IconHeight = TargetScale * Clamp( fabs( ScreenDownLoc.Y - ScreenLoc.Y + YL + 3 ), MinTargetHeight, MaxTargetHeight );
			FLOAT	IconWidth  = IconInfo->Coord.UL * ( IconHeight / IconInfo->Coord.VL );
			Render_Object( Canvas, ScreenLoc.X - IconWidth/2.0, ( ScreenDownLoc.Y + ScreenLoc.Y + YL + 3 ) / 2.0 - IconHeight/2.0, IconWidth, IconHeight, IconInfo, 1.0, -1, true, DrawColor );
		}
	}


}

void UavaUIRadar::DrawTargetInHUD( FCanvas* Canvas, const AActor* PawnOwner, const AavaPlayerController* PlayerOwner, const AActor* actor, const FLOAT TargettedTime, const int	IconCode, const FString str )
{
	FVector2D		ScreenLoc;
	FVector2D		ScreenDownLoc;
	AWorldInfo*		WorldInfo		= GWorld->GetWorldInfo();
	FLOAT			TargetScale		= 1.0;
	AavaHUD*		HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	FLinearColor	DrawColor;

	if ( PlayerOwner == NULL )	return;
	FVector			ViewLocation = PawnOwner != NULL ? PawnOwner->Location : PlayerOwner->Location;
	if ( !UnscaledWorldToScreen( actor->Location + DefaultTargetHeight * FVector(0,0,1), ScreenLoc, TRUE ) )		return;	// 화면상의 좌표를 가지고 옴	
	if ( !UnscaledWorldToScreen( actor->Location - DefaultTargetHeight * FVector(0,0,1), ScreenDownLoc, TRUE ) )	return;	// 화면상의 좌표를 가지고 옴
	
	TargetScale = Clamp( TargetMaxScale * (TargetDuration - ( WorldInfo->TimeSeconds - TargettedTime )) / TargetDuration, 1.0f, TargetMaxScale );
	DrawColor	= HUD->FriendlyColor;
	DrawColor.A *= 0.7f;

	const FIconCodeInfo* IconInfo = GetRadarIconInfo(HUD,IconCode);
	if ( IconInfo != NULL )
	{
		FLOAT	IconHeight = TargetScale * Clamp( fabs( ScreenDownLoc.Y - ScreenLoc.Y ), MinTargetHeight, MaxTargetHeight );
		FLOAT	IconWidth  = IconInfo->Coord.UL * ( IconHeight / IconInfo->Coord.VL );
		Render_Object( Canvas, ScreenLoc.X - IconWidth/2.0, ( ScreenDownLoc.Y + ScreenLoc.Y ) / 2.0 - IconHeight/2.0, IconWidth, IconHeight, IconInfo, 1.0, -1, true, DrawColor );
		FString	Message;
		Message = FString::Printf( TEXT( "%s (%d m)" ), *str, (INT)((actor->Location - ViewLocation).Size() / 53.0f) );
		DrawStringCentered( Canvas, ScreenLoc.X, ( ScreenDownLoc.Y + ScreenLoc.Y ) / 2.0 + IconHeight/2.0, *Message, Font, DrawColor );
	}
}

void UavaUIRadar::Render_Widget( FCanvas* Canvas )
{
	static bool	bTest = FALSE;

	if (!Map && MapMaterial != NULL)
	{
		UMaterialInstanceConstant* Instance = ConstructObject<UMaterialInstanceConstant>(UMaterialInstanceConstant::StaticClass());
		Instance->SetParent(MapMaterial);		
		Map					= Instance;
		LastMinimapTexture	= NULL;
	}

	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	if ( WorldInfo && WorldInfo->MinimapTexture != NULL && LastMinimapTexture != WorldInfo->MinimapTexture )
	{
		LastMinimapTexture = WorldInfo->MinimapTexture;
		Map->SetTextureParameterValue(TEXT("MapTexture"), LastMinimapTexture );
	}

	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	if ( PlayerOwner == NULL )	return;
	AavaPawn* PawnOwner					= GetPlayerViewTarget();
										  //PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;
	AavaPlayerReplicationInfo* PRIOwner = PlayerOwner ? Cast<AavaPlayerReplicationInfo>( PlayerOwner->PlayerReplicationInfo ) : NULL;
	AavaHUD* HUD						= PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	BYTE OwnerTeam						= PlayerOwner ? GetTeamNum( PlayerOwner ) : 255;

	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

	FLinearColor	White( 1, 1, 1, Opacity );
	FVector			Origin = PawnOwner ? PawnOwner->Location : PlayerOwner->Location;							/// Origin ; Player 위치

	FRotator		ViewRotator( 0, PawnOwner ? -PawnOwner->eventGetViewRotation().Yaw : -PlayerOwner->Rotation.Yaw, 0 );
	FVector			view_Location = PawnOwner ? PawnOwner->Location : PlayerOwner->Location;


	const FLOAT UUToMeter = 1.0f / 53.0f;															// 1 meter = 53 uu
	Scale = UUToMeter * ZoomScale;
	if ( PawnOwner != NULL && PawnOwner->MinimapScale > 0.0 )
		Scale *= PawnOwner->MinimapScale;

	if (Map != NULL && MapSize[0] > 0)
	{
		FLOAT Rotation = ViewRotator.Yaw / 65536.0f;
		FLOAT Zoom = 1.0f;		
		FLOAT MapScale = 1.0f;
		FVector MinimapCenter( 0, 0, 0 );

		if (WorldInfo)
		{			
			/// Map texture가 실제 map meter에 mapping?
			MapScale = WorldInfo->MinimapScale;						
			MinimapCenter = WorldInfo->MinimapOffset;
		}

		static FName FName_RowX( TEXT("RowX" ) );
		static FName FName_RowY( TEXT("RowY" ) );
		static FName FName_Opacity( TEXT("Opacity") );
		INT TextureWidth = LastMinimapTexture ? LastMinimapTexture->GetSurfaceWidth() : 512.0f;		
		INT TextureHeight = LastMinimapTexture ? LastMinimapTexture->GetSurfaceHeight() : 512.0f;
		FLOAT TextureWidthRate	=	512.0f / TextureWidth;
		FLOAT TextureHeightRate =	512.0f / TextureHeight;
		const FLOAT	MeterPerTexel = WorldInfo && WorldInfo->MinimapMeterPerTexel > 0.f ? WorldInfo->MinimapMeterPerTexel : 
									GEngine->MeterPerTexel > 0.0f ? GEngine->MeterPerTexel : 1.0f;	// meter per texel

		// compute offset
		FVector Offset = Origin - MinimapCenter;
		FLOAT TexCoordOffset_S_FromCenter = (Offset.Y * UUToMeter / (FLOAT)MeterPerTexel) / TextureWidth * TextureWidthRate;
		FLOAT TexCoordOffset_T_FromCenter = (Offset.X * UUToMeter / (FLOAT)MeterPerTexel) / TextureHeight * TextureHeightRate;

		// meter space
		FVector LeftTop		( -MapSize[0] / 2.0f, -MapSize[1] / 2.0f, 0.0f );
		FVector RightTop	(  MapSize[0] / 2.0f, -MapSize[1] / 2.0f, 0.0f );
		FVector LeftBottom	( -MapSize[0] / 2.0f,  MapSize[1] / 2.0f, 0.0f );

		// zoom scale
		LeftTop /= ZoomScale;
		RightTop /= ZoomScale;
		LeftBottom /= ZoomScale;

		if ( PawnOwner != NULL )
		{
			if ( PawnOwner->MinimapScale > 0.0 )
			{
				LeftTop /= PawnOwner->MinimapScale;
				RightTop /= PawnOwner->MinimapScale;
				LeftBottom /= PawnOwner->MinimapScale;
			}
		}

		FMatrix RadarRotateMatrix = FRotationMatrix( FRotator( 0, -ViewRotator.Yaw, 0 ) );

		// texture space
		LeftTop		= RadarRotateMatrix.TransformFVector( LeftTop )		/ (FLOAT)MeterPerTexel;
		RightTop	= RadarRotateMatrix.TransformFVector( RightTop )	/ (FLOAT)MeterPerTexel;
		LeftBottom	= RadarRotateMatrix.TransformFVector( LeftBottom )	/ (FLOAT)MeterPerTexel;

		// texcoord space
		FVector2D CoordOffset		= FVector2D( TexCoordOffset_S_FromCenter + 0.5f, -TexCoordOffset_T_FromCenter + 0.5f );
		FVector2D LeftTopCoord		= FVector2D( LeftTop.X / TextureWidth * TextureWidthRate, LeftTop.Y / TextureHeight * TextureHeightRate ) + CoordOffset;
		FVector2D RightTopCoord		= FVector2D( RightTop.X / TextureWidth * TextureWidthRate, RightTop.Y / TextureHeight * TextureHeightRate ) + CoordOffset;
		FVector2D LeftBottomCoord	= FVector2D( LeftBottom.X / TextureWidth * TextureWidthRate, LeftBottom.Y / TextureHeight * TextureHeightRate ) + CoordOffset;

		FVector2D SAxis = RightTopCoord - LeftTopCoord;
		FVector2D TAxis = LeftBottomCoord - LeftTopCoord;

		Map->SetVectorParameterValue( FName_RowX, FLinearColor( SAxis.X, TAxis.X, LeftTopCoord.X ) );
		Map->SetVectorParameterValue( FName_RowY, FLinearColor( SAxis.Y, TAxis.Y, LeftTopCoord.Y ) );
		Map->SetScalarParameterValue(FName_Opacity, Opacity );

		::DrawTile( Canvas, X + MapOffset[0], Y + MapOffset[1], MapSize[0], MapSize[1], 0, 0, 1, 1, Map->GetInstanceInterface(0));
	}


	AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
	
	FRotator	view_Rotator;
	// GetPlayerViewPoint 도 native 로 빼야 하는데 쉽지 않을것 같다...

	//view_Location	= FVector(0,0,0);
	//view_Rotator	= FRotator(0,0,0);
	
	/// Background!
	if (Image != NULL && BackgroundCoordinates.IsValidIndex(0) && BackgroundColor.IsValidIndex(0) )
	{		
		const FTextureCoordinates& BGCoord = BackgroundCoordinates(0);
		FLinearColor BGColor = BackgroundColor(0);
		BGColor.A *= BackgroundOpacity;
		DrawTileRotate( Canvas, Image, X + MapOffset[0] + MapSize[0]/2, Y + MapOffset[1] + MapSize[1]/2, ViewRotator.Yaw * 360.0f / 65535.0f, -MapSize[0]/2,-MapSize[1]/2, MapSize[0], MapSize[1], BGCoord.U, BGCoord.V, BGCoord.UL, BGCoord.VL, BGColor);
		//		DrawTile( Canvas, Image, X, Y, XL, YL, Coordinates.U, Coordinates.V, Coordinates.UL, Coordinates.VL, White );
	}

	FString		Message;
	FColor		BaseTeamColor;
	if ( PRIOwner != NULL && Cast<AavaTeamInfo>(PRIOwner->Team) != NULL )
		BaseTeamColor = (Cast<AavaTeamInfo>(PRIOwner->Team))->BaseTeamColor[OwnerTeam];

	if ( HUD == NULL )	return;

	const FIconCodeInfo* IconInfo = NULL;
	for ( INT i = 0 ; i < HUD->RadarDisplayedActors.Num() ; ++i )
	{
		const FRadarDisplayedActor& rda = HUD->RadarDisplayedActors(i);

		if (rda.DisplayedActor == NULL)
		{
			HUD->RadarDisplayedActors.Remove( i-- );
			continue;
		}
		//if( rda.DisplayedActor->bHidden == TRUE )
		//	continue;

		if ( ( rda.TeamIndex == 0 || rda.TeamIndex == 1 ) && !PlayerOwner->eventIsSameTeamByIndex( rda.TeamIndex ) )
			continue;

		// Leader 에게만 보이는 Icon 이다....
		if ( rda.bOnlyLeader && PRIOwner != NULL && PRIOwner->bSquadLeader == FALSE )
			continue;

		// 최우선 목표와 겹쳐진 경우 찍지 않는다...
		if ( HUD->targetActor == rda.DisplayedActor )
			continue;

		AavaPawn* Pawn = Cast<AavaPawn>(rda.DisplayedActor);
		if( Pawn != NULL && Pawn->SpecialInventory != NULL )
			continue;

		INT IconCode = rda.IconCode;
		IconInfo = GetRadarIconInfo(HUD,IconCode);
		if ( IconInfo != NULL )
		{
			if ( IconInfo->Image == NULL )	
			{	
				Render_Object( Canvas, rda.DisplayedActor->Location - Origin, IconInfo->Color, ViewRotator );
			}
			else						
			{
				Render_Object( Canvas, 
					rda.DisplayedActor->Location - Origin, 
					IconInfo, 
					ViewRotator,
					1.0f, 
					-1.0f, 
					FALSE, 
					FLinearColor(1.f,1.f,1.f),
					rda.bAdjustRotate, 
					( ViewRotator.Yaw + rda.DisplayedActor->Rotation.Yaw ) * 360.0f/65535.0f );
			}
		}
	}

	FLinearColor DrawColor;
	ULocalPlayer* LocalPlayer= GetLocalPlayer();
	for ( INT i = 0 ; i < HUD->DrawPawnList.Num() ; ++i )
	{		
		//AActor* Actor = *It;
		AavaPawn* Pawn = Cast<AavaPawn>( HUD->DrawPawnList(i).Pawn );
		if (Pawn != NULL)
		{			
			if ( Pawn->Health < 0 || Pawn->bDeleteMe )
				continue;

			// GetTeamNum 를 가지고 올때 Pawn 을 이용하지 말것...
			// PlayerOwner 를 이용해야만 함, Pawn 이 TearOff 되었을 경우 TeamNum 를 가지고 올 수 없음
			if ( Pawn == PawnOwner )
			{
				if ( Pawn->bTargetted && Pawn->bDrawNotifyTargetted )
				{
					FLOAT Wave			=	sinf( ( GWorld->GetTimeSeconds() - Pawn->TargettedTime ) * 2 * 3.141592f / TargettedWaveDuration ) + 1.0f;
					FLOAT TargetScale	=	TargettedMinScale + ( TargettedMaxScale - TargettedMinScale ) * Wave / 2.0f;
					Render_Object( Canvas, 
						FVector(0,0,0), 
						GetRadarIconInfo( HUD, TargettedCode ), 
						ViewRotator, 
						1.0f, 
						-1.0f, 
						FALSE,
						FLinearColor(1,1,1),
						FALSE,
						0.0f,
						TargetScale );
				}
				continue;
			}
			
			AavaPlayerReplicationInfo* PRI = Cast<AavaPlayerReplicationInfo>( GetPRI( Pawn ) );
			if( PRI == NULL )
				continue; 

			//데미지를 받았는지 조사하고 깜빡여야할지 조사
			UBOOL bDamaged = Pawn->LastPainTime > 0.0f && (Pawn->LastPainTime + DamageDisplayTime) > GWorld->GetTimeSeconds();
			FLOAT DisplayElapsed = (GWorld->GetTimeSeconds() - Pawn->LastPainTime);
			FLOAT Period = BlinkPeriod;	// milli second
			FLOAT CurrentPos = ((INT)(DisplayElapsed * 1000.0f) % (INT)(Period*1000.0f)) / 1000.0f;	//milli second
			UBOOL bShowDamaged = bDamaged && (Period * 0.5f > CurrentPos);

			// 퀵챗을 추가로 찍어줘야할지 조사
			UBOOL bRaiseQuickChat = Pawn->QuickChatUpdateTime > 0.0f && (Pawn->QuickChatUpdateTime + QuickChatDisplayTime) > GWorld->GetTimeSeconds();

			// 플레이어 상세정보를 표시하는 디버깅 옵션
			UBOOL bShowPlayerDetail = HUD != NULL && HUD->bShowPlayerDetail;

			if( Pawn == PawnOwner )
			{
				// 자기자신을 그리는 부분( 현재 Radar에서는 그려줄 필요없으나 FullScreenMap에서는 필요하다)
				// QA에서 사용할 플레이어 상세정보
				if( bShowPlayerDetail )
				{
					FVector2D ViewportSize;
					INT XL, YL;
					GetViewportSize(ViewportSize);
					Message = FString::Printf( TEXT( "%s ( HP = %d, AP = %d" ), *(PRI->PlayerName), Pawn->Health, Pawn->Armor_Stomach);
					StringSize(Font,XL, YL, *Message);
					DrawStringCentered( Canvas, (ViewportSize.X - XL)/2, 0 , *Message, Font , BaseTeamColor );
				}
			}
			else
			{		
				// 우리편을 HUD 에 찍어준다...
				DrawPawnInHUD( Canvas, Pawn, PawnOwner, PlayerOwner, view_Location, view_Rotator, OwnerTeam );

				// 우리편을 Radar 에 찍어준다...
				if( PlayerOwner->eventIsSameTeamByIndex( GetTeamNum(Pawn) ) )
				{
					FLinearColor	DrawColor		= HUD->FriendlyColor;
					INT				IconDataIndex	= ( 0 <= PRI->CurrentSpawnClassID && PRI->CurrentSpawnClassID <= 2 ) ? PRI->CurrentSpawnClassID : -1;
					INT				IconCode		= FriendCodes.IsValidIndex(IconDataIndex) ? FriendCodes(IconDataIndex) : -1;
					INT				IconCodeDamaged = FriendDamagedCodes.IsValidIndex(IconDataIndex) ? FriendDamagedCodes(IconDataIndex) : -1;
					const FIconCodeInfo* IconInfo;
					if( bShowDamaged )
					{
						if( (IconInfo = GetRadarIconInfo(HUD,IconCodeDamaged)) != NULL )	Render_Object( Canvas, Pawn->Location - Origin, IconInfo, ViewRotator);
						else															Render_Object( Canvas, Pawn->Location - Origin, DrawColor ,ViewRotator);
					}
					
					FLinearColor DrawColorPawn = Pawn->eventIsBIA() ? HUD->BIAColor : HUD->FriendlyColor;
					//if( Pawn->SpecialInventory != NULL )	DrawColorPawn = Pawn->SpecialInventory->RadarIconColor;
					if((IconInfo = GetRadarIconInfo(HUD,IconCode)) != NULL )				Render_Object( Canvas, Pawn->Location - Origin, IconInfo, ViewRotator,1.0,-1.0f, TRUE ,DrawColorPawn);
					else																Render_Object( Canvas, Pawn->Location - Origin, DrawColorPawn, ViewRotator);

					if( Pawn->SpecialInventory != NULL )
					{
						FVector2D resultPos, scaledPos;
						FLOAT DegAngle;
						UBOOL bOutside;
						if ( CalcPosInRadar( Pawn->Location - Origin, ViewRotator, resultPos, scaledPos, bOutside, DegAngle ,false) ) 
						{
							DrawStringCentered(Canvas, resultPos.X,resultPos.Y + 5, *Pawn->SpecialInventory->ItemShortName, TextFont, DrawColorPawn);
						}
					}

					// Quick Chat 을 하고 있는 놈을 찍어준다...
					if( bRaiseQuickChat )
					{
						FLOAT Elapsed = GWorld->GetTimeSeconds() - Pawn->QuickChatUpdateTime;
						FLOAT Current = ((INT)(Elapsed*1000.0f) % (INT)(BlinkPeriod * 1000.0f)) / 1000.0f;
						UBOOL bDrawQuickChat = Current < BlinkPeriod * 0.5f;
						if( bDrawQuickChat )
						{
							if((IconInfo = GetRadarIconInfo(HUD,QuickChatCode)) != NULL )
								Render_Object( Canvas, Pawn->Location - Origin, IconInfo, ViewRotator,1.0f, Pawn->QuickChatUpdateTime);
							else
								Render_Object( Canvas, Pawn->Location - Origin, DrawColor.White, ViewRotator);
						}
					}
				}
				else
				{
					//적군은 데미지를 받았고 깜빡임 중에 보일때만 찍어준다.
					// 그리고 시야내에 들어왔을때만 찍어준다.
					if( ( bDamaged && Pawn->bLastTakeHitVisibility ) || Pawn->bTargetted || ( PawnOwner != NULL && IsUsingBinocular( PawnOwner ) && Pawn->bDetected  ) )
					{
						//					FLOAT Alpha = Clamp(1.0f - (DisplayElapsed)/DamageDisplayTime, 0.0f, 1.0f) * (appCos(2*PI*CurrentPos/Period) * 2.0f);
						INT IconDataIndex = ( 0 <= PRI->CurrentSpawnClassID && PRI->CurrentSpawnClassID <= 2 ) ? PRI->CurrentSpawnClassID : -1;
						INT IconCode = EnemyCodes.IsValidIndex(IconDataIndex) ? EnemyCodes(IconDataIndex) : -1;
						INT IconCodeDamaged = EnemyDamagedCodes.IsValidIndex(IconDataIndex) ? EnemyDamagedCodes(IconDataIndex) : -1;
						const FIconCodeInfo* IconInfo;
						if( bShowDamaged )
						{
							if( (IconInfo = GetRadarIconInfo(HUD,IconCodeDamaged)) != NULL )	Render_Object( Canvas, Pawn->Location - Origin, IconInfo, ViewRotator);
							else															Render_Object( Canvas, Pawn->Location - Origin, DrawColor.White ,ViewRotator);
						}
						if( (IconInfo = GetRadarIconInfo(HUD,IconCode)) != NULL )				Render_Object( Canvas, Pawn->Location - Origin, IconInfo, ViewRotator);
						else																Render_Object( Canvas, Pawn->Location - Origin, DrawColor.White, ViewRotator);
					}
				}
			}
			
			if( bShowPlayerDetail )
			{
				if ( PRI != NULL && ( (PlayerOwner->Rotation.Vector() | (Pawn->Location - view_Location)) > 0 ) )
				{
					FLOAT	Distance = (view_Location - Pawn->Location).Size();
					FVector2D	ScreenLoc;
					if (UnscaledWorldToScreen( Pawn->Location + Pawn->CylinderComponent->CollisionHeight * FVector(0,0,1), ScreenLoc, TRUE ))
					{					
						Message = FString::Printf( TEXT( "%s ( HP = %d, AP = %d, Dist = %d, Stress = %d)" ), *(PRI->PlayerName), Pawn->Health, Pawn->Armor_Stomach, (INT)(Distance / 53.0f), Pawn->StressLevel );
						DrawStringCentered( Canvas, ScreenLoc.X, ScreenLoc.Y, *Message, Font , BaseTeamColor );						
					}
				}
			}
		}
		else
		{
			AavaVehicle* Vehicle = Cast<AavaVehicle>( HUD->DrawPawnList(i).Pawn );
			if ( Vehicle != NULL )
			{
				DrawVehicleInHUD( Canvas, Vehicle, PawnOwner, PlayerOwner, view_Location, view_Rotator, OwnerTeam == GetTeamNum( Vehicle ) );
			}
		}

	}

	// Draw dead players
	if( HUD != NULL && PRIOwner != NULL && GRI != NULL )
	{
		for( INT i = HUD->DeadPlayerList.Num() - 1 ; i >= 0 ; i--)
		{
			if( HUD->DeadPlayerList(i).PRI == NULL )
			{
				HUD->DeadPlayerList.Remove(i,1);
				continue;
			}

			// Need to Remove Some DeadBodies in the Radar.
			if( GRI->bReinforcement && bOverrideReinforcementTime )
			{
				// Use UserDefined-ReinforcementTime to remove deadbodies.
				if( HUD->DeadPlayerList(i).UpdateTime + ReinforcementTime <= GWorld->GetTimeSeconds() )
				{
					HUD->DeadPlayerList.Remove(i,1);
					continue;
				}
			}

			const FDeadPlayerInfo& DeadInfo = HUD->DeadPlayerList(i);
			UBOOL bOwnerTeam = PlayerOwner->eventIsSameTeamByIndex( DeadInfo.PRI->Team != NULL ? DeadInfo.PRI->Team->TeamIndex : 255 );
			const FIconCodeInfo* IconInfo;
			FLOAT Alpha = bOverrideReinforcementTime ? 1.0f -  (GWorld->GetTimeSeconds() - DeadInfo.UpdateTime) / ReinforcementTime : 1.0f;
			Alpha *= 2.0f;	// 보여줘야하는 시간 절반동안 Alpha값이 1.0으로 지속되는 효과.
			if( (IconInfo = GetRadarIconInfo(HUD,bOwnerTeam ? FriendDeadCode : EnemyDeadCode)) != NULL )
				Render_Object( Canvas, DeadInfo.Location - Origin, IconInfo, ViewRotator, Alpha);
			else
				Render_Object( Canvas, DeadInfo.Location - Origin, DrawColor.White, ViewRotator, Alpha);
		}
	}


	static AActor* Test;

	if ( PawnOwner != NULL )
	{
		for ( INT i = 0 ; i < HUD->POIList.Num() ; ++ i )
		{
			const AavaMissionNavPoint* NavPoint = HUD->POIList(i);
			float	TargettedTime = 0.0;
			int		TargetIconCode;
			
			if ( ( NavPoint->TeamIndex == 0 || NavPoint->TeamIndex == 1 ) && NavPoint->TeamIndex != OwnerTeam )
				continue;
			TargetIconCode = NavPoint->HUDIconCode;
			if ( HUD->targetActor == NavPoint )	
			{
				TargettedTime = HUD->TargettedTime;
				TargetIconCode=	HUD->HUDTopTargetIcon;
			}
			else if ( PawnOwner != NULL && IsUsingBinocular( PawnOwner ) == FALSE )	continue;
			FString	TargetName("");
			if ( NavPoint->NameIndex >= 0 && NavPoint->NameIndex < sizeof( AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->PlaceOfInterest ) / sizeof( AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->PlaceOfInterest[0] ))
				TargetName = AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->PlaceOfInterest[NavPoint->NameIndex];
			DrawTargetInHUD( Canvas, PawnOwner, PlayerOwner, NavPoint, TargettedTime, TargetIconCode, TargetName );
		}
	}
			

	if ( HUD->bSignalActivate == TRUE )
	{
		IconInfo = GetRadarIconInfo(HUD,HUD->SignalIcon );
		if ( IconInfo != NULL )
		{
			if ( IconInfo->Image == NULL )
			{
				Render_Object( Canvas, HUD->SignalPos - Origin, IconInfo->Color, ViewRotator );
			}
			else
			{
				Render_Object( Canvas, HUD->SignalPos - Origin, IconInfo, ViewRotator );
			}
		}
	}

	if ( HUD->targetActor != NULL )
	{
		IconInfo = GetRadarIconInfo(HUD, HUD->RadarTopTargetIcon );
		if ( IconInfo != NULL )
		{
			if ( IconInfo->Image == NULL )	Render_Object( Canvas, HUD->targetActor->Location - Origin, IconInfo->Color, ViewRotator );
			else							Render_Object( Canvas, HUD->targetActor->Location - Origin, IconInfo, ViewRotator );
		}
	}

}

// @ToDo : Render_Object 에 거리 표시를 해줄지에 대한 Flag 를 설정해서 true 라면 거리표시를 해주도록 한다....
void UavaUIRadar::Render_Object( FCanvas* Canvas, const FVector& DeltaLocation, const FLinearColor& Color, const FRotator& ViewRotator, FLOAT Alpha )
{
	FVector2D resultPos, scaledPos;
	FLOAT DegAngle;
	UBOOL bOutside;
	if (!CalcPosInRadar( DeltaLocation, ViewRotator, resultPos, scaledPos, bOutside, DegAngle ,bClamp)) 
		return;

	FLinearColor LC( Color );
	LC.A *= Opacity * Clamp(Alpha, 0.0f, 1.0f);

	// 자신보다 위에 있는 경우
	if ( DeltaLocation.Z > Hysteresis )
	{
		DrawRect( Canvas, scaledPos.X - 1, scaledPos.Y + 1, 3, 1, DefaultTexture, LC );
		DrawRect( Canvas, scaledPos.X, scaledPos.Y - 1, 1, 3, DefaultTexture, LC );		
	}
	// 자신보다 아래에 있는 경우
	else if ( - DeltaLocation.Z > Hysteresis )
	{
		DrawRect( Canvas, scaledPos.X - 1, scaledPos.Y - 1, 3, 1, DefaultTexture, LC );
		DrawRect( Canvas, scaledPos.X, scaledPos.Y - 1, 1, 3, DefaultTexture, LC );		
	}
	else
	{
		DrawRect( Canvas, scaledPos.X - 1, scaledPos.Y - 1, 3, 3, DefaultTexture, LC );
		//DrawTile( Canvas, Image, scaledPos.X - 1, scaledPos.Y - 1, 32, 32, 0, 0, 512, 512, Color );
	}

	if( bOutside && bShowOutIcon && OutIcon != NULL)
	{
		FLOAT Width = OutIcon->GetSurfaceWidth();
		FLOAT Height = OutIcon->GetSurfaceHeight();
		FIntPoint DrawExtent = bIgnoreOutIconExtent ? FIntPoint(Width, Height) : OutIconExtent;
		FTextureCoordinates TexCoord = bIgnoreOutIconCoord ? FTextureCoordinates(0,0,Width,Height) : OutIconCoord;
		FLinearColor OutColor = OutIconColor;
		OutColor *= ( Clamp(Alpha,0.0f,1.0f) * Opacity );
		DrawTileRotate(Canvas, OutIcon, resultPos.X, resultPos.Y, DegAngle + RotationOffsetDegree, -DrawExtent.X/2, -DrawExtent.Y/2, DrawExtent.X, DrawExtent.Y, TexCoord.U, TexCoord.V, TexCoord.UL, TexCoord.VL, OutIconColor );
	}
}

void UavaUIRadar::Render_Object( FCanvas* Canvas, const float xpos, const float ypos, const float sizex, const float sizey, const FIconCodeInfo* IconInfo, FLOAT Alpha , FLOAT UpdateTime, UBOOL bOverrideColor, FLinearColor OverrideColor)
{
	if( IconInfo->Image == NULL )
		return;
	FLinearColor LC( bOverrideColor ? OverrideColor : IconInfo->Color );
	LC.A *= ( Opacity * Clamp(Alpha, 0.0f, 1.0f) );
	FTextureCoordinates	ImageCoordinates;

	if( !IconInfo->bAnimated || UpdateTime < 0.0f)
	{
		ImageCoordinates = IconInfo->Coord;
		DrawTile( Canvas, IconInfo->Image, (INT)(xpos), (INT)(ypos), sizex, sizey,
			ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL, LC );
	}
	else
	{
		ImageCoordinates = IconInfo->Coord;
		INT CurrentTime = (INT)(GWorld->GetTimeSeconds() * 1000.f);	//milli second
		INT Period = (INT)(IconInfo->AnimPeriod * 1000.0f); // milli second
		INT TimeElapsed = CurrentTime - (INT)(UpdateTime*1000.0f); // milli second
		INT CurrentPos = TimeElapsed % Period;
		INT CurrentImageIndex = CurrentPos * IconInfo->AnimImgCnt / Period;
		INT ImageIndex = CurrentImageIndex;
		while( ImageCoordinates.U + ImageCoordinates.UL * (ImageIndex + 1) > IconInfo->Image->GetSurfaceWidth())
		{
			ImageIndex -= (IconInfo->Image->GetSurfaceWidth() - ImageCoordinates.U) / (ImageCoordinates.UL);
			ImageCoordinates.U = 0;
			ImageCoordinates.V += ImageCoordinates.VL;
		}
		ImageCoordinates.U += (ImageIndex * ImageCoordinates.UL);
		DrawTile( Canvas, IconInfo->Image, (INT)(xpos), (INT)(ypos), sizex, sizey,
			ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL, LC );
	}

}

void UavaUIRadar::Render_Object( FCanvas* Canvas, const FVector& DeltaLocation, const FIconCodeInfo* IconInfo, const FRotator& ViewRotator, FLOAT Alpha , FLOAT UpdateTime, UBOOL bOverrideColor, FLinearColor OverrideColor, UBOOL bAdjustRotator, FLOAT RotateDegree, FLOAT Scaler )
{
	FVector2D resultPos, scaledPos;
	UBOOL bOutside;
	FLOAT DegAngle;

	if( IconInfo->Image == NULL )
		return;

	if (!CalcPosInRadar( DeltaLocation, ViewRotator, resultPos, scaledPos, bOutside, DegAngle, bClamp)) 
		return;

	FLinearColor LC( bOverrideColor ? OverrideColor : IconInfo->Color );
	LC.A *= ( Opacity * Clamp(Alpha, 0.0f, 1.0f) );

	FTextureCoordinates	ImageCoordinates;
	if( ! IconInfo->bAnimated || UpdateTime < 0.0f)
	{
		ImageCoordinates = IconInfo->Coord;		
		if ( !bAdjustRotator )
		{
			DrawTile( Canvas, 
					  IconInfo->Image, 
					  (INT)(scaledPos.X - ImageCoordinates.UL * Scaler /2), 
					  (INT)(scaledPos.Y - ImageCoordinates.VL * Scaler /2), 
					  ImageCoordinates.UL * Scaler, 
					  ImageCoordinates.VL * Scaler,
					  ImageCoordinates.U,
					  ImageCoordinates.V,
					  ImageCoordinates.UL,
					  ImageCoordinates.VL,
					  LC );
		}
		else
		{
			DrawTileRotate( Canvas, 
							IconInfo->Image, 
							resultPos.X, 
							resultPos.Y, 
							RotateDegree,
							-ImageCoordinates.UL/2, 
							-ImageCoordinates.VL/2, 
							ImageCoordinates.UL,
							ImageCoordinates.VL,
							ImageCoordinates.U,
							ImageCoordinates.V,
							ImageCoordinates.UL,
							ImageCoordinates.VL,
							LC );
		}
	}
	else
	{
		ImageCoordinates = IconInfo->Coord;
		INT CurrentTime = (INT)(GWorld->GetTimeSeconds() * 1000.f);	//milli second
		INT Period = (INT)(IconInfo->AnimPeriod * 1000.0f); // milli second
		INT TimeElapsed = CurrentTime - (INT)(UpdateTime*1000.0f); // milli second
		INT CurrentPos = TimeElapsed % Period;
		INT CurrentImageIndex = CurrentPos * IconInfo->AnimImgCnt / Period;
		INT ImageIndex = CurrentImageIndex;
		while( ImageCoordinates.U + ImageCoordinates.UL * (ImageIndex + 1) > IconInfo->Image->GetSurfaceWidth())
		{
			ImageIndex -= (IconInfo->Image->GetSurfaceWidth() - ImageCoordinates.U) / (ImageCoordinates.UL);
			ImageCoordinates.U = 0;
			ImageCoordinates.V += ImageCoordinates.VL;
		}
		ImageCoordinates.U += (ImageIndex * ImageCoordinates.UL);
		DrawTile( Canvas, IconInfo->Image, (INT)(scaledPos.X - ImageCoordinates.UL/2), (INT)(scaledPos.Y - ImageCoordinates.VL/2), ImageCoordinates.UL, ImageCoordinates.VL,
			ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL, LC );
	}

	if( bOutside && bShowOutIcon && OutIcon != NULL)
	{
		FLOAT Width = OutIcon->GetSurfaceWidth();
		FLOAT Height = OutIcon->GetSurfaceHeight();
		FIntPoint DrawExtent = bIgnoreOutIconExtent ? FIntPoint(Width, Height) : OutIconExtent;
		FTextureCoordinates TexCoord = bIgnoreOutIconCoord ? FTextureCoordinates(0,0,Width,Height) : OutIconCoord;
		FLinearColor OutColor = OutIconColor;
		OutColor.A *=( Clamp(Alpha,0.0f,1.0f) * Opacity );
		DrawTileRotate(Canvas, OutIcon, resultPos.X, resultPos.Y, DegAngle + RotationOffsetDegree, -DrawExtent.X/2, -DrawExtent.Y/2, DrawExtent.X, DrawExtent.Y, TexCoord.U, TexCoord.V, TexCoord.UL, TexCoord.VL, OutIconColor );
	}
}

void UavaUIProgressBar::UpdateFadeInfos()
{
	// 0, 1사이로 clamp :) 
	Ratio = Clamp( Ratio, 0.0f, 1.0f );

	/// 마지막 value는 실제 값이므로 냅두자.
	for (INT i=0; i<FadeInfos.Num()-1; )
	{
		FGaugeFadeInfo& FI = FadeInfos(i);

		if (FadeTime > 0)
			FI.Alpha = Clamp( FI.Alpha - GWorld->GetDeltaSeconds() / FadeTime, 0.0f, 1.0f );
		else
			FI.Alpha = 0;

		if (FI.Alpha == 0)
		{			
			FadeInfos.Remove(0);
		}
		else
			++i;
	}

	/// 하나도 없을 때 무조건 추가!
	if (FadeInfos.Num() == 0)
	{
		FGaugeFadeInfo FI;
		FI.Alpha = 1.0f;
		FI.Value = Ratio;

		FadeInfos.AddItem( FI );
	}
	else 
	{
		FLOAT LastValue = FadeInfos( FadeInfos.Num()-1 ).Value ;		

		/// 얼레 더 커졌다.
		if (LastValue < Ratio)
		{
			FadeInfos.Empty(1);

			FGaugeFadeInfo FI;
			FI.Alpha = 1.0f;
			FI.Value = Ratio;

			FadeInfos.AddItem( FI );
		}
		/// 줄어들었다!
		else if (LastValue > Ratio)
		{
			FGaugeFadeInfo FI;
			FI.Alpha = 1.0f;
			FI.Value = Ratio;

			FadeInfos.AddItem( FI );
		}
	}
}

extern UavaNetHandler *GavaNetHandler;

void UavaUIHostLoadingBar::Render_Widget( FCanvas* Canvas )
{	
	Ratio = GavaNetHandler ? GavaNetHandler->HostLoadingProgress / 100.0f : 0.75f;	

	UpdateFadeInfos();

	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Render_Progress( Canvas, Image, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );
}

void UavaUIWeaponGauge::Render_Widget( FCanvas* Canvas )
{
	AavaWeapon* Weapon = GetViewTargetCurrentWeapon();
	if ( Weapon )
	{
		if ( Weapon->MaintenanceRate < 0 )
			return;
		Ratio = (FLOAT)Weapon->MaintenanceRate/100.0f;
	}
	else
	{
		Ratio = TestRatio;
	}

	UpdateFadeInfos();

	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Render_Progress( Canvas, Image, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );
}

void UavaUIAmmoGauge::Render_Widget( FCanvas* Canvas )
{
	AavaWeapon* Weapon = GetViewTargetCurrentWeapon();
	if ( Weapon != NULL )
	{
		AavaWeap_BaseGun* Gun = Cast<AavaWeap_BaseGun>(Weapon);
		if ( bReloadGuage && Gun != NULL )
		{
			if ( Gun->ClipCnt > 0)			Ratio = (FLOAT)Gun->ReloadCnt/Gun->ClipCnt;
			else							Ratio = 1.0;
		}
		else
		{
			if ( Weapon->MaxAmmoCount > 0 )	Ratio = (FLOAT)Weapon->AmmoCount/Weapon->MaxAmmoCount;
			else							Ratio = 1.0;
		}
	}
	else
	{
		Ratio = TestRatio;
	}

	UpdateFadeInfos();

	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Render_Progress( Canvas, Image, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );
}

void UavaUIHealthGauge::Render_Widget( FCanvas* Canvas )
{
	AavaPawn* PawnOwner = GetPlayerViewTarget();
	if (PawnOwner)
	{
		Ratio = (FLOAT)PawnOwner->Health / PawnOwner->HealthMax;
	}
	else
	{
		Ratio = GIsEditor ? TestRatio : 0;
	}
	UpdateFadeInfos();
	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Render_Progress( Canvas, Image, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );
}

void UavaUIArmorGauge::Render_Widget( FCanvas* Canvas )
{
	AavaPawn* PawnOwner = GetPlayerViewTarget();
	if (PawnOwner)
	{
		Ratio = (FLOAT)PawnOwner->Armor_Stomach / PawnOwner->ArmorMax;
	}
	else
	{
		Ratio = GIsEditor ? TestRatio : 0;
	}
	UpdateFadeInfos();
	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Render_Progress( Canvas, Image, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );
}

void UavaUIHelmet::Render_Widget( FCanvas* Canvas )
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;	
	FLinearColor Color	= NoHelmetColor;
	FTextureCoordinates ImageCoordinates = NoHelmetCoordinates;

	if (PawnOwner)
	{
		if ( PawnOwner->bHasHelmet )
		{
			ImageCoordinates = HelmetCoordinates;
			Color			 = NormalColor;
		}
		else
		{
			ImageCoordinates = NoHelmetCoordinates;
			Color			 = NoHelmetColor;
		}
	}
	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
	DrawTile( Canvas, Image, X, Y, XL, YL, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL, Color );
}

void UavaUIProgressBar::Render_Widget( FCanvas* Canvas )
{
	AavaPawn* PawnOwner = GetPlayerViewTarget();
	Ratio = TestRatio;
	AavaPlayerReplicationInfo* PRI = PawnOwner ? Cast<AavaPlayerReplicationInfo>( PawnOwner->PlayerReplicationInfo ) : NULL;
	if( GIsGame && (PRI == NULL || PRI->GaugeMax <= 0) )
		return;

	if (PRI)
	{
		Ratio = (((FLOAT)PRI->GaugeCur) / PRI->GaugeMax);
	}	

	UpdateFadeInfos();

	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Render_Progress( Canvas, Image, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );
}

void UavaUIProgressBar::Render_Bar( FCanvas* Canvas, FLOAT Ratio, FLOAT RatioL, USurface* Surface, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL, const FLinearColor& DrawColor )
{
	if (Surface == NULL) return;

	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];			

	if (Surface)
	{		
		switch (Direction)
		{
		default :
		case AVAUIPROGRESSDirection_Left :
			DrawTile( Canvas, Surface, X + appTrunc( XL * Ratio ), Y, appTrunc( XL * RatioL ), YL, CU + appTrunc( CUL * Ratio ), CV, appTrunc( CUL * RatioL ), CVL, DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Right :
			DrawTile( Canvas, Surface, X + appTrunc( XL * (1-(Ratio+RatioL)) ), Y, appTrunc( XL * RatioL ), YL, CU + appTrunc( CUL * (1-(Ratio+RatioL)) ), CV, appTrunc( CUL * RatioL ), CVL, DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Up :
			DrawTile( Canvas, Surface, X, Y + appTrunc( YL * Ratio ), XL, appTrunc( YL * RatioL ), CU, CV + appTrunc( CVL * Ratio ), CUL, appTrunc( CVL * RatioL ), DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Down :
			DrawTile( Canvas, Surface, X, Y + appTrunc( YL * (1-(Ratio+RatioL)) ), XL, appTrunc( YL * RatioL ), CU, CV + appTrunc(CVL * (1 - (Ratio+RatioL))), CUL, appTrunc( CVL * RatioL ), DrawColor );	
			break;							
		}		
	}		
}

void UavaUIProgressBar::Render_Progress( FCanvas* Canvas, USurface* Tex, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL )
{
	FLinearColor LC_Box( BackgroundColor ), LC_Progress( ProgressColor ), LC_Alert( AlertColor ), LC_Fade( FadeColor );
	LC_Box.A *= Opacity;
	LC_Progress.A *= Opacity;
	LC_Alert.A *= Opacity;

	Render_Bar( Canvas, 0, 1, Tex, CU, CV, CUL, CVL, LC_Box );

	FLOAT PreviousRatio = 0.0f;

	FLinearColor LC;
	if (FadeInfos(FadeInfos.Num()-1).Value < AlertStartRatio)
	{
		LC = LC_Alert;
	}
	else
	{
		LC = LC_Progress;
	}

	LC.A *= Opacity;

	for (INT i=FadeInfos.Num()-1; i>=0; --i)
	{
		const FGaugeFadeInfo& FI = FadeInfos(i);				

		LC.A *= FI.Alpha;

		Render_Bar( Canvas, PreviousRatio, FI.Value - PreviousRatio, Tex, CU, CV, CUL, CVL, LC );

		LC = LC_Fade;
		LC.A *= Opacity;

		PreviousRatio = FI.Value;
	}			
}

void UavaUIDeathLog::Render_Widget( FCanvas* Canvas )
{	
	FLOAT CurrentTime = GWorld->GetTimeSeconds();

	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;
	AavaPlayerReplicationInfo* PRIOwner = PlayerOwner ? Cast<AavaPlayerReplicationInfo>( PlayerOwner->PlayerReplicationInfo ) : NULL;
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if ( HUD == NULL || PRIOwner == NULL  )
		return;

	INT i, count;
	FLOAT ExpireTime = Lifetime + DimTime;
	count = HUD->DeathMessages.Num();

	for (i=0; i<HUD->DeathMessages.Num(); ++i)
	{
		if( CurrentTime - HUD->DeathMessages(i).DeathTime <= ExpireTime )
		{
			count = i;
			break;
		}
	}

	for (; i<HUD->DeathMessages.Num(); ++i)
	{
		FSDeathMessage& msg= HUD->DeathMessages(i);

		/// Not initialized
		if (msg.Height == 0)
		{
			if (msg.KilledName.Len() > 0 && Font)
			{
				msg.Str_Killed = FString::Printf( TEXT(" %s "), *msg.KilledName );

				FRenderParameters Parameters;

				Parameters.DrawFont = Font;
				UUIString::StringSize( Parameters, *msg.Str_Killed );

				msg.Width_Killed = appTrunc(Parameters.DrawXL);
				msg.Height = Max( msg.Height, appTrunc(Parameters.DrawYL) );
				msg.IconHeight = Parameters.DrawYL;
				msg.Width_Killed += msg.IconHeight;
			}			

			if (IconFont)
			{
				if (msg.bHeadShot)
				{
					if ( msg.bWallShot )
					{
						msg.Str_Icon = FString::Printf( TEXT("%sZ"), *msg.IconStr );
					}
					else
					{
						msg.Str_Icon = FString::Printf( TEXT("%sk"), *msg.IconStr );
					}
				}
				else if ( msg.bWallShot )
				{
					msg.Str_Icon = FString::Printf( TEXT("%sY"), *msg.IconStr );
				}
				else
				{
					msg.Str_Icon = msg.IconStr;
				}

				FRenderParameters Parameters;

				Parameters.DrawFont = IconFont;
				UUIString::StringSize( Parameters, *msg.Str_Icon );

				msg.Width_Icon = appTrunc(Parameters.DrawXL);
				msg.Height = Max( msg.Height, appTrunc(Parameters.DrawYL) - IconFontCutY );				
			}			

			if (msg.KillerName.Len() > 0 && Font)
			{
				msg.Str_Killer = FString::Printf( TEXT("%s "), *msg.KillerName );

				FRenderParameters Parameters;

				Parameters.DrawFont = Font;
				UUIString::StringSize( Parameters, *msg.Str_Killer );

				msg.Width_Killer = appTrunc(Parameters.DrawXL);
				msg.Height = Max( msg.Height, appTrunc(Parameters.DrawYL) );
				msg.IconHeight = Parameters.DrawYL;
				//msg.Width_Killer += Parameters.DrawYL;
			}			
		}
	}

	if( count > 0 )	HUD->DeathMessages.Remove(0, count);

	INT X, Y = appTrunc( RenderBounds[UIFACE_Top] );	

	UBOOL	bIsTeamGame		= IsTeamGame();
	UBOOL	bAmISpectator	= AmISpectator();

	for (INT i=0; i<HUD->DeathMessages.Num(); ++i)
	{		
		X = appTrunc( RenderBounds[UIFACE_Right] );

		const FSDeathMessage& msg=HUD->DeathMessages(i);

		/// 넘쳤다!
		if (Y + msg.Height > RenderBounds[UIFACE_Bottom])
			break;		

		/// Calculate opacity :)
		FLinearColor DrawColor( 1, 1, 1, Opacity );

		ExpireTime = CurrentTime-msg.DeathTime;

		FLOAT FadeRatio = ( 1.0f - (ExpireTime-Lifetime)/DimTime );
		FLinearColor NewShadowColor = HUD->ShadowColor;
		UBOOL bAlphaMod = ExpireTime > Lifetime; 
		if( bAlphaMod )
		{
			DrawColor.A *= FadeRatio;
			NewShadowColor.A *= FadeRatio;
		}

		int OwnerTeam;
		if ( PRIOwner->Team != NULL )	OwnerTeam = PRIOwner->Team->TeamIndex;
		else							OwnerTeam = 255;
		// 오른쪽에서 부터 그리기 때문에 죽은 사람부터 그려주자
		if (msg.Width_Killed)
		{				
			FLinearColor Color;

			if ( bIsTeamGame )
			{
				Color = ( OwnerTeam == 255 ? HUD->FriendlyColor : ( OwnerTeam == msg.KilledTeam ? HUD->FriendlyColor : HUD->EnemyColor) );
			}
			else
			{
				Color = bAmISpectator ? HUD->FriendlyColor : msg.bAmIVictim ? HUD->FriendlyColor : HUD->EnemyColor;
			}
			if( bAlphaMod )
				Color.A *= FadeRatio;
						
			//( OwnerTeam == 255 ? HUD->FriendlyColor : ( OwnerTeam == msg.KilledTeam ? HUD->FriendlyColor : HUD->EnemyColor ) );
			//Color.A = DrawColor.A;

			if ( bDrawOnlySlotNum == false )
			{
				X -= msg.Width_Killed;
				if( bDropShadow )
					DrawString( Canvas, X + ShadowX, Y + ShadowY + (msg.Height - msg.IconHeight)/2.0f , *msg.Str_Killed, Font, NewShadowColor );
				DrawString( Canvas, X, Y + (msg.Height - msg.IconHeight)/2.0f , *msg.Str_Killed, Font, Color );

				X -= msg.IconHeight;
				DrawLevelIcon( Canvas, HUD, X, Y + (msg.Height - msg.IconHeight)/2.0f , msg.IconHeight, msg.IconHeight,DrawColor.A, Max( msg.KilledLevel-1, 0 ) ,0,0);
			}

			if ( msg.KilledSlot >= 0 )
			{
				X -= msg.IconHeight;
				DrawSlotIcon( Canvas, msg.KilledTeam, msg.KilledSlot, X, Y + (msg.Height - msg.IconHeight)/2.0f , msg.IconHeight, msg.IconHeight,DrawColor.A, 0, 0 );
			}
		}

		if (msg.Width_Icon)
		{
			X -= msg.Width_Icon;
			if( bDropShadow )
				DrawString( Canvas, X + ShadowX, Y - IconFontCutY + ShadowY, *msg.Str_Icon, IconFont, NewShadowColor );
			DrawString( Canvas, X, Y - IconFontCutY, *msg.Str_Icon, IconFont, DrawColor );
		}						

		// 죽인 사람 그려주자
		if (msg.Width_Killer)
		{
			FLinearColor Color;
			if ( bIsTeamGame )
			{
				Color = ( OwnerTeam == 255 ? HUD->FriendlyColor : ( OwnerTeam == msg.KillerTeam ? HUD->FriendlyColor : HUD->EnemyColor) );
			}
			else
			{
				Color = bAmISpectator ? HUD->FriendlyColor : msg.bAmIKiller ? HUD->FriendlyColor : HUD->EnemyColor;
			}

			if( bAlphaMod )
				Color.A *= FadeRatio;

			if ( bDrawOnlySlotNum == false )
			{
				X -= msg.Width_Killer;
				if( bDropShadow )
					DrawString( Canvas, X + ShadowX, Y + ShadowY + (msg.Height - msg.IconHeight)/2.0f , *msg.Str_Killer, Font, NewShadowColor );
				DrawString( Canvas, X, Y + (msg.Height - msg.IconHeight)/2.0f , *msg.Str_Killer, Font, Color );			

				X -= msg.IconHeight;
				DrawLevelIcon( Canvas, HUD, X, Y + (msg.Height - msg.IconHeight)/2.0f , msg.IconHeight, msg.IconHeight,DrawColor.A, Max( msg.KillerLevel-1, 0 ) ,0,0);
			}


			if ( msg.KillerSlot >= 0 )
			{
				X -= msg.IconHeight;
				DrawSlotIcon( Canvas, msg.KillerTeam, msg.KillerSlot, X, Y + (msg.Height - msg.IconHeight)/2.0f , msg.IconHeight, msg.IconHeight,DrawColor.A, 0, 0 );
			}
		}

		Y += LineSpacing + msg.Height;
	}	
}

static FString GetFormattedTime( FLOAT fTime, UBOOL bHour = FALSE, UBOOL ms = FALSE )
{
	INT hour = 0, minute, second;
	FLOAT millisecond;
	FString Result;
	second = fTime;

	if ( fTime > 3600 && bHour )	
	{
		hour	= fTime / 3600;
		second	= fTime - hour * 3600;
	}
	minute		= second / 60;
	second		= fTime - minute * 60;
	millisecond	= fTime - minute * 60 - second;

	if ( bHour )
	{
		if( hour > 0 )	Result = FString::Printf( TEXT("%d:"), hour );
	}

	if ( hour == 0 )	Result = FString::Printf( TEXT("%s%2d:%02d"), *Result, minute, second );
	else				Result = FString::Printf( TEXT("%s%02d:%02d"), *Result, minute, second );

	if ( ms )
	{
		Result = FString::Printf( TEXT("%s:%02d"), *Result, appTrunc(millisecond) );		
	}

	return Result;
}

FLinearColor UavaUISimpleText::CalcWarmColor( const FLinearColor& SrcColor ) const
{
	const FLOAT Scaler = Hot_CurrentValue + 1.0f;
	FLinearColor OutColor = FLinearColor( SrcColor.R * Scaler, SrcColor.G * Scaler, SrcColor.B * Scaler, Min( 1.0f, SrcColor.A * Scaler ) );
	FLOAT MaxComponent = Max( Max( OutColor.R, OutColor.G ), OutColor.B );
	if (MaxComponent > 1)
	{
		OutColor.R /= MaxComponent;
		OutColor.G /= MaxComponent;
		OutColor.B /= MaxComponent;
	}	

	return OutColor;
}
void UavaUISimpleText::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize( inOwnerScene, inOwner );	
}

void UavaUISimpleText::CalculatePosition( FLOAT& X, FLOAT& Y, FLOAT XL, FLOAT YL )
{
	switch (HorizontalAlign)
	{
	case AVAUIALIGN_LeftOrTop :
		X = RenderBounds[UIFACE_Left];
		break;

	case AVAUIALIGN_Center :
		X = appTrunc((RenderBounds[UIFACE_Left] + RenderBounds[UIFACE_Right] - XL)/2) ;
		break;

	case AVAUIALIGN_RightOrBottom :
	default :
		X = RenderBounds[UIFACE_Right] - XL;
		break;
	}

	switch (VerticalAlign)
	{
	case AVAUIALIGN_LeftOrTop :
		Y = RenderBounds[UIFACE_Top];
		break;

	case AVAUIALIGN_Center :
		Y = appTrunc((RenderBounds[UIFACE_Top] + RenderBounds[UIFACE_Bottom] - YL)/2) ;
		break;

	case AVAUIALIGN_RightOrBottom :
	default :
		Y = RenderBounds[UIFACE_Bottom] - YL;
		break;
	}
}

void UavaUISimpleText::PostLoad()
{
	Super::PostLoad();

	if (!bMessageShouldBeSaved)
	{
		Message.Empty();
		bDirty = TRUE;
	}
}

void UavaUISimpleText::Render_Widget( FCanvas* Canvas )
{
	Hot_CurrentValue *= appExp( -GDeltaTime/Hot_Decay );

	if (UpdateString())
	{
		bDirty = TRUE;
	}

	if ( bSkipRender )
		return;

	if (GIsEditor && !GetavaPlayerOwner())
	{
		bDirty = TRUE;
	}

	UBOOL bMessageChanged = bDirty && FadeMode > 0;

	// Opacity 값 결정.
	if (FadeMode > 0)
	{
		if (FadeInTime == 0)
		{
			Opacity = FadeMax;
			FadeMode = 0;
		}
		else
		{
			Opacity += GWorld->GetDeltaSeconds() / FadeInTime; 

			if (Opacity > FadeMax)
			{
				FadeMode = 0;
				Opacity = FadeMax;
			}
		}		
	}
	else if (FadeMode < 0)
	{
		if (FadeOutTime == 0)
		{
			Opacity = 0;
			FadeMode = 0;

			Message.Empty();
			bDrawIcon = FALSE;
			Icon	  = NULL;
		}
		else
		{
			Opacity -= GWorld->GetDeltaSeconds() / FadeOutTime; 

			if (Opacity < 0)
			{
				FadeMode = 0;
				Opacity = 0;

				Message.Empty();
				bDrawIcon = FALSE;
				Icon	  = NULL;
			}
		}		
	}

	if ( bDirty )
	{
		DrawXL = 0.f;
		DrawYL = 0.f;

		FRenderParameters Parameters;
		Parameters.DrawFont = Font;
		UUIString::StringSize( Parameters, *Message );
		TextXL = appTrunc( Parameters.DrawXL );
		TextYL = appTrunc( Parameters.DrawYL );

		if( bMessageChanged )
		{
			TArray<UUIEvent*> EventsToActivate;
			ActivateEventByClass( GetBestPlayerIndex(), UavaUIEvent_SimpleTextChanged::StaticClass(), this, FALSE, NULL, &EventsToActivate);
			for( INT EventIndex = 0 ; EventIndex < EventsToActivate.Num() ; EventIndex++ )
			{
				TArray<FString*> StringVars;
				EventsToActivate(EventIndex)->GetStringVars( StringVars, TEXT("Text") );
				for( INT StrVarIndex = 0 ; StrVarIndex < StringVars.Num() ; StrVarIndex++ )
					*StringVars(StrVarIndex) = Message;
			}
		}

		bDirty = FALSE;
	}

	/// 메세지가 없는 경우
	if ( TextXL == 0.f || TextYL == 0.f) 
		return;

	FLOAT IconXL = IconCoordinates.UL;
	FLOAT IconYL = IconCoordinates.VL;
	FLOAT BackXL = 0.f;
	FLOAT BackYL = 0.f;
	FLOAT TextXL = this->TextXL;
	FLOAT TextYL = this->TextYL;


	if( bDrawIndep )
	{
		switch( IconPosition )
		{
		case UIFACE_Left:
		case UIFACE_Right:
			DrawXL = BackXL = TextXL + IconSpacing + IconXL;
			DrawYL = BackYL = Max(TextYL , IconYL);
			break;
		case UIFACE_Top:
		case UIFACE_Bottom:
			DrawXL = BackXL = Max(TextXL, IconXL);
			DrawYL = BackYL = TextYL + IconSpacing + IconYL;
			break;
		default:
			checkf(FALSE, TEXT("Invalid IconPosition %s @ UIKillMessage::Render_Widget"), IconPosition);	break;
		}

		// 아이콘은 확대 축소가 되지만 배경은 되지 않는다.
		// 그러므로 확대 축소를 반영하지 않은 상태로 DrawXL, DrawYL 계산
		FLOAT X = 0.f, Y = 0.f;
		Render_Background( Canvas, DrawXL, DrawYL, X, Y );
		FLOAT IconWidth = IconXL * InterpIconScaler.X;
		FLOAT IconHeight = IconYL * InterpIconScaler.Y;
		FLOAT TextWidth = TextXL * InterpTextScaler.X;
		FLOAT TextHeight = TextYL * InterpTextScaler.Y;
		FLOAT FirstX = X, SecondX = X;
		FLOAT FirstY = Y, SecondY = Y;

		switch( IconPosition )
		{
		case UIFACE_Left:
		case UIFACE_Top:
			if( InterpIconScaler.X != 1.f ) 
				FirstX -= ( (InterpIconScaleAxis.X) * ( IconWidth - IconCoordinates.UL) );
			if( InterpIconScaler.Y != 1.f )
				FirstY -= ( (InterpIconScaleAxis.Y) * ( IconHeight - IconCoordinates.VL) );
			if( InterpTextScaler.X != 1.f )
				SecondX -= ( (InterpTextScaleAxis.X) * ( TextWidth - this->TextXL));
			if( InterpTextScaler.Y != 1.f )
				SecondY -= ( (InterpTextScaleAxis.Y) * ( TextHeight - this->TextYL));
			break;
		case UIFACE_Right:
		case UIFACE_Bottom:
			if( InterpTextScaler.X != 1.f )
				FirstX -= ( (InterpTextScaleAxis.X) * ( TextWidth - this->TextXL));
			if( InterpTextScaler.Y != 1.f )
				FirstY -= ( (InterpTextScaleAxis.Y) * ( TextHeight - this->TextYL));
			if( InterpIconScaler.X != 1.f ) 
				SecondX -= ( (InterpIconScaleAxis.X) * ( IconWidth - IconCoordinates.UL) );
			if( InterpIconScaler.Y != 1.f )
				SecondY -= ( (InterpIconScaleAxis.Y) * ( IconHeight - IconCoordinates.VL) );
			break;
		}

		switch( IconPosition )
		{
		case UIFACE_Left:
			DrawXL = IconXL;	DrawYL = IconYL;
			Render_Icon( Canvas, FirstX, FirstY + appTrunc( (BackYL - IconYL) * 0.5f ), IconWidth, IconHeight );
			DrawXL = TextXL;	DrawYL = TextYL;
			Render_Text( Canvas, SecondX + IconSpacing + IconXL, SecondY + appTrunc( (BackYL - TextYL) * 0.5f ), InterpTextScaler.X, InterpTextScaler.Y );
			break;
		case UIFACE_Top:
			DrawXL = IconXL;	DrawYL = IconYL;
			Render_Icon( Canvas, FirstX + appTrunc((BackXL - IconXL) * 0.5f) , FirstY, IconWidth, IconHeight );
			DrawXL = TextXL;	DrawYL = TextYL;
			Render_Text( Canvas, SecondX + appTrunc((BackXL - TextXL) * 0.5f) , SecondY + IconSpacing + IconYL, InterpTextScaler.X, InterpTextScaler.Y );
			break;
		case UIFACE_Right:
			DrawXL = TextXL;	DrawYL = TextYL;
			Render_Text( Canvas, FirstX, FirstY + appTrunc( (BackYL - TextYL ) * 0.5f ), InterpTextScaler.X, InterpTextScaler.Y );
			DrawXL = IconXL;	DrawYL = IconYL;
			Render_Icon( Canvas, SecondX + IconSpacing + TextWidth, SecondY + appTrunc( (BackYL - IconYL) * 0.5f ), IconWidth, IconHeight);
			break;
		case UIFACE_Bottom:
			DrawXL = TextXL;	DrawYL = TextYL;
			Render_Text( Canvas, FirstX + appTrunc( (BackXL - TextXL) * 0.5f ), FirstY, InterpTextScaler.X, InterpTextScaler.Y);
			DrawXL = IconXL;	DrawYL = IconYL;
			Render_Icon( Canvas, SecondX + appTrunc((BackXL - IconXL) * 0.5f) , SecondY + IconSpacing + TextHeight, IconWidth, IconHeight);
			break;
		default:
			check(FALSE);
			break;
		}
	}
	else
	{
		// 아이콘과 글씨에는 확대 축소를 반영한다.
		IconXL *= InterpIconScaler.X;
		IconYL *= InterpIconScaler.Y;
		TextXL *= InterpTextScaler.X;
		TextYL *= InterpTextScaler.Y;

		switch( IconPosition )
		{
		case UIFACE_Left:
		case UIFACE_Right:
			DrawXL = BackXL = TextXL + IconSpacing + IconXL;
			DrawYL = BackYL = Max(TextYL , IconYL);
			break;
		case UIFACE_Top:
		case UIFACE_Bottom:
			DrawXL = BackXL = Max(TextXL, IconXL);
			DrawYL = BackYL = TextYL + IconSpacing + IconYL;
			break;
		default:
			checkf(FALSE, TEXT("Invalid IconPosition %s @ UIKillMessage::Render_Widget"), IconPosition);	break;
		}

		// 아이콘은 확대 축소가 되지만 배경은 되지 않는다.
		// 그러므로 확대 축소를 반영하지 않은 상태로 DrawXL, DrawYL 계산
		FLOAT X = 0.f, Y = 0.f;
		Render_Background( Canvas, DrawXL, DrawYL, X, Y );

		switch( IconPosition )
		{
		case UIFACE_Left:
		case UIFACE_Top:
			if( InterpIconScaler.X != 1.f ) 
				X -= ( (InterpIconScaleAxis.X - 0.5f) * (IconXL - IconCoordinates.UL) );
			// 기준을 0.0f ~ 1.0f 사이로 만들어주기위해 0.5f를 뺌
			// 0.0일때 Left/Top, 1.0f일때 Right/Bottom
			if( InterpIconScaler.Y != 1.f )
				Y -= ( (InterpIconScaleAxis.Y - 0.5f) * (IconYL - IconCoordinates.VL) );
			break;
		case UIFACE_Right:
		case UIFACE_Bottom:
			if( InterpTextScaler.X != 1.f )
				X -= ( (InterpTextScaleAxis.X - 0.5f) * (TextXL - this->TextXL));
			if( InterpTextScaler.Y != 1.f )
				Y -= ( (InterpTextScaleAxis.Y - 0.5f) * (TextYL - this->TextYL));
			break;
		}

		switch( IconPosition )
		{
		case UIFACE_Left:
			DrawXL = IconXL;	DrawYL = IconYL;
			Render_Icon( Canvas, X, Y + appTrunc( (BackYL - IconYL) * 0.5f ), IconXL, IconYL );
			DrawXL = TextXL;	DrawYL = TextYL;
			Render_Text( Canvas, X + IconSpacing + IconXL, Y + appTrunc( (BackYL - TextYL) * 0.5f ), InterpTextScaler.X, InterpTextScaler.Y );
			break;
		case UIFACE_Top:
			DrawXL = IconXL;	DrawYL = IconYL;
			Render_Icon( Canvas, X + appTrunc((BackXL - IconXL) * 0.5f) , Y, IconXL, IconYL );
			DrawXL = TextXL;	DrawYL = TextYL;
			Render_Text( Canvas, X + appTrunc((BackXL - TextXL) * 0.5f) , Y + IconSpacing + IconYL, InterpTextScaler.X, InterpTextScaler.Y );
			break;
		case UIFACE_Right:
			DrawXL = TextXL;	DrawYL = TextYL;
			Render_Text( Canvas, X, Y + appTrunc( (BackYL - TextYL ) * 0.5f ), InterpTextScaler.X, InterpTextScaler.Y );
			DrawXL = IconXL;	DrawYL = IconYL;
			Render_Icon( Canvas, X + IconSpacing + TextXL, Y + appTrunc( (BackYL - IconYL) * 0.5f ), IconXL, IconYL);
			break;
		case UIFACE_Bottom:
			DrawXL = TextXL;	DrawYL = TextYL;
			Render_Text( Canvas, X + appTrunc( (BackXL - TextXL) * 0.5f ), Y, InterpTextScaler.X, InterpTextScaler.Y);
			DrawXL = IconXL;	DrawYL = IconYL;
			Render_Icon( Canvas, X + appTrunc((BackXL - IconXL) * 0.5f) , Y + IconSpacing + TextYL, IconXL, IconYL);
			break;
		default:
			check(FALSE);
			break;
		}
	}

	DrawXL = BackXL;
	DrawYL = BackYL;
}

UBOOL UavaUISimpleText::Render_Icon( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL )
{
	if (bDrawIcon && Icon != NULL)
	{
		FLinearColor LC( CalcWarmColor( IconColor ) );
		LC.A *= (Opacity * InterpIconOpacity);

		DrawTile( Canvas, Icon, X + InterpIconOffset.X, Y + InterpIconOffset.Y, XL, YL, IconCoordinates.U, IconCoordinates.V, IconCoordinates.UL, IconCoordinates.VL, LC );

		return TRUE;
	}
	else
		return FALSE;
}

void UavaUISimpleText::Render_Text( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT ScaleX, FLOAT ScaleY )
{
	FLinearColor LC( CalcWarmColor( DrawColor ) );
	LC.A *= ( Opacity * InterpTextOpacity);

	if (bDropShadow)
	{
		FLinearColor ShadowColor( CalcWarmColor( AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor ) );
		ShadowColor.A = LC.A;

		::DrawString( Canvas, X + InterpTextOffset.X + 1, Y + InterpTextOffset.Y + 1, *Message, Font, ShadowColor, ScaleX, ScaleY );
	}
	::DrawString( Canvas, X + InterpTextOffset.X, Y + InterpTextOffset.Y, *Message, Font, LC, ScaleX, ScaleY );
}

void UavaUISimpleText::Render_Background( FCanvas* Canvas, FLOAT DrawXL, FLOAT DrawYL, FLOAT& X, FLOAT& Y )
{
	if (bDrawBackground && Background != NULL)
	{
		FLOAT CornerSizeX = LeftTopRounding.UL;
		FLOAT CornerSizeY = LeftTopRounding.VL;						

		FLOAT InnerXL = DrawXL + Padding[0] + Padding[2], InnerYL = DrawYL + Padding[1] + Padding[3];		

		FLOAT AdditionalPadding[2];

		FLOAT NewInnerXL = Max( InnerXL, MinSize[0] - CornerSizeX * 2 );
		FLOAT NewInnerYL = Max( InnerYL, MinSize[1] - CornerSizeY * 2 );

		AdditionalPadding[0] = - NewInnerXL + InnerXL;
		AdditionalPadding[1] = - NewInnerYL + InnerYL;

		InnerXL = NewInnerXL;
		InnerYL = NewInnerYL;

		switch (HorizontalAlign)
		{
		case AVAUIALIGN_Center :
			AdditionalPadding[0] = appTrunc( AdditionalPadding[0] / 2 );
			break;

		case AVAUIALIGN_RightOrBottom :
			AdditionalPadding[0] = 0;
			break;
		}

		switch (VerticalAlign)
		{
		case AVAUIALIGN_Center :
			AdditionalPadding[1] = appTrunc( AdditionalPadding[1] / 2 );
			break;

		case AVAUIALIGN_RightOrBottom :
			AdditionalPadding[1] = 0;
			break;
		}

		FLOAT BackXL = CornerSizeX * 2 + InnerXL, BackYL = CornerSizeY * 2 + InnerYL;		

		FLOAT BorderSizeX = Min( TopBorder.UL, BackXL - CornerSizeX * 2 );
		FLOAT BorderSizeY = Min( LeftBorder.VL, BackYL - CornerSizeY * 2 );
		FLOAT FillerX = Min( inner.UL, BackXL - CornerSizeX * 2 );
		FLOAT FillerY = Min( inner.VL, BackYL - CornerSizeY * 2 );

		CalculatePosition( X, Y, BackXL, BackYL );			

		FLinearColor LC( BackgroundColor );
		LC.A *= Opacity;

		// Left-Top
		DrawTile( Canvas, Background, X, Y, CornerSizeX, CornerSizeY, LeftTopRounding.U, LeftTopRounding.V, LeftTopRounding.UL, LeftTopRounding.VL, LC );

		/// Draw Rounding
		if ( bVSymmetry == true )
		{
			// Left-Bottom
			DrawTile( Canvas, Background, X, Y + BackYL - CornerSizeY, CornerSizeX, CornerSizeY, LeftTopRounding.U, LeftTopRounding.V + LeftTopRounding.VL, LeftTopRounding.UL, -LeftTopRounding.VL, LC );
		}
		else
		{
			// Left-Bottom
			DrawTile( Canvas, Background, X, Y + BackYL - CornerSizeY, CornerSizeX, CornerSizeY, LeftBottomRounding.U, LeftBottomRounding.V, LeftBottomRounding.UL, LeftBottomRounding.VL, LC );
		}

		if ( bHSymmetry == true )
		{
			// Right-Top
			DrawTile( Canvas, Background, X + BackXL - CornerSizeX, Y, CornerSizeX, CornerSizeY, LeftTopRounding.U + LeftTopRounding.UL, LeftTopRounding.V, -LeftTopRounding.UL, LeftTopRounding.VL, LC );
		}
		else
		{
			// Right-Top
			DrawTile( Canvas, Background, X + BackXL - CornerSizeX, Y, CornerSizeX, CornerSizeY, RightTopRounding.U, RightTopRounding.V, RightTopRounding.UL, RightTopRounding.VL, LC );
		}

		if ( bVSymmetry == true && bHSymmetry == true )
		{
			// Right-Bottom
			DrawTile( Canvas, Background, X + BackXL - CornerSizeX, Y + BackYL - CornerSizeY, CornerSizeX, CornerSizeY, LeftTopRounding.U + LeftTopRounding.UL, LeftTopRounding.V + LeftTopRounding.VL, -LeftTopRounding.UL, -LeftTopRounding.VL, LC );
		}
		else
		{
			// Right-Bottom
			DrawTile( Canvas, Background, X + BackXL - CornerSizeX, Y + BackYL - CornerSizeY, CornerSizeX, CornerSizeY, RightBottomRounding.U, RightBottomRounding.V, RightBottomRounding.UL, RightBottomRounding.VL, LC );
		}


		INT Wide = appTrunc( InnerXL / BorderSizeX );
		INT RemainW = InnerXL - Wide * BorderSizeX;		

		/// Horizontal
		FLOAT CX = X + CornerSizeX;
		if (bShouldTileBackground)
		{
			for (INT i=0; i<Wide; ++i)
			{
				DrawTile( Canvas, Background, CX, Y, BorderSizeX, CornerSizeY, TopBorder.U, TopBorder.V, TopBorder.UL, TopBorder.VL, LC );

				if ( bHSymmetry == true )
					DrawTile( Canvas, Background, CX, Y + BackYL - CornerSizeY, BorderSizeX, CornerSizeY, TopBorder.U, TopBorder.V + TopBorder.VL, TopBorder.UL, -TopBorder.VL, LC );
				else
					DrawTile( Canvas, Background, CX, Y + BackYL - CornerSizeY, BorderSizeX, CornerSizeY, BottomBorder.U, BottomBorder.V, BottomBorder.UL, BottomBorder.VL, LC );

				CX += BorderSizeX;
			}

			DrawTile( Canvas, Background, CX, Y, RemainW, CornerSizeY, TopBorder.U, TopBorder.V, RemainW, TopBorder.VL, LC );

			if ( bHSymmetry == true )
				DrawTile( Canvas, Background, CX, Y + BackYL - CornerSizeY, RemainW, CornerSizeY, TopBorder.U, TopBorder.V + TopBorder.VL, RemainW, -TopBorder.VL, LC );
			else
				DrawTile( Canvas, Background, CX, Y + BackYL - CornerSizeY, RemainW, CornerSizeY, BottomBorder.U, BottomBorder.V, RemainW, BottomBorder.VL, LC );
		}
		else
		{
			DrawTile( Canvas, Background, CX, Y, InnerXL, CornerSizeY, TopBorder.U, TopBorder.V, BorderSizeX, TopBorder.VL, LC );

			if ( bHSymmetry == true )
				DrawTile( Canvas, Background, CX, Y + BackYL - CornerSizeY, InnerXL, CornerSizeY, TopBorder.U, TopBorder.V + TopBorder.VL, BorderSizeX, -TopBorder.VL, LC );
			else
				DrawTile( Canvas, Background, CX, Y + BackYL - CornerSizeY, InnerXL, CornerSizeY, BottomBorder.U, BottomBorder.V, BorderSizeX, BottomBorder.VL, LC );
		}		

		/// Vertical
		INT Height = appTrunc( InnerYL / BorderSizeY );
		INT RemainH = InnerYL - Height * BorderSizeY;

		FLOAT CY = Y + CornerSizeY;
		if (bShouldTileBackground)
		{
			for (INT i=0; i<Height; ++i)
			{
				DrawTile( Canvas, Background, X, CY, CornerSizeX, BorderSizeY, LeftBorder.U, LeftBorder.V, LeftBorder.UL, LeftBorder.VL, LC );

				if ( bVSymmetry == true )
					DrawTile( Canvas, Background, X + BackXL - CornerSizeX, CY, CornerSizeX, BorderSizeY, LeftBorder.U + LeftBorder.UL, LeftBorder.V, -LeftBorder.UL, LeftBorder.VL, LC );
				else
					DrawTile( Canvas, Background, X + BackXL - CornerSizeX, CY, CornerSizeX, BorderSizeY, RightBorder.U, RightBorder.V, RightBorder.UL, RightBorder.VL, LC );
				CY += BorderSizeY;
			}

			DrawTile( Canvas, Background, X, CY, CornerSizeX, RemainH, LeftBorder.U, LeftBorder.V, LeftBorder.UL, RemainH, LC );
			if ( bVSymmetry == true )
				DrawTile( Canvas, Background, X + BackXL - CornerSizeX, CY, CornerSizeX, RemainH, LeftBorder.U + LeftBorder.UL, LeftBorder.V, -LeftBorder.UL, RemainH, LC );
			else
				DrawTile( Canvas, Background, X + BackXL - CornerSizeX, CY, CornerSizeX, RemainH, RightBorder.U, RightBorder.V, RightBorder.UL, RemainH, LC );
		}
		else
		{
			DrawTile( Canvas, Background, X, CY, CornerSizeX, InnerYL, LeftBorder.U, LeftBorder.V, LeftBorder.UL, BorderSizeY, LC );
			if ( bVSymmetry == true )
				DrawTile( Canvas, Background, X + BackXL - CornerSizeX, CY, CornerSizeX, InnerYL, LeftBorder.U + LeftBorder.UL, LeftBorder.V, -LeftBorder.UL, BorderSizeY, LC );
			else
				DrawTile( Canvas, Background, X + BackXL - CornerSizeX, CY, CornerSizeX, InnerYL, RightBorder.U, RightBorder.V, RightBorder.UL, BorderSizeY, LC );
		}

		Wide = appTrunc( InnerXL / FillerX );
		Height = appTrunc( InnerYL / FillerY );
		RemainW = InnerXL - Wide * FillerX;
		RemainH = InnerYL - Height * FillerY;

		if (bShouldTileBackground)
		{
			CY = Y + CornerSizeY;
			for (INT j=0; j<Height; ++j)
			{
				CX = X + CornerSizeX;

				for (INT i=0; i<Wide; ++i)
				{
					DrawTile( Canvas, Background, CX, CY, FillerX, FillerY, inner.U, inner.V, inner.UL, inner.VL, LC );
					CX += FillerX;
				}

				DrawTile( Canvas, Background, CX, CY, RemainW, FillerY, inner.U, inner.V, RemainW, inner.VL, LC );
				CY += FillerY;
			}

			CX = X + CornerSizeX;

			for (INT i=0; i<Wide; ++i)
			{
				DrawTile( Canvas, Background, CX, CY, FillerX, RemainH, inner.U, inner.V, inner.UL, RemainH, LC );
				CX += FillerX;
			}

			DrawTile( Canvas, Background, CX, CY, RemainW, RemainH, inner.U, inner.V, RemainW, RemainH, LC );
		}
		else
		{
			DrawTile( Canvas, Background, CX, CY, InnerXL, InnerYL, inner.U, inner.V, FillerX, FillerY, LC );
		}

		X += InnerXL - DrawXL - Padding[2] + CornerSizeX + AdditionalPadding[0];
		Y += InnerYL - DrawYL - Padding[3] + CornerSizeY + AdditionalPadding[1];
	}
	else
	{
		CalculatePosition( X, Y, DrawXL, DrawYL );	
	}	
}

void UavaUIClock::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize( inOwnerScene, inOwner );

	LastValue = -1;
}

void UavaUIClock::DecideColor( FLOAT NewValue, UBOOL bUseAlarm )
{
	// Alarm 시간내에 있고 MissionTime이 감소하는 상황에서만 깜빡임을 준다
	// TimeLimit == 0, ElapsedTime을 사용 -> TimeCountUp
	// TimeLimit != 0, RemainingTime을 사용 -> TimeCountDown 라 가정하였음
	if (NewValue <= AlarmSeconds && bUseAlarm)
	{
		FLOAT CurrentTime = GWorld->GetTimeSeconds();
		if (!bAlarm)
		{
			bAlarm = TRUE;
			AlarmStartTime = CurrentTime;
		}
		FLOAT Elapsed = CurrentTime - AlarmStartTime;

		FLOAT Alpha = appFmod( Elapsed / AlarmBlinkingPeriod, 1.0f );

		Alpha = GMath.SinTab( appTrunc( Alpha * 65536.f) );

		DrawColor = (FLinearColor(AlarmColor) - FLinearColor(NormalColor)) * Alpha + FLinearColor(NormalColor);
	}
	else
	{
		bAlarm = FALSE;
		DrawColor = NormalColor;
	}	
}

UBOOL UavaUIClock::UpdateString()
{
	AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;	

	FLOAT NewValue = 0;

	if (GRI)
	{
		NewValue = ( GRI->TimeLimit != 0 ) ? GRI->RemainingTime : GRI->ElapsedTime;	
	}
	else if (GIsEditor)
	{
		NewValue = 3893.0f;	
	}

	// Alarm 시간내에 있고 MissionTime이 감소하는 상황에서만 깜빡임을 준다
	// TimeLimit == 0, ElapsedTime을 사용 -> TimeCountUp
	// TimeLimit != 0, RemainingTime을 사용 -> TimeCountDown 라 가정하였음
	if (NewValue <= AlarmSeconds && (GRI != NULL && GRI->TimeLimit != 0))
	{
		FLOAT CurrentTime = GWorld->GetTimeSeconds();
		if (!bAlarm)
		{
			bAlarm = TRUE;
			AlarmStartTime = CurrentTime;
		}
		FLOAT Elapsed = CurrentTime - AlarmStartTime;

		FLOAT Alpha = appFmod( Elapsed / AlarmBlinkingPeriod, 1.0f );

		Alpha = GMath.SinTab( appTrunc( Alpha * 65536.f) );

		DrawColor = (FLinearColor(AlarmColor) - FLinearColor(NormalColor)) * Alpha + FLinearColor(NormalColor);
	}
	else
	{
		bAlarm = FALSE;
		DrawColor = NormalColor;
	}

	if (NewValue == LastValue) return FALSE;

	LastValue = NewValue;

	if ( bFormattedTime )
		Message =	GetFormattedTime( LastValue );
	else
		Message =	FString::Printf( TEXT("%d"), INT(LastValue) );

	return TRUE;	
}

void UavaUIKOTH3::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize( inOwnerScene, inOwner );

	LastNumPlayersInside = -1;
}

UBOOL UavaUIKOTH3::UpdateString()
{
	AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;

	if (TeamIndex != 0 && TeamIndex != 1)
	{		
		return FALSE;
	}

	INT NewNumPlayersInside = GRI ? GRI->KOTH3[TeamIndex].NumPlayersInside : 2;
	FLOAT NewTimeRemains = GRI ? GRI->KOTH3[TeamIndex].TimeRemains : 142;

	// Alarm 시간내에 있고 MissionTime이 감소하는 상황에서만 깜빡임을 준다
	// TimeLimit == 0, ElapsedTime을 사용 -> TimeCountUp
	// TimeLimit != 0, RemainingTime을 사용 -> TimeCountDown 라 가정하였음
	if (NewTimeRemains <= AlarmSeconds)
	{
		FLOAT CurrentTime = GWorld->GetTimeSeconds();
		if (!bAlarm)
		{
			bAlarm = TRUE;
			AlarmStartTime = CurrentTime;
		}
		FLOAT Elapsed = CurrentTime - AlarmStartTime;

		FLOAT Alpha = appFmod( Elapsed / AlarmBlinkingPeriod, 1.0f );

		Alpha = GMath.SinTab( appTrunc( Alpha * 65536.f) );

		DrawColor = (FLinearColor(AlarmColor) - FLinearColor(NormalColor)) * Alpha + FLinearColor(NormalColor);
	}
	else
	{
		bAlarm = FALSE;
		DrawColor = NormalColor;
	}	

	if (LastNumPlayersInside != NewNumPlayersInside || NewTimeRemains != LastValue)
	{
		LastValue = NewTimeRemains;

		if ( bFormattedTime )
			Message =	GetFormattedTime( LastValue );
		else
			Message =	FString::Printf( TEXT("%d"), INT(LastValue) );

		Message = FString::Printf( TEXT("%s(%d)"), *Message, NewNumPlayersInside );

		LastNumPlayersInside = NewNumPlayersInside;

		return TRUE;
	}	
	else
	{
		return FALSE;
	}
}

UBOOL UavaUITargetName::UpdateString()
{
	FLOAT CurrentTime = GWorld->GetRealTimeSeconds();
	FLOAT RenderDelta = CurrentTime - LastRenderTime;
	LastRenderTime = CurrentTime; 

	UBOOL Result = FALSE;

	do 
	{
		AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
		AavaPawn* PawnOwner =	GetPlayerViewTarget();
			//PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;
		AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
		APawn* PreviousTarget = TrackedTarget;

		if ( GRI == NULL || PawnOwner == NULL || PlayerOwner == NULL || PlayerOwner->bBehindView )
		{
			/// Editor에서 PIE가 아닌 경우!
			if (GIsEditor)
			{
				Message = TEXT("Enemy : Anti-AVA");
				TrackedTargetAlpha = 1.0f;	
				Result = TRUE;
				break;
			}
			else
			{
				Message.Empty();
			}
			return FALSE;	
		}

		FVector		ViewPos;	
		FRotator	ViewRot;

		PlayerOwner->eventGetPlayerViewPoint( ViewPos, ViewRot );
		//ViewPos = PawnOwner->Location + FVector( 0, 0, PawnOwner->BaseEyeHeight );

		FCheckResult Hit(1.f);
		GWorld->SingleLineCheck(Hit, PawnOwner, ViewPos + 10000 * ViewRot.Vector(),ViewPos, TRACE_ProjTargets|TRACE_Tesselation|TRACE_ComplexCollision );

		AavaPawn* HitActor = Cast<AavaPawn>( Hit.Actor );

		if (HitActor) 	
		{
			FVector					SmokeClosestPt;
			float					SmokeClosestDist;
			float					dist;
			float					alpha;
			float					temp;

			for (FDynamicActorIterator It; It; ++It)
			{		
				AActor* Actor = *It;
				AavaKProj_SmokeBomb* SmokeBomb = Cast<AavaKProj_SmokeBomb>(Actor);

				if (SmokeBomb != NULL)
				{
					// 내가 연막탄 안에 있다면 Enemy Name 을 찍지 않는다.
					if ( (ViewPos-SmokeBomb->Location).Size() < 250.0 )	
					{
						HitActor = NULL;
						break;
					}

					// Enemy 가 연막탄 안에 있다면 Enemy Name 을 찍지 않는다.
					if ( (SmokeBomb->Location-HitActor->Location).Size() < 250.0 ) 
					{
						HitActor = NULL;
						break;
					}

					FVector Direction = (HitActor->Location - ViewPos);
					Direction.Normalize();

					SmokeClosestDist = PointDistToLine( SmokeBomb->Location, Direction, ViewPos, SmokeClosestPt );

					// Smoke 의 원점에서 내린 수선의 길이가 Smoke 의 반지름보다 작다면, 구와 라인은 교차한다.
					if ( SmokeClosestDist < 250.0 )
					{
						temp = (ViewPos-SmokeBomb->Location).Size() - (ViewPos-Hit.Location).Size();
						if ( temp < 250.0 )
						{
							HitActor = NULL;
							break;
						}				
					}			
				}
			}

			if (HitActor != NULL)
			{		
				/// 새로운 녀석이다. alpha를 0으로 set
				if (TrackedTarget != HitActor)
				{
					TrackedTarget = HitActor;
					TrackedTargetAlpha = 0;
				}

				dist = (TrackedTarget->Location - PawnOwner->Location).Size() / ( 3 * 16 );					

				/// 5 미터 내의 적은 바로!
				if (dist < MinDist)
					TrackedTargetAlpha = 1;
				else
				{			
					alpha = 1 - Min( 1.0f, (dist - MinDist) / ( MaxDist - MinDist) );

					/// MinDist일 때는 즉시
					/// MaxDist일 때는 RisingTime 후에 도달

					/// MinDist일 때는 alpha = 1
					/// MaxDist일 때는 alpha = 0

					/// result = alpha + (1-alpha)*DeltaTime*RisingSpeed

					TrackedTargetAlpha += alpha + RenderDelta * (1-alpha) * RisingSpeed; 
				}		

				if (TrackedTargetAlpha > 1)
					TrackedTargetAlpha = 1;
			}
		}

		if (HitActor == NULL)
		{
			/// 1초 만에 fade out!
			TrackedTargetAlpha -= RenderDelta * FallingSpeed;

			if (TrackedTargetAlpha < 0)
			{
				TrackedTargetAlpha = 0;
				TrackedTarget = NULL;
			}
		}					

		UBOOL	bIsInSpawnProtection = IsInSpawnProtection( TrackedTarget );

		if (TrackedTarget != PreviousTarget || bPrevInvincibilityMode != bIsInSpawnProtection )
		{
			bPrevInvincibilityMode = bIsInSpawnProtection;
			if (TrackedTarget != NULL && !GRI->eventOnSameTeam( PawnOwner, TrackedTarget ))
			{
				APlayerReplicationInfo*	TargetPRI = NULL;
				if ( TrackedTarget->PlayerReplicationInfo )
				{
					TargetPRI = TrackedTarget->PlayerReplicationInfo;
				}
				else if ( TrackedTarget->DrivenVehicle != NULL && TrackedTarget->DrivenVehicle->PlayerReplicationInfo != NULL )
				{
					TargetPRI = TrackedTarget->DrivenVehicle->PlayerReplicationInfo;
				}

				if ( TargetPRI != NULL )
				{
					if ( !bIsInSpawnProtection )	Message = FString::Printf( TEXT( "Enemy : %s" ), *(TargetPRI->PlayerName) );
					else							Message = FString::Printf( TEXT( "Enemy : %s %s" ), *(TargetPRI->PlayerName), *AavaPlayerReplicationInfo::StaticClass()->GetDefaultObject<AavaPlayerReplicationInfo>()->InvincibleStr );
				}
				else
				{
					Message = FString::Printf( TEXT( "deadbody" ) );
				}
			}		
			else
			{
				Message.Empty();
			}

			Result = TRUE;						
		}		
	} while (false);

	if (TrackedTargetAlpha > VisibleStart)
	{		
		DrawColor.A = MaxAlpha * (TrackedTargetAlpha - VisibleStart) / (1-VisibleStart);
	}	

	return Result;
}

void UavaUIGameDigits::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize( inOwnerScene, inOwner );

	LastValue = -1;
}

UBOOL UavaUIGameDigits::UpdateString()
{
	AavaPlayerController*	PlayerOwner		= GetavaPlayerOwner();
	AavaPawn*				PawnOwner		= GetPlayerViewTarget();
	AavaWeapon*				 CurrentWeapon	= GetViewTargetCurrentWeapon();
	AavaGameReplicationInfo* GRI			= GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;

	INT NewValue = 0;

	extern INT GAVAVersion;
	extern INT GAVABuiltFromChangeList;

	switch (Binding)
	{
	case AVAUIDIGIT_WinCondition:
		if ( GRI != NULL )
			NewValue = GRI->WinCondition;
		else 
			NewValue = 0;
		break;
	case AVAUIDIGIT_TeamScore:
		if ( GRI != NULL && nTeam >= 0 && nTeam <= 1 && GRI->Teams[nTeam] != NULL )
			NewValue = GRI->Teams[nTeam]->Score;
		else 
			NewValue = 0;
		break;
	case AVAUIDIGIT_TeamSymbolName:
		if ( GRI != NULL && nTeam >= 0 && nTeam <= 1 && GRI->Teams[nTeam] != NULL )
		{
			const AavaTeamInfo* avaTeamInfo = Cast<AavaTeamInfo>(GRI->Teams[nTeam]);
			if ( avaTeamInfo != NULL )
				Message = avaTeamInfo->TeamSymbolName;
			return TRUE;
		}
		break;
	case AVAUIDIGIT_TeamPlayerCnt:
		{
			if ( GRI != NULL && nTeam >= 0 && nTeam <= 1 )
			{
				for ( INT i = 0 ; i < GRI->PRIArray.Num() ; ++ i )
				{
					const AavaPlayerReplicationInfo* PRI = Cast<AavaPlayerReplicationInfo>(GRI->PRIArray(i));
					if ( PRI != NULL && PRI->Team != NULL && PRI->bBot == FALSE )
					{
						if ( PRI->Team->TeamIndex == nTeam )
							++NewValue;
					}
				}
			}
			else
			{
				NewValue = 0;
			}
		}
		break;
	case AVAUIDIGIT_ReInforcementTime:
		{
			//nTeam
			if ( GRI != NULL )
			{
				// 증원이 안딘다...
				if ( GRI->bReinforcement == FALSE )
					return FALSE;

				INT nSelectedTeam = 0;
				INT nPlayerTeam   = 0;
				if ( PlayerOwner->PlayerReplicationInfo != NULL && 
					 PlayerOwner->PlayerReplicationInfo->Team != NULL )
				{
					nSelectedTeam	=	PlayerOwner->PlayerReplicationInfo->Team->TeamIndex;
					nPlayerTeam		=	nSelectedTeam;
				}
				else
				{
					nSelectedTeam	=	0;
					nPlayerTeam		=	2;
				}
				
				if ( GRI->ReinforcementFreq[nSelectedTeam] > 0 )
				{
					if ( nPlayerTeam != 2 )
						NewValue = GRI->ReinforcementFreq[nSelectedTeam] - ( GRI->ElapsedTime % GRI->ReinforcementFreq[nSelectedTeam] );
					else
					{
						const AavaPlayerReplicationInfo* avaPRI = Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo);
						NewValue = GRI->ReinforcementFreq[nSelectedTeam] - ( ( GRI->ElapsedTime - avaPRI->LastDeathTime ) % GRI->ReinforcementFreq[nSelectedTeam] );
					}
						
					if ( NewValue == GRI->ReinforcementFreq[nSelectedTeam] )
					{
						Message = FString::Printf( TEXT("") );
						return TRUE;
					}
				}
			}
			else
			{
				NewValue = 0;
			}
		}
		break;
	case AVAUIDIGIT_Health :
		if (PawnOwner)
			NewValue = Max( 0, PawnOwner->Health );
		else if (PlayerOwner)
		{
			if (Message.Len() > 0)
			{
				Message.Empty();
				return TRUE;
			}
		}
		else
			NewValue = 70;
		break;

	case AVAUIDIGIT_AmmoCount :
	case AVAUIDIGIT_ReloadCount :
		do 
		{
			if ( CurrentWeapon )
			{
				if ( CurrentWeapon->bDisplayAmmoCount == FALSE )	
				{
					LastValue = -1;
					Message	= TEXT("");
					return TRUE;
				}

				AavaWeap_BaseGun* Gun = Cast<AavaWeap_BaseGun>(CurrentWeapon);

				if (Gun)
				{
					if ( Binding == AVAUIDIGIT_AmmoCount )
					{
						if ( Gun->bInfinityAmmo == false)
							NewValue = (Gun->AmmoCount - Gun->ReloadCnt);
						else
						{
							LastValue = -1;
							if (Message.Len() == 0)
							{
								return FALSE;
							}

							Message.Empty();
							return TRUE;
						}
					}
					else
					{
						NewValue =	Gun->ReloadCnt;
					}
					break;
				}						
				else
				{
					AavaThrowableWeapon* Throwable = Cast<AavaThrowableWeapon>(CurrentWeapon);

					if (Throwable)
					{
						if (Binding == AVAUIDIGIT_ReloadCount)
						{
							LastValue = -1;
							if (Message.Len() == 0)
							{
								return FALSE;
							}

							Message.Empty();

							return TRUE;
						}

						NewValue = CurrentWeapon->AmmoCount;
						break;
					}
					else
					{
						// Gun 도 아니고 ThrowableWeapon 도 아니면....Knife!
						LastValue = -1;
						if (Message.Len() == 0)
						{
							return FALSE;
						}
						Message.Empty();
						return TRUE;
					}
				}
			}


			if ( GIsEditor )
			{
				NewValue = Binding == AVAUIDIGIT_AmmoCount ? 150 : 25;
				return TRUE;
			}

			LastValue = -1;
			if (Message.Len() == 0)
			{
				return FALSE;
			}
			Message.Empty();
			return TRUE;

		} while (false);			
		break;
	case AVAUIDIGIT_ArmorPercentage:
		NewValue = PawnOwner ? ((FLOAT)PawnOwner->Armor_Stomach/ PawnOwner->ArmorMax) * 100.0f : 0;
		break;
	case AVAUIDIGIT_ArmorHealth:
		NewValue = PawnOwner ? PawnOwner->Armor_Stomach : 0;
		break;
	case AVAUIDIGIT_Fixed:
		NewValue = DigitFixed;
		break;
	case AVAUIDIGIT_RemainingTeamScoreToGo :	
		if ( GRI != NULL && nTeam >= 0 && nTeam <= 1 && GRI->Teams[nTeam] != NULL )
			NewValue = GRI->WinCondition - GRI->Teams[nTeam]->Score;
		else 
			NewValue = 0;
		break;	
	case AVAUIDIGIT_DogTagCnt:
		NewValue = PawnOwner ? PawnOwner->DogTagCnt : 0 ;
		break;
	case AVAUIDIGIT_DogTagPackCnt:
		if ( GRI != NULL )
			NewValue = GRI->DogTagPackCnt;
		else
			NewValue = 0;
		break;
	case AVAUIDIGIT_AVAVersion:
		NewValue = GAVAVersion;
		break;
	case AVAUIDIGIT_AVABuiltFromChangelistNum:
		NewValue = GAVABuiltFromChangeList;
		break;
	case AVAUIDIGIT_WeaponMaintenanceRate:
		{
			AavaWeapon* Weapon = GetViewTargetCurrentWeapon();
			if ( Weapon != NULL )
			{
				if ( Weapon->MaintenanceRate >= 0 )
				{
					Message = FString::Printf( TEXT("%d %%"), int( Weapon->MaintenanceRate ) );
					bSkipRender = false;
				}
				else
					bSkipRender = true;
			}
			return TRUE;
		}
	default :
		break;
	}	

	if (NewValue == LastValue) return FALSE;

	LastValue = NewValue;
	Hot_CurrentValue = 2.0f;

	Message = FString::Printf( TEXT("%d"), NewValue );

	//  [10/31/2006 otterrrr], bLinearColor 플래그가 켜져있을 때 GameDigitColor의 중간값을 선택할 수 있도록함.
	//
	//	예) GameDigitColor(0)=(Value=0, Color=(R=255,G=0,B=0,A=255))),
	//		GameDigitColor(1)=(Value=100, Color(R=0,G=255,B=0,A255))) 에서
	//
	//		bLinearColor == true 이고 NewValue = 50 이면
	//		출력되는 색깔 DrawColor = (R=125,G=125,B=0,A=255) 이다.	
	if( bLinearColor && GameDigitColor.Num() >= 2 )
	{
		INT CeilIndex = -1, FloorIndex = -1;

		for( INT i = 0 ; i < GameDigitColor.Num() ; i++ )
		{
			INT Value = GameDigitColor(i).Value;
			INT Diff = Value - NewValue;

			if( Diff > 0 && ! ( CeilIndex >= 0 && Value < GameDigitColor(CeilIndex).Value ))
				CeilIndex = i;
			if( Diff <= 0 &&  ! ( FloorIndex >= 0 && Value > GameDigitColor(FloorIndex).Value ))
				FloorIndex = i;
		}

		if( CeilIndex < 0 && FloorIndex < 0 )
		{
			warnf(TEXT("Inappropriate UpdateString at %s"), *this->GetName());
			return FALSE;
		}
		if( CeilIndex < 0)
			CeilIndex = FloorIndex;
		if( FloorIndex < 0)
			FloorIndex = CeilIndex;

		FLinearColor NewColor = ( ( FLinearColor(GameDigitColor(CeilIndex).Color) + FLinearColor(GameDigitColor(FloorIndex).Color) ) /2 );
		DrawColor = NewColor;
	}

	return TRUE;
}

INT UavaUIUseInfo::UpdateInfo()
{
	//if (UpdateUseVolume())
	//{
	//	if (UseVolume)
	//	{
	//		Message = UseVolume->UseMessage;

	//		SetIcon( UseVolume->IconCode );			

	//		return 1;
	//	}
	//	else
	//	{
	//		return -1;
	//	}		
	//}	
	//else
	//{
	//	if ((GIsGame || GIsEditor && GetavaPlayerOwner()) && UseVolume == NULL && (Message.Len() > 0 || bDrawIcon))
	//	{
	//		Message.Empty();

	//		bDrawIcon = FALSE;
	//		Icon	  = NULL;

	//		return -1;
	//	}
	//}

	return 0;
}

UBOOL UavaUIInfoMessage::UpdateString()
{
	INT Result = UpdateInfo();
	if (Result > 0)
	{
		/// FadeIn!
		FadeMode = 1;

		return TRUE;
	}
	else if (Result < 0)
	{
		/// FadeOut!
		FadeMode = -1;		

		return TRUE;
	}	

	/// UI editor :)
	if (GIsEditor && !GetavaPlayerOwner())
	{
		FadeMode = 1;		

		Message = Editor_String;

		SetIcon( Editor_IconCode );								

		return TRUE;
	}		

	return FALSE;
}

void UavaUIInfoMessage::SetIcon( INT IconCode )
{
	for (INT i=0; i<Icons.Num(); ++i)
	{
		if (Icons(i).Code == IconCode)
		{
			bDrawIcon = TRUE;
			Icon = Icons(i).Image;
			IconCoordinates = Icons(i).Coordinates;
			IconColor = Icons(i).IconColor;

			return;
		}
	}

	bDrawIcon = FALSE;
	Icon	  = NULL;
}

UBOOL UavaUIUseInfo::UpdateUseVolume()
{
	//AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	//AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;

	//if (PawnOwner == NULL) 
	//{
	//	if (UseVolume == NULL)
	//		return FALSE;

	//	UseVolume = NULL;
	//	return TRUE;
	//}

	//UClass* BaseClass = AavaUseVolume::StaticClass();
	//FMemMark Mark(GMem);
	//FCheckResult* Link = GWorld->Hash->ActorOverlapCheck(GMem, PawnOwner, PawnOwner->Location, PawnOwner->eventGetCollisionRadius());

	//AavaUseVolume* useBest = NULL;
	//FVector PawnLoc, VLoc;
	//FLOAT NewDist, BestDist = 0.0f;	

	//PawnLoc = PawnOwner->Location;
	//if ( Link )
	//{
	//	while ( Link )
	//	{
	//		if( !Link->Actor
	//			|| Link->Actor->bDeleteMe
	//			|| !Link->Actor->IsA(BaseClass))
	//		{
	//			Link = Link->GetNext();
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}

	//	if ( Link )
	//	{
	//		AavaUseVolume* useO = Cast<AavaUseVolume>( Link->Actor );

	//		if (useO)
	//		{
	//			// Pawn must be facing objective
	//			VLoc	=	useO->Location;
	//			NewDist	=	(VLoc-PawnLoc).Size();
	//			if (useO->eventIsUseable( PawnOwner ))
	//			{
	//				if ( (useBest == NULL ) || (NewDist < BestDist) )
	//				{
	//					useBest		= useO;
	//					BestDist	= NewDist;
	//				}
	//			}
	//		}


	//		Link=Link->GetNext();
	//	}
	//}

	//Mark.Pop();

	//if (UseVolume != useBest)
	//{
	//	UseVolume = useBest;
	//	return TRUE;
	//}

	return FALSE;
}

INT UavaUIGameInfoMessage::UpdateInfo()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if (HUD)
	{
		if ( nType >= 0 && nType <= 2 )	// Type 은 두가지로 고정.... avaHUD.uc 에서 늘려주면 같이 제약을 풀도록 하자....
		{
			if (MessageFetchedTime != HUD->GameInfo[nType].GameInfoMessageSetTime && HUD->GameInfo[nType].GameInfoMessageSetTime > 0 )
			{
				MessageFetchedTime = HUD->GameInfo[nType].GameInfoMessageSetTime;
				HUD->GameInfo[nType].GameInfoMessageSetTime = 0;
				Message = HUD->GameInfo[nType].GameInfoMessageTxt;
				SetIcon( HUD->GameInfo[nType].GameInfoMessageCode );
				return 1;
			}
			else if ( HUD->GameInfo[nType].GameInfoMessageTime != 0 && GWorld->GetTimeSeconds() > HUD->GameInfo[nType].GameInfoMessageTime )
			{
				return -1;
			}
		}

		if ( Message == _T("") )
		{
			SetIcon( -1 );
			return 0;
		}
	}	

	return 0;
}

void UavaUIAmmoGraph::Render_Widget( FCanvas* Canvas )
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;		

	static FName WeaponReloading( TEXT("WeaponReloading") );

	UBOOL NewReload = FALSE;
	INT MaxAmmo = 0;

	INT OldValue = Value;

	do 
	{
		if (PawnOwner)
		{
			AavaWeapon* Weapon = Cast<AavaWeapon>( PawnOwner->CurrentWeapon );

			if (Weapon)
			{
				AavaWeap_BaseGun* Gun = Cast<AavaWeap_BaseGun>(Weapon);

				if (Gun)
				{
					Value = /*Gun->GetClass()->GetDefaultObject<AavaWeap_BaseGun>()->ReloadCnt - */Gun->ReloadCnt;
					MaxAmmo = Gun->GetClass()->GetDefaultObject<AavaWeap_BaseGun>()->ReloadCnt;

					FStateFrame* StateFrame = Gun->GetStateFrame();

					if (StateFrame && StateFrame->StateNode)
					{
						if (StateFrame->StateNode->GetFName() == WeaponReloading)
						{
							NewReload = TRUE;
						}
					}
					break;
				}						
				else
				{
					Value = -1;
					break;
				}
			}
		}

		if ( GIsEditor )
		{
			Value = TestValue;
			MaxAmmo = TestMaxAmmo;
		}
		else
		{
			Value = -1;
		}

	} while (false);		

	if (bIsReloading ^ NewReload)
	{
		bIsReloading = NewReload;

		TArray<INT> OutputLinkIndexes;
		OutputLinkIndexes.AddItem(NewReload);

		ActivateEventByClass( INDEX_NONE, UUIEvent_AmmoGraph::StaticClass(), this, FALSE, &OutputLinkIndexes );
	}

	if (Image != NULL)
	{	
		if (Value < 0 || Scroll >= 1.0f) return;		

		if (FadeTime > 0)
		{
			for (INT i=0; i<Alphas.Num();)
			{				
				Alphas(i) = Alphas(i) - GWorld->GetDeltaSeconds() / FadeTime;

				if (Alphas(i) < 0)
				{
					Alphas.Remove(i);
				}
				else
				{
					++i;
				}
			}

			while (Value < OldValue)
			{
				Alphas.AddItem( 1.0f );
				OldValue--;
			}
		}
		else
		{
			Alphas.Empty();
		}		


		INT N = (((MaxAmmo+2) / 2) / AmmoSize) * AmmoSize;

		FLOAT Y = Max( RenderBounds[UIFACE_Top], RenderBounds[UIFACE_Bottom] - N * AmmoSize );
		FLOAT X  = RenderBounds[UIFACE_Left];		
		Y  = Y + Clamp( Scroll, 0.0f, 1.0f ) * (RenderBounds[UIFACE_Bottom] - Y);

		Render_Background( Canvas, Y );

		INT Left = Value / 2, Right = (Value+1) / 2;
		INT LeftBlank = MaxAmmo / 2 - Left, RightBlank = (MaxAmmo+1) / 2 - Right;
		X += Padding[0];
		Y += Padding[1];

		Render_Column( Canvas, X, Y + SpacingY, Left, LeftBlank, 0, (Value & 1) ? 0 : 1 );
		Render_Column( Canvas, X + SpacingX, Y, Right, RightBlank, 1, (Value & 1) ? 1 : 0 );		
	}
}

void UavaUIAmmoGraph::Render_Background( FCanvas* Canvas, FLOAT Y )
{
	FLOAT YL = RenderBounds[UIFACE_Bottom] - Y;

	if (YL <= 0 || BackgroundImage == NULL) return;

	FLOAT X  = RenderBounds[UIFACE_Left];		
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];		

	FLinearColor LC( BackgroundImageColor );
	LC.A *= Opacity;

	FLOAT U = BackgroundImageCoordinates.U;
	FLOAT V = BackgroundImageCoordinates.V;
	FLOAT UL = BackgroundImageCoordinates.UL;
	FLOAT VL = BackgroundImageCoordinates.VL;

	if (BackgroundImageTop > 0)
	{
		FLOAT _YL = Min( YL, BackgroundImageTop );
		DrawTile( Canvas, BackgroundImage, X, Y, XL, _YL, U, V, UL, _YL, LC );

		Y += _YL;
		YL -= _YL;

		V += BackgroundImageTop;
		VL -= BackgroundImageTop;	
	}	

	if (bShouldTileBackground)
	{
		while (YL > VL)
		{
			DrawTile( Canvas, BackgroundImage, X, Y, XL, VL, U, V, UL, VL, LC );

			Y += VL;
			YL -= VL;
		}

		DrawTile( Canvas, BackgroundImage, X, Y, XL, YL, U, V, UL, YL, LC );
	}
	else
	{
		DrawTile( Canvas, BackgroundImage, X, Y, XL, YL, U, V, UL, VL, LC );
	}
}

void UavaUIAmmoGraph::Render_Column( FCanvas* Canvas, FLOAT X, FLOAT Y, INT N, INT Blank, INT Index, INT AlphaStart )
{
	FLOAT YL = Min( RenderBounds[UIFACE_Bottom] - Y, AmmoSize * N );
	Render_Ammo( Canvas, 1.0f, X, Y, YL );
	Y += YL;

	for (INT i=AlphaStart; i<Alphas.Num(); i+=2)
	{
		FLOAT Y1 = Y + AmmoSize;
		UBOOL bHitEnd = Y1 > RenderBounds[UIFACE_Bottom];

		FLOAT _YL;
		if (bHitEnd)
		{
			_YL = RenderBounds[UIFACE_Bottom] - Y;
		}
		else
		{
			_YL = AmmoSize;
		}

		Render_Ammo( Canvas, Alphas(i), X, Y, _YL );		

		/// 더 이상 뭘 그려~
		if (bHitEnd)
			return;

		Y += AmmoSize;

		Blank--;
	}

	if (Blank <= 0) return;

	YL = AmmoSize * Blank;

	Render_Ammo( Canvas, 0.0f, X, Y, Min( YL, RenderBounds[UIFACE_Bottom] - Y ) );
}

void UavaUIAmmoGraph::Render_Ammo( FCanvas* Canvas, FLOAT Alpha, FLOAT X, FLOAT Y, FLOAT YL )
{
	FLOAT BatchYL = AmmoSize * AmmoInImage;

	FLinearColor CL_Blank( BlankColor ), CL_Full( FullColor );	
	CL_Blank.A *= Opacity * (1-Alpha);
	CL_Full.A *= Opacity * Alpha;

	float U = BlankU, V = BlankV;

	for (INT i=0; i<2; ++i)
	{
		const FLinearColor& CL = i == 0 ? CL_Blank : CL_Full;

		if (CL.A > 0)
		{
			INT N = 0;
			while (YL > BatchYL)
			{
				/// 너무 많으면 안되삼... -_-;
				if (N++ > 10)
					break;

				DrawTile( Canvas, Image, X, Y, AmmoSize, BatchYL, U, V, AmmoSize, BatchYL, CL );

				YL -= BatchYL;
				Y += BatchYL;
			}

			DrawTile( Canvas, Image, X, Y, AmmoSize, YL, U, V, AmmoSize, YL, CL );		
		}

		U = FullU;
		V = FullV;
	}	
}

void UavaUIAmmoGraph::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize( inOwnerScene, inOwner );
}

#define MAX_WEAPON_INVENTORY	7

void UavaUIWeaponMenu::Render_Row( FCanvas* Canvas, FLOAT X, FLOAT Y, INT CategoryIndex )
{	
	FRenderParameters Parameters;

	FLOAT Offsets[16];
	FLOAT Alpha[16];
	check(Weapons.Num() < sizeof(Offsets) / sizeof(Offsets[0]) );

	FLOAT WX = CategoryIndexXL;

	INT i = 1;
	Offsets[0] = 0;
	Alpha[0] = RowAlpha[CategoryIndex];

	for (INT j=0; j<Weapons.Num(); ++j)
	{
		const FWeaponDrawInfo& WI = Weapons(j);		

		if (WI.Group != CategoryIndex) continue;		

		Offsets[i] = WX;		
		Alpha[i] = WI.Alpha;

		WX += WI.XL + Padding[0] + Padding[1];

		++i;
	}

	Offsets[i] = WX;
	Offsets[i+1] = Offsets[i];	



	FLOAT XL = Offsets[i];

	if (Background)
	{
		/// Alpha가 같은 것들 끼리 모아야 함.. -_-;
		for (INT j=0; j<i;)
		{
			INT k;
			for (k=j+1; k<i; ++k)
			{
				if (Alpha[j] != Alpha[k])
				{					
					break;
				}
			}

			Render_Background( Canvas, X, Y, XL, Offsets[j], Offsets[k] - Offsets[j], Alpha[j] );
			j = k;
		}		
	}


	TCHAR CategoryStr[2] = { TCHAR('0') + ((CategoryIndex + 1) % MAX_WEAPON_INVENTORY) , 0 };
	Parameters.DrawFont = CategoryIndexFont;
	UUIString::StringSize( Parameters, CategoryStr );	

	FLinearColor LC_Selected( SelectedColor[0] ), LC_Normal( DrawColor );		
	LC_Normal.A *= Opacity;	
	FLinearColor LC_Interpolated = (LC_Selected - LC_Normal) * RowAlpha[CategoryIndex] + LC_Normal;
	DrawString( Canvas, X + appTrunc( (CategoryIndexXL - Parameters.DrawXL) / 2 ), Y + appTrunc( ( BackgroundCoordinates.VL - Parameters.DrawXL) / 2 ), CategoryStr, CategoryIndexFont, LC_Interpolated );

	i = 1;
	for (INT j=0; j<Weapons.Num(); ++j)
	{		
		const FWeaponDrawInfo& WI = Weapons(j);
		if (WI.Group != CategoryIndex) continue;

		FLOAT Alpha = WI.Alpha;		
		LC_Selected	= SelectedColor[WI.MaintenanceRate];

		TCHAR IconStr[2] = { WI.Icon, 0 };

		FLOAT IX = X + Offsets[i] + Padding[0] + WI.Offset;
		FLOAT IY = Y + IconOffset;

		FLOAT NX = X + Offsets[i] + Padding[0] + NameOffset[0];
		FLOAT NY = Y + NameOffset[1];

		if (WI.Alpha > 0)
		{
			FLinearColor LC_Shadow( AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor );
			LC_Shadow.A *= Opacity * WI.Alpha;			

			DrawString( Canvas, IX-1, IY-1, IconStr, WeaponIconFont, LC_Shadow );
			DrawString( Canvas, IX, IY-1, IconStr, WeaponIconFont, LC_Shadow );			
			DrawString( Canvas, IX+1, IY-1, IconStr, WeaponIconFont, LC_Shadow );

			DrawString( Canvas, IX-1, IY, IconStr, WeaponIconFont, LC_Shadow );
			//DrawString( Canvas, IX, IY, IconStr, WeaponIconFont, LC_Shadow );			
			DrawString( Canvas, IX+1, IY, IconStr, WeaponIconFont, LC_Shadow );

			DrawString( Canvas, IX-1, IY+1, IconStr, WeaponIconFont, LC_Shadow );
			DrawString( Canvas, IX, IY+1, IconStr, WeaponIconFont, LC_Shadow );			
			DrawString( Canvas, IX+1, IY+1, IconStr, WeaponIconFont, LC_Shadow );

			DrawString( Canvas, NX-1, NY-1, WI.Name, WeaponNameFont, LC_Shadow );		
			DrawString( Canvas, NX, NY-1, WI.Name, WeaponNameFont, LC_Shadow );		
			DrawString( Canvas, NX+1, NY-1, WI.Name, WeaponNameFont, LC_Shadow );		

			DrawString( Canvas, NX-1, NY, WI.Name, WeaponNameFont, LC_Shadow );		
			//DrawString( Canvas, NX, NY, WI.Name, WeaponNameFont, LC );		
			DrawString( Canvas, NX+1, NY, WI.Name, WeaponNameFont, LC_Shadow );		

			DrawString( Canvas, NX-1, NY+1, WI.Name, WeaponNameFont, LC_Shadow );		
			DrawString( Canvas, NX, NY+1, WI.Name, WeaponNameFont, LC_Shadow );		
			DrawString( Canvas, NX+1, NY+1, WI.Name, WeaponNameFont, LC_Shadow );					
		}

		FLinearColor LC = (LC_Selected - LC_Normal) * Alpha + LC_Normal;

		DrawString( Canvas, IX, IY, IconStr, WeaponIconFont, LC );			
		DrawString( Canvas, NX, NY, WI.Name, WeaponNameFont, LC );		

		i++;
	}
}

void UavaUIWeaponMenu::Render_Column( FCanvas* Canvas, FLOAT X, FLOAT Y, INT CategoryIndex, INT OwnerTeam,UBOOL bSelectedCategory )
{
	FRenderParameters Parameters;

	FVector2D WeaponItemExtent = bSelectedCategory ? WeaponItemExtentLarge : WeaponItemExtentSmall;
	UFont *WeaponNameFont = bSelectedCategory ? WeaponNameFontLarge : WeaponNameFontSmall;
	UFont *WeaponIconFont = bSelectedCategory ? WeaponIconFontLarge : WeaponIconFontSmall;
	FLOAT DrawX = X;
	FLOAT DrawY = Y - WeaponItemExtent.Y;
	

	UINT DrawCount = 0;
	for(INT i = 0 ; i < Weapons.Num() ; i++)
	{
		const FWeaponDrawInfo& WI = Weapons(i);
		if( WI.Group != CategoryIndex ) 
			continue;

		FLinearColor LC_Selected( SelectedColor[WI.MaintenanceRate] ), LC_Normal( DrawColor );		

		FLinearColor LC0 = BackgroundColor[OwnerTeam];
		FLinearColor LC1 = BackgroundColor[OwnerTeam];
		
		LC0.A *= (1.0f - WI.Alpha) * BackgroundOpacity;
		LC1.A *= WI.Alpha * BackgroundOpacity;
		
		// Draw Background
		if( Background != NULL && BackgroundOpacity > 0.0f)
		{
			AavaWeapon* PendingWeapon = Cast<AavaWeapon>(PreviousWeapon);
			if( WI.Alpha < 1)
				DrawTile(Canvas, Background, DrawX, DrawY, WeaponItemExtent.X, WeaponItemExtent.Y, 
				BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC0);
			if( WI.Alpha > 0)
				DrawTile(Canvas, Background, DrawX, DrawY, WeaponItemExtent.X, WeaponItemExtent.Y, 
				BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC1);
			if( PendingWeapon != NULL && PendingWeapon == WI.Weapon)
			{
				USurface* DrawImage = SelectBackgroundImage ? SelectBackgroundImage : DefaultWhiteTexture;
				FTextureCoordinates DrawCoord = SelectBackgroundImageCoord;
				DrawTile(Canvas, DrawImage, DrawX, DrawY, WeaponItemExtent.X, WeaponItemExtent.Y, DrawCoord.U, DrawCoord.V, DrawCoord.UL, DrawCoord.VL, LC1);
			}
		}

		FLinearColor LC_Blend = (LC_Selected - LC_Normal) * WI.Alpha + LC_Normal;


		// Draw Number
		if( DrawCount == 0 )
		{
			FString NumStr = appItoa( CategoryIndex + 1 );
			FRenderParameters NumStrParam;
			NumStrParam.DrawFont = CategoryIndexFont;
			UUIString::StringSize(NumStrParam, *NumStr );
			DrawString(Canvas, DrawX /*+ WeaponItemExtent.X - NumStrParam.DrawXL*/  + NumberOffset.X,
				DrawY + WeaponItemExtent.Y - NumStrParam.DrawYL + NumberOffset.Y, CategoryIndexFont, *NumStr, LC_Blend );
		}

		FLOAT Left = DrawX + WeaponItemExtent.X/2;
		FLOAT Top = DrawY + WeaponItemExtent.Y/2;

		// Draw WeaponString
		if( WeaponIconFont )
		{
			TCHAR IconStr[] = { WI.Icon, TEXT('\0') };
			FRenderParameters StrParam;
			StrParam.DrawFont = WeaponIconFont;
			UUIString::StringSize(StrParam, IconStr);
			DrawString( Canvas, Left -=  StrParam.DrawXL/2, (Top -= StrParam.DrawYL/2) + IconOffset, WeaponIconFont, IconStr, LC_Blend);
		}

		// Draw Name
		if( WeaponNameFont )
		{
			FLOAT NameDrawX = Left + NameOffset[0];
			FLOAT NameDrawY = Top + NameOffset[1];
			FLinearColor LCShadow = NameShadowColor;
			LCShadow.A = LC_Blend.A; 
			DrawString( Canvas, NameDrawX + NameShadowOffset[0], NameDrawY + NameShadowOffset[0], WeaponNameFont, WI.Name, LCShadow);
			DrawString( Canvas, NameDrawX, NameDrawY, 	WeaponNameFont, WI.Name, LC_Blend);
		}

		DrawY -= WeaponItemExtent.Y;
		DrawCount++;
	}

	//	FLOAT Offsets[16];
	//	FLOAT Alpha[16];
	//	check(Weapons.Num() < sizeof(Offsets) / sizeof(Offsets[0]) );
	//
	//	// Row->Column이므로 CategoryIndexXL을 CategoryIndexYL로 사용
	//	FLOAT CategoryIndexYL = CategoryIndexXL;
	//	FLOAT WY = CategoryIndexYL;
	//
	//	INT i = 1;
	//	Offsets[0] = 0;
	//	Alpha[0] = RowAlpha[CategoryIndex];
	//
	//	for (INT j=0; j<Weapons.Num(); ++j)
	//	{
	//		const FWeaponDrawInfo& WI = Weapons(j);		
	//
	//		if (WI.Group != CategoryIndex) continue;		
	//
	//		Offsets[i] = WY;
	//		Alpha[i] = WI.Alpha;
	//
	//		FLOAT WI_YL = WI.XL;	//WI.XL은 Column 모드일때 WI.YL의 역할을 한다.
	//		WY += WI_YL;
	//		++i;
	//	}
	//
	//	Offsets[i] = WY;
	//	Offsets[i+1] = Offsets[i];	
	//
	//	FLinearColor LC_Selected( SelectedColor ), LC_Normal( DrawColor );		
	//	LC_Normal.A *= Opacity;	
	//
	//	FLinearColor LC_Interpolated = (LC_Selected - LC_Normal) * RowAlpha[CategoryIndex] + LC_Normal;
	//
	//	FLOAT YL = Offsets[i];
	//
	//	if (Background)
	//	{
	//		/// Alpha가 같은 것들 끼리 모아야 함.. -_-;
	//		for (INT j=0; j<i;)
	//		{
	//			INT k;
	//			for (k=j+1; k<i; ++k)
	//			{
	//				if (Alpha[j] != Alpha[k])
	//				{					
	//					break;
	//				}
	//			}
	//			Render_Background_Column( Canvas, X, Y, YL, Offsets[j] ,Offsets[k] - Offsets[j] , Alpha[j] );
	////			Render_Background( Canvas, X, Y, XL, Offsets[j], Offsets[k] - Offsets[j], Alpha[j] );
	//			j = k;
	//		}		
	//	}
	//
	//
	//	TCHAR CategoryStr[2] = { TCHAR('0') + ((CategoryIndex + 1) % MAX_WEAPON_INVENTORY) , 0 };
	//	Parameters.DrawFont = CategoryIndexFont;
	//	UUIString::StringSize( Parameters, CategoryStr );	
	//
	////	DrawString( Canvas, X + appTrunc( (CategoryIndexXL - Parameters.DrawXL) / 2 ), Y + appTrunc( ( BackgroundCoordinates.VL - Parameters.DrawXL) / 2 ), CategoryStr, CategoryIndexFont, LC_Interpolated );
	//	DrawString( Canvas, X + appTrunc( ( BackgroundCoordinates.UL - Parameters.DrawXL) / 2 ), Y + appTrunc( ( CategoryIndexYL - Parameters.DrawYL) / 2 ) - CategoryIndexYL, CategoryStr, CategoryIndexFont, LC_Interpolated );
	//
	//	i = 1;
	//	for (INT j=0; j<Weapons.Num(); ++j)
	//	{		
	//		const FWeaponDrawInfo& WI = Weapons(j);
	//		if (WI.Group != CategoryIndex) continue;
	//
	//		FLOAT Alpha = WI.Alpha;
	//		FLOAT WI_YL = WI.XL;
	//		TCHAR IconStr[2] = { WI.Icon, 0 };
	//
	//		FLOAT IX = X + Padding[0];
	//		FLOAT IY = Y - WI_YL - Offsets[i] + IconOffset;
	//
	//		FLOAT NX = X + NameOffset[0] + Padding[0];
	//		FLOAT NY = Y - WI_YL - Offsets[i] + NameOffset[1];
	//
	//		if (WI.Alpha > 0)
	//		{
	//			FLinearColor LC_Shadow( ShadowColor );
	//			LC_Shadow.A *= Opacity * WI.Alpha;			
	//
	//			DrawString( Canvas, IX-1, IY-1, IconStr, WeaponIconFont, LC_Shadow );
	//			DrawString( Canvas, IX, IY-1, IconStr, WeaponIconFont, LC_Shadow );			
	//			DrawString( Canvas, IX+1, IY-1, IconStr, WeaponIconFont, LC_Shadow );
	//
	//			DrawString( Canvas, IX-1, IY, IconStr, WeaponIconFont, LC_Shadow );
	//					//DrawString( Canvas, IX, IY, IconStr, WeaponIconFont, LC_Shadow );			
	//			DrawString( Canvas, IX+1, IY, IconStr, WeaponIconFont, LC_Shadow );
	//
	//			DrawString( Canvas, IX-1, IY+1, IconStr, WeaponIconFont, LC_Shadow );
	//			DrawString( Canvas, IX, IY+1, IconStr, WeaponIconFont, LC_Shadow );			
	//			DrawString( Canvas, IX+1, IY+1, IconStr, WeaponIconFont, LC_Shadow );
	//
	//			DrawString( Canvas, NX-1, NY-1, WI.Name, WeaponNameFont, LC_Shadow );		
	//			DrawString( Canvas, NX, NY-1, WI.Name, WeaponNameFont, LC_Shadow );		
	//			DrawString( Canvas, NX+1, NY-1, WI.Name, WeaponNameFont, LC_Shadow );		
	//
	//			DrawString( Canvas, NX-1, NY, WI.Name, WeaponNameFont, LC_Shadow );		
	//			//DrawString( Canvas, NX, NY, WI.Name, WeaponNameFont, LC );		
	//			DrawString( Canvas, NX+1, NY, WI.Name, WeaponNameFont, LC_Shadow );		
	//
	//			DrawString( Canvas, NX-1, NY+1, WI.Name, WeaponNameFont, LC_Shadow );		
	//			DrawString( Canvas, NX, NY+1, WI.Name, WeaponNameFont, LC_Shadow );		
	//			DrawString( Canvas, NX+1, NY+1, WI.Name, WeaponNameFont, LC_Shadow );					
	//		}
	//
	//		FLinearColor LC = (LC_Selected - LC_Normal) * Alpha + LC_Normal;
	//
	//		DrawString( Canvas, IX, IY, IconStr, WeaponIconFont, LC );			
	//		DrawString( Canvas, NX, NY, WI.Name, WeaponNameFont, LC );		
	//
	//		i++;
	//	}
}

void UavaUIWeaponMenu::Render_Background( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT Xe, FLOAT X0, FLOAT XL, FLOAT Alpha )
{	
	/// Background left, Background right
	FLOAT UL = BackgroundCoordinates.UL;
	FLOAT VL = BackgroundCoordinates.VL;
	FLOAT U = BackgroundCoordinates.U;
	FLOAT V = BackgroundCoordinates.V;

	FLinearColor LC0( 1, 1, 1, Opacity * (1-Alpha) );
	FLinearColor LC1( 1, 1, 1, Opacity * Alpha );

	FLOAT SU = U + SelectedBackgroundOffset[0];
	FLOAT SV = V + SelectedBackgroundOffset[1];	

	FLOAT BoxU = BackgroundLeft + U;
	FLOAT BoxSU = BackgroundLeft + SU;
	FLOAT BoxUL = Clamp( UL - BackgroundRight, 16.0f, UL );

	/// Left에 꼈다. :)
	if (X0 < BackgroundLeft)
	{
		FLOAT _X1 = Min( X0 + XL, BackgroundLeft );
		FLOAT _XL = _X1 - X0;

		if (Alpha < 1)
			DrawTile( Canvas, Background, X + X0, Y, _XL, VL, U + X0, V, _XL, VL, LC0 );
		if (Alpha > 0)
			DrawTile( Canvas, Background, X + X0, Y, _XL, VL, SU + X0, SV, _XL, VL, LC1 );

		XL -= _XL;
		X0 += _XL;
	}

	/// right에 꼈다 :)
	FLOAT Xre = (Xe - BackgroundRight);	
	if (X0 + XL > Xre)
	{
		FLOAT _X0 = Max( X0, Xre );
		FLOAT _XL = X0 + XL - _X0;

		if (Alpha < 1)
			DrawTile( Canvas, Background, X + _X0, Y, _XL, VL, (U + UL)/*Texture끝*/ - BackgroundRight/*오른쪽끝 시작*/ + Xe - _X0 /*오른쪽 끝에서 지난만큼*/, V, _XL, VL, LC0 );
		if (Alpha > 0)
			DrawTile( Canvas, Background, X + _X0, Y, _XL, VL, (SU + UL)/*Texture끝*/ - BackgroundRight/*오른쪽끝 시작*/ + Xe - _X0 /*오른쪽 끝에서 지난만큼*/, SV, _XL, VL, LC1 );

		XL -= _XL;
	}

	if (bShouldTileBackground)
	{
		/// 남은 건 BOX구간 뿐!
		while (XL > BoxUL)
		{
			if (Alpha < 1)
				DrawTile( Canvas, Background, X + X0, Y, BoxUL, VL, BoxU, V, BoxUL, VL, LC0 );
			if (Alpha > 0)
				DrawTile( Canvas, Background, X + X0, Y, BoxUL, VL, BoxSU, SV, BoxUL, VL, LC1 );

			X0 += BoxUL;
			XL -= BoxUL;
		}

		if (XL > 0)
		{
			if (Alpha < 1)
				DrawTile( Canvas, Background, X + X0, Y, XL, VL, BoxU, V, XL, VL, LC0 );
			if (Alpha > 0)
				DrawTile( Canvas, Background, X + X0, Y, XL, VL, BoxSU, SV, XL, VL, LC1 );
		}
	}
	else
	{
		if (Alpha < 1)
			DrawTile( Canvas, Background, X + X0, Y, XL, VL, BoxU, V, BoxUL, VL, LC0 );
		if (Alpha > 0)
			DrawTile( Canvas, Background, X + X0, Y, XL, VL, BoxSU, SV, BoxUL, VL, LC1 );
	}
}

void UavaUIWeaponMenu::Render_Background_Column( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT Ye, FLOAT Y0, FLOAT YL, FLOAT Alpha )
{
	/// Background left, Background right
	FLOAT UL = BackgroundCoordinates.UL;
	FLOAT VL = BackgroundCoordinates.VL;
	FLOAT U = BackgroundCoordinates.U;
	FLOAT V = BackgroundCoordinates.V;

	FLinearColor LC0( 1, 1, 1, Opacity * (1-Alpha) );
	FLinearColor LC1( 1, 1, 1, Opacity * Alpha );

	FLOAT SU = U + SelectedBackgroundOffset[0];
	FLOAT SV = V + SelectedBackgroundOffset[1];	

	FLOAT BackgroundBottom = BackgroundLeft;
	FLOAT BackgroundTop = BackgroundRight;

	FLOAT BoxV = BackgroundBottom + V;
	FLOAT BoxSV = BackgroundBottom + SV;
	FLOAT BoxVL = Clamp( VL - BackgroundTop, 16.0f, VL );

	//FLOAT BoxU = BackgroundLeft + U;
	//FLOAT BoxSU = BackgroundLeft + SU;
	//FLOAT BoxUL = Clamp( UL - BackgroundRight, 16.0f, UL );

	// Bottom에 꼈다. :) BackGroundLeft를 BackGroundBottom으로 사용
	if (Y0 < BackgroundLeft)
	{
		FLOAT _Y1 = Min( Y0 + YL, BackgroundLeft );
		FLOAT _YL = _Y1 - Y0;

		if (Alpha < 1)
			DrawTile( Canvas, Background, X, Y - Y0 - _YL,  UL, _YL,  U, V + Y0,  UL, _YL, LC0 );
		if (Alpha > 0)
			DrawTile( Canvas, Background, X, Y - Y0 - _YL,  UL, _YL,  SU, SV + Y0,  UL, _YL, LC1 );

		YL -= _YL;
		Y0 += _YL;
	}

	if( bShouldTileBackground )
	{
		while ( YL > BoxVL )
		{
			if( Alpha < 1)
				DrawTile( Canvas, Background, X , Y - Y0 - BoxVL,  UL, BoxVL,  U, BoxV,  UL, BoxVL, LC0);
			if( Alpha > 0)
				DrawTile( Canvas, Background, X , Y - Y0 - BoxVL,  UL, BoxVL,  SU, BoxSV,  UL, BoxVL, LC1);

			Y0 += BoxVL;
			YL -= BoxVL;
		}
		if( YL > 0 )
		{
			if( Alpha < 1)
				DrawTile( Canvas, Background, X, Y - Y0 - BoxVL, UL, YL, U, BoxV, UL, YL, LC0 );
			if( Alpha > 0)
				DrawTile( Canvas, Background, X, Y - Y0 - BoxVL, UL, YL, U, BoxSV, UL, YL, LC1);
		}
	}
	else
	{
		if( Alpha < 1)
			DrawTile( Canvas, Background, X, Y - Y0 - YL, UL, YL, U, BoxV, UL, BoxVL, LC0 );
		if( Alpha > 0)
			DrawTile( Canvas, Background, X, Y - Y0 - YL, UL, YL, U, BoxSV, UL, BoxVL, LC1);
	}
}

void UavaUIWeaponMenu::UpdateWeaponInfo( FWeaponDrawInfo& WI )
{
	FRenderParameters Parameters;

	TCHAR IconStr[2] = { WI.Icon, 0 };

	Parameters.DrawFont = WeaponIconFont;
	UUIString::StringSize( Parameters, IconStr );	

	if( WeaponLinkType == WEAPONLINK_Rows )
	{
		WI.Offset = Max( 0.0f, MinSizeX - Parameters.DrawXL );
		WI.XL = WI.Offset + Parameters.DrawXL;
	}
	else if ( WeaponLinkType == WEAPONLINK_Columns )
	{
		WI.Offset = Max( 0.0f, MinSizeX - Parameters.DrawYL );		// MinSizeX를 MinSizeY로 사용
		WI.XL = WI.Offset + Parameters.DrawYL;						// WI.XL을 WI.YL로 사용
	}
	else
	{
		checkf(false, TEXT("Unexceptable WeaponLinkType"));
	}
}

void UavaUIWeaponMenu::UpdateData()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;	
	//AavaInventoryManager* InvManager = PawnOwner ? Cast<AavaInventoryManager>(PawnOwner->InvManager) : NULL;
	UBOOL	bUpdate = FALSE;
	if ( PreviousPawn != PawnOwner )
	{
		PreviousPawn = PawnOwner;
		bUpdate = TRUE;
	}

	if (HUD && PawnOwner != NULL )
	{				
		if ( PreviousWeapon != PawnOwner->CurrentWeapon  ) 
		{
			bUpdate = TRUE;
			AavaWeapon* W = Cast<AavaWeapon>(PawnOwner->CurrentWeapon);
			PreviousWeapon = PawnOwner->CurrentWeapon;
			if ( W != NULL && W->bHideWeaponMenu == FALSE )
				MenuTTL = GWorld->GetTimeSeconds() + MenuActiveTime;
		}
		else if (MenuTTL < GWorld->GetTimeSeconds())
		{
			if (FadeOutTime > 0)
			{
				Opacity -= GWorld->GetDeltaSeconds() / FadeOutTime;
				Opacity = Clamp( Opacity, 0.0f, 1.0f );
			}
			else
			{
				if (Opacity > 0)
				{
					for (INT i=0; i<Weapons.Num(); ++i)
					{
						FWeaponDrawInfo& WI = Weapons(i);
						WI.Alpha = 0;
					}

					Opacity = 0;
				}

				return;
			}						
		}			
		else
		{
			Opacity = MaxOpacity;
		}

		if ( bUpdate )
		{
			Weapons.Empty();
			appMemzero( RowCount, sizeof(RowCount) );
			int InsertIndex = -1;
			for ( int i = 0 ; i < 12 ; ++i )
			{
				AavaWeapon* W = Cast<AavaWeapon>(PawnOwner->PossessedWeapon[i]);
				if ( W == NULL || W->AttachmentClass == NULL || W->bHideWeaponMenu == true )		continue;				
				FWeaponDrawInfo WI;
				WI.Icon = (*(W->AttachmentClass->GetDefaultObject<AavaWeaponAttachment>()->DeathIconStr))[0];
				WI.Name = const_cast<TCHAR*>( *(W->ItemName) );
				WI.Group = (W->InventoryGroup + MAX_WEAPON_INVENTORY - 1) % MAX_WEAPON_INVENTORY;
				WI.Weapon = W;
				WI.Alpha = 0;
				//if ( W->MaintenanceRate <= 0 )			WI.MaintenanceRate = 3;
				//else if ( W->MaintenanceRate <= 20 )	WI.MaintenanceRate = 2;
				//else if ( W->MaintenanceRate <= 80 )	WI.MaintenanceRate = 1;
				//else									WI.MaintenanceRate = 0;
				WI.MaintenanceRate = 0;
				RowCount[WI.Group] ++;
				UpdateWeaponInfo(WI);

				if ( PawnOwner->CurrentWeapon == W)
				{
					Weapons.InsertItem( WI, 0 );
					InsertIndex = 1;
				}
				else
				{
					if ( InsertIndex < 0 )	Weapons.AddItem( WI );
					else
					{
						Weapons.InsertItem( WI, InsertIndex++ );
					}
				}
			}
		}

		AavaWeapon* PendingWeapon	= Cast<AavaWeapon>(PreviousWeapon);		

		UBOOL bWeaponFound = FALSE;

		for (INT i=0; i<Weapons.Num(); ++i)
		{
			FWeaponDrawInfo& WI = Weapons(i);
			if (WI.Weapon == PendingWeapon)
			{
				WI.Alpha = ItemFadeInTime > 0 ? Clamp( WI.Alpha + GWorld->GetDeltaSeconds() / ItemFadeInTime, 0.0f, 1.0f ) : 1.0f;

				bWeaponFound = TRUE;

				for (INT j=0; j<MAX_WEAPON_INVENTORY; ++j)
				{
					if (j == WI.Group)
						RowAlpha[j] = ItemFadeInTime > 0 ? Clamp( RowAlpha[j] + GWorld->GetDeltaSeconds() / ItemFadeInTime, 0.0f, 1.0f ) : 1.0f;
					else
						RowAlpha[j] = ItemFadeOutTime < 0 ? Clamp( RowAlpha[j] - GWorld->GetDeltaSeconds() / ItemFadeOutTime, 0.0f, 1.0f ) : 0.0f;
				}				
			}
			else
			{
				WI.Alpha = ItemFadeOutTime > 0 ? Clamp( WI.Alpha - GWorld->GetDeltaSeconds() / ItemFadeOutTime, 0.0f, 1.0f ) : 0;
			}			
		}		
	}	
	else if (HUD == NULL && GIsEditor)
	{
		/// Update everytime! :)
		Weapons.Empty();

		FWeaponDrawInfo WI;

		WI.Icon = (*TestIcon)[0];
		WI.Name = const_cast<TCHAR*>(*TestWeaponName);
		WI.Alpha = 0;
		WI.Weapon = NULL;

		UpdateWeaponInfo( WI );

		WI.Group = 0; Weapons.AddItem( WI );
		WI.Group = 0; Weapons.AddItem( WI );
		WI.Group = 1; Weapons.AddItem( WI );
		WI.Group = 2; Weapons.AddItem( WI );
		WI.Group = 2; Weapons.AddItem( WI );
		WI.Alpha = 1.0f;
		WI.Group = 3; Weapons.AddItem( WI );
		WI.Alpha = 0.2f;
		WI.Group = 4; Weapons.AddItem( WI );				

		appMemzero( RowAlpha, sizeof(RowAlpha) );
		appMemzero( RowCount, sizeof(RowCount) );
		RowAlpha[3] = 1.0f;
		RowAlpha[4] = 0.2f;

		RowCount[0] = 2;
		RowCount[1] = 1;
		RowCount[2] = 2;
		RowCount[3] = 1;
		RowCount[4] = 1;

		Opacity = MaxOpacity;
	}	
}

void UavaUIWeaponMenu::Render_Widget( FCanvas* Canvas )
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->Pawn ) : NULL;
	AavaPlayerReplicationInfo* PRIOwner = PlayerOwner ? Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo) : NULL;


	AavaWeapon* PendingWeapon = PreviousWeapon ? Cast<AavaWeapon>(PreviousWeapon) : NULL;
	INT InventoryGroup = PawnOwner && PawnOwner->CurrentWeapon ? PawnOwner->CurrentWeapon->InventoryGroup : 
		PendingWeapon ? PendingWeapon->InventoryGroup : 0; 

	INT OwnerTeam = 0;
	
	if ( PRIOwner != NULL && PRIOwner->Team != NULL )
		OwnerTeam = PRIOwner->Team->TeamIndex;

	UpdateData();

	if (Opacity == 0.0f)
	{
		return;
	}

	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

	if( WeaponLinkType == WEAPONLINK_Rows )
	{
		for (INT i=0; i<MAX_WEAPON_INVENTORY; ++i)
		{
			if (RowCount[i])
				Render_Row( Canvas, X, Y, i );

			Y += BackgroundCoordinates.VL + RowSpacing;
		}
	}
	else if (WeaponLinkType == WEAPONLINK_Columns)
	{
		FLOAT ColSpacing = RowSpacing;
		//FLOAT ColumnWidth = Padding[0] + BackgroundCoordinates.UL + Padding[1] + ColSpacing;
		FLOAT DrawX = X;
		FLOAT DrawY = Y + YL;

		// 현재 잡고있는 무기도 없고 마지막으로 선택한 무기도 없음. 예기치 못한 상황
		if( InventoryGroup < 0 )
			return;

		for (INT i = 0 ; i < MAX_WEAPON_INVENTORY ; i++)
		{
			UBOOL bSelectedCategory = (i == (InventoryGroup - 1));
			if( RowCount[i] )
				Render_Column( Canvas, DrawX, DrawY, i, OwnerTeam, bSelectedCategory );
			DrawX += (bSelectedCategory ? WeaponItemExtentLarge.X : WeaponItemExtentSmall.X) + ColSpacing;
		}
	}
	else
	{
		checkf(false, TEXT("unexpected condition. UavaUIWeaponMenu::Render_Widget()"));
	}
}

UBOOL UavaUIThrowableIcon::UpdateString()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;		
	if ( PawnOwner == NULL )	return FALSE;
	if ( PawnOwner->InvManager == NULL )	return FALSE;
	AInventory* Inv = PawnOwner->InvManager->InventoryChain;
	FString NewMessage;
	for ( ; Inv != NULL ; Inv = Inv->Inventory )
	{
		AavaWeapon* weapon = Cast<AavaWeapon>( Inv );
		if ( weapon == NULL || weapon->AttachmentClass == NULL )		continue;
		if ( weapon->bAlwaysDrawIcon == false )	continue;
		NewMessage += FString::Printf( TEXT("%s"), *(weapon->AttachmentClass->GetDefaultObject<AavaWeaponAttachment>()->DeathIconStr) );
	}
	if ( Message != NewMessage )
	{
		Message = NewMessage;
		return true;
	}
	return false;
}

UBOOL UavaUISpecialInventory::UpdateString()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;		
	if ( PawnOwner == NULL )				return FALSE;
	if ( PawnOwner->InvManager == NULL )	return FALSE;
	AInventory* Inv = PawnOwner->InvManager->InventoryChain;
	FString NewMessage;
	for ( ; Inv != NULL ; Inv = Inv->Inventory )
	{
		AavaWeapon* weapon = Cast<AavaWeapon>( Inv );
		if ( weapon == NULL || weapon->AttachmentClass == NULL )		continue;
		if ( weapon->bSpecialInventory == false )	continue;
		NewMessage += FString::Printf( TEXT("%s"), *(weapon->AttachmentClass->GetDefaultObject<AavaWeaponAttachment>()->DeathIconStr) );
	}
	if ( Message != NewMessage )
	{
		Message = NewMessage;
		return true;
	}
	return false;
}

UBOOL UavaUIMissionIcon::UpdateString()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;
	if ( PlayerOwner == NULL )	return false;
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
	if ( HUD == NULL || GRI == NULL )	return false;
	if ( GRI->MissionType == MISSION_Bombing )
	{
		if ( !HUD->BombAlertBlink )
		{
			if ( HUD->BombInstalled )	DrawColor = BombAlarmColor;
			else						DrawColor = BombNormalColor;
		}
		else
		{
			DrawColor = BombBlinkColor;
		}
		Message = BombMissionCode;
	}
	else if ( GRI->MissionType == MISSION_Transport )
	{
		DrawColor = TransportNormalColor;
		Message = TransportMissionCode;
	}

	return true;
}

void UavaUIDamageIndicator::Render_Widget( FCanvas* Canvas )
{
	FLOAT CurrentTime = GWorld->GetTimeSeconds();
	FLOAT TotalRenderTime = DisplayTime + BlendTime;

	bool	bRender = false;

	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;		
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if ( Image == NULL )	
		return;

	FLinearColor	Color;

	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

	// PlayerOwner Should be here when Drawing.
	// Another Exception Condition Available?
	if (PlayerOwner == NULL || HUD == NULL)
	{
		Color	= IndicatorColor;
		if ( bDisplayDirection == true )
		{
			DrawTile( Canvas, Image, X + XL * ( 1 - TopWidth ) / 2, 
				Y + YL * TopPos, 
				XL * TopWidth, 
				YL * TopHeight, 
				TopIndicator.U, TopIndicator.V, TopIndicator.UL, TopIndicator.VL, Color );
		}
		else
		{
			DrawTile( Canvas, Image, X + XL * LeftPos, 
				Y + YL * TopPos, 
				XL * TopWidth, 
				YL * TopHeight, 
				TopIndicator.U, TopIndicator.V, TopIndicator.UL, TopIndicator.VL, Color );
		}
		return;
	}



	FRotator ViewRotator = PawnOwner ? PawnOwner->Rotation : FRotator(0,0,0);
	FVector ViewLocation = PawnOwner ? PawnOwner->Location : FVector(0,0,0);
	FVector ViewDir = ViewRotator.Vector();

	for( INT i = HUD->DamageIndicatorData.Num() - 1 ; i >= 0 ; i--)
	{
		FDamageIndicatorInfo& DamageInfo = HUD->DamageIndicatorData(i);
		FLOAT CurrentDamageTime = DamageInfo.DamageTime;
		FLOAT ElapsedTime		= CurrentTime - CurrentDamageTime;

		if ( ElapsedTime > TotalRenderTime )	continue;
		if ( bDisplayDirection == true && DamageInfo.bDisplayDirection == false )	continue;

		INT IndicatorAlpha = IndicatorColor.A;

		// Alpha가 빠지기 시작한다. 
		if ( ElapsedTime > DisplayTime && BlendTime > 0.0  )
			IndicatorAlpha *= (BlendTime + DisplayTime - ElapsedTime)/BlendTime;

		//else, Render Indicators
		Color	= IndicatorColor;
		Color.A = IndicatorAlpha;

		if ( bDisplayDirection == true )
		{
			FVector HitDirAbs = DamageInfo.HitLocAbs - ViewLocation;

			FVector ViewDir_ProjZ (ViewDir.X, ViewDir.Y, 0);
			FVector HitDirAbs_ProjZ (HitDirAbs.X, HitDirAbs.Y, 0);
			FVector ZAxis(0,0,1);

			ViewDir_ProjZ.Normalize();
			HitDirAbs_ProjZ.Normalize();

			FLOAT Angle = acosf( ViewDir_ProjZ | HitDirAbs_ProjZ);
			FVector YAxis = ZAxis ^ ViewDir_ProjZ;

			if( (YAxis | HitDirAbs_ProjZ) < 0 )
				Angle *= -1;

			Angle /= (2 * 3.141592f);
			Angle *= 65535;

			const INT RDSizeX = (INT) Canvas->GetRenderTarget()->GetSizeX();
			const INT RDSizeY = (INT) Canvas->GetRenderTarget()->GetSizeY();

			FMatrix TransMat = FTranslationMatrix( FVector(- RDSizeX / 2 , -RDSizeY / 2, 0))
				* FRotationMatrix( FRotator(0, appTrunc(Angle) ,0))
				* FTranslationMatrix( FVector(RDSizeX/2, RDSizeY/2, 0));
			Canvas->PushRelativeTransform(TransMat);

			DrawTile( Canvas, Image, X + XL * ( 1 - TopWidth ) / 2, 
				Y + YL * TopPos, 
				XL * TopWidth, 
				YL * TopHeight, 
				TopIndicator.U, TopIndicator.V, TopIndicator.UL, TopIndicator.VL, Color );

			Canvas->PopTransform();
		}
		else
		{
			DrawTile( Canvas, Image, X + XL * LeftPos, 
				Y + YL * TopPos, 
				XL * TopWidth, 
				YL * TopHeight, 
				TopIndicator.U, TopIndicator.V, TopIndicator.UL, TopIndicator.VL, Color );
		}
	}	
}

void UavaUIConsoleMessage::Initialize(UUIScene* inOwnerScene, UUIObject* inOwner/* =NULL  */)
{
	Super::Initialize( inOwnerScene, inOwner );

	if( Font )
	{
		FRenderParameters StringSizeParms( Font, 1.f, 1.f );
		UUIString::StringSize( StringSizeParms, TEXT("A") );
		DefaultHeight = StringSizeParms.DrawYL;
	}
	else
		DefaultHeight = 0.f;

	UpdateVisibleItemCount();
}

void UavaUIConsoleMessage::PostEditChange( UProperty* PropertyThatChanged )
{
	if( PropertyThatChanged )
	{
		if( PropertyThatChanged->GetFName() == FName(TEXT("Font")) && Font)
		{
			FRenderParameters StringSizeParms( Font, 1.f, 1.f );
			UUIString::StringSize( StringSizeParms, TEXT("A") );
			DefaultHeight = StringSizeParms.DrawYL;
			UpdateVisibleItemCount();
		}
		else if( PropertyThatChanged->GetFName() == FName(TEXT("MotionDuration")) )
		{
			MotionDuration = Clamp(MotionDuration, 0.1f, 1.f);
		}
	}
}

void UavaUIConsoleMessage::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition( Face );
	UpdateVisibleItemCount();
}

void UavaUIConsoleMessage::UpdateVisibleItemCount()
{
	VisibleItemCount = DefaultHeight > 0.f ? (GetBounds(UIORIENT_Vertical) / DefaultHeight) : 0.f;
}

void UavaUIConsoleMessage::SetTargetBottomIndex( FLOAT NewTargetBottomIndex)
{
	INT ConsoleMessageSize = GetConsoleMessageSize();
	FLOAT NewTargetIndex = Clamp<FLOAT>(NewTargetBottomIndex, 0, ConsoleMessageSize - 1 );

	PreviousBottomIndex = CurrentBottomIndex;
	TargetBottomIndex = NewTargetIndex;
	LeapTimeLeft = MotionDuration;

	if( NewTargetIndex == ConsoleMessageSize - 1 )
	{
		AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
		AavaHUD* HUD  = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
		if( HUD && IsInPageMode )
		{
			INT MsgIdx;
			INT FadeItemCount = VisibleItemCount + 1;
			for( INT i = 0 ; i < FadeItemCount && (MsgIdx = (ConsoleMessageSize - i - 1)) >= 0 ; i++ )
			{
				FLOAT MsgTime = GWorld->GetTimeSeconds() + MotionDuration + (FadeDuration / VisibleItemCount) * ( FadeItemCount - i);
				INT QueueIndex = (MsgIdx + HUD->ConsoleMessageInitIndex) % HUD->ConsoleMessageCount;
				if( ! HUD->ConsoleMessages.IsValidIndex(QueueIndex) )
					continue;

				HUD->ConsoleMessages( (MsgIdx + HUD->ConsoleMessageInitIndex) % HUD->ConsoleMessageCount).MessageLife = MsgTime;
			}
		}
		IsInPageMode = FALSE;
	}
	else
	{
		IsInPageMode = TRUE;
	}
}

UUIStyle_Text* UavaUIConsoleMessage::GetTextStyleByTypeName( FName TextTypeName )
{
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	AavaGameReplicationInfo* GRI = WorldInfo ? Cast<AavaGameReplicationInfo>(WorldInfo->GRI) : NULL;
	UUIStyle_Text* TextStyle;
	UUIStyle_Combo* ComboStyle;

	if( GRI == NULL )
		return NULL;

	if( GRI != NULL)
	{
		for( INT i = 0 ; i < GRI->TextStyleData.Num() ; i++ )
		{
			if( GRI->TextStyleData(i).Name == TextTypeName )
			{
				UUISkin* Skin = GetActiveSkin();
				if( Skin == NULL )
					continue;

				UUIStyle* Style = Skin->FindStyle( FName( *( GRI->TextStyleData(i).StyleTag), FNAME_Find) );
				if( Style == NULL )
					continue;

				UUIStyle_Data* StyleData = Style->GetStyleForStateByClass(UUIState_Enabled::StaticClass());
				TextStyle = Cast<UUIStyle_Text>(StyleData);
				if( TextStyle == NULL )
				{
					ComboStyle = Cast<UUIStyle_Combo>(StyleData);
					TextStyle = Cast<UUIStyle_Text>(ComboStyle->TextStyle.GetStyleData());
				}

				if( TextStyle != NULL )
					return TextStyle;
			}
		}
	}
	return NULL;
}


void UavaUIConsoleMessage::Render_Widget( FCanvas* Canvas )
{	
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaHUD* HUD  = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	INT ConsoleMessageSize = HUD ? HUD->ConsoleMessageSize : 0;

	// 현재 보고 있는 메세지들의 위치를 유지하기 위해 필요(반영안하면 새메세지가 들어올때 위치가 바뀐다)
	if( HUD && HUD->UpdateConsoleMessageThrowOuts > 0 )
	{
		FLOAT* BottomIndices[] = { &CurrentBottomIndex, &PreviousBottomIndex, &TargetBottomIndex };
		for( INT Index = 0 ; Index < ARRAY_COUNT(BottomIndices) ; Index++ )
		{
			INT ThrowOutCount = HUD->UpdateConsoleMessageThrowOuts;
			*BottomIndices[Index] = (*BottomIndices[Index] - ThrowOutCount >= 0) ? *BottomIndices[Index] : *BottomIndices[Index] + VisibleItemCount;
			*BottomIndices[Index] -= ThrowOutCount;
		}
		HUD->UpdateConsoleMessageThrowOuts = 0;
	}

	// 새아이템이 들어왔을경우
	if( HUD && HUD->UpdateConsoleMessageCount > 0 )
	{
		if( !IsInPageMode )
		{
			INT NewTargetBottomIndex = Clamp<FLOAT>(TargetBottomIndex + HUD->UpdateConsoleMessageCount, 0, ConsoleMessageSize - 1);
			SetTargetBottomIndex(NewTargetBottomIndex);
		}

		if( ScrollDownIcon )
			ScrollDownIcon->UpdateTime = GWorld->GetTimeSeconds();
		// HUD의 Update통지는 받았으므로 다시 초기화
		HUD->UpdateConsoleMessageCount = 0;
	}

	// 콘솔메세지 업/다운 이벤트 발생
	if( HUD && HUD->UpdateConsoleMessagePageUpDown != 0 )
	{
		if( HUD->UpdateConsoleMessagePageUpDown > 0 )
		{
			if( TargetBottomIndex > 0 )
			{
				INT NewTargetBottomIndex = Clamp<FLOAT>( TargetBottomIndex - VisibleItemCount, 0, ConsoleMessageSize - 1 );
				SetTargetBottomIndex( NewTargetBottomIndex );
			}
		}

		if ( HUD->UpdateConsoleMessagePageUpDown < 0 )
		{
			if( TargetBottomIndex < (ConsoleMessageSize - 1) )
			{
				INT NewTargetBottomIndex = Clamp<FLOAT>(TargetBottomIndex + VisibleItemCount, 0 , ConsoleMessageSize - 1);
				SetTargetBottomIndex( NewTargetBottomIndex );
			}
		}

		HUD->UpdateConsoleMessagePageUpDown = 0;
	}

	Render_Messages( Canvas );

	if( LeapTimeLeft > 0.f )
	{
		FLOAT Diff = (TargetBottomIndex - PreviousBottomIndex);
		CurrentBottomIndex += Diff * Clamp(GWorld->GetDeltaSeconds()/MotionDuration, 0.f, 1.f);
		CurrentBottomIndex = Diff > 0 ?  Min(CurrentBottomIndex,TargetBottomIndex) : Max(CurrentBottomIndex, TargetBottomIndex);

		if( (LeapTimeLeft -= GWorld->GetDeltaSeconds()) < 0.f)
		{
			LeapTimeLeft = 0;
			CurrentBottomIndex = TargetBottomIndex;
			PreviousBottomIndex = CurrentBottomIndex;
		}
	}
}

void UavaUIConsoleMessage::Render_Messages( FCanvas* Canvas )
{
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	AavaGameReplicationInfo* GRI = WorldInfo ? Cast< AavaGameReplicationInfo >(WorldInfo->GRI) : NULL;
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaHUD* HUD  = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	AavaPlayerReplicationInfo* PRI = PlayerOwner ? Cast<AavaPlayerReplicationInfo>( PlayerOwner->PlayerReplicationInfo )  : NULL;

	UBOOL	bIsTeamGame = IsTeamGame();

	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT XL = GetBounds(UIORIENT_Horizontal);
	FLOAT YL = DefaultHeight * VisibleItemCount;
	FLOAT Y  = RenderBounds[UIFACE_Bottom];
	FLOAT BottomY = Y;
	FLOAT TopY = Y - YL;

	FLOAT DrawXL, DrawYL;
	FRenderParameters Parameters;
	Parameters.DrawFont = Font;

	if ( GIsEditor && !GIsGame )
	{
		for ( INT i = TestMessages.Num() - 1 ; i >= 0  ; -- i )
		{
			FString Message = TestMessages(i);
			UUIString::StringSize( Parameters, *Message );
			DrawXL = appTrunc( Parameters.DrawXL );
			DrawYL = appTrunc( Parameters.DrawYL );
			Y -= DrawYL;
			FLinearColor DrawColor = AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ConsoleDefaultColor;
			FLinearColor ShadowColor = FLinearColor::Black;
			Render_MessageUnit( Canvas, X, Y, Message, Font, DrawColor, ShadowColor );
		}

		ScrollDownIconParms.DrawX = RenderBounds[UIFACE_Left] - ScrollDownIconParms.DrawXL;
		ScrollDownIconParms.DrawY = BottomY - ScrollDownIconParms.DrawYL;

		if( ScrollDownIcon )
			ScrollDownIcon->Render_Texture( Canvas, ScrollDownIconParms );

		ScrollUpIconParms.DrawX = RenderBounds[UIFACE_Left] - ScrollUpIconParms.DrawXL;
		ScrollUpIconParms.DrawY = TopY;

		if( ScrollUpIcon )
			ScrollUpIcon->Render_Texture( Canvas, ScrollUpIconParms );
	}

	if( GRI == NULL || PRI == NULL || ( HUD == NULL || HUD->ConsoleMessageSize <= 0 ) )
		return;

	INT MyTeamIndex = (PRI->Team && !PRI->bOnlySpectator) ? PRI->Team->TeamIndex : 2;
	const INT ConsoleMessageSize = HUD->ConsoleMessageSize;

	// 그려야할 데이터 범위 계산
	INT StartIndex , EndIndex;
	StartIndex = appCeil(Min(CurrentBottomIndex - VisibleItemCount - 1, TargetBottomIndex - VisibleItemCount - 1));
	StartIndex = Clamp(StartIndex, 0, ConsoleMessageSize - 1);
	EndIndex = appCeil(Max(CurrentBottomIndex,TargetBottomIndex));
	EndIndex = Clamp(EndIndex, 0, ConsoleMessageSize - 1);

	// 모션 오프셋
	FLOAT DiffY = Max(EndIndex - CurrentBottomIndex, 0.f);
	Y += appTrunc( DefaultHeight * DiffY );

	//debugf(TEXT("Start = %d, End = %d, CB = %f, TB = %f, DiffY = %f"), StartIndex, EndIndex, CurrentBottomIndex, TargetBottomIndex, DiffY);

	FString Message;
	INT XPos;

	for( INT i = EndIndex ; i >= StartIndex ; i-- )
	{
		INT QueueIndex = (i + HUD->ConsoleMessageInitIndex) % HUD->ConsoleMessageCount;
		if( ! HUD->ConsoleMessages.IsValidIndex( QueueIndex ) )
			continue;

		const FConsoleMessage& ConsoleMessage = HUD->ConsoleMessages( QueueIndex );

		UUIString::StringSize( Parameters, *ConsoleMessage.Text );
		DrawXL = appTrunc( Parameters.DrawXL );
		DrawYL = appTrunc( Parameters.DrawYL );
		XPos = X;
		Y	 -= DefaultHeight;

		FLOAT FadeOutAlpha = 1.f;

		// 페이지모드가 아닐때는 아이템들이 일정시간이 지나면 페이드아웃된다
		INT FadeItemCount = VisibleItemCount + 1;
		if( ( !IsInPageMode && ConsoleMessageSize - FadeItemCount <= i && i < ConsoleMessageSize ) )
		{
			if( (ConsoleMessage.MessageLife + MessageLifeTime + FadeDuration) < GWorld->GetTimeSeconds() )
				continue;

			FLOAT FadeOutTimeElapsed = GWorld->GetTimeSeconds() - (ConsoleMessage.MessageLife + MessageLifeTime);
			if( FadeOutTimeElapsed >= 0.f )
				FadeOutAlpha *= (1.f - Clamp(FadeOutTimeElapsed / FadeDuration, 0.f, 1.f));
		}

		// 렌더바운드를 벗어났거나 벗어나는중인 문자값들은 알파값을 다르게 적용한다.
		if( BottomY < (Y + DrawYL) )
			FadeOutAlpha *= Clamp( (BottomY - Y) / DrawYL, 0.f, 1.f);
		else if( TopY > Y )
			FadeOutAlpha *= Clamp( (Y + DrawYL - TopY) / DrawYL, 0.f, 1.f );

		/// 폰트와 색상을 준비한다
		UFont* DrawFont = Font;
		FLinearColor DrawColor		= ConsoleMessage.TextColor;
		FLinearColor LocationColor	= HUD->LocationColor;
		FLinearColor TeamColor;

		if ( ConsoleMessage.TeamIndex == 2 )
		{
			TeamColor = bIsTeamGame ? HUD->SpectatorColor : ConsoleMessage.PRI == PRI ? HUD->FriendlyColor : HUD->EnemyColor;
		}
		else
		{
			TeamColor = MyTeamIndex == ConsoleMessage.TeamIndex ? HUD->FriendlyColor : HUD->EnemyColor;
		}
		
		FLinearColor ShadowColor	= AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor;

		UUIStyle_Text* TextStyle = GetTextStyleByTypeName( ConsoleMessage.TypeName );
		if( TextStyle != NULL )
		{
			DrawFont = TextStyle->StyleFont ? TextStyle->StyleFont : DrawFont;
			DrawColor = TextStyle->StyleColor;
		}
		else
		{
			DrawColor = HUD->ChatColor;
		}

		/// 색상의 알파값을 바꾼다
		if( FadeOutAlpha < 1.f )
		{
			FLinearColor* DrawColorSeries[] = { &DrawColor, &LocationColor, &TeamColor, &ShadowColor };
			for( INT ColorIndex = 0 ; ColorIndex < ARRAY_COUNT(DrawColorSeries) ; ColorIndex++ )
				DrawColorSeries[ColorIndex]->A *= FadeOutAlpha;
		}

		/// 여기서 부터 진정 드로잉
		if ( ConsoleMessage.TypeName == MSGType_QC || ConsoleMessage.TypeName == MSGType_TeamSay || ConsoleMessage.TypeName == MSGType_Say )
		{	
			if ( ConsoleMessage.TypeName == MSGType_Say && bIsTeamGame )
			{
				Message = FString::Printf( TEXT("[%s]") , *HUD->ChatAll );
				Render_MessageUnit( Canvas, XPos, Y, Message, DrawFont, TeamColor, ShadowColor );
				UUIString::StringSize( Parameters, *Message );
				XPos += Parameters.DrawXL;
			}

			// TeamGame 일 경우에만 Team 이름을 보여준다.....
			if ( bIsTeamGame )
			{
				Message = FString::Printf( TEXT("[%s]") , *GRI->TeamNames[ConsoleMessage.TeamIndex] );
				Render_MessageUnit( Canvas, XPos, Y, Message, DrawFont, TeamColor, ShadowColor );
				UUIString::StringSize( Parameters, *Message );
				XPos += Parameters.DrawXL;
			}

			Message = FString::Printf( TEXT("%s") , *ConsoleMessage.PlayerName );
			Render_MessageUnit( Canvas, XPos, Y, Message, DrawFont, TeamColor, ShadowColor );
			UUIString::StringSize( Parameters, *Message );
			XPos += Parameters.DrawXL;

			// Spectator 는 Location 을 찍지 말자....
			if ( ConsoleMessage.TeamIndex != 2 && ( ConsoleMessage.TypeName == MSGType_QC || ConsoleMessage.TypeName == MSGType_TeamSay ) )		
			{
				Message = FString::Printf( TEXT( "[%s]" ), *ConsoleMessage.LocationName );
				Render_MessageUnit( Canvas, XPos, Y, Message, DrawFont, LocationColor, ShadowColor );
				UUIString::StringSize( Parameters, *Message );
				XPos += Parameters.DrawXL;
			}

			Message = FString::Printf( TEXT( ": %s" ), *ConsoleMessage.Text );
			Render_MessageUnit( Canvas, XPos, Y, Message, DrawFont, DrawColor, ShadowColor );
		}
		else
		{
			Message = ConsoleMessage.Text;
			Render_MessageUnit( Canvas, XPos, Y, Message, DrawFont, DrawColor, ShadowColor );
		}
	}

	// 스크롤이 가능한지 아이콘 드로잉
	if( IsInPageMode && TargetBottomIndex < (ConsoleMessageSize - 1) )
	{
		ScrollDownIconParms.DrawX = RenderBounds[UIFACE_Left] - ScrollDownIconParms.DrawXL;
		ScrollDownIconParms.DrawY = BottomY - ScrollDownIconParms.DrawYL;

		if( ScrollDownIcon )
			ScrollDownIcon->Render_Texture( Canvas, ScrollDownIconParms );
	}

	if( IsInPageMode && (TargetBottomIndex - VisibleItemCount) >= 0 )
	{
		ScrollUpIconParms.DrawX = RenderBounds[UIFACE_Left] - ScrollUpIconParms.DrawXL;
		ScrollUpIconParms.DrawY = TopY;

		if( ScrollUpIcon )
			ScrollUpIcon->Render_Texture( Canvas, ScrollUpIconParms );
	}
}

void UavaUIConsoleMessage::Render_MessageUnit(FCanvas* Canvas, FLOAT X, FLOAT Y, FString& Msg, UFont* DrawFont, FLinearColor& DrawColor, FLinearColor& ShadowColor )
{
	if( DrawColor.A < KINDA_SMALL_NUMBER )
		return;

	if ( bUseShadow )
		DrawString( Canvas, X+1, Y+1, *Msg, DrawFont, ShadowColor );
	DrawString( Canvas, X, Y, *Msg, DrawFont, DrawColor );
}

INT UavaUIConsoleMessage::GetConsoleMessageSize()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaHUD* HUD  = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	return HUD ? Max(HUD->ConsoleMessageSize,0) : 0;
}

// @TODO : bImmediately 구현 ( SetTargetIndex를 받자마자 중간에 움직이는 과정없이 상태가 변함 )
void UavaUIConsoleMessage::SetTargetIndex( INT NewTargetIndex, UBOOL Immediately /*=FALSE*/ )
{
	SetTargetBottomIndex( NewTargetIndex );
}

static void GetPlayerSummary( avaUIPlayerSummaryType Type, const AavaPlayerReplicationInfo* PRI, FString& string, INT OwnerTeam)
{
	// PRI가 NULL일 경우에 대한 처리 추가. deif
	if (PRI == NULL)
	{
		string = TEXT( "" );
		return;
	}

	switch(	Type )
	{
	case AVAUISummary_Level:	string = FString::Printf( TEXT("%d"), PRI->Level );				break;
	case AVAUISummary_Name:		
		if ( PRI->bSilentLogIn == FALSE )
		{
			string = PRI->PlayerName;
		}
		else
		{
			string = TEXT("");
		}
		break;
	case AVAUISummary_GuildName:
		{
		static const FString TestGuildName(TEXT("게임결과창테스트용길드명"));
		string  = TestGuildName;
		}
		break;
	case AVAUISummary_NextClass:
		{
			if ( PRI->PlayerClassID >= 0 && PRI->PlayerClassID < 3 )
				string = PRI->PlayerClassStr[PRI->PlayerClassID];
		}
		break;
	case AVAUISummary_Class:	
		{
			if ( PRI->CurrentSpawnClassID >= 0 && PRI->CurrentSpawnClassID < 3 )
				string = PRI->PlayerClassStr[PRI->CurrentSpawnClassID];
		}
		break;	

	case AVAUISummary_Score:	string = FString::Printf( TEXT("%d"), appTrunc(PRI->Score) );		break;	
	case AVAUISummary_Kill:		
		{
			INT nTotalKillCount = 0;
			for ( INT i = 0 ; i < 3 ; ++ i )
			{
				if ( PRI->avaCRI[i] != NULL )
				{
					nTotalKillCount += PRI->avaCRI[i]->KillCount;
				}
			}
			string = FString::Printf( TEXT("%d"), nTotalKillCount );	
		}
		break;	
	case AVAUISummary_Death:	string = FString::Printf( TEXT("%d"), appTrunc(PRI->Deaths) );	break;
	case AVAUISummary_Exp:		
		{
			//string = FString::Printf( TEXT("%d"), appTrunc(PRI->RoundWinScore * 40.0f + PRI->RoundLoseScore * 12.0f + PRI->Score * 10.0f + PRI->Deaths * 3.0f ) );
		}
		break;
	case AVAUISummary_Status:	
		{
			if ( PRI->bOnlySpectator == true )		string = PRI->SpectatorStr;
			else  if ( PRI->bIsSpectator == true )	
			{
				AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
				if ( GRI != NULL && GRI->bWarmupRound == true )	string = PRI->WhenWaitStr;
				else											string = PRI->WhenDeadStr;
			}
			else 
			{
				if ( PRI->Team != NULL && PRI->Team->TeamIndex == OwnerTeam )
					string = PRI->StatusStr;
			}
		}
		break;
	case AVAUISummary_Supply:	
		{
			//string = FString::Printf( TEXT("%d"), appTrunc(PRI->PlayTime * 6.0 /10.0 + PRI->HeadShotKill * 3.0 + PRI->RoundWinScore * 5.0 + PRI->Score * 5.0) );
		}
		break;
	case AVAUISummary_Bonus:	string = FString::Printf( TEXT("%d"), 100 );				break;
	case AVAUISummary_Ping:		string = FString::Printf( TEXT("%d"), PRI->Ping );			break;
	case AVAUISummary_Host:		string = FString::Printf( TEXT("%s"), _T("Host") );			break;
	case AVAUISummary_Leader:	string = FString::Printf( TEXT("%s"), _T("Leader") );		break;
	case AVAUISummary_SpectatorHelp :
		{
			AavaGameReplicationInfo*	GRI		= GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
			AavaPlayerController*		PCOwner = Cast<AavaPlayerController>(PRI->Owner);
			AavaHUD*					myHUD	= PCOwner != NULL ? Cast<AavaHUD>(PCOwner->myHUD) : NULL;
			if ( myHUD )	
			{
				if ( PCOwner->bUseFreeCam == false )
				{
					string = myHUD->DisplayedSpectatorHelp;
					if ( GRI != NULL &&  GRI->bAllowThirdPersonCam == TRUE )
						string += myHUD->DisplayedSpectatorHelpEx;
				}
				else
				{
					string = myHUD->FreeCamHelp;
				}
			}
		}
		break;
	case AVAUISummary_Help:	
		{
			AavaPlayerController* PCOwner = Cast<AavaPlayerController>(PRI->Owner);
			if ( PCOwner != NULL && Cast<AavaHUD>(PCOwner->myHUD) != NULL )	string = Cast<AavaHUD>(PCOwner->myHUD)->InGameHelp;
			else															string = FString::Printf(TEXT(""));
			break;
		}
	case AVAUISummary_PracticeHelp:
		{
			UavaNetHandler* NetHandler = UavaNetHandler::StaticClass()->GetDefaultObject<UavaNetHandler>()->GetAvaNetHandler();

			if ( GWorld->GetWorldInfo()->Pauser != NULL )
			{
				INT TimeSeconds = INT( GWorld->GetRealTimeSeconds() );
				if ( TimeSeconds % 2 == false )
					string = AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->PauseHelp;
				else
					FString::Printf(TEXT(""));
			}
			else if ( GWorld->GetNetMode() == NM_ListenServer && NetHandler->GetCurrentChannelMaskLevel() == 2 )
			{
				string = AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->RefereeNotify;
			}
			else if ( NetHandler != NULL && NetHandler->GetCurrentChannelFlag() == EChannelFlag_Practice )
			{
				AavaPlayerController* PCOwner = Cast<AavaPlayerController>(PRI->Owner);
				if ( PCOwner != NULL && Cast<AavaHUD>(PCOwner->myHUD) != NULL )	string = Cast<AavaHUD>(PCOwner->myHUD)->PracticeModeHelp;
				else															string = FString::Printf(TEXT(""));
			}
			break;
		}
	case AVAUISummary_CurrentWeapon:
		{
			AavaWeapon* Weapon = GetViewTargetCurrentWeapon();
			if ( Weapon != NULL )	string = Weapon->ItemName;
			else					string = FString::Printf(TEXT(""));
			break;
		}
	}
}

// [11/5/2006 YTS]
// [11/5/2006 YTS, add] GameStatus Scene에 쓰일 아이콘 정보를 반환한다.
// @param Surface - out value of IconTexture.
FPlayerSummaryIconInfo* UavaUIPlayersSummary::GetPlayerIconInfo(avaUIPlayerSummaryType Type, const AavaPlayerReplicationInfo* PRI, INT OwnerTeam)
{
	if( PRI == NULL )
		return NULL;

	TArray<FPlayerSummaryIconInfo> *IconInfo = NULL;
	for(INT i = 0 ; i < ColumnInfos.Num() ; i++)
	{
		if(ColumnInfos(i).SummaryType == Type)
		{
			IconInfo = &(ColumnInfos(i).IconInfo);
			break;
		}
	}

	if( IconInfo == NULL )
		return NULL;

	switch(Type)
	{

	case AVAUISummary_Level:
		//if( IconInfo->IsValidIndex(PRI->Level) )
		//	return &((*IconInfo)(PRI->Level));
		if( PRI->Level >= 0 && IconInfo->IsValidIndex(0))
			return &((*IconInfo)(0));
		break;
	case AVAUISummary_Class:	
		// waring : Index of PlayerClassID is slightly ambiguous ( inconsistent with others )
		if ( PRI->CurrentSpawnClassID >= 0 && PRI->CurrentSpawnClassID < 3 && IconInfo->IsValidIndex(PRI->CurrentSpawnClassID))
			return &((*IconInfo)(PRI->CurrentSpawnClassID));
		break;	
	case AVAUISummary_NextClass:
		if ( PRI->PlayerClassID >= 0 && PRI->PlayerClassID < 3 && IconInfo->IsValidIndex(PRI->PlayerClassID))
			return &((*IconInfo)(PRI->PlayerClassID));
		break;	
	case AVAUISummary_Ping:
		{
			int IconIndex;
			if ( PRI->bHost )			IconIndex = 3;
			else if ( PRI->Ping < 30 )	IconIndex = 0;
			else if ( PRI->Ping < 100 )	IconIndex = 1;
			else						IconIndex = 2;

			if ( IconInfo->IsValidIndex(IconIndex) )
				return &((*IconInfo)(IconIndex));
			else
				return NULL;
		}
		break;
	case AVAUISummary_Host:	

		if ( PRI->bHost )
		{
			if ( IconInfo->IsValidIndex(0) )
				return &((*IconInfo)(0));
		}
		break;
	case AVAUISummary_Leader:
		if ( PRI->bSquadLeader )
		{
			if ( IconInfo->IsValidIndex(0) )
				return &((*IconInfo)(0));
		}
	default:
		break;
	}

	return NULL;
}

// [11/5/2006 YTS]

static void GetRenderPos( float& XRet, float& YRet, UFont* Font, const TCHAR* String, avaUIVAlignType VAlign, avaUIHAlignType HAlign, float X, float Y, float XL, float YL )
{
	FRenderParameters Parameters;
	Parameters.DrawFont = Font;
	UUIString::StringSize( Parameters, String );

	if ( VAlign == VAlign_Top )			YRet = Y;
	else if ( VAlign == VAlign_Center )	YRet = Y + (YL - Parameters.DrawYL)/2;
	else if ( VAlign == VAlign_Bottom )	YRet = (Y + YL) - Parameters.DrawYL;

	if ( HAlign == HAlign_Left )		XRet = X;
	else if ( HAlign == HAlign_Right )	XRet = X + XL - Parameters.DrawXL;
	else if ( HAlign == HAlign_Center )	XRet = X + (XL - Parameters.DrawXL)/2;
}

void DrawThickBox( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT Thickness, const FLinearColor& DrawColor )
{
	/// vertical 
	::DrawTile( Canvas, X, Y + Thickness, Thickness, YL - Thickness * 2, 0, 0, 1, 1, DrawColor, GWhiteTexture );
	::DrawTile( Canvas, X + XL - Thickness, Y + Thickness, Thickness, YL - Thickness * 2, 0, 0, 1, 1, DrawColor, GWhiteTexture );
	/// horizontal
	::DrawTile( Canvas, X, Y, XL, Thickness, 0, 0, 1, 1, DrawColor, GWhiteTexture );
	::DrawTile( Canvas, X, Y + YL - Thickness, XL, Thickness, 0, 0, 1, 1, DrawColor, GWhiteTexture );
}

static void SortPRIList( TArray<AavaPlayerReplicationInfo*>& PRIResult, const AavaGameReplicationInfo* GRI, avaUIPlayerSummaryType SortBy, INT nTeam, bool bOnlySpectator, int LimitCnt = 0 )
{
	INT i,j;
	INT DstScore1, SrcScore1, DstScore2, SrcScore2;
	bool bInsert;

	for ( i = 0 ; i < GRI->PRIArray.Num() ; ++ i )
	{
		AavaPlayerReplicationInfo* PRI = Cast<AavaPlayerReplicationInfo>(GRI->PRIArray(i));
		if(PRI) { // 20070810 dEAthcURe
			// PRI 가 Bot이면 빼버린다....
			
			if ( PRI->bBot == true )													continue;
			if ( bOnlySpectator != PRI->bOnlySpectator )								continue;
			if ( PRI != NULL && PRI->Team != NULL && PRI->Team->TeamIndex != nTeam )	continue;

			bInsert = false;
			for ( j = 0 ; j < PRIResult.Num() ; ++ j )
			{
				const AavaPlayerReplicationInfo* PRIDest = PRIResult(j);

				if ( SortBy == AVAUISummary_SlotNum )
				{
					DstScore1	= 12 - PRIDest->SlotNum;
					SrcScore1	= 12 - PRI->SlotNum;
					DstScore2	= PRIDest->Score;
					SrcScore2	= PRI->Score;
				}
				else
				{
					DstScore1	= PRIDest->Score;
					SrcScore1	= PRI->Score;
					DstScore2	= PRIDest->Deaths;
					SrcScore2	= PRI->Deaths;
				}

				if ( DstScore1 < SrcScore1 )
				{
					bInsert = true;
					PRIResult.InsertItem( PRI, j );
					break;
				}
				else if ( DstScore1 == SrcScore1 )
				{
					if ( DstScore2 > SrcScore2 )
					{
						bInsert = true;
						PRIResult.InsertItem( PRI, j );
						break;
					}
				}
			}

			if ( bInsert == false )
			{
				PRIResult.AddItem( PRI );
			}
		} // 20070810 dEAthcURe

		if ( LimitCnt != 0 && PRIResult.Num() >= LimitCnt )
			return;
	}
}

void UavaUIPlayersSummary::Render_Widget( FCanvas* Canvas )
{
	FLOAT X  = RenderBounds[UIFACE_Left];								// xpos
	FLOAT Y  = RenderBounds[UIFACE_Top];								// ypos
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];	// width
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];	// height

	X = appTrunc(X);
	Y = appTrunc(Y);

	// Draw Header Control...
	INT i,j;
	FLOAT XAccum = X + Padding + XPos, YAccum = Y + YPos;
	FLOAT RenderX, RenderY;
	FString	DataString;

	AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
	if ( GRI == NULL )
	{
		FLOAT	SelectedX = X + XPos, 
			SelectedY = Y + YPos + HeaderHeight + RowSpacing, 
			SelectedWidth = 0, 
			SelectedHeight = DataHeight;

		// at first, Calculate row width of all items
		for( i = 0 ; i < ColumnInfos.Num() ; i++ )
		{
			const FColumnInfo& ColumnInfo = ColumnInfos(i);
			SelectedWidth += ColumnInfo.Width + ColumnInfo.Spacing;
		}
		
		INT TestIndex = 0 <= nTeam && nTeam <= 1 ? nTeam : 0;

		SelectedY += HightLightIdx * DataHeight;
		::DrawTile( Canvas, SelectedX,  SelectedY, SelectedWidth, SelectedHeight, 0, 0, 1, 1, HighlightBGColor[TestIndex], GWhiteTexture );
		::DrawTile( Canvas, SelectedX,  SelectedY + DataHeight, SelectedWidth, SelectedHeight, 0, 0, 1, 1, HighlightBGColor[TestIndex], GWhiteTexture );

		// Draw its Item left.
		SelectedWidth = 0;
		for ( i = 0 ; i < ColumnInfos.Num() ; ++ i )
		{
			YAccum = Y + YPos;
			const FColumnInfo& ColumnInfo = ColumnInfos(i);
			// Draw Header

			GetRenderPos( RenderX, RenderY, HeaderFont, *ColumnInfo.Label, (avaUIVAlignType)ColumnInfo.HeaderVerticalAlign, (avaUIHAlignType)ColumnInfo.HeaderHorizontalAlign, XAccum, YAccum, ColumnInfo.Width, HeaderHeight );
			DrawThickBox( Canvas, XAccum, YAccum, ColumnInfo.Width, HeaderHeight, 1, HeaderColor[TestIndex] );
			DrawString( Canvas, RenderX, RenderY, *ColumnInfo.Label, HeaderFont, HeaderColor[TestIndex] );

			YAccum += HeaderHeight + RowSpacing;	
			for ( j = 0 ; j < TestDataCnt ; ++ j )
			{
				FString DataString = TestData; 
				// Draw Column
				if( ColumnInfo.bClipString )
				{
					INT TableIndex = i * TestDataCnt + j;
					if( ! FieldLinearTable.IsValidIndex( TableIndex )  )
						FieldLinearTable.AddZeroed( (TableIndex + 1) - FieldLinearTable.Num() );
					check( FieldLinearTable.IsValidIndex( TableIndex ) );

					FSummaryFieldValueInfo& FieldValueInfo = FieldLinearTable(TableIndex);
					if( FieldValueInfo.FieldOrgValue != DataString )
					{
						FRenderParameters StrSizeParms( RenderX, RenderY, RenderX + (ColumnInfo.Width) , RenderY + DataHeight ,DataFont );
						UUIString::ClipString( StrSizeParms, *DataString, FieldValueInfo.FieldReplValue );
						FieldValueInfo.FieldOrgValue = DataString;
						if( FieldValueInfo.FieldOrgValue.Len() != FieldValueInfo.FieldReplValue.Len() )
							FieldValueInfo.FieldReplValue += TEXT("..");
					}
					DataString = FieldLinearTable(TableIndex).FieldReplValue;
				}

				GetRenderPos( RenderX, RenderY, HeaderFont, *DataString, (avaUIVAlignType)ColumnInfo.HeaderVerticalAlign, (avaUIHAlignType)ColumnInfo.DaraHorizontalAlign, XAccum, YAccum, ColumnInfo.Width, DataHeight );
				DrawThickBox( Canvas, XAccum, YAccum, ColumnInfo.Width, DataHeight, 1, DataColor[TestIndex] );

				FLinearColor DrawColor = (HightLightIdx == j || HightLightIdx == (j-1) ) ? HighlightFontColor[TestIndex] : DataColor[TestIndex];
				DrawColor = (HightLightIdx == (j-1) || HightLightIdx == (j+1)) ? DisabledColor[TestIndex] : DrawColor;
				DrawString( Canvas, RenderX, RenderY, *DataString, DataFont, DrawColor );

				YAccum += DataHeight;
			}
			XAccum += ColumnInfo.Width + ColumnInfo.Spacing;
			SelectedWidth += ColumnInfo.Width + ColumnInfo.Spacing;
		}
	}
	else
	{
		AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
		APlayerReplicationInfo* PRI = PlayerOwner ? PlayerOwner->PlayerReplicationInfo : NULL;
		if ( PlayerOwner == NULL || PRI == NULL )	return;
		// Sort PRI List
		TArray<AavaPlayerReplicationInfo*>	PRIArray;
		INT nOwnerTeam = PRI->Team ? PRI->Team->TeamIndex : 255;
		UBOOL nColorIndex = ( nOwnerTeam == nTeam || nOwnerTeam == 255 ) ? 0 : 1;

		// SortBy 추가...
		SortPRIList( PRIArray, GRI, AVAUISummary_Score, nTeam, bOnlySpectator, LimitCnt );

		// 비활성화할 항목을 미리 구함
		TArray<INT> DisabledColumns;
		for( INT PriIndex = 0 ; PriIndex < PRIArray.Num() ; PriIndex++ )
		{
			if( PRIArray(PriIndex) == NULL )	continue;
			FString StatusStr;
			GetPlayerSummary( AVAUISummary_Status, PRIArray(PriIndex), StatusStr, nOwnerTeam );
			if( PRIArray(PriIndex)->WhenDeadStr == StatusStr )
				DisabledColumns.AddUniqueItem(PriIndex);
		}

		// 선택된 항목을 구한다
		INT		SelectedColumn = INDEX_NONE;
		FLOAT	SelectedX = X + XPos, 
			SelectedY = Y + YPos + HeaderHeight + RowSpacing, 
			SelectedWidth = 0, 
			SelectedHeight = DataHeight;

		for( i = 0 ; i < ColumnInfos.Num() ; i++ )
		{
			const FColumnInfo& ColumnInfo = ColumnInfos(i);
			SelectedWidth += ColumnInfo.Width + ColumnInfo.Spacing;
			for( j = 0 ; j < PRIArray.Num() ; j++)
			{
				if( PRIArray(j) != NULL && PlayerOwner->PlayerReplicationInfo != NULL &&
					PRIArray(j) == PlayerOwner->PlayerReplicationInfo )
					SelectedColumn = j;
			}
		}

		// 선택된 항목을 그린다
		if ( SelectedColumn != INDEX_NONE )
		{
			FLinearColor BackColor = HighlightBGColor[nColorIndex];
			SelectedY += SelectedColumn * DataHeight;
			::DrawTile( Canvas, SelectedX,  SelectedY, SelectedWidth, SelectedHeight, 0, 0, 1, 1, BackColor, GWhiteTexture);
		}

		// and then draw another item.
		SelectedWidth = 0;
		for ( i = 0 ; i < ColumnInfos.Num() ; ++ i )
		{
			YAccum = Y + YPos;

			const FColumnInfo& ColumnInfo = ColumnInfos(i);

			GetRenderPos( RenderX, RenderY, HeaderFont, *ColumnInfo.Label, (avaUIVAlignType)ColumnInfo.HeaderVerticalAlign, (avaUIHAlignType)ColumnInfo.HeaderHorizontalAlign, XAccum , YAccum, ColumnInfo.Width, HeaderHeight );
			DrawString( Canvas, RenderX, RenderY, *ColumnInfo.Label, HeaderFont, HeaderColor[nColorIndex] );
			YAccum += HeaderHeight + RowSpacing;

			INT	PrvScore = -1, PrvDeath = -1;
			INT TieCount = 0;
			INT Rank	 = 0;

			for ( j = 0 ; j < PRIArray.Num() ; ++ j )
			{
				AavaPlayerReplicationInfo* PRI = PRIArray(j);
				if ( PRI == NULL )	continue;

				++Rank;
				if ( PRI->Score == PrvScore && PRI->Deaths == PrvDeath )
				{
					++TieCount;
				}
				else
				{
					TieCount = 0;
				}
				PrvScore = PRI->Score;
				PrvDeath = PRI->Deaths;

				FLOAT IconHeight = 0.0f;
				FLOAT IconWidth = 0.0f;
				UBOOL bDisabled = DisabledColumns.FindItemIndex( j ) != INDEX_NONE;
				// DrawIcon
				if( ColumnInfo.bShowIcon )
				{
					if ( ColumnInfo.SummaryType != AVAUISummary_ClanMarkIcon )
					{
						FPlayerSummaryIconInfo* IconInfo = GetPlayerIconInfo( avaUIPlayerSummaryType(ColumnInfo.SummaryType), PRI, nOwnerTeam);
						if( IconInfo != NULL && IconInfo->Surface != NULL)
						{
							FLOAT Width = IconInfo->Surface->GetSurfaceWidth();
							FLOAT Height = IconInfo->Surface->GetSurfaceHeight();
							FTextureCoordinates TexCoord = IconInfo->bIgnoreCoord ? FTextureCoordinates(0,0,Width,Height) : IconInfo->TexCoord;
							FIntPoint DrawExtent = IconInfo->bIgnoreExtent ? FIntPoint(TexCoord.UL, TexCoord.VL) : IconInfo->DrawExtent;
							FLinearColor IconColor = IconInfo->Color;
							if( bDisabled )
								IconColor.A = FLinearColor(DisabledColor[nColorIndex]).A;

							IconWidth = DrawExtent.X;
							IconHeight = DrawExtent.Y;
							if( ColumnInfo.SummaryType == AVAUISummary_Level )
							{
								static const INT SizeX = 5;
								static const INT SizeY = 6;
								INT Lev = Max(PRI->Level - 1,0);
								INT OffsetX = (Lev /(SizeX * SizeY)) * SizeX + ( Lev % SizeX );
								INT OffsetY = (Lev / SizeX) % SizeY;
								TexCoord.U += (OffsetX * TexCoord.UL);
								TexCoord.V += (OffsetY * TexCoord.VL);
							}

							DrawExtent.X	= DataHeight;
							DrawExtent.Y	= DataHeight;
							IconWidth		= DrawExtent.X;
							IconHeight		= DrawExtent.Y;

							DrawTile(Canvas, IconInfo->Surface, 
								appTrunc( XAccum + ( ColumnInfo.Width - IconWidth ) / 2 ), 
								appTrunc( YAccum + DataHeight - DrawExtent.Y ), 
								DrawExtent.X, 
								DrawExtent.Y, TexCoord.U, TexCoord.V, TexCoord.UL, TexCoord.VL, IconColor);
						}
					}
					else
					{
						DrawClanIcon( Canvas, XAccum, YAccum, DataHeight, DataHeight, bDisabled ? FLinearColor(DisabledColor[nColorIndex]).A : 1.0f, PRI->ClanMarkID, 0, 0 );
					}
				}

				// DrawString
				if( ColumnInfo.bShowString )
				{
					DataString = FString::Printf( TEXT("") );
					if ( ColumnInfo.SummaryType != AVAUISummary_Rank )
						GetPlayerSummary( avaUIPlayerSummaryType(ColumnInfo.SummaryType), PRI, DataString, nOwnerTeam);
					else
						DataString = FString::Printf( TEXT("%d"), Rank - TieCount );

					if( ColumnInfo.bClipString )
					{
						INT TableIndex = i * PRIArray.Num() + j;
						if( ! FieldLinearTable.IsValidIndex( TableIndex )  )
							FieldLinearTable.AddZeroed( (TableIndex + 1) - FieldLinearTable.Num() );
						check( FieldLinearTable.IsValidIndex( TableIndex ) );
						
						FSummaryFieldValueInfo& FieldValueInfo = FieldLinearTable(TableIndex);
						if( FieldValueInfo.FieldOrgValue != DataString )
						{
							FRenderParameters StrSizeParms( RenderX, RenderY, RenderX + (ColumnInfo.Width - IconWidth) , RenderY + DataHeight ,DataFont );
							UUIString::ClipString( StrSizeParms, *DataString, FieldValueInfo.FieldReplValue );
							FieldValueInfo.FieldOrgValue = DataString;
							if( FieldValueInfo.FieldOrgValue.Len() != FieldValueInfo.FieldReplValue.Len() )
								FieldValueInfo.FieldReplValue += TEXT("..");
						}
						DataString = FieldLinearTable(TableIndex).FieldReplValue;
					}
					GetRenderPos( RenderX, RenderY, HeaderFont, *DataString, (avaUIVAlignType)ColumnInfo.HeaderVerticalAlign, (avaUIHAlignType)ColumnInfo.DaraHorizontalAlign, XAccum + IconWidth, YAccum, ColumnInfo.Width - IconWidth, DataHeight );

					FLinearColor StrColor = SelectedColumn == j ? HighlightFontColor[nColorIndex] : DataColor[nColorIndex]; 
					StrColor = bDisabled ? DisabledColor[nColorIndex] : StrColor;
					DrawString( Canvas, RenderX, RenderY, *DataString, DataFont, StrColor );
				}
				YAccum += Max(DataHeight, IconHeight);
			}
			XAccum += ColumnInfo.Width + ColumnInfo.Spacing;
			SelectedWidth += ColumnInfo.Width + ColumnInfo.Spacing;
		}
	}
}

void UavaUIBackGroundImage::Render_Widget( FCanvas *Canvas )
{
	if ( TeamIndex != -1 )
	{
		INT ViewtargetTeamIndex = 2;
		if ( GIsEditor )
		{
			ViewtargetTeamIndex	= TestTeamIndex;
		}
		else
		{
			AavaPlayerReplicationInfo* ViewtargetPRI = GetPlayerViewTargetPRI();
			if ( ViewtargetPRI != NULL && ViewtargetPRI->Team != NULL)
			{
				ViewtargetTeamIndex = ViewtargetPRI->Team->TeamIndex;
			}
		}
		
		if ( ViewtargetTeamIndex != TeamIndex )
			return;
	}

	if ( BGImage != NULL )
	{
		FLOAT mW = BGImage->GetSurfaceWidth();
		FLOAT mH = BGImage->GetSurfaceHeight();
		for (INT i = 0 ; i < BGInfos.Num() ; ++ i )
		{
			const FBackGroundImageInfo& BGInfo = BGInfos(i);
			FLOAT X,Y,XL,YL,U,V,UL,VL;

			X  = RenderBounds[UIFACE_Left];
			Y  = RenderBounds[UIFACE_Top];
			XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
			YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
			
			//X	= BGInfo.XPos;
			//Y	= BGInfo.YPos;
			//XL	= BGInfo.Width;
			//YL	= BGInfo.Height;
			U	= BGInfo.TextureCoord.U;
			V	= BGInfo.TextureCoord.V;
			UL = BGInfo.TextureCoord.UL;
			VL = BGInfo.TextureCoord.VL;

			UTexture* Texture = Cast<UTexture>(BGImage);
			if ( Texture != NULL )
			{
				U /= mW; UL /= mW;
				V /= mH; VL /= mH;
				FTexture* RawTexture = Texture->Resource;
				::DrawTile(Canvas, X, Y, XL, YL, U, V, UL, VL, BGInfo.ImageColor, RawTexture);
			}
			else
			{
				UMaterialInstance* Material = Cast<UMaterialInstance>(BGImage);
				if ( Material != NULL )
				{
					::DrawTile(Canvas,X, Y, XL, YL, U, V, UL, VL, Material->GetInstanceInterface(0));
				}
			}

			//DrawTile( Canvas, BGImage, BGInfo.XPos, BGInfo.YPos, BGInfo.Width, BGInfo.Height, 
			//	BGInfo.TextureCoord.U, BGInfo.TextureCoord.V, BGInfo.TextureCoord.UL, BGInfo.TextureCoord.VL, BGInfo.ImageColor ); 
		}
	}
}

//  [9/12/2006 YTS]
//  Spectator의 정보를 나타냄 ( FreeCamera 상태 혹은 SpectatorPlayerName )
void UavaUISpectatorInfo::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);
}

void UavaUISpectatorInfo::Render_Text( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT ScaleX, FLOAT ScaleY )
{
	FLinearColor LC( CalcWarmColor( DrawColor ) );
	LC.A *= Opacity;

	if ( SpectatorLevel >= 0 )
	{
		DrawLevelIcon( Canvas, NULL, X, Y, DrawYL, DrawYL, Opacity, Max(SpectatorLevel - 1,0), 0, 0 );
		X += DrawYL + IconSpacing;
	}

	if (bDropShadow)
	{
		FLinearColor ShadowColor( CalcWarmColor( AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor ) );
		ShadowColor.A = LC.A;
		DrawString( Canvas, X + 1, Y + 1, *Message, Font, ShadowColor );									
	}

	DrawString( Canvas, X, Y, *Message, Font, LC );										
}

UBOOL UavaUISpectatorInfo::UpdateString()
{
	FString NewMsg;
	static INT LastValue = -2;
	INT NewValue;

	AavaPlayerReplicationInfo* PRI = GetPlayerViewTargetPRI();
	// "PlayerName( Health )"
	if (PRI != NULL)
	{
		NewMsg			= FString::Printf( TEXT("%s"), *(PRI->PlayerName));
		Message			= NewMsg;
		SpectatorLevel	= PRI->Level;
		return TRUE;
	}
	else
	{
		SpectatorLevel	= -1;
		NewValue		= -1;
		NewMsg			= TEXT("FreeCamera");
	}

	if(LastValue == NewValue)
		return FALSE;

	Message = NewMsg;
	return TRUE;
}
//  [9/12/2006 YTS]

//  [9/12/2006 YTS]
// Volume의 LocationName을 사용자 화면에 표시. 예시) 레이더 밑에 '창고'
void UavaUILocationInfo::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner)
{
	Super::Initialize(inOwnerScene, inOwner);
}

UBOOL UavaUILocationInfo::UpdateString()
{
	AavaPlayerController *PlayerOwner = GetavaPlayerOwner();
	AavaPlayerReplicationInfo *PRI = PlayerOwner ? Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo) : NULL;
	FString LocationName;

	const TCHAR* pszLocationName;
	static const TCHAR* pszLatestLocationName = NULL;

	if( PRI == NULL)
		pszLocationName = TEXT("Unknown");
	else if ( PRI->PlayerLocationHint == NULL)
		pszLocationName = TEXT("Spectator");
	else 
		pszLocationName = *PRI->LocationName;

	if( pszLocationName == NULL)
		pszLocationName = TEXT("");

	//if ( appStricmp( pszLocationName, pszLatestLocationName ) == 0)	
	//	return FALSE;

	pszLatestLocationName = pszLocationName;
	Message = pszLocationName;
	return TRUE;
}
//  [9/12/2006 YTS]

//  [9/13/2006 YTS]
// avaUIConsoleMessage의 내용을 빌려왔음. Radio QuickChat Menu를 화면에 그림.
// QuickChatMenuMessage에서 받아오는 StateUIMessage.Color는 더이상 사용하지 않음.
void UavaUIQuickChatMenu::Render_Widget( FCanvas* Canvas )
{

	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
	FLOAT Y  = RenderBounds[UIFACE_Top] + YL;

	FLOAT DrawXL, DrawYL;
	FRenderParameters Parameters;
	Parameters.DrawFont = Font;

	if ( GIsEditor )
	{
		for ( INT i = TestMessages.Num() - 1 ; i >= 0  ; -- i )
		{
			FString Message = TestMessages(i);
			UUIString::StringSize( Parameters, *Message );
			DrawXL = appTrunc( Parameters.DrawXL );
			DrawYL = appTrunc( Parameters.DrawYL );
			Y -= DrawYL;
			
			if ( bUseShadow )
				DrawString( Canvas, X+1, Y+1, *Message, Font, AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor );
			DrawString( Canvas, X, Y, *Message, Font, DefaultColor );
		}
	}

	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaHUD* HUD					  = NULL;

	if ( PlayerOwner != NULL )
		HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	if( HUD == NULL )
		return;

	for ( INT i = 0 ; i < HUD->QuickChatMenuMessages.Num() ;  )
	{
		const FStateUIMessage& StateUIMessage = HUD->QuickChatMenuMessages(i);

		if ( StateUIMessage.Text == "" || StateUIMessage.Life < GWorld->GetTimeSeconds() )
		{
			HUD->QuickChatMenuMessages.Remove( i, 1 );
		}
		else
			++i;
	}

	INT XPos;
	for ( INT i = HUD->QuickChatMenuMessages.Num() - 1  ; i >= 0 ; -- i )
	{
		const FStateUIMessage& StateUIMessage = HUD->QuickChatMenuMessages(i);

		if(StateUIMessage.Text.Len() == 0) 
			continue;

		UUIString::StringSize( Parameters, *StateUIMessage.Text );
		DrawXL = appTrunc( Parameters.DrawXL );
		DrawYL = appTrunc( Parameters.DrawYL );
		XPos = X;
		Y	 -= DrawYL;

		if ( bUseShadow )
			DrawString( Canvas, X+1, Y+1, *(StateUIMessage.Text), Font, AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor );
		DrawString(Canvas, XPos, Y, *(StateUIMessage.Text), Font, StateUIMessage.Color);
	}
}
//  [9/13/2006 YTS]

//  [9/13/2006 YTS]
// 
void UavaUIVoteMenu::Render_Widget( FCanvas* Canvas )
{
	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
	FLOAT Y  = RenderBounds[UIFACE_Top] + YL;

	FLOAT DrawXL, DrawYL;
	FRenderParameters Parameters;
	Parameters.DrawFont = Font;

	if ( GIsEditor )
	{
		for ( INT i = TestMessages.Num() - 1 ; i >= 0  ; -- i )
		{
			FString Message = TestMessages(i);
			UUIString::StringSize( Parameters, *Message );
			DrawXL = appTrunc( Parameters.DrawXL );
			DrawYL = appTrunc( Parameters.DrawYL );
			Y -= DrawYL;
			if ( bUseShadow )
				DrawString( Canvas, X+1, Y+1, *Message, Font, AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor );
			DrawString( Canvas, X, Y, *Message, Font, DefaultColor );
		}
	}

	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaHUD* HUD					  = NULL;

	if ( PlayerOwner != NULL )
		HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	if( HUD == NULL )
		return;

	for ( INT i = 0 ; i < HUD->VoteMenuMessages.Num() ; ++ i )
	{
		const FStateUIMessage& StateUIMessage = HUD->VoteMenuMessages(i);

		if ( StateUIMessage.Text == "" || StateUIMessage.Life < GWorld->GetTimeSeconds() )
		{
			HUD->VoteMenuMessages.Remove( i--, 1 );
		}
	}

	INT XPos;
	for ( INT i = HUD->VoteMenuMessages.Num() - 1  ; i >= 0 ; -- i )
	{
		const FStateUIMessage& StateUIMessage = HUD->VoteMenuMessages(i);

		if ( StateUIMessage.Text == "" )
			continue;

		UUIString::StringSize( Parameters, *StateUIMessage.Text );
		DrawXL = appTrunc( Parameters.DrawXL );
		DrawYL = appTrunc( Parameters.DrawYL );
		XPos = X;
		Y	 -= DrawYL;
		if ( bUseShadow )
			DrawString( Canvas, XPos+1, Y+1, *(StateUIMessage.Text), Font, AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor );
		DrawString(Canvas, XPos, Y, *(StateUIMessage.Text), Font, StateUIMessage.Color);
	}
}

void UavaUIDeathEffect::Render_Widget( FCanvas* Canvas )
{
	UavaNetHandler* NetHandler = UavaNetHandler::StaticClass()->GetDefaultObject<UavaNetHandler>()->GetAvaNetHandler();
	if ( NetHandler != NULL && !NetHandler->IsPlayerAdult() )
		return;

	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();

	FLinearColor	Color(EffectColor);
	//Color.A = TestAlpha;

	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	if ( HUD != NULL )
	{
		EffectStartTime = HUD->DeathSceneStartTime;
		FLOAT			EffectElapsedTime = GWorld->GetTimeSeconds() - EffectStartTime;
		if ( EffectElapsedTime > EffectTime )	EffectElapsedTime = EffectTime;
		Color.A	=	EffectMinAlpha + (EffectMaxAlpha - EffectMinAlpha) * EffectElapsedTime / EffectTime;
		Color.A /=  255.0;
	}
	else
	{
		Color.A = TestAlpha;
	}
	
	//if ( PlayerOwner != NULL )	
	//{
	//	FStateFrame* StateFrame = PlayerOwner->GetStateFrame();
	//	static FName DeadState( TEXT("Dead") );

	//	if (StateFrame && StateFrame->StateNode )
	//	{
	//		if (StateFrame->StateNode->GetFName() == DeadState )
	//		{
	//			if ( EffectStartTime == 0 )
	//			{
	//				EffectStartTime = GWorld->GetRealTimeSeconds();
	//			}
	//		}
	//		else
	//		{
	//			EffectStartTime = 0;
	//			return;
	//		}
	//	}


	//}

	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
	DrawTile( Canvas, EffectImage, X, Y, XL, YL, EffectCoordinates.U, EffectCoordinates.V, EffectCoordinates.UL, EffectCoordinates.VL, Color );
}

UBOOL UavaUIGameString::UpdateString()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	APlayerReplicationInfo* PRI;
	if ( PlayerOwner == NULL )	return FALSE;

	AavaPawn* PawnOwner = Cast<AavaPawn>(PlayerOwner->ViewTarget);
	// NextClass 는 Viewtarget에서 가지고 오면 안된다... 일단 임시로.....
	if ( PawnOwner != NULL && SummaryType == AVAUISummary_Class )	
		PRI = Cast<AavaPlayerReplicationInfo>(PawnOwner->PlayerReplicationInfo);
	else																
		PRI = Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo);

	if ( PRI == NULL )	return FALSE;
	INT nOwnerTeam = PRI->Team ? PRI->Team->TeamIndex : 255;
	FString	DataString;
	GetPlayerSummary( avaUIPlayerSummaryType(SummaryType), Cast<AavaPlayerReplicationInfo>(PRI), DataString, nOwnerTeam );
	if ( appStricmp( *DataString, *Message ) == 0)	return FALSE;
	Message =	DataString;
	return TRUE;
}

//  [9/13/2006 YTS]

//  [10/24/2006 YTS]
void UavaUIPDAPanel::Render_Widget( FCanvas* Canvas)
{
	//AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	//AavaPawn* PawnOwner = ( PlayerOwner ? Cast<AavaPawn>(PlayerOwner->Pawn) : NULL );
	//AavaPlayerReplicationInfo* PRIOwner;
	//AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI) : NULL;
	//AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	//if( PlayerOwner == NULL || PawnOwner == NULL || PawnOwner->bDeleteMe || GRI == NULL)
	//	return;

	//PRIOwner = Cast<AavaPlayerReplicationInfo>(PawnOwner->PlayerReplicationInfo);
	//if( PRIOwner == NULL || PRIOwner->Team == NULL)
	//	return;

	//if( PlayerOwner->ViewTarget == NULL)
	//	return;

	//// @TODO : drawing friend, enemy, quickchat wave, ... for pda display
	//INT Width, Height;
	//FVector2D ScreenLoc;

	//// Draw itSelf.
	//GPlayerSceneView->WorldToPixel(PawnOwner->Location,ScreenLoc);

	//// @TODO ?  'TeamIcon' can be null if you remove the default property of 'TeamIcon' in avaUIPDA
	//Width = Max( (INT)(IconScale / PlayerOwner->ViewTarget->Location.Z), MinIconSize);
	//Height = Max( (INT)(IconScale / PlayerOwner->ViewTarget->Location.Z), MinIconSize);

	//DrawTile(Canvas, TeamIcon, ScreenLoc.X - Width/2, ScreenLoc.Y - Height/2, Width, Height,
	//	0,0, TeamIcon->GetSurfaceWidth(), TeamIcon->GetSurfaceHeight(), TeamColor.IsValidIndex(PRIOwner->WaypointTeamindex) ? TeamColor(PRIOwner->WaypointTeamindex) : TeamColor(WPTeam_MAX) );

	//UUIStyle* UIStyle = PrimaryStyle.GetResolvedStyle();
	//UUIStyle_Combo* ComboStyle = UIStyle ? Cast<UUIStyle_Combo>(UIStyle->GetStyleForStateByClass(UUIState_Enabled::StaticClass())) : NULL;
	//UUIStyle_Text* TextStyle = ComboStyle ? Cast<UUIStyle_Text>(ComboStyle->TextStyle.GetStyleData()) : NULL;

	//if(bShowTeamName && TextStyle != NULL)
	//	DrawString(Canvas, ScreenLoc.X + Width/2 , ScreenLoc.Y + Height/2, TextStyle->StyleFont,
	//	*PRIOwner->PlayerName, TeamColor.IsValidIndex(PRIOwner->WaypointTeamindex) ? TeamColor(PRIOwner->WaypointTeamindex) : TeamColor(WPTeam_MAX) );

	//// Draw team members.
	//for (FDynamicActorIterator It; It; ++It)
	//{		
	//	AActor* Actor = *It;
	//	AavaPawn* Pawn = Cast<AavaPawn>(Actor);

	//	if ( Pawn == NULL || Pawn->bDeleteMe || PawnOwner == Pawn || 
	//		Pawn->PlayerReplicationInfo == NULL || Pawn->PlayerReplicationInfo->Team == NULL)
	//		continue;

	//	AavaPlayerReplicationInfo* PRI = Cast<AavaPlayerReplicationInfo>(Pawn->PlayerReplicationInfo);
	//	if (PRI != NULL &&  PRIOwner->Team->TeamIndex == PRI->Team->TeamIndex)
	//	{
	//		GPlayerSceneView->WorldToPixel(Pawn->Location,ScreenLoc);

	//		DrawTile( Canvas, TeamIcon, ScreenLoc.X - Width/2 , ScreenLoc.Y - Height/2, Width, Height, 
	//			0,0, TeamIcon->GetSurfaceWidth(), TeamIcon->GetSurfaceHeight(), PRI && TeamColor.IsValidIndex(PRI->WaypointTeamindex) ? TeamColor(PRI->WaypointTeamindex) : TeamColor(WPTeam_MAX));
	//		if( bShowTeamName && TextStyle != NULL)
	//			DrawString(Canvas, ScreenLoc.X + Width/2 , ScreenLoc.Y + Height/2, TextStyle->StyleFont, 
	//			*PRIOwner->PlayerName, TeamColor.IsValidIndex(PRI->WaypointTeamindex) ? TeamColor(PRI->WaypointTeamindex) : TeamColor(WPTeam_MAX) );

	//	}
	//}	

	////// @deprecated Draw the Enemy
	////// @TODO ?  'EnemyIcon' can be null if you remove the default property of 'EnemyIcon' in avaUIRadar
	////for( INT i = PawnOwner->EngageList.Num() - 1 ; i >= 0 ; i-- )
	////{
	////	APawn *P;
	////	if( (P = PawnOwner->EngageList(i).Pawn) == NULL)
	////	{
	////		PawnOwner->EngageList.Remove(i,1);
	////		continue;
	////	}

	////	ScreenLoc = Project(PawnOwner->EngageList(i).Pawn->Location);
	////	DrawTile( Canvas, EnemyIcon, ScreenLoc.X - Width/2, ScreenLoc.Y - Height/2, EnemyIcon->GetSurfaceWidth(), EnemyIcon->GetSurfaceHeight(), 0,0,Width, Height, EnemyColor);
	////	if( bShowEnemyName && TextStyle)
	////		DrawString( Canvas, ScreenLoc.X + Width/2 , ScreenLoc.Y + Height/2, TextStyle->StyleFont, P->PlayerReplicationInfo ? *P->PlayerReplicationInfo->PlayerName : TEXT("Unknown"), EnemyColor);
	////}

	//// Draw Waypoint
	//for( INT i = 0 ; i < WPTeam_MAX ; i++)
	//{
	//	FVector WaypointLoc = GRI->CurrentWaypoint[PRIOwner->Team->TeamIndex*WPTeam_MAX + i];
	//	if( WaypointLoc.Size() != 0.0f )
	//	{
	//		GPlayerSceneView->WorldToPixel(WaypointLoc,ScreenLoc);
	//		DrawTile(Canvas, FlagIcon, ScreenLoc.X - Width/2, ScreenLoc.Y - Height/2, Width, Height, 
	//			0,0, FlagIcon->GetSurfaceWidth(), FlagIcon->GetSurfaceHeight(), FlagColor(i));
	//		if(bShowFlagDistance && TextStyle)
	//		{
	//			FString Msg = appItoa(INT((PawnOwner->Location - WaypointLoc).Size()/30)) + TEXT("M");
	//			DrawString(Canvas, ScreenLoc.X + Width/2 , ScreenLoc.Y + Height/2, TextStyle->StyleFont, *Msg, FlagColor(i));
	//		}
	//	}
	//}

	//// Draw the location selected
	//if( VolumeSelected )
	//{
	//	GPlayerSceneView->WorldToPixel(VolumeSelected->Location,ScreenLoc);
	//	FLinearColor WhiteColor(1.0f,1.0f,1.0f,1.0f);
	//	DrawTile(Canvas, LocationIcon, ScreenLoc.X - Width/2, ScreenLoc.Y - Height/2, Width, Height,
	//		0,0, LocationIcon->GetSurfaceWidth(), LocationIcon->GetSurfaceHeight(), LocationColor);
	//}


	//// Move Camera from LatestLocation to NewLocation
	//AavaCameraActor* PDACamera = Cast<AavaCameraActor>(PlayerOwner->PDAViewTarget);
	//if(PDACamera && GWorld->GetWorldInfo() != NULL)
	//{
	//	if( PDACamera->MoveUpdateTime >= 0 && 
	//		PDACamera->MoveUpdateTime >= GWorld->GetWorldInfo()->TimeSeconds )
	//	{
	//		FLOAT TimeLeft = PDACamera->MoveUpdateTime - GWorld->GetWorldInfo()->TimeSeconds;
	//		FVector DiffLocation = PDACamera->LatestLocation - PDACamera->NewLocation;
	//		FRotator DiffRotation = PDACamera->LatestRotation - PDACamera->NewRotation;

	//		PDACamera->SetLocation( PDACamera->NewLocation + DiffLocation * ( TimeLeft / PDACamera->MoveDuration) );
	//		PDACamera->SetRotation( PDACamera->NewRotation + DiffRotation * ( TimeLeft / PDACamera->MoveDuration) );	
	//	}
	//	else
	//	{
	//		PDACamera->SetLocation( PDACamera->NewLocation );
	//		PDACamera->SetRotation( PDACamera->NewRotation );

	//		PDACamera->MoveUpdateTime = -1.0f;
	//	}
	//}
}

UBOOL UavaUIPDAPanel::ProcessInputAxis( const FInputEventParameters& EventParms)
{
	FName UIKey;
	if( !TranslateKey(EventParms, UIKey) )
		return FALSE;

	if( UIKey == UIKEY_CursorMove )
		UpdateIndicatorButton();

	return FALSE;
}

UBOOL UavaUIPDAPanel::ProcessInputKey( const FInputEventParameters& EventParms ) 
{
	//FName UIKey;
	//if(!TranslateKey(EventParms,UIKey) )
	//	return FALSE;

	////if( UIKey == UIKEY_TurnOn)
	//// an event 'TurnOn' performed by the script related with avaWeapon.Fire
	////else if( UIKey == UIKEY_TurnOff)
	//// an event 'TurnOff' performed by a kismet action (InputEvent TurnOff).

	//AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	//AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>(PlayerOwner->Pawn) : NULL;

	//if( PlayerOwner == NULL || PawnOwner == NULL)
	//	return FALSE;

	//if( UIKey == UIKEY_SelectLocation)
	//{
	//	if(EventParms.EventType == IE_Pressed)
	//	{
	//		HideButtons(TRUE,TRUE, FALSE);
	//		UpdateVolumeButton();
	//	}
	//}
	//else if( UIKey == UIKEY_DoStrategy)
	//{
	//	if( VolumeSelected == NULL )
	//		return FALSE;
	//	if( EventParms.EventType == IE_Pressed)
	//	{
	//		HideButtons(TRUE,TRUE,FALSE);
	//		UpdateStrategyButton();
	//	}
	//}
	//else if( UIKey == UIKEY_TurnOff)
	//{
	//	if(EventParms.EventType == IE_Released)
	//	{
	//		HideButtons();
	//		FName FNameKeyMappings[] = { KEY_Zero, KEY_One, KEY_Two, KEY_Three, KEY_Four, KEY_Five, KEY_Six, KEY_Seven, KEY_Eight, KEY_Nine, KEY_Escape};
	//		for(INT i = 0 ; i < ARRAY_COUNT(FNameKeyMappings) ; i++)
	//		{
	//			if( FNameKeyMappings[i] == EventParms.InputKeyName )
	//			{
	//				PlayerOwner->eventClosePDAMode(i);
	//				break;
	//			}
	//		}
	//	}
	//}

	return FALSE;
}

void UavaUIPDAPanel::UpdateVolumeButton()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>(PlayerOwner->Pawn) : NULL;

	if( PawnOwner == NULL )
		return;

	FIntPoint MousePosition = GetSceneClient()->MousePosition;
	FVector RayOrg, RayDir, RayExtent(0,0,0);
	CalcPickRay(MousePosition.X, MousePosition.Y, RayOrg, RayDir);
	FMemMark Mark(GMem);
	FCheckResult* Hit = GWorld->MultiLineCheck(GMem,RayOrg + RayDir * 100000, RayOrg, RayExtent, TRACE_Volumes, PawnOwner );

	INT TraceCount = 0;
	while( Hit != NULL && TraceCount < VolumeButtons.Num())
	{
		AVolume *Volume;
		if( TraceCount >= MaxMenuItems )
		{
			warnf(TEXT("Too Many Volumes. if you want to display more volumes, increase the 'MaxMenuItems' of avaUIPDAPanel"));
			break;
		}

		if( TraceCount < MaxMenuItems && (Volume = Cast<AVolume>(Hit->Actor)) != NULL )
		{
			VolumeButtons(TraceCount).Volume = Volume;
			VolumeButtons(TraceCount).Button->SetCaption(Volume->LocationName);
			VolumeButtons(TraceCount).Button->eventPrivateSetVisibility(TRUE);
			TraceCount++;
		}			
		Hit = Hit->GetNext();
	}
	Mark.Pop();

	if( VolumeButtons(0).Button != NULL)
	{
		VolumeButtons(0).Button->SetPosition(MousePosition.X, MousePosition.Y, 
			MousePosition.X + ButtonExtent.X, MousePosition.Y + ButtonExtent.Y);
	}

	// Hide reserved buttons not used
	for( INT i = TraceCount ; i < VolumeButtons.Num() ; i++ )
		if( VolumeButtons(i).Button !=NULL)
			VolumeButtons(i).Button->eventPrivateSetVisibility(FALSE);
}

void UavaUIPDAPanel::UpdateStrategyButton()
{
	AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI) : NULL;
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPlayerReplicationInfo* PRI = PlayerOwner ? Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo) : NULL;

	if( GRI == NULL || PlayerOwner == NULL || PRI == NULL || PRI->Team == NULL)
		return;

	// Invalid indices for referencing the array related with waypoints.
	if( ! ( 0 <= PRI->Team && PRI->Team->TeamIndex <= Max(TEAM_EU, TEAM_NRF ) ) )
		return;

	FIntPoint MousePosition = GetSceneClient()->MousePosition;

	for( INT i = 0 ; i < StrategyButtons.Num() ; i++ )
	{
		if(StrategyButtons(i).Button == NULL)
			continue;

		FString Msg;
		FVector WaypointLoc;
		UBOOL bIsEmptyWaypoint = ( (WaypointLoc = GRI->CurrentWaypoint[PRI->Team->TeamIndex*WPTeam_MAX + i]).Size() == 0.0f);
		UBOOL bIsVolumeSelected = VolumeSelected ? (VolumeSelected->Location.X == WaypointLoc.X &&
			VolumeSelected->Location.Y == WaypointLoc.Y) : FALSE;
		UBOOL bClearWaypoint = !bIsEmptyWaypoint && bIsVolumeSelected;

		Msg = FString(TEXT("WP")) + appItoa(i + 1)  + TEXT(" ") + (bClearWaypoint ? TEXT("Clear") : TEXT("Set"));
		StrategyButtons(i).Button->SetCaption(*Msg);
		StrategyButtons(i).Button->eventPrivateSetVisibility(TRUE);
		StrategyButtons(i).nWPTeam = i;
		StrategyButtons(i).nWPAction = bClearWaypoint ? WpAction_Clear : WPAction_Set;
	}

	if( StrategyButtons(0).Button != NULL )
		StrategyButtons(0).Button->SetPosition(MousePosition.X, MousePosition.Y, MousePosition.X + ButtonExtent.X, MousePosition.Y + ButtonExtent.Y);
}

void UavaUIPDAPanel::UpdateIndicatorButton()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>(PlayerOwner->Pawn) : NULL;

	if( PawnOwner == NULL )
		return;

	FIntPoint MousePosition = GetSceneClient()->MousePosition;
	FVector RayOrg, RayDir, RayExtent(0,0,0);
	CalcPickRay(MousePosition.X, MousePosition.Y, RayOrg, RayDir);
	FMemMark Mark(GMem);
	FCheckResult* Hit = GWorld->MultiLineCheck(GMem,RayOrg + RayDir * 100000, RayOrg, RayExtent, TRACE_Volumes, PawnOwner );

	INT TraceCount = 0;
	while( Hit != NULL && TraceCount < VolumeButtons.Num())
	{
		AVolume *Volume;
		if( TraceCount >= MaxMenuItems )
		{
			warnf(TEXT("Too Many Volumes. if you want to display more volumes, increase the 'MaxMenuItems' of avaUIPDAPanel"));
			break;
		}

		if( TraceCount < MaxMenuItems && (Volume = Cast<AVolume>(Hit->Actor)) != NULL )
		{
			IndicatorButtons(TraceCount)->SetCaption(Volume->LocationName);
			IndicatorButtons(TraceCount)->eventPrivateSetVisibility(TRUE);
			TraceCount++;
		}			
		Hit = Hit->GetNext();
	}
	Mark.Pop();


	if( IndicatorButtons(0) != NULL)
	{
		if(bFixUpperRightIndicator)
		{
			IndicatorButtons(0)->SetPosition(RenderBounds[UIFACE_Right] - ButtonExtent.X,  RenderBounds[UIFACE_Top] , RenderBounds[UIFACE_Right], RenderBounds[UIFACE_Top] + ButtonExtent.Y);
		}
		else
			IndicatorButtons(0)->SetPosition(MousePosition.X, MousePosition.Y, MousePosition.X + ButtonExtent.X, MousePosition.Y + ButtonExtent.Y);
	}

	// Hide reserved buttons not used
	for( INT i = TraceCount ; i < IndicatorButtons.Num() ; i++ )
		if( IndicatorButtons(i) !=NULL)
			IndicatorButtons(i)->eventPrivateSetVisibility(FALSE);
}


void UavaUIPDAPanel::GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames )
{
	out_KeyNames.AddItem(UIKEY_TurnOn);
	out_KeyNames.AddItem(UIKEY_TurnOff);
	out_KeyNames.AddItem(UIKEY_SelectLocation);
	out_KeyNames.AddItem(UIKEY_DoStrategy);
	out_KeyNames.AddItem(UIKEY_CursorMove);
}

void UavaUIPDAPanel::SetupDocLinksVolumeButton( UBOOL bResetLinks )
{
	//const FLOAT ButtonWidth = ButtonsExtent.GetValue(UIORIENT_Horizontal,EVALPOS_PixelViewport,this);
	//const FLOAT ButtonHeight = ButtonsExtent.GetValue(UIORIENT_Vertical,EVALPOS_PixelViewport,this);
	const FLOAT ButtonWidth = ButtonExtent.X;
	const FLOAT ButtonHeight = ButtonExtent.Y;

	FLOAT Left = 0 , Top = 0;
	for( INT i = 0 ; i < VolumeButtons.Num() ; i++)
	{
		// Remove an Invalid Button
		if( VolumeButtons(i).Button == NULL)
		{
			RemoveChild(VolumeButtons(i).Button);
			VolumeButtons.Remove(i--,0);
			continue;
		}

		// a First Element of Buttons.
		if( i == 0 )
			VolumeButtons(i).Button->SetPosition(Left,Top,Left+ButtonWidth,Top+ButtonHeight,EVALPOS_PixelViewport,TRUE);
		// Docking Left Buttons to First Button
		else
		{
			if( bResetLinks || ! VolumeButtons(i).Button->DockTargets.IsDocked(UIFACE_Left))
				VolumeButtons(i).Button->SetDockTarget(UIFACE_Left, VolumeButtons(i-1).Button, UIFACE_Left);
			if( bResetLinks || ! VolumeButtons(i).Button->DockTargets.IsDocked(UIFACE_Top))
				VolumeButtons(i).Button->SetDockTarget(UIFACE_Top, VolumeButtons(i-1).Button, UIFACE_Bottom);
			if( bResetLinks || ! VolumeButtons(i).Button->DockTargets.IsDocked(UIFACE_Right))
				VolumeButtons(i).Button->SetDockTarget(UIFACE_Right, VolumeButtons(i-1).Button, UIFACE_Right);
			if( bResetLinks || ! VolumeButtons(i).Button->DockTargets.IsDocked(UIFACE_Bottom))
				VolumeButtons(i).Button->SetDockTarget(UIFACE_Bottom, VolumeButtons(i-1).Button, UIFACE_Bottom, ButtonHeight);
		}
		Top += ButtonHeight;
	}
}

void UavaUIPDAPanel::SetupDocLinksStrategyButton( UBOOL bResetLinks )
{
	const FLOAT ButtonWidth = ButtonExtent.X;
	const FLOAT ButtonHeight = ButtonExtent.Y;

	FLOAT Left = 0 , Top = 0;
	for( INT i = 0 ; i < StrategyButtons.Num() ; i++)
	{
		// Remove an Invalid Button
		if( StrategyButtons(i).Button == NULL)
		{
			RemoveChild(StrategyButtons(i).Button);
			StrategyButtons.Remove(i--,0);
			continue;
		}

		// a First Element of Buttons.
		if( i == 0 )
			StrategyButtons(i).Button->SetPosition(Left,Top,Left+ButtonWidth,Top+ButtonHeight,EVALPOS_PixelViewport,TRUE);
		// Docking Left Buttons to First Button
		else
		{
			if( bResetLinks || ! StrategyButtons(i).Button->DockTargets.IsDocked(UIFACE_Left))
				StrategyButtons(i).Button->SetDockTarget(UIFACE_Left, StrategyButtons(i-1).Button, UIFACE_Left);
			if( bResetLinks || ! StrategyButtons(i).Button->DockTargets.IsDocked(UIFACE_Top))
				StrategyButtons(i).Button->SetDockTarget(UIFACE_Top, StrategyButtons(i-1).Button, UIFACE_Bottom);
			if( bResetLinks || ! StrategyButtons(i).Button->DockTargets.IsDocked(UIFACE_Right))
				StrategyButtons(i).Button->SetDockTarget(UIFACE_Right, StrategyButtons(i-1).Button, UIFACE_Right);
			if( bResetLinks || ! StrategyButtons(i).Button->DockTargets.IsDocked(UIFACE_Bottom))
				StrategyButtons(i).Button->SetDockTarget(UIFACE_Bottom, StrategyButtons(i-1).Button, UIFACE_Bottom, ButtonHeight);
		}
		Top += ButtonHeight;
	}
}

void UavaUIPDAPanel::SetupDocLinksIndicatorButton( UBOOL bResetLinks )
{
	const FLOAT ButtonWidth = ButtonExtent.X;
	const FLOAT ButtonHeight = ButtonExtent.Y;

	FLOAT Left = 0 , Top = 0;
	for( INT i = 0 ; i < IndicatorButtons.Num() ; i++)
	{
		// Remove an Invalid Button
		if( IndicatorButtons(i) == NULL)
		{
			RemoveChild(IndicatorButtons(i));
			IndicatorButtons.Remove(i--,0);
			continue;
		}

		// a First Element of Buttons.
		if( i == 0 )
			IndicatorButtons(i)->SetPosition(Left,Top,Left+ButtonWidth,Top+ButtonHeight,EVALPOS_PixelViewport,TRUE);
		// Docking Left Buttons to First Button
		else
		{
			if( bResetLinks || ! IndicatorButtons(i)->DockTargets.IsDocked(UIFACE_Left))
				IndicatorButtons(i)->SetDockTarget(UIFACE_Left, IndicatorButtons(i-1), UIFACE_Left);
			if( bResetLinks || ! IndicatorButtons(i)->DockTargets.IsDocked(UIFACE_Top))
				IndicatorButtons(i)->SetDockTarget(UIFACE_Top, IndicatorButtons(i-1), UIFACE_Bottom);
			if( bResetLinks || ! IndicatorButtons(i)->DockTargets.IsDocked(UIFACE_Right))
				IndicatorButtons(i)->SetDockTarget(UIFACE_Right, IndicatorButtons(i-1), UIFACE_Right);
			if( bResetLinks || ! IndicatorButtons(i)->DockTargets.IsDocked(UIFACE_Bottom))
				IndicatorButtons(i)->SetDockTarget(UIFACE_Bottom, IndicatorButtons(i-1), UIFACE_Bottom, ButtonHeight);
		}
		Top += ButtonHeight;
	}
}


void UavaUIPDAPanel::OnStyleResolved( UUIStyle* ResolvedStyle, const FStyleReferenceId& StylePropertyId, INT ArrayIndex, UBOOL bInvalidateStyleData )
{
	Super::OnStyleResolved(ResolvedStyle,StylePropertyId,ArrayIndex,bInvalidateStyleData);

	FString StylePropertyName = StylePropertyId.GetStyleReferenceName();
	if( StylePropertyName == TEXT("ButtonStyle"))
	{
		for(INT i = 0 ; i < VolumeButtons.Num() ; i++)
			if( VolumeButtons(i).Button != NULL)
				VolumeButtons(i).Button->SetWidgetStyle(ResolvedStyle);

		for(INT i = 0 ; i < StrategyButtons.Num() ; i++)
			if( StrategyButtons(i).Button != NULL)
				StrategyButtons(i).Button->SetWidgetStyle(ResolvedStyle);
	}
	else if (StylePropertyName == TEXT("IndicatorStyle"))
	{
		for(INT i = 0 ; i < IndicatorButtons.Num() ; i++)
			if( IndicatorButtons(i) != NULL)
				IndicatorButtons(i)->SetWidgetStyle(ResolvedStyle);
	}
}

void UavaUIPDAPanel::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner)
{
	UUILabelButton* LabelButton;
	// Initialize is called everytime when opening this scene. so cleaning them first.
	for(INT i = 0 ; i < VolumeButtons.Num() ; i++)
		RemoveChild(VolumeButtons(i).Button);
	VolumeButtons.Reset();

	for(INT i = 0 ; i < MaxMenuItems ; i++)
	{
		FVolumeButtonInfo VolumeButtonInfo;

		LabelButton = Cast<UUILabelButton>(CreateWidget(this, UUILabelButton::StaticClass()));
		check(LabelButton);
		InsertChild(LabelButton);
		VolumeButtonInfo.Button = LabelButton;
		VolumeButtons.AddItem(VolumeButtonInfo);
		LabelButton->eventPrivateSetVisibility(FALSE);
	}

	// [10/23/2006 YTS]
	// 전략버튼 총 2개가 필요 (블루,옐로 WaypointTeam 2개)
	for(INT i = 0 ; i < StrategyButtons.Num() ; i++)
		RemoveChild(StrategyButtons(i).Button);
	StrategyButtons.Reset();

	for(INT i = 0 ; i < WPTeam_MAX ; i++)
	{
		FStrategyButtonInfo StrategyButtonInfo;
		LabelButton = Cast<UUILabelButton>(CreateWidget(this, UUILabelButton::StaticClass()));
		check(LabelButton);
		// Attach to avaUIPDA
		InsertChild(LabelButton);
		// Storing Ref. for the futher processing.
		StrategyButtonInfo.Button = LabelButton;
		StrategyButtonInfo.nWPTeam = i;
		//		StrategyButtonInfo.nWPAction = i / WPTeam_MAX;
		StrategyButtons.AddItem(StrategyButtonInfo);

		LabelButton->SetEnabled(FALSE);		// Disable
		LabelButton->eventPrivateSetVisibility(FALSE);		// Hide
	}

	//  [10/25/2006 otterrrr] 
	// 볼륨 인디케이터 : 10개 만들어놓고 필요한 갯수만큼 Show/Hide해서 사용
	for( INT i = 0  ; i < IndicatorButtons.Num() ; i++)
		RemoveChild(IndicatorButtons(i));
	IndicatorButtons.Reset();

	for(INT i = 0 ; i < MaxMenuItems ; i++)
	{
		LabelButton = Cast<UUILabelButton>(CreateWidget(this, UUILabelButton::StaticClass()));
		check(LabelButton);
		InsertChild(LabelButton);
		IndicatorButtons.AddItem(LabelButton);

		LabelButton->SetEnabled(FALSE);
		LabelButton->eventPrivateSetVisibility(FALSE);
	}

	// Arrage buttons
	SetupDocLinksVolumeButton(TRUE);
	SetupDocLinksStrategyButton(TRUE);
	SetupDocLinksIndicatorButton(TRUE);

	Super::Initialize(inOwnerScene, inOwner);
}

void UavaUIPDAPanel::SetLocation(class UUIButton* Sender,INT PlayerIndex)
{
	UUILabelButton* LabelButton;
	if( (LabelButton = Cast<UUILabelButton>(Sender)) == NULL )
	{
		debugf(TEXT("Sender Button %s is not LabelButton"), *Sender->GetName());
		return;
	}

	// Find User-Selected Volume & Store it to 'VolumeSelected'
	for( INT i = 0 ; i < VolumeButtons.Num() ; i++)
	{
		if( (VolumeButtons(i).Button != NULL && VolumeButtons(i).Volume != NULL) &&
			VolumeButtons(i).Volume->LocationName == LabelButton->StringRenderComponent->GetValue())
		{
			VolumeSelected = VolumeButtons(i).Volume;
			// @TODO : another process for displaying a selected  region.
			break;
		}
	}

	HideButtons();
}

void UavaUIPDAPanel::DoStrategy( class UUIButton* Sender, INT PlayerIndex )
{
/*	UUILabelButton* LabelButton = Sender ? Cast<UUILabelButton>(Sender) : NULL;
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPlayerReplicationInfo* PRI = PlayerOwner ? Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo) : NULL;

	if( LabelButton == NULL || PlayerOwner == NULL || PRI == NULL)
		return;

	for(INT i = 0 ; i < StrategyButtons.Num() ; i++)
	{
		if( StrategyButtons(i).Button == NULL)
			continue;

		if( LabelButton == StrategyButtons(i).Button && VolumeSelected != NULL)
		{
			switch(StrategyButtons(i).nWPAction)
			{
			case WPAction_Set:
			case WPAction_Reset:
				PlayerOwner->eventSetWaypointFromPDA(StrategyButtons(i).nWPTeam, VolumeSelected->Location);
				break;
			case WpAction_Clear:
				PlayerOwner->eventClearWaypointFromPDA(StrategyButtons(i).nWPTeam);
				break;
			}
		}
	}

	VolumeSelected = NULL;
	HideButtons()*/;
}

void UavaUIPDAPanel::HideButtons(UBOOL bHideVolumeButton, UBOOL bHideStrategyButton, UBOOL bHideIndicatorButton)
{
	if(bHideVolumeButton)
	{
		for(INT i = 0 ; i < VolumeButtons.Num() ; i++)
			if(VolumeButtons(i).Button != NULL)
				VolumeButtons(i).Button->eventPrivateSetVisibility(FALSE);
	}

	if(bHideStrategyButton)
	{
		for(INT i = 0 ; i < StrategyButtons.Num() ; i++)
			if( StrategyButtons(i).Button != NULL)
				StrategyButtons(i).Button->eventPrivateSetVisibility(FALSE);
	}

	if(bHideIndicatorButton)
	{
		for(INT i = 0 ; i < IndicatorButtons.Num() ; i++)
			if( IndicatorButtons(i) != NULL)
				IndicatorButtons(i)->eventPrivateSetVisibility(FALSE);
	}
}

void UavaUIKillMessage::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);
	LatestUpdateTime = -1.0f;
}

void UavaUIKillMessage::Render_Text(FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT ScaleX , FLOAT ScaleY  )
{
	const INT LevelIconWidth = 19.f * ScaleX;

	if ( Level >= 0 )
	{
		switch( IconPosition )
		{
		case UIFACE_Left:								break;
		case UIFACE_Top:	X -= LevelIconWidth * 0.5f;	break;
		case UIFACE_Right:	X -= LevelIconWidth;		break;
		case UIFACE_Bottom:	X -= LevelIconWidth * 0.5f;	break;
		default: break;
		}

		DrawLevelIcon( Canvas, NULL, X, Y, DrawYL * ScaleX, DrawYL * ScaleY, Opacity, Max(Level - 1,0), 0, 0 );
		X += (LevelIconWidth /* + IconSpacing*/ );
	}

	Super::Render_Text( Canvas, X, Y, ScaleX, ScaleY );
}

BOOL UavaUIKillMessage::CheckViewtargetKiller( const FKillMessageInfo& MsgInfo, const AavaPlayerController* PlayerOwner )
{
	return ( PlayerOwner->PlayerReplicationInfo == MsgInfo.KillerPRI ||  PlayerOwner->RealViewTarget == MsgInfo.KillerPRI );
}

BOOL UavaUIKillMessage::CheckViewtargetVictim( const FKillMessageInfo& MsgInfo, const AavaPlayerController* PlayerOwner )
{
	return ( PlayerOwner->PlayerReplicationInfo == MsgInfo.KilledPRI ||  PlayerOwner->RealViewTarget == MsgInfo.KilledPRI );
}

INT UavaUIKillMessage::UpdateInfo()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPlayerReplicationInfo* PRIOwner = PlayerOwner ? Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo) : NULL;
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if( HUD == NULL || PRIOwner == NULL)
		return 0;

	// Remove exhaust.
	for(INT i = HUD->MyKillMessageData.Num() - 1; i >= 0 ; i--)
	{
		if ( CheckViewtargetKiller( HUD->MyKillMessageData(i), PlayerOwner ) == FALSE &&
			 CheckViewtargetVictim( HUD->MyKillMessageData(i), PlayerOwner ) == FALSE )
		{
			HUD->MyKillMessageData.Remove(i,1);
		}
	}

	FLOAT UpdateTime = -1.0f;			// -1.0f = No DeathEvent related with PlayerOwner
	FString MessageString( TEXT("") );
	UBOOL bKilled = FALSE;
	UBOOL bExplosive = FALSE;
	UBOOL bHeadShot = FALSE;
	UBOOL bWallShot = FALSE;

	for(INT i = HUD->MyKillMessageData.Num() - 1 ; i >= 0 ;i--)
	{
		const FKillMessageInfo& KillMsgInfo = HUD->MyKillMessageData(i);

		bHeadShot = KillMsgInfo.bHeadShot;
		bWallShot = KillMsgInfo.bWallShot;
		bExplosive = KillMsgInfo.bExplosive;

		if( CheckViewtargetKiller( KillMsgInfo, PlayerOwner ) && KillMsgInfo.KilledPRI != NULL )
		{
			UpdateTime = KillMsgInfo.TimeOfDeath;
			bKilled = FALSE;
			MessageString = *(KillMsgInfo.KilledPRI->PlayerName);
			Level		  =	Cast<AavaPlayerReplicationInfo>(KillMsgInfo.KilledPRI) ? Cast<AavaPlayerReplicationInfo>(KillMsgInfo.KilledPRI)->Level : -1;
			break;
		}
		if( CheckViewtargetVictim( KillMsgInfo, PlayerOwner ) && KillMsgInfo.KillerPRI != NULL )
		{
			UpdateTime = KillMsgInfo.TimeOfDeath;
			bKilled = TRUE;
			MessageString = *(KillMsgInfo.KillerPRI->PlayerName);
			Level		  =	Cast<AavaPlayerReplicationInfo>(KillMsgInfo.KillerPRI) ? Cast<AavaPlayerReplicationInfo>(KillMsgInfo.KillerPRI)->Level : -1;
			break;
		}
	}

	// 현재의 Message를 유지
	if( LatestUpdateTime == UpdateTime )
	{
		if( FadeMode == 0 && LatestUpdateTime == -1.f && Message.Len() > 0 )
			Message = TEXT("");
		return 0;
	}
	else if( LatestUpdateTime != -1.f && UpdateTime == -1.f )
	{
		LatestUpdateTime = UpdateTime; 
		return -1;
	}

	// 새메세지 드로잉
	Message = MessageString;
	DrawColor = bKilled ? KilledColor : KillerColor;
	LatestUpdateTime = UpdateTime;

	if( bHeadShot )				
	{
		if ( bWallShot )
			SetIcon(KILLMESSAGE_ICONTYPE_WALLHEADSHOT);	
		else
			SetIcon(KILLMESSAGE_ICONTYPE_HEADSHOT);
	}
	else if ( bWallShot )		SetIcon(KILLMESSAGE_ICONTYPE_WALLSHOT);
	else if ( bExplosive )		SetIcon(KILLMESSAGE_ICONTYPE_EXPLOSION);
	else						SetIcon(KILLMESSAGE_ICONTYPE_NORMAL);

	return 1;
}

//  [10/31/2006 otterrrr]

//  [11/1/2006 otterrrr]
void UavaUIGameIcon::Render_Widget( FCanvas* Canvas )
{
	if( GameIcons.Num() < 2 )
	{
		warnf(TEXT("insufficient GameIcon predefined. - add more GameIcon to the property 'GameIcons'"));
		return;
	}

	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>(PlayerOwner->Pawn) : NULL;
	AavaPlayerReplicationInfo* PRIOwner = PlayerOwner ? Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo) : NULL;

	if( PlayerOwner == NULL || PawnOwner == NULL || PRIOwner == NULL || PRIOwner->Team == NULL)
		return;

	INT IconIndex = -1;
	UBOOL bArmor = FALSE;
	UBOOL bHelmet = FALSE;
	UBOOL bUseHeartColor = FALSE;
	UBOOL bInDanger = FALSE;

	if( Binding == AVAUIGAMEICON_Team)
	{
		if( GameIcons.IsValidIndex(PRIOwner->Team->TeamIndex) )
			IconIndex = PRIOwner->Team->TeamIndex;
	}
	else if ( Binding == AVAUIGAMEICON_Armor )
	{
		bArmor = PawnOwner->Armor_Stomach > 0;
		if( GameIcons.IsValidIndex( bArmor ? 0 : 1 ))
			IconIndex = bArmor ? 0 : 1;
	}
	else if ( Binding == AVAUIGAMEICON_Helmet)
	{
		bHelmet = PawnOwner->bHasHelmet;
		if( GameIcons.IsValidIndex( bHelmet ? 0 : 1 ))
			IconIndex = bHelmet ? 0 : 1;
	}
	else if ( Binding == AVAUIGAMEICON_Armor_Heart )
	{
		// bHasArmor
		if( GameIcons.IsValidIndex(0) && GameIcons(0).Icon != NULL && PawnOwner->Armor_Stomach > 0 )
			IconIndex = 0;
		else 
		{
			bUseHeartColor = TRUE;
			bInDanger = ((FLOAT)PawnOwner->Health / PawnOwner->HealthMax ) < DangerRate;
			if(bInDanger)
			{
				INT OnePeriod = DangerHeartBeat[0]*1000.0f + DangerHeartBeat[1]*1000.0f;
				INT PosInPeriod = (INT)(GWorld->GetTimeSeconds() * 1000.0f) % OnePeriod;
				if( PosInPeriod < DangerHeartBeat[0]*1000.0f )
					IconIndex = 1;
				else
					IconIndex = 2;
			}
			else
			{
				INT OnePeriod = NormalHeartBeat[0]*1000.0f + NormalHeartBeat[1]*1000.0f;
				INT PosInPeriod = (INT)(GWorld->GetTimeSeconds() * 1000.0f) % OnePeriod;
				if( PosInPeriod < NormalHeartBeat[0]*1000.0f )
					IconIndex = 1;
				else
					IconIndex = 2;
			}
		}
	}

	FLinearColor NewDangerColor = (1 <= IconIndex && IconIndex <= 2) ? DangerColor[IconIndex-1] : DangerColor[0];

	if( IconIndex >= 0 && GameIcons.IsValidIndex(IconIndex))
	{
		FGameIconInfo& IconInfo = GameIcons(IconIndex);
		FLOAT U = IconInfo.bIgnoreCoord ?  0 : IconInfo.Coord.U;
		FLOAT V = IconInfo.bIgnoreCoord ?  0 : IconInfo.Coord.V;
		FLOAT UL = IconInfo.bIgnoreCoord ? IconInfo.Icon->GetSurfaceWidth() : IconInfo.Coord.UL;
		FLOAT VL = IconInfo.bIgnoreCoord ? IconInfo.Icon->GetSurfaceHeight() : IconInfo.Coord.VL;
		DrawTile(Canvas, GameIcons(IconIndex).Icon, RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top], RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top],
			U,V, UL, VL, bUseHeartColor ? (bInDanger ? NewDangerColor : NormalColor ) : IconInfo.Color);
	}
}
// [11/1/2006 otterrrr]

// [11/6/2006 otterrrr] Indicators ( Waypoint, Quickchat, MissionObject )
void UavaUIScreenIndicator::Render_Widget( FCanvas* Canvas )
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if( HUD == NULL )
		return;

	for(INT i = HUD->Indicators.Num() - 1; i >= 0 ; i-- )
	{
		const FIndicatorInfo& IndiInfo = HUD->Indicators(i);
		if( IndiInfo.Text == "" || IndicatorIcons.IsValidIndex(IndiInfo.Type) == FALSE || 
			IndicatorIcons(IndiInfo.Type).Icon.IsValidIndex(IndiInfo.ScreenArea) == FALSE ||
			IndicatorIcons(IndiInfo.Type).Icon(IndiInfo.ScreenArea) == NULL)
			continue;
		const FIndicatorIconInfo& IndiIcon = IndicatorIcons(IndiInfo.Type);

		INT nTimeLeft = 1000.0f*(IndiInfo.Life - GWorld->GetTimeSeconds());
		if( IndiInfo.bBlink )
		{
			INT Period = BlinkPeriod * 1000.0f;
			if( nTimeLeft >= 0 && (nTimeLeft % Period) > ( Period / 2 ))
				continue;
		}

		// Get String to Display
		FString Msg;
		if( bShowName )
		{
			switch( IndiInfo.Type)
			{
			case INDICATORTYPE_Waypoint1:
				Msg = WaypointName;
				break;
			case INDICATORTYPE_Waypoint2:
				Msg = WaypointName;
				break;
			case INDICATORTYPE_QuickChat_All:
				Msg = IndiInfo.Text;
				break;
			case INDICATORTYPE_QuickChat_Team:
				Msg = IndiInfo.Text;
				break;
			case INDICATORTYPE_MissionObject:
				Msg = MissionObjectName;
				break;
			default:
				break;
			}
		}

		if( bShowDistance )
			Msg += ( FString(TEXT("(")) + appItoa((UINT)(IndiInfo.Distance / 53.0f)) + TEXT("m)") );

		// Get an Icon
		USurface* Icon = IndiIcon.Icon(IndiInfo.ScreenArea);
		FTextureCoordinates Coord = IndiIcon.Coord(IndiInfo.ScreenArea);

		// Get DrawSize
		FLOAT Width = IndiIcon.bIgnoreExtent ? Coord.UL : IndiIcon.DrawExtent.X;
		FLOAT Height = IndiIcon.bIgnoreExtent ? Coord.VL : IndiIcon.DrawExtent.Y;

		// Get Position
		INT XL, YL;
		StringSize(Font, XL, YL, *Msg);
		FIntPoint PosStr(IndiInfo.pos.X, IndiInfo.pos.Y);
		FIntPoint Pos(IndiInfo.pos.X, IndiInfo.pos.Y);

		switch(IndiInfo.ScreenArea)
		{
		case ISA_Left:
			PosStr.X += Width;
			PosStr.Y += (-YL/2 + Height/2);
			break;
		case ISA_Right:		
			PosStr.X -= (XL + Width/2);
			PosStr.Y += (-YL/2 + Height/2);
			break;
		case ISA_Bottom: case ISA_Center: case ISA_None:
			PosStr.X += (-XL/2 + Width/2);
			PosStr.Y -= (YL);
			break;
		case ISA_Top:
			PosStr.X += (-XL/2 + Width/2);
			PosStr.Y += (YL/2 + Height);
			break;
		}

		// Draw String

		if( IndiInfo.ScreenArea != ISA_Center )
			DrawString(Canvas, PosStr.X, PosStr.Y, Font, *Msg, IndiIcon.TextColor);

		switch(IndiInfo.ScreenArea)
		{
		case ISA_Center:
		case ISA_Bottom:
			DrawTile(Canvas, Icon, Pos.X, Pos.Y, Width, Height, Coord.U, Coord.V, Coord.UL, Coord.VL, IndiIcon.IconColor);
			break;
		case ISA_Top:
			DrawTile(Canvas, Icon, Pos.X,Pos.Y, Width, Height, Coord.U, Coord.V, Coord.UL, Coord.VL, IndiIcon.IconColor);
			break;
		case ISA_Left:
			DrawTile(Canvas, Icon, Pos.X, Pos.Y, Width, Height, Coord.U, Coord.V, Coord.UL, Coord.VL, IndiIcon.IconColor);
			break;
		case ISA_Right:
			DrawTile(Canvas, Icon, Pos.X, Pos.Y, Width, Height, Coord.U, Coord.V, Coord.UL, Coord.VL, IndiIcon.IconColor);
			break;
		case ISA_None:
			break;
		}
	}
	for(INT i = HUD->Indicators.Num() - 1; i >= 0 ; i-- )
	{
		if( HUD->Indicators(i).Text == "" || HUD->Indicators(i).Life <= GWorld->GetTimeSeconds())
			HUD->Indicators.Remove(i,1);
	}
}

void UavaUIClassIcon::Render_Widget( FCanvas* Canvas )
{
	INT							nTeam = 0;
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	if ( PlayerOwner == NULL )	
		return;

	AavaPlayerReplicationInfo* PRIOwner;

	// 현재 병과 정보는 ViewTarget 에서, 다음 병과 정보는 Controller 를 통해서 알아온다...
	if ( bCurrentClass == TRUE )
	{
		AavaPawn* PawnOwner = Cast<AavaPawn>(PlayerOwner->ViewTarget);
		PRIOwner =	PawnOwner ? Cast<AavaPlayerReplicationInfo>( PawnOwner->PlayerReplicationInfo ) : NULL;
	}
	else
	{
		PRIOwner = Cast<AavaPlayerReplicationInfo>( PlayerOwner->PlayerReplicationInfo );
	}

	if ( PRIOwner == NULL )
		return;

	FLinearColor			LC;
	FTextureCoordinates		Coord;

	if ( PRIOwner )
	{
		nTeam = PRIOwner->Team ? PRIOwner->Team->TeamIndex : 0;
		if ( nTeam > 1 || nTeam < 0 )	nTeam = 0;

		if ( ( bCurrentClass == TRUE && PRIOwner->CurrentSpawnClassID == ClassIdx ) || (bCurrentClass == FALSE && PRIOwner->PlayerClassID == ClassIdx ) )
		{
			LC		= HighlightColor;
			Coord	= IconInfo[nTeam].HighlightCoord;
		}
		else
		{
			LC		=	NormalColor;
			Coord	=	IconInfo[nTeam].NormalCoord;
		}
		LC.A		*=	Opacity;
	}
	else
	{
		LC = NormalColor;
		Coord = IconInfo[nTeam].NormalCoord;
	}

	if ( IconInfo[nTeam].Image == NULL )	return;
	/// Alpha가 없는 경우도 그리고 있길래;; fixed by deif :)
	if (LC.A > 0)
		DrawTile( Canvas, IconInfo[nTeam].Image, 
				  RenderBounds[UIFACE_Left], 
				  RenderBounds[UIFACE_Top], 
				  RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left], 
				  RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top],
				  Coord.U, Coord.V, Coord.UL, Coord.VL, LC);
}

/** 2006/12/12 윤태식, SubClass를 찾아서 TArray로 반환. UclassIterator로부터 복사*/
static void FindSubClass(class UClass* BaseClass,TArray<class UClass*>& SubClassList)
{
	for( TObjectIterator<UClass> It ; It ; ++It )
		if( It->IsChildOf(BaseClass) && !(It->ClassFlags&CLASS_Abstract) )
			SubClassList.AddItem( *It );
}

void UavaUICrossHair::Initialize(UUIScene* inOwnerScene, UUIObject* inOwner/* =NULL  */)
{
	Super::Initialize(inOwnerScene, inOwner);
	for( TObjectIterator<UavaUITargetName> it ; it ; ++it )
		UITargetName = *it;
}

void UavaUICrossHair::Render_Widget( FCanvas* Canvas )
{
	AavaPlayerController*	PlayerOwner		= GetavaPlayerOwner();
	AavaPawn*				PawnOwner		= GetPlayerViewTarget();
	AavaHUD*				HUD				= PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
	AavaWeapon*				CurrentWeapon	= GetViewTargetCurrentWeapon();

	//AavaWeapon* CurrentWeapon;
	//
	if( PawnOwner == NULL || CurrentWeapon == NULL || HUD == NULL)
		return;

	// CrossHair 는 View Target 의 Controller 가 Local 일 경우에만 보여주도록 한다...
	if ( PlayerOwner->ViewTarget != NULL )
	{
		APawn* Pawn = Cast<APawn>(PlayerOwner->ViewTarget);
		if ( Pawn != NULL && !Pawn->IsLocallyControlled() )	
			return;
	}	

	if( LatestWeaponClass != CurrentWeapon->GetClass() )
	{
		LatestWeaponClass = CurrentWeapon->GetClass();
		INT FindIndex = -1;
		for(INT i = 0 ; i < CrossHairData.Num() ; i++ )
		{
			if( CrossHairData(i).WeaponClass == NULL )
				continue;

			if( CrossHairData(i).WeaponClass == CurrentWeapon->GetClass() )
			{
				FindIndex = i;
				break;
			}
			else if ( CurrentWeapon->IsA(CrossHairData(i).WeaponClass) )
				FindIndex = i;
		}
		CurrentCrossHairIndex = FindIndex;
	}

	// 대시중에는 그리지 않는다 
	if( PawnOwner->bIsDash )
		return;
	
	// Weapon 이 Active 상태가 아니라면 CrossHair 를 그리지 않는다... Why? 총을 쏠 수 없으니까...
	static FName WeaponActive( TEXT("Active") );
	static FName WeaponFiring( TEXT("WeaponFiring") );
	FStateFrame* StateFrame = CurrentWeapon->GetStateFrame();
	FName		 CurrentStateName;
	if (StateFrame && StateFrame->StateNode)
	{
		CurrentStateName = StateFrame->StateNode->GetFName();
		if ( CurrentStateName != WeaponActive && CurrentStateName != WeaponFiring && !CurrentWeapon->eventIsFiring() )
		{
			return;
		}
	}

	AavaWeap_BaseGun* BaseGun = Cast<AavaWeap_BaseGun>(CurrentWeapon);

	// SightMode이거나 FOV가 평상시의 FOV로 돌아오지 않았을때도 그리지 않는다.
	if (BaseGun != NULL && BaseGun->bHideCursorInSightMode && BaseGun->SightMode )
	{
		//if ( BaseGun->fov_current != BaseGun->fov_target )
		//	return;
		return;
	}
	static const FLOAT	RefViewportSizeY = 768.0f;
	FVector2D ViewportSize;
	UBOOL bGetViewportSizeResult = GetViewportSize(ViewportSize);

	// CrossHairSize를 계산한다. avaWeapon.uc 와 avaWeap_Gun.uc를 복사해 합친 모양
	FLOAT CrosshairSize		= BaseGun ? RefViewportSizeY * ( 0.5f * BaseGun->Inaccuracy / BaseGun->CrossHairSpeed) : 4.0f ;
	const FLOAT RefBarSize	= 3 + ( 2 * DefaultBarSize - 3 ) * Clamp( CrosshairSize / 20.0f, 0.0f, 1.0f );

	CrosshairSize = Max( 5, appFloor( CrosshairSize ) );
	CrosshairSize *= ViewportSize.Y/RefViewportSizeY;

	const FLOAT BarSize = RefBarSize * ViewportSize.Y/RefViewportSizeY;

	FCrossHairDataType& CrossHair = CrossHairData(CurrentCrossHairIndex);
	if( !CrossHairData.IsValidIndex(CurrentCrossHairIndex) || CrossHair.DisplayType == CROSSHAIRDISP_Default)
	{	
		INT CX = appFloor( 0.5 * ViewportSize.X );
		INT CY = appFloor( 0.5 * ViewportSize.Y );
		FLinearColor DrawColor = PawnOwner->NightvisionActivated ? CurrentWeapon->WeaponColorInNVG : CurrentWeapon->WeaponColor;		

		if (BaseGun)
		{
			FLinearColor HitColor = PawnOwner->NightvisionActivated ? FLinearColor( 1, 1, 1, 1 ) : FLinearColor( 1, 0, 0, 1 );

			FLOAT ElapsedTime = GWorld->GetTimeSeconds() - BaseGun->LastHitTime;

			const FLOAT Interval = BaseGun->FireInterval(0);
			const FLOAT Sustain = 1.0f * Interval;
			const FLOAT Release = 2.0f * Interval;
			const FLOAT ExtraSize = 3.0f;

			if ( ElapsedTime < (Sustain + Release) )
			{
				if ( !IsInSpawnProtection( BaseGun->LastHitPawn )  )
				{
					if ( ElapsedTime < Sustain )
					{
						DrawColor = HitColor; 
						CrosshairSize += ExtraSize * (1 - Square( ElapsedTime / Sustain ));
					}
					else
					{
						DrawColor = DrawColor + (HitColor - DrawColor) * Clamp(1.0f - (ElapsedTime - Sustain) / Release, 0.0f, 1.0f);
					}
				}
				else
				{
					if ( !PlayerOwner->eventIsSameTeam( BaseGun->LastHitPawn ) && InvincibleIcon.Texture != NULL )
					{
						DrawTile( Canvas, 
								  InvincibleIcon.Texture, 
								  CX - InvincibleIcon.DrawExtent.X/2.0f, 
								  CY - InvincibleIcon.DrawExtent.Y/2.0f,
								  InvincibleIcon.DrawExtent.X,
								  InvincibleIcon.DrawExtent.Y,
								  InvincibleIcon.TexCoord.U,
								  InvincibleIcon.TexCoord.V,
								  InvincibleIcon.TexCoord.UL,
								  InvincibleIcon.TexCoord.VL,
								  InvincibleIcon.DrawColor );
					}
				}
			}
		}

		DrawRect(Canvas, CX - CrosshairSize - BarSize + 1, CY, BarSize + 1 , 1, DefaultWhiteTexture,DrawColor);
		DrawRect(Canvas, CX + CrosshairSize - 1, CY, BarSize + 1, 1, DefaultWhiteTexture , DrawColor);
		DrawRect(Canvas, CX, CY - CrosshairSize - BarSize + 1, 1, BarSize + 1, DefaultWhiteTexture , DrawColor);
		DrawRect(Canvas, CX, CY + CrosshairSize, 1, BarSize + 1, DefaultWhiteTexture , DrawColor );
	}
	else if ( CrossHair.DisplayType != CROSSHAIRDISP_None)
	{
		FLOAT U = CrossHair.bIgnoreCoord ?  0 : CrossHair.TexCoord.U;
		FLOAT V = CrossHair.bIgnoreCoord ?  0 : CrossHair.TexCoord.V;
		FLOAT UL = CrossHair.bIgnoreCoord ? CrossHair.Texture->GetSurfaceWidth() : CrossHair.TexCoord.UL;
		FLOAT VL = CrossHair.bIgnoreCoord ? CrossHair.Texture->GetSurfaceHeight() : CrossHair.TexCoord.VL;
		FLOAT Width = CrossHair.bIgnoreExtent ? UL : CrossHair.DrawExtent.X;
		FLOAT Height = CrossHair.bIgnoreExtent ? VL : CrossHair.DrawExtent.Y;

		if( BaseGun != NULL && BaseGun->bEnableLaserSight && LaserSightMat != NULL)
		{
			FLOAT DrawImageLength;
			FVector2D RenderBoundSize( RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top] );
			FVector2D DrawStartPos;
			FLinearColor WhiteColor( 1.0f,1.0f,1.0f );
			if( bGetViewportSizeResult )
			{
				DrawImageLength = ViewportSize.X > ViewportSize.Y ? ViewportSize.Y : ViewportSize.X;
				DrawStartPos = ViewportSize.X > ViewportSize.Y ? FVector2D((ViewportSize.X - ViewportSize.Y) / 2, 0) : FVector2D(0, ViewportSize.Y - ViewportSize.X);
			}
			else
			{
				DrawImageLength = RenderBoundSize.X > RenderBoundSize.Y ? RenderBoundSize.Y : RenderBoundSize.X;
				DrawStartPos = RenderBoundSize.X > RenderBoundSize.Y ? FVector2D(RenderBoundSize.X - RenderBoundSize.Y,0) : FVector2D(0, RenderBoundSize.Y - RenderBoundSize.X);
			}
			DrawTile(Canvas, LaserSightMat, DrawStartPos.X, DrawStartPos.Y, DrawImageLength, DrawImageLength, 0,0, LaserSightMat->GetSurfaceWidth(), LaserSightMat->GetSurfaceHeight(), WhiteColor);
		}
		
		if( CrossHair.Texture != NULL)
		{
			FLinearColor DrawColor(1.0f,1.0f,1.0f,1.0f);
			// 적군일때만 크로스헤어 색을 바꾼다.
			if( UITargetName != NULL && UITargetName->TrackedTarget != NULL &&
				UITargetName->TrackedTarget->PlayerReplicationInfo != NULL && 
				UITargetName->TrackedTarget->PlayerReplicationInfo->Team != NULL &&
				PlayerOwner	!= NULL && PlayerOwner->PlayerReplicationInfo != NULL && 
				PlayerOwner->PlayerReplicationInfo->Team	!= NULL)
			{
				UBOOL bUseTargetAlpha = (UITargetName->TrackedTarget->PlayerReplicationInfo->Team->TeamIndex 
										!= PlayerOwner->PlayerReplicationInfo->Team->TeamIndex) && UITargetName->TrackedTargetAlpha > 0.9f;
				FLOAT TargetAlpha = bUseTargetAlpha ? 1.0f : 0.0f;
				DrawColor = DrawColor * (1.0f - TargetAlpha) + FLinearColor(TargetColor) * TargetAlpha;
			}
			if( CrossHair.DisplayType == CROSSHAIRDISP_Texture_Cross )
			{
				FLOAT DrawOffset = CrosshairSize * 0.707f;
				INT DrawRepeat = 4.0f;
				for( INT i = 0 ; i < DrawRepeat ; i++ )
					DrawTileRotate(Canvas, CrossHair.Texture, ViewportSize.X/2 , ViewportSize.Y/2, (INT)( 360.0f/ DrawRepeat ) * i,
					(INT)-(Width + DrawOffset),(INT) -(Height + DrawOffset), Width, Height, U, V, UL, VL, DrawColor);
				//DrawTile(Canvas, CrossHair.Texture, (INT)(ViewportSize.X/2 - Width - DrawOffset), (INT)(ViewportSize.Y/2 - Height - DrawOffset), Width , Height, U, V, UL, VL, DrawColor);
				//DrawTile(Canvas, CrossHair.Texture, (INT)(ViewportSize.X/2 + Width + DrawOffset), (INT)(ViewportSize.Y/2 - Height - DrawOffset), Width , Height, U + UL, V, -UL, VL, DrawColor);
				//DrawTile(Canvas, CrossHair.Texture, (INT)(ViewportSize.X/2 - Width - DrawOffset), (INT)(ViewportSize.Y/2 + Height + DrawOffset), Width , Height, U, V + VL, UL, -VL, DrawColor);
				//DrawTile(Canvas, CrossHair.Texture, (INT)(ViewportSize.X/2 + Width + DrawOffset), (INT)(ViewportSize.Y/2 + Height + DrawOffset), Width , Height, U +UL, V + VL, -UL, -VL, DrawColor);
			}
			else if ( CrossHair.DisplayType == CROSSHAIRDISP_Texture_Plus )
			{
				FLOAT DrawOffset = CrosshairSize;
				INT DrawRepeat = 4.0f;
				for( INT i = 0 ; i < DrawRepeat ; i++ )
					DrawTileRotate(Canvas, CrossHair.Texture, ViewportSize.X/2, ViewportSize.Y/2, (INT)(360.0f/DrawRepeat) * i,
					(INT)-(Width + DrawOffset),(INT) -(Height/2), Width, Height, U, V, UL, VL, DrawColor);
			}
			else if ( CrossHair.DisplayType == CROSSHAIRDISP_Texture_Once )
			{
				DrawTile(Canvas, CrossHair.Texture, (ViewportSize.X - Width) / 2, (ViewportSize.Y - Height) / 2, Width, Height, U, V, UL, VL, DrawColor);
			}

			// 크로스헤어를 다 그렸으나 크로스헤어 외에 중심점이 필요하다고 한다.
			if( CrossHair.DisplayType == CROSSHAIRDISP_Texture_Plus || CrossHair.DisplayType == CROSSHAIRDISP_Texture_Cross )
			{
				DrawRect(Canvas, ViewportSize.X/2 - 2, ViewportSize.Y/2 - 2, 4,4, DefaultWhiteTexture, DrawColor.Black );
				DrawRect(Canvas, ViewportSize.X/2 - 1, ViewportSize.Y/2 - 1, 2,2, DefaultWhiteTexture, DrawColor);
			}
		}
	}
}

UBOOL UavaUITouchWeapon::UpdateString( )
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>(PlayerOwner->Pawn) : NULL;
	AavaHUD* HUD		= PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if( PawnOwner == NULL || PawnOwner->CurrentWeapon == NULL || Font == NULL)
	{
		LatestTouchWeapon = NULL;
		if( Message.Len() == 0 )
			return FALSE;
		else
		{
			Message = TEXT("");
			return TRUE;
		}
	}

	UClass* CurrentTouchWeapon = NULL;
	FString CurrentTouchWeaponName;
	FString	SwapWeaponName;					// Swap 할 Weapon Name

	if ( PawnOwner->TouchedPP != NULL)
	{
		BYTE WeaponGroup = PawnOwner->TouchedPP->InventoryClass->GetDefaultObject<AavaWeapon>()->InventoryGroup;
		for (AInventory* Inv = PawnOwner->InvManager->InventoryChain; Inv!=NULL; Inv=Inv->Inventory)
		{
			AavaWeapon* W = Cast<AavaWeapon>(Inv);
			if ( W == NULL )	continue;

			if ( W->InventoryGroup == WeaponGroup && W->bCanThrow )
			{
				CurrentTouchWeapon		= PawnOwner->TouchedPP->InventoryClass;
				CurrentTouchWeaponName	= PawnOwner->TouchedPP->InventoryClass ? PawnOwner->TouchedPP->InventoryClass->GetDefaultObject<AavaWeapon>()->ItemName : FString(TEXT("(Unknown)"));
				SwapWeaponName			= W->ItemName;
				break;
			}
			if ( CurrentTouchWeapon != NULL )
				break;
		}
	}

	
	if ( CurrentTouchWeapon == NULL )
	{
		for(INT i = 0 ; i < PawnOwner->TouchedPickUp.Num() ; i++)
		{
			if( PawnOwner->TouchedPickUp(i) == NULL || PawnOwner->TouchedPickUp(i)->InventoryClass == NULL )
				continue;

			const AavaPickup& Pickup = *PawnOwner->TouchedPickUp(i);
			BYTE WeaponGroup = Pickup.InventoryClass->GetDefaultObject<AavaWeapon>()->InventoryGroup;

			for (AInventory* Inv = PawnOwner->InvManager->InventoryChain; Inv!=NULL; Inv=Inv->Inventory)
			{
				AavaWeapon* W = Cast<AavaWeapon>(Inv);
				if ( W == NULL )	continue;

				if ( W->InventoryGroup == WeaponGroup && W->bCanThrow )
				{
					CurrentTouchWeapon		= Pickup.InventoryClass;
					CurrentTouchWeaponName	= Pickup.InventoryClass ? Pickup.InventoryClass->GetDefaultObject<AavaWeapon>()->ItemName : FString(TEXT("(Unknown)"));
					SwapWeaponName			= W->ItemName;
					break;
				}

				if ( CurrentTouchWeapon != NULL )
					break;
			}		
		}
	}

	if( CurrentTouchWeapon == LatestTouchWeapon )
		return FALSE;

	if( CurrentTouchWeapon == NULL )
		Message = TEXT("");
	else if ( HUD != NULL )
	{
		// 아이콘이 있을때 찍어야할 크기와 아이콘텍스쳐좌표를 구함.
		FVector2D DrawExtent(0,0);
		FTextureCoordinates DrawCoord(0,0,0,0);
		if( KeyIcon )
		{
			DrawExtent = bIgnoreExtent ? FVector2D(KeyIcon->GetSurfaceWidth(), KeyIcon->GetSurfaceHeight()) : KeyIconExtent;
			DrawCoord = bIgnoreCoord ? FTextureCoordinates(0,0,KeyIcon->GetSurfaceWidth(), KeyIcon->GetSurfaceHeight()) : KeyIconCoord;
		}

		// 새로운 TouchWeapon. 찍을 메세지를 갱신해준다.
		FString TouchWeaponMessage;
		TouchWeaponMessage = SwapWeaponName + 
			Localize( *this->GetClass()->GetName(), TEXT("TouchWeaponMessage[0]"), TEXT("avaGame")) + 
			CurrentTouchWeaponName + 
			Localize( *this->GetClass()->GetName(), TEXT("TouchWeaponMessage[1]"), TEXT("avaGame"));

		// 중간에 들어갈 아이콘 처리 (여기서 미리 Icon의 Offset과 Extent를 저장해두었다가 Render_Widget에서 사용)
		INT XL, YL;
		//StringSize(Font, XL, YL, *TouchWeaponMessage);
		//IconOffset = FVector2D(XL, YL);
		//IconExtent = DrawExtent;
		//IconCoord = DrawCoord;
		//StringSize(Font, XL, YL, TEXT(" "));
		//INT IconSpace = DrawExtent.X / XL + 1;

		//for(INT i = 0 ; i < IconSpace ; i ++)
		//	TouchWeaponMessage += FString(TEXT(" "));
		TouchWeaponMessage += HUD->SwapKeyName;

		TouchWeaponMessage += Localize( *this->GetClass()->GetName(), TEXT("TouchWeaponMessage[2]"), TEXT("avaGame"));
		StringSize(Font, XL, YL, *TouchWeaponMessage);
		StringExtent = FVector2D(XL,YL);

		Message = TouchWeaponMessage;
	}
	LatestTouchWeapon = CurrentTouchWeapon;
	return TRUE;
}

void UavaUITouchWeapon::Render_Widget( FCanvas *Canvas )
{
	// 먼저 텍스트를 드로잉한다.
	Super::Render_Widget( Canvas );

	if( LatestTouchWeapon == NULL || KeyIcon == NULL || Message.Len() == 0)
		return;

	FVector2D StartPos;
	// 그다음 텍스트의 빈칸에 Icon을 드로잉한다.
	CalculatePosition(StartPos.X, StartPos.Y, StringExtent.X, StringExtent.Y);
	DrawTile(Canvas, KeyIcon, StartPos.X + IconOffset.X, StartPos.Y + (StringExtent.Y - IconExtent.Y)/2, IconExtent.X, IconExtent.Y, IconCoord.U, IconCoord.V, IconCoord.UL, IconCoord.VL, DrawColor);

	// 필요하다면 해당 무기의 아이콘을 그려준다.
	if( bShowWeaponIcon && WeaponIconFont != NULL)
	{
		FRenderParameters StrParam;
		UClass* AttachmentClass = LatestTouchWeapon->GetDefaultObject<AavaWeapon>()->AttachmentClass;
		FString WeaponStr = AttachmentClass? AttachmentClass->GetDefaultObject<AavaWeaponAttachment>()->DeathIconStr : FString(TEXT(""));
		StrParam.DrawFont = WeaponIconFont;
		UUIString::StringSize(StrParam, *WeaponStr);
		StrParam.DrawX = StartPos.X + StringExtent.X/2 - StrParam.DrawXL/2 + WeaponIconOffset.X;
		StrParam.DrawY = StartPos.Y - StrParam.DrawYL + WeaponIconOffset.Y;
		if( bDropShadow )
		{
			FLinearColor NewShadowColor = AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor;
			NewShadowColor.A *= Opacity;
			DrawString( Canvas, StrParam.DrawX + 1, StrParam.DrawY + 1, *WeaponStr, WeaponIconFont, NewShadowColor );
		}
		DrawString(Canvas, StrParam.DrawX, StrParam.DrawY, *WeaponStr, WeaponIconFont, WeaponIconColor);
	}
}

UBOOL UavaUICurrentWeapon::UpdateString()
{
	if( GIsEditor && !GIsGame )
	{
		if( TestString != Message )
		{
			Message = TestString;
			return TRUE;
		}
		else
			return FALSE;
	}
	else if ( GIsGame )
	{
		AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
		AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>(PlayerOwner->ViewTarget) : NULL;

		if( PawnOwner == NULL || PawnOwner->CurrentWeapon == NULL)
		{
			if( Message.Len() == 0 )
				return FALSE;
			else
			{
				Message = TEXT("");
				return TRUE;
			}
		}

		AavaWeapon* CurrentWeapon = Cast<AavaWeapon>(PawnOwner->CurrentWeapon);
		if( CurrentWeapon == LatestWeapon )
			return FALSE;

		Message = CurrentWeapon && CurrentWeapon->AttachmentClass ? 
			FString(CurrentWeapon->AttachmentClass->GetDefaultObject<AavaWeaponAttachment>()->DeathIconStr) : FString(TEXT(""));
		LatestWeapon = CurrentWeapon;
		return TRUE;
	}

	return FALSE;
}

void UavaUISimpleProgress::Render_Widget( FCanvas* Canvas )
{
	UpdateProgress();

	FLOAT DeltaTime = GWorld->GetTimeSeconds() - LatestRenderTime;
	LatestRenderTime = GWorld->GetTimeSeconds();
	
	EUISimpleProgressDirectionType Dir;
	if( IsMoving(Dir) )
		CurrentPos -= ((CurrentPos - targetPos) * Attenuation * DeltaTime);
	else
		CurrentPos = targetPos;

	CurrentPos = Clamp(CurrentPos, 0.0f, MaxElement);

	INT nMaxElement = MaxElement;
	INT nCurrentPos = CurrentPos;
	FLOAT DrawX = RenderBounds[UIFACE_Left];
	FLOAT DrawY = RenderBounds[UIFACE_Top];
	FLOAT DrawXL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT DrawYL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];	

	if( ProgressType == SIMPLE_PROGRESS_TYPE_STRING )
	{
		for(INT i = 0 ; i < nMaxElement ; i++ )
		{
			DrawString(Canvas, DrawX, DrawY, DrawFont, *StringElement, DrawColor );
		}
	}
	else  if ( ProgressType == SIMPLE_PROGRESS_TYPE_BOX )
	{
		FLOAT ScaledProgress = Clamp(CurrentPos/MaxElement, 0.0f, 1.0f);
		FLOAT UnitDist = 1.0f/(MaxElement);
		FLOAT Dist = UnitDist * 0.5f;
		FLOAT ClampedBoxSize= Clamp(BoxSize, 1.0f, UnitDist * DrawXL);
		while(Dist < ScaledProgress)
		{
			DrawRect(Canvas, DrawX + DrawXL * Dist - ClampedBoxSize/2, DrawY, ClampedBoxSize , DrawYL, DefaultWhiteTexture, DrawColor);
			Dist += UnitDist;
		}
	}
	else if ( ProgressType == SIMPLE_PROGRESS_TYPE_FLOW )
	{
		INT ScaledProgress = Clamp(CurrentPos/MaxElement, 0.0f, 1.0f);
		DrawRect( Canvas, DrawX , DrawY, DrawXL * ScaledProgress ,DrawYL, DefaultWhiteTexture, DrawColor);
	}
}

void UavaUISimpleProgress::SetTargetPos( FLOAT NewTargetPos )
{
	targetPos = NewTargetPos;
}

UBOOL UavaUISimpleProgress::IsMoving( EUISimpleProgressDirectionType& Dir)
{
	Dir = (fabsf(targetPos - CurrentPos) < KINDA_SMALL_NUMBER) ? SIMPLE_PROGRESS_DIR_NO_DIRECTION : 
		targetPos > CurrentPos ? SIMPLE_PROGRESS_DIR_INCREASE : SIMPLE_PROGRESS_DIR_DECREASE;
	return Dir != SIMPLE_PROGRESS_DIR_NO_DIRECTION;
}

void UavaUIGameGauge::UpdateProgress()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>(PlayerOwner->Pawn) : NULL;
	FLinearColor NewColor;

	if( PawnOwner == NULL )
		return;

	if( UIGameGaugeType == UI_GAME_GAUGE_PlayerHealth )
	{
		FLOAT HealthRate = fabsf(PawnOwner->HealthMax) < KINDA_SMALL_NUMBER ? 0.0f : ((FLOAT)PawnOwner->Health/PawnOwner->HealthMax);
		SetTargetPos(HealthRate * MaxElement);
		NewColor = HealthRate < Player_DangerRate ? Player_DangerColor : Player_NormalColor;
	}
	else if ( UIGameGaugeType == UI_GAME_GAUGE_ArmorHealth)
	{
		FLOAT HealthRate = fabsf(PawnOwner->ArmorMax) < KINDA_SMALL_NUMBER ? 0.0f : ((FLOAT)PawnOwner->Armor_Stomach/ PawnOwner->ArmorMax);
		SetTargetPos(HealthRate * MaxElement);
		NewColor = Armor_NormalColor;
	}

	DrawColor = NewColor;
	EUISimpleProgressDirectionType Dir;
	if( IsMoving(Dir) && Dir == SIMPLE_PROGRESS_DIR_DECREASE )
	{
		INT OnePeriod = ActiveTime[0]*1000.0f + ActiveTime[1]*1000.0f;
		INT PosInPeriod = (INT)(GWorld->GetTimeSeconds() * 1000.0f) % OnePeriod;
		DrawColor = PosInPeriod < (INT)(ActiveTime[0]*1000.0f) ? DrawColor : ActiveColor;
	}
}

void UavaUIGamePanel::Render_Widget( FCanvas* Canvas )
{
	if( GIsEditor && !GIsGame )
	{
		Super::Render_Widget(Canvas);
		return;
	}

	INT TeamIndex = 2;
	AavaPlayerReplicationInfo* ViewtargetPRI = GetPlayerViewTargetPRI();
	if ( ViewtargetPRI != NULL && ViewtargetPRI->Team != NULL)
	{
		TeamIndex = ViewtargetPRI->Team->TeamIndex;
	}

	// EU : 0 , NRF : 1 , Another : BadTeamIndex or Unknown
	if ( BackgroundImageComponent == NULL || BackgroundImageComponent->ImageRef == NULL)
		return;

	FUICombinedStyleData ImageStyle = BackgroundImageComponent->ImageRef->GetImageStyle();
	FLinearColor NewDrawColor = ImageStyle.ImageColor;

	if( BindingType == GAMEPANEL_TeamColor)
	{
		if( PanelColor.IsValidIndex(TeamIndex) )
		{
			NewDrawColor = PanelColor(TeamIndex);
			if ( NewDrawColor.A == 0.0 )
				return;
		}
		else return;

		if( ImageStyle.ImageColor != NewDrawColor)
		{
			ImageStyle.ImageColor = NewDrawColor;
			BackgroundImageComponent->ImageRef->SetImageStyle( ImageStyle );
		}
	}
	// further more ...

	Super::Render_Widget( Canvas );
}
void UavaUIFullScreenMap::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);
	for(TObjectIterator<UavaUIRadar> it ; it ; ++it)
		UIRadar = *it;	

	//if ( UIRadar != NULL )
	//{
	//	int i;
	//	for ( i = 0 ; i < ClassCodes.Num() ; ++ i )
	//	{
	//		if ( ClassCodes(i) == -1 && UIRadar->ClassCodes.IsValidIndex(i) )
	//			ClassCodes(i) = UIRadar->ClassCodes(i);
	//	}

	//	for ( i = 0 ; i < FriendCodes.Num() ; ++ i )
	//	{
	//		if ( FriendCodes(i) == -1 && UIRadar->FriendCodes.IsValidIndex(i) )
	//			FriendCodes(i) = UIRadar->FriendCodes(i);
	//	}

	//	for ( i = 0 ; i < EnemyCodes.Num() ; ++ i )
	//	{
	//		if ( EnemyCodes(i) == -1 && UIRadar->EnemyCodes.IsValidIndex(i) )
	//			EnemyCodes(i) = UIRadar->EnemyCodes(i);
	//	}

	//	for ( i = 0 ; i < FriendDamagedCodes.Num() ; ++ i )
	//	{
	//		if ( FriendDamagedCodes(i) == -1 && UIRadar->FriendDamagedCodes.IsValidIndex(i) )
	//			FriendDamagedCodes(i) = UIRadar->FriendDamagedCodes(i);
	//	}

	//	for ( i = 0 ; i < EnemyDamagedCodes.Num() ; ++ i )
	//	{
	//		if ( EnemyDamagedCodes(i) == -1 && UIRadar->EnemyDamagedCodes.IsValidIndex(i) )
	//			EnemyDamagedCodes(i) = UIRadar->EnemyDamagedCodes(i);
	//	}

	//	if ( FriendDeadCode == -1 )		FriendDeadCode = UIRadar->FriendDeadCode;
	//	if ( EnemyDeadCode == -1 )		EnemyDeadCode = UIRadar->EnemyDeadCode;
	//	if ( LocalPlayerCode == -1 )	LocalPlayerCode = UIRadar->LocalPlayerCode;
	//	if ( QuickChatCode == -1 )		QuickChatCode = UIRadar->QuickChatCode;
	//}
}

void UavaUIFullScreenMap::Render_Widget( FCanvas *Canvas )
{
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	if (WorldInfo )
	{
		if ( !bUseLargeMap )
		{
			if ( LatestMapTexture != WorldInfo->MinimapTexture )
				LatestMapTexture = WorldInfo->MinimapTexture;
			
		}
		else
		{
			if ( WorldInfo->LargemapTexture != NULL && LatestMapTexture != WorldInfo->LargemapTexture )
				LatestMapTexture = WorldInfo->LargemapTexture;
			else
				LatestMapTexture = WorldInfo->LargemapMaterial;
		}
	}

	if( LatestMapTexture == NULL)
		return;	


	FVector2D RenderBoundSize( RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left] , RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top]);

	FVector TransVec( RenderBounds[UIFACE_Left] + RenderBoundSize.X * 0.5f, RenderBounds[UIFACE_Top] + RenderBoundSize.Y * 0.5f, 0.f );
	FMatrix RotTransMat = FTranslationMatrix(-TransVec) * FRotationMatrix(FRotator(0.f, MapRotationDegree * 65535.f / 360.f , 0.f)) * FTranslationMatrix(TransVec);
	Canvas->PushRelativeTransform( RotTransMat );

	FVector2D DrawExtent;
	FTextureCoordinates FullMapCoord, MapCoord, DrawCoord;
	FLinearColor DrawColor(1.0f,1.0f,1.0f);

	FullMapCoord = FTextureCoordinates(0,0,LatestMapTexture->GetSurfaceWidth(), LatestMapTexture->GetSurfaceHeight());
	MapCoord = bOverrideMapTexCoord ? MapTexCoord :FullMapCoord;

	if( GIsEditor && !GIsGame )
	{
		DrawColor = FLinearColor (1.0f,1.0f,1.0f,0.2f);
		DrawTile(Canvas, LatestMapTexture, RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top], RenderBoundSize.X, RenderBoundSize.Y, MapCoord.U, MapCoord.V, MapCoord.UL, MapCoord.VL, DrawColor );
		return;
	}

	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>(PlayerOwner->Pawn) : NULL;
	AavaPlayerReplicationInfo* PRIOwner = PlayerOwner ? Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo) : NULL;
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if( PlayerOwner == NULL || PRIOwner == NULL || HUD == NULL )
		return;

	if( UIRadar == NULL )
	{
		warnf(TEXT("There's no UIRadar for referencing (%s)"), *GetName());
		return;
	}

	if( !bShowAlways && !HUD->bShowFullScreenMap ) // Radar에서 모든 아이콘 정보를 얻어오므로 반드시 필요
		return;

	// FullScreenMap을 먼저 그린다.
	DrawTile(Canvas, LatestMapTexture, RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top], RenderBoundSize.X , RenderBoundSize.Y, MapCoord.U, MapCoord.V, MapCoord.UL, MapCoord.VL , DrawColor );

	INT OwnerTeam = GetTeamNum( PlayerOwner );
	ULocalPlayer* LocalPlayer = GetLocalPlayer();

	// RadarDisplayActor( MissionNavigationPoint, Bomb 등등 )
	if( bShowRadarActor )
	{
		for ( INT i = 0 ; i < HUD->RadarDisplayedActors.Num() ; ++i )
		{
			const FRadarDisplayedActor& rda = HUD->RadarDisplayedActors(i);
			if ( ( rda.TeamIndex == 0 || rda.TeamIndex == 1 ) && !PlayerOwner->eventIsSameTeamByIndex( rda.TeamIndex ) )
				continue;

			// Leader 에게만 보이는 Icon 이다....
			if ( rda.bOnlyLeader && PRIOwner != NULL && PRIOwner->bSquadLeader == FALSE )
				continue;

			if ( HUD->targetActor == rda.DisplayedActor )
				continue;

			AavaPawn* Pawn = Cast<AavaPawn>(rda.DisplayedActor);
			if( Pawn != NULL && Pawn->SpecialInventory != NULL )
				continue;

			INT IconCode = rda.IconCode;
			Render_Object( Canvas, 
				HUD, 
				rda.DisplayedActor->Location, 
				IconCode,
				1.0f,
				FColor(255,255,255,255),
				FALSE,
				rda.bAdjustRotate,
				RotateDegAngle + ( rda.DisplayedActor->Rotation.Yaw ) * 360.0f/65535.0f );
		}
	}

	// 아군 적군을 맵에 표시
	for( FDynamicActorIterator it ; it ; ++it )
	{
		AavaPawn* Pawn = Cast<AavaPawn>(*it);
		AavaPlayerReplicationInfo* PRI = Pawn ? Cast<AavaPlayerReplicationInfo>(Pawn->PlayerReplicationInfo) : NULL;
		if( Pawn == NULL || Pawn->Health <= 0 || Pawn->bDeleteMe || PRI == NULL )
			continue;

		INT TeamIndex = PRI->Team != NULL ? PRI->Team->TeamIndex : 255;
		UBOOL bDamaged = Pawn->LastPainTime > 0.0f && (Pawn->LastPainTime + UIRadar->DamageDisplayTime) > GWorld->GetTimeSeconds();
		FLOAT DisplayElapsed = (GWorld->GetTimeSeconds() - Pawn->LastPainTime);
		FLOAT Period = UIRadar->BlinkPeriod;	// second
		FLOAT CurrentPos = ((INT)(DisplayElapsed * 1000.0f) % (INT)(Period*1000.0f)) / 1000.0f;	//milli second
		UBOOL bShowDamaged = bDamaged && (Period * 0.5f > CurrentPos);

		// 퀵챗을 추가로 찍어줘야할지 조사
		UBOOL bRaiseQuickChat = Pawn->QuickChatUpdateTime > 0.0f && (Pawn->QuickChatUpdateTime + UIRadar->QuickChatDisplayTime) > GWorld->GetTimeSeconds();

		if( PawnOwner != NULL && PawnOwner == Pawn)
		{
			if( bShowLocalPlayer )
			{
				FVector ScreenLocation = Pawn->Location;
				CalcPosInMap( ScreenLocation, RotateDegAngle );
				FRotator ViewRotator = PawnOwner ? PawnOwner->Rotation : FRotator(0,0,0);
				FLOAT DegAngle = ViewRotator.Yaw * (360.0f / 65535.0f) + RotateDegAngle;

				const FIconCodeInfo* IconInfo = GetIconInfo( LocalPlayerCode );
				if( IconInfo != NULL && IconInfo->Image != NULL)
				{
					FVector Scaler = InterpLocalPlayerIconScaler;
					FTextureCoordinates DrawCoord = IconInfo->Coord;
					FVector2D DrawExt( DrawCoord.UL * Scaler.X, DrawCoord.VL * Scaler.Y );
					FVector2D DrawXY = -FVector2D( DrawExt.X * 0.5f + VisibleRegionRotateAxis.X * Scaler.X, DrawExt.Y * 0.5f + VisibleRegionRotateAxis.Y * Scaler.Y );
					DrawTileRotate( Canvas, IconInfo->Image, (INT)ScreenLocation.X, (INT)ScreenLocation.Y, DegAngle,
						DrawXY.X, DrawXY.Y, DrawExt.X, DrawExt.Y, DrawCoord.U, DrawCoord.V, DrawCoord.UL, DrawCoord.VL, 
						IconInfo ? IconInfo->Color : FLinearColor::White);

					if( Pawn->SpecialInventory != NULL )
					{
						DrawStringCentered(Canvas, ScreenLocation.X + SpecialInvNameOffset.X ,ScreenLocation.Y + SpecialInvNameOffset.Y, *Pawn->SpecialInventory->ItemShortName, Font, SpecialInvNameColor);
					}
				}

				if ( Pawn->bTargetted && Pawn->bDrawNotifyTargetted )
				{
					FLOAT Wave			=	sinf( ( GWorld->GetTimeSeconds() - Pawn->TargettedTime ) * 2 * 3.141592f / TargettedWaveDuration ) + 1.0f;
					FLOAT TargetScale	=	TargettedMinScale + ( TargettedMaxScale - TargettedMinScale ) * Wave / 2.0f;
					Render_Object( Canvas, HUD, Pawn->Location, TargettedCode, 1.0f, FLinearColor(1,1,1), FALSE, TRUE, DegAngle,  FVector(TargetScale,TargetScale,TargetScale) );
				}
			}
		}
		else 
		{
			if( PlayerOwner->eventIsSameTeamByIndex( TeamIndex ) )
			{
				if ( !bDrawOnlySlotNum )
				{
					if( bShowFriendPlayer)
					{
						FVector ScreenLocation = Pawn->Location;
						CalcPosInMap( ScreenLocation, RotateDegAngle );

						const BOOL bIsBIA = Pawn->eventIsBIA();
						// 전우...
						FVector Scaler = bIsBIA ? InterpBIAIconScaler : FVector(1.0f,1.0f,0.0f);
						//FVector Scaler = bIsBIA ? InterpLocalPlayerIconScaler : FVector(1.0f,1.0f,0.0f);
						if( bShowFriendPlayerName )
						{
							DrawStringCentered(Canvas, ScreenLocation.X + FriendPlayerNameOffset.X ,ScreenLocation.Y + FriendPlayerNameOffset.Y, *PRI->PlayerName, Font, FriendPlayerNameColor);
						}
						//FLOAT HealthRate = fabsf( Pawn->HealthMax ) < KINDA_SMALL_NUMBER ? 0.0f : ((FLOAT)Pawn->Health / Pawn->HealthMax);
						//FLOAT Alpha = bDamaged ? HealthRate * appCos(2*PI*CurrentPos/Period) * 2.0f : HealthRate * 2.0f;
						INT IconDataIndex = (0 <= PRI->CurrentSpawnClassID && PRI->CurrentSpawnClassID <= 2) ? PRI->CurrentSpawnClassID : -1;
						INT IconCode = FriendCodes.IsValidIndex(IconDataIndex) ? FriendCodes(IconDataIndex) : -1;
						INT IconCodeDamaged = FriendDamagedCodes.IsValidIndex(IconDataIndex) ? FriendDamagedCodes(IconDataIndex) : -1;
						if( bShowDamaged )
							Render_Object( Canvas, HUD, Pawn->Location, IconCodeDamaged, 1.0f, HUD->BIAColor, bIsBIA, FALSE, 0, Scaler );

						//	Alpha = 1.0f ,FColor Color = FColor(255,255,255,255), UBOOL bOverrideColor = FALSE, UBOOL bAdjustRotator = FALSE, FLOAT RotateDegree = 0
						Render_Object( Canvas, HUD, Pawn->Location, IconCode, 1.0f, HUD->BIAColor, bIsBIA, FALSE, 0, Scaler );
						if( Pawn->SpecialInventory != NULL )
						{
							DrawStringCentered(Canvas, ScreenLocation.X + SpecialInvNameOffset.X ,ScreenLocation.Y + SpecialInvNameOffset.Y, *Pawn->SpecialInventory->ItemShortName, Font, SpecialInvNameColor);
						}

						if ( bShowQuickChat && bRaiseQuickChat )
						{
							FLOAT Elapsed = GWorld->GetTimeSeconds() - Pawn->QuickChatUpdateTime;
							FLOAT Current = ((INT)(Elapsed*1000.0f) % (INT)(UIRadar->BlinkPeriod * 1000.0f)) / 1000.0f;
							UBOOL bDrawQuickChat = Current < UIRadar->BlinkPeriod * 0.5f;
							if( bDrawQuickChat )
								Render_Object(Canvas, HUD, Pawn->Location, QuickChatCode, 1.0f, HUD->BIAColor, bIsBIA, FALSE, 0, Scaler );
						}
					}
				}
				else
				{
					FVector Location = Pawn->Location;
					if( CalcPosInMap( Location , 0 ) )
						DrawSlotIcon( Canvas, TeamIndex, PRI->SlotNum, Location.X, Location.Y, SlotNumSize.X, SlotNumSize.Y, 1.0, 1, 1 );
				}
			}
			else
			{
				if ( !bDrawOnlySlotNum )
				{
					if( ( bShowEnemyPlayer && Pawn->bLastTakeHitVisibility && (bDamaged) ) || Pawn->bTargetted || ( PawnOwner != NULL && IsUsingBinocular( PawnOwner ) && Pawn->bDetected ) )
					{
						//FLOAT Alpha = Clamp(1.0f - (DisplayElapsed)/UIRadar->DamageDisplayTime,0.0f,1.0f) * Clamp((appCos(2*PI*CurrentPos/Period) * 2.0f),0.0f,1.0f);
						INT IconDataIndex = (0 <= PRI->CurrentSpawnClassID && PRI->CurrentSpawnClassID <= 2) ? PRI->CurrentSpawnClassID : -1;
						INT IconCode = EnemyCodes.IsValidIndex(IconDataIndex) ? EnemyCodes(IconDataIndex) : -1;
						INT IconCodeDamaged = EnemyDamagedCodes.IsValidIndex(IconDataIndex) ? EnemyDamagedCodes(IconDataIndex) : -1;
						if( bShowDamaged )
							Render_Object( Canvas, HUD, Pawn->Location,  IconCodeDamaged);
						Render_Object( Canvas, HUD, Pawn->Location, IconCode);
					}
				}
				else
				{
					FVector Location = Pawn->Location;
					if( CalcPosInMap( Location , 0 ) )
						DrawSlotIcon( Canvas, TeamIndex, PRI->SlotNum, Location.X, Location.Y, SlotNumSize.X, SlotNumSize.Y, 1.0, 1, 1 );
				}
			}
		}
	}

	// DeadPlayer
	if( bShowDeadPlayer )
	{
		for( INT i = 0 ; i < HUD->DeadPlayerList.Num() ; i++)
		{
			const FDeadPlayerInfo& DeadInfo = HUD->DeadPlayerList(i);
			if( DeadInfo.PRI == NULL )
				continue;

			UBOOL bFriend = PlayerOwner->eventIsSameTeamByIndex( DeadInfo.PRI->Team != NULL ? DeadInfo.PRI->Team->TeamIndex : 255 );
			FLOAT Alpha = UIRadar->bOverrideReinforcementTime ? 1.0f -  (GWorld->GetTimeSeconds() - DeadInfo.UpdateTime) / UIRadar->ReinforcementTime : 1.0f;
			Render_Object(Canvas, HUD, DeadInfo.Location, bFriend ? FriendDeadCode : EnemyDeadCode, Alpha * 2.0f );
		}
	}

	if ( HUD != NULL )
	{
		if ( HUD->bSignalActivate == TRUE )
			Render_Object( Canvas, HUD, HUD->SignalPos, HUD->SignalIcon);

		if ( HUD->targetActor != NULL )
			Render_Object( Canvas, HUD, HUD->targetActor->Location, HUD->RadarTopTargetIcon );
	}
	
	Canvas->PopTransform();
}


UBOOL UavaUIFullScreenMap::CalcPosInMap( FVector& WorldToScreen, const FLOAT RotDegAngle)
{
	AWorldInfo* WorldInfo = GWorld ? GWorld->GetWorldInfo() : NULL;
	check(WorldInfo);

	const FLOAT UUToMeter = 1.0f / 53.0f;	// 1 meter = 53 uu
	const FLOAT	MeterPerTexel = WorldInfo && WorldInfo->MinimapMeterPerTexel > 0.f ? WorldInfo->MinimapMeterPerTexel *  512.0f / LatestMapTexture->GetSurfaceWidth() :
								GEngine->MeterPerTexel > 0.0f ? GEngine->MeterPerTexel : 1.0f;	// meter per texel
	FLOAT Scale = UUToMeter / (ZoomScale * MeterPerTexel);
	FVector2D RenderBoundSize(RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top]);
	FTextureCoordinates FullMapCoord(0,0,LatestMapTexture->GetSurfaceWidth(), LatestMapTexture->GetSurfaceHeight());
	FTextureCoordinates SelectedMapCoord = bOverrideMapTexCoord ? MapTexCoord :FullMapCoord;

	// 여기서 부터 Texel 단위 계산
	FVector ScreenLocation = WorldToScreen - WorldInfo->MinimapOffset;

	// 맵의 중심으로부터 RotDegAngle만큼 돌린다.(약간 기울어진 맵을 표현하기 위함)
	FRotationMatrix RotMat(FRotator(0, RotDegAngle * 65535.0f / 360.0f, 0.0f));
	ScreenLocation = RotMat.TransformNormal(ScreenLocation);

	ScreenLocation = (FVector( ScreenLocation.Y, -ScreenLocation.X, 0 )) * Scale;		// 맵그림 전체를 원래크기로 화면 중앙에 찍을때 점의 좌표 (축은 중심에 있음)
	ScreenLocation += (FVector(FullMapCoord.UL - (2.0f *SelectedMapCoord.U + SelectedMapCoord.UL), FullMapCoord.VL - (2.0f * SelectedMapCoord.V + SelectedMapCoord.VL),0) / 2.0f);	// 드로잉하려는 MapCoordinates 영역의 중심으로 현재의 좌표축을 이동
	// 여기서 부터 Pixel단위 계산
	ScreenLocation = ScreenLocation * FVector(RenderBoundSize.X/SelectedMapCoord.UL, RenderBoundSize.Y/SelectedMapCoord.VL, 0);	// RenderBoundSize만큼 좌표를 확대한다
	ScreenLocation = ScreenLocation + FVector(RenderBounds[UIFACE_Left] + RenderBoundSize.X/2, RenderBounds[UIFACE_Top] + RenderBoundSize.Y/2, 0); // 좌표축을 중심에서 RenderBounds좌상단으로 이동한다.

	WorldToScreen = ScreenLocation;
	return  (RenderBounds[UIFACE_Left] <= WorldToScreen.X && WorldToScreen.X < RenderBounds[UIFACE_Right]) &&
			(RenderBounds[UIFACE_Top] <= WorldToScreen.Y && WorldToScreen.Y < RenderBounds[UIFACE_Bottom] )  ? TRUE : FALSE;
}

void UavaUIFullScreenMap::Render_Object( FCanvas* Canvas, 
										 const AavaHUD* HUD, 
										 const FVector WorldLocation, 
										 const INT IconCode, 
										 FLOAT Alpha, 
										 FColor Color, 
										 UBOOL bOverrideColor, 
										 UBOOL bAdjustRotator, 
										 FLOAT RotateDegree,
										 FVector Scaler )
{
	check(UIRadar);

	const FIconCodeInfo* IconInfo = GetIconInfo( IconCode );
	UBOOL bHasIcon = IconInfo != NULL;
	USurface* DrawImage = bHasIcon ? IconInfo->Image : DefaultWhiteTexture;
	FVector2D DrawExtent = bHasIcon ? FVector2D(IconInfo->Coord.UL, IconInfo->Coord.VL) : FVector2D(10,10);
	FTextureCoordinates DrawCoord = bHasIcon ? IconInfo->Coord : FTextureCoordinates(0,0,DrawImage->GetSurfaceWidth(), DrawImage->GetSurfaceHeight());
	FLinearColor DrawColor = bOverrideColor ? Color : bHasIcon ? IconInfo->Color : Color;

	DrawExtent.X *= Scaler.X * DefaultScale;
	DrawExtent.Y *= Scaler.Y * DefaultScale;

	DrawColor.A *= Clamp(Alpha,0.0f,1.0f);
	FVector Location = WorldLocation;
	//FRotationMatrix RotMat(FRotator(0, RotateDegAngle * 65535.0f / 360.0f, 0.0f));
	//Location = RotMat.TransformNormal(Location);
	if( CalcPosInMap( Location , RotateDegAngle) )
	{
		if ( !bAdjustRotator )
		{
			DrawTile( Canvas, 
					  DrawImage, 
					  (INT)(Location.X - DrawExtent.X/2), 
					  (INT)(Location.Y - DrawExtent.Y/2), 
					  (INT)DrawExtent.X, 
					  (INT)DrawExtent.Y, 
					  DrawCoord.U, 
					  DrawCoord.V, 
					  DrawCoord.UL, 
					  DrawCoord.VL, 
					  DrawColor);
		}
		else
		{
			DrawTileRotate( Canvas, 
							DrawImage, 
							(INT)(Location.X),  
							(INT)(Location.Y), 
							RotateDegree,
							(INT)-DrawExtent.X/2, 
							(INT)-DrawExtent.Y/2, 
							(INT)DrawExtent.X,
							(INT)DrawExtent.Y,
							DrawCoord.U, 
							DrawCoord.V, 
							DrawCoord.UL,
							DrawCoord.VL,
							DrawColor );

		}
	}
}

void UavaUIRTNotice::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);
	LatestUpdateTime = -1.f;
}

UBOOL UavaUIRTNotice::UpdateString()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if( HUD == NULL )
	{
		Message.Empty();
		LatestUpdateTime = -1.f;
		return FALSE;
	}

	UBOOL bHasNoMsg = HUD->RTNoticeUpdateTime < 0.f;
	UBOOL bInvalidTime = GWorld->GetTimeSeconds() < (HUD->RTNoticeUpdateTime);

	// 맵(월드)가 바뀌었을 경우. 허드를 포함해 초기화 해줘야한다.
	if( bInvalidTime )
	{
		HUD->RTNoticeUpdateTime = -1.f;
		LatestUpdateTime = -1.f;
		Message.Empty();
		return TRUE;
	}


	if(bHasNoMsg)
		return FALSE;
	else if( LatestUpdateTime != HUD->RTNoticeUpdateTime)
	{
		Message = HUD->RTNoticeMessage;
		LatestUpdateTime = HUD->RTNoticeUpdateTime;
		Opacity = FadeMax;
		FadeMode = 0;
		AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI) : NULL;
		if( GRI != NULL)
		{
			for( INT i = 0 ; i < GRI->TextStyleData.Num() ; i++)
			{
				if( GRI->TextStyleData(i).Name == TextStyleName )
				{
					UUISkin* Skin = GetActiveSkin();
					if( Skin == NULL )
						continue;

					UUIStyle* Style = Skin->FindStyle( FName( *( GRI->TextStyleData(i).StyleTag), FNAME_Find) );
					if( Style == NULL )
						continue;

					UUIStyle_Data* StyleData = Style->GetStyleForStateByClass(UUIState_Enabled::StaticClass());
					UUIStyle_Text* TextStyle = Cast<UUIStyle_Text>(StyleData);
					if( TextStyle == NULL )
					{
						UUIStyle_Combo* ComboStyle = Cast<UUIStyle_Combo>(StyleData);
						TextStyle = Cast<UUIStyle_Text>(ComboStyle->TextStyle.GetStyleData());
					}

					if( TextStyle != NULL )
					{
						DrawColor = TextStyle->StyleColor;
						Font = TextStyle->StyleFont;
						break;
					}
				}
			}
		}
		return TRUE;
	}

	else if ( GWorld->GetTimeSeconds() > (HUD->RTNoticeUpdateTime + DisplayDuration) )
	{
		if( GWorld->GetTimeSeconds() < (HUD->RTNoticeUpdateTime + DisplayDuration + FadeOutTime) )
		{		
			FadeMode = -1;	// Fade Out
			return FALSE;
		}

		else
		{
			Message.Empty();
			LatestUpdateTime = -1.f;
			HUD->RTNoticeUpdateTime = -1.f;
			return TRUE;		
		}
	}

	return FALSE;
}

void UavaUINVGGauge::Render_Widget( FCanvas* Canvas )
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->Pawn ) : NULL;
	
	if( GIsGame )
	{
		if ( PawnOwner == NULL || PawnOwner->NightvisionActivated == FALSE )	return;
		Ratio = PlayerOwner->CurrentBattery / PlayerOwner->MaximumBattery;
	}
	else
	{
		Ratio = TestRatio;
	}
	UpdateFadeInfos();
	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Render_Progress( Canvas, Image, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );
}

void UavaUIProjectileWeapon::Render_Widget( FCanvas* Canvas )
{
	UpdateWeaponData();
	FLOAT AccumXL = 0.f;
	FLOAT AccumYL = 0.f;

	FLOAT DrawX = RenderBounds[UIFACE_Right];
	FLOAT DrawY = RenderBounds[UIFACE_Bottom];

	for( INT i = 0 ; i < ProjWeapData.Num() ; i++ )
	{
		const FProjWeapDrawInfo& DrawInfo = ProjWeapData(i);
		if( DrawInfo.WeaponClass == NULL )
			continue;

		AavaWeapon* Weapon = DrawInfo.WeaponClass->GetDefaultObject<AavaWeapon>();
		if( Weapon == NULL )
			continue;

		AavaWeaponAttachment* Attachment = Weapon->AttachmentClass->GetDefaultObject<AavaWeaponAttachment>();
		if( Attachment == NULL)
			continue;

		// 찍을 문자를 준비
		TCHAR szIconStr[] = { TEXT('\0'), TEXT('\0') };
		szIconStr[0] = Attachment->DeathIconStr[0];
		FString WeapStr = szIconStr;
		FString CountStr;
		CountStr = appItoa( DrawInfo.AmmoCnt );

		// 찍을 문자의 크기를 얻음.
		FRenderParameters WeapStrParam(WeaponFont, 1.f, 1.f);
		UUIString::StringSize(WeapStrParam, szIconStr);

		FRenderParameters CountStrParam(CharacterFont, 1.f, 1.f);
		UUIString::StringSize(CountStrParam, *CountStr);

		FRenderParameters InterStrParam(CharacterFont, 1.f,1.f);
		UUIString::StringSize(InterStrParam, *InterMedString);

		DrawX = RenderBounds[UIFACE_Right];

		// 문자를 뒤에서 부터 찍음
		FLinearColor NewDrawColor = FLinearColor::White;//DrawColor;
		FLinearColor NewShadowColor = AavaHUD::StaticClass()->GetDefaultObject<AavaHUD>()->ShadowColor;
		FLOAT Ratio = (1.0f - Clamp( (GWorld->GetTimeSeconds() - DrawInfo.DetachTime) / FadeOutTime , 0.f, 1.f));
		if( DrawInfo.AmmoCnt == 0 && DrawInfo.DetachTime >= 0 )		// 페이드 아웃중임 (가진게 없음)
		{
			NewDrawColor.A *= Ratio;
			NewShadowColor.A *= Ratio;
		}

		(DrawX -= CountStrParam.DrawXL);
		if( bDropShadow )
			DrawString( Canvas, DrawX + 1, DrawY - CountStrParam.DrawYL + 1 , *CountStr, CharacterFont, NewShadowColor );
		DrawString( Canvas,  DrawX, DrawY - CountStrParam.DrawYL, *CountStr, CharacterFont, NewDrawColor );

		(DrawX -= InterStrParam.DrawXL);
		if( bDropShadow )
			DrawString( Canvas, DrawX + 1, DrawY - InterStrParam.DrawYL + 1, *InterMedString, CharacterFont, NewShadowColor );
		DrawString( Canvas, DrawX , DrawY - InterStrParam.DrawYL, *InterMedString, CharacterFont, NewDrawColor );

		(DrawX -= WeapStrParam.DrawXL);
		if( bDropShadow )	
			DrawString( Canvas, DrawX + 1 , DrawY - WeapStrParam.DrawYL + 1, *WeapStr, WeaponFont, NewShadowColor );
		DrawString( Canvas, DrawX, DrawY - WeapStrParam.DrawYL, *WeapStr, WeaponFont, NewDrawColor );


		// 실제 찍은 너비와 높이 계산
		FLOAT XL = Max( 0.f , WeapStrParam.DrawXL + CountStrParam.DrawXL + InterStrParam.DrawXL );
		FLOAT YL = Max( 0.f , Max( Max(WeapStrParam.DrawYL, CountStrParam.DrawYL) , InterStrParam.DrawYL));
		AccumXL = Max(XL, AccumXL);
		AccumYL += YL;

		DrawY -= YL;
	}

	// 렌더바운드를 조정해준다. (찍은 크기 = 렌더바운드 크기)
	if( !(GIsEditor && ! GIsGame) )
	{
		RenderBounds[UIFACE_Left] = RenderBounds[UIFACE_Right] - AccumXL;
		RenderBounds[UIFACE_Top] = RenderBounds[UIFACE_Bottom] - AccumYL;
	}
}

void UavaUIProjectileWeapon::UpdateWeaponData()
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	AavaPawn* PawnOwner = GetPlayerViewTarget();
	AavaHUD* HUD = PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;

	if (HUD && PawnOwner )
	{	
		FLOAT	CurrentTime = GWorld->GetTimeSeconds();
		if ( PreviousPawn != PawnOwner )	ProjWeapData.Empty();
		PreviousPawn = PawnOwner;
		for ( int i = 0 ; i < ProjWeapData.Num() ; ++ i )
		{
			const FProjWeapDrawInfo& WeapData = ProjWeapData(i);
			if ( WeapData.DetachTime > 0 )
			{
				if ( CurrentTime > WeapData.DetachTime + FadeOutTime )
				{
					ProjWeapData.Remove( i--, 1 );
					continue;
				}
			}
		}
		TArray<AavaWeapon*> FoundWeap;
		for ( int i = 0 ; i < 12 ; ++ i )
		{
			AavaWeapon* Weap = Cast<AavaWeapon>( PawnOwner->PossessedWeapon[i] );
			if ( Weap == NULL || Weap->AttachmentClass == NULL || Weap->bHideWeaponMenu )
				continue;

			if ( Cast<AavaThrowableWeapon>(Weap) != NULL ||
				 Cast<AavaWeap_BaseMissionObject>(Weap) != NULL ||
				 Cast<AavaWeap_Binocular>(Weap) != NULL )
			{
				FoundWeap.AddItem( Weap );
			}
		}
		UBOOL	bFound;
		for ( int i = 0 ; i < ProjWeapData.Num() ; ++ i )
		{
			bFound = FALSE;
			for ( int j = 0 ; j < FoundWeap.Num() ; ++ j )
			{
				if ( ProjWeapData(i).WeaponInstance == FoundWeap(j) )
				{
					bFound = TRUE;
					break;
				}
			}
			
			if ( bFound == FALSE )
			{
				if ( ProjWeapData(i).DetachTime < 0 )
				{
					ProjWeapData(i).AmmoCnt = 0;
					ProjWeapData(i).DetachTime = CurrentTime;
				}
			}
		}
		for ( int i = 0 ; i < FoundWeap.Num() ; ++ i )
		{
			bFound = FALSE;
			for ( int j = 0 ; j < ProjWeapData.Num() ; ++ j )
			{
				if ( FoundWeap(i) == ProjWeapData(j).WeaponInstance )
				{
					ProjWeapData(j).AmmoCnt = FoundWeap(i)->AmmoCount;
					bFound = TRUE;
					break;
				}
			}

			if ( bFound == FALSE )
			{
				FProjWeapDrawInfo	WeapData;
				WeapData.WeaponClass	=	FoundWeap(i)->GetClass();
				WeapData.WeaponInstance	=	FoundWeap(i);
				WeapData.AmmoCnt		=	FoundWeap(i)->AmmoCount;
				WeapData.DetachTime		=	-1;
				ProjWeapData.AddItem( WeapData );
			}
		}
	}	
}

void UavaUIOccupationBar::RenderCheckPoint( FCanvas* Canvas, float Cur, float Max )
{
	FLOAT	IndiX, IndiY, IndiXL, IndiYL;;
	FLOAT	X	= RenderBounds[UIFACE_Left];
	FLOAT	Y	= RenderBounds[UIFACE_Top];
	FLOAT	XL	= RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT	YL	= RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
	FLOAT	Rate	=	Max > 0.0 ?	Cur/Max : 0.0;

	IndiY	=	Y + Checkpoint.YPos;
	IndiXL	=	Checkpoint.Width;
	IndiYL	=	Checkpoint.Height;
	for( INT i = 0 ; i < TestCheckPoint.Num() ; ++ i )
	{
		IndiX	=	X +	Rate * XL;
		DrawTile( Canvas,
			Checkpoint.Image,
			IndiX - IndiXL/2, IndiY, IndiXL, IndiYL,
			Checkpoint.TextureCoord.U, 
			Checkpoint.TextureCoord.V, 
			Checkpoint.TextureCoord.UL,
			Checkpoint.TextureCoord.VL,
			Checkpoint.ImageColor );
	}
}

void UavaUIOccupationBar::RenderTargetPoint( FCanvas* Canvas, float Cur, float Max )
{
	if ( TargetPoint.Image == NULL )	return;

	FLOAT	IndiX, IndiY, IndiXL, IndiYL;
	FLOAT	X	= RenderBounds[UIFACE_Left];
	FLOAT	Y	= RenderBounds[UIFACE_Top];
	FLOAT	XL	= RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT	YL	= RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
	FLOAT	Rate	=	Max > 0.0 ?	Cur/Max : 0.0;
	FString	MsgRate;

	if ( Rate <= 0.0 )			return;

	IndiX	=	X +	Rate * XL;
	IndiY	=	Y + TargetPoint.YPos;
	IndiXL	=	TargetPoint.Width;
	IndiYL	=	TargetPoint.Height;
	DrawTile( Canvas,
			TargetPoint.Image,
			IndiX - IndiXL/2, IndiY, IndiXL, IndiYL,
			TargetPoint.TextureCoord.U, 
			TargetPoint.TextureCoord.V, 
			TargetPoint.TextureCoord.UL,
			TargetPoint.TextureCoord.VL,
			TargetPoint.ImageColor );
	if ( bDrawProgressRate && ProgressRateFont != NULL && HasBroadCastAuthority() )
	{
		IndiX	+=	TargetProgressRateOffset.X;
		IndiY	+=	TargetProgressRateOffset.Y;
		if ( bDrawProgressByPercentage )	MsgRate = FString::Printf( TEXT( "%.2f %%" ), Rate );
		else								MsgRate = FString::Printf( TEXT( "%.2f m" ), Cur * 0.01875 );
		DrawStringCentered( Canvas, IndiX, IndiY, *MsgRate, ProgressRateFont, ProgressRateColor );
	}
}

void UavaUIOccupationBar::Render_Widget( FCanvas* Canvas )
{
	AavaGameReplicationInfo* avaGRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI) : NULL;

	FLOAT	IndiX, IndiY, IndiXL, IndiYL;;
	FLOAT	X	= RenderBounds[UIFACE_Left];
	FLOAT	Y	= RenderBounds[UIFACE_Top];
	FLOAT	XL	= RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT	YL	= RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
	FString	MsgRate;
	
	// Draw Background Image
	if ( BackgroundBar.Image != NULL )
	{
		DrawTile( Canvas,
				  BackgroundBar.Image,
				  X, Y, XL, YL,
				  BackgroundBar.TextureCoord.U, 
				  BackgroundBar.TextureCoord.V, 
				  BackgroundBar.TextureCoord.UL,
				  BackgroundBar.TextureCoord.VL,
				  BackgroundBar.ImageColor, FALSE );
	}

	INT		IndicatorIdx = -1;
	FLOAT	IndicatorPos = 0.0; 

	if ( GIsEditor )
	{
		IndicatorPos	= TestIndicatorPoint;	
	}
	else
	{
		IndicatorPos	= ( avaGRI != NULL && avaGRI->MissionMaxTime != 0.0 ) ? avaGRI->MissionTime/avaGRI->MissionMaxTime : 0.0;
	}

	// Draw LeftBar & RightBar
	if ( LeftBar.Image != NULL )
	{
		DrawTile( Canvas,
			LeftBar.Image, X, Y, IndicatorPos * XL, YL, 
			LeftBar.TextureCoord.U, 
			LeftBar.TextureCoord.V, 
			LeftBar.TextureCoord.UL * IndicatorPos,
			LeftBar.TextureCoord.VL,
			LeftBar.ImageColor, FALSE );
	}

	if ( RightBar.Image != NULL )
	{
		DrawTile( Canvas,
			RightBar.Image, X + IndicatorPos * XL, Y, XL - (IndicatorPos * XL), YL, 
			RightBar.TextureCoord.U * ( 1.0 - IndicatorPos ), 
			RightBar.TextureCoord.V, 
			RightBar.TextureCoord.UL * ( 1.0 - IndicatorPos ),
			RightBar.TextureCoord.VL,
			RightBar.ImageColor, FALSE );
	}

	if ( GIsEditor )
	{
		// Draw CheckPoint Image
		if ( Checkpoint.Image != NULL )
		{
			for( INT i = 0 ; i < TestCheckPoint.Num() ; ++ i )
				RenderCheckPoint( Canvas, TestCheckPoint(i), 1.0f );
		}

		RenderTargetPoint( Canvas, TestTargetPoint, 1.0f );


		// Draw Indicator
		IndicatorIdx	= TestIndicatorIdx;
		
		if ( bDrawProgressByPercentage )
			MsgRate = FString::Printf( TEXT( "%s %%" ), *TestRate );
		else
			MsgRate = FString::Printf( TEXT( "%s m" ), *TestRate );
	}
	else if( avaGRI != NULL)
	{
		// Draw CheckPoint Image
		if ( Checkpoint.Image != NULL && avaGRI->MissionMaxTime != 0.0 )
		{
			for( INT i = 0 ; i < 5 ; ++ i )
			{
				if ( avaGRI->MissionCheckPoint[i] != 0.0 )
				{
					RenderCheckPoint( Canvas, avaGRI->MissionCheckPoint[i], avaGRI->MissionMaxTime );
				}
			}
		}

		if ( avaGRI->TargetMissionTime > 0.0 )
		{
			RenderTargetPoint( Canvas, avaGRI->TargetMissionTime, avaGRI->MissionMaxTime );
		}

		IndicatorIdx	= avaGRI->MissionIndicatorIdx;
		

		if ( bDrawProgressByPercentage )
			MsgRate = FString::Printf( TEXT( "%.2f %%" ), avaGRI->MissionMaxTime != 0.0 ? avaGRI->MissionTime/avaGRI->MissionMaxTime : 0.0 );
		else
			MsgRate = FString::Printf( TEXT( "%.2f m" ), avaGRI->MissionTime * 0.01875 );

		if ( bUseDominance )
		{
			if ( avaGRI->DominanceTeamIdx == 0 )		IndicatorPos = 0.5f - IndicatorPos/2.0f;
			else if ( avaGRI->DominanceTeamIdx == 1)	IndicatorPos = 0.5f + IndicatorPos/2.0f;
			else										IndicatorPos = 0.5f;
		}
	}

	if ( IndicatorIdx >= 0 && IndicatorIdx < Indicator.Num() && Indicator.Num() >= 0 )
	{
		FOccupationIndicatorInfo* IndicatorInfo = &Indicator(IndicatorIdx);

		if ( IndicatorInfo->Image != NULL )
		{
			IndiX	=	X + IndicatorPos * XL;
			IndiY	=	Y + IndicatorInfo->YPos;
			IndiXL	=	IndicatorInfo->Width;
			IndiYL	=	IndicatorInfo->Height;
			DrawTile( Canvas,
				IndicatorInfo->Image,
				IndiX - IndiXL/2, IndiY, IndiXL, IndiYL,
				IndicatorInfo->TextureCoord.U, 
				IndicatorInfo->TextureCoord.V, 
				IndicatorInfo->TextureCoord.UL,
				IndicatorInfo->TextureCoord.VL,
				IndicatorInfo->ImageColor );

			UavaNetHandler* NetHandler = UavaNetHandler::StaticClass()->GetDefaultObject<UavaNetHandler>()->GetAvaNetHandler();
			if ( bDrawProgressRate && ProgressRateFont != NULL && ( NetHandler != NULL && NetHandler->GetCurrentChannelMaskLevel() >= 2 ) )
			{
				IndiX	+=	ProgressRateOffset.X;
				IndiY	+=	ProgressRateOffset.Y;
				DrawStringCentered( Canvas, IndiX, IndiY, *MsgRate, ProgressRateFont, ProgressRateColor );
			}
		}
	}
}

UBOOL UavaUIVoteMessage::UpdateString()
{
	AavaGameReplicationInfo* GRI = GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;

	if ( GIsEditor )				return TRUE;
	if ( GRI == NULL )				return FALSE;
	if ( GRI->AmITarget == true	)	return FALSE;	// 내가 Vote 의 대상자이면 찍지 않는다.

	FString NewMessage;
	switch( messagetype )
	{
	case VOTEMSG_TITLE		:		if ( GRI->bVoting ) 							NewMessage = GRI->VoteTitleMsg;		break;
	case VOTEMSG_WARN		:		if ( GRI->bVoting )								NewMessage = GRI->VoteWarn;			break;
	case VOTEMSG_VOTING		:		if ( GRI->AmIVote == FALSE && GRI->bVoting )	NewMessage = GRI->VotingMsg;		break;
	case VOTEMSG_PROGRESS	:		if ( GRI->bVoting && GRI->bVoteProgressMsg )	NewMessage = GRI->VoteProgressMsg;	break;
	case VOTEMSG_RESULT		:		if ( GRI->bVoteResultMsg )						NewMessage = GRI->VoteResultMsg;	break;
	case VOTEMSG_TIME		:		if ( GRI->bVoting )								NewMessage = GRI->VoteTimerMsg;		break;
	}
	
	if ( Message != NewMessage )
	{
		Message = NewMessage;
		return TRUE;
	}
	return FALSE;
}

void UavaUICustomProgressBar::UpdateFadeInfos()
{
}

void UavaUICustomProgressBar::Render_Widget( FCanvas* Canvas )
{
	if ( GIsEditor )
		Ratio	=	TestRatio;
	UpdateFadeInfos();
	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Render_Progress( Canvas, Image, ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );
}

void UavaUICustomProgressBar::Render_Progress( FCanvas* Canvas, USurface* Tex, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL )
{
	FLinearColor LC_Box( BackgroundColor ), LC_Progress( ProgressColor ), LC_Alert( AlertColor ), LC_Fade( FadeColor );
	LC_Box.A *= Opacity;
	LC_Progress.A *= Opacity;
	LC_Alert.A *= Opacity;

	Render_Bar( Canvas, 0, 1, Tex, CU, CV, CUL, CVL, LC_Box );

	FLOAT PreviousRatio = 0.0f;

	FLinearColor LC;
	if (FadeInfos(FadeInfos.Num()-1).Value < AlertStartRatio)
	{
		LC = LC_Alert;
	}
	else
	{
		LC = LC_Progress;
	}

	LC.A *= Opacity;

	for (INT i=FadeInfos.Num()-1; i>=0; --i)
	{
		const FGaugeFadeInfo& FI = FadeInfos(i);				

		LC.A *= FI.Alpha;

		Render_Bar( Canvas, PreviousRatio, FI.Value - PreviousRatio, Tex, CU, CV, CUL, CVL, LC );

		LC = LC_Fade;
		LC.A *= Opacity;

		PreviousRatio = FI.Value;
	}			
}

void UavaUICustomProgressBar::Render_Bar( FCanvas* Canvas, FLOAT Ratio, FLOAT RatioL, USurface* Surface, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL, const FLinearColor& DrawColor )
{
	if (Surface == NULL) return;

	FLOAT X  = XPos;
	FLOAT Y  = YPos;
	FLOAT XL = XSize;
	FLOAT YL = YSize;

	if (Surface)
	{		
		switch (Direction)
		{
		default :
		case AVAUIPROGRESSDirection_Left :
			DrawTile( Canvas, Surface, X + appTrunc( XL * Ratio ), Y, appTrunc( XL * RatioL ), YL, CU + appTrunc( CUL * Ratio ), CV, appTrunc( CUL * RatioL ), CVL, DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Right :
			DrawTile( Canvas, Surface, X + appTrunc( XL * (1-(Ratio+RatioL)) ), Y, appTrunc( XL * RatioL ), YL, CU + appTrunc( CUL * (1-(Ratio+RatioL)) ), CV, appTrunc( CUL * RatioL ), CVL, DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Up :
			DrawTile( Canvas, Surface, X, Y + appTrunc( YL * Ratio ), XL, appTrunc( YL * RatioL ), CU, CV + appTrunc( CVL * Ratio ), CUL, appTrunc( CVL * RatioL ), DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Down :
			DrawTile( Canvas, Surface, X, Y + appTrunc( YL * (1-(Ratio+RatioL)) ), XL, appTrunc( YL * RatioL ), CU, CV + appTrunc(CVL * (1 - (Ratio+RatioL))), CUL, appTrunc( CVL * RatioL ), DrawColor );	
			break;							
		}		
	}		
}

void UavaUIMemberInfo::UpdateArmorBarFadeInfo( int nSlot, float InRatio, float FadeOutTime )
{
	// 0, 1사이로 clamp :) 
	InRatio = Clamp( InRatio, 0.0f, 1.0f );

	/// 마지막 value는 실제 값이므로 냅두자.
	for ( INT i = 0 ; i < ArmorBarFadeInfos[nSlot].FadeInfos.Num() - 1 ; )
	{
		FGaugeFadeInfo& FI = ArmorBarFadeInfos[nSlot].FadeInfos(i);

		if (FadeOutTime > 0)
			FI.Alpha = Clamp( FI.Alpha - GWorld->GetDeltaSeconds() / FadeOutTime, 0.0f, 1.0f );
		else
			FI.Alpha = 0;

		if (FI.Alpha == 0)
		{			
			ArmorBarFadeInfos[nSlot].FadeInfos.Remove(0);
		}
		else
			++i;
	}

	/// 하나도 없을 때 무조건 추가!
	if (ArmorBarFadeInfos[nSlot].FadeInfos.Num() == 0)
	{
		FGaugeFadeInfo FI;
		FI.Alpha = 1.0f;
		FI.Value = InRatio;

		ArmorBarFadeInfos[nSlot].FadeInfos.AddItem( FI );
	}
	else 
	{
		FLOAT LastValue = ArmorBarFadeInfos[nSlot].FadeInfos( ArmorBarFadeInfos[nSlot].FadeInfos.Num()-1 ).Value ;		

		/// 얼레 더 커졌다.
		if (LastValue < InRatio)
		{
			ArmorBarFadeInfos[nSlot].FadeInfos.Empty(1);

			FGaugeFadeInfo FI;
			FI.Alpha = 1.0f;
			FI.Value = InRatio;

			ArmorBarFadeInfos[nSlot].FadeInfos.AddItem( FI );
		}
		/// 줄어들었다!
		else if (LastValue > InRatio)
		{
			FGaugeFadeInfo FI;
			FI.Alpha = 1.0f;
			FI.Value = InRatio;

			ArmorBarFadeInfos[nSlot].FadeInfos.AddItem( FI );
		}
	}
}

void UavaUIMemberInfo::UpdateHealthBarFadeInfo( int nSlot, float InRatio, float FadeOutTime )
{
	// 0, 1사이로 clamp :) 
	InRatio = Clamp( InRatio, 0.0f, 1.0f );

	/// 마지막 value는 실제 값이므로 냅두자.
	for ( INT i = 0 ; i < HealthBarFadeInfos[nSlot].FadeInfos.Num() - 1 ; )
	{
		FGaugeFadeInfo& FI = HealthBarFadeInfos[nSlot].FadeInfos(i);

		if (FadeOutTime > 0)
			FI.Alpha = Clamp( FI.Alpha - GWorld->GetDeltaSeconds() / FadeOutTime, 0.0f, 1.0f );
		else
			FI.Alpha = 0;

		if (FI.Alpha == 0)
		{			
			HealthBarFadeInfos[nSlot].FadeInfos.Remove(0);
		}
		else
			++i;
	}

	/// 하나도 없을 때 무조건 추가!
	if (HealthBarFadeInfos[nSlot].FadeInfos.Num() == 0)
	{
		FGaugeFadeInfo FI;
		FI.Alpha = 1.0f;
		FI.Value = InRatio;

		HealthBarFadeInfos[nSlot].FadeInfos.AddItem( FI );
	}
	else 
	{
		FLOAT LastValue = HealthBarFadeInfos[nSlot].FadeInfos( HealthBarFadeInfos[nSlot].FadeInfos.Num()-1 ).Value ;		

		/// 얼레 더 커졌다.
		if (LastValue < InRatio)
		{
			HealthBarFadeInfos[nSlot].FadeInfos.Empty(1);

			FGaugeFadeInfo FI;
			FI.Alpha = 1.0f;
			FI.Value = InRatio;

			HealthBarFadeInfos[nSlot].FadeInfos.AddItem( FI );
		}
		/// 줄어들었다!
		else if (LastValue > InRatio)
		{
			FGaugeFadeInfo FI;
			FI.Alpha = 1.0f;
			FI.Value = InRatio;

			HealthBarFadeInfos[nSlot].FadeInfos.AddItem( FI );
		}
	}
}

void UavaUIMemberInfo::DrawBG( FCanvas* Canvas, int BGType, float X, float Y, float XL, float YL )
{
	// 1. Draw Background first...
	if ( BGInfo[BGType].BackgorundImage != NULL )
	{
		DrawTile( Canvas, 
			BGInfo[BGType].BackgorundImage, 
			X, Y, XL, YL, 
			BGInfo[BGType].BackgroundCoordinate.U, 
			BGInfo[BGType].BackgroundCoordinate.V,
			BGInfo[BGType].BackgroundCoordinate.UL,
			BGInfo[BGType].BackgroundCoordinate.VL,
			BGInfo[BGType].BackgroundColor );
	}
}

void UavaUIMemberInfo::DrawPlayerName( FCanvas* Canvas, const BOOL bSquadLeader, const INT Level, const TCHAR* PlayerName, float X, float Y, float XL, float YL, float AddAlpha )
{
	// 2. Draw Player Name
	if ( PlayerNameFont != NULL )
	{
		FString		Message;
		FLOAT		NameX, NameY;
		INT			NameXL, NameYL;
		FLinearColor	DrawColor( bSquadLeader ? LeaderPlayerNameColor : PlayerNameColor );
		DrawColor.A *= AddAlpha;
		Message = FString::Printf( TEXT( "%s" ), PlayerName );
		StringSize( PlayerNameFont, NameXL, NameYL, *Message );
		NameX = X + PlayerNameOffset.X;
		NameY = Y + PlayerNameOffset.Y;

		if ( PlayerNameHorizontalAlign == AVAUIALIGN_Center )				NameX -= NameXL/2.0f;
		else if ( PlayerNameHorizontalAlign == AVAUIALIGN_RightOrBottom )	NameX -= NameXL;
		if ( PlayerNameVerticalAlign == AVAUIALIGN_Center )					NameY -= NameYL/2.0f;
		else if ( PlayerNameVerticalAlign == AVAUIALIGN_RightOrBottom )		NameY -= NameYL;

		if ( bUsePlayerNameShadow )
			DrawString( Canvas, NameX + NameYL + PlayerNameShadowOffset.X, NameY + PlayerNameShadowOffset.Y, *Message, PlayerNameFont, PlayerNameShadowColor );
		DrawString( Canvas, NameX + NameYL, NameY, *Message, PlayerNameFont, DrawColor );
		//DrawLevelIcon( Canvas, NULL, NameX, NameY, NameYL, NameYL, PlayerNameColor.A, Max(Level - 1,0), 0, 0 );
	}
}

#define CHECK_VALID_ARRAY_INDEX( ARRAY, INDEX ) ( INDEX >= 0 && INDEX < ARRAYSIZE(ARRAY) )
void UavaUIMemberInfo::DrawHealthBar( FCanvas* Canvas, int nSlot, float Ratio, float X, float Y, float XL, float YL )
{
	// 3. Draw Health Bar
	if ( bDrawHealthBar && CHECK_VALID_ARRAY_INDEX( HealthBarFadeInfos, nSlot ))
	{
		HealthBar->XPos		=	X + HealthBarOffset.X;
		HealthBar->YPos		=	Y + HealthBarOffset.Y;
		HealthBar->XSize	=	HealthBarSize.X;
		HealthBar->YSize	=	HealthBarSize.Y;
		UpdateHealthBarFadeInfo( nSlot, Ratio, HealthBar->FadeTime );
		HealthBar->eventSetFadeInfos( HealthBarFadeInfos[nSlot].FadeInfos );
		HealthBar->Render_Widget( Canvas );
	}
}

void UavaUIMemberInfo::DrawArmorBar( FCanvas* Canvas, int nSlot, float Ratio, float X, float Y, float XL, float YL )
{			// 4. Draw Armor Bar
	if ( bDrawArmorBar && CHECK_VALID_ARRAY_INDEX( ArmorBarFadeInfos, nSlot ))
	{
		ArmorBar->XPos		=	X + ArmorBarOffset.X;
		ArmorBar->YPos		=	Y + ArmorBarOffset.Y;
		ArmorBar->XSize		=	ArmorBarSize.X;
		ArmorBar->YSize		=	ArmorBarSize.Y;
		UpdateArmorBarFadeInfo( nSlot, Ratio, ArmorBar->FadeTime );
		ArmorBar->eventSetFadeInfos( ArmorBarFadeInfos[nSlot].FadeInfos );
		ArmorBar->Render_Widget( Canvas );
	}
}

void UavaUIMemberInfo::DrawWeaponInfo( FCanvas* Canvas, const TCHAR* WeaponInfo, float X, float Y, float XL, float YL, float AddAlpha, bool bDead )
{
	// 5. Draw Weapon Info

	if ( !bDead )
	{
		if ( bDrawWeaponInfo && WeaponInfoFont != NULL )
		{
			FString		Message;
			FLOAT		NameX, NameY;
			INT			NameXL, NameYL;

			FLinearColor	DrawColor(WeaponInfoColor);
			DrawColor.A *= AddAlpha;

			Message = FString::Printf( TEXT( "%s" ), WeaponInfo );
			StringSize( WeaponInfoFont, NameXL, NameYL, *Message );
			NameX = X + WeaponInfoOffset.X;
			NameY = Y + WeaponInfoOffset.Y;

			if ( WeaponInfoHorizontalAlign == AVAUIALIGN_Center )				NameX -= NameXL/2.0f;
			else if ( WeaponInfoHorizontalAlign == AVAUIALIGN_RightOrBottom )	NameX -= NameXL;
			if ( WeaponInfoVerticalAlign == AVAUIALIGN_Center )					NameY -= NameYL/2.0f;
			else if ( WeaponInfoVerticalAlign == AVAUIALIGN_RightOrBottom )		NameY -= NameYL;

			if ( bUseWeaponInfoShadow )
				DrawString( Canvas, NameX + WeaponInfoShadowOffset.X, NameY + WeaponInfoShadowOffset.Y, *Message, WeaponInfoFont, WeaponInfoShadowColor );
			DrawString( Canvas, NameX, NameY, *Message, WeaponInfoFont, DrawColor );
		}
	}
	else
	{
		FLinearColor	DrawColor(StatusColor);		
		DrawString( Canvas, 
					X + StatusOffset.X, 
					Y + StatusOffset.Y, 
					*AavaPlayerReplicationInfo::StaticClass()->GetDefaultObject<AavaPlayerReplicationInfo>()->WhenDeadStr, 
					StatusFont, 
					DrawColor );
	}
}

void UavaUIMemberInfo::DrawClassInfo( FCanvas* Canvas, const int ClassNum, float X, float Y, float XL, float YL, float AddAlpha )
{
	if ( bDrawClassInfo && ClassIconIndex.IsValidIndex( ClassNum ) )
	{
		const FIconCodeInfo* IconInfo = GetIconInfo(ClassIconIndex(ClassNum));
		if ( IconInfo != NULL )
		{
			DrawIconInfo( Canvas, X, Y, XL, YL, IconInfo, AddAlpha );
		}
	}
}

void UavaUIMemberInfo::DrawUseActionInfo( FCanvas* Canvas, const int UseAction, float X, float Y, float XL, float YL , float AddAlpha)
{
	if ( bDrawUseAction && UseActionIconIndex.IsValidIndex( UseAction ) )
	{
		const FIconCodeInfo* IconInfo = GetIconInfo(UseActionIconIndex(UseAction));
		if ( IconInfo != NULL )
		{
			DrawIconInfo( Canvas, X, Y, XL, YL, IconInfo, AddAlpha );
		}
	}
}

void UavaUIMemberInfo::Render_Widget( FCanvas* Canvas )
{
	FLOAT X		=	RenderBounds[UIFACE_Left];
	FLOAT Y		=	RenderBounds[UIFACE_Top];
	FLOAT XL	=	RenderBounds[UIFACE_Right]	- RenderBounds[UIFACE_Left];
	FLOAT YL	=	RenderBounds[UIFACE_Bottom]	- RenderBounds[UIFACE_Top];
	if ( GIsEditor )
	{
		for ( int i = 0 ; i < TestCnt ; ++ i )
		{
			FLOAT YAccum	=	Y + ( YL + ColumnSpacing ) * i;
			int	BGTYPE;
			if ( TestDisabledCol == i )			BGTYPE = 1;
			else if ( TestSelectedCol == i )	BGTYPE = 2;
			else								BGTYPE = 0;

			FLOAT	Alpha	=	BGInfo[BGTYPE].BackgroundColor.A/255.0f;

			DrawBG( Canvas, BGTYPE, X, YAccum, XL, YL );
			
			DrawHealthBar( Canvas, i, HealthBar->TestRatio, X, YAccum, XL, YL );
			DrawArmorBar( Canvas, i, ArmorBar->TestRatio, X, YAccum, XL, YL );

			DrawSlotIcon( Canvas, TeamIndex, i, X + SlotNumOffset.X, YAccum + SlotNumOffset.Y, SlotNumSize.X, SlotNumSize.Y, Alpha, SlotNumHorizontalAlign, SlotNumVerticalAlign );
			
			DrawPlayerName( Canvas, i==0 ? TRUE : FALSE, 0, *(TestPlayerName), X, YAccum, XL, YL, Alpha );
			DrawWeaponInfo( Canvas, *(TestWeaponCode), X, YAccum, XL, YL, Alpha, BGTYPE == 1 );
			
			DrawClassInfo( Canvas, TestClassNum, X + ClassInfoOffset.X, YAccum + ClassInfoOffset.Y, ClassInfoSize.X, ClassInfoSize.Y, Alpha );

			DrawUseActionInfo( Canvas, TestUseActionNum, X + UseActionIconOffset.X, YAccum + UseActionIconOffset.Y, UseActionIconSize.X, UseActionIconSize.Y, Alpha );
		}
	}
	else
	{
		AavaGameReplicationInfo*			GRI			= GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;

		if ( GRI == NULL )	return;
		AavaPlayerReplicationInfo*			ViewtargetPRI	= GetPlayerViewTargetPRI();
		TArray<AavaPlayerReplicationInfo*>	PRIArray;
		SortPRIList( PRIArray, GRI, AVAUISummary_SlotNum, TeamIndex, false );

		for( INT i = 0 ; i < PRIArray.Num() ; i++ )
		{
			AavaPlayerReplicationInfo* PRI = PRIArray(i);
			if ( PRI == NULL )	continue;

			FLOAT YAccum	=	Y + ( YL + ColumnSpacing ) * i;
			int	BGTYPE;

			if ( ViewtargetPRI != NULL && ViewtargetPRI == PRI )	BGTYPE = 2;
			else if ( PRI->bIsSpectator == true )					BGTYPE = 1;
			else													BGTYPE = 0;

			FLOAT	Alpha	=	BGInfo[BGTYPE].BackgroundColor.A/255.0f;

			DrawBG( Canvas, BGTYPE, X, YAccum, XL, YL );

			AavaPawn* avaPawn = GetAvaPawn( PRI );
			if ( avaPawn != NULL )
			{
				DrawHealthBar( Canvas, i, avaPawn->HealthMax > 0.0 ? avaPawn->Health/(FLOAT)avaPawn->HealthMax : 0.0f, X, YAccum, XL, YL );
				DrawArmorBar( Canvas, i, avaPawn->ArmorMax > 0.0 ? avaPawn->Armor_Stomach/(FLOAT)avaPawn->ArmorMax : 0.0f, X, YAccum, XL, YL );
				if ( avaPawn->CurrentWeapon != NULL && avaPawn->CurrentWeapon->AttachmentClass != NULL )
					DrawWeaponInfo( Canvas, *(avaPawn->CurrentWeapon->AttachmentClass->GetDefaultObject<AavaWeaponAttachment>()->DeathIconStr), X, YAccum, XL, YL, Alpha, BGTYPE == 1 );
			}
			else
			{
				DrawHealthBar( Canvas, i, 0.0f, X, YAccum, XL, YL );
				DrawArmorBar( Canvas, i, 0.0f, X, YAccum, XL, YL );
				DrawWeaponInfo( Canvas, _T(""), X, YAccum, XL, YL, Alpha, BGTYPE == 1 );
			}

			DrawSlotIcon( Canvas, PRI->Team != NULL ? PRI->Team->TeamIndex : 255, PRI->SlotNum, X + SlotNumOffset.X, YAccum + SlotNumOffset.Y, SlotNumSize.X, SlotNumSize.Y, Alpha, SlotNumHorizontalAlign, SlotNumVerticalAlign );
			DrawPlayerName( Canvas, PRI->bSquadLeader, PRI->Level, *(PRI->PlayerName), X, YAccum, XL, YL,  Alpha );
			DrawClassInfo( Canvas, PRI->CurrentSpawnClassID, X + ClassInfoOffset.X, YAccum + ClassInfoOffset.Y, ClassInfoSize.X, ClassInfoSize.Y, Alpha );
			DrawUseActionInfo( Canvas, PRI->CurrentUseActionType, X + UseActionIconOffset.X, YAccum + UseActionIconOffset.Y, UseActionIconSize.X, UseActionIconSize.Y, Alpha );
		}
	}
}

void UavaUILevelIcon::Render_Widget( FCanvas* Canvas )
{
	AavaPlayerController* PlayerOwner	= GetavaPlayerOwner();
	AavaPawn* PawnOwner					= PlayerOwner ? Cast<AavaPawn>( PlayerOwner->ViewTarget ) : NULL;
	AavaPlayerReplicationInfo* PRI		= PawnOwner ? Cast<AavaPlayerReplicationInfo>( PawnOwner->PlayerReplicationInfo ) : NULL;
	INT	Level;
	if ( PRI != NULL )	Level = PRI->Level;
	else				Level = TestLevel;
	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];
	DrawLevelIcon( Canvas, NULL, X, Y, XL, YL, Oppacity, Max(Level - 1,0), 0, 0 );
}

FLOAT AavaHUD::AlterScreenQuality( FLOAT Amount )
{
	static const FLOAT MinPercentage = 50.0f;	
	static const FLOAT MaxQuality = 10.0f;	

	FlushRenderingCommands();

	FLOAT OldQuality = Clamp( (GSystemSettings->ScreenPercentage - MinPercentage) / (100 - MinPercentage) * MaxQuality, 0.0f, MaxQuality );
	FLOAT NewQuality = Clamp( OldQuality + Amount, 0.0f, MaxQuality );

	GSystemSettings->ScreenPercentage = NewQuality / MaxQuality * (100 - MinPercentage) + MinPercentage;
	
	//Message(PlayerReplicationInfo, Msg, '');

	return NewQuality;
}


void UavaUITopPlayerInfo::DrawBG( FCanvas* Canvas, float X, float Y, float XL, float YL )
{
	// 1. Draw Background first...
	if ( BackgroundImage != NULL )
	{
		DrawTile( Canvas, 
			BackgroundImage, 
			X, Y, XL, YL, 
			BackgroundCoord.U, 
			BackgroundCoord.V,
			BackgroundCoord.UL,
			BackgroundCoord.VL,
			BackgroundImageColor );
	}
}

FLOAT GetCurveValue( class UCurveEdPresetCurve* PresetCurve, FLOAT Ratio)
{
	return PresetCurve ? PresetCurve->EvalRatio(Clamp(Ratio, 0.f, 1.f)) : Ratio;
}

void UavaUITopPlayerInfo::DrawPlayerName( FCanvas* Canvas, const int Number, const BOOL bSelected, const INT Level, const INT Score1, const INT Score2, const TCHAR* PlayerName, float X, float Y )
{
	UFont* Font			=	bSelected ? SelectedNumberFont : NumberFont;
	FLOAT  FontScale	=	1.0f;

	if ( bSelected == TRUE )
	{
		FLOAT	DeltaTime =	GWorld->GetTimeSeconds() - UpdateScoreTime;
		if ( DeltaTime < FontScalerMaxTime )
			FontScale = GetCurveValue( UpdateScoreCurve, Clamp(DeltaTime/FontScalerMaxTime, 0.f, 1.f) );
	}

	FString	Message;
	if ( bSelected && SelectedImage != NULL)
	{
		DrawTile( Canvas, 
			SelectedImage, 
			X + SelectedImagePos.X, 
			Y + SelectedImagePos.Y, 
			SelectedImageSize.X, 
			SelectedImageSize.Y, 
			SelectedCoord.U, 
			SelectedCoord.V,
			SelectedCoord.UL,
			SelectedCoord.VL,
			SelectedImageColor );
	}

	// Draw Number
	if ( Font != NULL )	
	{
		FLOAT	XPos, YPos;
		XPos = X;
		YPos = Y;

		Message = FString::Printf( TEXT( "%d" ), Number );

		if ( bSelected )
		{
			INT XL1, YL1, XL2, YL2;
			StringSize(NumberFont, XL1, YL1, *Message);
			StringSize(SelectedNumberFont, XL2, YL2, *Message);
			YPos -= ( YL2 / 2.0f );
			XPos -= XL2;
		}
		else
		{
			INT XL1, YL1;
			StringSize(Font, XL1, YL1, *Message);
			YPos -= ( YL1 / 2.0f );
			XPos -= XL1;
		}
		DrawString( Canvas, XPos, YPos, *Message, Font, NumberColor );
	}

	// Draw Level
	INT XL, YL;
	StringSize(NameFont, XL, YL, PlayerName);
	DrawLevelIcon( Canvas, NULL, X + NameSpacing, Y, YL, YL, 1.0f, Level, 0, 1 );

	FLinearColor	NameColor = bSelected ? NameSelectedColor : NameNormalColor;
	// Draw Name
	DrawString( Canvas, X + NameSpacing + YL, Y - (YL/2.0f), PlayerName, NameFont, NameColor );

	// Draw Point
	if ( Score1 > Score2  )		Message = FString::Printf( TEXT( "%d Pts(+%d)" ), Score1, Score1-Score2 );
	else if ( Score1 < Score2 )	Message = FString::Printf( TEXT( "%d Pts(%d)" ), Score1, Score1-Score2 );
	else						Message = FString::Printf( TEXT( "%d Pts" ), Score1 );		
	
	StringSize(NameFont, XL, YL, *Message);
	DrawString( Canvas, INT( X + PointSpacing - ( XL * ( FontScale - 1.0f ) ) / 2.0f ),  INT( Y - (YL/2.0f) - ( YL * ( FontScale - 1.0f ) ) / 2.0f ) , *Message, NameFont, NameColor, FontScale, FontScale );
}

void UavaUITopPlayerInfo::Render_Widget( FCanvas* Canvas)
{

	FLOAT X		=	RenderBounds[UIFACE_Left];
	FLOAT Y		=	RenderBounds[UIFACE_Top];
	FLOAT XL	=	RenderBounds[UIFACE_Right]	- RenderBounds[UIFACE_Left];
	FLOAT YL	=	RenderBounds[UIFACE_Bottom]	- RenderBounds[UIFACE_Top];
	
	// Draw Background Image
	DrawBG( Canvas, X, Y, XL, YL );

	X += DrawPos.X;
	Y += DrawPos.Y;

	if ( GIsEditor && !GIsGame )
	{
		if ( bDrawAbsoluteRank )
		{
			for ( int i = 0 ; i < DrawCnt ; ++ i )
			{
				DrawPlayerName( Canvas, i+TestStartNum, FALSE, 0, DrawCnt * 2, DrawCnt, *TestName, X , Y );
				Y += ColumnSpacing;
			}
		}
		else
		{
			for ( int i = 0 ; i < DrawCnt * 2 + 1 ; ++ i )
			{
				DrawPlayerName( Canvas, i+TestStartNum, ( i == DrawCnt ), 0, DrawCnt * 2  - i, DrawCnt, *TestName, X , Y );
				Y += ColumnSpacing;
			}
		}
	}
	else
	{
		AavaGameReplicationInfo*			GRI			= GWorld->GetWorldInfo() ? Cast<AavaGameReplicationInfo>( GWorld->GetWorldInfo()->GRI ) : NULL;
		AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
		AavaPlayerReplicationInfo* PRIOwner;
		
		if ( AmISpectator() )
		{
			PRIOwner = GetPlayerViewTargetPRI();
		}
		else
		{
			PRIOwner = PlayerOwner ? Cast<AavaPlayerReplicationInfo>( PlayerOwner->PlayerReplicationInfo ) : NULL;
			if ( PRIOwner->Score != PrevScore )
				UpdateScoreTime = GWorld->GetTimeSeconds();
			PrevScore = PRIOwner->Score;
		}

		if ( GRI == NULL )	return;

		TArray<AavaPlayerReplicationInfo*>	PRIArray;
		SortPRIList( PRIArray, GRI, AVAUISummary_Score, 255, FALSE );

		INT		Rank = 0;
		INT		PrvScore = -1, PrvDeath = -1;
		INT		TieCount = 0;
		INT		MyScore = PRIOwner != NULL ? PRIOwner->Score : 0;

		if ( bDrawAbsoluteRank )
		{
			for ( int i = 0 ; i < DrawCnt ; ++ i )
			{
				AavaPlayerReplicationInfo* PRI = PRIArray(i);
				if ( PRI == NULL )	continue;

				++Rank;
				if ( PRI->Score == PrvScore && PRI->Deaths == PrvDeath )
				{
					++TieCount;
				}
				else
				{
					TieCount = 0;
				}
				PrvScore = PRI->Score;
				PrvDeath = PRI->Deaths;

				DrawPlayerName( Canvas, Rank - TieCount, FALSE, Max(PRI->Level - 1,0), PRI->Score, MyScore, *PRI->PlayerName, X, Y );
				Y += ColumnSpacing;
			}
		}
		else
		{
			if ( PRIOwner == NULL )
				return;

			INT	MyIndex = 0;

			if ( PRIOwner != NULL )
			{
				for( INT i = 0 ; i < PRIArray.Num() ; i++ )
				{
					AavaPlayerReplicationInfo* PRI = PRIArray(i);
					if ( PRI == PRIOwner )
						break;
					++MyIndex;
				}
			}

			INT Cnt = DrawCnt * 2 + 1;
			INT	PrevIndex = MyIndex - DrawCnt;
			if ( PrevIndex + Cnt >= PRIArray.Num() )
				PrevIndex = PRIArray.Num() - Cnt;
			for( INT i = 0 ; i < PRIArray.Num() ; i++ )
			{
				AavaPlayerReplicationInfo* PRI = PRIArray(i);

				if ( PRI == NULL )	continue;

				++Rank;
				if ( PRI->Score == PrvScore && PRI->Deaths == PrvDeath )
				{
					++TieCount;
				}
				else
				{
					TieCount = 0;
				}

				PrvScore = PRI->Score;
				PrvDeath = PRI->Deaths;

				if ( i >= PrevIndex )
				{
					--Cnt;

					DrawPlayerName( Canvas, Rank - TieCount, PRI == PRIOwner , Max(PRI->Level - 1,0), PRI->Score, MyScore, *PRI->PlayerName, X, Y );
					Y += ColumnSpacing;

					if ( Cnt <= 0 )
						break;
				}
			}
		}
	}
}

void UavaUIGrenadeIndicator::Render_Icon( FCanvas* Canvas, FString IconStr )
{
	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

	FRenderParameters StrParam;
	StrParam.DrawFont = WeaponIconFont;
	UUIString::StringSize(StrParam, *IconStr);
	StrParam.DrawX = X + XL/2.0f - StrParam.DrawXL/2;
	StrParam.DrawY = Y + YL/2.0f - StrParam.DrawYL/2;
	DrawString(Canvas, StrParam.DrawX, StrParam.DrawY, *IconStr, WeaponIconFont, WeaponIconColor);
}

void UavaUIGrenadeIndicator::Render_Indicator( FCanvas* Canvas, FLOAT Angle )
{
	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

	/**	2006/12/11 YTS, 돌려찍기위한 DrawTile
	*	
	*	@param	Canvas - 그릴 대상
	*	@param	Surface - 찍으려고 하는 Texture2D
	*	@param	AxisX - Tex를 돌릴 회전축의 X좌표
	*	@param	AxisY - Tex를 돌릴 회전축의 Y좌표
	*	@param	DegAngle - 회전 각도 ( Degree )
	*	@param	X - 회전축을 0,0으로 한 상대적인 찍을 위치 X좌표
	*	@param	Y - 회전축을 0,0으로 한 상대적인 찍을 위치 Y좌표
	*	@param	XL - 화면에 찍을 너비 (Width)
	*	@param	YL - 화면에 찍을 높이 (Height)
	*	@param	U - TexCoord.U
	*	@param	V - TexCoord.V
	*	@param	UL - TexCoord.UL
	*	@param	VL - TexCoord.VL
	*/

	DrawTileRotate( Canvas, 
					Image, 
					X + XL/2, 
					Y + YL/2, 
					appTrunc(Angle),
					-XL * TopWidth/2.0, 
					-YL/2, 
					XL * TopWidth, 
					YL * TopHeight, 
					TopIndicator.U, 
					TopIndicator.V, 
					TopIndicator.UL, 
					TopIndicator.VL, 
					IndicatorColor );
}

void UavaUIGrenadeIndicator::Render_Widget( FCanvas* Canvas )
{
	if ( GIsEditor && !GIsGame )
	{
		if ( WeaponIconFont != NULL )	Render_Icon( Canvas, TestStr );
		if ( Image != NULL )			Render_Indicator( Canvas, TestAngle );
	}
	else
	{
		AavaPlayerController*	PlayerOwner = GetavaPlayerOwner();
		AavaHUD*				HUD			= PlayerOwner ? Cast<AavaHUD>(PlayerOwner->myHUD) : NULL;
		AavaPawn*				PlayerPawn  = PlayerOwner ? Cast<AavaPawn>( PlayerOwner->Pawn ) : NULL;

		if ( PlayerPawn == NULL || HUD == NULL )		return;
		if ( !PlayerPawn->bDrawGrenadeIndicator )		return;
		if ( HUD->GrenadeForIndicatingList.Num() <= 0 )	return;

		FRotator	ViewRotator		= PlayerPawn->Rotation;
		FVector		ViewLocation	= PlayerPawn->Location;
		FVector		ViewDir			= ViewRotator.Vector();

		AavaProjectile* NearestProjectile = NULL;
		FLOAT			MinDistance = 0.0f;
		if ( Image != NULL )
		{
			ViewDir.Z = 0.0f;
			ViewDir.Normalize();
			FVector YAxis = FVector(0,0,1) ^ ViewDir;

			for( INT i = 0 ; i < HUD->GrenadeForIndicatingList.Num() ; ++ i )
			{
				AavaProjectile* Projectile = HUD->GrenadeForIndicatingList(i);
				if ( Projectile == NULL )
					continue;

				FVector	ProjDirAbs = Projectile->Location - PlayerPawn->Location;
				FLOAT	Distance   = ProjDirAbs.Size();
				if ( Distance > Projectile->DamageRadius )
					continue;

				if ( NearestProjectile == NULL || MinDistance > Distance )
				{
					NearestProjectile = Projectile;
					MinDistance = Distance;
				}

				ProjDirAbs.Z = 0.0f;
				ProjDirAbs.Normalize();
				FLOAT Angle = acosf( ViewDir | ProjDirAbs );
				
				if ( (YAxis | ProjDirAbs) < 0 )
					Angle *= -1;

				Angle /= (2 * 3.141592f);
				Angle *= 360;

				Render_Indicator( Canvas, Angle );
			}
		}

		// Draw Icon
		if ( WeaponIconFont != NULL && NearestProjectile != NULL )
		{
			UClass* AttachmentClass		=	NearestProjectile->weaponBy != NULL ? NearestProjectile->weaponBy->GetDefaultObject<AavaWeapon>()->AttachmentClass : NULL;
			FString WeaponStr			=	AttachmentClass? AttachmentClass->GetDefaultObject<AavaWeaponAttachment>()->DeathIconStr : FString(TEXT(""));
			Render_Icon( Canvas, WeaponStr );
		}
	}
}

void UavaUIWeaponSpread::Render_Progress( FCanvas* Canvas,USurface* Tex, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL )
{
	FLinearColor LC_Box( BackgroundColor ), LC_Progress( ProgressColor ), LC_Alert( AlertColor ), LC_Fade( FadeColor );
	LC_Box.A *= Opacity;
	LC_Progress.A *= Opacity;
	LC_Alert.A *= Opacity;

	Render_Bar( Canvas, X, Y, XL, YL, 0, 1, Tex, CU, CV, CUL, CVL, LC_Box );

	FLOAT PreviousRatio = 0.0f;

	FLinearColor LC;
	if (FadeInfos(FadeInfos.Num()-1).Value < AlertStartRatio)
	{
		LC = LC_Alert;
	}
	else
	{
		LC = LC_Progress;
	}

	LC.A *= Opacity;

	for (INT i=FadeInfos.Num()-1; i>=0; --i)
	{
		const FGaugeFadeInfo& FI = FadeInfos(i);				

		LC.A *= FI.Alpha;

		Render_Bar( Canvas, X, Y, XL, YL, PreviousRatio, FI.Value - PreviousRatio, Tex, CU, CV, CUL, CVL, LC );

		LC = LC_Fade;
		LC.A *= Opacity;

		PreviousRatio = FI.Value;
	}
}

void UavaUIWeaponSpread::Render_Bar( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT Ratio, FLOAT RatioL, USurface* Surface, FLOAT CU, FLOAT CV, FLOAT CUL, FLOAT CVL, const FLinearColor& DrawColor )
{
	if (Surface == NULL) return;

	if (Surface)
	{		
		switch (Direction)
		{
		default :
		case AVAUIPROGRESSDirection_Left :
			DrawTile( Canvas, Surface, X + appTrunc( XL * Ratio ), Y, appTrunc( XL * RatioL ), YL, CU + appTrunc( CUL * Ratio ), CV, appTrunc( CUL * RatioL ), CVL, DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Right :
			DrawTile( Canvas, Surface, X + appTrunc( XL * (1-(Ratio+RatioL)) ), Y, appTrunc( XL * RatioL ), YL, CU + appTrunc( CUL * (1-(Ratio+RatioL)) ), CV, appTrunc( CUL * RatioL ), CVL, DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Up :
			DrawTile( Canvas, Surface, X, Y + appTrunc( YL * Ratio ), XL, appTrunc( YL * RatioL ), CU, CV + appTrunc( CVL * Ratio ), CUL, appTrunc( CVL * RatioL ), DrawColor );	
			break;

		case AVAUIPROGRESSDirection_Down :
			DrawTile( Canvas, Surface, X, Y + appTrunc( YL * (1-(Ratio+RatioL)) ), XL, appTrunc( YL * RatioL ), CU, CV + appTrunc(CVL * (1 - (Ratio+RatioL))), CUL, appTrunc( CVL * RatioL ), DrawColor );	
			break;							
		}		
	}
}

void UavaUIWeaponSpread::Render_Widget( FCanvas* Canvas )
{
	AavaPlayerController*	PlayerOwner		= GetavaPlayerOwner();
	AavaPawn*				PawnOwner		= GetPlayerViewTarget();
	AavaWeapon*				CurrentWeapon	= GetViewTargetCurrentWeapon();

	if ( GIsEditor && !GIsGame )
	{
		Ratio = TestRatio;
	}
	else
	{
		if( PawnOwner == NULL || CurrentWeapon == NULL )
			return;

		if ( PlayerOwner->ViewTarget != NULL )
		{
			APawn* Pawn = Cast<APawn>(PlayerOwner->ViewTarget);
			if ( Pawn != NULL && !Pawn->IsLocallyControlled() )	
				return;
		}
		AavaWeap_BaseGun* BaseGun = Cast<AavaWeap_BaseGun>(CurrentWeapon);
		// SightMode이거나 FOV가 평상시의 FOV로 돌아오지 않았을때도 그리지 않는다.
		if ( BaseGun != NULL && 
			 BaseGun->bDisplaySpreadInfoInSightMode &&
			 BaseGun->bHideCursorInSightMode && 
			 BaseGun->ScopeComp != NULL && 
			 !BaseGun->ScopeComp->HiddenGame )
		{
			Ratio = BaseGun->CurrentSpread / BaseGun->DisplayedSpreadMax;
		}
		else
		{
			return;
		}
	}


	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

	UpdateFadeInfos();
	FLinearColor LC_Box( BackgroundColor );
	LC_Box.A *= Opacity;			
	Render_Bar( Canvas, X, Y, XL, YL, 0, 1, BackgroundImage, BackgroundCoordinates.U, BackgroundCoordinates.V, BackgroundCoordinates.UL, BackgroundCoordinates.VL, LC_Box );
	Direction=AVAUIPROGRESSDirection_Down;
	Render_Progress( Canvas, Image, appTrunc(X), appTrunc(Y), appTrunc(XL), appTrunc(YL/2.0f), ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );		
	Direction=AVAUIPROGRESSDirection_Up;
	Render_Progress( Canvas, Image, appTrunc(X), appTrunc(Y + YL/2.0f), appTrunc(XL), appTrunc(YL/2.0f), ImageCoordinates.U, ImageCoordinates.V, ImageCoordinates.UL, ImageCoordinates.VL );		
}