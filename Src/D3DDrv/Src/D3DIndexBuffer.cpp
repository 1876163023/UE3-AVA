/*=============================================================================
	D3DIndexBuffer.cpp: D3D Index buffer RHI implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3DDrvPrivate.h"

#if USE_D3D_RHI

extern UBOOL GDoNotUseHarewareDynamicBuffers;

FIndexBufferRHIRef RHICreateIndexBuffer(UINT Stride,UINT Size,FResourceArrayInterface* ResourceArray,UBOOL bIsDynamic)
{
	SCOPE_CYCLE_COUNTER(STAT_RHICreateIndexBufferTime);
	INC_DWORD_STAT(STAT_RHICreateIndexBuffer);

	// Explicitly check that the size is nonzero before allowing CreateIndexBuffer to opaquely fail.
	check(Size > 0);

	// Determine the appropriate usage flags, pool and format for the resource.
	const DWORD Usage = bIsDynamic ? ((GDoNotUseHarewareDynamicBuffers ? 0 : D3DUSAGE_DYNAMIC) | D3DUSAGE_WRITEONLY) : 0;
	const D3DPOOL Pool = bIsDynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
	const D3DFORMAT Format = (Stride == sizeof(WORD) ? D3DFMT_INDEX16 : D3DFMT_INDEX32);

	// Create the index buffer.
	FIndexBufferRHIRef IndexBuffer;
	VERIFYD3DRESULT(GDirect3DDevice->CreateIndexBuffer(Size,Usage,Format,Pool,IndexBuffer.GetInitReference(),NULL));

	// If a resource array was provided for the resource, copy the contents into the buffer.
	if(ResourceArray)
	{
		// Initialize the buffer.
		void* Buffer = RHILockIndexBuffer(IndexBuffer,0,Size);
		check(Buffer);
		check(Size == ResourceArray->GetResourceDataSize());
		appMemcpy(Buffer,ResourceArray->GetResourceData(),Size);
		RHIUnlockIndexBuffer(IndexBuffer);

		// Discard the resource array's contents.
		ResourceArray->Discard();
	}

	return IndexBuffer;
}

void* RHILockIndexBuffer(FIndexBufferRHIParamRef IndexBuffer,UINT Offset,UINT Size,UBOOL bAppend)
{	
	INC_DWORD_STAT(STAT_RHILockIndexBuffer);

	// Determine whether this is a static or dynamic IB.
	D3DINDEXBUFFER_DESC IndexBufferDesc;
	VERIFYD3DRESULT(IndexBuffer->GetDesc(&IndexBufferDesc));
	const UBOOL bIsDynamic = (IndexBufferDesc.Usage & D3DUSAGE_DYNAMIC) != 0;

	// For dynamic IBs, discard the previous contents before locking.	
	const DWORD LockFlags = bIsDynamic ? (D3DLOCK_NOSYSLOCK | (bAppend ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD)) : 0;

	// Lock the index buffer.
	void* Data = NULL;
	VERIFYD3DRESULT(IndexBuffer->Lock(Offset,Size,&Data,LockFlags));
	return Data;
}

void RHIUnlockIndexBuffer(FIndexBufferRHIParamRef IndexBuffer)
{	
	VERIFYD3DRESULT(IndexBuffer->Unlock());
}

#endif