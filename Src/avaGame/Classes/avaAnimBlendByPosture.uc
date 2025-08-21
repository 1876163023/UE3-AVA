/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAnimBlendByPosture extends avaAnimBlendBase
	native;                                         

cpptext
{
	virtual	void TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight  );
	virtual void PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */);
}


defaultproperties
{
	Children(0)=(Name="Stand",Weight=1.0)
	Children(1)=(Name="Crouch")
	Children(2)=(Name="Dash")
	Children(3)=(Name="Crouch-Dash")
	bFixNumChildren=true
}
