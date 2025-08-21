class avaUIProjectileWeapon extends UIObject
	native;

var() Font		WeaponFont;
var() Font		CharacterFont;
var() float		FadeOutTime;
var() string	InterMedString<ToolTip=eg. 'X' 'x' '=' '-' ' '>;
var() color		DrawColor;

var(Shadow) bool bDropShadow;

var transient avaPawn	PreviousPawn;

struct native ProjWeapDrawInfo
{
	var class<avaWeapon>	WeaponClass;
	var	avaWeapon			WeaponInstance;
	var	int					AmmoCnt;
	var float				DetachTime;
	structdefaultproperties
	{
		DetachTime=-1.0
	}
};

var transient array<ProjWeapDrawInfo> ProjWeapData;

cpptext
{
	void Render_Widget( FCanvas* Canvas );
	void UpdateWeaponData();
}

event UpdateDrawInfo( avaPawn PawnOwner, float CurrentTime )
{
	local int				i, j;
	local avaWeapon			weap;
	local array<avaWeapon>	FoundWeap;
	local bool				bFound;
	local int				index;

	if ( PreviousPawn != PawnOwner )	ProjWeapData.length = 0;
	PreviousPawn = PawnOwner;

	for ( i = 0 ; i < ProjWeapData.length ; ++ i )
	{
		if ( ProjWeapData[i].DetachTime > 0 )
		{
			// 무기를 지우자...
			if ( CurrentTime > ProjWeapData[i].DetachTime + FadeOutTime )
			{
				ProjWeapData.Remove( i--, 1);
				continue;
			}
		}
	}

	for ( i = 0 ; i < 12 ; ++ i )
	{
		weap = PawnOwner.PossessedWeapon[i];
		if ( weap == None || weap.AttachmentClass == None || weap.bHideWeaponMenu )
			continue;
		if ( avaThrowableWeapon( weap ) != None || avaWeap_BaseMissionObject( weap ) != None || avaWeap_Binocular( weap ) != None )
		{
			FoundWeap.length = FoundWeap.length+1;
			FoundWeap[FoundWeap.length-1] = weap;
		}
	}

	// 없어진 무기를 찾자...
	for ( i = 0 ; i < ProjWeapData.length ; ++ i )
	{
		bFound = false;
		for ( j = 0 ; j < FoundWeap.length ; ++ j )
		{
			if ( ProjWeapData[i].WeaponInstance == FoundWeap[j] )
			{
				bFound = true;
				break;
			}
		}

		if ( bFound == false )
		{
			// 이전에 지우려고 예약한 놈은 빼고....
			if ( ProjWeapData[i].DetachTime < 0 )
			{
				ProjWeapData[i].AmmoCnt		= 0;
				ProjWeapData[i].DetachTime	= CurrentTime;
			}
		}
	}

	// 새로운 무기를 넣자...
	for ( i = 0 ; i < FoundWeap.length ; ++ i )
	{
		bFound = false;
		for ( j = 0 ; j < ProjWeapData.length ; ++ j )
		{
			if ( FoundWeap[i] == ProjWeapData[j].WeaponInstance )
			{
				ProjWeapData[j].AmmoCnt = FoundWeap[i].AmmoCount;
				bFound  = true;
				break;
			}
		}

		if ( bFound == false )
		{
			`log( "Add" @FoundWeap[i].Class );
			index = ProjWeapData.length;
			ProjWeapData.length = ProjWeapData.length + 1;
			ProjWeapData[index].WeaponClass = FoundWeap[i].Class;
			ProjWeapData[index].WeaponInstance = FoundWeap[i];
			ProjWeapData[index].AmmoCnt = FoundWeap[i].AmmoCount;
			ProjWeapData[index].DetachTime = -1;
		}
	}
}

event AddDrawInfo( class<avaWEapon> WeaponClass, int AmmoCnt, float DetachTime = -1.0 )
{
	Local ProjWeapDrawInfo DrawInfo;
	DrawInfo.WeaponClass = WeaponClass;
	DrawInfo.DetachTime = DetachTime;
	DrawInfo.AmmoCnt	= AmmoCnt;
	ProjWeapData[ ProjWeapData.Length ] = DrawInfo;
}

defaultproperties
{
	DrawColor=(R=255,G=255,B=255,A=255)
}