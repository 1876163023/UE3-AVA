/*=============================================================================
	BitArray.cpp: Bit array implementation
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "CorePrivate.h"

FBitArray::FBitReference::operator UBOOL() const
{
	 return (Data & Mask) != 0;
}

void FBitArray::FBitReference::operator=(const UBOOL NewValue)
{
	if(NewValue)
	{
		Data |= Mask;
	}
	else
	{
		Data &= ~Mask;
	}
}

FBitArray::FBitArray( const UBOOL Value, const UINT InNumBits ):
	Data(NULL),
	NumBits(InNumBits),
	MaxBits(InNumBits)
{
	Realloc();
	if(NumBits)
	{
		appMemset(Data,Value ? 0xff : 0,(NumBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD * sizeof(DWORD));
	}
}

FBitArray::FBitArray(const FBitArray& Copy):
	Data(NULL)
{
	*this = Copy;
}

FBitArray::~FBitArray()
{
	Empty();
}

FBitArray& FBitArray::operator =(const FBitArray& Copy)
{
	// check for self assignment since we don't use swamp() mechanic
	if( this == &Copy )
	{
		return *this;
	}

	Empty();
	NumBits = MaxBits = Copy.NumBits;
	if(NumBits)
	{
		const INT NumDWORDs = (MaxBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD;
		Data = appMalloc(NumDWORDs * sizeof(DWORD));
		appMemcpy(Data,Copy.Data,NumDWORDs * sizeof(DWORD));
	}
	return *this;
}

INT FBitArray::AddItem(const UBOOL Value)
{
	const INT Index = NumBits;
	NumBits++;

	if(NumBits > MaxBits)
	{
		// Allocate memory for the new bits.
		const UINT MaxDWORDs = (NumBits + 3 * NumBits / 8 + 31 + NumBitsPerDWORD - 1) / NumBitsPerDWORD;
		MaxBits = MaxDWORDs * NumBitsPerDWORD;
		Realloc();
	}

	(*this)(Index) = Value;

	return Index;
}

void FBitArray::Empty()
{
	appFree(Data);
	Data = NULL;
	NumBits = MaxBits = 0;
}

void FBitArray::Realloc()
{
	if(Data || MaxBits)
	{
		const INT NumDWORDs = (MaxBits + NumBitsPerDWORD - 1) / NumBitsPerDWORD;
		Data = appRealloc(Data,NumDWORDs * sizeof(DWORD));
	}
}
