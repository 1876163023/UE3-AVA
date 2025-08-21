class avaAnimBlendBySpeed extends avaAnimBlendBase
	native;

cpptext
{
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
	virtual void PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */);
}

defaultproperties
{
	Children(0)=(Name="Idle",Weight=1.0)
	Children(1)=(Name="Walk")
	Children(2)=(Name="Run")
	bFixNumChildren=true
}
