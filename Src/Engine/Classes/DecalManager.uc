/**
 * Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
 */
class DecalManager extends Object
	native(Decal);

/** List of lifetime policies owned by this decal manager. */
var const array<DecalLifetime> LifetimePolicies;

/**
 * Connects the specified decal component to the lifetime policy specified by the decal's LifetimePolicyName property.
 */
native function Connect(DecalComponent InDecalComponent);

/**
 * Disconnects the specified decal component from the lifetime policy specified by the decal's LifetimePolicyName property.
 */
native function Disconnect(DecalComponent InDecalComponent);

cpptext
{
public:
	/**
	 * Initializes the LifetimePolicies array with instances of all classes that derive from UDecalLifetime.
	 */
	void Init();

	/**
	 * Ticks all policies in the LifetimePolicies array.
	 */
	void Tick(FLOAT DeltaSeconds);
	
	/**
	 * Returns the named lifetime policy, or NULL if no lifetime policy with that name exists in LifetimePolicies.
	 */
	UDecalLifetime* GetPolicy(const FString& PolicyName) const;
}
