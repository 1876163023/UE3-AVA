#include "SHMath.h"
#include <math.h>

/**
* Copyright2006 Epic Games, Inc. All Rights Reserved.
*/


//
//	Spherical harmonic globals.
//
float	NormalizationConstants[MAX_SH_BASIS];
int		BasisL[MAX_SH_BASIS];
int		BasisM[MAX_SH_BASIS];

/** Computes a factorial. */
int Factorial(int A)
{
	if(A == 0)
	{
		return 1;
	}
	else
	{
		return A * Factorial(A - 1);
	}
}

/** Initializes the tables used to calculate SH values. */
int InitSHTables()
{
	int	L = 0,
		M = 0;

	for(int BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
	{
		BasisL[BasisIndex] = L;
		BasisM[BasisIndex] = M;

		NormalizationConstants[BasisIndex] = sqrtf(
			(float(2 * L + 1) / float(4 * M_PI)) *
			(float(Factorial(L - fabsf(M))) / float(Factorial(L + fabsf(M))))
			);

		if(M != 0)
			NormalizationConstants[BasisIndex] *= sqrtf(2.f);

		M++;
		if(M > L)
		{
			L++;
			M = -L;
		}
	}

	return 0;
}
static int InitDummy = InitSHTables();

/** Returns the basis index of the SH basis L,M. */
__forceinline int SHGetBasisIndex(int L,int M)
{
	return L * (L + 1) + M;
}

/** Evaluates the LegendrePolynomial for L,M at X */
__forceinline float LegendrePolynomial(int L,int M,float X)
{
	switch(L)
	{
	case 0:
		return 1;
	case 1:
		if(M == 0)
			return X;
		else if(M == 1)
			return -sqrtf(1 - X * X);
		break;
	case 2:
		if(M == 0)
			return -0.5f + (3 * X * X) / 2;
		else if(M == 1)
			return -3 * X * sqrtf(1 - X * X);
		else if(M == 2)
			return -3 * (-1 + X * X);
		break;
	case 3:
		if(M == 0)
			return -(3 * X) / 2 + (5 * X * X * X) / 2;
		else if(M == 1)
			return -3 * sqrtf(1 - X * X) / 2 * (-1 + 5 * X * X);
		else if(M == 2)
			return -15 * (-X + X * X * X);
		else if(M == 3)
			return -15 * powf(1 - X * X,1.5f);
		break;
	case 4:
		if(M == 0)
			return 0.125f * (3.0f - 30.0f * X * X + 35.0f * X * X * X * X);
		else if(M == 1)
			return -2.5f * X * sqrtf(1.0f - X * X) * (7.0f * X * X - 3.0f);
		else if(M == 2)
			return -7.5f * (1.0f - 8.0f * X * X + 7.0f * X * X * X * X);
		else if(M == 3)
			return -105.0f * X * powf(1 - X * X,1.5f);
		else if(M == 4)
			return 105.0f * Square(X * X - 1.0f);
		break;
	case 5:
		if(M == 0)
			return 0.125f * X * (15.0f - 70.0f * X * X + 63.0f * X * X * X * X);
		else if(M == 1)
			return -1.875f * sqrtf(1.0f - X * X) * (1.0f - 14.0f * X * X + 21.0f * X * X * X * X);
		else if(M == 2)
			return -52.5f * (X - 4.0f * X * X * X + 3.0f * X * X * X * X * X);
		else if(M == 3)
			return -52.5f * powf(1.0f - X * X,1.5f) * (9.0f * X * X - 1.0f);
		else if(M == 4)
			return 945.0f * X * Square(X * X - 1);
		else if(M == 5)
			return -945.0f * powf(1.0f - X * X,2.5f);
		break;
	};

	return 0.0f;
}

SHVector SHBasisFunction(const Vector& Vector)
{
	SHVector	Result;

	// Initialize the result to the normalization constant.
	for(int BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		Result.V[BasisIndex] = NormalizationConstants[BasisIndex];

	// Multiply the result by the phi-dependent part of the SH bases.
	float	Phi = atan2f(Vector.y,Vector.x);
	for(int BandIndex = 1;BandIndex < MAX_SH_ORDER;BandIndex++)
	{
		float	SinPhiM = sinf(BandIndex * Phi),
			CosPhiM = cosf(BandIndex * Phi);

		for(int RecurrentBandIndex = BandIndex;RecurrentBandIndex < MAX_SH_ORDER;RecurrentBandIndex++)
		{
			Result.V[SHGetBasisIndex(RecurrentBandIndex,-BandIndex)] *= SinPhiM;
			Result.V[SHGetBasisIndex(RecurrentBandIndex,+BandIndex)] *= CosPhiM;
		}
	}

	// Multiply the result by the theta-dependent part of the SH bases.
	for(int BasisIndex = 1;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		Result.V[BasisIndex] *= LegendrePolynomial(BasisL[BasisIndex],fabsf(BasisM[BasisIndex]),Vector.z);

	return Result;
}

/** Returns the SH coefficients for a point light from the given direction and brightness. */
SHVector PointLightSH(const Vector& Direction)
{
	SHVector Result = SHBasisFunction(Direction);

	// Normalize the SH so its surface adds up to 1.
	Result /= sqrtf(Dot(Result,Result));

	return Result;
}