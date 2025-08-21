/**
 * Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class MaterialInstanceConstant extends MaterialInstance
	native(Material)
	hidecategories(Object);
	
	// ava needs category ; 2007. 1. 17 changmin
	//collapsecategories;

struct native VectorParameterValue
{
	var() name			ParameterName;
	var() LinearColor	ParameterValue;
};

struct native ScalarParameterValue
{
	var() name	ParameterName;
	var() float	ParameterValue;
};

struct native TextureParameterValue
{
    var() name		ParameterName;
    var() Texture	ParameterValue;
	var int			PlatformDependency; 
};

/** Physical material to use for this graphics material. Used for sounds, effects etc.*/
var() PhysicalMaterial PhysMaterial;

var() const MaterialInstance				Parent;
var() const array<VectorParameterValue>		VectorParameterValues;
var() const array<ScalarParameterValue>		ScalarParameterValues;
var() const array<TextureParameterValue>	TextureParameterValues;
var() native const array<TextureParameterValue>	NewTextureParameterValues;

var const native duplicatetransient pointer Resources[2]{class FMaterialInstanceResource};
var private const native bool ReentrantFlag;
var private const native bool bPlatformDependencyDirty;

cpptext
{
	// Constructor.
	UMaterialInstanceConstant();

	// UMaterialInstance interface.
	virtual UMaterial* GetMaterial();
    virtual UBOOL GetVectorParameterValue(FName ParameterName,FLinearColor& OutValue);
    virtual UBOOL GetScalarParameterValue(FName ParameterName,FLOAT& OutValue);
    virtual UBOOL GetTextureParameterValue(FName ParameterName,class UTexture*& OutValue);
	virtual UBOOL IsDependent(UMaterialInstance* TestDependency);
	virtual FMaterialInstance* GetInstanceInterface(UBOOL Selected) const;
	virtual UPhysicalMaterial* GetPhysicalMaterial();

	// UObject interface.
	virtual void PostLoad();
	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void FinishDestroy();
	virtual void Serialize(FArchive& Ar);

	void DeterminePlatformTextureDependencies();
}

// SetParent - Updates the parent.

native final function SetParent(MaterialInstance NewParent);

// Set*ParameterValue - Updates the entry in ParameterValues for the named parameter, or adds a new entry.

native final function SetVectorParameterValue(name ParameterName, LinearColor Value);
native final function SetScalarParameterValue(name ParameterName, float Value);
native final function SetTextureParameterValue(name ParameterName, Texture Value);

/** Removes all parameter values */
native final function ClearParameterValues();
