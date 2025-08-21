/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class ParticleModuleLifetimeBase extends ParticleModule
	native(Particle)
	editinlinenew
	collapsecategories
	hidecategories(Object)
	abstract;

cpptext
{
	virtual FLOAT	GetMaxLifetime()
	{
		return 0.0f;
	}
}
