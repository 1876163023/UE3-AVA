class avaAnimBlendByWeaponHanded extends avaAnimBlendBase
	native;

cpptext
{
	virtual	void TickAnim( float DeltaSeconds, float TotalWeight  );
	virtual void PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */);
}

defaultproperties
{
	Children(0)=(Name="Left-Hand",Weight=1.0)
	Children(1)=(Name="Right-Hand")
	bFixNumChildren=true
}

