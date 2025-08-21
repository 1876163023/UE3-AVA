/**
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class LensFlareComponent extends PrimitiveComponent
	native(LensFlare)
	hidecategories(Object)
	hidecategories(Physics)
	hidecategories(Collision)
	editinlinenew
	dependson(LensFlare);

var()	const						LensFlare			Template;

struct LensFlareElementInstance
{
	// No UObject reference
};

/** If TRUE, automatically enable this flare when it is attached */
var()								bool				bAutoActivate;

/** Internal variables */
var transient						bool				bIsActive;
var	transient						bool				bHasTranslucency;
var	transient						bool				bHasUnlitTranslucency;
var	transient						bool				bHasUnlitDistortion;
var	transient						bool				bUsesSceneColor;

/** Command fence used to shut down properly */
var		native				const	pointer			ReleaseResourcesFence{class FRenderCommandFence};

native final function SetTemplate(LensFlare NewTemplate);

cpptext
{
	// UObject interface
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	virtual void FinishDestroy();

protected:
	// UActorComponent interface.

public:
	// UPrimitiveComponent interface
	virtual void UpdateBounds();
	
	/** Returns true if the prim is using a material with unlit distortion */
	virtual UBOOL HasUnlitDistortion() const;
	/** Returns true if the prim is using a material with unlit translucency */
	virtual UBOOL HasUnlitTranslucency() const;

	/**
	* Returns true if the prim is using a material that samples the scene color texture.
	* If true then these primitives are drawn after all other translucency
	*/
	virtual UBOOL UsesSceneColor() const;

	virtual FPrimitiveSceneProxy* CreateSceneProxy();

	// InstanceParameters interface
	void	AutoPopulateInstanceProperties();
}

defaultproperties
{
	bAutoActivate=true
	bTickInEditor=true
	TickGroup=TG_PostAsyncWork
}
