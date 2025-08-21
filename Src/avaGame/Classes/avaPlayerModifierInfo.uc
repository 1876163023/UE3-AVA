/*

	Player �� Modifier �� ���� ������ �����Ѵ�.

	1. ClassType �� Default Modifier �� �ִ�.
	2. ClassType �� Common Character Modifier �� ����Ѵ�.


	AddModifier �� RemoveModifier �� ���� Function �� �����Ѵ�.
	String �� ���ڷ� �޾Ƽ� Modifier �� ��ȯ���� �����Ѵ�.

	���� ���� ����, Network ���� Sync �� ������ �ʴ´�.
	PIP â�� �����Ͽ� PIP Window �� �� ������ �Ѱ� �� �� �ֵ��� �Ѵ�.

	
	avaPlayerReplicationInfo ���� avaPlayerModifierInfo �� ���� Spawn �ؼ� ����ϵ��� �Ѵ�...
	avaPlayerModifierInfo �� ����鼭 �� Class Type �� Default Modifier �� ����Ѵ�...
	�������� ���� Common Character Modifier �� ����Ѵ�.
	�������� ���� Character Type �� Modifier �� ����Ѵ�.

*/
class avaPlayerModifierInfo extends Info;

`include(avaGame/avaGame.uci)

struct WeaponInfo
{
	var class< avaWeapon >						Class;				// Weapon Class
	var int										MaintenanceRate;	// Weapon �� ����...
	var array< class<avaMod_Weapon> >			Mod;				// �� Weapon �� Modifier
};

struct ClassTypeInfo												// Character Class �� ���ؼ� ���еǾ����� Modifier
{
	var array< WeaponInfo >						WeaponInfos;		// Weapon Modifier
	var array< class<avaCharacterModifier> >	CharMod;			// �������п� ���� Character Modifier
};

var	ClassTypeInfo								ClassTypeInfos[`MAX_PLAYER_CLASS];	//

//! �κ��丮���� ��� �����ϴ� ��� �ٽ� ȣ���ϱ� ������ �ʱ�ȭ �ϴ� �ڵ带 �߰���(2007/02/07 ����).
simulated function ResetCharacterModifiers()
{
	local int i;

	for ( i = 0; i < `MAX_PLAYER_CLASS; ++ i )
		ClassTypeInfos[i].CharMod.Length = 0;
}

// string �� character �� modifier �� ��ȯ�ؼ� ����Ѵ�.
simulated function FetchCharMod( string Code, optional int ClassType = -1 )
{
	if ( Code == "" )	return;
	class'avaMod_TranslatorBase'.static.FetchCharModifiers( class'avaNetHandler'.static.GetAvaNetHandler().CharacterModifierList, 
															self, 
															Code, 
															ClassType );
}

// Character Modifier �� ����Ѵ�...
simulated function AddCharMod( int ClassType, class<avaCharacterModifier> Mod )
{
	local int	nSize;
	local int	i;
	
	if ( Mod == None )	return;
	// �̹� ��ϵǾ� �ִٸ� �����ϵ��� ����.
	for ( i = 0 ; i < ClassTypeInfos[ClassType].CharMod.length ; ++ i )
	{
		if ( ClassTypeInfos[ClassType].CharMod[i] == Mod )	
			return;
	}
	// Modifier �� ����Ѵ�.
	nSize = ClassTypeInfos[ClassType].CharMod.length;
	ClassTypeInfos[ClassType].CharMod[nSize] = Mod;

	`log( "AddCharMod" @ClassType @Mod );
}

//! �κ��丮���� ���⸦ �����ϴ� ��� �ٽ� ȣ���ϱ� ������ �ʱ�ȭ �ϴ� �ڵ带 �߰���(2007/02/07 ����).
simulated function ResetWeaponModifiers()
{
	local int i;

	for ( i = 0; i < `MAX_PLAYER_CLASS; ++ i )
		ClassTypeInfos[i].WeaponInfos.Length = 0;
}

// string �� weapon �� modifier �� ��ȯ�ؼ� ����Ѵ�.
simulated function FetchWeaponMod( string Code, int ClassType )
{
	if ( Code == "" )	return;
	class'avaMod_TranslatorBase'.static.FetchWeapModifiers( class'avaNetHandler'.static.GetAvaNetHandler().WeaponModifierList,
															self,
															Code,
															ClassType );
}

// weapon �� add �Ѵ�.
simulated function AddWeapon( int ClassType, class<avaWeapon> WeapClass, int MaintenanceRate )
{
	//local	int			i;
	local	int			nSize;
	local	WeaponInfo	weapInfo;

	if ( WeapClass == None )	return;
	// �ߺ��Ǵ� Weapon �� �ִ��� Check �Ѵ�...
	// Weapon �� �ߺ��ؼ� ������ ����... ex) ����ź...
	//for ( i = 0 ; i < ClassTypeInfos[ClassType].WeaponInfos.length ; ++ i )
	//{
	//	if ( ClassTypeInfos[ClassType].WeaponInfos[i].Class == WeapClass )
	//		return;
	//}
	// Weapon �� ����Ѵ�...
	weapInfo.Class				= WeapClass;
	weapInfo.MaintenanceRate	= MaintenanceRate;
	nSize = ClassTypeInfos[ClassType].WeaponInfos.length;
	ClassTypeInfos[ClassType].WeaponInfos[nSize] = weapInfo;
}

// weapon �� weapon �� modifier �� add �Ѵ�.
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
			// �ߺ��Ǵ� Modifier �� �ִ��� Check �Ѵ�...
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