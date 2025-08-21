#pragma once
#ifndef __AVA_SORT_TEMPLATE__
#define __AVA_SORT_TEMPLATE__

#include "AvaTemplate.h"

struct FUIListSortingParameters;
struct FUIListItem;

/************************************************************************/
/* avaNet_SortHelper		                                            */
/************************************************************************/
template< typename _PrimarySortType, typename _SecondarySortType = _PrimarySortType >
struct UIListItemSortBase
{
	UIListItemSortBase( const FUIListSortingParameters& SortParms, _PrimarySortType PriSort, UBOOL bPriProcessed = TRUE, UBOOL bSecProcessed = TRUE ) 
		: SortParameters(SortParms), PrimarySort(PriSort), SecondarySort(PriSort), bPriProcessedText(bPriProcessed), bSecProcessedText(bSecProcessed) {}

	UIListItemSortBase( const FUIListSortingParameters& SortParms, _PrimarySortType PriSort, _SecondarySortType SecSort, UBOOL bPriProcessed = TRUE, UBOOL bSecProcessed = TRUE ) 
		: SortParameters(SortParms), PrimarySort(PriSort), SecondarySort(SecSort), bPriProcessedText(bPriProcessed), bSecProcessedText(bSecProcessed) {}

	UBOOL operator() ( FUIListItem& lhs, FUIListItem& rhs )
	{
		INT PrimaryCellIndex = SortParameters.PrimaryIndex;
		INT SecondaryCellIndex = SortParameters.SecondaryIndex;
		if( lhs.Cells.IsValidIndex(PrimaryCellIndex) && rhs.Cells.IsValidIndex(PrimaryCellIndex) && 
			lhs.Cells(PrimaryCellIndex).ValueString != NULL && rhs.Cells(PrimaryCellIndex).ValueString != NULL)
		{
			static const FString StrEmpty = "";
			const FString* StrLeft = &StrEmpty;
			const FString* StrRight = &StrEmpty;

			UUIListString& ListStrLeft = *lhs.Cells(PrimaryCellIndex).ValueString;
			UUIListString& ListStrRight = *rhs.Cells(PrimaryCellIndex).ValueString;

			for( INT NodeIndex = 0 ; NodeIndex < ListStrLeft.Nodes.Num() ; NodeIndex++ )
			{
				FUIStringNode* Node = ListStrLeft.Nodes(NodeIndex);
				if( Node != NULL )
				{
					if( ! bPriProcessedText && Node->SourceText.Len() > 0 )
					{
						StrLeft = &Node->SourceText;
						break;
					}
					else if( Node->IsTextNode() && ((FUIStringNode_Text*)Node)->RenderedText.Len() > 0 )
					{
						StrLeft = &(((FUIStringNode_Text*)Node)->RenderedText);
						break;
					}
				}
			}

			for( INT NodeIndex = 0 ; NodeIndex < ListStrRight.Nodes.Num() ; NodeIndex++ )
			{
				FUIStringNode* Node = ListStrRight.Nodes(NodeIndex);
				if( Node != NULL )
				{
					if( ! bPriProcessedText && Node->SourceText.Len() > 0 )
					{
						StrRight = &Node->SourceText;
						break;
					}
					else if( Node->IsTextNode() && ((FUIStringNode_Text*)Node)->RenderedText.Len() > 0 )
					{
						StrRight = &(((FUIStringNode_Text*)Node)->RenderedText);
						break;
					}
				}
			}

			// 복사하면 느려서 안된다.
			//FString StrLeft = lhs.Cells(PrimaryCellIndex).ValueString->GetValue(bUseProcessedText);
			//FString StrRight = rhs.Cells(PrimaryCellIndex).ValueString->GetValue(bUseProcessedText);

			// 음; 최적화 과정에 이건 빼는게 좋겠다 (스트링 크기가 큰게 들어오면 복사가 심하다 )
			//if( !SortParameters.bCaseSensitive)
			//{
			//	StrLeft = StrLeft.ToLower();
			//	StrRight = StrRight.ToLower();
			//}
			//UBOOL PriLessThan = PrimaryStringLessThan( *StrLeft, *StrRight, !SortParameters.bReversePrimarySorting );
			//UBOOL PriGreaterThan = PrimaryStringLessThan( *StrRight, *StrLeft, !SortParameters.bReversePrimarySorting );

			UBOOL PriLessThan = PrimarySort( *StrLeft, *StrRight );
			UBOOL PriGreaterThan = PrimarySort( *StrRight, *StrLeft );
			if( SortParameters.bReversePrimarySorting )
				ava::swap(PriLessThan, PriGreaterThan);

			if( ( !PriLessThan && !PriGreaterThan ) &&
				lhs.Cells.IsValidIndex(SecondaryCellIndex) && rhs.Cells.IsValidIndex(SecondaryCellIndex) && 
				lhs.Cells(SecondaryCellIndex).ValueString != NULL && rhs.Cells(SecondaryCellIndex).ValueString != NULL)
			{
				StrLeft = &StrEmpty;
				StrRight = &StrEmpty;

				UUIListString& ListStrLeft = *lhs.Cells(SecondaryCellIndex).ValueString;
				UUIListString& ListStrRight = *rhs.Cells(SecondaryCellIndex).ValueString;

				for( INT NodeIndex = 0 ; NodeIndex < ListStrLeft.Nodes.Num() ; NodeIndex++ )
				{
					FUIStringNode* Node = ListStrLeft.Nodes(NodeIndex);
					if( Node != NULL )
					{
						if( ! bSecProcessedText && Node->SourceText.Len() > 0 )
						{
							StrLeft = &Node->SourceText;
							break;
						}
						else if( Node->IsTextNode() && ((FUIStringNode_Text*)Node)->RenderedText.Len() > 0 )
						{
							StrLeft = &(((FUIStringNode_Text*)Node)->RenderedText);
							break;
						}
					}
				}

				for( INT NodeIndex = 0 ; NodeIndex < ListStrRight.Nodes.Num() ; NodeIndex++ )
				{
					FUIStringNode* Node = ListStrRight.Nodes(NodeIndex);
					if( Node != NULL )
					{
						if( ! bSecProcessedText && Node->SourceText.Len() > 0 )
						{
							StrRight = &Node->SourceText;
							break;
						}
						else if( Node->IsTextNode() && ((FUIStringNode_Text*)Node)->RenderedText.Len() > 0 )
						{
							StrRight = &(((FUIStringNode_Text*)Node)->RenderedText);
							break;
						}
					}
				}

				// 복사하면 느려서 안되고 여기서하는 대소문자 처리는 뺀다
				//StrLeft = lhs.Cells(SecondaryCellIndex).ValueString->GetValue(bUseProcessedText);
				//StrRight = rhs.Cells(SecondaryCellIndex).ValueString->GetValue(bUseProcessedText);
				//if( !SortParameters.bCaseSensitive )
				//{
				//	StrLeft = StrLeft.ToLower();
				//	StrRight = StrRight.ToLower();
				//}
				//return SecondaryStringLessThan( *StrLeft, *StrRight, !SortParameters.bReversePrimarySorting );
				UBOOL bSecLessThan = SecondarySort( *StrLeft, *StrRight );
				return SortParameters.bReversePrimarySorting ? !bSecLessThan : bSecLessThan;
				//return SortParameters.bReverseSecondarySorting ? !bSecLessThan : bSecLessThan;
			}
			else
			{
				return PriLessThan;
			}
		}
		else
		{
			return FALSE;
		}
	}

	//virtual UBOOL PrimaryStringLessThan( const FString& lhs, const FString& rhs , UBOOL bAscent )=0;
	//virtual UBOOL SecondaryStringLessThan( const FString& lhs, const FString &rhs, UBOOL bAscent )=0;

	FUIListSortingParameters SortParameters;
	UBOOL bPriProcessedText;
	UBOOL bSecProcessedText;
	_PrimarySortType PrimarySort;
	_SecondarySortType SecondarySort;
};

template< typename _SortTypePri, typename _SortTypeSec >
inline UIListItemSortBase< _SortTypePri, _SortTypeSec > UIListItemSort ( const FUIListSortingParameters& SortParms, const _SortTypePri SortPri, const _SortTypeSec SortSec, UBOOL bPriProcessed = TRUE, UBOOL bSecProcessed = TRUE  )
{
	return UIListItemSortBase< _SortTypePri, _SortTypeSec>( SortParms, SortPri, SortSec, bPriProcessed, bSecProcessed );
}

// 두개의 SortPredecate 들을 합침.
template< typename _SortTypePri, typename _SortTypeSec >
struct binder2sort
{
	binder2sort( _SortTypePri _S1, _SortTypeSec _S2 ) : S1(_S1), S2(_S2) {}
	UBOOL operator() (const FString& lhs, const FString& rhs) 
	{
		UBOOL bCompareRes = S1( lhs , rhs );
		if( bCompareRes )
			return bCompareRes;
		bCompareRes =  S1( rhs, lhs );
		if( bCompareRes )
			return !bCompareRes;

		return S2( lhs, rhs );
	}

	_SortTypePri S1;
	_SortTypeSec S2;
};

// binder2sort를 바로 사용하기 위한 함수
template< typename _SortTypePri, typename _SortTypeSec >
inline binder2sort< _SortTypePri, _SortTypeSec > bind2Sort( const _SortTypePri SortPri, const _SortTypeSec SortSec )
{
	return binder2sort<_SortTypePri, _SortTypeSec>( SortPri, SortSec );
}

struct NormalStringSort
{
	UBOOL operator() (const FString& lhs, const FString& rhs) { return (lhs < rhs); }
};

struct NormalIntegerSort
{
	UBOOL operator() (const FString& lhs, const FString& rhs) { return (appAtoi(*lhs) < appAtoi(*rhs)); }
};

struct PartialIntegerSort
{
	UBOOL operator() (const FString& lhs, const FString& rhs ) { return (GetPartialInteger(lhs) < GetPartialInteger(rhs)); }

private:
	INT GetPartialInteger( const FString& TargetString )
	{
		const TCHAR* CurrChar = *TargetString;
		INT Length = TargetString.Len();
		for ( INT i = 0 ; i < Length ; i++ )
			if( appIsDigit( *(CurrChar + i )) )
				return appAtoi(CurrChar + i);

		return 0;
	}
};

struct PartialStringSort
{
	PartialStringSort( UBOOL _bIgnoreAlpha = FALSE, UBOOL _bIgnoreDigit = FALSE, UBOOL _bIgnoreWhiteSpace = FALSE ) : bIgnoreAlpha( _bIgnoreAlpha), bIgnoreDigit(_bIgnoreDigit), bIgnoreWhiteSpace(_bIgnoreWhiteSpace) {}
	UBOOL operator() (const FString& lhs, const FString& rhs) { return GetFilteredStr(lhs) < GetFilteredStr(rhs); }
private:
	inline FString GetFilteredStr( const FString& TargetString )
	{
		FString Result = TargetString;
		INT ResCh = 0;
		for( INT ch = 0 ; ch < TargetString.Len() ; ch++ )
		{
			if( (!bIgnoreAlpha && appIsChar(TargetString[ch])) ||
				(!bIgnoreDigit && appIsDigit(TargetString[ch])) ||
				(!bIgnoreWhiteSpace && appIsWhitespace(TargetString[ch])) )
			{
				Result[ResCh] = TargetString[ch];
				ResCh++;
			}
		}
		return Result.Left(ResCh);
	}

	UBOOL bIgnoreAlpha;
	UBOOL bIgnoreDigit;
	UBOOL bIgnoreWhiteSpace;
};

template<class _ParseArgType>
struct ParseArgumentSort
{
	ParseArgumentSort( const FString PrefixExpected ,UBOOL bReverseFind = FALSE, UBOOL bIgnoreCharCase = FALSE ) 
		: bReverse(bReverseFind), bIgnoreCase( bIgnoreCharCase ), Prefix(PrefixExpected) {}

	UBOOL operator() ( const FString& lhs, const FString& rhs ) { return (GetParsedToken(lhs) < GetParsedToken(rhs));}

private:

	inline const TCHAR* _StrrStr( const TCHAR* Str, const TCHAR* Find, UBOOL bIgnoreCase )
	{
		// both strings must be valid
		if( Find == NULL || Str == NULL )
		{
			return NULL;
		}

		INT Length = appStrlen(Str);
		INT FindLength = appStrlen(Find);

		if( FindLength > Length )
			return NULL;

		TCHAR FindFinal = *(Find + FindLength - 1);
		FindFinal = bIgnoreCase ? appToUpper(FindFinal) : FindFinal;
		const TCHAR* Strr = Str + Length - 1;

		while( (Strr - Str) >= (FindLength - 1) )
		{
			const TCHAR* CompStr = Strr - (FindLength - 1);
			TCHAR StrChar = bIgnoreCase ? appToUpper(*Strr) : *Strr;
			if( StrChar == FindFinal && 
				( (bIgnoreCase && appStrnicmp(CompStr, Find, FindLength) == 0) || ( !bIgnoreCase && appStrncmp(CompStr, Find, FindLength) == 0) ) )
				return CompStr;
			Strr--;
		}

		// if nothing was found, return NULL
		return NULL;
	}

	inline _ParseArgType GetParsedToken( const FString& Text )
	{
		const TCHAR* szTarget = *Text;
		const TCHAR* szPrefix = *Prefix;
		_ParseArgType ParsedToken = _ParseArgType();

		const TCHAR* CmpRes = NULL;
		if( bReverse )
			CmpRes = _StrrStr( szTarget, szPrefix, bIgnoreCase );			
		else
			CmpRes = bIgnoreCase ? appStristr( szTarget, szPrefix ) : appStrstr(szTarget, szPrefix);

		if( CmpRes )
			Parse( CmpRes , szPrefix, ParsedToken );

		return ParsedToken;
	}

	FString Prefix;
	UBOOL bReverse;
	UBOOL bIgnoreCase;
};

#endif