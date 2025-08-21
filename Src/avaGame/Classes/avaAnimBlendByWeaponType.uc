/**
 * Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
 */
class avaAnimBlendByWeaponType extends avaAnimBlendBase
	Native;

enum EBlendWeaponType
{
	WBT_Knife,		// Knife
	WBT_C4,			// C4
	WBT_Grenade,	// 수류탄
	WBT_PISTOL01,	// Pistol류
	WBT_SMG01,		// SMG류
	WBT_Rifle01,	// 라이플류
	WBT_RPG,		// RPG
	WBT_None
};


/** The current state this node believes the pawn to be in */
var() EBlendWeaponType	ForcedWeaponType;
var() array<Name>		PrvPrefix;

native final function InitAnimSequence( name NewPrefix );

cpptext
{
	virtual	void TickAnim( float DeltaSeconds, float TotalWeight  );

	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
}

defaultproperties
{
	ForcedWeaponType=WBT_None

	Children(0)=(Name="Knife",Weight=1.0)
	Children(1)=(Name="C4")
	Children(2)=(Name="Grenade")
	Children(3)=(Name="Pistol01")
	Children(4)=(Name="SMG01")
	Children(5)=(Name="Rifle01")
	Children(6)=(Name="RPG")

	PrvPrefix(0)=Knife
	PrvPrefix(1)=C4
	PrvPrefix(2)=G
	PrvPrefix(3)=P226
	PrvPrefix(4)=MP5
	PrvPrefix(5)=G36
	PrvPrefix(6)=RPG7

	bFixNumChildren=true
}
