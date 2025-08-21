class avaInterpActor extends InterpActor;

var() int Team;

event TakeDamage(int DamageAmount, Controller EventInstigator, vector HitLocation, vector Momentum, class<DamageType> DamageType, optional TraceHitInfo HitInfo, optional class<Weapon> DamageCauser)
{
	if ( EventInstigator.GetTeamNum() != Team )
	{
		Super.TakeDamage( DamageAmount, 
						  EventInstigator, 
						  HitLocation, 
						  Momentum, 
						  DamageType, 
						  HitInfo, 
						  DamageCauser );
	}
}

defaultproperties
{
	Team = 255
} 