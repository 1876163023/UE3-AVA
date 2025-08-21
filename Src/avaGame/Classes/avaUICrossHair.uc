class avaUICrossHair extends UIObject
	native;


enum ECrossHairDisplayType
{
	CROSSHAIRDISP_None,
	CROSSHAIRDISP_Default,
	CROSSHAIRDISP_Texture_Cross,
	CROSSHAIRDISP_Texture_Plus,
	CROSSHAIRDISP_Texture_Once,
};

struct native CrossHairDataType
{
	var(Draw) class<avaWeapon>			WeaponClass;
	var(Draw) Surface					Texture;
	var(Draw) Vector2D					DrawExtent;
	var(Draw) bool						bIgnoreExtent;
	var(Draw) TextureCoordinates		TexCoord;
	var(Draw) bool						bIgnoreCoord;
	var(Draw) ECrossHairDisplayType		DisplayType<ToolTip=/None/Default-BarSize/Cross-'X'/Plus-'+'/Once-DrawCenteredOnce/>;
	//var(Draw) MaterialInstance			SightModeMat;

	structdefaultproperties
	{
		DrawExtent=(X=10,Y=10)
		TexCoord=(U=0,V=0,UL=0,VL=0)
		bIgnoreExtent=true
		bIgnoreCoord=true

		DisplayType = CROSSHAIRDISP_Default;
	}
};

var() int						DefaultBarSize;
var() array<CrossHairDataType>	CrossHairData;
var() color						TargetColor;
var class<avaWeapon>			BaseWeaponClass;

var() MaterialInstance			LaserSightMat;


// CrossHair를 드로잉마다 재검색하는것을 막기위한 맴버들
var private class<avaWeapon>			LatestWeaponClass;		/**< 마지막으로 사용한 Weapon의 Class */
var private int							CurrentCrossHairIndex;	/**< 현재 사용중인 CrossHair의 Index*/
var private Texture2D					DefaultWhiteTexture;

var private avaUITargetName				UITargetName;

struct native InvincibleIconInfo
{
	var()	Surface					Texture;
	var()	TextureCoordinates		TexCoord;
	var()	Vector2D				DrawExtent;
	var()	Color					DrawColor;
};

var()	InvincibleIconInfo			InvincibleIcon;
	
cpptext
{
public:
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );
	virtual void Render_Widget( FCanvas *Canvas );
}

defaultproperties
{
	BaseWeaponClass=class'avaWeapon'
	DefaultBarSize=8
	DefaultWhiteTexture=Texture2D'EngineResources.WhiteSquareTexture'
	TargetColor=(R=255,G=0,B=0,A=255)
}