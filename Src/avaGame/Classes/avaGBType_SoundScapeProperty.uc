class avaGBType_SoundScapeProperty
	extends GenericBrowserType
	native(Editor);

cpptext
{
	virtual void Init();
	virtual UBOOL ShowObjectEditor( UObject* InObject );
}
	
defaultproperties
{
	Description="AVA Sound Scape"
}

