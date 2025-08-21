#include "avaGame.h"

IMPLEMENT_CLASS(UavaHudObject);
IMPLEMENT_CLASS(UavaHudPercBar);


/*=========================================================================================
	avaHudObject - All of our huds will be a child of this.
========================================================================================= */

/**
* @Returns the HUD associated with this object
*/
AHUD* UavaHudObject::GetHudOwner()
{
	AavaPlayerController* PlayerOwner = GetAvaPlayerOwner();
	if (PlayerOwner)
	{
		return PlayerOwner->myHUD;
	}
	return NULL;
}

void UavaHudPercBar::PostEditChange( UProperty* PropertyThatChanged )
{
	PerBarFill->Position.SetPositionValue(this, 0, UIFACE_Left,		EVALPOS_PercentageOwner);
	PerBarFill->Position.SetPositionValue(this, 0, UIFACE_Top,		EVALPOS_PercentageOwner);
	PerBarFill->Position.SetPositionValue(this, 1, UIFACE_Right,	EVALPOS_PercentageOwner);
	PerBarFill->Position.SetPositionValue(this, 1, UIFACE_Bottom,	EVALPOS_PercentageOwner);
	Super::PostEditChange(PropertyThatChanged);
}