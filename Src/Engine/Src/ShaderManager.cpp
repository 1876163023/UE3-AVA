/*=============================================================================
	ShaderManager.cpp: Shader manager implementation
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

IMPLEMENT_CLASS(UShaderCache);

// local and reference shader cache IDs

enum EShaderCacheType
{
	SC_Local				= 0,
	SC_Reference			= 1,

	SC_NumShaderCacheTypes	= 2,
};

/** Global shader caches. Not handled by GC and code associating objects is responsible for adding them to the root set. */
UShaderCache* GShaderCaches[SC_NumShaderCacheTypes][SP_NumPlatforms];

UBOOL GSerializingLocalShaderCache = FALSE;

/** The global shader map. */
TShaderMap<FGlobalShaderType>* GGlobalShaderMap[SP_NumPlatforms] = {0};

IMPLEMENT_SHADER_TYPE(,FNULLPixelShader,TEXT("NULLPixelShader"),TEXT("Main"),SF_Pixel,0,0);

UBOOL FShaderParameterMap::FindParameterAllocation(const TCHAR* ParameterName,WORD& OutBaseRegisterIndex,WORD& OutNumRegisters) const
{
	const FParameterAllocation* Allocation = ParameterMap.Find(ParameterName);
	if(Allocation)
	{
		OutBaseRegisterIndex = Allocation->BaseRegisterIndex;
		OutNumRegisters = Allocation->NumRegisters;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void FShaderParameterMap::AddParameterAllocation(const TCHAR* ParameterName,WORD BaseRegisterIndex,WORD NumRegisters)
{
	FParameterAllocation Allocation;
	Allocation.BaseRegisterIndex = BaseRegisterIndex;
	Allocation.NumRegisters = NumRegisters;
	ParameterMap.Set(ParameterName,Allocation);
}

FString LoadShaderSourceFile(const TCHAR* Filename)
{
	// Load the specified file from the System/Shaders directory.
	FFilename ShaderFilename = FString(appBaseDir()) * appShaderDir() * Filename;

	if (ShaderFilename.GetExtension() != TEXT("usf"))
	{
		ShaderFilename += TEXT(".usf");
	}

	FString	FileContents;
	if( !appLoadFileToString(FileContents, *ShaderFilename))
	{
		appErrorf(TEXT("Couldn't load shader file \'%s\'"),Filename);
	}
	return FileContents;
}

void FShaderParameter::Bind(const FShaderParameterMap& ParameterMap,const TCHAR* ParameterName,UBOOL bIsOptional)
{
	if(!ParameterMap.FindParameterAllocation(ParameterName,BaseRegisterIndex,NumRegisters) && !bIsOptional)
	{
		// Error: Wasn't able to bind a non-optional parameter.
		appErrorf(TEXT("Failure to bind non-optional shader parameter %s"),ParameterName);
	}
}

FArchive& operator<<(FArchive& Ar,FShaderParameter& P)
{
	return Ar << P.BaseRegisterIndex << P.NumRegisters;
}

TLinkedList<FShaderType*>*& FShaderType::GetTypeList()
{
	static TLinkedList<FShaderType*>* TypeList = NULL;
	return TypeList;
}

/**
* @return The global shader name to type map
*/
TMap<FName, FShaderType*>& FShaderType::GetNameToTypeMap()
{
	static TMap<FName, FShaderType*> NameToTypeMap;
	return NameToTypeMap;
}

FArchive& operator<<(FArchive& Ar,FShaderType*& Ref)
{
	if(Ar.IsSaving())
	{
		FName FactoryName = Ref ? FName(Ref->Name) : NAME_None;
		Ar << FactoryName;
	}
	else if(Ar.IsLoading())
	{
		FName FactoryName;
		Ar << FactoryName;

		Ref = NULL;

		if(FactoryName != NAME_None)
		{
			// look for the shader type in the global name to type map
			FShaderType** ShaderType = FShaderType::GetNameToTypeMap().Find(FactoryName);
			if (ShaderType)
			{
				// if we found it, use it
				Ref = *ShaderType;
			}
		}
	}
	return Ar;
}

void FShaderType::RegisterShader(FShader* Shader)
{
	ShaderIdMap.Set(Shader->GetId(),Shader);
	Shader->CodeMapRef = ShaderCodeMap.Add(Shader);
}

void FShaderType::DeregisterShader(FShader* Shader)
{
	ShaderIdMap.Remove(Shader->GetId());
	ShaderCodeMap.Remove(Shader->CodeMapRef);
}

FShader* FShaderType::FindShaderByCode(const TArray<BYTE>& Code) const
{
	FShader*const* CodeMapRef = ShaderCodeMap.Find(Code);
	return CodeMapRef ? *CodeMapRef : NULL;
}

FShader* FShaderType::FindShaderById(const FGuid& Id) const
{
	return ShaderIdMap.FindRef(Id);
}

FShader* FShaderType::ConstructForDeserialization() const
{
	return (*ConstructSerializedRef)();
}

UBOOL FShaderType::CompileShader(EShaderPlatform Platform,const FShaderCompilerEnvironment& InEnvironment,FShaderCompilerOutput& Output,UBOOL bSilent)
{
	// Allow the shader type to modify its compile environment.
	FShaderCompilerEnvironment Environment = InEnvironment;
	(*ModifyCompilationEnvironmentRef)(Environment);

	// Construct shader target for the shader type's frequency and the specified platform.
	FShaderTarget Target;
	Target.Platform = Platform;
	Target.Frequency = Frequency;

	// Compile the shader environment passed in with the shader type's source code.
	return ::CompileShader(
		SourceFilename,
		FunctionName,
		Target,
		Environment,
		Output,
		bSilent
		);
}

FShader* FGlobalShaderType::CompileShader(EShaderPlatform Platform,TArray<FString>& OutErrors)
{
	// Construct the shader environment.
	FShaderCompilerEnvironment Environment;

	//<@ ava specific ; 2007. 7. 25 changmin 추가
	// SM2Poor platform 에서는 hdr integer path를 쓴다....
	//if(	Platform == SP_PCD3D_SM2_POOR )
	if( IsHDRIntegerPath( Platform ) )
	{
		Environment.Definitions.Set(TEXT("HDR_INTEGER_PATH"), TEXT("1"));
	}
	//>@ ava

	// Compile the shader code.
	FShaderCompilerOutput Output;
	if(!FShaderType::CompileShader(Platform,Environment,Output))
	{
		OutErrors = Output.Errors;
		return NULL;
	}

	// Check for shaders with identical compiled code.
	FShader* Shader = FindShaderByCode(Output.Code);
	if(!Shader)
	{
		// Create the shader.
		Shader = (*ConstructCompiledRef)(CompiledShaderInitializerType(this,Output));
	}
	return Shader;
}

FShader::FShader(const FGlobalShaderType::CompiledShaderInitializerType& Initializer):
	Code(Initializer.Code),
	Target(Initializer.Target),
	Type(Initializer.Type),
	NumRefs(0),
	NumInstructions(Initializer.NumInstructions)
{
	Id = appCreateGuid();
	Type->RegisterShader(this);

#if STATS && !XBOX
	if(Target.Frequency == SF_Vertex)
	{
		INC_DWORD_STAT_BY(STAT_VertexShaderMemory,Code.Num());
	}
	else if(Target.Frequency == SF_Pixel)
	{
		INC_DWORD_STAT_BY(STAT_PixelShaderMemory,Code.Num());
	}
#endif
}

FShader::~FShader()
{
#if STATS && !XBOX
	if(Target.Frequency == SF_Vertex)
	{
		DEC_DWORD_STAT_BY(STAT_VertexShaderMemory,Code.Num());
	}
	else if(Target.Frequency == SF_Pixel)
	{
		DEC_DWORD_STAT_BY(STAT_PixelShaderMemory,Code.Num());
	}
#endif
}

/**
* @return the shader's vertex shader
*/
const FVertexShaderRHIRef& FShader::GetVertexShader() 
{ 
	// If the shader resource hasn't been initialized yet, initialize it.
	// In game, shaders are always initialized on load, so this check isn't necessary.
	if(GIsEditor && !IsInitialized())
	{
		Init();
	}

	return VertexShader; 
}

/**
* @return the shader's pixel shader
*/
const FPixelShaderRHIRef& FShader::GetPixelShader() 
{ 
	// If the shader resource hasn't been initialized yet, initialize it.
	// In game, shaders are always initialized on load, so this check isn't necessary.
	if(GIsEditor && !IsInitialized())
	{
		Init();
	}

	return PixelShader; 
}

void FShader::Serialize(FArchive& Ar)
{
	BYTE TargetPlatform = Target.Platform;
	BYTE TargetFrequency = Target.Frequency;
	Ar << TargetPlatform << TargetFrequency;
	Target.Platform = TargetPlatform;
	Target.Frequency = TargetFrequency;

	Ar << Code << Id << Type;
	if(Ar.IsLoading())
	{
		Type->RegisterShader(this);
	}

	if(Ar.Ver() >= VER_SHADER_NUMINSTRUCTIONS)
	{
		Ar << NumInstructions;
	}
}

void FShader::InitRHI()
{
	// we can't have this called on the wrong platform's shaders
	if (Target.Platform != GRHIShaderPlatform)
	{
#if CONSOLE
		appErrorf( TEXT("FShader::Init got platform %i but expected %i"), (INT) Target.Platform, (INT) GRHIShaderPlatform );
#endif
		return;
	}
	check(Code.Num());
	if(Target.Frequency == SF_Vertex)
	{
		VertexShader = RHICreateVertexShader(Code);
	}
	else if(Target.Frequency == SF_Pixel)
	{
		PixelShader = RHICreatePixelShader(Code);
	}
#if CONSOLE && !PS3
	Code.Empty();
#endif
}

void FShader::ReleaseRHI()
{
	VertexShader.Release();
	PixelShader.Release();
}

void FShader::AddRef()
{
	++NumRefs;
}

void FShader::RemoveRef()
{
	if(--NumRefs == 0)
	{
		// Deregister the shader now to eliminate references to it by the type's ShaderIdMap and ShaderCodeMap.
		Type->DeregisterShader(this);

		// Send a release message to the rendering thread when the shader loses its last reference.
		BeginReleaseResource(this);

		BeginCleanup(this);
	}
}

void FShader::FinishCleanup()
{
	delete this;
}

FArchive& operator<<(FArchive& Ar,FShader*& Ref)
{
	if(Ar.IsSaving())
	{
		if(Ref)
		{
			// Serialize the shader's ID and type.
			FGuid ShaderId = Ref->GetId();
			FShaderType* ShaderType = Ref->GetType();
			Ar << ShaderId << ShaderType;
		}
		else
		{
			FGuid ShaderId(0,0,0,0);
			FShaderType* ShaderType = NULL;
			Ar << ShaderId << ShaderType;
		}
	}
	else if(Ar.IsLoading())
	{
		// Deserialize the shader's ID and type.
		FGuid ShaderId;
		FShaderType* ShaderType = NULL;
		Ar << ShaderId << ShaderType;

		Ref = NULL;

		if(ShaderType)
		{
			// Find the shader using the ID and type.
			Ref = ShaderType->FindShaderById(ShaderId);
		}
	}

	return Ar;
}

/**
* Adds this to the list of global bound shader states 
*/
FGlobalBoundShaderStateRHIRef::FGlobalBoundShaderStateRHIRef() :
ShaderRecompileGroup(SRG_GLOBAL_MISC)
{
}

/**
* Initializes a global bound shader state with a vanilla bound shader state and required information.
*/
void FGlobalBoundShaderStateRHIRef::Init(
	const FBoundShaderStateRHIRef& InBoundShaderState, 
	EShaderRecompileGroup InShaderRecompileGroup, 
	FGlobalShaderType* InGlobalVertexShaderType, 
	FGlobalShaderType* InGlobalPixelShaderType)
{
	BoundShaderState = InBoundShaderState;
	ShaderRecompileGroup = InShaderRecompileGroup;

	//make sure that we are only accessing these maps from the rendering thread
	check(IsInRenderingThread());

	//add a shaderType to global bound shader state map so that we can iterate over all
	//global bound shader states and look them up by shaderType
	GetTypeToBoundStateMap().Set(InGlobalVertexShaderType, this);
	GetTypeToBoundStateMap().Set(InGlobalPixelShaderType, this);
}

/**
* Removes this from the list of global bound shader states 
*/
FGlobalBoundShaderStateRHIRef::~FGlobalBoundShaderStateRHIRef()
{
	TMap<FGlobalShaderType*, FGlobalBoundShaderStateRHIRef*>& TypeToBoundStateMap = GetTypeToBoundStateMap();

	//find and remove one of the entries (either the vertex or pixel shader type)
	FGlobalShaderType* const * GlobalShaderType = TypeToBoundStateMap.FindKey(this);
	if (GlobalShaderType && *GlobalShaderType)
	{
		TypeToBoundStateMap.Remove(*GlobalShaderType);

		//find and remove the second entry
		GlobalShaderType = TypeToBoundStateMap.FindKey(this);
		if (GlobalShaderType && *GlobalShaderType)
		{
			TypeToBoundStateMap.Remove(*GlobalShaderType);
		}
	}
}

/**
* A map used to find global bound shader states by global shader type and keep track of loaded global bound shader states
*/
TMap<FGlobalShaderType*, FGlobalBoundShaderStateRHIRef*>& FGlobalBoundShaderStateRHIRef::GetTypeToBoundStateMap()
{
	static TMap<FGlobalShaderType*, FGlobalBoundShaderStateRHIRef*> TypeToBoundStateMap;
	return TypeToBoundStateMap;
}

/**
 * Constructor.
 *
 * @param	PlatformToSerialize		Platform to serialize or SP_AllPlatforms for all
 */
UShaderCache::UShaderCache( EShaderPlatform InPlatform )
:	Platform( InPlatform )
{}

/**
 * Whether the passed in platform should be serialized.
 */
UBOOL UShaderCache::ShouldSerializePlatform( EShaderPlatform Platform )
{
	check(FALSE);
	return FALSE;
}


/**
 * Flushes the shader map for a material from the cache.
 * @param MaterialId - The id of the material.
 * @param Platform - Optional platform to flush. Defaults to all platforms
 */
void UShaderCache::FlushId(const FGuid& MaterialId, EShaderPlatform Platform)
{
	UShaderCache* ShaderCache = GShaderCaches[SC_Local][Platform];
	if (ShaderCache)
	{
		// Remove the shaders cached for this material ID from the shader cache.
		ShaderCache->MaterialShaderMap.Remove(MaterialId);
		// make sure the reference in the map is removed
		ShaderCache->MaterialShaderMap.Shrink();
		ShaderCache->bDirty = TRUE;			
	}
}

/**
 * Adds a material shader map to the cache fragment.
 * @param MaterialShaderIndex - The shader map for the material.
 * @param Platform - The shader platform the map is compiled for.
 */
void UShaderCache::AddMaterialShaderMap(FMaterialShaderMap* InMaterialShaderMap)
{
	MaterialShaderMap.Set(InMaterialShaderMap->GetMaterialId(),InMaterialShaderMap);
	bDirty = TRUE;
}

/**
 * Removes all entries in the cache
 * @param bSetDirty Set this to false if the cache shouldn't be marked dirty
 */
// @GEMINI_TODO: Remove bSetDirty?
void UShaderCache::FlushCache(UBOOL bSetDirty)
{
	// Remove all cached shaders
	for (INT PlatformIndex = 0; PlatformIndex < SP_NumPlatforms; PlatformIndex++)
	{
		UShaderCache* ShaderCache = GShaderCaches[SC_Local][PlatformIndex];
		if (ShaderCache && ShaderCache->MaterialShaderMap.Num())
		{
			// Remove all cached shaders
			ShaderCache->MaterialShaderMap.Empty();
			if (bSetDirty)
			{
				ShaderCache->bDirty = TRUE;
			}
		}

		delete GGlobalShaderMap[PlatformIndex];
		GGlobalShaderMap[PlatformIndex] = NULL;
	}
}

/**
 * Removes all entries in the cache with exceptions based on a platform
 * @param Platform - The shader platform to flush or keep (depending on second param)
 * @param bFlushAllButPlatform - TRUE if all platforms EXCEPT the given should be flush. FALSE will flush ONLY the given platform
 * @param bSetDirty - TRUE if this operation should mark the shader cache as dirty.
 */
void UShaderCache::FlushCacheByPlatform(EShaderPlatform Platform, UBOOL bFlushAllButPlatform, UBOOL bSetDirty)
{
	for (INT PlatformIndex = 0; PlatformIndex < SP_NumPlatforms; PlatformIndex++)
	{
		// determine if this platform should be flushed
		if ((bFlushAllButPlatform && (EShaderPlatform)PlatformIndex != Platform) ||
			(!bFlushAllButPlatform && (EShaderPlatform)PlatformIndex == Platform))
		{
			UShaderCache* ShaderCache = GShaderCaches[SC_Local][PlatformIndex];
			if (ShaderCache && ShaderCache->MaterialShaderMap.Num())
			{
				// Remove all cached shaders
				ShaderCache->MaterialShaderMap.Empty();
			}

			if (ShaderCache && bSetDirty)
			{
				ShaderCache->bDirty = TRUE;
			}

			delete GGlobalShaderMap[PlatformIndex];
			GGlobalShaderMap[PlatformIndex] = NULL;
		}
	}
}

/**
 * Removes all entries in the cache with exceptions based on a shader type
 * @param ShaderType - The shader type to flush or keep (depending on second param)
 * @param bFlushAllButShaderType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given shader type
 */
void UShaderCache::FlushCacheByShaderType(FShaderType* ShaderType, UBOOL bFlushAllButShaderType)
{
	for (INT PlatformIndex = 0; PlatformIndex < SP_NumPlatforms; PlatformIndex++)
	{
		if (ShaderType->GetGlobalShaderType())
		{
			GGlobalShaderMap[PlatformIndex]->RemoveShaderType(ShaderType->GetGlobalShaderType());
		}
		else
		{
			UShaderCache* ShaderCache = GShaderCaches[SC_Local][PlatformIndex];
			if (ShaderCache)
			{
				// flush cached shaders
				for(TDynamicMap<FGuid,TRefCountPtr<FMaterialShaderMap> >::TIterator MaterialIt(ShaderCache->MaterialShaderMap); MaterialIt; ++MaterialIt)
				{
					MaterialIt.Value()->FlushShadersByShaderType(ShaderType, bFlushAllButShaderType);
				}
				// mark the shader cache as dirty
				ShaderCache->bDirty = TRUE;
			}
		}
	}
}

/**
 * Removes all entries in the cache with exceptions based on a vertex factory type
 * @param ShaderType - The shader type to flush or keep (depending on second param)
 * @param bFlushAllButVertexFactoryType - TRUE if all shaders EXCEPT the given type should be flush. FALSE will flush ONLY the given vertex factory type
 */
void UShaderCache::FlushCacheByVertexFactoryType(FVertexFactoryType* VertexFactoryType, UBOOL bFlushAllButVertexFactoryType)
{
	for (INT PlatformIndex = 0; PlatformIndex < SP_NumPlatforms; PlatformIndex++)
	{
		UShaderCache* ShaderCache = GShaderCaches[SC_Local][PlatformIndex];
		if (ShaderCache)
		{
			// flush cached shaders
			for(TDynamicMap<FGuid,TRefCountPtr<FMaterialShaderMap> >::TIterator MaterialIt(ShaderCache->MaterialShaderMap); MaterialIt; ++MaterialIt)
			{
				MaterialIt.Value()->FlushShadersByVertexFactoryType(VertexFactoryType, bFlushAllButVertexFactoryType);
			}
			// mark the shader cache as dirty
			ShaderCache->bDirty = TRUE;
		}
	}
}


void UShaderCache::FinishDestroy()
{
	Super::FinishDestroy();
	for (INT Index = 0; Index < SC_NumShaderCacheTypes; Index++)
	{
		for( INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; PlatformIndex++ )
		{
			UShaderCache* ShaderCache = GShaderCaches[Index][PlatformIndex];
			if (ShaderCache == this)
			{
				// The shader cache is a root object, but it will still be purged on exit.  Make sure there isn't a dangling reference to it.
				GShaderCaches[Index][PlatformIndex] = NULL;
			}
		}
	}
}

IMPLEMENT_COMPARE_CONSTREF(FGuid,SortMaterialsByGuid,
{
	for ( INT i = 0; i < 4; i++ )
	{
		if ( A[i] > B[i] )
		{
			return 1;
		}
		else if ( A[i] < B[i] )
		{
			return -1;
		}
	}

	return 0;
})

/**
 * Presave function. Gets called once before an object gets serialized for saving.  Sorts the MaterialShaderMap
 * maps so that shader cache serialization is deterministic.
 */
void UShaderCache::PreSave()
{
	Super::PreSave();

	for (INT PlatformIndex = 0; PlatformIndex < SP_NumPlatforms; PlatformIndex++)
	{
		MaterialShaderMap.KeySort<COMPARE_CONSTREF_CLASS(FGuid,SortMaterialsByGuid)>();
	}
}

void UShaderCache::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar << Platform;	

	if(Ar.IsSaving())
	{
		// the reference shader should never be saved (when it's being created, it's actually the local shader cache for that run of the commandlet)
		check(this != GShaderCaches[SC_Reference][Platform]);

		// Find the shaders used by materials in the cache.
		TMap<FGuid,FShader*> Shaders;

		for(TDynamicMap<FGuid,TRefCountPtr<FMaterialShaderMap> >::TIterator MaterialIt(MaterialShaderMap);MaterialIt;++MaterialIt)
		{
			MaterialIt.Value()->GetShaderList(Shaders);
		}
		GetGlobalShaderMap((EShaderPlatform)Platform)->GetShaderList(Shaders);
			
		// Save the shaders in the cache.
		INT NumShaders = Shaders.Num();
		Ar << NumShaders;
		for(TMap<FGuid,FShader*>::TIterator ShaderIt(Shaders);ShaderIt;++ShaderIt)
		{
			FShader* Shader = ShaderIt.Value();

			// Serialize the shader type and ID separately, so at load time we can see whether this is a redundant shader without fully
			// deserializing it.
			FShaderType* ShaderType = Shader->GetType();
			FGuid ShaderId = Shader->GetId();
			Ar << ShaderType << ShaderId;

			// Write a placeholder value for the skip offset.
			INT SkipOffset = Ar.Tell();
			Ar << SkipOffset;

			// Serialize the shader.
			Shader->Serialize(Ar);

			// Write the actual offset of the end of the shader data over the placeholder value written above.
			INT EndOffset = Ar.Tell();
			Ar.Seek(SkipOffset);
			Ar << EndOffset;
			Ar.Seek(EndOffset);
		}

		INT NumMaterialShaderMaps = MaterialShaderMap.Num();
		Ar << NumMaterialShaderMaps;
		for(TDynamicMap<FGuid,TRefCountPtr<FMaterialShaderMap> >::TIterator MaterialIt(MaterialShaderMap);MaterialIt;++MaterialIt)
		{
			// Serialize the material ID separate, so at load time we can see whether this is a redundant material shader map without fully
			// deserializing it.
			FGuid MaterialId = MaterialIt.Key();
			Ar << MaterialId;

			// Write a placeholder value for the skip offset.
			INT SkipOffset = Ar.Tell();
			Ar << SkipOffset;

			// Serialize the material shader map.
			MaterialIt.Value()->Serialize(Ar);

			// Write the actual offset of the end of the material shader map data over the placeholder value written above.
			INT EndOffset = Ar.Tell();
			Ar.Seek(SkipOffset);
			Ar << EndOffset;
			Ar.Seek(EndOffset);
		}		
		
		// Save the global shader map
		check( ARRAY_COUNT(GGlobalShaderMap) == SP_NumPlatforms );
		GetGlobalShaderMap((EShaderPlatform)Platform)->Serialize(Ar);		

		// Mark the cache as not dirty.
		bDirty = FALSE;
	}
	else if(Ar.IsLoading())
	{
		INT NumShaders			= 0;
		INT NumRedundantShaders = 0;
		INT NumLegacyShaders	= 0;		

		Ar << NumShaders;

		// Load the shaders in the cache.
		for(INT ShaderIndex = 0;ShaderIndex < NumShaders;ShaderIndex++)
		{
			// Deserialize the shader type and shader ID.
			FShaderType* ShaderType = NULL;
			FGuid ShaderId;
			Ar << ShaderType << ShaderId;

			// Deserialize the offset of the next shader.
			INT SkipOffset = 0;
			Ar << SkipOffset;

			if(!ShaderType)
			{
				// If the shader type doesn't exist anymore, skip the shader.
				Ar.Seek(SkipOffset);
				NumLegacyShaders++;
			}
			else
			{
				FShader* Shader = ShaderType->FindShaderById(ShaderId);
				if(Shader)
				{
					// If a shader with the same type and ID is already resident, skip this shader.
					Ar.Seek(SkipOffset);
					NumRedundantShaders++;
				}
				else if(Ar.Ver() < ShaderType->GetMinPackageVersion() || Ar.LicenseeVer() < ShaderType->GetMinLicenseePackageVersion())
				{
					// If the shader type's serialization is compatible with the version the shader was saved in, skip it.
					Ar.Seek(SkipOffset);
					NumLegacyShaders++;
				}
				else
				{
					// Create a new instance of the shader type.
					Shader = ShaderType->ConstructForDeserialization();

					// Deserialize the shader into the new instance.
					Shader->Serialize(Ar);
				}
			}
		}
		
		
		INT NumMaterialShaderMaps = 0;
		Ar << NumMaterialShaderMaps;

		// Load the material shader indices in the cache.		
		for(INT MaterialIndex = 0;MaterialIndex < NumMaterialShaderMaps;MaterialIndex++)
		{
			// Deserialize the material ID.
			FGuid MaterialId;
			Ar << MaterialId;

			// Deserialize the offset of the next material.
			INT SkipOffset = 0;
			Ar << SkipOffset;

			FMaterialShaderMap* MaterialShaderIndex = FMaterialShaderMap::FindId(MaterialId, (EShaderPlatform)Platform);
			if(MaterialShaderIndex)
			{
				// If a shader with the same type and ID is already resident, skip this shader.
				Ar.Seek(SkipOffset);
			}
			else
			{
				// Deserialize the material shader map.
				MaterialShaderIndex = new FMaterialShaderMap();
				MaterialShaderIndex->Serialize(Ar);
			}

			// Add a reference to the shader from the cacheIf yo.
			// This ensures that the shader isn't deleted between the cache being deserialized and PostLoad being called on materials
			// in the same package.
			MaterialShaderMap.Set(MaterialId,MaterialShaderIndex);
		}	

		// Load the global shader map, but ONLY in the local shader cache				
		TShaderMap<FGlobalShaderType>* NewGlobalShaderMap = new TShaderMap<FGlobalShaderType>();
		NewGlobalShaderMap->Serialize(Ar);

#if USE_SEEKFREE_LOADING
		// Only load once.
		if (GGlobalShaderMap[Platform] == NULL )
#else
		// only use the local one
		if (GSerializingLocalShaderCache)
#endif
		{
			delete GGlobalShaderMap[Platform];
			GGlobalShaderMap[Platform] = NewGlobalShaderMap;
#if USE_SEEKFREE_LOADING
			// Initialize global shaders now. They are only loaded once on consoles.
			GGlobalShaderMap[Platform]->BeginInit();
#endif
		}
		else
		{
			delete NewGlobalShaderMap;
		}			

		// Log some cache stats.
		debugf(
			TEXT("Loaded shader cache %s: %u shaders(%u legacy, %u redundant), %u materials"),
			*GetPathName(),
			NumShaders,
			NumLegacyShaders,
			NumRedundantShaders,
			NumMaterialShaderMaps
			);
	}

	if( Ar.IsCountingMemory() )
	{
		MaterialShaderMap.CountBytes( Ar );
		for( TDynamicMap<FGuid,TRefCountPtr<class FMaterialShaderMap> >::TIterator It(MaterialShaderMap); It; ++It )
		{
			It.Value()->Serialize( Ar );
		}
	}
}

/**
 * Dumps shader stats to the log.
 */
void DumpShaderStats()
{
	// Iterate over all shader types and log stats.
	INT TotalShaderCount	= 0;
	INT TotalTypeCount		= 0;
	debugf(TEXT("Shader dump:"));
	for( TLinkedList<FShaderType*>::TIterator It(FShaderType::GetTypeList()); It; It.Next() )
	{
		const FShaderType* Type = *It;
		debugf(TEXT("%7i shaders of type '%s'"), Type->GetNumShaders(), Type->GetName());
		TotalShaderCount += Type->GetNumShaders();
		TotalTypeCount++;
	}
	debugf(TEXT("%i shaders spread over %i types"),TotalShaderCount,TotalTypeCount);
}

#if !USE_SEEKFREE_LOADING

extern UBOOL GOptimizingShaderForGameRuntime;

FString GetLocalShaderCacheFilename( EShaderPlatform Platform )
{
	if (GOptimizingShaderForGameRuntime)
		return FString::Printf(TEXT("%s%s%sGameLocalSC-%s.upk"),*appGameDir(),TEXT("Content"),PATH_SEPARATOR,ShaderPlatformToText(Platform));
	else
		return FString::Printf(TEXT("%s%s%sLocalShaderCache-%s.upk"),*appGameDir(),TEXT("Content"),PATH_SEPARATOR,ShaderPlatformToText(Platform));
}

FString GetReferenceShaderCacheFilename( EShaderPlatform Platform )
{
	if (GOptimizingShaderForGameRuntime)
		return FString::Printf(TEXT("%s%s%sGameRefSC-%s.upk"),*appGameDir(),TEXT("Content"),PATH_SEPARATOR,ShaderPlatformToText(Platform));
	else
		return FString::Printf(TEXT("%s%s%sRefShaderCache-%s.upk"),*appGameDir(),TEXT("Content"),PATH_SEPARATOR,ShaderPlatformToText(Platform));
}

static void LoadShaderCaches( EShaderPlatform Platform )
{
	//initialize GRHIShaderPlatform based on the current system's capabilities
	RHIInitializeShaderPlatform();

	for (INT Index = 0; Index < SC_NumShaderCacheTypes; Index++)
	{
		UShaderCache*& ShaderCacheRef = GShaderCaches[Index][Platform];
		//check for reentry
		check(ShaderCacheRef == NULL);		

		// mark that we are serializing the local shader cache, so we can load the global shaders
		GSerializingLocalShaderCache = Index == SC_Local;	
		
		// by default, we have no shadercache
		ShaderCacheRef = NULL;

		// only look for the shader cache object if the package exists (avoids a throw inside the code)
		FString Filename;
		if (GPackageFileCache->FindPackageFile((Index == SC_Local ? *GetLocalShaderCacheFilename(Platform) : *GetReferenceShaderCacheFilename(Platform)), NULL, Filename))
		{
			// if another instance is writing the shader cache while we are reading, then opening will fail, and 
			// that's not good, so retry until we can open it
#if !CONSOLE
			// this "lock" will make sure that another process can't be writing to the package before we actually 
			// read from it with LoadPackage, etc below
			FArchive* ReaderLock = NULL;
			// try to open the shader cache for 
			DOUBLE StartTime = appSeconds();
			const DOUBLE ShaderRetryDelaySeconds = 15.0;

			// try until we can read the file, or until ShaderRetryDelaySeconds has passed
			while (ReaderLock == NULL && appSeconds() - StartTime < ShaderRetryDelaySeconds)
			{
				ReaderLock = GFileManager->CreateFileReader(*Filename);
				if(!ReaderLock)
				{
					// delay a bit
					appSleep(1.0f);
				}
			}
#endif
			UBOOL bSkipLoading = FALSE;
			if( Index == SC_Local )
			{
				// This function is being called during script compilation, which is why we need to use LOAD_FindIfFail.
				UObject::BeginLoad();
				ULinkerLoad* Linker = UObject::GetPackageLinker( NULL, *Filename, LOAD_NoWarn | LOAD_FindIfFail, NULL, NULL );
				UObject::EndLoad();
				// Skip loading the local shader cache if it was built with an old version.
				if( Linker && Linker->Summary.EngineVersion != GEngineVersion )
				{
					bSkipLoading = TRUE;
				}
			}

			// Skip loading the shader cache if wanted.
			if( !bSkipLoading )
			{
				// This function is being called during script compilation, which is why we need to use LOAD_FindIfFail.
				UPackage* ShaderCachePackage = UObject::LoadPackage( NULL, *Filename, LOAD_NoWarn | LOAD_FindIfFail );
				if( ShaderCachePackage )
				{
					ShaderCacheRef = FindObject<UShaderCache>( ShaderCachePackage, TEXT("CacheObject") );
				}
			}
#if !CONSOLE
			delete ReaderLock;
#endif
		}

		if(!ShaderCacheRef)
		{
			// if we didn't find the local shader cache, create it. if we don't find the refshadercache, that's okay, just leave it be
			if (Index == SC_Local)
			{
				// If the local shader cache couldn't be loaded, create an empty cache.				
				FString LocalShaderCacheName = FString(TEXT("LocalShaderCache-")) + ShaderPlatformToText(Platform);
				ShaderCacheRef = new(UObject::CreatePackage(NULL,*LocalShaderCacheName),TEXT("CacheObject")) UShaderCache(Platform);
				ShaderCacheRef->MarkPackageDirty(FALSE);
			}
		}
		// if we found it, make sure it's loaded
		else
		{
			// if this function was inside a BeginLoad()/EndLoad(), then the LoadObject above didn't actually serialize it, this will
			ShaderCacheRef->GetLinker()->Preload(ShaderCacheRef);
		}

		if (ShaderCacheRef)
		{
			// make sure it's not GC'd
			ShaderCacheRef->AddToRoot();
		}

		GSerializingLocalShaderCache = FALSE;
	}

	// Ensure that the global shader map contains all global shader types.
	VerifyGlobalShaders();
}

/**
 * Makes sure all global shaders are loaded and/or compiled
 *
 * @platform Platform to verify global shaders for
 */
void VerifyGlobalShaders(EShaderPlatform Platform)
{
	// Ensure local shader cache for this platform has been loaded if present.
	GetLocalShaderCache(Platform);

	// Ensure that the global shader map contains all global shader types.
	TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(Platform);
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		FGlobalShaderType* GlobalShaderType = ShaderTypeIt->GetGlobalShaderType();
		if(GlobalShaderType && GlobalShaderType->ShouldCache(Platform))
		{
			if(!GlobalShaderMap->HasShader(GlobalShaderType))
			{
				// Compile this global shader type.
				TArray<FString> ShaderErrors;
				FShader* Shader = GlobalShaderType->CompileShader(Platform,ShaderErrors);
				if(Shader)
				{
					// Add the new global shader instance to the global shader map.
					// This will cause FShader::AddRef to be called, which will cause BeginInitResource(Shader) to be called.
					GlobalShaderMap->AddShader(GlobalShaderType,Shader);

					// make sure the shader cache is saved out
					GetLocalShaderCache(Platform)->MarkDirty();
				}
				else
				{
					appErrorf(TEXT("Failed to compile global shader %s"), GlobalShaderType->GetName());
				}
			}
		}
	}
	GGlobalShaderMap[Platform]->BeginInit();
}

UShaderCache* GetLocalShaderCache( EShaderPlatform Platform )
{
	if (!GShaderCaches[SC_Local][Platform])
	{
		LoadShaderCaches( Platform );
	}

	return GShaderCaches[SC_Local][Platform];
}

/**
* Saves the local shader cache for the passed in platform.
*
* @param	Platform	Platform to save shader cache for.
* @param	OverrideCacheFilename If non-NULL, then the shader cache will be saved to the given path
*/
void SaveLocalShaderCache(EShaderPlatform Platform, const TCHAR* OverrideCacheFilename)
{
#if FINAL_RELEASE
	return;
#endif

	// Only save the shader cache for the first instance running.
	if( GIsFirstInstance )
	{
		UShaderCache* ShaderCache = GShaderCaches[SC_Local][Platform];
		if (ShaderCache && ShaderCache->IsDirty())
		{
			// Reset the LinkerLoads for all shader caches, since we may be saving the local shader cache over the refshadercache file.
			for(INT TypeIndex = 0;TypeIndex < SC_NumShaderCacheTypes;TypeIndex++)
			{
				if(GShaderCaches[TypeIndex][Platform])
				{
					UObject::ResetLoaders(GShaderCaches[TypeIndex][Platform]);
				}
			}

			UPackage* ShaderCachePackage = ShaderCache->GetOutermost();

			// the shader cache isn't network serializable
			ShaderCachePackage->PackageFlags |= PKG_ServerSideOnly;

			// See if there is any other packages for the local shader cache
			FFilename Filename = GetLocalShaderCacheFilename(Platform);
			
			// epic bug fix :)
			if (OverrideCacheFilename)
			{
				Filename = OverrideCacheFilename;
			}			

			UObject::SavePackage(ShaderCachePackage, ShaderCache, 0, OverrideCacheFilename ? OverrideCacheFilename : *Filename);

			// mark it as clean, as its been saved!
			ShaderCache->MarkClean();
		}
	}
	else
	{
		// Only warn once.
		static UBOOL bAlreadyWarned = FALSE;
		if( !bAlreadyWarned )
		{
			bAlreadyWarned = TRUE;
			debugf( NAME_Warning, TEXT("Skipping saving the shader cache as another instance of the game is running.") );
		}
	}
}

#endif	//#if !USE_SEEKFREE_LOADING

extern void SaveLocalShaderCaches()
{
	for( INT PlatformIndex=0; PlatformIndex<SP_NumPlatforms; PlatformIndex++ )
	{
		SaveLocalShaderCache( (EShaderPlatform)PlatformIndex );
	}
}

TShaderMap<FGlobalShaderType>* GetGlobalShaderMap(EShaderPlatform Platform)
{
	if(!GGlobalShaderMap[Platform])
	{
		GGlobalShaderMap[Platform] = new TShaderMap<FGlobalShaderType>();
	}
	return GGlobalShaderMap[Platform];
}

/**
* Forces a recompile of the global shaders.
*/
void RecompileGlobalShaders()
{
#if !USE_SEEKFREE_LOADING
	// Flush pending accesses to the existing global shaders.
	FlushRenderingCommands();

	// Get rid of the existing global shaders.
	if(GGlobalShaderMap[GRHIShaderPlatform])
	{
		delete GGlobalShaderMap[GRHIShaderPlatform];
	}

	// Compile new global shaders.
	GGlobalShaderMap[GRHIShaderPlatform] = new TShaderMap<FGlobalShaderType>();
	VerifyGlobalShaders(GRHIShaderPlatform);
#endif
}

/**
* Recompiles the specified global shader types, and flushes their bound shader states.
*/
void RecompileGlobalShaders(const TArray<FShaderType*>& OutdatedShaderTypes)
{
#if USE_SEEKFREE_LOADING
	return;
#endif
	/*
	if( !GUseSeekFreeLoading )
	*/
	{
		// Flush pending accesses to the existing global shaders.
		FlushRenderingCommands();

		TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(GRHIShaderPlatform);

		for (INT TypeIndex = 0; TypeIndex < OutdatedShaderTypes.Num(); TypeIndex++)
		{
			FGlobalShaderType* CurrentGlobalShaderType = OutdatedShaderTypes(TypeIndex)->GetGlobalShaderType();
			if (CurrentGlobalShaderType)
			{
				debugf(TEXT("Flushing Global Shader %s"), CurrentGlobalShaderType->GetName());
				GlobalShaderMap->RemoveShaderType(CurrentGlobalShaderType);
				FGlobalBoundShaderStateRHIRef* CurrentBoundShaderState = FGlobalBoundShaderStateRHIRef::GetTypeToBoundStateMap().FindRef(CurrentGlobalShaderType);
				if (CurrentBoundShaderState)
				{
					debugf(TEXT("Invalidating Bound Shader State"));
					CurrentBoundShaderState->Invalidate();
				}
			}
		}

		VerifyGlobalShaders(GRHIShaderPlatform);
	}
}

/**
* Forces a recompile of only the specified group of global shaders. 
* Also invalidates global bound shader states so that the new shaders will be used.
*/
void RecompileGlobalShaderGroup(EShaderRecompileGroup FlushGroup)
{
#if USE_SEEKFREE_LOADING
	return;
#endif
	/*
	if( !GUseSeekFreeLoading )
	*/
	{
		// Flush pending accesses to the existing global shaders.
		FlushRenderingCommands();

		TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(GRHIShaderPlatform);
		TMap<FGuid,FShader*> GlobalShaderList;
		GlobalShaderMap->GetShaderList(GlobalShaderList);
		for(TMap<FGuid,FShader*>::TIterator ShaderIt(GlobalShaderList);ShaderIt;++ShaderIt)
		{
			FShader* CurrentGlobalShader = ShaderIt.Value();
			if (CurrentGlobalShader->GetRecompileGroup() == FlushGroup)
			{
				FShaderType* CurrentShaderType = CurrentGlobalShader->GetType();
				FGlobalShaderType* CurrentGlobalShaderType = CurrentShaderType->GetGlobalShaderType();
				check(CurrentGlobalShaderType);
				debugf(TEXT("Flushing Global Shader %s"), CurrentGlobalShaderType->GetName());
				GlobalShaderMap->RemoveShaderType(CurrentGlobalShaderType);
			}
		}

		debugf(TEXT("Flushing Global Bound Shader States..."));

		TArray<FGlobalBoundShaderStateRHIRef*> GlobalBoundShaderStates;
		FGlobalBoundShaderStateRHIRef::GetTypeToBoundStateMap().GenerateValueArray(GlobalBoundShaderStates);
		//invalidate global bound shader states so they will be created with the new shaders the next time they are set (in SetGlobalBoundShaderState)
		for (INT TypeIndex = 0; TypeIndex < GlobalBoundShaderStates.Num(); TypeIndex++)
		{
			FGlobalBoundShaderStateRHIRef* CurrentGlobalBoundShaderState = GlobalBoundShaderStates(TypeIndex);
			if (CurrentGlobalBoundShaderState && CurrentGlobalBoundShaderState->ShaderRecompileGroup == FlushGroup)
			{
				CurrentGlobalBoundShaderState->Invalidate();
			}
		}

		VerifyGlobalShaders(GRHIShaderPlatform);
	}
}


#if PS3
/**
 * Set a Linear Color parameter (used to swaps Red/Blue too)
 */
// @GEMINI_TODO: Make sure this is never needed, then remove
template<>
void SetPixelShaderValue(
	FCommandContextRHI* Context,
	FPixelShaderRHIParamRef PixelShader,
	const FShaderParameter& Parameter,
	const FLinearColor& Value,
	UINT ElementIndex
	)
{
	if(Parameter.IsBound())
	{
		check((ElementIndex + 1) <= Parameter.GetNumRegisters());
		FLinearColor Temp(Value.R, Value.G, Value.B, Value.A);
		RHISetPixelShaderParameter(
			Context,
			PixelShader,
			Parameter.GetBaseRegisterIndex() + ElementIndex,
			1,
			(FLOAT*)&Temp
			);
	}
}
#endif

//
FShaderType* FindShaderTypeByName(const TCHAR* ShaderTypeName)
{
	for(TLinkedList<FShaderType*>::TIterator ShaderTypeIt(FShaderType::GetTypeList());ShaderTypeIt;ShaderTypeIt.Next())
	{
		if(!appStricmp(ShaderTypeIt->GetName(),ShaderTypeName))
		{
			return *ShaderTypeIt;
		}
	}
	return NULL;
}

/**
* SetGlobalBoundShaderState - sets the global bound shader state, also creates and caches it if necessary
*
* @param Context - command buffer context
* @param BoundShaderState - current bound shader state, will be updated if it wasn't a valid ref
* @param VertexDeclaration - the vertex declaration to use in creating the new bound shader state
* @param VertexShader - the vertex shader to use in creating the new bound shader state
* @param PixelShader - the pixel shader to use in creating the new bound shader state
* @param Stride
*/
void SetGlobalBoundShaderState(
							   FCommandContextRHI* Context,
							   FGlobalBoundShaderStateRHIRef &GlobalBoundShaderState,
							   FVertexDeclarationRHIParamRef VertexDeclaration,
							   FShader* VertexShader,
							   FShader* PixelShader,
							   UINT Stride)
{
	if( !IsValidRef(GlobalBoundShaderState.BoundShaderState))
	{
		DWORD Strides[MaxVertexElementCount];
		appMemzero(Strides, sizeof(Strides));
		Strides[0] = Stride;

		GlobalBoundShaderState.Init(
			RHICreateBoundShaderState(VertexDeclaration, Strides, VertexShader->GetVertexShader(), PixelShader->GetPixelShader()),
			PixelShader->GetRecompileGroup(),
			VertexShader->GetType()->GetGlobalShaderType(),
			PixelShader->GetType()->GetGlobalShaderType());
	}
	RHISetBoundShaderState(Context, GlobalBoundShaderState.BoundShaderState);
}