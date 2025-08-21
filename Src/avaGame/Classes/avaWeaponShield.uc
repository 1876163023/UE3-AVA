/** special actor that only blocks weapons fire */
class avaWeaponShield extends Actor
	native
	abstract;

cpptext
{
	virtual UBOOL IgnoreBlockingBy(const AActor* Other) const;
	virtual UBOOL ShouldTrace(UPrimitiveComponent* Primitive, AActor* SourceActor, DWORD TraceFlags);
}

defaultproperties
{
	bProjTarget=true
	bCollideActors=true
}
