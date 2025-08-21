//=============================================================================
//  avaGameInfoMessage
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//=============================================================================
class avaGameInfoMessage extends Actor
	native;

struct native GameInfoMessageData
{
	var	int					index;			// Message index
	var int					Type;			// Message �� ���� ��ġ Type
	var int					DisplayType;	// 0�̸� Always, 1�̸� Game�� 1ȸ 
	var	localized string	Msg;			// Message
	var string				SoundCueName;	// Message �� ���� ����� SoundCue Name
	var string				BindingArg1;	// Binding �� Command Name �� �����Ѵ�.
	var string				BindingArg2;	// Binding �� Command Name �� �����Ѵ�.
	var	bool				bActive;
	structdefaultproperties
	{
		bActive	= true;
	}
};

var array<GameInfoMessageData>	GIMData;

simulated function string GetKeyName( PlayerController PC, string Command )
{
	local int	BindingIndex;
	local int	Code;
	BindingIndex = PC.PlayerInput.Bindings.find('Command', Command );
	if ( BindingIndex >= 0 )
	{
		Code =	PC.PlayerInput.GetKeyCodeByName( PC.PlayerInput.Bindings[BindingIndex].Name );
		return Localize( "PlayerInput", "KeyCode["$Code$"]", "avaGame" );
	}
	return "";
}

simulated function ShowGameInfoMessage( int nIndex, avaPlayerController avaPC, optional int nType = 1, optional float fTime )
{
	local SoundCue		sc;
	local int			i;
	local string		Msg;
	local int			ShowType;
	for ( i = 0 ; i < GIMData.length ; ++ i )
	{
		if ( GIMData[i].index != nIndex )	continue;
		if ( GIMData[i].bActive != true )	return;
		if ( GIMData[i].DisplayType == 1 )	
		{
			GIMData[i].bActive = false;
		}
		if ( GIMData[i].BindingArg1 != "" )
		{
			Msg = GIMData[i].Msg;
			Msg	 =  class'avaStringHelper'.static.Replace( GetKeyName( avaPC, GIMData[i].BindingArg1 ) , "%s", Msg );
			if ( GIMData[i].BindingArg2 != "" )
				Msg	= class'avaStringHelper'.static.Replace( GetKeyName( avaPC, GIMData[i].BindingArg2 ) , "%s", Msg );
		}
		else
		{
			Msg = GIMData[i].Msg;
		}
		ShowType = nType;
		if ( ShowType == -1 )	ShowType = GIMData[i].Type;
		avaHUD( avaPC.myHUD ).GameInfoMessage( Msg, fTime, 0, ShowType );
		if ( GIMData[i].SoundCueName != "" )
		{
			sc = SoundCue( DynamicLoadObject( GIMData[i].SoundCueName, class'SoundCue' ) );
			avaPC.ClientPlaySound( sc );
		}
		return;
	}
}

static event LoadDLOs()
{
	local int i;
	for ( i = 0 ; i < default.GIMData.length ; ++ i )
	{
		if ( default.GIMData[i].SoundCueName != "" )
			DynamicLoadObject( default.GIMData[i].SoundCueName, class'Object' );
	}
}


defaultproperties
{
	RemoteRole=ROLE_None
}
