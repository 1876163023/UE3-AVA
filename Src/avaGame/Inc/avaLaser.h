#pragma once

class UavaParticleLaserEmitterInstance;
//
//	TypeDataLaser payload
//

struct FLaserTypeDataPayload
{
	/** The source of this Laser											*/
	FVector		SourcePoint;
		
	/** The target of this Laser											*/
	FVector		TargetPoint;
	
	/** Direction to step in											*/
	FVector		Direction;
		
	/** The number of triangles to render for this Laser					*/
	INT			TriangleCount;

	/**
	*	Type and indexing flags
	* 3               1              0
	* 1...|...|...|...5...|...|...|..0
	* TtPppppppppppppppNnnnnnnnnnnnnnn
	* Tt				= Type flags --> 00 = Middle of Laser (nothing...)
	* 									 01 = Start of Laser
	* 									 10 = End of Laser
	* Ppppppppppppppp	= Previous index
	* Nnnnnnnnnnnnnnn	= Next index
	* 		INT				Flags;
	* 
	* NOTE: These values DO NOT get packed into the vertex buffer!
	*/
	INT			Flags;
};

/**	Particle Source/Target Data Payload									*/
struct FLaserParticleSourceTargetPayloadData
{
	INT			ParticleIndex;
};

/**	Particle Source Branch Payload										*/
struct FLaserParticleSourceBranchPayloadData
{
	INT			NoiseIndex;
};

//
//	FParticleLaserVertexBuffer.
//
struct FParticleLaserVertexBuffer : public FVertexBuffer
{
	UavaParticleLaserEmitterInstance*	Owner;

	FParticleLaserVertexBuffer(UavaParticleLaserEmitterInstance* InOwner);

	void SetSize(UINT InNumQuads);
	virtual void GetData(void* Buffer);	
	virtual FString DescribeResource() { return TEXT("Laser particle vertices"); }
};

//
//	FLaserIndexBuffer.
//
struct FLaserIndexBuffer : public FQuadIndexBuffer
{
	UavaParticleLaserEmitterInstance*	Owner;

	FLaserIndexBuffer(UavaParticleLaserEmitterInstance* Owner);

	virtual void SetSize(UINT InNumQuads);
	virtual void GetData(void* Buffer);
};

/* must be 16bytes-aligned */
struct FLaserPoint
{
	FVector Location;
	float Alpha;
	float Intensity;	
};