/**
 * MaterialEditorInstance.uc: This class is used by the material instance editor to hold a set of inherited parameters which are then pushed to a material instance.
 * Copyright ?2006 Epic Games, Inc. All Rights Reserved.
 */
class MaterialEditorInstance extends Object
	native
	hidecategories(Object);
	
	// ava needs category ; 2007. 1. 17 changmin
	//collapsecategories;

struct native EditorVectorParameterValue
{
	var() bool			bOverride;
	var() name			ParameterName;
	var() LinearColor	ParameterValue;
};

struct native EditorScalarParameterValue
{
	var() bool		bOverride;
	var() name		ParameterName;
	var() float		ParameterValue;
};

struct native EditorTextureParameterValue
{
	var() bool		bOverride;
    var() name		ParameterName;
    var() Texture	ParameterValue;
};

/** Physical material to use for this graphics material. Used for sounds, effects etc.*/
var() PhysicalMaterial						PhysMaterial;

var() MaterialInstance						Parent;
var() array<EditorVectorParameterValue>		VectorParameterValues;
var() array<EditorScalarParameterValue>		ScalarParameterValues;
var() array<EditorTextureParameterValue>	TextureParameterValues;
var	  MaterialInstanceConstant				SourceInstance;

var(AVA_Radiosity) float					ReflectivityScale;
var(AVA_Radiosity) color					ReflectivityColor;
var(AVA_Radiosity) bool						bOverrideReflectivityColor;
var(AVA_Radiosity) float					BrightnessScale;
var(AVA_Radiosity) color					BrightnessColor;
var(AVA_Radiosity) bool						bOverrideBrightnessColor;

cpptext
{
	// UObject interface.
	virtual void PostEditChange(UProperty* PropertyThatChanged);

	/** Regenerates the parameter arrays. */
	void RegenerateArrays();

	/** Copies the parameter array values back to the source instance. */
	void CopyToSourceInstance();

	/** 
	 * Sets the source instance for this object and regenerates arrays. 
	 *
	 * @param MaterialInstance		Instance to use as the source for this material editor instance.
	 */
	void SetSourceInstance(UMaterialInstanceConstant* MaterialInstance);
}
