/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class Material extends MaterialInstance
	native
	hidecategories(object)
	collapsecategories;

enum EBlendMode
{
	BLEND_Opaque,
	BLEND_Masked,
	BLEND_Translucent,
	BLEND_Additive,
	BLEND_Modulate
};

enum EMaterialLightingModel
{
	MLM_Phong,
	MLM_NonDirectional,
	MLM_Unlit,
	MLM_SHPRT,
	MLM_Custom
};

// Material input structs.

struct MaterialInput
{
	var MaterialExpression	Expression;
	var int					Mask,
							MaskR,
							MaskG,
							MaskB,
							MaskA;
	var int					GCC64_Padding; // @todo 64: if the C++ didn't mismirror this structure (with ExpressionInput), we might not need this
};

struct ColorMaterialInput extends MaterialInput
{
	var bool				UseConstant;
	var color	Constant;
};

struct ScalarMaterialInput extends MaterialInput
{
	var bool				UseConstant;
	var float	Constant;
};

struct VectorMaterialInput extends MaterialInput
{
	var bool				UseConstant;
	var vector	Constant;
};

struct Vector2MaterialInput extends MaterialInput
{
	var bool				UseConstant;
	var float	ConstantX,
				ConstantY;
};

// Physics.

/** Physical material to use for this graphics material. Used for sounds, effects etc.*/
var() PhysicalMaterial		PhysMaterial;

/** For backwards compatibility only. */
var class<PhysicalMaterial>	PhysicalMaterial;

// Reflection.

var ColorMaterialInput		DiffuseColor;
var ColorMaterialInput		SpecularColor;
var ScalarMaterialInput		SpecularPower;
var VectorMaterialInput		Normal;

// Emission.

var ColorMaterialInput		EmissiveColor;

// Transmission.

var ScalarMaterialInput		Opacity;
var ScalarMaterialInput		OpacityMask;

/** If BlendMode is BLEND_Masked, the surface is not rendered where OpacityMask < OpacityMaskClipValue. */
var() float OpacityMaskClipValue;

var Vector2MaterialInput	Distortion;

var() EBlendMode BlendMode;

var() EMaterialLightingModel LightingModel;

var ColorMaterialInput		CustomLighting;

var ScalarMaterialInput		TwoSidedLightingMask;
var ColorMaterialInput		TwoSidedLightingColor;
var() bool TwoSided;

var(Usage) const bool bUsedAsLightFunction;
var(Usage) const bool bUsedAsSpecialEngineMaterial;
var(Usage) const bool bUsedWithSkeletalMesh;
var(Usage) const bool bUsedWithParticleSystem;
var(Usage) const bool bUsedWithLensFlare;


var() bool Wireframe;

var private deprecated bool	Unlit;
var private deprecated bool	NonDirectionalLighting;

/**
 * This is deprecated(moved into FMaterialResource), but the GUID is used to allow caching shaders in Gemini
 * without requiring a resave of all packages.
 */
var private deprecated Guid	PersistentIds[2];

// Resources.

var const native duplicatetransient pointer MaterialResource{FMaterialResource};
var const native duplicatetransient pointer DefaultMaterialInstances[2]{class FDefaultMaterialInstance};

var int		EditorX,
			EditorY,
			EditorPitch,
			EditorYaw;

/** Array of material expressions, excluding Comments and Compounds.  Used by the material editor. */
var array<MaterialExpression>			Expressions;

/** Array of comments associated with this material; viewed in the material editor. */
var editoronly array<MaterialExpressionComment>	EditorComments;

/** Array of material expression compounds associated with this material; viewed in the material editor. */
var editoronly array<MaterialExpressionCompound> EditorCompounds;

/** TRUE if Material uses distortion */
var private bool						bUsesDistortion;

/** TRUE if Material uses a scene color exprssion */
var private bool						bUsesSceneColor;
var private bool						bUsesEnvCube;

/** TRUE if Material is masked and uses custom opacity */
var private bool						bIsMasked;

/** Array of textures referenced, set in PreSave. */
var private const array<texture> ReferencedTextures;

var ScalarMaterialInput					AmbientMask;

cpptext
{
	// Constructor.
	UMaterial();

	/** @return TRUE if the material uses distortion */
	UBOOL HasDistortion() const;
	/** @return TRUE if the material uses the scene color texture */
	UBOOL UsesSceneColor() const;
	UBOOL UsesEnvCube() const;

	/**
	 * Allocates a material resource off the heap to be stored in MaterialResource.
	 */
	virtual FMaterialResource* AllocateResource();

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

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 *
	 * @return	Returns a array of vector parameter names used in this material.
	 */
	virtual void GetAllVectorParameterNames(TArray<FName> &OutParameterNames);

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 *
	 * @return	Returns a array of scalar parameter names used in this material.
	 */
	virtual void GetAllScalarParameterNames(TArray<FName> &OutParameterNames);

	/**
	 * @param	OutParameterNames		Storage array for the parameter names we are returning.
	 *
	 * @return	Returns a array of texture parameter names used in this material.
	 */
	virtual void GetAllTextureParameterNames(TArray<FName> &OutParameterNames);

	// UMaterialInstance interface.
	virtual UMaterial* GetMaterial();
    virtual UBOOL GetVectorParameterValue(FName ParameterName,FLinearColor& OutValue);
    virtual UBOOL GetScalarParameterValue(FName ParameterName,FLOAT& OutValue);
    virtual UBOOL GetTextureParameterValue(FName ParameterName,class UTexture*& OutValue);
	virtual FMaterialInstance* GetInstanceInterface(UBOOL Selected) const;
	virtual UPhysicalMaterial* GetPhysicalMaterial();

	// UObject interface.
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();
	virtual void PreEditChange(UProperty* PropertyAboutToChange);
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	virtual void FinishDestroy();
	/**
	 * Called before serialization on save to propagate referenced textures. This is not done
	 * during content cooking as the material expressions used to retrieve this information will
	 * already have been dissociated via RemoveExpressions
	 */
	void PreSave();

	/**
	 * @return		Sum of the size of textures referenced by this material.
	 */
	virtual INT GetResourceSize();

	/**
	 * Used by various commandlets to purge Editor only data from the object.
	 * 
	 * @param TargetPlatform Platform the object will be saved for (ie PC vs console cooking, etc)
	 */
	virtual void StripData(UE3::EPlatformType TargetPlatform);

	/**
	* Null any material expression references for this material
	*/
	void RemoveExpressions();

private:

	/**
	 * Releases the resources for the material.
	 */
	void ReleaseResources();

	/**
	 * Creates new resources for the material, and updates any cached references to the resource.
	 */
	void UpdateResource();
}


/** returns the Referneced Textures so one may set flats on them  (e.g. bForceMiplevelsToBeResident ) **/
function array<texture> GetTextures()
{
	return ReferencedTextures;
}


defaultproperties
{
	BlendMode=BLEND_Opaque
	Unlit=False
	DiffuseColor=(Constant=(R=128,G=128,B=128))
	SpecularColor=(Constant=(R=128,G=128,B=128))
	SpecularPower=(Constant=15.0)
	Distortion=(ConstantX=0,ConstantY=0)
	Opacity=(Constant=1)
	OpacityMask=(Constant=1)
	AmbientMask=(Constant=1)
	OpacityMaskClipValue=0.5
	TwoSidedLightingColor=(Constant=(R=255,G=255,B=255))
}
