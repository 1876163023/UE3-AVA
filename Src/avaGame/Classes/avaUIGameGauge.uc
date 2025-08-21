class avaUIGameGauge extends avaUISimpleProgress
	native;

var(GeneralOption)	Color			ActiveColor<ToolTip=color when moving>;
var(GeneralOption)	float			ActiveTime[2]<ToolTip=/OriginalColor:0,ActiveColor:1/>;

var(PlayerHealthOption)	float		Player_DangerRate;
var(PlayerHealthOption)	Color		Player_DangerColor;
var(PlayerHealthOption)	Color		Player_NormalColor;

var(ArmorHealthOption)		Color		Armor_NormalColor;

enum EUIGameGaugeType
{
	UI_GAME_GAUGE_PlayerHealth,
	UI_GAME_GAUGE_ArmorHealth,
};

var() EUIGameGaugeType		UIGameGaugeType;

cpptext
{
	virtual void UpdateProgress();
}

defaultproperties
{
	Player_DangerColor=(R=255,G=0,B=0,A=255)
	Player_NormalColor=(R=255,G=255,B=255,A=255)
	Player_DangerRate=0.3
	Armor_NormalColor=(R=128,G=128,B=128,A=255)

	ActiveColor=(R=255,G=255,B=255,A=255)
	ActiveTime(0)=0.1
	ActiveTime(1)=0.1
}