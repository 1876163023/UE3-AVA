class avaUIImage extends avaUIImageBase
	native;

/** The actual image */
var(Widget) const Surface Image;

/** Texture Coordinates */
var(Widget) TextureCoordinates Coordinates;

/** Used to colorize the texture */
var(Widget)	const LinearColor ImageColor;

/** If true, this widget will multiple the ImageColor by the TeamColor as dictated from the PlayerOwner */
var(Widget)	bool bTeamColored;

/** This image is centered about the X/Y */
var(Widget) bool bCentered;

var(Widget) bool bStretched;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );
	virtual void DrawTile(FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLinearColor DrawColor);
	virtual void DrawStretchedTile(FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLinearColor DrawColor);
}

native function SetImage( Surface NewImage, optional TextureCoordinates NewCoordinates );
native function SetColor( LinearColor NewColor);


defaultproperties
{
	ImageColor=(R=1.0,G=1.0,B=1.0,A=1.0)
	
	Position={(	Value[UIFACE_Right]=50,Value[UIFACE_Bottom]=50,
				ScaleType[UIFACE_Right]=EVALPOS_PixelOwner,
				ScaleType[UIFACE_Bottom]=EVALPOS_PixelOwner)}

}