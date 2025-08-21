#ifndef __AMBIENTCUBE_H
#define __AMBIENTCUBE_H

struct FAmbientCube
{
	FVector										Color[6];

	friend FArchive& operator<<(FArchive& Ar,FAmbientCube& V)
	{			
		Ar << V.Color[0] << V.Color[1] << V.Color[2] << V.Color[3] << V.Color[4] << V.Color[5];
		return Ar;
	}
};

struct FAmbientCubeContainer : public FSerializableObject
{		
	INT											NumX, NumY, NumZ;
	FVector										MinCorner, MaxCorner;
	TArray<FAmbientCube>						Cubes;		

	virtual void Serialize( FArchive& Ar );

	const FVector* GetAmbientCube( INT& Index, const FVector& Location );
};

#endif

