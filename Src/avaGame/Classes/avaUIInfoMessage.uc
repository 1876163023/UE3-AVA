class avaUIInfoMessage extends avaUISimpleText native
	hidecategories(Icon);

struct native IconInfo
{
	var() int Code;
	var() Surface Image;
	var() color IconColor;
	var() TextureCoordinates Coordinates;
};

var() array<IconInfo> Icons;
var() int Editor_IconCode;
var() string Editor_String;

cpptext
{
	virtual UBOOL UpdateString();

	virtual INT UpdateInfo() { return 0; /* no changes at all */ }

	void SetIcon( INT Code );
}

defaultproperties
{
	Editor_String = "test info message"
}