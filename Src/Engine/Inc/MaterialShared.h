/*=============================================================================
	MaterialShared.h: Shared material definitions.
	Copyright 2003-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#define ME_CAPTION_HEIGHT		18
#define ME_STD_VPADDING			16
#define ME_STD_HPADDING			32
#define ME_STD_BORDER			8
#define ME_STD_THUMBNAIL_SZ		96
#define ME_PREV_THUMBNAIL_SZ	256
#define ME_STD_LABEL_PAD		16
#define ME_STD_TAB_HEIGHT		21

/**
 * The minimum package version which stores valid material compilation output.  Increasing this version will cause all materials saved in
 * older versions to generate their code on load.  Additionally, material shaders cached before the version will be discarded.
 */
#define VER_MIN_COMPILEDMATERIAL VER_TEXTUREDENSITY

//<@ ava specific ; 2007. 1. 15 changmin
// licensee_version up
/** Same as VER_MIN_COMPILEDMATERIAL, but compared against the licensee package version. */
#define LICENSEE_VER_MIN_COMPILEDMATERIAL	VER_AVA_NEEDSBUMPEDLIGHTMAP


/**
 * The types which can be used by materials.
 */
enum EMaterialValueType
{
	MCT_Float1		= 1,
	MCT_Float2		= 2,
	MCT_Float3		= 4,
	MCT_Float4		= 8,
	MCT_Float		= 8|4|2|1,
	MCT_Texture2D	= 16,
	MCT_TextureCube	= 32,
	MCT_Texture		= 16|32,
	MCT_Unknown		= 64
};

/**
 * The context of a material being rendered.
 */
struct FMaterialRenderContext
{
	/** material instance used for the material shader */
	const FMaterialInstance* MaterialInstance;	
	/** current scene time */
	FLOAT CurrentTime;
	/** The current real-time */
	FLOAT CurrentRealTime;
	/** view matrix used for transform expression */
	const FSceneView* View;

	/** 
	* Constructor
	*/
	FMaterialRenderContext(const FMaterialInstance* InMaterialInstance,FLOAT InCurrentTime,FLOAT InCurrentRealTime,const FSceneView* InView)
		:	MaterialInstance(InMaterialInstance)
		,	CurrentTime(InCurrentTime)
		,	CurrentRealTime(InCurrentRealTime)
		,	View(InView)
	{
	}
};

/**
 * Represents a subclass of FMaterialUniformExpression.
 */
class FMaterialUniformExpressionType
{
public:

	typedef class FMaterialUniformExpression* (*SerializationConstructorType)();

	/**
	 * @return The global uniform epression type list.
	 */
	static TLinkedList<FMaterialUniformExpressionType*>*& GetTypeList();

	/**
	 * Minimal initialization constructor.
	 */
	FMaterialUniformExpressionType(const TCHAR* InName,SerializationConstructorType InSerializationConstructor);

	/**
	 * Serializer for references to uniform expressions.
	 */
	friend FArchive& operator<<(FArchive& Ar,class FMaterialUniformExpression*& Ref);

private:

	const TCHAR* Name;
	SerializationConstructorType SerializationConstructor;
};

#define DECLARE_MATERIALUNIFORMEXPRESSION_TYPE(Name) \
	public: \
	static FMaterialUniformExpressionType StaticType; \
	static FMaterialUniformExpression* SerializationConstructor() { return new Name(); } \
	virtual FMaterialUniformExpressionType* GetType() const { return &StaticType; }

#define IMPLEMENT_MATERIALUNIFORMEXPRESSION_TYPE(Name) \
	FMaterialUniformExpressionType Name::StaticType(TEXT(#Name),&Name::SerializationConstructor);

/**
 * Represents an expression which only varies with uniform inputs.
 */
class FMaterialUniformExpression : public FRefCountedObject
{
public:

	virtual ~FMaterialUniformExpression() {}

	virtual FMaterialUniformExpressionType* GetType() const = 0;
	virtual void Serialize(FArchive& Ar) = 0;

	virtual void GetNumberValue(const FMaterialRenderContext& Context,FLinearColor& OutValue) const {}
	virtual void GetTextureValue(const FMaterialRenderContext& Context,const FTexture** OutValue) const {}
	virtual void GetGameThreadTextureValue(class UMaterialInstance* MaterialInstance,UTexture** OutValue) const {}
	virtual UBOOL IsConstant() const { return FALSE; }
	virtual UBOOL IsIdentical(const FMaterialUniformExpression* OtherExpression) const { return FALSE; }
};

//
//	FMaterialCompiler
//

struct FMaterialCompiler
{
	virtual INT Error(const TCHAR* Text) = 0;
	INT Errorf(const TCHAR* Format,...);

	virtual INT CallExpression(UMaterialExpression* MaterialExpression,FMaterialCompiler* InCompiler) = 0;

	virtual EMaterialValueType GetType(INT Code) = 0;
	virtual INT ForceCast(INT Code,EMaterialValueType DestType) = 0;

	virtual INT VectorParameter(FName ParameterName,const FLinearColor& DefaultValue) = 0;
	virtual INT ScalarParameter(FName ParameterName,FLOAT DefaultValue) = 0;
	virtual INT FlipBookOffset(UTexture* InFlipBook) = 0;

	virtual INT Constant(FLOAT X) = 0;
	virtual INT Constant2(FLOAT X,FLOAT Y) = 0;
	virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z) = 0;
	virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W) = 0;

	virtual INT GameTime() = 0;
	virtual INT RealTime() = 0;
	virtual INT PeriodicHint(INT PeriodicCode) { return PeriodicCode; }

	virtual INT Sine(INT X) = 0;
	virtual INT Cosine(INT X) = 0;

	virtual INT Floor(INT X) = 0;
	virtual INT Ceil(INT X) = 0;
	virtual INT Frac(INT X) = 0;
	virtual INT Abs(INT X) = 0;

	virtual INT ReflectionVector() = 0;
	virtual INT CameraVector() = 0;
	virtual INT LightVector() = 0;

	virtual INT ScreenPosition( UBOOL bScreenAlign ) = 0;

	virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB) = 0;

	virtual INT TextureCoordinate(UINT CoordinateIndex) = 0;
	virtual INT TextureSample(INT Texture,INT Coordinate) = 0;

	virtual INT Texture(UTexture* Texture) = 0;
	virtual INT TextureParameter(FName ParameterName,UTexture* DefaultTexture) = 0;

	virtual	INT EnvCube( INT CoordinateIdx)=0;
	virtual	INT SceneTextureSample( BYTE TexType, INT CoordinateIdx)=0;
	virtual	INT SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx)=0;
	virtual	INT PixelDepth(UBOOL bNormalize)=0;
	virtual	INT DestColor()=0;
	virtual	INT DestDepth(UBOOL bNormalize)=0;
	virtual INT DepthBiasedAlpha( INT SrcAlphaIdx, INT BiasIdx, INT BiasScaleIdx )=0;
	virtual INT DepthBiasedBlend( INT SrcColorIdx, INT BiasIdx, INT BiasScaleIdx )=0;

	virtual INT VertexColor() = 0;

	virtual INT Add(INT A,INT B) = 0;
	virtual INT Sub(INT A,INT B) = 0;
	virtual INT Mul(INT A,INT B) = 0;
	virtual INT Div(INT A,INT B) = 0;
	virtual INT Dot(INT A,INT B) = 0;
	virtual INT Cross(INT A,INT B) = 0;

	virtual INT Power(INT Base,INT Exponent) = 0;
	virtual INT SquareRoot(INT X) = 0;

	virtual INT Lerp(INT X,INT Y,INT A) = 0;
	virtual INT Min(INT A,INT B) = 0;
	virtual INT Max(INT A,INT B) = 0;
	virtual INT Clamp(INT X,INT A,INT B) = 0;

	virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A) = 0;
	virtual INT AppendVector(INT A,INT B) = 0;
	virtual INT TransformVector(BYTE CoordType,INT A) = 0;

	virtual INT LensFlareIntesity() = 0;
	virtual INT LensFlareRadialDistance() = 0;
	virtual INT LensFlareRayDistance() = 0;
	virtual INT LensFlareSourceDistance() = 0;
};

extern EShaderPlatform GShaderCompilePlatform;

//
//	FProxyMaterialCompiler - A proxy for the compiler interface which by default passes all function calls unmodified.
//

struct FProxyMaterialCompiler: FMaterialCompiler
{
	FMaterialCompiler*	Compiler;

	// Constructor.

	FProxyMaterialCompiler(FMaterialCompiler* InCompiler):
		Compiler(InCompiler)
	{}

	// Simple pass through all other material operations unmodified.

	virtual INT Error(const TCHAR* Text) { return Compiler->Error(Text); }

	virtual INT CallExpression(UMaterialExpression* MaterialExpression,FMaterialCompiler* InCompiler) { return Compiler->CallExpression(MaterialExpression,InCompiler); }

	virtual EMaterialValueType GetType(INT Code) { return Compiler->GetType(Code); }
	virtual INT ForceCast(INT Code,EMaterialValueType DestType) { return Compiler->ForceCast(Code,DestType); }

	virtual INT VectorParameter(FName ParameterName,const FLinearColor& DefaultValue) { return Compiler->VectorParameter(ParameterName,DefaultValue); }
	virtual INT ScalarParameter(FName ParameterName,FLOAT DefaultValue) { return Compiler->ScalarParameter(ParameterName,DefaultValue); }

	virtual INT Constant(FLOAT X) { return Compiler->Constant(X); }
	virtual INT Constant2(FLOAT X,FLOAT Y) { return Compiler->Constant2(X,Y); }
	virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z) { return Compiler->Constant3(X,Y,Z); }
	virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W) { return Compiler->Constant4(X,Y,Z,W); }

	virtual INT GameTime() { return Compiler->GameTime(); }
	virtual INT RealTime() { return Compiler->RealTime(); }

	virtual INT PeriodicHint(INT PeriodicCode) { return Compiler->PeriodicHint(PeriodicCode); }

	virtual INT Sine(INT X) { return Compiler->Sine(X); }
	virtual INT Cosine(INT X) { return Compiler->Cosine(X); }

	virtual INT Floor(INT X) { return Compiler->Floor(X); }
	virtual INT Ceil(INT X) { return Compiler->Ceil(X); }
	virtual INT Frac(INT X) { return Compiler->Frac(X); }
	virtual INT Abs(INT X) { return Compiler->Abs(X); }

	virtual INT ReflectionVector() { return Compiler->ReflectionVector(); }
	virtual INT CameraVector() { return Compiler->CameraVector(); }
	virtual INT LightVector() { return Compiler->LightVector(); }

	virtual INT ScreenPosition( UBOOL bScreenAlign ) { return Compiler->ScreenPosition( bScreenAlign ); }

	virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB) { return Compiler->If(A,B,AGreaterThanB,AEqualsB,ALessThanB); }

	virtual INT TextureSample(INT Texture,INT Coordinate) { return Compiler->TextureSample(Texture,Coordinate); }
	virtual INT TextureCoordinate(UINT CoordinateIndex) { return Compiler->TextureCoordinate(CoordinateIndex); }

	virtual INT Texture(UTexture* Texture) { return Compiler->Texture(Texture); }
	virtual INT TextureParameter(FName ParameterName,UTexture* DefaultValue) { return Compiler->TextureParameter(ParameterName,DefaultValue); }

	virtual	INT EnvCube(INT CoordinateIdx) { return Compiler->EnvCube(CoordinateIdx);	}
	virtual	INT SceneTextureSample(BYTE TexType,INT CoordinateIdx) { return Compiler->SceneTextureSample(TexType,CoordinateIdx);	}
	virtual	INT SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx) { return Compiler->SceneTextureDepth(bNormalize,CoordinateIdx);	}
	virtual	INT PixelDepth(UBOOL bNormalize) { return Compiler->PixelDepth(bNormalize);	}
	virtual	INT DestColor() { return Compiler->DestColor(); }
	virtual	INT DestDepth(UBOOL bNormalize) { return Compiler->DestDepth(bNormalize); }
	virtual INT DepthBiasedAlpha( INT SrcAlphaIdx, INT BiasIdx, INT BiasScaleIdx ) { return Compiler->DepthBiasedAlpha(SrcAlphaIdx,BiasIdx,BiasScaleIdx); }
	virtual INT DepthBiasedBlend( INT SrcColorIdx, INT BiasIdx, INT BiasScaleIdx ) { return Compiler->DepthBiasedBlend(SrcColorIdx,BiasIdx,BiasScaleIdx); }

	virtual INT VertexColor() { return Compiler->VertexColor(); }

	virtual INT Add(INT A,INT B) { return Compiler->Add(A,B); }
	virtual INT Sub(INT A,INT B) { return Compiler->Sub(A,B); }
	virtual INT Mul(INT A,INT B) { return Compiler->Mul(A,B); }
	virtual INT Div(INT A,INT B) { return Compiler->Div(A,B); }
	virtual INT Dot(INT A,INT B) { return Compiler->Dot(A,B); }
	virtual INT Cross(INT A,INT B) { return Compiler->Cross(A,B); }

	virtual INT Power(INT Base,INT Exponent) { return Compiler->Power(Base,Exponent); }
	virtual INT SquareRoot(INT X) { return Compiler->SquareRoot(X); }

	virtual INT Lerp(INT X,INT Y,INT A) { return Compiler->Lerp(X,Y,A); }
	virtual INT Min(INT A,INT B) { return Compiler->Min(A,B); }
	virtual INT Max(INT A,INT B) { return Compiler->Max(A,B); }
	virtual INT Clamp(INT X,INT A,INT B) { return Compiler->Clamp(X,A,B); }

	virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A) { return Compiler->ComponentMask(Vector,R,G,B,A); }
	virtual INT AppendVector(INT A,INT B) { return Compiler->AppendVector(A,B); }
	virtual INT TransformVector(BYTE CoordType,INT A) { return Compiler->TransformVector(CoordType,A); }
};

//
//	EMaterialProperty
//

enum EMaterialProperty
{
	MP_EmissiveColor = 0,
	MP_Opacity,
	MP_OpacityMask,
	MP_Distortion,
	MP_TwoSidedLightingMask,
	MP_DiffuseColor,
	MP_SpecularColor,
	MP_SpecularPower,
	MP_Normal,
	MP_CustomLighting,
	MP_AmbientMask,
	MP_MAX
};

/**
 * @return The type of value expected for the given material property.
 */
extern EMaterialValueType GetMaterialPropertyType(EMaterialProperty Property);

/** transform types usable by a material shader */
enum ECoordTransformUsage
{
	// no transforms used
	UsedCoord_None		=0,
	// local to world used
	UsedCoord_World		=1<<0,
	// local to view used
	UsedCoord_View		=1<<1,
	// local to local used
	UsedCoord_Local		=1<<2
};

/**
 * A material.
 */
class FMaterial
{
	friend class FMaterialPixelShaderParameters;
	//<@ ava specific ; 2007. 1. 16 changmin
	friend class FViewSpaceNormalPixelShader;
	//>@ ava
public:

	/**
	 * Minimal initialization constructor.
	 */
	FMaterial():
		//<@ ava speicifc ; 2007. 1. 24 changmin
		// 주석처리된 parameter는 platform별로 저장할 수 있게 확장되었습니다.
		//MaxTextureDependencyLength(0),
		ShaderMap(NULL),
		Id(0,0,0,0)
		//UsingTransforms(UsedCoord_None),
		//bUsesSceneColor(FALSE),
		//bUsesSceneDepth(FALSE),
		//bUsesEnvCube(FALSE),
		//bValidCompilationOutput(FALSE)
		//>@ ava
	{}

	/**
	 */
	virtual UBOOL Compile( EShaderPlatform Platform, TRefCountPtr<FMaterialShaderMap>& OutShaderMap, UBOOL bForceCompile=FALSE);

	/**
	* Compiles OutShaderMap using the shader code from MaterialShaderCode on Platform
	*
	* @param OutShaderMap - the shader map to compile
	* @param MaterialShaderCode - a filled out instance of MaterialTemplate.usf to compile
	* @param bForceCompile - force discard previous results 
	* @param bSilent - indicates that no error message should be outputted on shader compile failure
	*/
	virtual UBOOL FallbackCompile( EShaderPlatform Platform, 
		TRefCountPtr<FMaterialShaderMap>& OutShaderMap, 
		FString MaterialShaderCode,
		UBOOL bForceCompile,
		UBOOL bSilent = FALSE);

	/**
	* Caches the material shaders for the current platform.
	*/
	void CacheShaders();

	/**
	 * Should the shader for this material with the given platform, shader type and vertex 
	 * factory type combination be compiled
	 *
	 * @param Platform		The platform currently being compiled for
	 * @param ShaderType	Which shader is being compiled
	 * @param VertexFactory	Which vertex factory is being compiled (can be NULL)
	 *
	 * @return TRUE if the shader should be compiled
	 */
	virtual UBOOL ShouldCache(EShaderPlatform Platform, const FShaderType* ShaderType, const FVertexFactoryType* VertexFactoryType) const;

	/**
	 * Called by the material compilation code with a map of the compilation errors.
	 * Note that it is called even if there were no errors, but it passes an empty error map in that case.
	 * @param Errors - A set of expression error pairs.
	 */
	virtual void HandleMaterialErrors(const TMultiMap<UMaterialExpression*,FString>& Errors) {}

	/** Serializes the material. */
	void Serialize(FArchive& Ar);

	/** 
	 * Initializes the material's shader map. 
	 * Invalidate된 경우 TRUE
	 */	
	virtual UBOOL InitShaderMap();

	/**
	 * Null any material expression references for this material
	 */
	void RemoveExpressions();


	// Material properties.
	virtual INT CompileProperty(EMaterialProperty Property,FMaterialCompiler* Compiler) const = 0;
	virtual UBOOL IsTwoSided() const = 0;
	virtual UBOOL IsLightFunction() const = 0;
	virtual UBOOL IsWireframe() const = 0;
	virtual UBOOL IsDistorted() const = 0;
	virtual UBOOL IsSpecialEngineMaterial() const = 0;
	virtual UBOOL IsTerrainMaterial() const = 0;
	virtual UBOOL IsDecalMaterial() const = 0;
	virtual UBOOL IsUsedWithSkeletalMesh() const { return FALSE; }
	virtual UBOOL IsUsedWithParticleSystem() const { return FALSE; }
	virtual UBOOL IsUsedWithLensFlare() const { return FALSE; }
	virtual UBOOL IsMasked() const = 0;
	virtual enum EBlendMode GetBlendMode() const = 0;
	virtual enum EMaterialLightingModel GetLightingModel() const = 0;
	virtual FLOAT GetOpacityMaskClipValue() const = 0;
	virtual FString GetFriendlyName() const
	{
		return FString();
	}

	/**
	 * Should shaders compiled for this material be saved to disk?
	 */
	virtual UBOOL IsPersistent() const = 0;

	// Accessors.
	//const TMultiMap<UMaterialExpression*,FString>& GetCompileErrors() const { return CompileErrors; }
	//const TMap<UMaterialExpression*,INT>& GetTextureDependencyLengthMap() const { return TextureDependencyLengthMap; }
	//INT GetMaxTextureDependencyLength() const { return MaxTextureDependencyLength; }
	//const TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniform2DTextureExpressions() const { return Uniform2DTextureExpressions; }
	//const TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniformCubeTextureExpressions() const { return UniformCubeTextureExpressions; }
	//<@ ava specific ; 2007. 1. 24 changmin
	// Accessors
	const TMultiMap<UMaterialExpression*,FString>& GetCompileErrors() const		{ return MaterialInfos[GRHIShaderPlatform].CompileErrors; }
	const TMap<UMaterialExpression*,INT>& GetTextureDependencyLengthMap() const	{ return MaterialInfos[GRHIShaderPlatform].TextureDependencyLengthMap; }
	INT GetMaxTextureDependencyLength() const									{ return MaterialInfos[GRHIShaderPlatform].MaxTextureDependencyLength; }
	const TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniformVectorExpressions() const		{ return MaterialInfos[GRHIShaderPlatform].UniformVectorExpressions; }
	const TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniformScalarExpressions() const		{ return MaterialInfos[GRHIShaderPlatform].UniformScalarExpressions; }
	const TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniform2DTextureExpressions() const		{ return MaterialInfos[GRHIShaderPlatform].Uniform2DTextureExpressions; }
	const TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniformCubeTextureExpressions() const	{ return MaterialInfos[GRHIShaderPlatform].UniformCubeTextureExpressions; }

	TMultiMap<UMaterialExpression*,FString>& GetCompileErrors()					{ return MaterialInfos[GRHIShaderPlatform].CompileErrors; }
	TMap<UMaterialExpression*,INT>& GetTextureDependencyLengthMap()				{ return MaterialInfos[GRHIShaderPlatform].TextureDependencyLengthMap; }
	TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniformVectorExpressions()		{ return MaterialInfos[GRHIShaderPlatform].UniformVectorExpressions; }
	TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniformScalarExpressions()		{ return MaterialInfos[GRHIShaderPlatform].UniformScalarExpressions; }
	TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniform2DTextureExpressions()		{ return MaterialInfos[GRHIShaderPlatform].Uniform2DTextureExpressions; }
	TArray<TRefCountPtr<FMaterialUniformExpression> >& GetUniformCubeTextureExpressions()	{ return MaterialInfos[GRHIShaderPlatform].UniformCubeTextureExpressions; }
	//>@ ava

	class FMaterialShaderMap* GetShaderMap() const { return ShaderMap; }
	const FGuid& GetId() const { return Id; }
	void SetId(const FGuid& NewId)
	{
		Id = NewId;
	}
	//DWORD GetTransformsUsed() const { return UsingTransforms; }
	//UINT GetUserTexCoordsUsed() const { return NumUserTexCoords; }
	///** Boolean indicators of using SceneColorTexture or SceneDepthTexture	*/
	//UBOOL GetUsesSceneColor() const { return bUsesSceneColor; }
	//UBOOL GetUsesSceneDepth() const { return bUsesSceneDepth; }
	
	/** Information about one texture lookup. */
	struct FTextureLookup
	{
		void	Serialize(FArchive& Ar);
		INT		TexCoordIndex;
		INT		TextureIndex;			// Index into Uniform2DTextureExpressions
		FLOAT	ResolutionMultiplier;	// Multiplier that can be different from 1.0f if the artist uses tiling
	};
	typedef TArray<FTextureLookup> FTextureLookupInfo;

	/** Returns information about all texture lookups. */
	//const FTextureLookupInfo &	GetTextureLookupInfo() const	{ return TextureLookups; }

	//<@ ava specific ; 2007. 1. 22 changmin
	DWORD GetTransformsUsed() const		{ return MaterialInfos[GRHIShaderPlatform].UsingTransforms; }
	UINT GetUserTexCoordsUsed() const	{ return MaterialInfos[GRHIShaderPlatform].NumUserTexCoords; }
	UBOOL GetUsesSceneColor() const		{ return MaterialInfos[GRHIShaderPlatform].bUsesSceneColor; }
	UBOOL GetUsesSceneDepth() const		{ return MaterialInfos[GRHIShaderPlatform].bUsesSceneDepth; }
	UBOOL GetUsesEnvCube() const		{ return MaterialInfos[GRHIShaderPlatform].bUsesEnvCube; }
	UBOOL GetUsesSpecular() const		{ return MaterialInfos[GRHIShaderPlatform].bUsesSpecular; }
	UBOOL GetUsesNormal() const			{ return MaterialInfos[GRHIShaderPlatform].bUsesNormal; }
	UBOOL GetUsesDistortion() const		{ return MaterialInfos[GRHIShaderPlatform].bUsesDistortion; }
	UBOOL GetUsesMask() const			{ return MaterialInfos[GRHIShaderPlatform].bUsesMask; }
	const FTextureLookupInfo &	GetTextureLookupInfo() const	{ return MaterialInfos[GRHIShaderPlatform].TextureLookups; }
	virtual UBOOL NeedsBumpedLightmap() const = 0;
	//>@ ava

protected:
	//void SetUsesSceneColor(UBOOL bInUsesSceneColor) { bUsesSceneColor = bInUsesSceneColor; }
	//void SetUsesSceneDepth(UBOOL bInUsesSceneDepth) { bUsesSceneDepth = bInUsesSceneDepth; }

	//<@ ava specific ; 2007. 1. 23 changmin
	void SetTransformsUsed(DWORD InUsingTransforms)	const		{ MaterialInfos[GRHIShaderPlatform].UsingTransforms = InUsingTransforms; }
	void SetMaxTextureDependencyLength(INT InMaxTextureDependencyLength ) const { MaterialInfos[GRHIShaderPlatform].MaxTextureDependencyLength = InMaxTextureDependencyLength; } 
	void SetUserTexCoordsUsed(UINT InNumUserTexCoords ) const		{ MaterialInfos[GRHIShaderPlatform].NumUserTexCoords = InNumUserTexCoords; }
	void SetUsesSceneColor(UBOOL bInUsesSceneColor) const			{ MaterialInfos[GRHIShaderPlatform].bUsesSceneColor = bInUsesSceneColor; }
	void SetUsesSceneDepth(UBOOL bInUsesSceneDepth) const			{ MaterialInfos[GRHIShaderPlatform].bUsesSceneDepth = bInUsesSceneDepth; }
	void SetUsesEnvCube(UBOOL bInUsesEnvCube) const				{ MaterialInfos[GRHIShaderPlatform].bUsesEnvCube = bInUsesEnvCube; }
	void SetUsesSpecular(UBOOL bInUsesSpecular)	const			{ MaterialInfos[GRHIShaderPlatform].bUsesSpecular = bInUsesSpecular; }
	void SetUsesNormal(UBOOL bInUsesNormal)	const				{ MaterialInfos[GRHIShaderPlatform].bUsesNormal = bInUsesNormal; }
	void SetUsesDistortion( UBOOL bInUsesDistortion ) const		{ MaterialInfos[GRHIShaderPlatform].bUsesDistortion = bInUsesDistortion; }
	void SetUsesMask( UBOOL bInUsesMask ) const					{ MaterialInfos[GRHIShaderPlatform].bUsesMask = bInUsesMask; }
	//>@ ava

	/** Rebuilds the information about all texture lookups. */
	void	RebuildTextureLookupInfo( UMaterial *Material );

	/** Returns the index to the Expression in the Expressions array, or -1 if not found. */
	INT		FindExpression( const TArray<TRefCountPtr<FMaterialUniformExpression> >&Expressions, const FMaterialUniformExpression &Expression );

public:
	//<@ ava specific ; 2007. 1. 24 changmin
	// per platform data를 모아봅시다~
	struct AvaMaterialInfo
	{
		TMultiMap<UMaterialExpression*,FString> CompileErrors;
		TMap<UMaterialExpression*,INT> TextureDependencyLengthMap;
		TArray<TRefCountPtr<FMaterialUniformExpression> > UniformVectorExpressions;
		TArray<TRefCountPtr<FMaterialUniformExpression> > UniformScalarExpressions;
		TArray<TRefCountPtr<FMaterialUniformExpression> > Uniform2DTextureExpressions;
		TArray<TRefCountPtr<FMaterialUniformExpression> > UniformCubeTextureExpressions;
		FTextureLookupInfo	TextureLookups;

		INT MaxTextureDependencyLength;
		UINT NumUserTexCoords;
		DWORD UsingTransforms;
		UBOOL bUsesSceneColor;
		UBOOL bUsesSceneDepth;
		UBOOL bUsesEnvCube;
		UBOOL bUsesSpecular;
		UBOOL bUsesNormal;
		UBOOL bUsesDistortion;
		UBOOL bUsesMask;
		UBOOL bValidCompilationOutput;

		AvaMaterialInfo() : 
			MaxTextureDependencyLength(0),
			NumUserTexCoords(0),
			UsingTransforms(UsedCoord_None),
			bUsesSceneColor(FALSE),
			bUsesSceneDepth(FALSE),
			bUsesEnvCube(FALSE),
			bUsesSpecular(FALSE),
			bUsesNormal(FALSE),
			bUsesDistortion(FALSE),
			bUsesMask(FALSE),
			bValidCompilationOutput(FALSE)
		{}
	};
	mutable AvaMaterialInfo MaterialInfos[SP_NumPlatforms];
	//>@ ava

private:
	//TMultiMap<UMaterialExpression*,FString> CompileErrors;

	///** The texture dependency lengths for the materials' expressions. */
	//TMap<UMaterialExpression*,INT> TextureDependencyLengthMap;

	///** The maximum texture dependency length for the material. */
	//INT MaxTextureDependencyLength;

	TRefCountPtr<FMaterialShaderMap> ShaderMap;
	FGuid Id;

	//TArray<TRefCountPtr<FMaterialUniformExpression> > UniformVectorExpressions;
	//TArray<TRefCountPtr<FMaterialUniformExpression> > UniformScalarExpressions;
	//TArray<TRefCountPtr<FMaterialUniformExpression> > Uniform2DTextureExpressions;
	//TArray<TRefCountPtr<FMaterialUniformExpression> > UniformCubeTextureExpressions;

	///** Information about each texture lookup in the pixel shader. */
	//FTextureLookupInfo	TextureLookups;

	//UINT NumUserTexCoords;
	///** combination of ECoordTransformUsage flags used by this shader */
	//DWORD UsingTransforms;

	///** Boolean indicators of using SceneColorTexture or SceneDepthTexture	*/
	//UBOOL bUsesSceneColor;
	//UBOOL bUsesSceneDepth;

	///**
	// * False if the material's persistent compilation output was loaded from an archive older than VER_MIN_COMPILEDMATERIAL.
	// * (VER_MIN_COMPILEDMATERIAL is defined in MaterialShared.cpp)
	// */
	//UBOOL bValidCompilationOutput;
};

/**
 * A material instance.
 */
class FMaterialInstance 
{
public:
	//<@ ava specific ; 2006. 11. 30 changmin
	FMaterialInstance	*Next;
	void				*FirstElement;
	INT					FrameCount;
	UINT				MinVertexIndex, MaxVertexIndex;
	FMaterialInstance() : Next(NULL), FirstElement(NULL), FrameCount(-1), MinVertexIndex(0), MaxVertexIndex(0)
	{}
	//>@ ava

	// These functions should only be called by the rendering thread.
	virtual const class FMaterial* GetMaterial() const = 0;
	virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const = 0;
	virtual UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const = 0;
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const = 0;
};

/**
 * An instance of a material which overrides the material's Color vector parameter.
 */
class FColoredMaterialInstance : public FMaterialInstance
{
public:

	const FMaterialInstance* const Parent;
	const FLinearColor Color;

	/** Initialization constructor. */
	FColoredMaterialInstance(const FMaterialInstance* InParent,const FLinearColor& InColor):
		Parent(InParent),
		Color(InColor)
	{}

	// FMaterialInstance interface.
	virtual const class FMaterial* GetMaterial() const;
	virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const;
	virtual UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const;
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const;
};

/**
 * A material instance of GEngine->SimpleElementMaterial which specifies the texture parameter.
 */
class FTexturedMaterialInstance : public FMaterialInstance
{
public:

	const FMaterialInstance* const Parent;
	const FTexture* const Texture;

	/** Initialization constructor. */
	FTexturedMaterialInstance(const FMaterialInstance* InParent,const FTexture* InTexture):
		Parent(InParent),
		Texture(InTexture)
	{}

	// FMaterialInstance interface.
	virtual const class FMaterial* GetMaterial() const;
	virtual UBOOL GetVectorValue(const FName& ParameterName,FLinearColor* OutValue) const;
	virtual UBOOL GetScalarValue(const FName& ParameterName,FLOAT* OutValue) const;
	virtual UBOOL GetTextureValue(const FName& ParameterName,const FTexture** OutValue) const;
};

//
//	FExpressionInput
//

//@warning: FExpressionInput is mirrored in MaterialExpression.uc and manually "subclassed" in Material.uc (FMaterialInput)
struct FExpressionInput
{
	class UMaterialExpression*	Expression;
	UBOOL						Mask,
								MaskR,
								MaskG,
								MaskB,
								MaskA;
	DWORD						GCC64Padding; // @todo 64: if the C++ didn't mismirror this structure, we might not need this

	INT Compile(FMaterialCompiler* Compiler);
};

//
//	FMaterialInput
//

template<class InputType> struct FMaterialInput: FExpressionInput
{
	BITFIELD	UseConstant:1;
	InputType	Constant;
};

struct FColorMaterialInput: FMaterialInput<FColor>
{
	
	INT Compile(FMaterialCompiler* Compiler,const FColor& Default);

	//<@ ava specific ; 2007. 1. 22 changmin
	INT Compile(FMaterialCompiler *Compiler, const FColor &Default, UBOOL *bHasValidExpression );
	//>@ ava
};
struct FScalarMaterialInput: FMaterialInput<FLOAT>
{
	INT Compile(FMaterialCompiler* Compiler,FLOAT Default);

	//<@ ava specific ;  2007. 1. 22 changmin
	INT Compile(FMaterialCompiler *Compiler, FLOAT Default, UBOOL *bHasValidExpression );
	//>@ ava
};

struct FVectorMaterialInput: FMaterialInput<FVector>
{
	INT Compile(FMaterialCompiler* Compiler,const FVector& Default);

	//<@ ava specific ; 2007. 1. 22 changmin
	INT Compile(FMaterialCompiler *Compiler, const FVector &Default, UBOOL *bHasValidExpression );
	//>@ ava
};

struct FVector2MaterialInput: FMaterialInput<FVector2D>
{
	INT Compile(FMaterialCompiler* Compiler,const FVector2D& Default);
	//<@ ava specific ; 2007. 1. 22 changmin
	INT Compile(FMaterialCompiler *Compiler, const FVector2D &Default, UBOOL *bHasValidExpression );
	//>@ ava
};

//
//	FExpressionOutput
//

struct FExpressionOutput
{
	FString	Name;
	UBOOL	Mask,
			MaskR,
			MaskG,
			MaskB,
			MaskA;

	FExpressionOutput(UBOOL InMask,UBOOL InMaskR = 0,UBOOL InMaskG = 0,UBOOL InMaskB = 0,UBOOL InMaskA = 0):
		Mask(InMask),
		MaskR(InMaskR),
		MaskG(InMaskG),
		MaskB(InMaskB),
		MaskA(InMaskA)
	{}
};

/**
 * @return True if BlendMode is translucent, False if it is opaque.
 */
extern UBOOL IsTranslucentBlendMode(enum EBlendMode BlendMode);

/**
 * A resource which represents UMaterial to the renderer.
 */
class FMaterialResource : public FMaterial
{
public:

	FRenderCommandFence ReleaseFence;

	FMaterialResource(UMaterial* InMaterial);
	virtual ~FMaterialResource() {}

	// FMaterial interface.
	virtual UBOOL InitShaderMap();
	virtual INT CompileProperty(EMaterialProperty Property,FMaterialCompiler* Compiler) const;
	virtual UBOOL IsTwoSided() const;
	virtual UBOOL IsLightFunction() const;
	virtual UBOOL IsWireframe() const;
	virtual UBOOL IsSpecialEngineMaterial() const;
	virtual UBOOL IsTerrainMaterial() const;
	virtual UBOOL IsDecalMaterial() const;
	virtual UBOOL IsUsedWithSkeletalMesh() const;
	virtual UBOOL IsUsedWithParticleSystem() const;
	virtual UBOOL IsUsedWithLensFlare() const;
	virtual enum EBlendMode GetBlendMode() const;
	virtual enum EMaterialLightingModel GetLightingModel() const;
	virtual FLOAT GetOpacityMaskClipValue() const;
	virtual UBOOL IsDistorted() const;
	virtual UBOOL IsMasked() const;
	virtual FString GetFriendlyName() const;

	//<@ ava specific ; 2007. 1. 18 changmin
	virtual UBOOL NeedsBumpedLightmap() const;
	//>@ ava
	/**
	 * Should shaders compiled for this material be saved to disk?
	 */
	virtual UBOOL IsPersistent() const;

	/** Allows the resource to do things upon compile. */
	virtual UBOOL Compile( EShaderPlatform Platform, TRefCountPtr<FMaterialShaderMap>& OutShaderMap, UBOOL bForceCompile=FALSE);

protected:
	UMaterial* Material;
};
