/*=============================================================================
	TerrainVertexFactory.h: Terrain vertex factory definitions.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/
struct FTerrainObject;

/** Vertex factory with vertex stream components for terrain vertices */
class FTerrainVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FTerrainVertexFactory);

public:
	struct DataType
	{
		/** The stream to read the vertex position from.		*/
		FVertexStreamComponent PositionComponent;
		/** The stream to read the vertex displacement from.	*/
		FVertexStreamComponent DisplacementComponent;
		/** The stream to read the vertex gradients from.		*/
		FVertexStreamComponent GradientComponent;
	};

	/**
	 * Should we cache the material's shadertype on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
	{
		//<@ ava specific ; 2006. 9. 22 changmin
		if (appStristr(ShaderType->GetName(), TEXT("<FPointAndSkyLightPolicy>")))
		{
			return FALSE;
		}
		//>@ ava
		// only compile terrain materials for terrain vertex factory
		// The special engine materials must be compiled for the terrain vertex factory because they are used with it for wireframe, etc.
		return Material->IsTerrainMaterial() || Material->IsSpecialEngineMaterial();

	}

	/**
	* An implementation of the interface used by TSynchronizedResource to 
	* update the resource with new data from the game thread.
	* @param	InData - new stream component data
	*/
	void SetData(const DataType& InData)
	{
		Data = InData;
		UpdateRHI();
	}

	/** accessor */
	void SetTerrainObject(FTerrainObject* InTerrainObject)
	{
		TerrainObject = InTerrainObject;
	}

	FTerrainObject* GetTerrainObject()
	{
		return TerrainObject;
	}

	INT GetTessellationLevel()
	{
		return TessellationLevel;
	}

	void SetTessellationLevel(INT InTessellationLevel)
	{
		TessellationLevel = InTessellationLevel;
	}

	// FRenderResource interface.
	virtual void InitRHI();

private:
	/** stream component data bound to this vertex factory */
	DataType Data;  

	FTerrainObject* TerrainObject;
	INT TessellationLevel;
};

/** Decal vertex factory with vertex stream components for terrain vertices */
class FTerrainDecalVertexFactory : public FTerrainVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FTerrainDecalVertexFactory);

public:
	/**
	 * Should we cache the material's shader type on this platform with this vertex factory? 
	 */
	static UBOOL ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);

	void SetDecalMatrix(const FMatrix& InDecalMatrix)
	{
		DecalMatrix = InDecalMatrix;
	}

	const FMatrix& GetDecalMatrix() const
	{
		return DecalMatrix;
	}

	void SetDecalLocation(const FVector& InDecalLocation)
	{
		DecalLocation = InDecalLocation;
	}

	const FVector& GetDecalLocation() const
	{
		return DecalLocation;
	}

	void SetDecalOffset(const FVector2D& InDecalOffset)
	{
		DecalOffset = InDecalOffset;
	}

	const FVector2D& GetDecalOffset() const
	{
		return DecalOffset;
	}

private:
	FMatrix DecalMatrix;
	FVector DecalLocation;
	FVector2D DecalOffset;
};
