#include "EnginePrivate.h"

enum ETrueTypeQuality
{
	TTQ_Default = 0,
	TTQ_Antialiased,
	TTQ_ClearType
};

struct FTrueTypeFontPage
{	
	FTrueTypeFontPage( INT iClass = 0 )
		: iClass( iClass ), x(0), y(0), height(0), Texture(NULL), bDirty(FALSE), Bitmap(NULL)
	{
	}	

	INT									iClass;
	class UTexture2D*					Texture;
	INT									x, y;
	UBOOL								bDirty;
	INT									height;			

	FTrueTypeFontPage*					NextDirty;
	TArray<DWORD>						Bitmap;
};

class FFontRasterizer;

class UTrueTypeFont : public UFont
{
	DECLARE_CLASS(UTrueTypeFont,UFont,CLASS_NoExport,Engine);

public :
	FFontRasterizer* Face;

	FString FontResource;
	FString FontFamily;
	INT Height;
	INT iClass;
	INT DropShadow;
	ETrueTypeQuality Quality;
	BITFIELD bInitialized : 1;	
	BITFIELD bPrecacheAlphabets : 1;
	BITFIELD bBold : 1;
	BITFIELD bLocked : 1;	
	FLOAT WidthScaler;
	TArrayNoInit<class UTexture2D*> TTFTextures;

	UTrueTypeFont();
	virtual void FinishDestroy();
	void SetFontFamily();	
	const WORD* PrepareOntheFly( WCHAR CharCode );
	static int Classify( int height );

	virtual void PostEditChange(UProperty* PropertyThatChanged);

	void Uninitialize();

	virtual void PostLoad();

	virtual INT GetNumTextures() const
	{
		return TTFTextures.Num();
	}

	virtual UTexture2D* GetTexture( INT Index ) const
	{
		return Index < TTFTextures.Num() ? TTFTextures(Index) : NULL;
	}
};

class UTrueTypeFontManager : public UObject
{
	DECLARE_CLASS(UTrueTypeFontManager,UObject,CLASS_NoExport,Engine);

	TArrayNoInit<FTrueTypeFontPage> Pages;

	WORD AddLetter( UTrueTypeFont*, WORD CharCode );

	FTrueTypeFontPage* FindOrCreatePage( int width, int height, int iClass );

	virtual void Serialize( FArchive& Ar );
};