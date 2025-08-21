/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialInstance extends Surface
	abstract
	native;

var(AVA_Radiosity) float					ReflectivityScale;
var(AVA_Radiosity) color					ReflectivityColor;
var(AVA_Radiosity) bool						bOverrideReflectivityColor;
var(AVA_Radiosity) float					BrightnessScale;
var(AVA_Radiosity) color					BrightnessColor;
var(AVA_Radiosity) bool						bOverrideBrightnessColor;

cpptext
{
	// GetMaterial - Get the material which this is an instance of.
	virtual class UMaterial* GetMaterial() PURE_VIRTUAL(UMaterialInstance::GetMaterial,return NULL;);

	/**
	 * Tests this material instance for dependency on a given material instance.
	 * @param	TestDependency - The material instance to test this instance for dependency upon.
	 * @return	True if the material instance is dependent on TestDependency.
	 */
	virtual UBOOL IsDependent(UMaterialInstance* TestDependency) { return FALSE; }

	/**
	 * Returns a pointer to the FMaterialInstance used for rendering.
	 *
	 * @param	Selected	specify TRUE to return an alternate material used for rendering this material when part of a selection
	 *						@note: only valid in the editor!
	 *
	 * @return	The resource to use for rendering this material instance.
	 */
	virtual const FMaterialInstance* GetInstanceInterface(UBOOL Selected) const PURE_VIRTUAL(UMaterialInstance::GetInstanceInterface,return NULL;);

	/**
	 * Returns a pointer to the physical material used by this material instance.
	 * @return The physical material.
	 */
	virtual UPhysicalMaterial* GetPhysicalMaterial() PURE_VIRTUAL(UMaterialInstance::GetPhysicalMaterial,return NULL;);

	/**
	 * Gathers the textures used to render the material instance.
	 * @param OutTextures	Upon return contains the textures used to render the material instance.
	 * @param bOnlyAddOnce	Whether to add textures that are sampled multiple times uniquely or not
	 */
	void GetTextures(TArray<UTexture*>& OutTextures,UBOOL bOnlyAddOnce = TRUE);

	/**
	 * 주의 : 이는 GRHIShaderPlatform, GShaderCompilePlatform을 이용해서 처리한다. 
			RenderThread는 멈춰있어야 한다!
	 */
	void GetTexturesNeededForPlatform(TArray<UTexture*>& OutTextures,UBOOL bOnlyAddOnce = TRUE);

	/**
	 * Checks if the material has the appropriate flag set to be used with skeletal meshes.
	 * If it's the editor, and it isn't set, it sets it.
	 * If it's the game, and it isn't set, it will return False.
	 * @return True if the material may be used with the skeletal mesh, False if the default material should be used.
	 */
	UBOOL UseWithSkeletalMesh();
	
	/**
	 * Checks if the material has the appropriate flag set to be used with particle systems.
	 * If it's the editor, and it isn't set, it sets it.
	 * If it's the game, and it isn't set, it will return False.
	 * @return True if the material may be used with the particle system, False if the default material should be used.
	 */
	UBOOL UseWithParticleSystem();

	INT GetWidth() const;
	INT GetHeight() const;

	// USurface interface
	virtual FLOAT GetSurfaceWidth() const { return GetWidth(); }
	virtual FLOAT GetSurfaceHeight() const { return GetHeight(); }
}

/** The mesh used by the material editor to preview the material.*/
var() editoronly string PreviewMesh;

native final noexport function Material GetMaterial();

/**
 * Returns a pointer to the physical material used by this material instance.
 * @return The physical material.
 */
native final noexport function PhysicalMaterial GetPhysicalMaterial();

// Get*ParameterValue - Gets the entry from the ParameterValues for the named parameter.
// Returns false is parameter is not found.

native function bool GetVectorParameterValue(name ParameterName, out LinearColor OutValue);
native function bool GetScalarParameterValue(name ParameterName, out float OutValue);
native function bool GetTextureParameterValue(name ParameterName, out Texture OutValue);

defaultproperties
{
	ReflectivityColor=(R=255,G=255,B=255)
	BrightnessColor=(R=255,G=255,B=255)
	ReflectivityScale=0.3
	BrightnessScale=1000
}