class avaAnimBlendByLadder extends avaAnimBlendBase
	native;

cpptext
{
	virtual	void TickAnim( float DeltaSeconds, float TotalWeight  );
}

defaultproperties
{
	Children(0)=(Name="Idle",Weight=1.0)
	Children(1)=(Name="Upward - Look Ladder")
	Children(2)=(Name="Downward - Look Ladder")
	Children(3)=(Name="Upward - Look Other")
	Children(4)=(Name="Downward - Look Other")
	bFixNumChildren=true
}
