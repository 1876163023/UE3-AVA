/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAnimBlendByIdle extends avaAnimBlendBase
	native;

var transient avaSkelControl_Twist	TwistControl;

var() name	TwistControlName;
var() name	TurnAnimNames[2];	

var array< AnimNodeSynch > SyncNodes;

cpptext
{
	// AnimNode interface
	virtual void InitAnim( USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent );
	virtual	void TickAnim( float DeltaSeconds, FLOAT TotalWeight  );
	virtual void PlayAnim( UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */);
}

event OnInit()
{
	Super.OnInit();
	TwistControl = avaSkelControl_Twist( SkelComponent.FindSkelControl(TwistControlName) );
}


defaultproperties
{
	Children(0)=(Name="Idle",Weight=1.0)
	Children(1)=(Name="TurnInPlace")
	Children(2)=(Name="Moving")
	bFixNumChildren=true
}
