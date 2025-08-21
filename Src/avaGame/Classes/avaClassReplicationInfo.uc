/*=============================================================================
	avaClassReplicationInfo
 
	Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.

	2006/12/21

		1. Class 별로 Replication을 해야할 정보들을 관리하기 위해서 추가함...

=============================================================================*/
class avaClassReplicationInfo extends ReplicationInfo
	native
	dependson(avaWeapon);

`include(avaGame/avaGame.uci)

var hmserialize int	PlayTime;								// 플레이한 시간(Seconds)
var	hmserialize int	SpawnCount;								// 스폰한 횟수
var	hmserialize int	SprintTime;								// 스프린트한 시간(Seconds)

var hmserialize int	KillCount;								// 죽인수

var hmserialize int HitCount[`MAX_WEAPON_TYPE];				// 각 무기 Type 별 Hit 횟수
var int FireCount[`MAX_WEAPON_TYPE];			// 각 무기 Type 별 발사 횟수
var	hmserialize int	HeadshotHitCount[`MAX_WEAPON_TYPE];		// 각 무기 Type 별 Head Hit Count
var	hmserialize int	HeadshotKillCount[`MAX_WEAPON_TYPE];	// 각 무기 Type 별 HeadShot Kill Count
var	hmserialize int	WeaponDamage[`MAX_WEAPON_TYPE];			// 각 무기 Type 별로 준 Damage
var hmserialize int WeaponKillCount[`MAX_WEAPON_TYPE];		// 각 무기 Type 별 Kill 수

var	hmserialize int	TakenDamage;							// 받은 Damage
//var hmserialize int DeathCount;								// Death Count

var int	LastSpawnTime;
var int	LastSprintTime;

replication
{
	if ( bNetDirty && ROLE==ROLE_Authority )
		PlayTime, SpawnCount, SprintTime,
		HitCount, HeadShotHitCount, HeadShotKillCount, WeaponDamage, WeaponKillCount,
		TakenDamage, KillCount;//, DeathCount;
}

//function AddDeathCount()
//{
//	++DeathCount;
//}

function ClearScore()
{
	local int i;
	PlayTime		=	0;
	SpawnCount		=	0;
	SprintTime		=	0;
	KillCount		=	0;
	TakenDamage		=	0;
	for ( i = 0 ; i < `MAX_WEAPON_TYPE ; ++ i )
	{
		HitCount[i]				=	0;
		FireCount[i]			=	0;
		HeadshotHitCount[i]		=	0;
		HeadshotKillCount[i]	=	0;
		WeaponDamage[i]			=	0;
		WeaponKillCount[i]		=	0;
	}
}

// Kill 수를 증가시킨다...
function AddKillCount( int weaponType, bool bHeadShot )
{
	++KillCount;
	if ( weaponType < 0 || weaponType > `MAX_WEAPON_TYPE )	return;
	++WeaponKillCount[weaponType];
	if ( bHeadShot == true )	++HeadShotKillCount[weaponType];
}

// 따로 Replication 을 하지는 말자....
simulated function AddFireCount( int weaponType )
{
	++FireCount[weaponType];
}

// Hit 수를 증가시킨다...
function AddHitCount( int weaponType, int nDamage, bool bHeadHit )
{
	if ( weaponType < 0 || weaponType > `MAX_WEAPON_TYPE )	return;
	WeaponDamage[weaponType] += nDamage;
	if ( bHeadHit == true )		++HeadShotHitCount[weaponType];
	++HitCount[weaponType];
}

function DamageTaken( int nDamage )
{
	TakenDamage += nDamage;
} 

function SpawnStart()
{
	LastSpawnTime = WorldInfo.TimeSeconds;
	++SpawnCount;
}

function SpawnEnd()
{
	if ( LastSpawnTime == 0 )	return;
	PlayTime += (WorldInfo.TimeSeconds - LastSpawnTime);
	LastSpawnTime = 0;
}

function SprintStat( bool bStart )
{
	if ( bStart )
	{
		LastSprintTime = WorldInfo.TimeSeconds;
	}
	else
	{
		if ( LastSprintTime == 0 )	return;
		SprintTime	+=	(WorldInfo.TimeSeconds - LastSprintTime );
		LastSprintTime = 0;
	}
}

defaultproperties
{

}