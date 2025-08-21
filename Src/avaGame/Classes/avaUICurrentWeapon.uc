class avaUICurrentWeapon extends avaUISimpleText
	native;

var transient			avaWeapon		LatestWeapon;
var(Test)				string			TestString;

cpptext
{
	virtual UBOOL UpdateString();
}