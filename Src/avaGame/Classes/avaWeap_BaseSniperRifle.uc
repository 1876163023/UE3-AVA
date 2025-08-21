/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaWeap_BaseSniperRifle extends avaWeap_BaseGun;

`include(avaGame/avaGame.uci)

//var() StaticMeshComponent	ScopeComp;
//var string					ScopeMeshName;

simulated function AttachScope()
{
	local StaticMesh	tempStaticMesh;
	local vector		translation;
	if ( ScopeMeshName != "" )
	{
		tempStaticMesh = StaticMesh( DynamicLoadObject( ScopeMeshName, class'StaticMesh' ) );
		translation		= ScopeComp.Translation;
		Translation.x	= 25.0; 
		translation.y	= -PlayerViewOffset.y;
		translation.z	= -PlayerViewOffset.z;
		ScopeComp.SetTranslation( translation );
		StaticMeshComponent(ScopeComp).SetStaticMesh( tempStaticMesh );
		ScopeComp.SetShadowParent( Mesh );
		ScopeComp.SetOcclusionGroup( Mesh );		
		ScopeComp.SetHidden( true );
	}
}

simulated function AttachItems()
{
	Super.AttachItems();
	AttachScope();
}

simulated function SetLightEnvironment( LightEnvironmentComponent env )
{
	Super.SetLightEnvironment( env );
	//if (ScopeComp != none)
	//{
	//	ScopeComp.SetLightEnvironment( env );
	//}
}

simulated function DrawWeaponCrosshair( Hud HUD )
{
}

//simulated function SetPosition(avaPawn Holder)
//{
//	local vector DrawOffset, ViewOffset;
//	local avaPawn.EWeaponHand CurrentHand;
//	local rotator NewRotation;
//
//	if ( Instigator == None )	return;
//
//	if ( !Instigator.IsFirstPerson() )
//		return;
//
//	// Hide the weapon if hidden
//	CurrentHand = GetHand();
//	if ( CurrentHand == HAND_Hidden)
//	{
//		SetHidden( true );
//		return;
//	}
//	SetHidden( false );
//
//	// Adjust for the current hand
//	ViewOffset = Vect(0,0,0);
//	switch ( CurrentHand )
//	{
//		case HAND_Left:
//			ViewOffset.Y *= -1;
//			break;
//
//		case HAND_Centered:
//			ViewOffset.Y = 0;
//			break;
//	}
//
//	// Calculate the draw offset
//	if ( Holder.Controller == None )
//		DrawOffset = (ViewOffset >> Rotation) + Holder.GetEyeHeight() * vect(0,0,1);
//	else
//	{
//		DrawOffset.Z = Holder.GetEyeHeight();
//		if ( Holder.bWeaponBob )
//		{
//			DrawOffset += Holder.WeaponBob(BobDamping,JumpDamping);
//		}
//
//		if ( avaPlayerController(Holder.Controller) != None )
//		{
//			DrawOffset += avaPlayerController(Holder.Controller).ShakeOffset >> Holder.Controller.Rotation;
//		}
//
//		DrawOffset = DrawOffset + ( ViewOffset >> Holder.Controller.Rotation );
//	}
//
//	// Adjust it in the world
//	ScopeComp.SetTranslation( Holder.Location + DrawOffset );
//
//	// 2006/7/20  �Ʒ�ó�� �ϸ� PunchAngle �� ������ �ȵǱ� ������ �����Ͽ���... by OZ
//	Holder.Controller.GetPlayerViewPoint( ViewOffset, NewRotation );
//	//NewRotation = (Holder.Controller == None ) ? Holder.Rotation : Holder.Controller.Rotation;
//
//	// �Ʒ� Code �� �����ϸ� ���Ⱑ �ʰ� ������� ������ ����� ���� �� �־ ����...
//	//if ( Holder.bWeaponBob )
//	//{
//	//	// if bWeaponBob, then add some rotation lag
//	//	NewRotation.Yaw = LagRot(NewRotation.Yaw & 65535, Rotation.Yaw & 65535, MaxYawLag);
//	//	NewRotation.Pitch = LagRot(NewRotation.Pitch & 65535, Rotation.Pitch & 65535, MaxPitchLag);
//	//}
//	ScopeComp.SetRotation(NewRotation);
//
//	Super.SetPosition( Holder );
//}

simulated function DrawZoomedOverlay( HUD H )
{
	//local float DrawImageLength, ImagePosX, ImagePosY;
	//H.Canvas.SetDrawColor(0,0,0);
	//if( H.Canvas.ClipX > H.Canvas.ClipY )
	//{
	//	DrawImageLength = H.Canvas.ClipY;
	//	ImagePosX = (H.Canvas.ClipX - H.Canvas.ClipY)/2;
	//	ImagePosY = 0;

	//	// �׸��� ���� �κ��� �������.
	//	H.Canvas.SetPos(0, 0);
	//	H.Canvas.DrawRect((H.Canvas.ClipX - H.Canvas.ClipY)/2, H.Canvas.ClipY);

	//	// �׸��� ������ �κ��� �������
	//	H.Canvas.SetPos( (H.Canvas.ClipX + H.Canvas.ClipY)/2, 0);
	//	H.Canvas.DrawRect((H.Canvas.ClipX - H.Canvas.ClipY)/2+1, H.Canvas.ClipY);
	//}
	//else
	//{
	//	DrawImageLength = H.Canvas.ClipX;
	//	ImagePosX = 0;
	//	ImagePosY = (H.Canvas.ClipY - H.Canvas.ClipX)/2;

	//	// �׸��� ���� �κ��� �������.
	//	H.Canvas.SetPos(0, 0);
	//	H.Canvas.DrawRect(H.Canvas.ClipX, (H.Canvas.ClipY - H.Canvas.ClipX)/2);

	//	// �׸��� �Ʒ��� �κ��� �������
	//	H.Canvas.SetPos(0, (H.Canvas.ClipX + H.Canvas.ClipY)/2);
	//	H.Canvas.DrawRect(H.Canvas.ClipX, (H.Canvas.ClipY - H.Canvas.ClipX)/2);
	//}
	//H.Canvas.SetDrawColor(255, 255, 255);
	//H.Canvas.SetPos( ImagePosX, ImagePosY );

	////HudMtrl
	//H.Canvas.DrawMaterialTile( HudMtrl, DrawImageLength, DrawImageLength, 0, 0, 1.0, 1.0);
	//H.Canvas.DrawTile(HudMaterial, DrawImageLength, DrawImageLength, 0, 0, HudMaterial.SizeX, HudMaterial.SizeY);
}

simulated function bool SwitchSightMode( int requestMode, float transitionTime, optional bool bPlayAnim = true )
{
	local bool bSuccess;
	bSuccess = Super.SwitchSightMode( requestMode, transitionTime, bPlayAnim );
	if ( bSuccess && Role == ROLE_Authority )
	{
		avaPlayerController(avaPawn(Instigator).Controller).ServerWeaponZoomModeChange();
	}
	return bSuccess;
}

`devexec simulated function AdjustScope(string cmd)
{
	local string c,v;
	local vector s,k;
	local rotator r;
	local float sc;

	c = left(Cmd,InStr(Cmd,"="));
	v = mid(Cmd,InStr(Cmd,"=")+1);

	r  = ScopeComp.Rotation;
	s  = ScopeComp.Scale3D;
	sc = ScopeComp.Scale;
	k  = ScopeComp.Translation;

	if (c~="kx")  k.X += float(v);
	if (c~="kax") k.X =  float(v);
	if (c~="ky")  k.Y += float(v);
	if (c~="kay") k.Y =  float(v);
	if (c~="kz")  k.Z += float(v);
	if (c~="kaz") k.Z =  float(v);

	if (c~="r")   R.Roll  += int(v);
	if (c~="ar")  R.Roll  =  int(v);
	if (c~="p")   R.Pitch += int(v);
	if (c~="ap")  R.Pitch =  int(v);
	if (c~="w")   R.Yaw   += int(v);
	if (c~="aw")  R.Yaw   =  int(v);

	if (c~="scalex") s.X = float(v);
	if (c~="scaley") s.Y = float(v);
	if (c~="scalez") s.Z = float(v);

	if (c~="scale") sc = float(v);

	if (c~="resetscale")
	{
		sc = 1.0;
		s.X = 1.0;
		s.Y = 1.0;
		s.Z = 1.0;
	}

	ScopeComp.SetTranslation(k);
	ScopeComp.SetRotation(r);
	ScopeComp.SetScale(sc);
	ScopeComp.SetScale3D(s);	

	`log("#### AdjustScope ####");
	`log("####    Translation :"@ScopeComp.Translation);
	`log("####    Rotation    :"@ScopeComp.Rotation);
	`log("####    Scale3D     :"@ScopeComp.Scale3D);
	`log("####    scale       :"@ScopeComp.Scale);
}


defaultproperties
{
	InventoryGroup=1
	WeaponColor=(R=255,G=0,B=64,A=255)

	// Weapon SkeletalMesh
	InstantHitDamage(0)=70
	InstantHitDamage(1)=0
	InstantHitMomentum(0)=10000.0

	FireInterval(0)=+1.2
	FireInterval(1)=+1.2	

	bAutoFire=False

	WeaponRange=17000

	InstantHitDamageTypes(0)=class'avaDmgType_Gun'
	
	//WeaponFireSnd=SoundCue'A_Weapon.SniperRifle.Cue.A_Weapon_SN_Fire01_Cue'	

	WeaponFireTypes(0)=EWFT_InstantHit

	ShouldFireOnRelease(0)=0
	ShouldFireOnRelease(1)=0
	GroupWeight=0.5

	//PickupSound=SoundCue'A_Pickups.Weapons.Cue.A_Pickup_Weapons_Sniper_Cue'

	AmmoCount=10
	MaxAmmoCount=40

	MuzzleFlashSocket=MuzzleFlashSocket
	MuzzleFlashPSCTemplate=ParticleSystem'avaEffect.Particles.P_WP_SniperRifle_MuzzleFlash_1P'
	MuzzleFlashDuration=0.05
	
	WeaponFireAnim(0)	=Fire
 	WeaponPutDownAnim	=Down
	WeaponEquipAnim		=BringUp
	WeaponReloadAnim	=Reload
	EquipTime			=+1.2
	PutDownTime			=+0.0333
	ReloadTime			=3.0333
	WeaponIdleAnims(0)	=Idle

	// Weapon �� ���� Pawn �� �ӵ� ����
	BaseSpeed			= 240	// �⺻�ӵ�
	AimSpeedPct			= 0.6	// ���ؽ� ����ġ
	WalkSpeedPct		= 0.4	// �ȱ�� ����ġ
	CrouchSpeedPct		= 0.25	// �ɾ��̵��� ����ġ
	CrouchAimSpeedPct	= 0.1	// �ɾƼ� ���� �̵��� ����ġ
	SwimSpeedPct		= 0.7	// ������ ����ġ
	SprintSpeedPct		= 1.25	// ������Ʈ�� ����ġ
	CrouchSprintSpeedPct= 1	// �ɾƼ� ������Ʈ�� ����ġ

	BulletTemplete		= ParticleSystem'avaEffect.Gun_Effect.Ps_Wp_SniperRifle_cartridge'

	WeaponType			= WEAPON_SNIPER

	UnZoomPenalty		= 0.4

	Begin Object Class=StaticMeshComponent Name=ScopeComponent0
		bOnlyOwnerSee=true
		DepthPriorityGroup=SDPG_Foreground
		Rotation=(Yaw=16384)
		Translation=(X=25.0,Y=4.0,Z=-1.0)
		bCastDynamicShadow=false
	End Object
	ScopeComp=ScopeComponent0
	Components.Add(ScopeComponent0)


	bAvailableInstantZoomFire = False
	DisplayedSpreadMax = 0.1
	bDisplaySpreadInfoInSightMode = TRUE
}
