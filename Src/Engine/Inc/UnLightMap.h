/*=============================================================================
	UnLightMap.h: Light-map definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** The number of coefficients which the light-map stores for each color component. */ 
#define NUM_LIGHTMAP_COEFFICIENTS 3

//<@ ava specific ; 2007. 1. 19 changmin
#define NUM_AVA_LIGHTMAPS	4
//>@ ava

//@BEGIN deif ; 여러 lighting 방식을 지원하기 위해 policy로 확장
struct FOldLightmapPolicy
{
	enum
	{
		PolicyIndex = 0,
		IndexOffset = 0,
		NumTextures = 4,
	};
};

struct FSunExcludedLightmapPolicy
{
	enum
	{
		PolicyIndex = 1,
		IndexOffset = 4,
		NumTextures = 4,
	};
};

enum
{
	NUM_LIGHTMAP_POLICES = 2,
	NUM_ALL_LIGHTMAP_TEXTURES = FOldLightmapPolicy::NumTextures + FSunExcludedLightmapPolicy::NumTextures
};
//@END deif

class FTextureLayout;

/**
 * The abstract base class of 1D and 2D light-maps.
 */
class FLightMap : private FDeferredCleanupInterface
{
public:

	/** The GUIDs of lights which this light-map stores. */
	TArray<FGuid> LightGuids;

	//<@ ava specific ; 2007. 12. 21 changmin
	// add cascaded shadow
	UBOOL bSupportsCascadedShadow;
	//>@ ava

	/** Default constructor. */
	FLightMap(): NumRefs(0), bSupportsCascadedShadow(FALSE)	//<@ ava specific ; 2007. 12. 21 changmin
		{}

	//<@ ava specific ; 2007. 12. 21 changmin
	FLightMap( UBOOL bInSupportsCascadedShadow ) : NumRefs(0), bSupportsCascadedShadow(bInSupportsCascadedShadow ) {}
	//>@ ava

	/** Destructor. */
	virtual ~FLightMap() { check(!NumRefs); }

	/**
	 * Checks if a light is stored in this light-map.
	 * @param	LightGuid - The GUID of the light to check for.
	 * @return	True if the light is stored in the light-map.
	 */
	UBOOL ContainsLight(const FGuid& LightGuid) const
	{
		return LightGuids.FindItemIndex(LightGuid) != INDEX_NONE;
	}

	// FLightMap interface.
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray ) {}
	virtual void Serialize(FArchive& Ar);
	virtual void InitResources() {}

	/**
	 * Checks whether the light-map is ready to render.  Light-map textures aren't packed immediately during rebuild, so a LightMap may not
	 * be valid immediately after allocation.
	 */
	virtual UBOOL IsValid() const { return TRUE; }

	// Runtime type casting.
	virtual const class FLightMap1D* GetLightMap1D() const { return NULL; }
	virtual const class FLightMap2D* GetLightMap2D() const { return NULL; }



	// Reference counting.
	void AddRef()
	{
		NumRefs++;
	}
	void RemoveRef()
	{
		if(--NumRefs == 0)
		{
			Cleanup();
		}
	}

protected:

	/**
	 * Called when the light-map is no longer referenced.  Should release the lightmap's resources.
	 */
	virtual void Cleanup()
	{
		BeginCleanup(this);
	}

private:
	INT NumRefs;
	
	// FDeferredCleanupInterface
	virtual void FinishCleanup();
};

/** A reference to a light-map. */
typedef TRefCountPtr<FLightMap> FLightMapRef;

/** Lightmap reference serializer */
extern FArchive& operator<<(FArchive& Ar,FLightMap*& R);

/** The light incident for a point on a surface. */
struct FLightSample
{
	///** The lighting coefficients, colored. */
	//FLOAT Coefficients[NUM_LIGHTMAP_COEFFICIENTS][3];
	//<@ ava specific ; 2007. 1. 19 changmin
	FLOAT Coefficients[NUM_AVA_LIGHTMAPS][3];
	//>@ ava

	/** True if this sample maps to a valid point on a surface. */
	UBOOL bIsMapped;

	/** Initialization constructor. */
	FLightSample():
		bIsMapped(FALSE)
	{
		appMemzero(Coefficients,sizeof(Coefficients));
	}

	/**
	 * Constructs a light sample representing a point light.
	 * @param Color - The color/intensity of the light at the sample point.
	 * @param Direction - The direction toward the light at the sample point.
	 */
	FLightSample(const FLinearColor& Color,const FVector& Direction);

	//!{ 2006-04-10	허 창 민
	void AddLight(const FLinearColor& Color, INT BumpIndex);
	//!} 2006-04-10	허 창 민

	/**
	 * Adds a weighted light sample to this light sample.
	 * @param OtherSample - The sample to add.
	 * @param Weight - The weight to multiply the other sample by before addition.
	 */
	void AddWeighted(const FLightSample& OtherSample,FLOAT Weight);

	friend FArchive& operator<<(FArchive& Ar, FLightSample& S)
	{
		//<@ ava specific ; 2007. 1. 19 changmin
		if( Ar.LicenseeVer() < VER_AVA_NEEDSBUMPEDLIGHTMAP )
		{
			// load old data
			FLOAT Coefficients[NUM_LIGHTMAP_COEFFICIENTS][3];
			Ar.Serialize(Coefficients, sizeof(Coefficients) );

			// compute diffuse lightsample
			appMemzero( S.Coefficients, sizeof(S.Coefficients) );
			for( INT Index = 0; Index < 3; ++Index )
			{
				S.Coefficients[0][0] += Coefficients[Index][0];
				S.Coefficients[0][1] += Coefficients[Index][1];
				S.Coefficients[0][2] += Coefficients[Index][2];
			}
			S.Coefficients[0][0] /= 3.0f;
			S.Coefficients[0][1] /= 3.0f;
			S.Coefficients[0][2] /= 3.0f;

			// copy coefficient lightsample
			appMemcpy( &S.Coefficients[1], Coefficients, sizeof(Coefficients));

			Ar << S.bIsMapped;
		}
		else
		//>@ ava
		{
			Ar.Serialize(S.Coefficients,sizeof(S.Coefficients));
			Ar << S.bIsMapped;
		}

		return Ar;
	}
};

//!{ 2006-05-09	허 창 민
// deprecated !!! 없어질 것이니.. 굳이 4개의 lightmap을 사용하는 업데이트에 포함하지 않는다~~~~~
/**
* The light incident for a point on a surface.
*/
struct FLightSample2
{
	/** The lighting coefficients, colored. */
	FLOAT Coefficients[3];

	/** True if this sample maps to a valid point on a surface. */
	UBOOL IsMapped;

	/**
	* Adds the contribution of light coming from a specific direction.
	* @param	Color - The color/intensity of the light.
	*/
	void AddLight(const FLinearColor& Color);

	/**
	* Serializer
	*/
	friend FArchive& operator<<(FArchive& Ar, FLightSample2& S)
	{
		Ar.Serialize(S.Coefficients,sizeof(S.Coefficients));
		Ar << S.IsMapped;
		return Ar;
	}
};
//!} 2006-05-09	허 창 민


/**
 * The raw data which is used to construct a 2D light-map.
 */
class FLightMapData2D
{
public:

	/** The lights which this light-map stores. */
	TArray<ULightComponent*> Lights;

	/**
	 * Minimal initialization constructor.
	 */
	FLightMapData2D(UINT InSizeX,UINT InSizeY):
		SizeX(InSizeX),
		SizeY(InSizeY)
	{
		Data.Empty(SizeX * SizeY);
		Data.AddZeroed(SizeX * SizeY);
	}

	// Accessors.
	const FLightSample& operator()(UINT X,UINT Y) const { return Data(SizeX * Y + X); }
	FLightSample& operator()(UINT X,UINT Y) { return Data(SizeX * Y + X); }
	UINT GetSizeX() const { return SizeX; }
	UINT GetSizeY() const { return SizeY; }

	//!{ 2006-05-08	허 창 민
	friend FArchive& operator<<(FArchive& Ar,FLightMapData2D& LightMap)
	{
		return Ar << LightMap.Data << LightMap.SizeX << LightMap.SizeY;
	}
	//!} 2006-05-08	허 창 민

private:

	/** The incident light samples for a 2D array of points on the surface. */
	TArray<FLightSample> Data;

	/** The width of the light-map. */
	UINT SizeX;

	/** The height of the light-map. */
	UINT SizeY;
};




/**
 * A 2D texture containing lightmap coefficients.
 */
class ULightMapTexture2D: public UTexture2D
{
	DECLARE_CLASS(ULightMapTexture2D,UTexture2D,CLASS_Intrinsic,Engine)

	// UObject interface.
	virtual void Serialize( FArchive& Ar );

	/** 
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc();

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT Index );
};

/**
 * A 2D array of incident lighting data.
 */
class FLightMap2D : public FLightMap
{
public:
	static UBOOL NeedsBumpedLightmap(UMaterialInstance* InMaterial)
	{
		const FMaterial *Material = InMaterial ? InMaterial->GetInstanceInterface(FALSE)->GetMaterial() : NULL;
		UBOOL bNeedsBump = !Material || Material->NeedsBumpedLightmap();

		return bNeedsBump;
	}

	FLightMap2D() {}

	// FLightMap2D interface.

	/**
	 * Returns the texture containing the RGB coefficients for a specific basis.
	 * @param	BasisIndex - The basis index.
	 * @param	OutScale - The scale to apply to the coefficients of the texture.
	 * @return	The RGB coefficient texture.
	 */
	const UTexture2D* GetTexture(UINT BasisIndex) const;

	// FLightMap interface.
	virtual void AddReferencedObjects( TArray<UObject*>& ObjectArray );
	virtual void Serialize(FArchive& Ar);

//@EDIT deif ; support lightmap policy 
	virtual UBOOL IsValid() const
	{		
		return AreLightmapsValid<FOldLightmapPolicy>() && (!bSupportsCascadedShadow || AreLightmapsValid<FSunExcludedLightmapPolicy>());		
	}

	// Runtime type casting.
	virtual const FLightMap2D* GetLightMap2D() const { return this; }

	// Accessors.	
	const FVector2D& GetCoordinateScale() const { return CoordinateScale; }
	const FVector2D& GetCoordinateBias() const { return CoordinateBias; }

	/**
	 * Allocates texture space for the light-map and stores the light-map's raw data for deferred encoding.
	 * If the light-map has no lights in it, it will return NULL.
	 * @param	LightMapOuter - The package to create the light-map and textures in.
	 * @param	InRawData - The raw light-map data to fill the texture with.
	 * @param	Material - The material which the light-map will be rendered with.  Used as a hint to pack light-maps for the same material in the same texture.  Can be NULL.
	 * @param	Bounds - The bounds of the primitive the light-map will be rendered on.  Used as a hint to pack light-maps on nearby primitives in the same texture.
	 */
	//<@ ava sepcific ; 2007. 12. 3. changmin
	// realtime sun shadow를 위해 확장. RawData = Lightmap, RawData2 = Lightmap without sun
	//static class FLightMap2D* AllocateLightMap(UObject* LightMapOuter,FLightMapData2D* RawData,UMaterialInstance* Material,const FBoxSphereBounds& Bounds);
	static class FLightMap2D* AllocateLightMap(UObject* LightMapOuter,FLightMapData2D* RawData, FLightMapData2D* RawData2, UBOOL bNeedsBumpedLightmap,const FBoxSphereBounds& Bounds, UBOOL bInSupportCascadedShadow, FTextureLayout* TextureLayout = NULL );
	//>@ ava

	/**
	 * Executes all pending light-map encoding requests.
	 */
	static void FinishEncoding();

	//<@ ava specific ; 2006. 12. 05 changmin
	void ResetCoordinateScaleAndBias()
	{
		CoordinateScale.X = 1.0f;
		CoordinateScale.Y = 1.0f;
		CoordinateBias.X = 0.0f;
		CoordinateBias.Y = 0.0f;
	}
	//>@ ava

protected:
	//@BEGIN deif - lightmap policy support
	template <typename LightmapPolicy>
	UBOOL AreLightmapsValid() const
	{
		for (INT Index=LightmapPolicy::IndexOffset; Index<LightmapPolicy::IndexOffset+LightmapPolicy::NumTextures; ++Index)
		{
			if (!Textures[Index])
				return FALSE;
		}

		return TRUE;
	}

	template <typename LightmapPolicy>
	ULightMapTexture2D* CheckedTexture(INT Index) const
	{
		check(Textures[LightmapPolicy::IndexOffset + Index] != NULL);
		return Textures[LightmapPolicy::IndexOffset + Index];
	}

	template <typename LightmapPolicy>
	ULightMapTexture2D* Texture(INT Index) const
	{
		return Textures[LightmapPolicy::IndexOffset + Index];
	}

	template <typename LightmapPolicy>
	ULightMapTexture2D*& Texture(INT Index) 
	{
		return Textures[LightmapPolicy::IndexOffset + Index];
	}
	//@END deif

	friend struct FLightMapPendingTexture;

	//<@ ava specific ; 2007. 12. 21 changmin
	// add cascaded shadow
	//FLightMap2D(const TArray<FGuid>& InLightGuids);
	FLightMap2D(const TArray<FGuid>& InLightGuids, UBOOL bInSupportsCascadedShadow);
	//>@ ava

	/** The textures containing the light-map data. */
	//ULightMapTexture2D* Textures[NUM_LIGHTMAP_COEFFICIENTS];
	/** A scale to apply to the coefficients. */
	//FVector4 ScaleVectors[NUM_LIGHTMAP_COEFFICIENTS];

	//@EDIT deif ; Collapsed to one huge array :)
	ULightMapTexture2D *Textures[NUM_ALL_LIGHTMAP_TEXTURES];	

	/** The scale which is applied to the light-map coordinates before sampling the light-map textures. */
	FVector2D CoordinateScale;

	/** The bias which is applied to the light-map coordinates before sampling the light-map textures. */
	FVector2D CoordinateBias;
};

/**
 * The raw data which is used to construct a 1D light-map.
 */
class FLightMapData1D
{
public:

	/** The lights which this light-map stores. */
	TArray<ULightComponent*> Lights;

	/**
	 * Minimal initialization constructor.
	 */
	FLightMapData1D(INT Size)
	{
		Data.Empty(Size);
		Data.AddZeroed(Size);
	}

	//!{ 2006-05-11	허 창 민
	void SetSize(UINT InSize)
	{
		Size = InSize;
		Data.Empty( Size );
		Data.AddZeroed( Size );
	}
	//!} 2006-05-11	허 창 민

	// Accessors.
	const FLightSample& operator()(UINT Index) const { return Data(Index); }
	FLightSample& operator()(UINT Index) { return Data(Index); }
	INT GetSize() const { return Data.Num(); }

	//!{ 2006-05-08	허 창 민
	friend FArchive& operator<<(FArchive& Ar,FLightMapData1D& LightMap)
	{
		return Ar << LightMap.Data << LightMap.Size;
	}
	//!} 2006-05-08	허 창 민

private:

	/** The incident light samples for a 1D array of points. */
	TArray<FLightSample> Data;

	/** The number of elements in the light-map. */
	UINT Size;
};


/**
 * The light incident for a point on a surface, stored as bytes representing values from 0-1.
 *
 * @warning BulkSerialize: FQuantizedLightSample is serialized as memory dump
 * See TArray::BulkSerialize for detailed description of implied limitations.
 */
struct FQuantizedLightSample
{
	/** The lighting coefficients, colored. */
	FColor	Coefficients[NUM_LIGHTMAP_COEFFICIENTS];
};

/**
 * Bulk data array of FQuantizedLightSamples
 */
struct FQuantizedLightSampleBulkData : public FUntypedBulkData
{
	/**
	 * Returns whether single element serialization is required given an archive. This e.g.
	 * can be the case if the serialization for an element changes and the single element
	 * serialization code handles backward compatibility.
	 */
	virtual UBOOL RequiresSingleElementSerialization( FArchive& Ar );

	/**
	 * Returns size in bytes of single element.
	 *
	 * @return Size in bytes of single element
	 */
	virtual INT GetElementSize() const;

	/**
	 * Serializes an element at a time allowing and dealing with endian conversion and backward compatiblity.
	 * 
	 * @param Ar			Archive to serialize with
	 * @param Data			Base pointer to data
	 * @param ElementIndex	Element index to serialize
	 */
	virtual void SerializeElement( FArchive& Ar, void* Data, INT ElementIndex );
};

class FDynamicLightMap1D;

/** A 1D array of incident lighting data. */
class FLightMap1D : public FLightMap, public FVertexBuffer
{
public:

	FLightMap1D(): Owner(NULL), CachedSampleData(NULL) {}

	/**
	 * Uses the raw light-map data to construct a vertex buffer.
	 * @param	Owner - The object which owns the light-map.
	 * @param	Data - The raw light-map data.
	 */
	FLightMap1D(UObject* InOwner,FLightMapData1D& Data);
	//<@ ava specific ; 2007. 11. 28 data2 추가...(lightmap data without sun) 
	FLightMap1D(UObject* InOwner,FLightMapData1D& Data, FLightMapData1D& Data2, UBOOL bInSupportsCascadedShadow);
	//>@ ava

	/** Destructor. */
	virtual ~FLightMap1D();

	// FLightMap interface.
	virtual void Serialize(FArchive& Ar);
	virtual void InitResources();
	virtual void Cleanup()
	{
		BeginReleaseResource(this);
		FLightMap::Cleanup();
	}

	// Runtime type casting.
	virtual const FLightMap1D* GetLightMap1D() const { return this; }

	// Accessors.
	const FVector4* GetScaleArray() const
	{
		extern UBOOL GUseCascadedShadow;
		if( GUseCascadedShadow && bSupportsCascadedShadow )
			return ScaleVectorsWithoutSun;
		else
			return ScaleVectors;
	}
	INT NumSamples() const { return Samples.GetElementCount(); }

	//<@ ava specific ; 2006. 10. 16 changmin
	const void* GetCachedSampleData() const
	{
		return CachedSampleData;
	}
	void SetSamples( const TArray<FQuantizedLightSample>& SourceSamples);
	void CopyScaleVectors( const FLightMap1D* SourceLightmap );

	UBOOL Copy( void* Buffer ) const
	{		
		if (CachedSampleData == NULL)
			return FALSE;

		SIZE_T Size = Samples.GetBulkDataSize();
		appMemcpy(Buffer,CachedSampleData,Size);
		return TRUE;
	}
	//>@ ava

	/**
	 * Creates a new lightmap that is a copy of this light map, but where the sample data
	 * has been remapped according to the specified sample index remapping.
	 *
	 * @param		SampleRemapping		Sample remapping: Dst[i] = Src[RemappedIndices[i]].
	 * @return							The new lightmap.
	 */
	FDynamicLightMap1D* DuplicateWithRemappedVerts(const TArray<INT>& SampleRemapping);
	FDynamicLightMap1D* DuplicateWithNewSamples(const TArray<FQuantizedLightSample>& SourceSamples);

private:

	/** The object which owns the light-map. */
	UObject* Owner;

	/** The incident light samples for a 1D array of points. */
	FQuantizedLightSampleBulkData Samples;

	//<@ ava specific ; 2007. 11. 28 changmin
	// add per pixel shadow of sun light
	/** The incident light samples for a 1D array of points. */
	FQuantizedLightSampleBulkData SamplesWithoutSun;
	//>@ ava

	/** 
	 * Cached copy of bulk data that is freed by rendering thread and valid between BeginInitResource
	 * and InitRHI.
	 */
	void* CachedSampleData;

	/** A scale to apply to the coefficients. */
	FVector4 ScaleVectors[NUM_LIGHTMAP_COEFFICIENTS];

	//<@ ava specific ; 2007. 11. 28 changmin
	// add per-pixel shadow for sun light
	/** A scale to apply to the coefficients. */
	FVector4 ScaleVectorsWithoutSun[NUM_LIGHTMAP_COEFFICIENTS];
	//>@ ava

	// FRenderResource interface.
	virtual void InitRHI();
};

class FDynamicLightMap1D : public FLightMap1D
{
public :
	/// Do nothing
	virtual void InitRHI()
	{
	}	
};
