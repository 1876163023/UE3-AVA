class avaWeap_FixedMachineGun extends avaWeap_BaseFixedHeavyWeapon;

defaultproperties
{
	BulletType	=	class'avaBullet_556NATO'
	HitDamage	= 	50
	FireInterval(0)	=	0.08
	Penetration	=	2
	RangeModifier	=	0.95

	Kickback_WhenMoving	=	(UpBase=1.0,LateralBase=0.45,UpModifier=0.28,LateralModifier=0.045,UpMax=4.75,LateralMax=3,DirectionChange=7)
	Kickback_WhenFalling	=	(UpBase=1.2,LateralBase=0.5,UpModifier=0.23,LateralModifier=0.15,UpMax=5.5,LateralMax=3.5,DirectionChange=6)
	Kickback_WhenDucking	=	(UpBase=0.6,LateralBase=0.3,UpModifier=0.2,LateralModifier=0.0125,UpMax=4.25,LateralMax=2,DirectionChange=7)
	Kickback_WhenSteady	=	(UpBase=1.5,LateralBase=0.45,UpModifier=0.225,LateralModifier=0.05,UpMax=6.5,LateralMax=2.5,DirectionChange=7)

	Spread_WhenFalling	=	(param1=0.04,param2=0.3)
	Spread_WhenMoving	=	(param1=0.04,param2=0.12)
	Spread_WhenDucking	=	(param1=0.005,param2=0.03)
	Spread_WhenSteady	=	(param1=0.01,param2=0.06)	

	ReloadTime		= 5

	InstantHitDamageTypes(0)	=	class'avaDmgType_MachineGun'
}