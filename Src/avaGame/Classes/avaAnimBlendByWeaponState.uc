/**
 * Copyright 2006 Ntix Soft, Inc. All Rights Reserved.
 */
class avaAnimBlendByWeaponState extends avaAnimBlendBase
	Native;


struct native StateChangeSequence
{
	var() array<Name> SequenceName;
};

/** The current state this node believes the pawn to be in */
var() BYTE							ForcedWeaponState;
var() array<StateChangeSequence>	StateChangeSequenceName;
var() bool							bAutoPlayAnimWhenChangeState;

cpptext
{
	virtual	void TickAnim( float DeltaSeconds, float TotalWeight  );
	virtual void PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */);

	virtual INT GetNumSliders() const { return 1; }
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
}

simulated function Name GetStateChangeSequenceName( int PrvState, int CurState )
{
	if ( PrvState < StateChangeSequenceName.length )
	{
		if ( CurState < StateChangeSequenceName[PrvState].SequenceName.length )
			return StateChangeSequenceName[PrvState].SequenceName[CurState];
	}
	return '';
}

defaultproperties
{
	Children(0)=(Name="child0",Weight=1.0)
	bAutoPlayAnimWhenChangeState=true
}
