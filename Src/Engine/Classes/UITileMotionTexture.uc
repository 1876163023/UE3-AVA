/**
 * Acts as the raw interface for providing a texture or material to the UI.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved
 */
class UITileMotionTexture extends UITexture
	native(UIPrivate);

cpptext
{
	/**
	 * Fills in the extent with the size of this texture's material
	 *
	 * @param	Extent	[out] set to the width/height of this texture's material
	 */
	virtual void CalculateExtent( FVector2D& Extent ) const;

	/**
	 * Fills in the extent with the size of this texture's material
	 *
	 * @param	out_SizeX	[out] filled in with the width this texture's material
	 * @param	out_SizeY	[out] filled in with the height of this texture's material
	 */
	virtual void CalculateExtent( FLOAT& out_SizeX, FLOAT& out_SizeY ) const;

	/**
	 * Render this UITexture using the parameters specified.
	 *
	 * @param	Canvas		the FCanvas to use for rendering this texture
	 * @param	Parameters	the bounds for the region that this texture can render to.
	 */
	virtual void Render_Texture( FCanvas* Canvas, const FRenderParameters& Parameters );
}

var()	int						TileDimension[EUIOrientation.UIORIENT_MAX];
var()	int						NumActivated<ToolTip= NumActivated <= (TileDimension[0] * TileDimension[1])>;
var()	float					MotionPeriod;
var()	int						MotionRepeat<ToopTip=0 means the infinite loop>;
var()	transient float			UpdateTime<ToolTip=Time It Is Created>;
var()	int						OffsetIndex<ToolTip=Initial Index>;

defaultproperties
{
	TileDimension[UIORIENT_Horizontal]=1
	TileDimension[UIORIENT_Vertical]=1
	NumActivated=1
	MotionPeriod=1.0
	MotionRepeat=0
	OffsetIndex=0
}