class avaTexture2DComposite extends Texture2DComposite
	native;

native final function UpdateCompositeTextueEx(int nSizeX, int nSizeY, int NumMipsToGenerate);

cpptext
{
private:
	/**
	* Initializes the list of ValidRegions with only valid entries from the list of source regions
	*/
	virtual void InitValidSourceRegions();

	virtual void RenderThread_CopyRectRegions();
	void RHICopyTexture2D(FTexture2DRHIParamRef DstTexture, UINT MipIdx, INT BaseSizeX, INT BaseSizeY, INT Format, const TArray<FCopyTextureRegion2D>& Regions);
}