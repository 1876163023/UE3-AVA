class avaUITouchWeapon extends avaUISimpleText
	native;

var		transient		class<avaWeapon>			LatestTouchWeapon;
var(KeyIcon)			Surface						KeyIcon;
var(KeyIcon)			TextureCoordinates			KeyIconCoord;
var(KeyIcon)			bool						bIgnoreExtent;
var(KeyIcon)			bool						bIgnoreCoord;
var(KeyIcon)			Vector2D					KeyIconExtent;

var		transient		Vector2D					IconOffset;
var		transient		Vector2D					IconExtent;
var		transient		TextureCoordinates			IconCoord;
var		transient		Vector2D					StringExtent;

var(WeaponIcon)			bool						bShowWeaponIcon;
var(WeaponIcon)			Vector2D					WeaponIconOffset;
var(WeaponIcon)			Font						WeaponIconFont;
var(WeaponIcon)			Color						WeaponIconColor;

cpptext
{
	virtual UBOOL UpdateString();
	virtual void Render_Widget(FCanvas* Canvas);
}

defaultproperties
{
	Font = TrueTypeFont'GameFonts.HUD.HUDMedium'
	KeyIconExtent=(X=10,Y=10)
	bIgnoreExtent=true
	bIgnoreCoord=true
	KeyIcon=Texture2D'EngineResources.WhiteSquareTexture'
	KeyIconCoord=(U=0,V=0,UL=0,VL=0)
	DrawColor=(R=255,G=255,B=255,A=255)

	bShowWeaponIcon=true
	WeaponIconColor=(A=255,R=255,G=255,B=255)
}