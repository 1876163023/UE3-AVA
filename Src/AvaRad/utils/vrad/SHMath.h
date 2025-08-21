#include "mathlib.h"
/**
* Copyright2006 Epic Games, Inc. All Rights Reserved.
*/

//	Constants.

/**
* A linear, 32-bit/component floating point RGBA color.
*/

struct LinearColor
{
	float	R,
		G,
		B,
		A;

	LinearColor() {}
	LinearColor(float InR,float InG,float InB,float InA = 1.0f): R(InR), G(InG), B(InB), A(InA) {}	

	const float& Component(int Index) const
	{
		return (&R)[Index];
	}

	LinearColor operator+(const LinearColor& ColorB) const
	{
		return LinearColor(
			this->R + ColorB.R,
			this->G + ColorB.G,
			this->B + ColorB.B,
			this->A + ColorB.A
			);
	}
	LinearColor& operator+=(const LinearColor& ColorB)
	{
		R += ColorB.R;
		G += ColorB.G;
		B += ColorB.B;
		A += ColorB.A;
		return *this;
	}

	LinearColor operator-(const LinearColor& ColorB) const
	{
		return LinearColor(
			this->R - ColorB.R,
			this->G - ColorB.G,
			this->B - ColorB.B,
			this->A - ColorB.A
			);
	}
	LinearColor& operator-=(const LinearColor& ColorB)
	{
		R -= ColorB.R;
		G -= ColorB.G;
		B -= ColorB.B;
		A -= ColorB.A;
		return *this;
	}

	LinearColor operator*(const LinearColor& ColorB) const
	{
		return LinearColor(
			this->R * ColorB.R,
			this->G * ColorB.G,
			this->B * ColorB.B,
			this->A * ColorB.A
			);
	}
	LinearColor& operator*=(const LinearColor& ColorB)
	{
		R *= ColorB.R;
		G *= ColorB.G;
		B *= ColorB.B;
		A *= ColorB.A;
		return *this;
	}

	LinearColor operator*(float Scalar) const
	{
		return LinearColor(
			this->R * Scalar,
			this->G * Scalar,
			this->B * Scalar,
			this->A * Scalar
			);
	}

	LinearColor& operator*=(float Scalar)
	{
		R *= Scalar;
		G *= Scalar;
		B *= Scalar;
		A *= Scalar;
		return *this;
	}

	LinearColor operator/(float Scalar) const
	{
		const float	InvScalar = 1.0f / Scalar;
		return LinearColor(
			this->R * InvScalar,
			this->G * InvScalar,
			this->B * InvScalar,
			this->A * InvScalar
			);
	}
	LinearColor& operator/=(float Scalar)
	{
		const float	InvScalar = 1.0f / Scalar;
		R *= InvScalar;
		G *= InvScalar;
		B *= InvScalar;
		A *= InvScalar;
		return *this;
	}

	bool operator==(const LinearColor& ColorB) const
	{
		return this->R == ColorB.R && this->G == ColorB.G && this->B == ColorB.B && this->A == ColorB.A;
	}	

	// Common colors.	
	static const LinearColor White;
	static const LinearColor Black;
};

inline LinearColor operator*(float Scalar,const LinearColor& Color)
{
	return Color.operator*( Scalar );
}

#define MAX_SH_ORDER	2
#define MAX_SH_BASIS	(MAX_SH_ORDER*MAX_SH_ORDER)

/** A vector of spherical harmonic coefficients. */
class SHVector
{
public:

	float V[MAX_SH_BASIS];

	/** Default constructor. */
	SHVector()
	{
		for(int BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		{
			V[BasisIndex] = 0.0f;
		}
	}

	/** Scalar multiplication operator. */
	friend SHVector operator*(const SHVector& A,float B)
	{
		SHVector Result;
		for(int BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		{
			Result.V[BasisIndex] = A.V[BasisIndex] * B;
		}
		return Result;
	}

	/** Addition operator. */
	friend SHVector operator+(const SHVector& A,const SHVector& B)
	{
		SHVector Result;
		for(int BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		{
			Result.V[BasisIndex] = A.V[BasisIndex] + B.V[BasisIndex];
		}
		return Result;
	}

	/** Subtraction operator. */
	friend SHVector operator-(const SHVector& A,const SHVector& B)
	{
		SHVector Result;
		for(int BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		{
			Result.V[BasisIndex] = A.V[BasisIndex] - B.V[BasisIndex];
		}
		return Result;
	}

	/** Dot product operator. */
	friend float Dot(const SHVector& A,const SHVector& B)
	{
		float Result = 0.0f;
		for(int BasisIndex = 0;BasisIndex < MAX_SH_BASIS;BasisIndex++)
		{
			Result += A.V[BasisIndex] * B.V[BasisIndex];
		}
		return Result;
	}

	/** In-place addition operator. */
	SHVector& operator+=(const SHVector& B)
	{
		*this = *this + B;
		return *this;
	}

	/** In-place subtraction operator. */
	SHVector& operator-=(const SHVector& B)
	{
		*this = *this - B;
		return *this;
	}

	/** In-place scalar division operator. */
	SHVector& operator/=(float B)
	{
		*this = *this * (1.0f / B);
		return *this;
	}
};

/** A vector of colored spherical harmonic coefficients. */
class SHVectorRGB
{
public:

	SHVector R;
	SHVector G;
	SHVector B;

	/** Calculates greyscale spherical harmonic coefficients. */
	SHVector GetLuminance()
	{
		return R * 0.3f + G * 0.59f + B * 0.11f;
	}

	/** Scalar multiplication operator. */
	friend SHVectorRGB operator*(const SHVectorRGB& A,float B)
	{
		SHVectorRGB Result;
		Result.R = A.R * B;
		Result.G = A.G * B;
		Result.B = A.B * B;
		return Result;
	}

	/** Addition operator. */
	friend SHVectorRGB operator+(const SHVectorRGB& A,const SHVectorRGB& B)
	{
		SHVectorRGB Result;
		Result.R = A.R + B.R;
		Result.G = A.G + B.G;
		Result.B = A.B + B.B;
		return Result;
	}

	/** Subtraction operator. */
	friend SHVectorRGB operator-(const SHVectorRGB& A,const SHVectorRGB& B)
	{
		SHVectorRGB Result;
		Result.R = A.R - B.R;
		Result.G = A.G - B.G;
		Result.B = A.B - B.B;
		return Result;
	}

	/** Dot product operator. */
	friend LinearColor Dot(const SHVectorRGB& A,const SHVector& B)
	{
		LinearColor Result(LinearColor::Black);
		Result.R = Dot(A.R,B);
		Result.G = Dot(A.G,B);
		Result.B = Dot(A.B,B);
		return Result;
	}

	/** In-place addition operator. */
	SHVectorRGB& operator+=(const SHVectorRGB& B)
	{
		*this = *this + B;
		return *this;
	}

	/** In-place subtraction operator. */
	SHVectorRGB& operator-=(const SHVectorRGB& B)
	{
		*this = *this - B;
		return *this;
	}
};

/** Color multiplication operator. */
__forceinline SHVectorRGB operator*(const SHVector& A,const LinearColor& B)
{
	SHVectorRGB Result;
	Result.R = A * B.R;
	Result.G = A * B.G;
	Result.B = A * B.B;
	return Result;
}

/** Returns the value of the SH basis L,M at the point on the sphere defined by the unit vector Vector. */
SHVector SHBasisFunction(const Vector& Vector);
SHVector PointLightSH(const Vector& Direction);