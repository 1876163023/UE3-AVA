/*

	Player 의 Modifier 에 대한 정보를 보관한다.

	1. ClassType 별 Default Modifier 가 있다.
	2. ClassType 별 Common Character Modifier 를 등록한다.


	AddModifier 와 RemoveModifier 에 대한 Function 을 제공한다.
	String 을 인자로 받아서 Modifier 로 변환시켜 보관한다.

	각자 만들어서 쓰며, Network 으로 Sync 를 하지는 않는다.
	PIP 창과 연계하여 PIP Window 에 이 정보를 넘겨 줄 수 있도록 한다.

	
	avaPlayerReplicationInfo 에서 avaPlayerModifierInfo 를 각자 Spawn 해서 사용하도록 한다...
	avaPlayerModifierInfo 를 만들면서 각 Class Type 별 Default Modifier 를 등록한다...
	서버에서 받은 Common Character Modifier 를 등록한다.
	서버에서 받은 Character Type 별 Modifier 를 등록한다.

*/
class avaPlayerModifierInfo extends Info;

`include(avaGame/avaGame.uci)

struct WeaponInfo
{
	var class< avaWeapon >						Class;				// Weapon Class
	var int										MaintenanceRate;	// Weapon 의 정비도...
	var array< class<avaMod_Weapon> >			Mod;				// 각 Weapon 별 Modifier
};

struct ClassTypeInfo												// Character Class 에 의해서 구분되어지는 Modifier
{
	var array< WeaponInfo >						WeaponInfos;		// Weapon Modifier
	var array< class<avaCharacterModifier> >	CharMod;			// 병과구분에 의한 Character Modifier
};

var	ClassTypeInfo								ClassTypeInfos[`MAX_PLAYER_CLASS];	//

//! 인벤토리에서 장비를 장착하는 경우 다시 호출하기 때문에 초기화 하는 코드를 추가함(2007/02/07 고광록).
simulated function ResetCharacterModifiers()
{
	local int i;

	for ( i = 0; i < `MAX_PLAYER_CLASS; ++ i )
		ClassTypeInfos[i].CharMod.Length = 0;
}

// string 을 character 용 modifier 로 변환해서 등록한다.
simulated function FetchCharMod( string Code, optional int ClassType = -1 )
{
	if ( Code == "" )	return;
	class'avaMod_TranslatorBase'.static.FetchCharModifiers( class'avaNetHandler'.static.GetAvaNetHandler().CharacterModifierList, 
															self, 
															Code, 
															ClassType );
}

// Character Modifier 를 등록한다...
simulated function AddCharMod( int ClassType, class<avaCharacterModifier> Mod )
{
	local int	nSize;
	local int	i;
	
	if ( Mod == None )	return;
	// 이미 등록되어 있다면 무시하도록 하자.
	for ( i = 0 ; i < ClassTypeInfos[ClassType].CharMod.length ; ++ i )
	{
		if ( ClassTypeInfos[ClassType].CharMod[i] == Mod )	
			return;
	}
	// Modifier 를 등록한다.
	nSize = ClassTypeInfos[ClassType].CharMod.length;
	ClassTypeInfos[ClassType].CharMod[nSize] = Mod;

	`log( "AddCharMod" @ClassType @Mod );
}

//! 인벤토리에서 무기를 장착하는 경우 다시 호출하기 때문에 초기화 하는 코드를 추가함(2007/02/07 고광록).
simulated function ResetWeaponModifiers()
{
	local int i;

	for ( i = 0; i < `MAX_PLAYER_CLASS; ++ i )
		ClassTypeInfos[i].WeaponInfos.Length = 0;
}

// string 을 weapon 용 modifier 로 변환해서 등록한다.
simulated function FetchWeaponMod( string Code, int ClassType )
{
	if ( Code == "" )	return;
	class'avaMod_TranslatorBase'.static.FetchWeapModifiers( class'avaNetHandler'.static.GetAvaNetHandler().WeaponModifierList,
															self,
															Code,
															ClassType );
}

// weapon 을 add 한다.
simulated function AddWeapon( int ClassType, class<avaWeapon> WeapClass, int MaintenanceRate )
{
	//local	int			i;
	local	int			nSize;
	local	WeaponInfo	weapInfo;

	if ( WeapClass == None )	return;
	// 중복되는 Weapon 이 있는지 Check 한다...
	// Weapon 을 중복해서 가질수 있음... ex) 수류탄...
	//for ( i = 0 ; i < ClassTypeInfos[ClassType].WeaponInfos.length ; ++ i )
	//{
	//	if ( ClassTypeInfos[ClassType].WeaponInfos[i].Class == WeapClass )
	//		return;
	//}
	// Weapon 을 등록한다...
	weapInfo.Class				= WeapClass;
	weapInfo.MaintenanceRate	= MaintenanceRate;
	nSize = ClassTypeInfos[ClassType].WeaponInfos.length;
	ClassTypeInfos[ClassType].WeaponInfos[nSize] = weapInfo;
}

// weapon 에 weapon 용 modifier 를 add 한다.
simulated function AddWeaponModifier( int ClassType, class<avaWeapon> WeapClass, class<avaMod_Weapon> Mod )
{
	local	int			i;
	local	int			j;
	local	int			nSize;

	if ( Mod == None )	return;
	for ( i = 0 ; i < ClassTypeInfos[ClassType].WeaponInfos.length ; ++ i )
	{
		if ( ClassTypeInfos[ClassType].WeaponInfos[i].Class == WeapClass )
		{
			// 중복되는 Modifier 가 있는지 Check 한다...
			for ( j = 0 ; j < ClassTypeInfos[ClassType].WeaponInfos[i].Mod.length ; ++ j )
			{
				if ( ClassTypeInfos[ClassType].WeaponInfos[i].Mod[j] == Mod )
					return;
			}
			nSize = ClassTypeInfos[ClassType].WeaponInfos[i].Mod.length;
			ClassTypeInfos[ClassType].WeaponInfos[i].Mod[nSize] = Mod;
			return;
		}
	}
}

defaultproperties
{
	RemoteRole=ROLE_None
}