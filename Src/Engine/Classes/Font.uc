/**
 * Copyright 2005-2006 Epic Games, Inc.
 *
 * A font object, containing information about a set of glyphs.
 * The glyph bitmaps are stored in the contained textures, while
 * the font database only contains the coordinates of the individual
 * glyph.
 */
class Font extends Object
	hidecategories(object)
	native;

// This is the character that RemapChar will return if the specified character doesn't exist in the font
const NULLCHARACTER = 127;

struct native FontCharacter
{
	var() int StartU;
	var() int StartV;
	var() int USize;
	var() int VSize;
	var() editconst BYTE TextureIndex;
	var() int A, C;
	var private const native Map{WORD,INT} Kerning;

	structcpptext
	{
		// Serializer.
		friend FArchive& operator<<( FArchive& Ar, FFontCharacter& Ch )
		{
			return Ar << Ch.StartU << Ch.StartV << Ch.USize << Ch.VSize << Ch.TextureIndex;
		}

		FORCEINLINE INT CalculateKerning( INT FontKerning, TCHAR PrevChar ) const
		{
			INT Result = 0;

			if (PrevChar)
			{
				if (Kerning.Num() > 0)
				{
					const INT* CharKerning = Kerning.Find( PrevChar );

					if (CharKerning)
					{
						Result += *CharKerning;
					}
				}

				Result += FontKerning;
			}

			return Result;
		}
	}
};

var() editinline array<FontCharacter> Characters;

//NOTE: Do not expose this to the editor as it has nasty crash potential
var array<Texture2D> Textures;

var private const native Map{WORD,WORD} CharRemap;

var int IsRemapped;
var() int Kerning;

cpptext
{
	void Serialize( FArchive& Ar );

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return		Size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	virtual INT GetResourceSize();

	virtual INT GetNumTextures() const
	{
		return Textures.Num();
	}

	virtual UTexture2D* GetTexture( INT Index ) const
	{
		return Index < Textures.Num() ? Textures(Index) : NULL;
	}

	virtual const WORD* PrepareOntheFly( WCHAR CharCode )
	{ return NULL; }

   	// UFont interface
   	FORCEINLINE TCHAR RemapChar(TCHAR CharCode) const
   	{
   		const WORD UCode = ToUnicode(CharCode);
 
 		const WORD* FontChar = const_cast<UFont*>(this)->PrepareOntheFly( CharCode );
 		if (FontChar != NULL)
 			return (TCHAR)*FontChar;
 

		if ( IsRemapped )
		{
			// currently, fonts are only remapped if they contain Unicode characters.
			// For remapped fonts, all characters in the CharRemap map are valid, so
			// if the characters exists in the map, it's safe to use - otherwise, return
			// the null character (an empty square on windows)
			const WORD* FontChar = CharRemap.Find(UCode);
			if ( FontChar == NULL )
				return UCONST_NULLCHARACTER;

			return (TCHAR)*FontChar;
		}

		// Otherwise, our Characters array will contains 256 members, and is
		// a one-to-one mapping of character codes to array indexes, though
		// not every character is a valid character.
		if ( UCode >= Characters.Num() )
			return UCONST_NULLCHARACTER;

		// If the character's size is 0, it's non-printable or otherwise unsupported by
		// the font.  Return the default null character (an empty square on windows).
		if ( Characters(UCode).VSize == 0 )
			return UCONST_NULLCHARACTER;

		return CharCode;
	}

	FORCEINLINE void GetCharSize(TCHAR InCh, FLOAT& Width, FLOAT& Height, INT ResolutionPageIndex=0, TCHAR PrevChar=0) const
	{
		Width = Height = 0.f;

		const INT Ch = (TCHARU)RemapChar(InCh) + ResolutionPageIndex;
		if( Ch < Characters.Num() )
		{
			const FFontCharacter& Char = Characters(Ch);
			if( GetTexture(Char.TextureIndex) != NULL )
			{
				Width = Char.USize + Char.A + Char.C + Char.CalculateKerning( Kerning, PrevChar );
				Height = Char.VSize;				
			}
		}
	}

	FORCEINLINE INT GetStringSize( const TCHAR *Text, INT ResolutionPageIndex = 0 ) const
	{
		WCHAR PrevChar = 0;
		FLOAT	Width, Height, Total;

		Total = 0.0f;
		while( *Text )
		{
			GetCharSize( *Text, Width, Height, ResolutionPageIndex, PrevChar );
			Total += Width;
			PrevChar = *Text;
			Text++;
		}

		return( appCeil( Total ) );
	}

}

native function int GetResolutionPageIndex(float HeightTest);
native function float GetScalingFactor(float HeightTest);

