#include "EnginePrivate.h"
#include "UnTruetypeFont.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftbitmap.h>
#include <freetype/ftglyph.h>

#define FT_PIX_FLOOR( x )     ( (x) & ~63 )

void FT_GlyphSlot_Embolden( FT_GlyphSlot  slot )
{
	FT_Library  library = slot->library;
	FT_Face     face    = slot->face;
	FT_Error    error;
	FT_Pos      xstr, ystr;


	if ( slot->format != FT_GLYPH_FORMAT_OUTLINE &&
		slot->format != FT_GLYPH_FORMAT_BITMAP )
		return;

	/* some reasonable strength */
	xstr = FT_MulFix( face->units_per_EM, face->size->metrics.y_scale ) / 24;
	ystr = xstr;

	check( slot->format == FT_GLYPH_FORMAT_BITMAP );
	
	xstr = FT_PIX_FLOOR( xstr );
	if ( xstr == 0 )
		xstr = 1 << 6;
	ystr = FT_PIX_FLOOR( ystr );
	
	error = FT_Bitmap_Embolden( library, &slot->bitmap, xstr, ystr );
	if ( error )
		return;	

	if ( slot->advance.x )
		slot->advance.x += xstr;

	if ( slot->advance.y )
		slot->advance.y += ystr;

	slot->metrics.width        += xstr;
	slot->metrics.height       += ystr;
	//slot->metrics.horiBearingX -= xstr / 2;
	slot->metrics.horiBearingY += ystr;
	slot->metrics.horiAdvance  += xstr;
	slot->metrics.vertBearingX -= xstr / 2;
	slot->metrics.vertBearingY += ystr;
	//slot->metrics.vertAdvance  += ystr;

	if ( slot->format == FT_GLYPH_FORMAT_BITMAP )
		slot->bitmap_top += ystr >> 6;
}

static FT_Library FTLibrary;

static UBOOL s_bFontFound = FALSE;
INT CALLBACK FontEnumProc( const LOGFONT *lpelfe, const TEXTMETRIC *lpntme, DWORD FontType, LPARAM lParam )
{
	s_bFontFound = TRUE;
	return 0;
}

struct FFontParameters
{
	INT Tall;
	INT DropShadowOffset;
	INT OutlineSize;
	INT Blur;
	FLOAT WidthScaler;

	UBOOL bAntialiased;
	UBOOL bBold;

	FFontParameters( INT InTall, UBOOL bInAntialiased, UBOOL bInBold, INT InDropShadowOffset, INT InOutlineSize, INT InBlur, FLOAT InWidthScaler )
		: Tall( InTall ), bAntialiased(bInAntialiased), bBold( bInBold ), DropShadowOffset( InDropShadowOffset ), OutlineSize( InOutlineSize ), Blur( InBlur ), WidthScaler( InWidthScaler )
	{
	}
};

class FFontRasterizer
{
public :
	INT Height;
	INT Ascent, Descent;

	FFontParameters Parameters;
	UBOOL Caps_DropShadow;

	FFontRasterizer( const FFontParameters& InParameters )
		: Parameters( InParameters ), Height( InParameters.Tall ), Caps_DropShadow(FALSE), Ascent( InParameters.Tall ), Descent( 0 )
	{
	}

	virtual ~FFontRasterizer()
	{
	}

	void Rasterize( WCHAR CharCode, INT CharXL, INT CharYL, DWORD* RGBA, INT Pitch )
	{
		FillCharPixels( CharCode, CharXL, CharYL, RGBA, Pitch );

		if (Parameters.DropShadowOffset && !Caps_DropShadow)
		{
			DropShadow( CharXL, CharYL, RGBA, Pitch );
		}		
	}

	void DropShadow( INT width, INT height, DWORD* RGBA, INT Pitch )
	{		
		for (INT y = height - 1; y >= Parameters.DropShadowOffset; y--)
		{
			for (INT x = width - 1; x >= Parameters.DropShadowOffset; x--)
			{
				DWORD& Dest = RGBA[ x + y * Pitch ];

				DWORD Src = RGBA[ (x - Parameters.DropShadowOffset) + (y - Parameters.DropShadowOffset) * Pitch ];			

				const BYTE SrcAlpha = (Src >> 24) & 0xff;
				const BYTE DestAlpha = (Dest >> 24) & 0xff;
				
				// safe!
				if (DestAlpha == 0)
				{
					Dest = SrcAlpha << 24;
				}
				// 애매한 경우!
				else if (DestAlpha < 0xff)
				{
					const BYTE FinalAlpha = Clamp( SrcAlpha + DestAlpha, 0, 0xff );

					const BYTE R = Clamp( (INT)((Dest >> 16) & 0xff) * FinalAlpha / DestAlpha, 0, 255 );
					const BYTE G = Clamp( (INT)((Dest >> 8) & 0xff) * FinalAlpha / DestAlpha, 0, 255 );
					const BYTE B = Clamp( (INT)((Dest >> 0) & 0xff) * FinalAlpha / DestAlpha, 0, 255 );					

					Dest = FinalAlpha | (R << 16) | (G << 8) | (B);
				}					
			}
		}
	}

	void GetCharABCWidths( WCHAR CharCode, INT& a, INT& b, INT& c )
	{
		GetPureCharABCWidths( CharCode, a, b, c );

		a -= Parameters.Blur + Parameters.OutlineSize;
		b += + ((Parameters.Blur + Parameters.OutlineSize) * 2) + Parameters.DropShadowOffset;
		c -= Parameters.Blur + Parameters.DropShadowOffset + Parameters.OutlineSize;					
	}	

	// !
	virtual void FillCharPixels( WCHAR CharCode, INT CharXL, INT CharYL, DWORD* RGBA, INT Pitch ) = 0;
	virtual void GetPureCharABCWidths( WCHAR CharCode, INT& a, INT& b, INT& c ) = 0;
	virtual void UpdateKerningInfo( WCHAR CharCode, FFontCharacter* cc )
	{
	}
};

static LONG FT_Ref = 0;
struct FFreetypeFontProvider : public FFontRasterizer
{
public :
	struct FT_FaceRec_*  Face;

	FFreetypeFontProvider( const TCHAR* Path, const FFontParameters& InParameters )
		: FFontRasterizer( InParameters )
	{
		// Setting caps :)
		Caps_DropShadow = TRUE;

		if (!FT_Ref++)
		{
			FT_Init_FreeType( &FTLibrary );	
		}

		FT_New_Face( FTLibrary, TCHAR_TO_ANSI( Path ), 0, &Face );
		//debugf( TEXT("TrueTypeFont name was %s"), ANSI_TO_TCHAR( FT_Get_Postscript_Name(Face) ) );
		FT_Set_Char_Size( Face, 0, (Parameters.Tall) << 6, 0, 0 );

		Ascent = (Face->ascender >> 6);
		Ascent = Parameters.Tall;
		Descent = -(Face->descender >> 6);
		Height = Ascent + Descent + Parameters.DropShadowOffset;
	}

	~FFreetypeFontProvider()
	{
		if (Face)
			FT_Done_Face( Face );

		if (!--FT_Ref)
			FT_Done_FreeType( FTLibrary );
	}

	virtual void GetPureCharABCWidths( WCHAR CharCode, INT& a, INT& b, INT& c )
	{
		FT_UInt glyphIndex = FT_Get_Char_Index( Face, CharCode );		

		if (Parameters.WidthScaler != 1.0f)
		{
			FT_Matrix matrix;

			matrix.xx = appFloor(Parameters.WidthScaler * (0x10000));
			matrix.yy = (0x10000);
			matrix.xy = matrix.yx = 0;			
			FT_Set_Transform( Face, &matrix, 0 );
		}		

		FT_Load_Glyph( Face, glyphIndex, (Parameters.bAntialiased ? FT_LOAD_NO_BITMAP : FT_LOAD_DEFAULT) | FT_LOAD_FORCE_AUTOHINT ); 

		FT_Face face = Face;
		FT_GlyphSlot slot = Face->glyph;				

		if (slot->format != FT_GLYPH_FORMAT_BITMAP)
		{		
			if (Parameters.bAntialiased)
			{
				FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
			}
			else
			{
				FT_Render_Glyph( slot, FT_RENDER_MODE_MONO ); 
			}		
		}

		if (Parameters.bBold)
		{
			FT_GlyphSlot_Embolden( slot );
		}		

		FT_Glyph g;
		FT_BBox box;						

		if (FT_Get_Glyph(slot, &g) != 0)
		{
			a = 0; b = 0; c = 0;
			return;
		}

		FT_Glyph_Get_CBox(g, FT_GLYPH_BBOX_UNSCALED, &box);		

		FT_Pos advance = (slot->metrics.horiAdvance * Parameters.WidthScaler);
		FT_Pos bearing = (slot->metrics.horiBearingX * Parameters.WidthScaler);

		a = ((box.xMin ) >> 6);
		b = slot->bitmap.width;
		c = ((advance - box.xMax) >> 6);		
	}

	virtual void UpdateKerningInfo( WCHAR CharCode, FFontCharacter* cc )
	{		
		// update kerning info :)
		if (FT_HAS_KERNING( Face ) && CharCode > 0x20 && CharCode < 0x80)
		{
			FT_UInt glyphIndex = FT_Get_Char_Index( Face, CharCode );		

			for (INT PrevChar = 0x20; PrevChar < 0x80; ++PrevChar)
			{
				FT_UInt previous = FT_Get_Char_Index( Face, PrevChar );

				FT_Vector  delta;

				FT_Get_Kerning( Face, previous, glyphIndex, FT_KERNING_DEFAULT, &delta );

				cc->Kerning.Set( PrevChar, delta.x >> 6 );
			}
		}
	}

	virtual void FillCharPixels(WCHAR CharCode, INT CharXL, INT CharYL, DWORD* RGBA, INT Pitch )
	{
		FT_UInt glyphIndex = FT_Get_Char_Index( Face, CharCode );		

		if (Parameters.WidthScaler != 1.0f)
		{
			FT_Matrix matrix;

			matrix.xx = appFloor(Parameters.WidthScaler * (0x10000));
			matrix.yy = (0x10000);
			matrix.xy = matrix.yx = 0;			
			FT_Set_Transform( Face, &matrix, 0 );
		}		

		FT_Load_Glyph( Face, glyphIndex, (Parameters.bAntialiased ? FT_LOAD_NO_BITMAP : FT_LOAD_DEFAULT) | FT_LOAD_FORCE_AUTOHINT ); 		

		FT_Face face = Face;
		FT_GlyphSlot slot = Face->glyph;		

		if (slot->format != FT_GLYPH_FORMAT_BITMAP)
		{		
			if (Parameters.bAntialiased)
			{
				FT_Render_Glyph( slot, FT_RENDER_MODE_NORMAL );
			}
			else
			{
				FT_Render_Glyph( slot, FT_RENDER_MODE_MONO ); 
			}		
		}

		if (Parameters.bBold)
		{
			FT_GlyphSlot_Embolden( slot );
		}

		UBOOL bFirstRound = TRUE;
		for (INT Shadow=Parameters.DropShadowOffset; Shadow>=0; Shadow--)
		{
			const DWORD Color = Shadow ? 0x000000 : 0xffffff;

			if (Parameters.bAntialiased)
			{
				for (INT y=0; y<slot->bitmap.rows; ++y)
				{
					for (INT x=0; x<slot->bitmap.width; ++x)		
					{
						unsigned char src = slot->bitmap.buffer[ slot->bitmap.pitch * y + x ];

						INT centerX = x + Shadow;
						INT centerY = y + Shadow + Ascent - (slot->metrics.horiBearingY >> 6);				

						if ( ( centerY >= 0 ) && ( centerY < CharYL ) )
						{
							DWORD& B = RGBA[ centerY * Pitch + centerX ];

							if (src)
							{
								const BYTE DestAlpha = (B >> 24) & 0xff;
								const BYTE NewAlpha = Clamp( DestAlpha + src, 0, 0xff );

								if (Shadow)
								{
									B = NewAlpha << 24;								
								}							
								else
								{
									const BYTE Whiteness = Clamp( src * 0xff / NewAlpha, 0, 0xff );
									B = (NewAlpha << 24) | (0x010101 * Whiteness);
								}
							}
							else if (bFirstRound)
							{
								B = 0;
							}
						}
					}
				}
			}
			else
			{
				for (INT y=0; y<slot->bitmap.rows; ++y)
				{
					for (INT x=0; x<slot->bitmap.width; ++x)
					{
						unsigned char src = slot->bitmap.buffer[ slot->bitmap.pitch * y + (x >> 3) ] & (1 << (7-(x&7)));
						if (src) src = 0xff;

						INT centerX = x + Shadow;
						INT centerY = y + Shadow + Ascent - (slot->metrics.horiBearingY >> 6);

						if ( ( centerY >= 0 ) && ( centerY < CharYL ) )
						{
							DWORD& B = RGBA[ centerY * Pitch + centerX ];

							B = (((UINT)src) << 24) | Color;
						}
					}
				}
			}

			bFirstRound = FALSE;
		}
	}
};

struct FFontProperties_OffsetTable
{
	USHORT	uMajorVersion;
	USHORT	uMinorVersion;
	USHORT	uNumOfTables;
	USHORT	uSearchRange;
	USHORT	uEntrySelector;
	USHORT	uRangeShift;
};

struct FFontProperties_TableDirectory
{
	char	szTag[4];			//table name
	ULONG	uCheckSum;			//Check sum
	ULONG	uOffset;			//Offset from beginning of file
	ULONG	uLength;			//length of the table in bytes
};

struct FFontProperties_NameTableHeader
{
	USHORT	uFSelector;			//format selector. Always 0
	USHORT	uNRCount;			//Name Records count
	USHORT	uStorageOffset;		//Offset for strings storage, from start of the table
};

struct FFontProperties_NameRecord
{
	USHORT	uPlatformID;
	USHORT	uEncodingID;
	USHORT	uLanguageID;
	USHORT	uNameID;
	USHORT	uStringLength;
	USHORT	uStringOffset;	//from start of storage area
};

#define SWAPWORD(x)		MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x)		MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

struct FFontProperties
{
	FString			Name, Copyright, Trademark, Family;

	UBOOL ReadFromFile( const TCHAR* Path )
	{
		UBOOL bRetVal = FALSE;

		FArchive* Ar = GFileManager->CreateFileReader( Path );

		if (!Ar)
			return bRetVal;

		FFontProperties_OffsetTable ttOffsetTable;
		Ar->Serialize( &ttOffsetTable, sizeof(FFontProperties_OffsetTable) );
		ttOffsetTable.uNumOfTables = SWAPWORD(ttOffsetTable.uNumOfTables);
		ttOffsetTable.uMajorVersion = SWAPWORD(ttOffsetTable.uMajorVersion);
		ttOffsetTable.uMinorVersion = SWAPWORD(ttOffsetTable.uMinorVersion);

		FFontProperties_TableDirectory tblDir = {0};
		UBOOL bFound = FALSE;
		char temp[1024];
		FString csTemp;

		for(INT i=0; i< ttOffsetTable.uNumOfTables; i++)
		{
			Ar->Serialize(&tblDir, sizeof(FFontProperties_TableDirectory));
			strncpy(temp, tblDir.szTag, 4);			
			csTemp = ANSI_TO_TCHAR( temp );
			if(appStrnicmp( *csTemp, _T("name"), 4 ) == 0)
			{
				bFound = TRUE;
				tblDir.uLength = SWAPLONG(tblDir.uLength);
				tblDir.uOffset = SWAPLONG(tblDir.uOffset);
				break;
			}
			else if(csTemp.Len() == 0)
			{
				break;
			}
		}

		if(bFound)
		{
			Ar->Seek(tblDir.uOffset);
			FFontProperties_NameTableHeader ttNTHeader;
			Ar->Serialize(&ttNTHeader, sizeof(FFontProperties_NameTableHeader));
			ttNTHeader.uNRCount = SWAPWORD(ttNTHeader.uNRCount);
			ttNTHeader.uStorageOffset = SWAPWORD(ttNTHeader.uStorageOffset);
			FFontProperties_NameRecord ttRecord;
			bFound = FALSE;

			for (INT i=0; i<ttNTHeader.uNRCount && (Copyright.Len() == 0 || Name.Len() == 0 || Trademark.Len() == 0 || Family.Len() == 0); i++)
			{
				Ar->Serialize(&ttRecord, sizeof(FFontProperties_NameRecord));
				ttRecord.uNameID = SWAPWORD(ttRecord.uNameID);
				ttRecord.uStringLength = SWAPWORD(ttRecord.uStringLength);
				ttRecord.uStringOffset = SWAPWORD(ttRecord.uStringOffset);

				if(ttRecord.uNameID == 1 || ttRecord.uNameID == 0 || ttRecord.uNameID == 7)
				{
					int nPos = Ar->Tell();
					Ar->Seek(tblDir.uOffset + ttRecord.uStringOffset + ttNTHeader.uStorageOffset);
					
					Ar->Serialize(temp, ttRecord.uStringLength);
					temp[ttRecord.uStringLength] = 0;					

					// Unicode?
					if (temp[ttRecord.uStringLength] <= 0)
					{						
						for (INT i=0; i<ttRecord.uStringLength; i+=2)
						{			
							WORD& w = *(WORD*)(temp + i);
							
							w = SWAPWORD(w);							
						}

						csTemp = (wchar_t*)temp;
					}				
					else
					{
						csTemp = temp;
					}
					
					if(csTemp.Len() > 0)
					{
						switch(ttRecord.uNameID)
						{
						case 1:
							Family.Len() == 0 ? Family = csTemp : void(0);
							bRetVal = TRUE;
							break;
						case 0:
							Copyright.Len() == 0 ? Copyright = csTemp : void(0);
							break;
						case 7:
							Trademark.Len() == 0 ? Trademark = csTemp : void(0);
							break;
						case 4:
							Name.Len() == 0 ? Name = csTemp : void(0);
							break;
						default:
							break;
						}
					}
					Ar->Seek( nPos );
				}
			}			
		}
		Ar->Close();

		delete Ar;

		if(Name == TEXT(""))
			Name = Family;

		return bRetVal;
	}
};

struct FWindowsFontProvider : public FFontRasterizer
{
public :	
	INT MaxCharWidth;	

	HDC hDC;
	HFONT hFont;
	HBITMAP hDIB;
	BYTE* pBuffer;

	UBOOL bInitialized;

	INT XL, YL;

	FWindowsFontProvider( const TCHAR* Path, const TCHAR* FamilyName, const FFontParameters& InParameters )
		: MaxCharWidth( 0 ), bInitialized(FALSE), FFontRasterizer( InParameters ) 
	{
		// Setting caps :)
		Caps_DropShadow = TRUE;

		::AddFontResource( Path );

		hDC = ::CreateCompatibleDC(NULL);	

		INT nLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);

		// see if the font exists on the system
		LOGFONT logfont;
		logfont.lfCharSet = DEFAULT_CHARSET;
		logfont.lfPitchAndFamily = 0;
		_tcscpy(logfont.lfFaceName, FamilyName);
		s_bFontFound = FALSE;

		::EnumFontFamiliesEx(hDC, &logfont, &FontEnumProc, 0, 0);
		if (!s_bFontFound)
		{
			::ReleaseDC( NULL, hDC );			
			return;
		}

		::TEXTMETRIC tm;
		for (INT TryTall = Parameters.Tall;;TryTall--)
		{	
			hFont = ::CreateFont( 
				-TryTall * nLogPixelsY / 72, 0, 0, 0, 
				Parameters.bBold ? FW_BOLD : FW_NORMAL, 
				0/*italic*/, 
				0/*underline*/, 
				0/*strikeout*/, 
				DEFAULT_CHARSET, 
				OUT_DEFAULT_PRECIS, 
				CLIP_DEFAULT_PRECIS, 
				Parameters.bAntialiased ? ANTIALIASED_QUALITY : NONANTIALIASED_QUALITY, 
				DEFAULT_PITCH | FF_DONTCARE, 
				FamilyName );

			if (!hFont)
			{
				::ReleaseDC( NULL, hDC );
				return;
			}

			::SelectObject( hDC, hFont );
			::SetTextAlign( hDC, TA_LEFT | TA_TOP | TA_UPDATECP );

			// get info about the font			
			memset( &tm, 0, sizeof( tm ) );

			if ( !GetTextMetrics(hDC, &tm) )
			{				
				::DeleteObject(hFont);
				::ReleaseDC( NULL, hDC );
				return;
			}

			if (tm.tmHeight <= Parameters.Tall)
				break;

			::DeleteObject(hFont);
		}

		Height = tm.tmHeight + Parameters.DropShadowOffset + 2 * Parameters.OutlineSize;
		MaxCharWidth = tm.tmMaxCharWidth;
		Ascent = tm.tmAscent;
		Descent = tm.tmDescent;

		// code for rendering to a bitmap
		XL = tm.tmExternalLeading + tm.tmMaxCharWidth + Parameters.OutlineSize * 2;
		YL = tm.tmHeight + Parameters.DropShadowOffset + Parameters.OutlineSize * 2;

		::BITMAPINFOHEADER header;
		memset(&header, 0, sizeof(header));
		header.biSize = sizeof(header);
		header.biWidth = XL;
		header.biHeight = -YL;
		header.biPlanes = 1;
		header.biBitCount = 32;
		header.biCompression = BI_RGB;

		hDIB = ::CreateDIBSection( hDC, (BITMAPINFO*)&header, DIB_RGB_COLORS, (void**)(&pBuffer), NULL, 0 );
		::SelectObject( hDC, hDIB );

		bInitialized = TRUE;
	}	

	~FWindowsFontProvider()
	{
		if (bInitialized)
		{
			::DeleteObject(hDIB);
			::DeleteObject(hFont);
			::ReleaseDC( NULL, hDC );
		}		
	}	

	virtual void GetPureCharABCWidths( WCHAR CharCode, INT& a, INT& b, INT& c ) 
	{
		check(bInitialized);

		ABC abc;
		if (::GetCharABCWidthsW(hDC, CharCode, CharCode, &abc))
		{
			a = abc.abcA;
			b = abc.abcB;
			c = abc.abcC;
		}
		else
		{
			// failed to get width, just use the max width
			a = c = 0;
			b = MaxCharWidth;
		}
	}	

	void FillCharPixels( WCHAR CharCode, INT CharXL, INT CharYL, DWORD* RGBA, INT Pitch )
	{
		INT a, b, c;
		GetCharABCWidths( CharCode, a, b, c);

		// set us up to render into our dib
		::SelectObject(hDC, hFont);

		INT wide = b;
		INT tall = Height;
		GLYPHMETRICS glyphMetrics;
		MAT2 mat2 = { { 0, 1}, { 0, 0}, { 0, 0}, { 0, 1} };
		DWORD dwBytesNeeded = 0;

		if (Parameters.bAntialiased)
		{
			// try and get the glyph directly
			::SelectObject(hDC, hFont);
			dwBytesNeeded = ::GetGlyphOutlineW( hDC, CharCode, GGO_GRAY8_BITMAP, &glyphMetrics, 0, NULL, &mat2 );
		}

		if (dwBytesNeeded > 0)
		{
			// take it
			LPBYTE lpbuf = (LPBYTE)_alloca(dwBytesNeeded);
			::GetGlyphOutlineW( hDC, CharCode, GGO_GRAY8_BITMAP, &glyphMetrics, dwBytesNeeded, lpbuf, &mat2 );

			// rows are on DWORD boundaries
			wide = glyphMetrics.gmBlackBoxX;
			while (wide % 4 != 0)
			{
				wide++;
			}

			// see where we should start rendering
			INT pushDown = Ascent - glyphMetrics.gmptGlyphOrigin.y;

			// set where we start copying from
			INT xstart = 0;

			// don't copy the first set of pixels if the antialiased bmp is bigger than the char width
			if ((INT)glyphMetrics.gmBlackBoxX >= b + 2)
			{
				xstart = (glyphMetrics.gmBlackBoxX - b) / 2;
			}

			//debugf( NAME_Log, TEXT("Font Code : %4d, pushDown : %2d, boxY : %2d, orgY : %2d, ascent : %2d"), CharCode, pushDown, glyphMetrics.gmBlackBoxY, glyphMetrics.gmptGlyphOrigin.y, Ascent );

			// iterate through copying the generated dib into the texture
			for (UINT j = 0; j < glyphMetrics.gmBlackBoxY; j++)
			{
				for (UINT i = xstart; i < glyphMetrics.gmBlackBoxX; i++)
				{			
					INT x = i - xstart + Parameters.Blur + Parameters.OutlineSize;
					INT y = j + pushDown;

					if ((x < CharXL) && (y < CharYL))
					{
						unsigned char grayscale = lpbuf[(j*wide+i)];

						FLOAT r, g, b, a;
						if (grayscale)
						{
							r = g = b = 1.0f;
							a = (grayscale + 0) / 64.0f;
							if (a > 1.0f) a = 1.0f;
						}
						else
						{
							r = g = b = a = 0.0f;
						}

						// Don't want anything drawn for tab characters.
						if (CharCode == '\t')
						{
							r = g = b = 0;
						}						

						BYTE A = (unsigned char)(a * 255.0f);				

						RGBA[ y * Pitch + x ] = (A << 24) | 0xffffff;
					}
				}
			}
		}
		else
		{
			// use render-to-bitmap to get our font texture
			::SetBkColor(hDC, RGB(0, 0, 0));			
			SelectObject(hDC, GetStockObject(BLACK_BRUSH)); 
			::Rectangle(hDC, 0, 0, XL, YL);
			::SetBkMode(hDC, OPAQUE);

			int padding = 0 + Parameters.Blur + Parameters.OutlineSize;

			::MoveToEx(hDC, -(INT)a, padding, NULL);

			// render the character
			wchar_t wch = (wchar_t)CharCode;

			if (Parameters.DropShadowOffset)
			{
				::SetTextColor(hDC, RGB(0, 0, 0));
				::MoveToEx(hDC, -(INT)a + Parameters.DropShadowOffset, padding + Parameters.DropShadowOffset, NULL);
				::ExtTextOutW(hDC, 0, 0, 0, NULL, &wch, 1, NULL);
			}

			::SetTextColor(hDC, RGB(255, 255, 255));
			::ExtTextOutW(hDC, 0, 0, 0, NULL, &wch, 1, NULL);
			::SetBkMode(hDC, TRANSPARENT);

			if (wide > XL)
			{
				wide = XL;
			}

			if (tall > YL)
			{
				tall = YL;
			}		

			// iterate through copying the generated dib into the texture
			for (int j = 0; j < tall; j++)
			{			
				for (int i = padding; i < wide - Parameters.DropShadowOffset - padding; i++)
				{
					INT x = i;
					INT y = j;
					if ((x < CharXL) && (y < CharYL))
					{
						unsigned char *src = &pBuffer[(j*XL+i)*4];

						FLOAT r = (src[0]) / 255.0f;
						FLOAT g = (src[1]) / 255.0f;
						FLOAT b = (src[2]) / 255.0f;

						// Don't want anything drawn for tab characters.
						if (CharCode == '\t')
						{
							r = g = b = 0;
						}

						FLOAT a = (r * 0.34f + g * 0.55f + b * 0.11f);

						if (a>0)
						{
							r = Clamp( r / a, 0.0f, 1.0f );
							g = Clamp( g / a, 0.0f, 1.0f );
							b = Clamp( b / a, 0.0f, 1.0f );
						}

						BYTE R = (unsigned char)(r * 255.0f);
						BYTE G = (unsigned char)(g * 255.0f);
						BYTE B = (unsigned char)(b * 255.0f);
						BYTE A = (unsigned char)(a * 255.0f);				

						RGBA[ y * Pitch + x ] = (A << 24) | (R << 16) | (G << 8) | B;						
					}
				}
			}
		}
	}
};

static UTrueTypeFontManager* GTrueTypeFontManager;

#define TTF_TEXTURE_WIDTH 512
#define TTF_TEXTURE_HEIGHT 256

static FTrueTypeFontPage* FirstDirty = NULL;

FArchive& operator<<( FArchive& Ar, FTrueTypeFontPage& Page )
{
	Ar << Page.iClass << Page.Texture << Page.x << Page.y << Page.bDirty << Page.height;
	Ar << Page.Bitmap;

	return Ar;
}

class FUpdateTTFCommand
{
public :
	UINT Execute()
	{
		for (;FirstDirty;FirstDirty=FirstDirty->NextDirty)
		{		
			FTexture2DResource* Resource = (FTexture2DResource*)(FirstDirty->Texture->Resource);

			UINT SrcStride;
			void* Dst = RHILockTexture2D( Resource->Texture2DRHI, 0, TRUE, SrcStride );

			checkSlow( SrcStride == TTF_TEXTURE_WIDTH * sizeof(DWORD) );
			
			appMemcpy( Dst, &FirstDirty->Bitmap(0), sizeof(DWORD) * TTF_TEXTURE_WIDTH * TTF_TEXTURE_HEIGHT );

			RHIUnlockTexture2D( Resource->Texture2DRHI, 0 );
			
			FirstDirty->bDirty = false;
		}	

		FirstDirty = NULL;

		return sizeof(*this);
	}
};	

void UpdateTrueTypeFontMips()
{
	FUpdateTTFCommand command;

	command.Execute();
}

INT UTrueTypeFont::Classify( INT size )
{	
	if (size < 8) return 8;
	if (size < 16) return 16;
	if (size < 32) return 32;
	if (size < 64) return 64;
	if (size < 128) return 128;
	return 256;	
}

UTrueTypeFont::UTrueTypeFont()
: Face(NULL)
{	
}

void UTrueTypeFont::FinishDestroy()
{	
	Uninitialize();

	delete Face;
	Face = NULL;

	Super::FinishDestroy();
}


void UTrueTypeFont::SetFontFamily()
{		
	FString path = appGameDir() + TEXT( "Fonts\\") + FontResource;

	delete Face;

	if (FontFamily == TEXT(""))
	{
		FFontProperties FontProperties;
		if (FontProperties.ReadFromFile( *path ))
		{
			FontFamily = FontProperties.Family;

			debugf( TEXT( "Font family-name is %s for UTrueTypeFont'%s'[%s]"), *FontFamily, *GetFullName(), *FontResource );
		}
		else
		{
			FontFamily = TEXT("굴림");
		}
	}

	FFontParameters FontParameters( Height, Quality != TTQ_Default, bBold, DropShadow, 0, 0, WidthScaler );
	
	//Face = new FWindowsFontProvider( *path, *FontFamily, FontParameters );	
	Face = new FFreetypeFontProvider( *path, FontParameters );	

	iClass = Classify( Height );
}

void UTrueTypeFont::PostEditChange(UProperty* PropertyThatChanged)
{
	Uninitialize();

	bInitialized = FALSE;
}

void UTrueTypeFont::PostLoad()
{
	Super::PostLoad();

	bInitialized = FALSE;

	CharRemap.Empty();

	Characters.Empty();
}

void UTrueTypeFont::Uninitialize()
{
	if (bInitialized)
	{
		CharRemap.Empty();

		Characters.Empty();

		delete Face;

		Face = NULL;
	}	

	bInitialized = FALSE;
}

const WORD* UTrueTypeFont::PrepareOntheFly( WCHAR CharCode )
{
	// Render thread에서는 Locked되지 않은 font 접근 불가
	if (!IsInGameThread() && !bLocked)
		return NULL;

	if (CharCode < 32)
		return NULL;	

	const WORD* result = CharRemap.Find( CharCode );

	if (result)
	{
		return result;
	}

	// Locked 된 상태에서는 새 글자 추가 불가
	if (bLocked)
		return NULL;

	if (!GTrueTypeFontManager)
	{
		GTrueTypeFontManager = ConstructObject<UTrueTypeFontManager>( UTrueTypeFontManager::StaticClass() );		
		GTrueTypeFontManager->AddToRoot();
	}	

	if (!bInitialized)
	{
		SetFontFamily();

		if (!Face)
			return NULL;

		if (bPrecacheAlphabets)
		{
			for (WCHAR CharCode=0x20; CharCode<0x7f; ++CharCode)
				GTrueTypeFontManager->AddLetter( this, CharCode );			
		}

		bInitialized = TRUE;
	}

	if (!Face)
		return NULL;	

	static WORD code;
	
	code = GTrueTypeFontManager->AddLetter( this, CharCode );	

	return &code;
}

FTrueTypeFontPage* UTrueTypeFontManager::FindOrCreatePage( INT width, INT height, INT iClass ) 
{
	FTrueTypeFontPage* page = NULL;

	for (INT iPage=0; iPage<Pages.Num(); ++iPage)
	{
		FTrueTypeFontPage& testPage = Pages(iPage);

		if (testPage.iClass != iClass) continue;

		/// fit?
		if ((DWORD)(testPage.x + width + 1) <= TTF_TEXTURE_WIDTH && (DWORD)(testPage.y + height  + 1) <= TTF_TEXTURE_HEIGHT)
		{
			page = &testPage;
			break;
		}

		if ((DWORD)(testPage.y + height + testPage.height + 1) <= (DWORD)TTF_TEXTURE_HEIGHT)
		{
			testPage.y += testPage.height;
			testPage.height = 0;
			testPage.x = 0;
			page = &testPage;
			break;
		}
	}

	UTexture2D* Texture;

	if (!page)
	{
		page = new (Pages) FTrueTypeFontPage( iClass );			

		page->Bitmap.AddZeroed( TTF_TEXTURE_WIDTH * TTF_TEXTURE_HEIGHT );

		Texture = page->Texture = CastChecked<UTexture2D>(StaticConstructObject(UTexture2D::StaticClass(),this,NAME_None,0));		

		Texture->Init( TTF_TEXTURE_WIDTH, TTF_TEXTURE_HEIGHT, PF_A8R8G8B8 );

		Texture->AddressX = Texture->AddressY = TA_Clamp;
		Texture->LODGroup = TEXTUREGROUP_UI;
		//Texture->UpdateHint = RUH_CacheableDynamic;

		Texture->PostLoad();		
	}
	else
	{
		Texture = page->Texture;
	}		

	return page;
}

WORD UTrueTypeFontManager::AddLetter( UTrueTypeFont* pFont, WORD CharCode )
{
	INT A, B, C;
	FFontRasterizer* Face = pFont->Face;
	Face->GetCharABCWidths( CharCode, A, B, C );

	INT width = B;
	INT height = Face->Height;

	FTrueTypeFontPage* page = FindOrCreatePage( width, height, pFont->iClass );	
	UTexture2D* Texture = page->Texture;		

	INT TextureIndex;
	if (!pFont->TTFTextures.FindItem( Texture, TextureIndex ))
	{
		TextureIndex = pFont->TTFTextures.AddItem( Texture );
	}		

	FFontCharacter* cc = new (pFont->Characters) FFontCharacter;

	cc->StartU = page->x;
	cc->StartV = page->y;
	cc->USize = width;
	cc->VSize = height;
	cc->A = A;
	cc->C = C;
	cc->TextureIndex = TextureIndex;

	Face->UpdateKerningInfo( CharCode, cc );

	TArray<DWORD>& TextureData = page->Bitmap;

	Face->Rasterize( CharCode, width, height, &TextureData(page->y * TTF_TEXTURE_WIDTH + page->x), TTF_TEXTURE_WIDTH );

	page->x += width + 1;
	if (page->height < height)
		page->height = height;

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		PageDirtyCommand,
		FTrueTypeFontPage*,page,page,
	{
		if (!page->bDirty)
		{
			page->bDirty = true;
			page->NextDirty = FirstDirty;
			FirstDirty = page;
		}	
	});

	return pFont->CharRemap.Set( CharCode, pFont->Characters.Num() - 1 );
}

void UTrueTypeFontManager::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );

	Ar << Pages;
}

void FlushTrueTypeFontPages()
{
	if (GTrueTypeFontManager == NULL)
		return;

	// Flush all true type font pages
	FlushRenderingCommands();

	for (INT i=0; i<GTrueTypeFontManager->Pages.Num(); ++i)
	{
		FTrueTypeFontPage& Page = GTrueTypeFontManager->Pages(i);

		Page.x = 0;
		Page.y = 0;

		appMemzero( &Page.Bitmap(0), Page.Bitmap.Num() * sizeof(DWORD) );
	}

	for (TObjectIterator<UTrueTypeFont> It; It; ++It)
	{
		UTrueTypeFont* Font = *It;

		Font->Characters.Empty();
		Font->CharRemap.Empty();
		Font->Textures.Empty();
	}

	for (TObjectIterator<UTrueTypeFont> It; It; ++It)
	{
		UTrueTypeFont* Font = *It;
		if (Font->bPrecacheAlphabets && Font->bInitialized)
		{
			for (WCHAR CharCode=0x20; CharCode<0x7f; ++CharCode)
				GTrueTypeFontManager->AddLetter( Font, CharCode );			
		}
	}
}

IMPLEMENT_CLASS(UTrueTypeFont);
IMPLEMENT_CLASS(UTrueTypeFontManager);