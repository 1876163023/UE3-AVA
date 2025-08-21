#include "avaGame.h"

IMPLEMENT_CLASS(UavaSeqAct_EnableRatingInfo)

extern void appRatingInfo_Enable( UBOOL bEnable );

void UavaSeqAct_EnableRatingInfo::Activated()
{
	appRatingInfo_Enable( bEnable );
}
