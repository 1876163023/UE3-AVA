class avaDecalLifetimeRound extends DecalLifetime
	config(game)
	native;

var globalconfig int	MaxDecal;

cpptext
{
public:
	/**
	 * Called by UDecalManager::Tick.
	 */
	virtual void Tick(FLOAT DeltaSeconds);
	virtual void AddDecal(UDecalComponent* InDecalComponent);
}

defaultproperties
{
	PolicyName	="Round"
}
