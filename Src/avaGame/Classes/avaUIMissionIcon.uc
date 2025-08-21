class avaUIMissionIcon extends avaUISimpleText native;

var(MissionIcon) color	BombNormalColor, BombAlarmColor, BombBlinkColor;
var(MissionIcon) string	BombMissionCode;

var(MissionIcon) color	TransportNormalColor;
var(MissionIcon) string	TransportMissionCode;

cpptext
{
	virtual UBOOL UpdateString();
}

defaultproperties
{
	Font = TrueTypeFont'GameFonts.UI.UIMedium'
	BombNormalColor	=	(R=123, G=237, B=10, A=179)		  
	BombAlarmColor	=	(R=255, G=148, B=22, A=255)		
	BombBlinkColor	=	(R=255, G=255, B=255, A=0)		
}