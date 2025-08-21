/*=============================================================================
	DecalVertexFactory.h: Decal vertex factory implementation
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __DECALVERTEXFACTORY_H__
#define __DECALVERTEXFACTORY_H__
#if 0
/**
 * A vertex factory that transforms vertex positions from local to world space and into world space, generating texture coordinateswith vertex stream components for decal vertices.
 */
class FDecalVertexFactory : public FVertexFactory
{
	DECLARE_VERTEX_FACTORY_TYPE(FDecalVertexFactory);

public:
	struct DataType
	{
		/** The stream to read the vertex position from. */
		FVertexStreamComponent PositionComponent;

		/** The streams to read the tangent basis from. */
		FVertexStreamComponent TangentBasisComponents[3];

		/** The stream to read light map coordinates from. */
		FVertexStreamComponent ShadowMapCoordinateComponent;
	};

	/**
	 * An implementation of the interface used by TSynchronizedResource to update the resource with new data from the game thread.
	 *
	 * @param	InData		New stream component data.
	 */
	void SetData(const DataType& InData)
	{
		Data = InData;
		UpdateRHI();
	}

	FORCEINLINE void SetWorldToDecal(const FMatrix& InWorldToDecal)
	{
		WorldToDecal = InWorldToDecal;
	}

	FORCEINLINE const FMatrix& GetWorldToDecal() const
	{
		return WorldToDecal;
	}

	// FRenderResource interface.

	/**
	 * Creates declarations for each of the vertex stream components and
	 * initializes the device resource.
	 */
	virtual void InitRHI();

private:
	/** stream component data bound to this vertex factory */
	DataType Data;  
	FMatrix WorldToDecal;
};
#endif
#endif // __DECALVERTEXFACTORY_H__
