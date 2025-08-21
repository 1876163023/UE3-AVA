class avaAnimBlendBySpeedSimple extends AnimNodeBlend
	native;

cpptext
{
	virtual	void TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight);
}

defaultproperties
{
	Children(0)=(Name="Walk",Weight=1.0)
	Children(1)=(Name="Run")
}
