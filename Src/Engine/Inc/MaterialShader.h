/*=============================================================================
	MaterialShader.h: Material shader definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** The minimum package version to load material pixel shaders with. */
#define VER_MIN_MATERIAL_PIXELSHADER	VER_FP_BLENDING_FALLBACK

/** Same as VER_MIN_MATERIAL_PIXELSHADER, but for the licensee package version. */
#define LICENSEE_VER_MIN_MATERIAL_PIXELSHADER	0

/** The minimum package version to load material vertex shaders with. */
#define VER_MIN_MATERIAL_VERTEXSHADER	VER_AUGUST_XDK_UPGRADE

/** Same as VER_MIN_MATERIAL_VERTEXSHADER, but for the licensee package version. */
#define LICENSEE_VER_MIN_MATERIAL_VERTEXSHADER	0

/** A macro to implement material shaders which checks the package version for VER_MIN_MATERIAL_*SHADER and LICENSEE_VER_MIN_MATERIAL_*SHADER. */
#define IMPLEMENT_MATERIAL_SHADER_TYPE(TemplatePrefix,ShaderClass,SourceFilename,FunctionName,Frequency,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_SHADER_TYPE( \
		TemplatePrefix, \
		ShaderClass, \
		SourceFilename, \
		FunctionName, \
		Frequency, \
		Max(MinPackageVersion,Frequency == SF_Pixel ? Max(VER_MIN_COMPILEDMATERIAL, VER_MIN_MATERIAL_PIXELSHADER) : Max(VER_MIN_COMPILEDMATERIAL, VER_MIN_MATERIAL_VERTEXSHADER)), \
		Max(MinLicenseePackageVersion,Frequency == SF_Pixel ? Max(LICENSEE_VER_MIN_COMPILEDMATERIAL, LICENSEE_VER_MIN_MATERIAL_PIXELSHADER) : Max(LICENSEE_VER_MIN_COMPILEDMATERIAL, LICENSEE_VER_MIN_MATERIAL_VERTEXSHADER)) \
		);

/**
 * An encapsulation of the material parameters for a shader.
 */
class FMaterialPixelShaderParameters
{
public:

	void Bind(const FMaterial* Material,const FShaderParameterMap& ParameterMap);
	void Set(FCommandContextRHI* Context,FShader* PixelShader,const FMaterialRenderContext& MaterialRenderContext) const;
	
	/**
	* Set local transforms for rendering a material with a single mesh
	* @param Context - command context
	* @param MaterialInstance - material used for the shader
	* @param LocalToWorld - l2w for rendering a single mesh
	* @param bBackFace - True if the backfaces of a two-sided material are being rendered.
	*/
	void SetLocalTransforms(
		FCommandContextRHI* Context,
		FShader* PixelShader,
		const FMaterialInstance* MaterialInstance,
		const FMatrix& LocalToWorld,
		UBOOL bBackFace
		) const;

	friend FArchive& operator<<(FArchive& Ar,FMaterialPixelShaderParameters& Parameters);

private:
	struct FUniformParameter
	{
		BYTE Type;
		INT Index;
		FShaderParameter ShaderParameter;
		friend FArchive& operator<<(FArchive& Ar,FUniformParameter& P)
		{
			return Ar << P.Type << P.Index << P.ShaderParameter;
		}
	};
	TArray<FUniformParameter> UniformParameters;
	/** matrix parameter for materials with a world transform */
	FShaderParameter LocalToWorldParameter;
	/** matrix parameter for materials with a view transform */
	FShaderParameter WorldToViewParameter;
	/** The scene texture parameters. */
	FSceneTextureShaderParameters SceneTextureParameters;
	/** Parameter indicating whether the front-side or the back-side of a two-sided material is being rendered. */
	FShaderParameter TwoSidedSignParameter;
};

/**
 * A shader meta type for material-linked shaders.
 */
class FMaterialShaderType : public FShaderType
{
public:

	struct CompiledShaderInitializerType : FGlobalShaderType::CompiledShaderInitializerType
	{
		const FMaterial* Material;
		CompiledShaderInitializerType(
			FShaderType* InType,
			const FShaderCompilerOutput& CompilerOutput,
			const FMaterial* InMaterial
			):
			FGlobalShaderType::CompiledShaderInitializerType(InType,CompilerOutput),
			Material(InMaterial)
		{}
	};
	typedef FShader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);
	typedef UBOOL (*ShouldCacheType)(EShaderPlatform,const FMaterial*);

	FMaterialShaderType(
		const TCHAR* InName,
		const TCHAR* InSourceFilename,
		const TCHAR* InFunctionName,
		DWORD InFrequency,
		INT InMinPackageVersion,
		INT InMinLicenseePackageVersion,
		ConstructSerializedType InConstructSerializedRef,
		ConstructCompiledType InConstructCompiledRef,
		ModifyCompilationEnvironmentType InModifyCompilationEnvironmentRef,
		ShouldCacheType InShouldCacheRef
		):
		FShaderType(InName,InSourceFilename,InFunctionName,InFrequency,InMinPackageVersion,InMinLicenseePackageVersion,InConstructSerializedRef,InModifyCompilationEnvironmentRef),
		ConstructCompiledRef(InConstructCompiledRef),
		ShouldCacheRef(InShouldCacheRef)
	{}

	/**
	 * Compiles a shader of this type.  After compiling the shader, either returns an equivalent existing shader of this type, or constructs
	 * a new instance.
	 * @param Compiler - The shader compiler to use.
	 * @param Material - The material to link the shader with.
	 * @param MaterialShaderCode - The shader code for the material.
	 * @param OutErrors - Upon compilation failure, OutErrors contains a list of the errors which occured.
	 * @return NULL if the compilation failed.
	 */
	FShader* CompileShader(
		EShaderPlatform Platform,
		const FMaterial* Material,
		const TCHAR* MaterialShaderCode,
		TArray<FString>& OutErrors
		);

	/**
	 * Checks if the shader type should be cached for a particular platform and material.
	 * @param Platform - The platform to check.
	 * @param Material - The material to check.
	 * @return True if this shader type should be cached.
	 */
	UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material) const
	{
		return (*ShouldCacheRef)(Platform,Material);
	}

	// Dynamic casting.
	virtual FMaterialShaderType* GetMaterialShaderType() { return this; }

private:
	ConstructCompiledType ConstructCompiledRef;
	ShouldCacheType ShouldCacheRef;
};

/**
 * The set of material shaders for a single material.
 */
class FMaterialShaderMap : public TShaderMap<FMaterialShaderType>, public FRefCountedObject
{
public:

	/**
	 * Finds the shader map for a material.
	 * @param Id - The shader map's ID.
	 * @param Platform - The platform to lookup for
	 * @return NULL if no cached shader map was found.
	 */
	static FMaterialShaderMap* FindId(const FGuid& Id, EShaderPlatform Platform);

	// Destructor.
	~FMaterialShaderMap();

	/**
	 * Compiles the shaders for a material and caches them in this shader map.
	 * @param Material - The material to compile shaders for.
	 * @param MaterialShaderCode - The shader code for Material.
	 * @param Platform - The platform to compile to
	 * @param OutErrors - Upon compilation failure, OutErrors contains a list of the errors which occured.
	 * @return True if the compilation succeeded.
	 */
	UBOOL Compile(const FMaterial* Material,const TCHAR* MaterialShaderCode,EShaderPlatform Platform,TArray<FString>& OutErrors,UBOOL bSilent = FALSE);

	/**
	 * Checks whether the material shader map is missing any shader types necessary for the given material.
	 * @param Material - The material which is checked.
	 * @return True if the shader map has all of the shader types necessary.
	 */
	UBOOL IsComplete(const FMaterial* Material) const;

	/**
	 * Builds a list of the shaders in a shader map.
	 */
	void GetShaderList(TMap<FGuid,FShader*>& OutShaders) const;

	/**
	 * Begins initializing the shaders used by the material shader map.
	 */
	void BeginInit();

	/**
	 * Removes all entries in the cache with exceptions based on a shader type
	 * @param ShaderType - The shader type to flush or keep (depending on second param)
	 * @param bFlushAllButShaderType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given shader type
	 */
	void FlushShadersByShaderType(FShaderType* ShaderType, UBOOL bFlushAllButShaderType=FALSE);

	/**
	 * Removes all entries in the cache with exceptions based on a vertex factory type
	 * @param ShaderType - The shader type to flush or keep (depending on second param)
	 * @param bFlushAllButVertexFactoryType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given vertex factory type
	 */
	void FlushShadersByVertexFactoryType(FVertexFactoryType* VertexFactoryType, UBOOL bFlushAllButVertexFactoryType=FALSE);

	// Serializer.
	void Serialize(FArchive& Ar);

	// Accessors.
	const class FMeshMaterialShaderMap* GetMeshShaderMap(FVertexFactoryType* VertexFactoryType) const;
	const FGuid& GetMaterialId() const { return MaterialId; }

private:

	/** A global map from material ID to any shader map cached for that material. */
	static TDynamicMap<FGuid,FMaterialShaderMap*> GIdToMaterialShaderMap[SP_NumPlatforms];

	/** The material's cached shaders for vertex factory type dependent shaders. */
	TIndirectArray<class FMeshMaterialShaderMap> MeshShaderMaps;

	/** The persistent GUID of this material shader map. */
	FGuid MaterialId;

	/** The material's user friendly name, typically the object name. */
	FString FriendlyName;

	/** The platform this shader map was compiled with */
	EShaderPlatform Platform;

	/** A map from vertex factory type to the material's cached shaders for that vertex factory type. */
	TMap<FVertexFactoryType*,class FMeshMaterialShaderMap*> VertexFactoryMap;

	/**
	 * Initializes VertexFactoryMap from the contents of MeshShaderMaps.
	 */
	void InitVertexFactoryMap();
};
