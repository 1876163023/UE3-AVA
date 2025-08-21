class avaUIHelmet extends UIObject native;

var() TextureCoordinates HelmetCoordinates;
var() TextureCoordinates NoHelmetCoordinates;
var() Surface			 Image;
var() color				 NormalColor;	
var() color				 NoHelmetColor;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );		
}

defaultproperties
{
	NormalColor		=(R=255,G=255,B=255,A=255)
	NoHelmetColor	=(R=255,G=255,B=255,A=255)
	Image=Texture2D'EngineResources.WhiteSquareTexture'
}