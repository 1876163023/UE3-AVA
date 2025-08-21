#include "avaGame.h"

IMPLEMENT_CLASS(UavaUIImageBase);
IMPLEMENT_CLASS(UavaUIImage);
IMPLEMENT_CLASS(UavaUILabel);


/*=========================================================================================
UTUIImage - Our image class is designed to by-pass the style system so as to support colorization
and alpha effects (like fading).  The Image can also be flagged as bTeamColored and in that case
it will try to resolve the team to get the color.  It also removes the needed for a secondary class to hold
the texture
========================================================================================= */

/**
* Allows for seeting the default image without overhead having to create a new style
*
* @Param	RI		The Render Interface
*/
void UavaUIImage::Render_Widget( FCanvas* Canvas )
{
	Super::Render_Widget(Canvas);

	if ( !Image )
	{
		return;
	}

	// Grab the bounds

	FLOAT X  = RenderBounds[UIFACE_Left];
	FLOAT Y  = RenderBounds[UIFACE_Top];
	FLOAT XL = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
	FLOAT YL = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

	FLinearColor DrawColor = ImageColor;

	// Look to see if this is team colored, if so, get the modifier

	//if ( bTeamColored )
	//{
	//	AavaPlayerController* PlayerOwner = GetAvaPlayerOwner();
	//	if ( PlayerOwner )
	//	{
	//		AavaPlayerReplicationInfo* PRI = Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo);
	//		if ( PRI )
	//		{
	//			FLinearColor TeamColor = FLinearColor( PRI->eventGetHUDColor() );

	//			// Multiply in the modifier

	//			DrawColor *= TeamColor;
	//		}
	//	}
	//}	

	if (bCentered)
	{
		X -= XL/2;
		Y -= YL/2;
	}

	if ( !bStretched )
	{
		DrawTile(Canvas, X,Y,XL,YL,Coordinates.U, Coordinates.V, Coordinates.UL, Coordinates.VL, DrawColor);
	}
	else
	{
		DrawStretchedTile(Canvas, X, Y, XL, YL, DrawColor);
	}

}
void UavaUIImage::DrawTile(FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, FLinearColor DrawColor)
{
	// Depending on the type, render the texture

	UTexture* Texture = Cast<UTexture>(Image);

	FLOAT W = Image->GetSurfaceWidth();
	FLOAT H = Image->GetSurfaceHeight();

	if ( Texture != NULL )
	{
		FTexture* RawTexture = Texture->Resource;
		::DrawTile(Canvas, X, Y, XL, YL, U/W, V/H, UL/W, VL/H, DrawColor, RawTexture);
	}
	else
	{
		UMaterialInstance* Material = Cast<UMaterialInstance>(Image);
		if ( Material != NULL )

		{
			::DrawTile(Canvas, X, Y, XL, YL, U/W, V/H, UL/W, VL/H, Material->GetInstanceInterface(0));
		}
	}
}


void UavaUIImage::DrawStretchedTile( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLinearColor DrawColor)
{
	FLOAT SurfaceWidth = Image->GetSurfaceWidth();
	FLOAT SurfaceHeight = Image->GetSurfaceHeight();

	FLOAT SizeX	= XL;
	FLOAT SizeY	= YL;
	FLOAT mU	= Coordinates.U;
	FLOAT mV	= Coordinates.V;
	FLOAT SizeU	= Coordinates.UL;
	FLOAT SizeV	= Coordinates.VL;

	// Get the size of the image
	FLOAT mW = SizeU;
	FLOAT mH = SizeV;

	// Get the midpoints of the image
	FLOAT MidX = mW * 0.5f;
	FLOAT MidY = mH * 0.5f;

	// Grab info about the scaled image
	FLOAT AbsWidth = Abs<FLOAT>(SizeX);
	FLOAT AbsHeight = Abs<FLOAT>(SizeY);

	// this represents the size of the region that will be filled 
	FLOAT FillerTileW = appCopySign(AbsWidth - mW, SizeX);
	FLOAT FillerTileH = appCopySign(AbsHeight - mH, SizeY);

	FLOAT AbsTileW, AbsTileY;	// absolute values of TileW and TileY; these are calculated using known non-negative values and then copysign is used to
	FLOAT TileW, TileY;			// set the signed values here. AbsTileW is always used when refering into the image and TileW to refer to the screen.


	UBOOL bStretchHorizontally	= mW < AbsWidth;
	UBOOL bStretchVertically	= mH < AbsHeight;
	FLOAT MaterialTileW = MidX, MaterialTileH = MidY;

	// Draw the spans first - these are the sections of the image that are stretched to fill the gap between the corners of the region

	// Top and Bottom
	if ( bStretchHorizontally )
	{
		// Need to stretch material horizontally
		AbsTileW = MidX;
		if ( !bStretchVertically )
		{
			// if we're not stretching the vertical orientation, we should scale it
			AbsTileY = AbsHeight * 0.5f;
		}
		else
		{
			AbsTileY = MidY;
		}

		TileW = appCopySign(AbsTileW, SizeX);
		TileY = appCopySign(AbsTileY, SizeY);

		// draw the upper middle stretched portion of the image
		DrawTile(Canvas,X + TileW, Y, FillerTileW, TileY, mU+MidX, mV, 1, MaterialTileH, DrawColor);

		// draw the lower middle stretched portion of the image
		DrawTile(Canvas,X + TileW, Y+SizeY-TileY, FillerTileW, TileY, mU+MidX, mV+mH-MaterialTileH, 1, MaterialTileH, DrawColor);
	}
	else
	{
		AbsTileW = AbsWidth * 0.5f;
	}

	// Left and Right
	if ( bStretchVertically )
	{
		// Need to stretch material vertically
		AbsTileY = MidY;
		if ( !bStretchHorizontally )
		{
			// if we're not stretching the horizontal orientation, scale it
			AbsTileW = AbsWidth * 0.5f;
		}
		else
		{
			AbsTileW = MidX;
		}

		TileW = appCopySign(AbsTileW, SizeX);
		TileY = appCopySign(AbsTileY, SizeY);

		// draw the left middle stretched portion of the image
		DrawTile(Canvas,X, Y+TileY, TileW, FillerTileH, mU, mV+AbsTileY, MaterialTileW,1, DrawColor);

		// draw the right middle stretched portion of the image
		DrawTile(Canvas,X+SizeX-TileW,Y+TileY,TileW,FillerTileH,mU+mW-MaterialTileW,mV+AbsTileY, MaterialTileW, 1, DrawColor);
	}
	else
	{
		AbsTileY = AbsHeight * 0.5f;
	}

	// Center
	TileW = appCopySign(AbsTileW, SizeX);
	TileY = appCopySign(AbsTileY, SizeY);

	// If we had to stretch the material both ways, repeat the middle pixels of the image to fill the gap
	if ( bStretchHorizontally && bStretchVertically )
	{
		DrawTile(Canvas,X+TileW, Y+TileY, FillerTileW, FillerTileH, mU+MaterialTileW, mV+MaterialTileH, 1, 1, DrawColor);
	}

	// Draw the 4 corners - each quadrant is scaled if its area is smaller than the destination area
	DrawTile(Canvas,X,				Y,				TileW, TileY,	mU,						mV,						MaterialTileW, MaterialTileH, DrawColor);		//topleft
	DrawTile(Canvas,X+SizeX-TileW,	Y,				TileW, TileY,	mU+mW-MaterialTileW,	mV,						MaterialTileW, MaterialTileH, DrawColor);		//topright
	DrawTile(Canvas,X,				Y+SizeY-TileY,	TileW, TileY,	mU,						mV+mH-MaterialTileH,	MaterialTileW, MaterialTileH, DrawColor);		//bottomleft
	DrawTile(Canvas,X+SizeX-TileW,	Y+SizeY-TileY,	TileW, TileY,	mU+mW-MaterialTileW,	mV+mH-MaterialTileH,	MaterialTileW, MaterialTileH, DrawColor);		//bottomright

}
/*
{


// Get the size of the image

FLOAT mW = Coordinates.UL;
FLOAT mH = Coordinates.VL;

// Get the midpoints of the image

FLOAT MidX = appFloor(mW/2);
FLOAT MidY = appFloor(mH/2);

// Grab info about the scaled image

FLOAT SmallTileW = XL - mW;
FLOAT SmallTileH = YL - mH;

FLOAT fX = Coordinates.U;
FLOAT fY = Coordinates.V;

// Draw the spans first

// Top and Bottom

if (mW<XL)
{
fX = MidX;

if (mH>YL)
fY = YL/2;
else
fY = MidY;

DrawTile(Canvas, X+fX, Y,			SmallTileW, fY, MidX,0,			1,	fY, DrawColor);
DrawTile(Canvas, X+fX, Y+YL-fY,		SmallTileW,	fY,	MidX, mH-fY,	1,	fY,	DrawColor);
}
else
fX = XL / 2;

// Left and Right

if (mH<YL)
{

fY = MidY;

DrawTile(Canvas, X,			Y+fY,	fX, SmallTileH, 0,		fY, fX, 1, DrawColor);
DrawTile(Canvas, X+XL-fX,	Y+fY,	fX,	SmallTileH,	mW-fX,	fY, fX,	1, DrawColor);

}
else
fY = YL / 2; 

// Center

if ( (mH<YL) && (mW<XL) )
DrawTile(Canvas, X+fX, Y+fY, SmallTileW, SmallTileH, fX, fY, 1, 1, DrawColor);

// Draw the 4 corners.

DrawTile(Canvas,X,			Y,			fX, fY, 0,		0,		fX, fY, DrawColor);
DrawTile(Canvas,X+XL-fX,	Y,			fX, fY,	mW-fX,	0,		fX, fY, DrawColor);
DrawTile(Canvas,X,			Y+YL-fY,	fX, fY, 0,		mH-fY,	fX, fY, DrawColor);
DrawTile(Canvas,X+XL-fX,	Y+YL-fY,	fX, fY,	mW-fX,	mH-fY,	fX, fY, DrawColor);
}
*/
/**
* Accessor function for setting the image.
*
* @Param	NewImage		the surface to use
* @Param	NewCoordinates	The texture coordinates to use
*/

void UavaUIImage::SetImage(class USurface* NewImage,struct FTextureCoordinates NewCoordinates)
{
	Image = NewImage;
	if ( NewCoordinates.U + NewCoordinates.V + NewCoordinates.UL + NewCoordinates.VL != 0 )
	{
		Coordinates = NewCoordinates;
	}
}

/**
* Access for setting the color.  @TODO - Add support for setting a color parameter in a material
*
* @Param	NewColor		the color to use
*/
void UavaUIImage::SetColor(FLinearColor NewColor)
{
	ImageColor = NewColor;
}


/*=========================================================================================
UTUILabel - Our label class is designed to by-pass the style system so as to support colorization
and alpha effects (like fading).  
========================================================================================= */

/** 
* Recaculate the bounds for the Text string 
*
* @Param	Face		the edge that changed
*/
void UavaUILabel::ResolveFacePosition(EUIWidgetFace Face)
{
	Super::ResolveFacePosition(Face);

	// Resize the caption

	ResizeCaption();
}

void UavaUILabel::ResizeCaption()
{
	if ( Font )
	{
		const TCHAR* String = *Caption;

		// Get the width and font

		FLOAT SceneHeight = GetScene()->Position.GetBoundsExtent( GetScene(), UIORIENT_Vertical, EVALPOS_PixelViewport);

		FontScaling = Font->GetScalingFactor( SceneHeight );
		FontPageIndex = Font->GetResolutionPageIndex( SceneHeight );

		TextBounds[UIFACE_Right]=0.0;
		TextBounds[UIFACE_Bottom]=0.0;

		FLOAT Kerning = Font->Kerning * FontScaling;

		// Calculate the width/height of the string

		while ( *String )
		{
			FLOAT Width, Height;
			Font->GetCharSize(*String++, Width, Height, FontPageIndex);

			// Apply the Scaling

			Width *= FontScaling;
			Height *= FontScaling;

			TextBounds[UIFACE_Right] += (*String) ? Width + Kerning : Width;
			TextBounds[UIFACE_Bottom] = Max<FLOAT>(TextBounds[UIFACE_Bottom], Height);
		}

		TextBounds[UIFACE_Right] = appTrunc(TextBounds[UIFACE_Right]);
		TextBounds[UIFACE_Bottom] = appTrunc(TextBounds[UIFACE_Bottom]);

		FLOAT WidgetWidth = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
		FLOAT WidgetHeight = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

		// Calculcate the the Left Position

		if (Justification == UIALIGN_Left)
		{
			TextBounds[UIFACE_Left] = RenderBounds[UIFACE_Left];
		}
		else if (Justification == UIALIGN_Right)
		{
			TextBounds[UIFACE_Left] = RenderBounds[UIFACE_Right] - TextBounds[UIFACE_Right];
		}
		else
		{
			TextBounds[UIFACE_Left] = RenderBounds[UIFACE_Left] + (WidgetWidth / 2) - (TextBounds[UIFACE_Right] / 2);
		}

		TextBounds[UIFACE_Top] = RenderBounds[UIFACE_Top];

		// Prcache everything

		DrawX = appTrunc(TextBounds[UIFACE_Left]);
		DrawY = appTrunc(TextBounds[UIFACE_Top]);

		// Get the bounds

		LeftBoundary = appTrunc( RenderBounds[UIFACE_Left] );
		RightBoundary = appTrunc( RenderBounds[UIFACE_Right] );
	}
}

/**
* Accessor for setting the caption.  When it changes, we update precache the bounds
*
* @Param	NewCaption	The new caption
*/
void UavaUILabel::SetCaption(const FString& NewCaption)
{
	if ( Caption != NewCaption )
	{
		Caption = NewCaption;
		if ( IsInitialized() )
		{
			ResizeCaption();
		}
	}
}

/**
* Accessor for setting the font.  When it changes, we update precache the bounds
*
* @Param	NewFont		The new font
*/


void UavaUILabel::SetFont(UFont* NewFont)
{
	if (Font != NewFont)
	{
		Font = NewFont;
		ResizeCaption();
	}
}
/**
* Accessor for changing the color.
*/
void UavaUILabel::SetColor(FLinearColor NewColor)
{
	FontColor = NewColor;
}

/**
* Actually draw the string.  Don't bother with the FRenderInterface DrawString since it doesn't clip anymore
*
* @Param	RI		The Render Interface
*/
void UavaUILabel::Render_Widget(FCanvas* Canvas)
{
	Super::Render_Widget(Canvas);

	FLOAT SRT=0.0f;

	clock(SRT);

	// Draw the Shadow First

	FLinearColor TmpColor = FontColor;
	FontColor = ShadowColor;

	switch ( ShadowedType )
	{
	case ESD_UpperLeft:
		DrawText(Canvas,-1,-1);
		break;

	case ESD_UpperMid:
		DrawText(Canvas,0,-1);
		break;

	case ESD_UpperRight:
		DrawText(Canvas,1,-1);
		break;

	case ESD_Left:
		DrawText(Canvas,-1,0);
		break;

	case ESD_Glow:

		for( INT X = -1; X < 2;  X++)
		{
			for(INT Y = -1; Y < 2; Y++)
			{
				DrawText(Canvas,X,Y);
			}
		}
		break;

	case ESD_Right:
		DrawText(Canvas,1,0);
		break;


	case ESD_LowerLeft:
		DrawText(Canvas,-1,1);
		break;

	case ESD_LowerMid:
		DrawText(Canvas,0,1);
		break;

	case ESD_LowerRight:
		DrawText(Canvas,1,1);
		break;
	}

	// Draw the Text

	FontColor = TmpColor;
	DrawText(Canvas);

	unclock(SRT);
	//UUTGameUISceneClient* SC = Cast<UUTGameUISceneClient>( GetSceneClient() );
	//if (SC)
	//{
	//	SC->StringRenderTime+=SRT;
	//}
}

void UavaUILabel::DrawText(FCanvas* Canvas, INT XMod, INT YMod)
{
	if (Caption.Len() > 0 && Font)
	{
		const TCHAR* String = *Caption;

		FLinearColor DrawColor = FontColor;

		FLOAT X=0.0f;

		while ( *String )
		{
			INT Ch = (TCHARU)Font->RemapChar(*String++);

			// Process character if it's valid.
			if( Ch < Font->Characters.Num() )
			{
				FFontCharacter& Char = Font->Characters(Ch + FontPageIndex);
				UTexture2D* Tex;

				if( Char.TextureIndex < Font->Textures.Num() && (Tex=Font->Textures(Char.TextureIndex))!=NULL )
				{
					INT CU     = Char.StartU;
					INT CV     = Char.StartV;
					INT CUSize = Char.USize;
					INT CVSize = Char.VSize;

					// Attempt to reject fully clipped characters

					if ( DrawX <= RightBoundary && DrawX + ( CUSize * FontScaling)  >= LeftBoundary )
					{
						// Look to see if the Left side is outside the left bounds
						if (DrawX < LeftBoundary )
						{
							INT Dist = LeftBoundary - DrawX;
							CU += appTrunc( Dist / FontScaling );
							CUSize -= appTrunc( Dist / FontScaling );
							DrawX = LeftBoundary;
						}

						// Look at the right side and clip
						if ( DrawX + (CUSize * FontScaling) > RightBoundary )
						{
							CUSize -= appTrunc(((DrawX+ (CUSize * FontScaling) ) - RightBoundary ) / FontScaling);
						}
						// Covert to 1.0 - 0.0

						FLOAT fCU = (FLOAT)CU / (FLOAT)Tex->SizeX;
						FLOAT fCV = (FLOAT)CV / (FLOAT)Tex->SizeY;

						FLOAT fCUSize = (FLOAT)CUSize / (FLOAT)Tex->SizeX;
						FLOAT fCVSize = (FLOAT)CVSize / (FLOAT)Tex->SizeY;

						// We are within the bounds, draw the character
						DrawTile(Canvas, DrawX+X+XMod, DrawY+YMod, (CUSize*FontScaling), (CVSize*FontScaling), fCU, fCV, fCUSize, fCVSize, DrawColor, Tex->Resource);
						X += ( CUSize * FontScaling );
					}
				}
			}				
		}
	}
}

void UavaUILabel::PostEditChange( UProperty* PropertyThatChanged )
{
	if ( PropertyThatChanged->GetFName() == FName(TEXT("Caption" )) || PropertyThatChanged->GetFName() == FName(TEXT("Justification" )) )
	{
		FString C = Caption;
		Caption = TEXT("");
		SetCaption(C);
	}
	else if ( PropertyThatChanged->GetFName() == FName(TEXT("Font")) )
	{
		ResizeCaption();
	}

	Super::PostEditChange(PropertyThatChanged);
}