/**
 * UIParamDataProvider ������ ���ι��̴� ������ ���� �Ķ���� ���� ���� �����Ѵ�
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