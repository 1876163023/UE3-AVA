#include "avaGame.h"

void AavaShatterGlassActor::CheckForErrors()
{
	Super::CheckForErrors();
	if( ShatterGlassComponent == NULL )
	{
		GWarn->MapCheck_Add( MCTYPE_WARNING, this, *FString::Printf(TEXT("%s : Shatter glass actor has NULL ShatterGlassComponent property - please delete!"), *GetName() ) );
	}	
}