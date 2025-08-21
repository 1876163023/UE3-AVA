class avaSeqAct_HurtRadius extends SequenceAction;

/** Type of damage to apply */
var() class<DamageType>		DamageType;

/** Amount of momentum to apply */
var() float					Momentum;

/** Amount of damage to apply */
var() float					DamageAmount;

/** Distance to Instigator within which to damage actors */
var()	float				DamageRadius;

/** Whether damage should decay linearly based on distance from the instigator. */
var()	bool				bFullDamage;

/** player that should take credit for the damage (Controller or Pawn) */
var Actor					Instigator;

var Actor					InstigateBy;

event Activated()
{
	local Actor			Victim/*, HitActor*/;
	local Controller	C;
//	local Vector		HitLocation, HitNormal;

	C = Controller( InstigateBy );
	if ( C == None && Pawn( InstigateBy ) != None )
		C = Pawn( InstigateBy ).Controller;
	
	foreach Instigator.CollidingActors( class'Actor', Victim, DamageRadius, Instigator.Location )
	{
		// Victim 이 Pawn 인 경우에 장애물에 가려지 있는지 Check 해본다....
		//if ( avaPawn( Victim ) != None )
		//{
		//	HitActor = Victim.Trace( HitLocation, HitNormal, Instigator.Location, Victim.Location, true, , , 1 );
		//	`log( "avaSeqAct_HurtRadius" @HitActor );
		//	if ( HitActor != None && HitActor != Instigator  )
		//		continue;
		//}
		Victim.TakeRadiusDamage( C, DamageAmount, DamageRadius, DamageType, Momentum, Instigator.Location, bFullDamage );
	}
}

defaultproperties
{
	ObjName="Hurt Radius"
	ObjCategory="Actor"
	ObjClassVersion=2

	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="Amount",PropertyName=DamageAmount)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Instigator",PropertyName=Instigator)
	VariableLinks(3)=(ExpectedType=class'SeqVar_Object',LinkDesc="InstigateBy",PropertyName=InstigateBy)
	Momentum=500.f
	DamageRadius=200.f
}