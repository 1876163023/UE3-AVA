class avaUIGamePanel extends UIPanel
	native;

enum EGAMEPANEL_BindingType
{
	GAMEPANEL_TeamColor,
};

var() array<Color>					PanelColor<ToolTip=/TeamColor-EU:0,NRF:1/>;
var() EGAMEPANEL_BindingType		BindingType;

cpptext
{
	void Render_Widget( FCanvas* Canvas );
}

defaultproperties
{
	PanelColor(0)=(R=255,G=255,B=255,A=255)
	PanelColor(1)=(R=255,G=255,B=255,A=255)

	BindingType=GAMEPANEL_TeamColor
}