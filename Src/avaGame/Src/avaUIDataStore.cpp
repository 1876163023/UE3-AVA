//=============================================================================
// Redduck - All Rights Reserved.
//=============================================================================
#include "PrecompiledHeaders.h"
#include "avaGame.h"

#include "avaNetClient.h"
#include "avaNetStateController.h"
#include "avaStaticData.h"
#include "ComDef/Inventory.h"

#include "SupportedResolution.h"
#include "AvaSortTemplate.h"

#include "avaTransactions.h"

IMPLEMENT_CLASS(UavaUIParamDataProvider);

IMPLEMENT_CLASS(UavaUIDataStore_AvaGame)

IMPLEMENT_CLASS(UavaUIDataProvider_AvaNetChatMsgs);

IMPLEMENT_CLASS(UavaUIDataProvider_PlayerSkill);
IMPLEMENT_CLASS(UavaUIDataProvider_PlayerAward);
IMPLEMENT_CLASS(UavaUIDataProvider_KeyMap);

IMPLEMENT_CLASS(UavaUIDataProvider_OptionValueList);
IMPLEMENT_CLASS(UavaUIDataProvider_GroupValueList);

IMPLEMENT_CLASS(UavaUIDataProvider_LastResultAwardSkill);
IMPLEMENT_CLASS(UavaUIDataProvider_ChannelListField);

IMPLEMENT_CLASS(UavaUIDataProvider_GameProperty);

static AavaPlayerController* GetavaPlayerOwner(INT Index=-1)
{
	// Attempt to find the first available
	static AavaPlayerController* Result;
	static FLOAT LastTime = -1;

	FLOAT CurrentTime = GWorld->GetRealTimeSeconds();
	if (LastTime == CurrentTime && !GIsEditor)
	{
		return Result;
	}

	LastTime = CurrentTime;

	if (Index < 0)
	{
		for (INT i=0;i<GEngine->GamePlayers.Num(); i++)
		{
			// {{ 20071005 dEAthcURe|HM check valid
			ULocalPlayer* pLocalPlayer = GEngine->GamePlayers(i);
			if(0x0 != pLocalPlayer && FALSE == pLocalPlayer->IsValid()) {
				debugf(TEXT("invalid local player"));
			}

			if(0x0 != pLocalPlayer && TRUE == pLocalPlayer->IsValid())
			//if ( GEngine->GamePlayers(i) )
			// }} 20071005 dEAthcURe|HM check valid
			{
				Index = i;
				break;
			}
		}
	}

	// Cast and return

	ULocalPlayer* CurrentPlayer = GEngine->GamePlayers.Num() > 0 ? GEngine->GamePlayers(Index) : NULL;
	if ( CurrentPlayer )
	{
		AavaPlayerController* avaPC = Cast<AavaPlayerController>( CurrentPlayer->Actor );

		Result = avaPC;

		return avaPC;
	}
	return NULL;
}

/************************************************************************/
/* avaUIParamDataProvider	                                            */
/************************************************************************/

/** 
* FieldName에 있는 Parameter를 읽어들임
*
* @return 업데이트로 값이 변했는지의 여부
*/
UBOOL UavaUIParamDataProvider::UpdateParameters( FString& FieldStr, UBOOL bCutParamStr /*=TRUE*/ )
{
	UBOOL bValueChanged = FALSE;
	NameValueMapType NameValueMap;

	FString FieldName = FieldStr;
	FString ParamStr;
	ParseParameter( FieldName, ParamStr, NameValueMap );

	if( NameValueMap.Num() )
	{
		FName NewName;
		for( UProperty* Property = GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext )
		{
			FString* StrPtr = NULL;
			if( (Property->PropertyFlags & CPF_Input) && (StrPtr = NameValueMap.Find(Property->GetFName()) ) != NULL )
			{
				FString& StrRef = *StrPtr;
				UPropertyValue NewValue, OldValue;
				BYTE* PropAddr = (BYTE*)this + Property->Offset;

				Property->GetPropertyValue( PropAddr, OldValue );

				if( Property->IsA(UBoolProperty::StaticClass()) )
				{
					NewValue.BoolValue = appAtoi(*StrRef) > 0 || appStricmp( *StrRef, GTrue) == 0 ? TRUE : FALSE;
					bValueChanged = bValueChanged || (OldValue.BoolValue != NewValue.BoolValue);
				}
				else if( Property->IsA(UByteProperty::StaticClass()) )
				{
					NewValue.ByteValue = appAtoi(*StrRef);
					bValueChanged = bValueChanged || (OldValue.ByteValue != NewValue.ByteValue);
				}
				else if ( Property->IsA(UIntProperty::StaticClass()) )
				{
					NewValue.IntValue = appAtoi(*StrRef);
					bValueChanged = bValueChanged || (OldValue.IntValue != NewValue.IntValue);
				}
				else if ( Property->IsA(UFloatProperty::StaticClass()) )
				{
					// @TODO : 실수를 나타내는 점(dot)이 MarkupString의 하위관계를 나타내는 점과 표시가 같다
					// 따라서 FieldName에 소수점이 들어가면 Markup을 읽지못하는 문제가 있음
					checkf(FALSE, TEXT("do not use float value as a paramter"));
					NewValue.FloatValue = appAtof(*StrRef);
					bValueChanged = bValueChanged || (OldValue.FloatValue != NewValue.FloatValue);
				}
				else if ( Property->IsA(UStrProperty::StaticClass()) )
				{
					NewValue.StringValue = &StrRef;
					bValueChanged = bValueChanged || (OldValue.StringValue != NewValue.StringValue);
				}
				else if ( Property->IsA(UNameProperty::StaticClass()) )
				{
					NewName = FName(*StrRef);
					NewValue.NameValue = &NewName;
					bValueChanged = bValueChanged || (OldValue.NameValue != NewValue.NameValue);
				}
				else
					checkf(FALSE, TEXT("not implemented yet"));

				Property->SetPropertyValue( PropAddr , NewValue );
			}
		}
	}

	if( bCutParamStr )
		FieldStr = FieldName;

	PostUpdateParamters( bValueChanged );

	return FALSE;
}

/** 
* 읽어들여온 FieldName에서 파라메터부분을 추출. 파라메터 부분을 잘라낼지 결정
*
*/
void UavaUIParamDataProvider::ParseParameter( FString& FieldName, FString& ParameterStr, NameValueMapType& NameValueMap )
{
	TArray<FString> ContextTokens;
	if( FieldName.ParseIntoArray( &ContextTokens, *FieldDelimiter, TRUE ) >= 2 )
	{
		TArray<FString> ParamTokens;
		if( ContextTokens(1).ParseIntoArray( &ParamTokens, TEXT(","), TRUE) > 0 )
		{
			for( INT ParmIdx = 0 ; ParmIdx < ParamTokens.Num() ; ParmIdx++ )
			{
				TArray<FString> AtomicTokens;
				if( ParamTokens(ParmIdx).ParseIntoArray( &AtomicTokens, TEXT("="), TRUE) >= 2 )
				{
					FString ValueStr = AtomicTokens(1).Trim().TrimTrailing().TrimQuotes();
					FString KeyStr = AtomicTokens(0).Trim().TrimTrailing().TrimQuotes();
					NameValueMap.Set( FName(*KeyStr), *ValueStr );
				}
			}
		}
	}
}

/**
* 파라메터들을 초기화한다
*
* @param	bUseDefault		0이나 ""로 초기화하는대신 DefaultObject의 값을 얻어와 쓴다.
*/
void UavaUIParamDataProvider::ClearParameters( UBOOL bUseDefault /*=TRUE*/ )
{
	UObject* DefaultObject = GetClass()->GetDefaultObject();
	for( UProperty* Property = GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext )
	{
		BYTE* DefPropAddr = (BYTE*)DefaultObject + Property->Offset;
		BYTE* PropAddr = (BYTE*)this + Property->Offset;

		if( bUseDefault )
		{
			Property->CopyCompleteValue( PropAddr, DefPropAddr );
		}
		else
		{
			Property->ClearValue(PropAddr);
		}
	}	
}

/** 
* 설정 가능한 파라메터 이름을 얻어온다
*
*/
void UavaUIParamDataProvider::GetSupportedParameterNames( TArray<FName>& OutPropertyNames )
{
	TArray<UProperty*> Props;
	GetSupportedParameterProps( Props );
	for( INT i = 0 ; i < Props.Num() ; i++ )
	{
		if( Props(i) != NULL )
			OutPropertyNames.AddItem( Props(i)->GetFName() );
	}
}

void UavaUIParamDataProvider::GetSupportedParameterProps( TArray<UProperty*>& OutProperties )
{
	for( UProperty* Property = GetClass()->PropertyLink; Property; Property = Property->PropertyLinkNext )
	{
		if( (Property->PropertyFlags & CPF_Input) && Property->GetFName() != NAME_None )
			OutProperties.AddItem( Property );
	}
}

void UavaUIDataStore_AvaGame::InitializeDataStore()
{
	if( AvaNetChatMsgProvider == NULL )
	{
		AvaNetChatMsgProvider = ConstructObject<UavaUIDataProvider_AvaNetChatMsgs>(UavaUIDataProvider_AvaNetChatMsgs::StaticClass());
	}
	if( PlayerSkillProvider == NULL )
	{
		PlayerSkillProvider = ConstructObject<UavaUIDataProvider_PlayerSkill>(UavaUIDataProvider_PlayerSkill::StaticClass());
	}
	if( PlayerAwardProvider == NULL )
	{
		PlayerAwardProvider = ConstructObject<UavaUIDataProvider_PlayerAward>(UavaUIDataProvider_PlayerAward::StaticClass());
	}
	if( KeyMapProvider == NULL )
	{
		KeyMapProvider = ConstructObject<UavaUIDataProvider_KeyMap>(UavaUIDataProvider_KeyMap::StaticClass());
	}
	if( OptionValueProvider == NULL )
	{
		OptionValueProvider = ConstructObject<UavaUIDataProvider_OptionValueList>(UavaUIDataProvider_OptionValueList::StaticClass());
	}
	if( GroupValueProvider == NULL )
	{
		GroupValueProvider = ConstructObject<UavaUIDataProvider_GroupValueList>(UavaUIDataProvider_GroupValueList::StaticClass());
	}
	if( LastResultAwardSkillProvider == NULL )
	{
		LastResultAwardSkillProvider = ConstructObject<UavaUIDataProvider_LastResultAwardSkill>(UavaUIDataProvider_LastResultAwardSkill::StaticClass());
	}
	if( ChannelListFieldProvider == NULL )
	{
		ChannelListFieldProvider = ConstructObject<UavaUIDataProvider_ChannelListField>(UavaUIDataProvider_ChannelListField::StaticClass());
	}
	if( GamePropertyProvider == NULL )
	{
		GamePropertyProvider = ConstructObject<UavaUIDataProvider_GameProperty>(UavaUIDataProvider_GameProperty::StaticClass());
	}
}

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	out_FieldValue	receives the resolved value for the property specified.
*							@see ParseDataStoreReference for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UavaUIDataStore_AvaGame::GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	if( FieldName == TEXT("RTNoticeMsg"))
	{
		if( GIsEditor && !GIsGame )
		{
			out_FieldValue.StringValue = TEXT("This is the real-time notice message");
		}
		else
		{
			AavaGameReplicationInfo* GRI = NULL;
			FString RTNoticeMsg = _StateController->RTNotice;
			INT ChatStyleID = -1;
			INT LeftBracket = RTNoticeMsg.InStr(TEXT("["));
			INT RightBracket = RTNoticeMsg.InStr(TEXT("]"));
			if( LeftBracket < RightBracket)
				ChatStyleID = appAtoi(*RTNoticeMsg.Mid(LeftBracket + 1));

			if( ChatStyleID >= 0 &&
				GWorld->GetWorldInfo() != NULL  &&
				(GRI = Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI)) != NULL )
			{	
				for( INT i = 0 ; i < GRI->TextStyleData.Num() ; i++ )
				{
					if( ChatStyleID == GRI->TextStyleData(i).Id )
					{
						out_FieldValue.StringValue = FString(TEXT("<Styles:")) + GRI->TextStyleData(i).StyleTag + FString(TEXT(">")) +
							RTNoticeMsg.Mid(RightBracket + 1);
						break;
					}
				}
			}
			else
			{
				out_FieldValue.StringValue = RTNoticeMsg;
			}
		}

		if( out_FieldValue.StringValue.Len() == 0 )
			out_FieldValue.StringValue = TEXT(" ");

		return TRUE;
	}
	return FALSE;
}

/**
* Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	FieldValue		the value to store for the property specified.
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UavaUIDataStore_AvaGame::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex )
{
	return FALSE;
}

/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UavaUIDataStore_AvaGame::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	check( AvaNetChatMsgProvider &&
		KeyMapProvider && OptionValueProvider && GroupValueProvider && LastResultAwardSkillProvider && ChannelListFieldProvider && GamePropertyProvider );
	AvaNetChatMsgProvider->GetSupportedDataFields(out_Fields);
	PlayerSkillProvider->GetSupportedDataFields(out_Fields);
	PlayerAwardProvider->GetSupportedDataFields(out_Fields);
	KeyMapProvider->GetSupportedDataFields(out_Fields);
	OptionValueProvider->GetSupportedDataFields(out_Fields);
	GroupValueProvider->GetSupportedDataFields(out_Fields);
	LastResultAwardSkillProvider->GetSupportedDataFields(out_Fields);
	GamePropertyProvider->GetSupportedDataFields(out_Fields);

	new(out_Fields) FUIDataProviderField(TEXT("LastResultItem"), DATATYPE_Provider, LastResultAwardSkillProvider );
	new(out_Fields) FUIDataProviderField(TEXT("ChannelListField"), DATATYPE_Provider, ChannelListFieldProvider);
	new(out_Fields) FUIDataProviderField(TEXT("RTNoticeMsg"));
	new(out_Fields) FUIDataProviderField(TEXT("GameProperties"),DATATYPE_Provider, GamePropertyProvider);
}
	

/**
* Generates filler data for a given tag.  This is used by the editor to generate a preview that gives the
* user an idea as to what a bound datastore will look like in-game.
*
* @param		DataTag		the tag corresponding to the data field that we want filler data for
*
* @return		a string of made-up data which is indicative of the typical [resolved] value for the specified field.
*/
FString UavaUIDataStore_AvaGame::GenerateFillerData( const FString& DataTag )
{
	return TEXT("TestFiller");
}

/* === IUIListElementProvider interface === */

/**
* Retrieves the list of all data tags contained by this element provider which correspond to list element data.
*
* @return	the list of tags supported by this element provider which correspond to list element data.
*/
TArray<FName> UavaUIDataStore_AvaGame::	GetElementProviderTags()
{
	TArray<FName> Tags;
	Tags.AddItem(FName(TEXT("avaGameStatusEU")));
	Tags.AddItem(FName(TEXT("avaGameStatusNRF")));
	Tags.AddItem(FName(TEXT("avaNetChatMsgs")));
	Tags.AddItem(FName(TEXT("ResList")));
	Tags.AddItem(FName(TEXT("AspectRatioList")));
	Tags.AddItem(FName(TEXT("PlayerSkillList")));
	Tags.AddItem(FName(TEXT("PlayerAwardList")));

	return Tags;
}

/**
* Returns the number of list elements associated with the data tag specified.
*
* @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
*						from GetElementProviderTags.
*
* @return	the total number of elements that are required to fully represent the data specified.
*/
INT UavaUIDataStore_AvaGame::GetElementCount( FName FieldName )
{
	return 1;
}

static QSORT_RETURN CDECL ComparePrimitiveInt( const INT* A, const INT* B )
{
	return *A > *B;
}

/**
* Retrieves the list elements associated with the data tag specified.
*
* @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
*							from GetElementProviderTags.
* @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
*
* @return	TRUE if this data store contains a list element data provider matching the tag specified.
*/
UBOOL UavaUIDataStore_AvaGame::GetListElements( FName FieldName, TArray<INT>& out_Elements )
{
	UBOOL bResult = FALSE;

	if ( AvaNetChatMsgProvider && FieldName == FName(TEXT("avaNetChatMsgs")) )
	{
		if( GIsEditor && !GIsGame )
		{
			for( INT Index = 0 ; Index < 10 ; Index++ )
				out_Elements.AddItem(Index);
		}
		else
		{
			for( INT Index = 0 ; Index < _StateController->ChatMsgList.ChatList.Num() ; Index++ )
				out_Elements.AddItem(Index);
		}
	}
	else if ( PlayerSkillProvider && appStristr( FieldName.GetName(), TEXT("PlayerSkillList") ) == FieldName.GetName() )
	{
		PlayerSkillProvider->ClassIndex = INDEX_NONE;
		INT ClassIndex = INDEX_NONE;
		FString ClassStr;

		if( Parse(FieldName.GetName(), TEXT("Class="), ClassStr) )
		{
			FString ClassStrLowerCase = ClassStr.ToLower();
			if( ClassStrLowerCase == TEXT("p") || ClassStrLowerCase == TEXT("pointman") )
				ClassIndex = _CLASS_POINTMAN;
			else if ( ClassStrLowerCase == TEXT("r") || ClassStrLowerCase == TEXT("rifleman") )
				ClassIndex = _CLASS_RIFLEMAN;
			else if ( ClassStrLowerCase == TEXT("s") || ClassStrLowerCase == TEXT("sniper") )
				ClassIndex = _CLASS_SNIPER;
			PlayerSkillProvider->ClassIndex = ClassIndex;
		}

		INT GroupIndex = INDEX_NONE;
		Parse(FieldName.GetName(), TEXT("Group="), GroupIndex);

		if( 0 <= ClassIndex && ClassIndex < _CLASS_MAX )
		{
			for( INT i = 0 ; i < MAX_SKILL_PER_CLASS ; i++ )
			{
				FSkillInfo* SkillInfo = _SkillDesc().GetSkillInfo(ClassIndex,i);
				if( SkillInfo != NULL && (GroupIndex == INDEX_NONE || GroupIndex == SkillInfo->TypeID) )
					out_Elements.AddItem(i);
			}
		}

		bResult = TRUE;
	}
	else if ( PlayerSkillProvider && FieldName == FName(TEXT("LastResultSkillList")) )
	{
		if(GIsEditor && !GIsGame)
		{
			for( INT i = 0 ; i < _CLASS_MAX ; i++)
				for( INT j = 1 ; j <= 1 ; j++)
					if( _SkillDesc().GetSkillInfo(i,j) )
						out_Elements.AddItem( _SkillDesc().GetSkillInfo(i,j)->id );
		}
		else
		{
			for( INT ClassIndex = 0 ; ClassIndex < Def::_CLASS_MAX ; ClassIndex++ )
			{
				for( INT SkillIndex = 0 ; SkillIndex < _StateController->LastResultInfo.SkillList[ClassIndex].Num() ; SkillIndex++ )
				{
					out_Elements.AddItem( _StateController->LastResultInfo.SkillList[ClassIndex](SkillIndex) );
				}
			}
		}
		bResult = TRUE;
	}
	else if ( PlayerAwardProvider && appStristr( FieldName.GetName(), TEXT("PlayerAwardList") ) == FieldName.GetName() )
	{
		INT GroupIndex = INDEX_NONE;
		Parse( FieldName.GetName(), TEXT("GroupIndex="), GroupIndex );

		for( INT i = 0 ; i < MAX_AWARD_PER_PLAYER  ; i++)
		{
			FAwardInfo* AwardInfo = _AwardDesc().GetAwardInfo(i);
			if( AwardInfo != NULL && (GroupIndex == INDEX_NONE || GroupIndex == AwardInfo->Type) )
			{
				out_Elements.AddItem( i );
			}
		}
		//for( INT i = 0 ; i < MAX_AWARD_PER_PLAYER ; i++ )
		//	if( Localize(TEXT("AwardName"), *FString::Printf(TEXT("Name_Award[%d]"), i) ,TEXT("avaNet")).Trim().Len() != 0 )
		//		out_Elements.AddItem(i);

		bResult = TRUE;
	}
	else if ( PlayerAwardProvider && FieldName == FName(TEXT("LastResultAwardList")) )
	{
		if( GIsEditor && !GIsGame )
		{
			for( INT i = 0 ; i < 5 ; i++ )
				out_Elements.AddItem(i);
		}
		else
		{
			for( INT AwardIndex = 0 ; AwardIndex < _StateController->LastResultInfo.AwardList.Num() ; AwardIndex++ )
				out_Elements.AddItem( _StateController->LastResultInfo.AwardList(AwardIndex) );
		}

		bResult = TRUE;
	}
	else if ( KeyMapProvider && appStristr( FieldName.GetName(), TEXT("KeyMapList") ) == FieldName.GetName() )
	{
		FString GroupName;
		UBOOL bGroupFilter = Parse(FieldName.GetName(), TEXT("Group="), GroupName);

		AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
		UPlayerInput* Input = PlayerOwner ? PlayerOwner->PlayerInput : NULL;
		if( Input != NULL )
		{
			for( INT i = 0 ; i < Input->Bindings.Num() ; i++ )
			{
				const FKeyBind& KeyBind = Input->Bindings(i);
				if( KeyBind.Slot != 0 && 
					(!bGroupFilter || GroupName == FString(KeyBind.Group.GetName())) )
					out_Elements.AddItem(i);
			}
		}

		bResult = TRUE;
	}
	else if ( OptionValueProvider && appStristr( FieldName.GetName(), TEXT("OptionValueList")) == FieldName.GetName() )
	{
		INT Level = 0;
		if( Parse(FieldName.GetName(), TEXT("Level"), Level) )
		{
			for( INT i = 0 ; i < Level ; i++)
				out_Elements.AddItem(i);
		}

		OptionValueProvider->AvailableItemCount = Level;
		bResult = TRUE;
	}
	else if ( OptionValueProvider && appStristr( FieldName.GetName(), TEXT("LODBiasList")) == FieldName.GetName() )
	{
		INT Level = 0;
		if( Parse(FieldName.GetName(), TEXT("Level"), Level) )
		{
			for( INT i = Level - 1 ; i >= 0 ; i--)
				out_Elements.AddItem(i);
		}

		OptionValueProvider->AvailableItemCount = Level;
		bResult =TRUE;
	}
	else if ( OptionValueProvider && appStristr( FieldName.GetName(), TEXT("AnisotropyModeList")) == FieldName.GetName() )
	{
		extern INT GMaxLogAnisotropy;
		for( INT i = 0 ; i <= GMaxLogAnisotropy ; i++)
			out_Elements.AddItem(i);

		OptionValueProvider->AvailableItemCount = GMaxLogAnisotropy;
		bResult = TRUE;
	}
	else if ( OptionValueProvider && appStristr( FieldName.GetName(), TEXT("AntiAliasingModeList")) == FieldName.GetName() )
	{
		INT Mode = 0;
		FString AAModeName;
		while( RHIGetUserFriendlyAntialiasingName( Mode , AAModeName) )
		{
			out_Elements.AddItem(Mode);
			Mode++;
		}
		OptionValueProvider->AvailableItemCount = Mode;
		bResult = TRUE;
	}
	else if ( OptionValueProvider && appStristr( FieldName.GetName(), TEXT("PostProcessList")) == FieldName.GetName() )
	{
		INT Level[] = { 0, 2 };
		//if( Parse(FieldName.GetName(), TEXT("Level"), Level) )
		//{
		for( INT i = 0 ; i < ARRAY_COUNT(Level) ; i++)
			out_Elements.AddItem(Level[i]);
		//}

		OptionValueProvider->AvailableItemCount = Level[ARRAY_COUNT(Level)-1];
		bResult =TRUE;
	}
	else if ( OptionValueProvider && appStristr( FieldName.GetName(), TEXT("ShaderModelList")) == FieldName.GetName() )
	{
		TArray<INT> Elements;
		INT DeviceShaderVersion = GetDeviceShaderVersion();
		for( INT i = 0 ; i < SP_MaxPlatforms ; i++)
			if( IsAvailShaderPlatform( (EShaderPlatform)i ) )
				Elements.AddItem( GetExtShaderNum((EShaderPlatform)i) );

		// SM2P, SM2, SM3 순으로 나오도록 거꾸로 값을 넣는다.
		for( INT i = Elements.Num() - 1 ; i >= 0 ; i-- )
			out_Elements.AddItem( Elements(i) );
		OptionValueProvider->AvailableItemCount = out_Elements.Num();
		bResult =TRUE;
	}
	else if ( OptionValueProvider && FieldName == FName(TEXT("AudioChannelList")) )
	{
		static const INT AudioChannels[] = { 16, 32, 64, 128 };
		for( INT i = 0 ; i < ARRAY_COUNT(AudioChannels) ; i++ )
			out_Elements.AddItem( AudioChannels[i] );

		OptionValueProvider->AvailableItemCount = ARRAY_COUNT(AudioChannels);
		bResult = TRUE;
	}
	else if ( OptionValueProvider && FieldName == FName(TEXT("StableFrameModeList")) )
	{
		static const INT StableFrameModeNum = 3;
		for( INT i = 0 ; i < StableFrameModeNum ; i++ )
			out_Elements.AddItem(i);

		OptionValueProvider->AvailableItemCount = StableFrameModeNum;
		bResult = TRUE;
	}
	else if ( OptionValueProvider && FieldName == FName(TEXT("DynamicLightModeList")) )
	{
		static const INT DynamicLightModeNum = 3;
		for( INT i = 0 ; i < DynamicLightModeNum ; i++ )
			out_Elements.AddItem(i);

		OptionValueProvider->AvailableItemCount = DynamicLightModeNum;
		bResult = TRUE;
	}
	else if ( GroupValueProvider && appStristr(FieldName.GetName(), TEXT("GroupValueList_Skill")) == FieldName.GetName() )
	{
		GroupValueProvider->ClassIndex = INDEX_NONE;
		INT ClassIndex = INDEX_NONE;
		FString ClassStr;
		if( Parse(FieldName.GetName(), TEXT("Class="), ClassStr ) )
		{
			ClassStr = ClassStr.ToLower();
			if( ClassStr == TEXT("p") || ClassStr == TEXT("pointman"))
				ClassIndex = _CLASS_POINTMAN;
			else if ( ClassStr == TEXT("r") || ClassStr == TEXT("rifleman") )
				ClassIndex = _CLASS_RIFLEMAN;
			else if ( ClassStr == TEXT("s") || ClassStr == TEXT("sniper") )
				ClassIndex = _CLASS_SNIPER;
		}

		if( 0 <= ClassIndex && ClassIndex < _CLASS_MAX )
		{
			for( INT i = 0 ; i < MAX_SKILL_PER_CLASS ; i++ )
			{
				FSkillInfo* SkillInfo = _SkillDesc().GetSkillInfo(ClassIndex, i);
				if( SkillInfo != NULL )
					out_Elements.AddUniqueItem( SkillInfo->TypeID );
			}
			GroupValueProvider->ClassIndex = ClassIndex;
		}
		appQsort(out_Elements.GetTypedData(), out_Elements.Num(), out_Elements.GetTypeSize(), (QSORT_COMPARE)ComparePrimitiveInt);

		bResult = TRUE;
	}
	else if ( GroupValueProvider && FieldName == FName(TEXT("GroupValueList_Award")) )
	{

		bResult = TRUE;
	}
	else if ( LastResultAwardSkillProvider && FieldName == FName(TEXT("LastResultAwardSkillList")) )
	{
		if( GIsEditor && !GIsGame )
		{
			for(INT AwardIndex = 0 ; AwardIndex < 2 ; AwardIndex++ )
			{
				out_Elements.AddItem( AwardIndex );
			}

			//for( INT ClassIndex = 0 ; ClassIndex < _CLASS_MAX ; ClassIndex++ )
			//	for ( INT SkillIndex = 1 ; SkillIndex <=1 ; SkillIndex++ )
			//		out_Elements.AddItem( SkillIndex + UCONST_SKILL_LISTINDEX_BASE * (ClassIndex + 1) );
		}
		else
		{
			//for(INT AwardIndex = 0 ; AwardIndex < 2 ; AwardIndex++ )
			//{
			//	out_Elements.AddItem( AwardIndex );
			//}

			//for( INT ClassIndex = 0 ; ClassIndex < _CLASS_MAX ; ClassIndex++ )
			//	for ( INT SkillIndex = 1 ; SkillIndex <=1 ; SkillIndex++ )
			//		out_Elements.AddItem( SkillIndex + UCONST_SKILL_LISTINDEX_BASE * (ClassIndex + 1) );

			for(INT AwardIndex = 0 ; AwardIndex < _StateController->LastResultInfo.AwardList.Num() ; AwardIndex++ )
			{
				out_Elements.AddItem( _StateController->LastResultInfo.AwardList(AwardIndex) );
			}

			//for( INT ClassIndex = 0 ; ClassIndex < _CLASS_MAX ; ClassIndex++ )
			//	for ( INT SkillIndex = 0 ; SkillIndex < _StateController->LastResultInfo.SkillList[ClassIndex].Num() ; SkillIndex++ )
			//		out_Elements.AddItem( _StateController->LastResultInfo.SkillList[ClassIndex](SkillIndex) + UCONST_SKILL_LISTINDEX_BASE * (ClassIndex + 1) );
		}

		bResult = TRUE;
	}

	return FieldName == FName(TEXT("avaNetChatMsgs")) || bResult;
}

UBOOL UavaUIDataStore_AvaGame::IsElementEnabled( FName FieldName, INT CollectionIndex )
{
	//if( FieldName == TEXT("AnisotropyModeList") )
	//{
	//	extern INT GMaxLogAnisotropy;
	//	return CollectionIndex <= GMaxLogAnisotropy;
	//}
	//else if ( FieldName == TEXT("ShaderModelList") )
	//{
	//	return CollectionIndex <= GetDeviceShaderVersion();
	//}
	//else if ( FieldName == TEXT("AudioChannelList") )
	//{
	//	UAudioDevice* AudioDevice = GEngine && GEngine->Client ? GEngine->Client->GetAudioDevice() : NULL;
	//	return AudioDevice ? CollectionIndex <= AudioDevice->GetMaxChannels() : FALSE;
	//}

	return TRUE;
}

/**
* Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
* Used by the UI editor to know which cells are available for binding to individual list cells.
*
* @param	FieldName		the tag of the list element data provider that we want the schema for.
*
* @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
*			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
*/
TScriptInterface<IUIListElementCellProvider> UavaUIDataStore_AvaGame::GetElementCellSchemaProvider( FName FieldName )
{
	if (FieldName == FName(TEXT("avaNetChatMsgs")))
	{
		return AvaNetChatMsgProvider;
	}
	if ( appStristr( FieldName.GetName(), TEXT("PlayerSkillLIst")) == FieldName.GetName() || FieldName == FName(TEXT("LastResultSkillList")))
	{
		return PlayerSkillProvider;
	}
	if ( appStristr( FieldName.GetName(), TEXT("PlayerAwardLIst")) == FieldName.GetName() || FieldName == FName(TEXT("LastResultAwardList")) )
	{
		return PlayerAwardProvider;
	}
	if ( appStristr( FieldName.GetName(), TEXT("KeyMapList")) == FieldName.GetName() )
	{
		return KeyMapProvider;
	}
	if ( appStristr( FieldName.GetName(), TEXT("OptionValueList")) == FieldName.GetName() || 
		appStristr( FieldName.GetName(), TEXT("LODBiasList")) == FieldName.GetName() || 
		appStristr( FieldName.GetName(), TEXT("ShaderModelList")) == FieldName.GetName() ||
		appStristr( FieldName.GetName(), TEXT("PostProcessList")) == FieldName.GetName() ||
		appStristr( FieldName.GetName(), TEXT("DynamicLightModeList")) == FieldName.GetName() ||
		appStristr( FieldName.GetName(), TEXT("StableFrameModeList")) == FieldName.GetName() ||
		FieldName == FName(TEXT("AudioChannelList")))
	{
		return OptionValueProvider;
	}
	if ( FieldName == FName(TEXT("AnisotropyModeList"))  
		|| FieldName == FName(TEXT("AntiAliasingModeList")))
	{
		return OptionValueProvider;
	}
	if ( appStristr( FieldName.GetName(), TEXT("GroupValueList_Skill") ) == FieldName.GetName()
		|| FieldName == FName(TEXT("GroupValueList_Award")) )
	{
		return GroupValueProvider;
	}
	if ( FieldName == FName(TEXT("LastResultAwardSkillList")) )
	{
		return LastResultAwardSkillProvider;
	}
	return TScriptInterface<IUIListElementCellProvider>();
}

/**
* Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
* of the list element indicated by CellValueProvider.DataSourceIndex
*
* @param	FieldName		the tag of the list element data field that we want the values for
* @param	ListIndex		the list index for the element to get values for
*
* @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
*/
TScriptInterface<class IUIListElementCellProvider> UavaUIDataStore_AvaGame::GetElementCellValueProvider( FName FieldName, INT ListIndex )
{
	return GetElementCellSchemaProvider(FieldName);
}

/**
* Resolves PropertyName into a list element provider that provides list elements for the property specified.
*
* @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
*
* @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
*			there is no list element provider associated with the specified property.
*/
TScriptInterface<class IUIListElementProvider> UavaUIDataStore_AvaGame::ResolveListElementProvider( const FString& PropertyName )
{
	return this;
}

UBOOL UavaUIDataStore_AvaGame::SortListElements(FName CollectionDataFieldName, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	if( ListItems.Num() == 0 || ListItems(0).Cells.Num() == 0 )
		return FALSE;

	UUIList *OwnerList = ListItems(0).Cells(0).OwnerList;
	UUIComp_ListPresenter* ListPresenter = OwnerList ? OwnerList->CellDataComponent : NULL;
	if( ListPresenter == NULL )
		return FALSE;

	FUIElementCellSchema& Schema = ListPresenter->ElementSchema;
	TScriptInterface<class IUIListElementCellProvider> CellProvider = GetElementCellSchemaProvider( CollectionDataFieldName );
	FUIListSortingParameters SortParms = SortParameters;

	FName EmptyName;
	return CellProvider->SortListElements( Schema.Cells.IsValidIndex(SortParms.PrimaryIndex) ? Schema.Cells(SortParms.PrimaryIndex).CellDataField : EmptyName,
		Schema.Cells.IsValidIndex(SortParms.SecondaryIndex) ? Schema.Cells(SortParms.SecondaryIndex).CellDataField : EmptyName, ListItems, SortParms);

}

/************************************************************************/
/* UavaUIDataProver_AvaNetChatMsgs                                      */
/************************************************************************/

void UavaUIDataProvider_AvaNetChatMsgs::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("MsgText")),TEXT("MsgText"));
}

UBOOL UavaUIDataProvider_AvaNetChatMsgs::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	if( GIsEditor && !GIsGame )
	{
		if(CellTag == FName(TEXT("MsgText")))
		{
			out_FieldValue.StringValue = FString(TEXT("[StyleNum]MsgText"));
		}
	}
	else
	{
		if(CellTag == FName(TEXT("MsgText")) && _StateController != NULL)
		{
			AavaGameReplicationInfo* GRI = NULL;
			FString ChatMsg = _StateController->ChatMsgList[_StateController->ChatMsgList.ChatList.Num() - ListIndex -1 ];
			const TCHAR* szChatMsg = *ChatMsg;
			INT ChatStyleID = -1;
			INT LeftBracket = ChatMsg.InStr(TEXT("["));
			INT RightBracket = ChatMsg.InStr(TEXT("]"));
			if( LeftBracket < RightBracket)
				ChatStyleID = appAtoi(*ChatMsg.Mid(LeftBracket + 1));

			if( ChatStyleID >= 0 &&
				GWorld->GetWorldInfo() != NULL  &&
				(GRI = Cast<AavaGameReplicationInfo>(GWorld->GetWorldInfo()->GRI)) != NULL )
			{	
				for( INT i = 0 ; i < GRI->TextStyleData.Num() ; i++ )
				{
					if( ChatStyleID == GRI->TextStyleData(i).Id )
					{
						out_FieldValue.StringValue = FString(TEXT("<Styles:")) + GRI->TextStyleData(i).StyleTag + FString(TEXT(">")) +
							ChatMsg.Mid(RightBracket + 1);
						break;
					}
				}
			}
			else
			{
				out_FieldValue.StringValue = ChatMsg;
			}
		}
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UavaUIDataProvider_AvaNetChatMsgs::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/************************************************************************/
/* UavaUIDataProvider_PlayerSkill                                       */
/************************************************************************/

void UavaUIDataProvider_PlayerSkill::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("SkillIcon")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_SkillIcon"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("SkillName")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_SkillName"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("SkillCondDesc")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_SkillCondDesc"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("SkillEffectDesc")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_SkillEffectDesc"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("SkillExplanation")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_SkillExplanation"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("SkillPassStatus")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_SkillPassStatus"), TEXT("avaNet")) );
	OutCellTags.Set(FName(TEXT("SkillPassFlag")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_SkillPassFlag"), TEXT("avaNet")) );
}

UBOOL UavaUIDataProvider_PlayerSkill::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bHaveSkill = _StateController->PlayerInfo.IsValid() ? _StateController->PlayerInfo.PlayerInfo.skillInfo.skill[ClassIndex] & (0x01 << (ListIndex - 1)) : FALSE;

	if( CellTag == FName(TEXT("SkillIcon")))
	{
		if( 0 <= ClassIndex && ClassIndex < _CLASS_MAX )
		{
			out_FieldValue.StringValue = Localize(TEXT("SkillName"), *FString::Printf(TEXT("Icon_Skill[%d][%d]"),ClassIndex,ListIndex) ,TEXT("avaNet"));	
		}

		if( out_FieldValue.StringValue.Trim().Len() == 0)
		{
			out_FieldValue.StringValue = ClassIndex == _CLASS_POINTMAN ? TEXT("P") :
				ClassIndex == _CLASS_RIFLEMAN ? TEXT("R") :
				ClassIndex == _CLASS_SNIPER ? TEXT("S") :TEXT("?");

		out_FieldValue.StringValue += appItoa(ListIndex);
		}
	}
	else if (CellTag == FName(TEXT("SkillCondDesc")) )
	{
		if( 0 <= ClassIndex && ClassIndex < _CLASS_MAX )
		{
			if( bHaveSkill )
			{
				FSkillInfo* SkillInfo = _SkillDesc().GetSkillInfo( ClassIndex, ListIndex);
				out_FieldValue.StringValue = SkillInfo ? (SkillInfo->CondDesc) : TEXT("Inv Skill Info");
			}
			else
			{
				out_FieldValue.StringValue = Localize(TEXT("PlayerStat"), TEXT("Text_Skill_Unknown"), TEXT("avaNet"));
			}
		}

		if(  out_FieldValue.StringValue.Len() == 0 )
			out_FieldValue.StringValue = TEXT("inv class index");
	}
	else if (CellTag == FName(TEXT("SkillEffectDesc")) )
	{
		if( 0 <= ClassIndex && ClassIndex < _CLASS_MAX )
		{
			if( bHaveSkill )
			{
				FSkillInfo* SkillInfo = _SkillDesc().GetSkillInfo( ClassIndex, ListIndex);
				out_FieldValue.StringValue = SkillInfo ? (SkillInfo->EffectDesc) : TEXT("Inv Skill Info");
			}
			else
			{
				out_FieldValue.StringValue = Localize(TEXT("PlayerStat"), TEXT("Text_Skill_Unknown"), TEXT("avaNet"));
			}
		}

		if(  out_FieldValue.StringValue.Len() == 0 )
			out_FieldValue.StringValue = TEXT("inv class index");
	}
	else if (CellTag == FName(TEXT("SkillExplanation")) )
	{
		if( 0 <= ClassIndex && ClassIndex < _CLASS_MAX )
		{
			FSkillInfo* SkillInfo = _SkillDesc().GetSkillInfo( ClassIndex, ListIndex);
			out_FieldValue.StringValue = SkillInfo ? (SkillInfo->SkillName + TEXT("\n") + SkillInfo->EffectDesc) : TEXT("Inv Skill Info");
		}

		if(  out_FieldValue.StringValue.Len() == 0 )
			out_FieldValue.StringValue = TEXT("inv class index");
	}
	else if (CellTag == FName(TEXT("SkillName")) )
	{
		if( 0 <= ClassIndex && ClassIndex < _CLASS_MAX )
		{
			FSkillInfo* SkillInfo = _SkillDesc().GetSkillInfo( ClassIndex, ListIndex);
			out_FieldValue.StringValue = SkillInfo ? (SkillInfo->SkillName) : TEXT("Inv Skill Info");
		}

		if(  out_FieldValue.StringValue.Len() == 0 )
			out_FieldValue.StringValue = TEXT("inv class index");

	}
	else if ( CellTag == FName(TEXT("SkillPassStatus")) )
	{
		if( _StateController == NULL || _StateController->PlayerInfo.IsValid() == FALSE)
			out_FieldValue.StringValue = TEXT("Invalid info");
		else
		{
			INT SkillBitSize = sizeof(_StateController->PlayerInfo.PlayerInfo.skillInfo.skill[0]) * 8;
			INT SkillIndex = ListIndex % ( SkillBitSize );
			UBOOL bPassFlag = _StateController->PlayerInfo.PlayerInfo.skillInfo.skill[ClassIndex] & (0x01 << (SkillIndex - 1));
			out_FieldValue.StringValue = bPassFlag ? Localize(TEXT("PlayerStat"), TEXT("Text_Skill_Pass"), TEXT("avaNet")) : 
				Localize(TEXT("PlayerStat"), TEXT("Text_Skill_NotYet"), TEXT("avaNet"));
		}
	}
	else if ( CellTag == FName(TEXT("SkillPassFlag")) )
	{
		if( _StateController == NULL || _StateController->PlayerInfo.IsValid() == FALSE)
			out_FieldValue.StringValue = TEXT("Invalid info");
		else
		{
			INT SkillBitSize = sizeof(_StateController->PlayerInfo.PlayerInfo.skillInfo.skill[0]) * 8;
			INT SkillIndex = ListIndex % ( SkillBitSize );
			UBOOL bPassFlag = _StateController->PlayerInfo.PlayerInfo.skillInfo.skill[ClassIndex] & (0x01 << (SkillIndex - 1));
			out_FieldValue.StringValue = bPassFlag ? TEXT("P") : TEXT("F");
		}
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UavaUIDataProvider_PlayerSkill::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/************************************************************************/
/* UavaUIDataProvider_PlayerAward                                       */
/************************************************************************/

void UavaUIDataProvider_PlayerAward::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("AwardIcon")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_AwardIcon"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("AwardName")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_AwardName"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("AwardCount")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_AwardCount"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("WonFlag")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_WonFlag"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("AwardDesc")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_AwardDesc"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("AwardExplanation")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_AwardExplanation"), TEXT("avaNet")));
	OutCellTags.Set(FName(TEXT("AwardProgress")), *Localize(TEXT("PlayerStat"), TEXT("Text_List_AwardProgress"), TEXT("avaNet")));
}

UBOOL UavaUIDataProvider_PlayerAward::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	FAwardInfo* AwardInfo = _AwardDesc().GetAwardInfo(ListIndex);
	if( CellTag == FName(TEXT("AwardIcon")) )
	{
		out_FieldValue.StringValue = AwardInfo ? Localize(TEXT("AwardName"), *FString::Printf(TEXT("Icon_Award[%d]"),ListIndex) ,TEXT("avaNet")) : TEXT("Award Icon");
		//if( _StateController == NULL || _StateController->PlayerInfo.IsValid() == FALSE )
		//	out_FieldValue.StringValue = TEXT("Award Icon");
		//else
		//{
		//	out_FieldValue.StringValue = Localize(TEXT("AwardName"), *FString::Printf(TEXT("Icon_Award[%d]"),ListIndex) ,TEXT("avaNet"));
		//	if( out_FieldValue.StringValue.Trim().Len() == 0)
		//		out_FieldValue.StringValue = FString(TEXT("Award")) + appItoa(ListIndex);
		//}
	}
	else if(CellTag == FName(TEXT("AwardName")) )
	{
		out_FieldValue.StringValue = AwardInfo ? AwardInfo->AwardName : TEXT("Award Icon");

		//if( _StateController == NULL || _StateController->PlayerInfo.IsValid() == FALSE )
		//	out_FieldValue.StringValue = TEXT("Award Name");
		//else
		//{
		//	out_FieldValue.StringValue = Localize(TEXT("AwardName"), *FString::Printf(TEXT("Name_Award[%d]"),ListIndex) ,TEXT("avaNet"));
		//	if( out_FieldValue.StringValue.Trim().Len() == 0)
		//		out_FieldValue.StringValue = FString(TEXT("award")) + appItoa(ListIndex);
		//}
	}
	else if (CellTag == FName(TEXT("WonFlag")))
	{
		if( _StateController == NULL || _StateController->PlayerInfo.IsValid() == FALSE ||
			AwardInfo == NULL || !( 0 <= AwardInfo->id && AwardInfo->id < MAX_AWARD_PER_PLAYER) )
			out_FieldValue.StringValue = TEXT("WonOrNot");
		else
		{
			UBOOL bWonFlag = _StateController->PlayerInfo.PlayerInfo.awardInfo.info[ListIndex] > 0;
			out_FieldValue.StringValue = bWonFlag ? TEXT("W") : TEXT(" ");
		}
	}
	else if ( CellTag == FName(TEXT("AwardCount")) )
	{
		if( _StateController == NULL || _StateController->PlayerInfo.IsValid() == FALSE )
			out_FieldValue.StringValue = TEXT("Award Count");
		else
		{
			if(_StateController->PlayerInfo.PlayerInfo.awardInfo.info[ListIndex] >= Def::MAX_COUNT_PER_AWARD )
				out_FieldValue.StringValue = Localize(TEXT("PlayerStat"), TEXT("Text_ListElem_MaxAwardCount"), TEXT("avaNet"));
			else
				out_FieldValue.StringValue = appItoa(_StateController->PlayerInfo.PlayerInfo.awardInfo.info[ListIndex]) ;
		}
	}
	else if ( CellTag == FName(TEXT("AwardDesc")) )
	{
		out_FieldValue.StringValue = AwardInfo ? AwardInfo->Desc : TEXT("Award Desc");
	}
	else if ( CellTag == FName(TEXT("AwardExplanation")) )
	{
		out_FieldValue.StringValue = AwardInfo ? AwardInfo->AwardName + TEXT("\n\n") + AwardInfo->Desc : TEXT("Award Explanation");
	}
	else if ( CellTag == FName(TEXT("AwardProgress")) )
	{
		extern FString GetAwardGaugeIconString( INT AwardProgress );

		INT pct = GIsEditor && !GIsGame ? 50 : _StateController->PlayerInfo.GetAwardProgress(ListIndex);
		out_FieldValue.StringValue = FString::Printf(TEXT("%s %2d%%"), *GetAwardGaugeIconString(pct), Max(pct, 0));
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UavaUIDataProvider_PlayerAward::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/************************************************************************/
/* UavaUIDataProvider_KeyMap	                                        */
/************************************************************************/

void UavaUIDataProvider_KeyMap::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("UserKeyName")),TEXT("UserKeyName"));
	OutCellTags.Set(FName(TEXT("KeyCombination")),TEXT("KeyCombination"));
	OutCellTags.Set(FName(TEXT("Command")), TEXT("Command"));
	OutCellTags.Set(FName(TEXT("Slot")), TEXT("Slot"));
}

UBOOL UavaUIDataProvider_KeyMap::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	UPlayerInput* Input = PlayerOwner ? PlayerOwner->PlayerInput : NULL;

	if( Input != NULL && Input->Bindings.IsValidIndex(ListIndex) && GEngine->Client != NULL)
	{
		const FKeyBind& KeyBind = Input->Bindings(ListIndex);
		if(CellTag == FName(TEXT("UserKeyName")) )
		{
			out_FieldValue.StringValue = Localize(TEXT("PlayerInput"), *FString::Printf(TEXT("UserKeyName[%d]"),KeyBind.Slot), TEXT("avaGame"));
		}
		else if (CellTag == FName(TEXT("KeyCombination")))
		{
			FString ModKey = KeyBind.Shift ? TEXT("Shift + ") : TEXT("");
			ModKey += KeyBind.Control ? TEXT("Ctrl + ") : TEXT("");
			ModKey += KeyBind.Alt ? TEXT("Alt + ") : TEXT("");
			FString KeyName = Localize(TEXT("PlayerInput"), *FString::Printf(TEXT("KeyCode[%d]"), GEngine->Client->GetKeyCode(KeyBind.Name)), TEXT("avaGame") );
			out_FieldValue.StringValue = ModKey + KeyName;
		}
		else if (CellTag == FName(TEXT("Command")) )
		{
			out_FieldValue.StringValue = KeyBind.Command;
		}
		else if (CellTag == FName(TEXT("Slot")))
		{
			out_FieldValue.StringValue = appItoa(KeyBind.Slot);
		}

	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UavaUIDataProvider_KeyMap::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

// DATATYPE_Collection 에 SetFieldValue하는 것은 구현되어있지 않다.
// SetDataFieldValue::Activated() > SetDataFieldValue::ResolveDataStore() > UUIDataProvider::ParseDataStoreReference 참고
UBOOL UavaUIDataProvider_KeyMap::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex)
{
	return FALSE;
}


/************************************************************************/
/* UavaUIDataProvider_OptionValueList                                   */
/************************************************************************/

void UavaUIDataProvider_OptionValueList::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("CommonOptionValue")),TEXT("CommonOptionValue"));
	OutCellTags.Set(FName(TEXT("AnisotropyOptionValue")), TEXT("AnisotropyOptionValue"));
	OutCellTags.Set(FName(TEXT("AntialiasingOptionValue")), TEXT("AntialiasingOptionValue"));
	OutCellTags.Set(FName(TEXT("LODBiasOptionValue")), TEXT("LODBiasOptionValue"));
	OutCellTags.Set(FName(TEXT("ShaderModelValue")), TEXT("ShaderModelValue"));
	OutCellTags.Set(FName(TEXT("AudioChannelValue")), TEXT("AudioChannelValue"));
	OutCellTags.Set(FName(TEXT("PostProcessValue")), TEXT("PostProcessValue"));
	OutCellTags.Set(FName(TEXT("StableFrameModeValue")), TEXT("StableFrameModeValue"));
	OutCellTags.Set(FName(TEXT("DynamicLightModeValue")), TEXT("DynamicLightModeValue"));
}

UBOOL UavaUIDataProvider_OptionValueList::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	AavaPlayerController* PlayerOwner = GetavaPlayerOwner();
	UPlayerInput* Input = PlayerOwner ? PlayerOwner->PlayerInput : NULL;

	if( CellTag == FName(TEXT("CommonOptionValue")))
	{
		switch( AvailableItemCount )
		{
		case 2:			out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"),*FString::Printf((TEXT("Text_CommonValue_Level2[%d]")), ListIndex), TEXT("avaNet"));	break;
		case 3:			out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"),*FString::Printf((TEXT("Text_CommonValue_Level3[%d]")), ListIndex), TEXT("avaNet"));	break;
		default:		out_FieldValue.StringValue = FString(TEXT("UnknownType ")) + appItoa(ListIndex);	break;
		}
	}
	else if ( CellTag == FName(TEXT("AnisotropyOptionValue")))
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_Anisotropy_Level[%d]"),ListIndex), TEXT("avaNet"));
	}
	else if ( CellTag == FName(TEXT("AntialiasingOptionValue")) )
	{
		FString AAModeName;
		if( RHIGetUserFriendlyAntialiasingName(ListIndex, AAModeName) )
		{
			UBOOL bFurtherMore = TRUE;
			UBOOL bNotUsed = (AAModeName == TEXT("N/A"));
			if( bNotUsed )
			{
				FString NextAAMode;
				bFurtherMore = RHIGetUserFriendlyAntialiasingName( 1, NextAAMode);
			}

			if( bNotUsed )
				out_FieldValue.StringValue = bFurtherMore ? Localize(TEXT("UIOptionScene"), TEXT("Text_AntiAliasing_Level[0]"), TEXT("avaNet")) : Localize(TEXT("UIOptionScene"),TEXT("Text_AntiAliasing_UnAvail"),TEXT("avaNet"));
			else
				out_FieldValue.StringValue = AAModeName;
		}
	}
	else if ( CellTag == FName(TEXT("LODBiasOptionValue")) )
	{
		switch( AvailableItemCount )
		{
		case 2:			out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"),*FString::Printf((TEXT("Text_LODBias_Level2[%d]")), ListIndex), TEXT("avaNet"));	break;
		case 3:			out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"),*FString::Printf((TEXT("Text_LODBias_Level3[%d]")), ListIndex), TEXT("avaNet"));	break;
		default:		out_FieldValue.StringValue = FString(TEXT("UnknownType ")) + appItoa(ListIndex);	break;
		}
	}
	else if ( CellTag == FName(TEXT("ShaderModelValue")) )
	{
		FString ActiveLabel = (ListIndex == GetExtShaderNum(GRHIShaderPlatform)) ? Localize(TEXT("UIOptionScene"),TEXT("Text_ActiveShaderModel_Prefix"), TEXT("avaNet")) : FString(TEXT(""));
		FString ShaderModelName = FString::Printf( TEXT("<Strings:avaNet.UIOptionScene.Text_ShaderModel[%d]>"), ListIndex );
		out_FieldValue.StringValue = ShaderModelName.Len() > 0 ? ActiveLabel + ShaderModelName : FString(TEXT("UnknownModel"));
	}
	else if ( CellTag == FName(TEXT("AudioChannelValue")) )
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_AudioChannel[%d]"),ListIndex), TEXT("avaNet"));
	}
	else if ( CellTag == FName(TEXT("PostProcessValue")) )
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_PostProcess_Level3[%d]"),ListIndex), TEXT("avaNet"));
	}
	else if ( CellTag == FName(TEXT("DynamicLightModeValue")) )
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_DynamicLight_Level3[%d]"),ListIndex), TEXT("avaNet"));;
	}
	else if ( CellTag == FName(TEXT("StableFrameModeValue")) )
	{
		out_FieldValue.StringValue = Localize(TEXT("UIOptionScene"), *FString::Printf(TEXT("Text_StableFrameMode_Level3[%d]"),ListIndex), TEXT("avaNet"));;
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UavaUIDataProvider_OptionValueList::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

void UavaUIDataProvider_OptionValueList::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( FName(TEXT("OptionValueList_Level2")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("OptionValueList_Level3")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("AnisotropyModeList")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("AntialiasingModeList")), DATATYPE_Collection);
	new(out_Fields) FUIDataProviderField( FName(TEXT("LODBiasList_Level2")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("LODBiasList_Level3")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("PostProcessList_Level3")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("ShaderModelList_Level2")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("AudioChannelList")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("DynamicLightModeList")), DATATYPE_Collection );
	new(out_Fields) FUIDataProviderField( FName(TEXT("StableFrameModeList")), DATATYPE_Collection );
}

UBOOL UavaUIDataProvider_OptionValueList::SortListElements(const FName& PrimaryCellTag, const FName& SecondaryCellTag, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters )
{
	return FALSE;
}


/************************************************************************/
/* UavaUIDataProvider_GroupValueList                                    */
/************************************************************************/

void UavaUIDataProvider_GroupValueList::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("SkillGroupValue")),TEXT("SkillGroupValue"));
}

UBOOL UavaUIDataProvider_GroupValueList::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	if( CellTag == FName(TEXT("SkillGroupValue")) )
	{
		out_FieldValue.StringValue = "SkillGroupValue";

		if( 0 <= ClassIndex && ClassIndex < _CLASS_MAX )
		{
			for( INT i = 0 ; i < MAX_SKILL_PER_CLASS ; i++ )
			{
				FSkillInfo* SkillInfo = _SkillDesc().GetSkillInfo(ClassIndex, i);
				if( SkillInfo != NULL && SkillInfo->TypeID == ListIndex )
				{
					out_FieldValue.StringValue = SkillInfo->TypeName;
				}
			}
		}
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UavaUIDataProvider_GroupValueList::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/************************************************************************/
/* UavaUIDataProvider_LastResultAwardSkill                              */
/************************************************************************/

/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UavaUIDataProvider_LastResultAwardSkill::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField( FName(TEXT("AwardSkillName")) );
	new(out_Fields) FUIDataProviderField( FName(TEXT("AwardSkillDesc")) );
	new(out_Fields) FUIDataProviderField( FName(TEXT("AwardSkillIcon")) );
	new(out_Fields) FUIDataProviderField( FName(TEXT("AwardSkillEffect")) );

	new(out_Fields) FUIDataProviderField( FName(TEXT("LastResultAwardSkillList")), DATATYPE_Collection );
}

void UavaUIDataProvider_LastResultAwardSkill::GetElementCellTags( TMap<FName,FString>& OutCellTags )
{
	OutCellTags.Set(FName(TEXT("ItemIcon")),TEXT("ItemIcon"));
	OutCellTags.Set(FName(TEXT("ItemName")),TEXT("ItemName"));
}

UBOOL UavaUIDataProvider_LastResultAwardSkill::GetCellFieldValue( const FName& CellTag, INT ListIndex, FUIProviderFieldValue& out_FieldValue, INT ArrayIndex )
{
	UBOOL bUseSkillInfo = (ListIndex >= UCONST_SKILL_LISTINDEX_BASE);
	INT SkillIndex = (ListIndex - UCONST_SKILL_LISTINDEX_BASE) % UCONST_SKILL_LISTINDEX_BASE;
	INT ClassIndex = (ListIndex - UCONST_SKILL_LISTINDEX_BASE) / UCONST_SKILL_LISTINDEX_BASE;
	FAwardInfo* AwardInfo = bUseSkillInfo ? NULL : _AwardDesc().GetAwardInfo( ListIndex );
	FSkillInfo* SkillInfo = bUseSkillInfo ? _SkillDesc().GetSkillInfo( ClassIndex, SkillIndex ) : NULL;

	if ( CellTag == FName(TEXT("ItemName")) )
	{
		out_FieldValue.StringValue = AwardInfo ? AwardInfo->AwardName :
			SkillInfo ? SkillInfo->SkillName : TEXT("Inv. idx");
	}
	else if ( CellTag == FName(TEXT("ItemIcon")) )
	{
		out_FieldValue.StringValue = AwardInfo ? Localize(TEXT("AwardName"), *FString::Printf(TEXT("Icon_Award[%d]"),ListIndex) ,TEXT("avaNet")) :
			SkillInfo ? Localize(TEXT("SkillName"), *FString::Printf(TEXT("Icon_Skill[%d][%d]"),ClassIndex,SkillIndex) ,TEXT("avaNet")) : TEXT("Inv. info");
	}

	if( out_FieldValue.StringValue.Len() == 0 )
		out_FieldValue.StringValue = TEXT("N/A");

	return TRUE;
}

UBOOL UavaUIDataProvider_LastResultAwardSkill::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	FString FieldNameLower = FieldName.ToLower();
	INT ItemIndex = 1;

	Parse( *FieldNameLower, TEXT("itemindex="), ItemIndex );
	UBOOL bUseSkillIndex = ItemIndex >= UCONST_SKILL_LISTINDEX_BASE;
	ItemIndex = bUseSkillIndex ? ItemIndex - UCONST_SKILL_LISTINDEX_BASE : ItemIndex;

	FAwardInfo* AwardInfo = bUseSkillIndex ? NULL : _AwardDesc().GetAwardInfo( ItemIndex );
	FSkillInfo* SkillInfo = bUseSkillIndex ? _SkillDesc().GetSkillInfo( ItemIndex / UCONST_SKILL_LISTINDEX_BASE, ItemIndex % UCONST_SKILL_LISTINDEX_BASE ) : NULL;

	if( appStristr(*FieldNameLower, TEXT("awardskillname")) == *FieldNameLower )
	{
		OutFieldValue.StringValue = AwardInfo ? AwardInfo->AwardName : 
			SkillInfo ? SkillInfo->SkillName : TEXT("Inv. Item");
	return TRUE;
	}
	else if ( appStristr(*FieldNameLower,TEXT("awardskilleffect")) == *FieldNameLower )
	{
		OutFieldValue.StringValue = AwardInfo ? AwardInfo->RewardDesc: 
			SkillInfo ? SkillInfo->EffectDesc : TEXT("Inv. Item");
	return TRUE;
	}
	else if ( appStristr(*FieldNameLower,TEXT("awardskilldesc")) == *FieldNameLower )
	{
		OutFieldValue.StringValue = AwardInfo ? AwardInfo->Desc :
			SkillInfo ? SkillInfo->CondDesc : TEXT("Inv. Item");
	return TRUE;
	}
	else if ( appStristr(*FieldNameLower,TEXT("awardskillicon")) == * FieldNameLower )
	{
		INT ClassIndex = ItemIndex / UCONST_SKILL_LISTINDEX_BASE;
		INT SkillIndex = ItemIndex % UCONST_SKILL_LISTINDEX_BASE;

		OutFieldValue.StringValue = AwardInfo ? Localize(TEXT("AwardName"), *FString::Printf(TEXT("Icon_Award[%d]"), ItemIndex) ,TEXT("avaNet")) :
			SkillInfo ? Localize(TEXT("SkillName"), *FString::Printf(TEXT("Icon_Skill[%d][%d]"),ClassIndex,SkillIndex) ,TEXT("avaNet")) : TEXT("Inv. info");
		return TRUE;
	}
	else if ( appStristr(*FieldNameLower,TEXT("Isskillinfo")) == * FieldNameLower )
	{
		OutFieldValue.StringValue = bUseSkillIndex ? GTrue : GFalse;
		return TRUE;
	}

	return GetCellFieldValue(FName(*FieldName),INDEX_NONE,OutFieldValue,ArrayIndex);
}

/* ==========================================================================================================
UUIDataProvider_AvaNetItemDescField
========================================================================================================== */

/**
* Resolves the value of the data field specified and stores it in the output parameter.
*
* @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
*							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
* @param	OutFieldValue	receives the resolved value for the property specified.
*							@see GetDataStoreValue for additional notes
* @param	ArrayIndex		optional array index for use with data collections
*/
UBOOL UavaUIDataProvider_ChannelListField::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	if( GIsEditor && !GIsGame )
	{
		if( appStristr(*FieldName,TEXT("ChannelGroup")) == *FieldName )
		{
			INT ChannelGroupNum = INDEX_NONE;
			if( Parse(*FieldName, TEXT("GroupNum="),ChannelGroupNum) )
			{
				OutFieldValue.StringValue = Localize(TEXT("Channel"), *FString::Printf(TEXT("Text_ChannelGroup[%d]"), ChannelGroupNum), TEXT("avaNet"));
			}
		}
		else
		{
			OutFieldValue.StringValue = FieldName;
		}
	}
	else
	{
		if( appStristr(*FieldName,TEXT("ChannelGroup")) == *FieldName )
		{
			INT ChannelGroupNum = INDEX_NONE;
			if( Parse(*FieldName, TEXT("GroupNum="),ChannelGroupNum) )
			{
				OutFieldValue.StringValue = Localize(TEXT("Channel"), *FString::Printf(TEXT("Text_ChannelGroup[%d]"), ChannelGroupNum), TEXT("avaNet"));
			}
		}
	}

	if( OutFieldValue.StringValue.Len() == 0 )
	{
		OutFieldValue.StringValue = TEXT("Nothing");
	}
	return TRUE;
}


/**
* Gets the list of data fields exposed by this data provider.
*
* @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
*						Will call GetScriptDataTags to allow script-only child classes to add to this list.
*/
void UavaUIDataProvider_ChannelListField::GetSupportedDataFields(TArray<FUIDataProviderField>& out_Fields)
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("ChannelGroup")) );
}

UBOOL UavaUIDataProvider_ChannelListField::SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex/*=INDEX_NONE*/ )
{
	UBOOL bResult = FALSE;

	UBOOL BoolParam = (UBOOL)appTrunc(FieldValue.RangeValue.GetCurrentValue());

	if( appStristr(*FieldName,TEXT("ChannelGroup")) == *FieldName )
	{
		INT ChannelGroupNum = INDEX_NONE;
		if( Parse(*FieldName, TEXT("GroupNum="),ChannelGroupNum) &&
			0 <= ChannelGroupNum && ChannelGroupNum < CF_MAX )
		{
			if( BoolParam )
			{
				UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
				OptionSettings->SetLastChannelGroup(ChannelGroupNum);
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/************************************************************************/
/* UavaUIDataProvider_GameProperty										*/
/************************************************************************/
void UavaUIDataProvider_GameProperty::GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields )
{
	new(out_Fields) FUIDataProviderField(FName(TEXT("SpawnProtectionTime")));
}

UBOOL UavaUIDataProvider_GameProperty::GetFieldValue(const FString& FieldName,FUIProviderFieldValue& OutFieldValue,INT ArrayIndex)
{
	UBOOL	bIsGame = GIsEditor && !GIsGame;
	AWorldInfo* WorldInfo = GWorld->GetWorldInfo();
	AavaGameReplicationInfo* GRI = WorldInfo ? Cast<AavaGameReplicationInfo>( WorldInfo->GRI ) : NULL;
	if (FieldName == TEXT("SpawnProtectionTime"))
	{
		OutFieldValue.StringValue = ( GRI != NULL && GRI->GameClass != NULL ) ?
			FString::Printf(TEXT("%d"), INT( GRI->GameClass->GetDefaultObject<AavaGame>()->SpawnProtectionTime ) ) :
			TEXT("10");
	}
	return TRUE;
}