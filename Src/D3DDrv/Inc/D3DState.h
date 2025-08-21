/*=============================================================================
	D3DState.h: D3D state definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#if USE_D3D_RHI

// State block
typedef TD3DRef<IDirect3DStateBlock9> FStateBlockRHIRef;
typedef IDirect3DStateBlock9* FStateBlockRHIParamRef;

class FD3DBaseState : public FD3DRefCountedObject
{
public :
	FD3DBaseState();
	~FD3DBaseState();

	FStateBlockRHIRef StateBlock;

	static void ResetStateBlocks();
};
typedef TD3DRef<FD3DBaseState> FBaseStateRHIRef;
typedef FD3DBaseState* FBaseStateRHIParamRef;

class FD3DSamplerState : public FD3DBaseState
{
public:
	D3DTEXTUREFILTERTYPE MagFilter;
	D3DTEXTUREFILTERTYPE MinFilter;
	D3DTEXTUREFILTERTYPE MipFilter;
	D3DTEXTUREADDRESS AddressU;
	D3DTEXTUREADDRESS AddressV;
	D3DTEXTUREADDRESS AddressW;
	DWORD MipMapLODBias;
};
typedef TD3DRef<FD3DSamplerState> FSamplerStateRHIRef;
typedef FD3DSamplerState* FSamplerStateRHIParamRef;

class FD3DRasterizerState : public FD3DBaseState
{
public:
	D3DFILLMODE FillMode;
	D3DCULL CullMode;
	FLOAT DepthBias;
	FLOAT SlopeScaleDepthBias;
};
typedef TD3DRef<FD3DRasterizerState> FRasterizerStateRHIRef;
typedef FD3DRasterizerState* FRasterizerStateRHIParamRef;

class FD3DDepthState : public FD3DBaseState
{
public:
	UBOOL bZEnable;
	UBOOL bZWriteEnable;
	D3DCMPFUNC ZFunc;
};
typedef TD3DRef<FD3DDepthState> FDepthStateRHIRef;
typedef FD3DDepthState* FDepthStateRHIParamRef;

class FD3DStencilState : public FD3DBaseState
{
public:
	UBOOL bStencilEnable;
	UBOOL bTwoSidedStencilMode;
	D3DCMPFUNC StencilFunc;
	D3DSTENCILOP StencilFail;
	D3DSTENCILOP StencilZFail;
	D3DSTENCILOP StencilPass;
	D3DCMPFUNC CCWStencilFunc;
	D3DSTENCILOP CCWStencilFail;
	D3DSTENCILOP CCWStencilZFail;
	D3DSTENCILOP CCWStencilPass;
	DWORD StencilReadMask;
	DWORD StencilWriteMask;
	DWORD StencilRef;
};
typedef TD3DRef<FD3DStencilState> FStencilStateRHIRef;
typedef FD3DStencilState* FStencilStateRHIParamRef;

class FD3DBlendState : public FD3DBaseState
{
public:
	UBOOL bAlphaBlendEnable;
	D3DBLENDOP ColorBlendOperation;
	D3DBLEND ColorSourceBlendFactor;
	D3DBLEND ColorDestBlendFactor;
	UBOOL bSeparateAlphaBlendEnable;
	D3DBLENDOP AlphaBlendOperation;
	D3DBLEND AlphaSourceBlendFactor;
	D3DBLEND AlphaDestBlendFactor;
	UBOOL bAlphaTestEnable;
	D3DCMPFUNC AlphaFunc;
	DWORD AlphaRef;
};
typedef TD3DRef<FD3DBlendState> FBlendStateRHIRef;
typedef FD3DBlendState* FBlendStateRHIParamRef;

#endif
