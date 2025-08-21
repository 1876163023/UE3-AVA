#include "PrecompiledHeader.h"
#include "avaGame.h"

IMPLEMENT_CLASS(AavaDustVolume);

/**
* Overridden to handle weapon firing, but not allow any projectiles to be blocked by
* this actor.  NOTE: bProjTarget is used as a way to indicate that this trigger should
* temporarily ignore further traces (for recursive checks through the component).
*/
#define TRACE_Dust 0x80000000
UBOOL AavaDustVolume::ShouldTrace(UPrimitiveComponent* Primitive,AActor *SourceActor, DWORD TraceFlags)
{
	if (TraceFlags & TRACE_Dust) 
		return true;

	return false;

	if (SourceActor != NULL &&
		TraceFlags & TRACE_ProjTargets &&
		SourceActor->IsA(AWeapon::StaticClass()))
	{
		return (!bProjTarget);
	}
	else
	{
		return Super::ShouldTrace(Primitive,SourceActor,TraceFlags);
	}
}