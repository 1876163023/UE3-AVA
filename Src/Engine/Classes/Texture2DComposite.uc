/**
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class Texture2DComposite extends Texture2D
	native
	hidecategories(Object);

/**
 * Defines a source texture and UV region in that texture
 */
struct native SourceTexture2DRegion
{
	var int OffsetX;
	var int OffsetY;
	var int SizeX;
	var int SizeY;
	var Texture2D Texture2D;
};

/** list of source textures and UV regions for compositing */
var array<SourceTexture2DRegion> SourceRegions;
/** list of valid source textures and UV regions for compositing */
var private transient array<SourceTexture2DRegion> ValidRegions;

/** Additional LOD bias applied to creation of this composite texture. */
var int	CompositeLODBias;

/** Utility that checks to see if all Texture2Ds specified in the SourceRegions array are fully streamed in. */
native final function bool SourceTexturesFullyStreamedIn();

/**
* Regenerates this composite texture using the list of source texture regions.
* The existing mips are reallocated and the RHI resource for the texture is updated
*
* @param NumMipsToGenerate - number of mips to generate. if 0 then all mips are created
*/
native final function UpdateCompositeTextue(int NumMipsToGenerate);

cpptext
{
	/**
	* Calculate the first available mip from a set of textures based on the LOD bias for each
	* texture.
	*
	* @return first available mip index from the source regions
	*/
	INT GetFirstAvailableMipIndex();

	/**
	* Reallocates the mips array
	*
	* @param NumMipsToGenerate - number of mips to generate. if 0 then all mips are created
	*/
	void InitMips( INT NumMipsToGenerate );

	/**
	* Locks each region of the source texture mip and copies the block of data
	* for that region to the destination mip buffer. This is done for all mip levels.
	*/
	void CopyRectRegions();

private:
	/**
	* Initializes the list of ValidRegions with only valid entries from the list of source regions
	*/
	virtual void InitValidSourceRegions();

	/**
	* Locks each region of the source RHI texture 2d resources and copies the block of data
	* for that region to the destination mip buffer. This is done for all mip levels.
	*
	* (Only called by the rendering thread)
	*/
	virtual void RenderThread_CopyRectRegions();
}

defaultproperties
{
	// all mip levels will be resident in memory
	NeverStream=True
}
