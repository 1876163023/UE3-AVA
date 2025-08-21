class TrueTypeFont extends Font native noexport;

enum ETrueTypeQuality
{
	TTQ_Default,
	TTQ_Antialiased,
	TTQ_ClearType
};

var native private const pointer Face;
var() string FontResource;
var() string FontFamily;
var() int Height;
var private int iClass;
var() int DropShadow;
var() ETrueTypeQuality Quality;
var private transient bool bInitialized;
var() bool bPrecacheAlphabets;
var() bool bBold;
var bool bLocked;
var() float WidthScaler;

//NOTE: Do not expose this to the editor as it has nasty crash potential
var transient const	array<Texture2D> TTFTextures;

defaultproperties
{
	bInitialized = false	
	Height = 16
	iClass = 0
	Quality=TTQ_Antialiased
	bPrecacheAlphabets=true
	WidthScaler = 1.0
//	LetterSpacing = 0
}