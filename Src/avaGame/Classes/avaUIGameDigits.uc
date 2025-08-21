class avaUIGameDigits extends avaUISimpleText native;

enum avaUIGameDigitsType
{
	AVAUIDIGIT_Health,
	AVAUIDIGIT_AmmoCount,
	AVAUIDIGIT_ReloadCount,
	AVAUIDIGIT_TeamScore,
	AVAUIDIGIT_TeamPlayerCnt,
	AVAUIDIGIT_ArmorPercentage,
	AVAUIDIGIT_ArmorHealth,
	AVAUIDIGIT_WinCondition,
	AVAUIDIGIT_ReInforcementTime,
	AVAUIDIGIT_Fixed,
	AVAUIDIGIT_RemainingTeamScoreToGo,
	AVAUIDIGIT_DogTagCnt,
	AVAUIDIGIT_DogTagPackCnt,
	AVAUIDIGIT_AVAVersion,
	AVAUIDIGIT_AVABuiltFromChangelistNum,
	AVAUIDIGIT_TeamSymbolName,
	AVAUIDIGIT_WeaponMaintenanceRate,
};

var() avaUIGameDigitsType Binding;
var transient int LastValue;

var() int						nTeam;			// which team wants?

var() int						DigitFixed;

struct native GameDigitColorInfo
{
	var() int	Value;
	var() Color Color;
};
var() array<GameDigitColorInfo>		GameDigitColor;
var() bool							bLinearColor;

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );

	virtual UBOOL UpdateString();		
}

defaultproperties
{
	Font=TrueTypeFont'GameFonts.HUD.HUDLarge'
	DrawColor=(R=255,G=255,B=255,A=255)

	bDropShadow=true

	nTeam=-1
}