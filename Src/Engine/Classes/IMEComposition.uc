class IMEComposition extends Object
	abstract
	native;

const IMEPROP_AT_CARET = 0x00010000;

// Declare enum type Languages
enum EKBDINPUTLANGType
{
	KBDINPUTLANG_Default,
	KBDINPUTLANG_TraditionalChinese,
	KBDINPUTLANG_Japanese,
	KBDINPUTLANG_Korean,
	KBDINPUTLANG_SimplifiedChinese,
};

enum EIMECOMPOSITIONType
{
	IMECOMPOSITION_None,
	IMECOMPOSITION_InputLangChanged,
	IMECOMPOSITION_Start,
	IMECOMPOSITION_Composition,
	IMECOMPOSITION_Result,
	IMECOMPOSITION_End,
};

enum EIMENotifyAction
{
	IMENOTIFY_OpenCandidate,
	IMENOTIFY_CloseCandidate,
	IMENOTIFY_SelectCandidatestr,
	IMENOTIFY_ChangeCandidatelist,
	IMENOTIFY_FinalizeConversionResult,
	IMENOTIFY_CompositionStr,
	IMENOTIFY_SetCandidate_PageStart,
	IMENOTIFY_SetCandidate_PageSize,
	IMENOTIFY_ImeMenuSelected,
};

enum EIMENotifyCompositionString
{
	IMENOTIFYCOMPSTR_Cancel,
	IMENOTIFYCOMPSTR_Complete,
	IMENOTIFYCOMPSTR_Convert,
	IMENOTIFYCOMPSTR_Revert,
};

cpptext
{
public:
	static UBOOL StaticNotifyAction( BYTE Action, INT Index, INT Value );
	static BYTE StaticGetImeLanguage();
	static INT StaticGetImeProperty();
}

// GameViewportClient::InputChar에서 사용할 CompositionString정보
struct native InputCompositionStringData
{
	var EIMECOMPOSITIONType		CompType;				// what type of comp message
	var init string				CompStr;				// if bCompStr FALSE then CompStr means ResultStr
	var init array<byte>		CompAttr;				// Traditional Chinese, Japanese Support
	var int						CompCursorPos;			// ignore level 3 feature flag when inputlangchanged. otherwise supporting for Simplified Chinese
	var init array<int>			CompClause;				// Japanese Support

	structcpptext
	{
		FInputCompositionStringData( BYTE InCompType = IMECOMPOSITION_None ) : CompCursorPos(0), CompType(InCompType) {}
		FInputCompositionStringData( BYTE InCompType, const FString& InResStr ) : CompCursorPos(0), CompStr(InResStr), CompType(InCompType) {}
		FInputCompositionStringData( BYTE InCompType, const FString& InCompStr, const TArray<BYTE>& InCompAttr, INT InCompCursorPos, const TArray<INT>& InCompClause ) 
			: CompStr(InCompStr), CompAttr(InCompAttr), CompCursorPos(InCompCursorPos), CompClause(InCompClause), CompType(InCompType) { checkf( InCompType == IMECOMPOSITION_Composition ); }
	}
};

struct native InputCandidateStringData
{
	var init array<string>		StrList;
	var int						Count;
	var int						Selection;				// index of selected candidate string
	var int						PageStart;				// index of first candidate string
	var int						PageSize;

	structcpptext
	{
		FInputCandidateStringData( TArray<FString>& InStrList, INT InCount, INT InSelection, INT InPageStart, INT InPageSize ) : StrList(InStrList), Count(InCount), Selection(InSelection), PageStart(InPageStart), PageSize(InPageSize) {}
		FInputCandidateStringData() : Count(0),Selection(0),PageStart(0),PageSize(0) {}
	}
};


struct native InputReadingStringData
{
	var init string			ReadingString;				// CHT(ChineseTraditional), CHS(ChineseSimplified)

	structcpptext
	{
		FInputReadingStringData( const FString& InReadingString ) : ReadingString(InReadingString) {}
		FInputReadingStringData() {}
	}
};

static native function bool NotifyAction( EIMENotifyAction Action, int Index, int Value );

static native function BYTE GetImeLanguage();
static native function INT GetImeProperty();