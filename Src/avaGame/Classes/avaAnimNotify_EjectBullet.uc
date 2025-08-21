//=============================================================================
//  avaAnimNotify_EjectBullet
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
// 
//	2006/03/30 by OZ
//		1. Animation ���� ���� Notify �� �޾� ź�Ǹ� ������Ų��.
//=============================================================================
class avaAnimNotify_EjectBullet extends AnimNotify
	native;

cpptext
{
	// AnimNotify interface.
	virtual void Notify( class UAnimNodeSequence* NodeSeq );
}

simulated event EjectBullet( Actor Owner )
{
	local Actor o;

	o = Owner;

	if (avaWeapon(o) != none)
		o = avaWeapon(o).Instigator;

	if (avaPawn(o) != none )
	{
		avaPawn(o).PlayEffect_EjectBullet();
	}
}