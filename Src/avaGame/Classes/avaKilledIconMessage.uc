/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
//
// A Death Message.
//
// Switch 0: Kill
//	RelatedPRI_1 is the Killer.
//	RelatedPRI_2 is the Victim.
//	OptionalObject is the DamageType Class.
//

class avaKilledIconMessage extends avaLocalMessage;

static function color GetConsoleColor( PlayerReplicationInfo RelatedPRI_1 )
{
    return class'HUD'.Default.GreenColor;
}

static function string GetString(
	optional int Switch,
	optional bool bPRI1HUD,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	return "";
}

static function ClientReceive(
	PlayerController P,
	optional int Switch,
	optional PlayerReplicationInfo RelatedPRI_1,
	optional PlayerReplicationInfo RelatedPRI_2,
	optional Object OptionalObject
	)
{
	local avaHUD avaHUD;
	local class<avaWeaponAttachment> avaWA;
	local string WeaponStr;
	local int PlayerTeam1;
	local bool bDamageHead;
	local bool bWallShot;
	local avaPawn	SenderPawn;
	local string	DamageCode;
	
	avaHUD = avaHUD(P.myHUD);

	if( avaHUD == None )
		return;

	// 일반 DamageType
	if( Switch == 0)
	{
		if ( OptionalObject == class'avaDmgType_C4' )	DamageCode = "i";
		else											DamageCode = "R";
		avaHUD.KillMessage(RelatedPRI_1, RelatedPRI_2, class<avaDmgType_Explosion>(OptionalObject) != None , false, false );
		avaHUD.DeathMessage( None, avaPlayerReplicationInfo( RelatedPRI_2 ), DamageCode, None, false, false );
	}
	// 무기에 의한 죽음
	else 
	{
		avaWA = class<avaWeaponAttachment>( OptionalObject );

		if( avaWA != None )
		{
			WeaponStr = avaWA.default.DeathIconStr;
		}
		else
		{
			WeaponStr = "R";
		}
		
		if( RelatedPRI_1.Team != None )
			PlayerTeam1 = RelatedPRI_1.Team.TeamIndex;

		// Switch 번호를 가지고, Headshot 판별
		switch( Switch )
		{
		case 1:											break;		// 일반
		case 2:	bDamageHead = true;						break;		// HeadShot
		case 3:	bWallShot = true;						break;		// WallShot
		case 4:	bDamageHead = true; bWallShot = true;	break;		// Wall-HeadShot
		}

		// [2006/10/31 otterrrr] KillMessage 추가
		avaHUD.KillMessage(RelatedPRI_1, RelatedPRI_2, avaWA.default.DamageCode == 'Grenade', bDamageHead, bWallShot );
		
		if( RelatedPRI_1.Team != None && RelatedPRI_2.Team != None )
		{
			if ( PlayerTeam1 != RelatedPRI_2.Team.TeamIndex )
			{
				SenderPawn = avaPlayerReplicationInfo( RelatedPRI_1).GetAvaPawn();
				if ( SenderPawn != None )
					SenderPawn.UpdateKillTime();
			}
            avaHUD.DeathMessage( avaPlayerReplicationInfo( RelatedPRI_1 ) , avaPlayerReplicationInfo( RelatedPRI_2 ), WeaponStr, None, bDamageHead, bWallShot );
		}
		else
			avaHUD.DeathMessage( avaPlayerReplicationInfo( RelatedPRI_1 ), avaPlayerReplicationInfo( RelatedPRI_2 ), WeaponStr, None, bDamageHead, bWallShot );
	}
}

defaultproperties
{
	DrawColor=(R=255,G=0,B=0,A=255)
	bIsSpecial=True
	bIsUnique=True
}
