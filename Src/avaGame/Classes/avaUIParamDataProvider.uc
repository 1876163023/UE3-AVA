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
	 * ���� ������ �Ķ���� �̸��� ���´�
	 */
	void GetSupportedParameterNames( TArray<FName>& OutPropertyNames );

	/** 
	 * ���� ������ ������Ƽ���� ���´�
	 */
	void GetSupportedParameterProps( TArray<UProperty*>& OutProperties );
	
	/**
	 * �Ķ���͵��� �ʱ�ȭ�Ѵ�
	 *
	 * @param	bUseDefault		0�̳� ""�� �ʱ�ȭ�ϴ´�� DefaultObject�� ���� ���� ����.
	 */
	void ClearParameters( UBOOL bUseDefault = TRUE );

	/** 
	 * FieldName�� ������ �Ķ���͸� �о����
	 *
	 * @return ������Ʈ�� �Ķ���� ���� ���ߴ����� ����
	 */
	UBOOL UpdateParameters( FString& FieldStr, UBOOL bCutOffParamStr = TRUE );

	/**
	 * �Ķ���͵��� ��� ������Ʈ ���Ŀ� �ʿ��� ó���� �ִٸ� �߰�
	 * ��) ���� �κ��丮�� ����Ʈ �ε����� �Ķ���ͷ� �޾��� -(PostUpdateParamters)-> ItemID�� ����Ѵ�
	 */
	virtual void PostUpdateParamters( UBOOL bParmChanged ) { }

protected:

	typedef TMap<FName,FString> NameValueMapType;

	/** 
	 * �о�鿩�� FieldName���� �Ķ���ͺκ��� ����
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
