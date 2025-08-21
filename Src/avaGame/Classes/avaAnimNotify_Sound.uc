/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaAnimNotify_Sound extends AnimNotify
	native;

var()	SoundCue	SoundCue;
var()	Name		BoneName;
var()	bool		bNotReplicated;
var()	bool		bNoFirstPerson;

cpptext
{
	// AnimNotify interface.
	virtual void Notify( class UAnimNodeSequence* NodeSeq );
}

simulated event PlaySound2( Actor Owner, vector Location )
{
	local Actor o;

	o = Owner;

	if (avaWeapon(o) != none)
		o = avaWeapon(o).Instigator;

	if ( bNoFirstPerson == true && avaPawn(o) != None && avaPawn(o).IsFirstPerson() )
		return;

	o.PlaySound( SoundCue, bNotReplicated, true );
}

defaultproperties
{
	bNotReplicated	=	false
	bNoFirstPerson	=	false
}

