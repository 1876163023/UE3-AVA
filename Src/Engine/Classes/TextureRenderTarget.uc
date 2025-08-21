/**
 * TextureRenderTarget
 *
 * Base for all render target texture resources
 *
 * Copyright 2006 Epic Games, Inc. All Rights Reserved.
 */
class TextureRenderTarget extends Texture
	native
	abstract;

cpptext
{
	/**
	 * Access the render target resource for this texture target object
	 * @return pointer to resource or NULL if not initialized
	 */
	FTextureRenderTargetResource* GetRenderTargetResource();

	// UTexture interface.

	/**
	 * No mip data used so compression is not needed
	 */
	virtual void Compress();

	/**
	 * Create a new 2D render target texture resource
	 * @return newly created FTextureRenderTarget2DResource
	 */
	virtual FTextureResource* CreateResource();
	
	/**
	 * Materials should treat a render target 2D texture like a regular 2D texture resource.
	 * @return EMaterialValueType for this resource
	 */
	virtual EMaterialValueType GetMaterialType();
}

defaultproperties
{	
	// no mip data so no streaming
	NeverStream=True
	// render target surface never compressed
	CompressionNone=True
	// assume contents are in gamma corrected space
	SRGB=True
	// all render target texture resources belong to this group
	LODGroup=TEXTUREGROUP_RenderTarget
}
