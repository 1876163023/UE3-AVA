class avaUIUseInfo extends avaUIInfoMessage native
	hidecategories(Icon);

var transient avaUseVolume UseVolume;

cpptext
{
	UBOOL UpdateUseVolume();

	virtual INT UpdateInfo();	
}

defaultproperties
{
	Editor_String = "test use info message"
}