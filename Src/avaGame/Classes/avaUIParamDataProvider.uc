/**
 * Base class for all classes which provide data stores with data about specific instances of a particular data type.
 *
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved
 */
class avaUIParamDataProvider extends UIDataProvider
	native(UIPrivate)
	transient
	abstract;

cpptext
{
public:
	/** 
	 * 설정 가능한 파라메터 이름을 얻어온다
	 */
	void GetSupportedParameterNames( TArray<FName>& OutPropertyNames );

	/** 
	 * 설정 가능한 프로퍼티들을 얻어온다
	 */
	void GetSupportedParameterProps( TArray<UProperty*>& OutProperties );
	
	/**
	 * 파라메터들을 초기화한다
	 *
	 * @param	bUseDefault		0이나 ""로 초기화하는대신 DefaultObject의 값을 얻어와 쓴다.
	 */
	void ClearParameters( UBOOL bUseDefault = TRUE );

	/** 
	 * FieldName에 지정된 파라메터를 읽어들임
	 *
	 * @return 업데이트로 파라메터 값이 변했는지의 여부
	 */
	UBOOL UpdateParameters( FString& FieldStr, UBOOL bCutOffParamStr = TRUE );

	/**
	 * 파라메터들을 모두 업데이트 한후에 필요한 처리가 있다면 추가
	 * 예) 무기 인벤토리의 리스트 인덱스를 파라메터로 받았음 -(PostUpdateParamters)-> ItemID를 계산한다
	 */
	virtual void PostUpdateParamters( UBOOL bParmChanged ) { }

protected:

	typedef TMap<FName,FString> NameValueMapType;

	/** 
	 * 읽어들여온 FieldName에서 파라메터부분을 추출
	 *
	 */
	void ParseParameter( FString& FieldName, FString& ParameterStr, NameValueMapType& NameValueMap );

	/**
	 * Resolves the value of the data field specified and stores it in the output parameter.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see ParseDataStoreReference for additional notes
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE )
	{
		FString FieldStr = FieldName;
		UpdateParameters( FieldStr );
		return GetField( FieldStr, out_FieldValue, ArrayIndex );
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
	virtual UBOOL GetField( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE ) { PURE_VIRTUAL(UavaUIParamDataProvider::GetField,return FALSE;) };
}

var const editconst string FieldDelimiter;

DefaultProperties
{
	WriteAccessType=ACCESS_ReadOnly
	FieldDelimiter = "?"
}
