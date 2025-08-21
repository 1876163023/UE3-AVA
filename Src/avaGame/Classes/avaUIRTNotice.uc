class avaUIRTNotice extends avaUISimpletext
	native;

var() float				DisplayDuration;
var transient float		LatestUpdateTime;
var() Name				TextStyleName<ToolTip=Ref. DefaultGame.ini>;

cpptext
{
	virtual UBOOL UpdateString();
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner );
}

defaultproperties
{
	DisplayDuration=3.0
	LatestUpdateTime=-1.0
}