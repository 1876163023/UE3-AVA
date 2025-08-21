/*=============================================================================
  avaMod_TranslatorBase
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/08/30 by OZ

		String 을 Modifier 로 변환시켜주는 Class 이다.
		Decoding 도 여기에 들어가면 된다
=============================================================================*/

class avaMod_TranslatorBase extends Object
	abstract;

`include(avaGame/avaGame.uci)

static function bool GrabCode( string key, out string Options, out string Result, out string Remainder )
{
	local int Pos;
	if ( Options == "" )	return false;

	Pos = InStr(Options,key); 
	if( Pos >= 0 )
	{
		Result		= Left( Options, Pos );
		Remainder	= Right( Options, Pos );
		Options		= Mid( Options, Pos + 1 );
		return true;
	}
	else
	{
		Result = Options;
		Options = "";
		return true;
	}
	return false;
}

static function class<avaModifier> GetModifier( array< class<avaModifier> > ModList, int id )
{
	local int i;
	for ( i = 0 ; i < ModList.length ; ++i )
	{
		if ( ModList[i].default.Id == Id )
			return ModList[i];
	}
	return None;
}

static function GetCharacterModifierList( string code, array< class<avaModifier> > ModList, out array< class<object> > OutList )
{
	local string Value;
	local string Remainder;	
	local int	 index;
	OutList.length = 0;
	while( GrabCode( ";", code, Value, Remainder ) )
	{
		index = OutList.length;
		OutList.length = index + 1;
		OutList[index] = GetModifier( ModList, int(Value) );
	}
}

static function FetchCharModifiers( array< class<avaModifier> > ModList, avaPlayerModifierInfo avaPMI, string code, optional int nType = -1 )
{
	local int	 i,j;
	local array< class<object> >	outList;
	GetCharacterModifierList( code, ModList, outList );
	for ( i = 0 ; i < outList.length ; ++ i )
	{
		if ( nType >= 0 && nType < `MAX_PLAYER_CLASS )
		{
			avaPMI.AddCharMod( nType, class<avaCharacterModifier>(outList[i]) );
		}
		else
		{
			for ( j = 0 ; j < `MAX_PLAYER_CLASS ; ++ j )
			{
				avaPMI.AddCharMod( j, class<avaCharacterModifier>(outList[i]) );
			}
		}
	}

	// code 를 분리한다.
	//local string Value;
	//local string Remainder;
	//local class< avaModifier >	Modifier;
	//while( GrabCode( ";", code, Value, Remainder ) )
	//{
	//	Modifier = GetModifier( ModList, int(Value) );
	//	if ( Modifier != None )
	//	{
	//		if ( nType >= 0 && nType < `MAX_PLAYER_CLASS )
	//		{
	//			avaPMI.AddCharMod( nType, class<avaCharacterModifier>(Modifier) );
	//		}
	//		else
	//		{
	//			for ( i = 0 ; i < `MAX_PLAYER_CLASS ; ++ i )
	//			{
	//				avaPMI.AddCharMod( i, class<avaCharacterModifier>(Modifier) );
	//			}
	//		}
	//	}
	//}
}

static function GetWeaponModifierList( string code, array< class<avaModifier> > ModList, optional out array< class<object> > OutList )
{
	local string				Weapons;
	local string				WeaponMod;
	local string				Remainder;
	local int					index;
	local class< avaModifier >	Modifier;
	while( GrabCode( ";", code, Weapons, Remainder ) )
	{
		while( GrabCode( "@", Weapons, WeaponMod, Remainder ) )
		{
			Modifier = GetModifier( ModList, int(WeaponMod) );
			index = OutList.length;
			OutList.length = index + 1;
			if ( class< avaMod_Weapon >( Modifier ).default.WeaponClass != None )
				OutList[index] = class< avaMod_Weapon >( Modifier ).default.WeaponClass;
			else
				OutList[index] = Modifier;
		}
	}
}

static function FetchWeapModifiers( array< class<avaModifier> > ModList, avaPlayerModifierInfo avaPMI, string code, int nType )
{
	local string				Weapons;
	local string				WeaponMod;
	local string				Remainder;

	local class< avaModifier >	Modifier;
	local class< avaWeapon >	WeaponClass;
	local int					MaintenanceRate;
	local int					MaintenancePos;	

	GetWeaponModifierList( code,  ModList );

	while( GrabCode( ";", code, Weapons, Remainder ) )
	{
		MaintenancePos	=	InStr( Weapons, "*" );
		MaintenanceRate	=	-1;
		if ( MaintenancePos >= 0 )
		{
			Remainder = Right( Weapons, Len( Weapons ) - ( MaintenancePos + 1 ) );
			Weapons	  = Left( Weapons, MaintenancePos );
			MaintenanceRate = int( Remainder );
		}

		while( GrabCode( "@", Weapons, WeaponMod, Remainder ) )
		{
			Modifier = GetModifier( ModList, int(WeaponMod) );
			if ( Modifier != None )
			{
				if ( class< avaMod_Weapon > ( Modifier ).default.WeaponClass != None )
				{
					WeaponClass = class< avaMod_Weapon >( Modifier ).default.WeaponClass;
					avaPMI.AddWeapon( nType, WeaponClass, MaintenanceRate );
				}
				else
				{
					avaPMI.AddWeaponModifier( nType, WeaponClass, class<avaMod_Weapon>( Modifier ) );
				}
			}
		}
	}
}








// old version
//static function FetchCharacterModifiers( array< class<avaModifier> > ModList, avaPlayerReplicationInfo avaPRI, string code )
//{
//	// code 를 분리한다.
//	local string Value;
//	local class< avaModifier >	Modifier;
//	while( GrabCode( ";", code, Value ) )
//	{
//		Modifier = GetModifier( ModList, int(Value) );
//		if ( Modifier != None )
//		{
//			avaPRI.AddCharacterModifierByClass( Modifier );
//		}
//	}
//}
//
//static function FetchWeaponModifiers( array< class<avaModifier> > ModList, avaPlayerReplicationInfo avaPRI, int CharacterType, string code )
//{
//	local string				Weapons;
//	local string				WeaponMod;
//	local class< avaModifier >	Modifier;
//	local class< avaWeapon >	WeaponClass;
//	while( GrabCode( ";", code, Weapons ) )
//	{
//		while( GrabCode( "@", Weapons, WeaponMod ) )
//		{
//			Modifier = GetModifier( ModList, int(WeaponMod) );
//			if ( Modifier != None )
//			{
//				if ( class<avaMod_Weapon>( Modifier ).default.WeaponClass != None )
//				{
//					WeaponClass = class<avaMod_Weapon>( Modifier ).default.WeaponClass;
//					avaPRI.AddWeaponByClass( CharacterType, WeaponClass );
//				}
//				else
//					avaPRI.AddWeaponModifierByClass( CharacterType, WeaponClass , Modifier );
//			}
//		}
//	}
//}
