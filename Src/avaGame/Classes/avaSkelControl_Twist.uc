/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class avaSkelControl_Twist extends SkelControlBase
	native;


struct native TwistBoneData
{
	var()	name						BoneName;				// Name of the bone to twist from
	var  	int							BoneIndex;				// Internal Use for quick compare
	var  	int							BoneYawOffset;			// The current offset in WorldSpace
	var  	int							BoneAdj;				// The Current adjustment in bone space
	var()  	int							BoneYawTolerance;		// How much difference should be allowed
	var		bool						bInMotion;				// Is this segment auto-centering
};

struct native LeanBoneData
{
	var()	name	BoneName;
	var()	int		BonePitchLowerTolerance;
	var() 	int		BonePitchUpperTolerance;
	var  	int		BoneIndex;
};

var	(Twist) array<TwistBoneData>	TwistData;
var	(Twist) array<LeanBoneData>		LeanData;

/** Hold the Yaw Info */

var transient	int HeadYaw, LastHeadYaw;

var	transient float LastZeroed;

/** Natively set to true when this control has been initialized */
var transient bool	bInitialized;
var transient bool	bDormant;

/** The Twist Control can be forced to look at either a vector or an object */

var bool    bForcedLookAt;
var bool	bRecentering;
var vector  ForceFocalPoint;
var actor	ForceFocalActor;

var string  debugstr;

var() float RecenteringTime;

cpptext
{
	virtual void TickSkelControl(FLOAT DeltaSeconds, USkeletalMeshComponent* SkelComp);
	virtual void CalculateNewBoneTransforms(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<FMatrix>& OutBoneTransforms);
	virtual void GetAffectedBones(INT BoneIndex, USkeletalMeshComponent* SkelComp, TArray<INT>& OutBoneIndices);
}

delegate OwnerNotification(name BoneName, int Offset);

function Reset(int NewYaw)
{
	local int i;
	for (i=0;i<TwistData.Length;i++)
	{
		TwistData[i].BoneYawOffset = NewYaw;
		TwistData[i].bInMotion = false;
	}
}

function ResetAbove(name BoneName, int NewYaw)
{
	local int i,j;

	for (i=0;i<TwistData.Length;i++)
	{
		if (TwistData[i].BoneName == BoneName)
		{
			j = i;
			for (j=i;j>=0;j--)
			{
				TwistData[j].BoneYawOffset = NewYaw;
				TwistData[j].bInMotion = false;
			}
			return;
		}
	}

	`log(Self@"Could not find an Entry for"@BoneName);
}

simulated function int Fix(int num)
{
	num = num & 65535;
	if (num>32767)
		num-=65535;

	return num;
}

simulated function TwistDisplayDebug(HUD HUD, out float out_YL, out float out_YPos, avaCharacter PawnOwner)
{
	local int i;
	local int x,y;


	Hud.Canvas.SetDrawColor(255,2255,200);
	Hud.Canvas.SetPos(4,out_YPos);
	Hud.Canvas.DrawText("-- [Twist] HeadYaw = "@Fix(HeadYaw)@"ViewYaw:"@PawnOwner.LookYaw);
	out_YPos += out_YL;

	for (i=0;i<TwistData.Length;i++)
	{
		Hud.Canvas.SetPos(4,out_YPos);
		
		if (i>0)
		{
			x = (TwistData[i-1].BoneYawOffset - TwistData[i].BoneYawOffset);
			y = x & 65535;
		}
		else
		{
		  	x = 0;
		  	y = 0;
		}
		
		Hud.Canvas.DrawText("  ["$TwistData[i].BoneName$" ("$TwistData[i].BoneIndex$") ] Yaw:"@TwistData[i].BoneYawOffset@"("$Fix(TwistData[i].BoneYawOffset)$")  Adj:"@Fix(TwistData[i].BoneAdj)@"  Tol:"@Fix(TwistData[i].BoneYawTolerance)@"  ["$TwistData[i].bInMotion$"]"@x@y);
		out_YPos += out_YL;
	}

	if (bForcedLookAt)
	{
		if (ForceFocalActor!=none)
		{
			Hud.Canvas.SetPos(4,out_YPos);
			Hud.Canvas.DrawText("  [FORCED] "@ForceFocalActor@":"@Rotator(ForceFocalActor.Location - PawnOwner.Location).Yaw);
		}
		else
		{
			Hud.Canvas.SetPos(4,out_YPos);
			Hud.Canvas.DrawText("  [FORCED] "@ForceFocalPoint@":"@Rotator(PawnOwner.Location - ForceFocalPoint).Yaw);
		}

	}

}

simulated function ForceLookAt(optional actor FocalActor, optional vector FocalPoint)
{
	bForcedLookAt = true;
	ForceFocalPoint = FocalPoint;
	ForceFocalActor = FocalActor;
}

simulated function UnlockLook()
{
	bRecentering = true;
}


defaultproperties
{
//	TwistData(0)=(BoneName=Neck,BoneYawTolerance=0)
	//TwistData(0)=(BoneName=Spine2,BoneYawTolerance=0)
	//TwistData(1)=(BoneName=Spine1,BoneYawTolerance=3072)
	//TwistData(2)=(BoneName=Spine,BoneYawTolerance=2048)
	//TwistData(3)=(BoneName=Hips,BoneYawTolerance=1024)

	//TwistData(0)=(BoneName=Bip01_Neck,BoneYawTolerance=0)
	//TwistData(1)=(BoneName=Bip01_Spine3,BoneYawTolerance=3072)
	//TwistData(2)=(BoneName=Bip01_Spine2,BoneYawTolerance=2048)
	//////TwistData(3)=(BoneName=Bip01_Spine1,BoneYawTolerance=1536)
	//TwistData(3)=(BoneName=Bip01_Pelvis,BoneYawTolerance=1024)

	TwistData(0)=(BoneName=Bip01_Spine3,BoneYawTolerance=3072)
	TwistData(1)=(BoneName=Bip01_Spine2,BoneYawTolerance=2048)
	////TwistData(3)=(BoneName=Bip01_Spine1,BoneYawTolerance=1536)
	TwistData(2)=(BoneName=Bip01_Pelvis,BoneYawTolerance=1024)

	LeanData(0)=(BoneName=Neck,BonePitchLowerTolerance=-6144,BonePitchUpperTolerance=6144)
	LeanData(1)=(BoneName=Spine2,BonePitchLowerTolerance=-4096,BonePitchUpperTolerance=2048)
	LeanData(2)=(BoneName=LeftArm,BonePitchLowerTolerance=-2048,BonePitchUpperTolerance=6144)
	LeanData(3)=(BoneName=RightArm,BonePitchLowerTolerance=-2048,BonePitchUpperTolerance=6144)
	LeanData(4)=(BoneName=LeftClav,BonePitchLowerTolerance=-2048,BonePitchUpperTolerance=6144)
	LeanData(5)=(BoneName=RightClav,BonePitchLowerTolerance=-2048,BonePitchUpperTolerance=6144)

    bDormant=false
    bForcedLookAt=false

	RecenteringTime=1
}
