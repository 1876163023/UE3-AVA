class avaGBType_DSPPreset
	extends GenericBrowserType
	native(Editor);

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
	void InvokeCustomCommand( INT InCommand, UObject* InObject );
}
	
defaultproperties
{
	Description="AVA DSP Preset"
}

