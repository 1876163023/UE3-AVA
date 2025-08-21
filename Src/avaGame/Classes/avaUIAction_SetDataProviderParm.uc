/**
 * UIParamDataProvider 데이터 프로바이더 각각에 대해 파라메터 값을 새로 설정한다
 */

class avaUIAction_SetDataProviderParm extends UIAction_DataStoreField
	native;

var() bool	bClearParameters;

cpptext
{
	virtual void Activated();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
}

defaultproperties
{
	bAutoActivateOutputLinks=true

	ObjName="Set DataProv Parameters"
	VariableLinks.Empty

	OutputLinks(0)=(LinkDesc="Out")
}