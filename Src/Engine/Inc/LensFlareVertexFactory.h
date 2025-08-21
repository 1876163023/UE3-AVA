/**
*	LensFlareVertexFactory.h: Lens flare vertex factory definitions.
*	Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
*/

/**
*	
*/
class FLensFlareVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FLensFlareVertexFactory);

public:
	FLensFlareVertexFactory()
	{
	}

	virtual ~FLensFlareVertexFactory()
	{
	}

	// FRenderResource interface.
	virtual void InitRHI();
	virtual void ReleaseRHI();

	virtual FString GetFriendlyName()
	{
		return FString( TEXT("LensFlare Vertex Factory") );
	}

	/**
	* Should we cache the material's shadertype on this platform with this vertex factory? 
	*/
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	/**
	* Can be overridden by FVertexFactory subclasses to modify their compile environment just before compilation occurs.
	*/
	static void ModifyCompilationEnvironment(FShaderCompilerEnvironment& OutEnvironment);

	static void FillElementList(FVertexDeclarationElementList& Elements, INT& Offset);
	static UBOOL CreateCachedLensFlareDeclaration();

protected:
	/** The RHI vertex declaration used to render the factory normally. */
	static FVertexDeclarationRHIRef LensFlareDeclaration;

private:
};

/**
*	
*/
class FLensFlareVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	virtual void Bind(const FShaderParameterMap& ParameterMap)
	{
		CameraWorldPositionParameter.Bind(ParameterMap,TEXT("CameraWorldPosition"),TRUE);
		CameraRightParameter.Bind(ParameterMap,TEXT("CameraRight"),TRUE);
		CameraUpParameter.Bind(ParameterMap,TEXT("CameraUp"),TRUE);
		LocalToWorldParameter.Bind(ParameterMap,TEXT("LocalToWorld"));
	}

	virtual void Serialize(FArchive& Ar)
	{
		Ar << CameraWorldPositionParameter;
		Ar << CameraRightParameter;
		Ar << CameraUpParameter;
		Ar << LocalToWorldParameter;
	}

	virtual void Set(FCommandContextRHI* Context,FShader* VertexShader,const FVertexFactory* VertexFactory,const FSceneView* View) const;

	virtual void SetLocalTransforms(FCommandContextRHI* Context,FShader* VertexShader,const FMatrix& LocalToWorld,const FMatrix& WorldToLocal) const
	{
		SetVertexShaderValue(Context,VertexShader->GetVertexShader(),LocalToWorldParameter,LocalToWorld);
	}

private:
	FShaderParameter CameraWorldPositionParameter;
	FShaderParameter CameraRightParameter;
	FShaderParameter CameraUpParameter;
	FShaderParameter LocalToWorldParameter;
};
