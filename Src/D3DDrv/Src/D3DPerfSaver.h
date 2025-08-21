#ifndef INC_D3DPERFSAVER_H
#define INC_D3DPERFSAVER_H

#define MAX_D3D_SAMPLERS 16

template <typename T>
FORCEINLINE void MakeUninitialized( T& t )
{
	appMemset(&t,0xcd,sizeof(T));
}

struct FD3DStatePerfSaver
{
public :

#define MAX_NUM_SHADER_CONSTANTS	256

	IDirect3DVertexShader9*			VertexShader;
	IDirect3DPixelShader9*			PixelShader;
	IDirect3DVertexDeclaration9*	VertexDeclaration;
	FVector4						PixelShaderConstants[MAX_NUM_SHADER_CONSTANTS];
	FVector4						VertexShaderConstants[MAX_NUM_SHADER_CONSTANTS];

	void BeginFrame()
	{
		MakeUninitialized(VertexShader);
		MakeUninitialized(PixelShader);
		MakeUninitialized(VertexDeclaration);
		MakeUninitialized(VertexShaderConstants);
		MakeUninitialized(PixelShaderConstants);
	}

	void InvalidateVertexShaderConstant(UINT BaseRegisterIndex,UINT NumVectors)
	{
		for (;NumVectors;--NumVectors)
		{
			MakeUninitialized(VertexShaderConstants[BaseRegisterIndex++]);
		}		
	}

	void InvalidatePixelShaderConstant(UINT BaseRegisterIndex,UINT NumVectors)
	{
		for (;NumVectors;--NumVectors)
		{
			MakeUninitialized(PixelShaderConstants[BaseRegisterIndex++]);
		}		
	}
	
	void SetVertexShaderConstantF(UINT BaseRegisterIndex,const FLOAT* NewValue,UINT NumVectors)
	{
		check(BaseRegisterIndex + NumVectors < MAX_NUM_SHADER_CONSTANTS);		

		if (!appMemcmp( &VertexShaderConstants[BaseRegisterIndex], NewValue, sizeof(FVector4) * NumVectors ))
			return;
		
		appMemcpy( &VertexShaderConstants[BaseRegisterIndex], NewValue, sizeof(FVector4) * NumVectors );
		GDirect3DDevice->SetVertexShaderConstantF(BaseRegisterIndex, NewValue, NumVectors);				
	}

	void SetPixelShaderConstantF(UINT BaseRegisterIndex,const FLOAT* NewValue,UINT NumVectors)
	{
		check(BaseRegisterIndex + NumVectors < MAX_NUM_SHADER_CONSTANTS);		

		if (!appMemcmp( &PixelShaderConstants[BaseRegisterIndex], NewValue, sizeof(FVector4) * NumVectors ))
			return;

		appMemcpy( &PixelShaderConstants[BaseRegisterIndex], NewValue, sizeof(FVector4) * NumVectors );
		GDirect3DDevice->SetPixelShaderConstantF(BaseRegisterIndex, NewValue, NumVectors);				
	}

	void SetVertexDeclaration( IDirect3DVertexDeclaration9* InVertexDeclaration )
	{
		if (InVertexDeclaration != VertexDeclaration)
		{
			VertexDeclaration = InVertexDeclaration;
			GDirect3DDevice->SetVertexDeclaration(VertexDeclaration);
		}
	}

	void SetVertexShader( IDirect3DVertexShader9* InVertexShader )
	{
		if (InVertexShader != VertexShader)
		{
			VertexShader = InVertexShader;
			GDirect3DDevice->SetVertexShader(VertexShader);
		}
	}

	void SetPixelShader( IDirect3DPixelShader9* InPixelShader )
	{
		if (InPixelShader != PixelShader)
		{
			PixelShader = InPixelShader;
			GDirect3DDevice->SetPixelShader(PixelShader);
		}
	}

	struct FD3DRenderStateEntry
	{			
		DWORD Value;
		FD3DRenderStateEntry( DWORD InValue )
			: Value( InValue )
		{
		}

		FORCEINLINE void Set( D3DRENDERSTATETYPE Index, DWORD NewValue )
		{
			if (NewValue != Value)
			{
				Value = NewValue;
				GDirect3DDevice->SetRenderState(Index,NewValue);
			}
		}
	};

	struct FD3DSamplerState
	{
		struct FD3DSamplerStateEntry
		{			
			DWORD Value;
			FD3DSamplerStateEntry( DWORD InValue )
				: Value( InValue )
			{
			}

			FORCEINLINE void Set( INT SamplerIndex, D3DSAMPLERSTATETYPE Index, DWORD NewValue )
			{
				if (NewValue != Value)
				{
					Value = NewValue;
					GDirect3DDevice->SetSamplerState(SamplerIndex,Index,NewValue);
				}
			}
		};
		FD3DSamplerStateEntry	bIsSRGB;
		FD3DSamplerStateEntry	MagFilter, MinFilter, MipFilter, AddressU, AddressV, AddressW, MipMapLODBias;

		FD3DSamplerState()
			: bIsSRGB(FALSE), MagFilter(D3DTEXF_POINT), MinFilter(D3DTEXF_POINT), MipFilter(D3DTEXF_NONE), AddressU(D3DTADDRESS_WRAP), AddressV(D3DTADDRESS_WRAP), AddressW(D3DTADDRESS_WRAP), MipMapLODBias(0)
		{
		}	

		FORCEINLINE void SetState( INT SamplerIndex, UBOOL bNewIsSRGB, FSamplerStateRHIParamRef NewState )
		{
			bIsSRGB.Set(SamplerIndex,D3DSAMP_SRGBTEXTURE,bNewIsSRGB);

			if (NewState)
			{
				MagFilter.Set(SamplerIndex,D3DSAMP_MAGFILTER,NewState->MagFilter);
				MinFilter.Set(SamplerIndex,D3DSAMP_MINFILTER,NewState->MinFilter);
				MipFilter.Set(SamplerIndex,D3DSAMP_MIPFILTER,NewState->MipFilter);
				AddressU.Set(SamplerIndex,D3DSAMP_ADDRESSU,NewState->AddressU);
				AddressV.Set(SamplerIndex,D3DSAMP_ADDRESSV,NewState->AddressV);
				AddressW.Set(SamplerIndex,D3DSAMP_ADDRESSW,NewState->AddressW);
				MipMapLODBias.Set(SamplerIndex,D3DSAMP_MIPMAPLODBIAS,NewState->MipMapLODBias);					
			}
		}
	};

	FD3DStatePerfSaver()
		: ColorWriteEnable(0x0000000F), bScissorTestEnable(FALSE), bSRGBWriteEnable(FALSE)
	{		
	}

	FD3DRenderStateEntry bSRGBWriteEnable, bScissorTestEnable, ColorWriteEnable;

	FD3DSamplerState SamplerStates[MAX_D3D_SAMPLERS];	

	FORCEINLINE void SetSamplerState( INT SamplerIndex, UBOOL bIsSRGB, FSamplerStateRHIParamRef NewState )
	{
		extern UBOOL Ava_GForceSRGB;
		SamplerStates[SamplerIndex].SetState( SamplerIndex, Ava_GForceSRGB ? TRUE : bIsSRGB, NewState );
	}	

	FORCEINLINE void SetRenderState_COLORWRITEENABLE( DWORD StateValue )
	{
		ColorWriteEnable.Set(D3DRS_COLORWRITEENABLE,StateValue);
	}

	FORCEINLINE void SetRenderState_SCISSORTESTENABLE( UBOOL bEnable )
	{
		bScissorTestEnable.Set(D3DRS_SCISSORTESTENABLE,bEnable);		
	}	

	FORCEINLINE void SetRenderState_SRGBWRITEENABLE( UBOOL bEnable )
	{
		bSRGBWriteEnable.Set(D3DRS_SRGBWRITEENABLE,bEnable);
	}	
};

extern FD3DStatePerfSaver* GD3DPerfSaver;

void D3DPerfSaver_Reset();

#endif