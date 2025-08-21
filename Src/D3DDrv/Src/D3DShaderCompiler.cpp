/*=============================================================================
	D3DShaderCompiler.cpp: D3D shader compiler implementation.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "D3DDrvPrivate.h"

#if USE_D3D_RHI || USE_NULL_RHI

UBOOL GSaveShaderIncludes = FALSE;

/**
 * An implementation of the D3DX include interface to access a FShaderCompilerEnvironment.
 */
class FD3DIncludeEnvironment : public ID3DXInclude
{
public:

	STDMETHOD(Open)(D3DXINCLUDE_TYPE Type,LPCSTR Name,LPCVOID ParentData,LPCVOID* Data,UINT* Bytes)
	{
		FString Filename(ANSI_TO_TCHAR(Name));
		FString FileContents;

		FString* OverrideContents = Environment.IncludeFiles.Find(*Filename);
		if(OverrideContents)
		{
			FileContents = *OverrideContents;
		}
		else
		{
			if(Filename.Mid(1,2) == TEXT(":\\"))
			{
				if( !appLoadFileToString(FileContents,*Filename) )
				{
					appErrorf(TEXT("Couldn't load shader file \'%s\'"),*Filename);
				}
			}
			else
			{
				FileContents = LoadShaderSourceFile(*Filename);
			}
		}

		// Convert the file contents to ANSI.
		ANSICHAR* AnsiFileContents = new ANSICHAR[FileContents.Len() + 1];
		strncpy( AnsiFileContents, TCHAR_TO_ANSI(*FileContents), FileContents.Len() + 1 );

		// Write the result to the output parameters.
		*Data = (LPCVOID)AnsiFileContents;
		*Bytes = FileContents.Len();

		// Write out to file, if requested.
		if ( GSaveShaderIncludes )
		{
			const TCHAR* ShaderPath = TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders");
			appSaveStringToFile( FileContents, *(FString(ShaderPath) + PATH_SEPARATOR + TEXT("CompilingShader-") + Filename) );
		}
		return D3D_OK;
	}

	STDMETHOD(Close)(LPCVOID Data)
	{
		delete Data;
		return D3D_OK;
	}

	FD3DIncludeEnvironment(const FShaderCompilerEnvironment& InEnvironment):
		Environment(InEnvironment)
	{}

private:

	FShaderCompilerEnvironment Environment;
};

/**
 * TranslateCompilerFlag - translates the platform-independent compiler flags into D3DX defines
 * @param CompilerFlag - the platform-independent compiler flag to translate
 * @return DWORD - the value of the appropriate D3DX enum
 */
static DWORD TranslateCompilerFlag(ECompilerFlags CompilerFlag)
{
	switch(CompilerFlag)
	{
	case CFLAG_PreferFlowControl: return D3DXSHADER_PREFER_FLOW_CONTROL;
	case CFLAG_Debug: return D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
	case CFLAG_AvoidFlowControl: return D3DXSHADER_AVOID_FLOW_CONTROL;
	default: return 0;
	};
}

/**
 * The D3DX/HLSL shader compiler.
 */ 
UBOOL D3DCompileShader(
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	UBOOL bSilent = FALSE
	)
{
	TD3DRef<ID3DXBuffer> Shader;
	TD3DRef<ID3DXBuffer> Errors;
	TD3DRef<ID3DXConstantTable> ConstantTable;

	// Translate the input environment's definitions to D3DXMACROs.
	TArray<D3DXMACRO> Macros;
	for(TMap<FName,FString>::TConstIterator DefinitionIt(Environment.Definitions);DefinitionIt;++DefinitionIt)
	{
	  FString Name = DefinitionIt.Key().ToString();
	  FString Definition = DefinitionIt.Value();
	  
	  D3DXMACRO* Macro = new(Macros) D3DXMACRO;
	  Macro->Name = strncpy(new ANSICHAR[Name.Len() + 1],TCHAR_TO_ANSI(*Name),Name.Len() + 1);
	  Macro->Definition = strncpy(new ANSICHAR[Definition.Len() + 1],TCHAR_TO_ANSI(*Definition),Definition.Len() + 1);
	}
	
	// set the COMPILER type
	D3DXMACRO* Macro = new(Macros) D3DXMACRO;
#define COMPILER_NAME "COMPILER_HLSL"
	Macro->Name = strcpy(new ANSICHAR[strlen(COMPILER_NAME) + 1], COMPILER_NAME);
	Macro->Definition = strcpy(new ANSICHAR[2], "1");

	// set the SUPPORTS_DEPTH_TEXTURES if needed
	if( GSupportsDepthTextures )
	{
		D3DXMACRO* MacroDepthSupport = new(Macros) D3DXMACRO;
		MacroDepthSupport->Name = strcpy(new ANSICHAR[strlen("SUPPORTS_DEPTH_TEXTURES") + 1], "SUPPORTS_DEPTH_TEXTURES");
		MacroDepthSupport->Definition = strcpy(new ANSICHAR[2], "1");
	}

	if( GIsLowEndHW )
	{
		D3DXMACRO* MacroDepthSupport = new(Macros) D3DXMACRO;
		MacroDepthSupport->Name = strcpy(new ANSICHAR[strlen("LOWEND") + 1], "LOWEND");
		MacroDepthSupport->Definition = strcpy(new ANSICHAR[2], "1");
	}
	
	DWORD CompileFlags = 0;
	if (DEBUG_SHADERS) 
	{
		//add the debug flags
		CompileFlags |= D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
	}

	for(INT FlagIndex = 0;FlagIndex < Environment.CompilerFlags.Num();FlagIndex++)
	{
		//accumulate flags set by the shader
		CompileFlags |= TranslateCompilerFlag(Environment.CompilerFlags(FlagIndex));
	}

	UBOOL bShaderCompileFailed = FALSE;

	{
		FString SourceFile = LoadShaderSourceFile(SourceFilename);
		FD3DIncludeEnvironment IncludeEnvironment(Environment);
		const TCHAR* ShaderPath = TEXT("..") PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders");

		TCHAR ShaderProfile[32];
		//set defines and profiles for the appropriate shader paths
		if (GRHIShaderPlatform == SP_PCD3D)
		{
			if (Target.Frequency == SF_Pixel)
			{
				appStrcpy(ShaderProfile, TEXT("ps_3_0"));
			}
			else
			{
				appStrcpy(ShaderProfile, TEXT("vs_3_0"));
			}
		}
		else if (GRHIShaderPlatform == SP_PCD3D_SM2 || GRHIShaderPlatform == SP_PCD3D_SM2_POOR)
		{
			if (GRHIShaderPlatform == SP_PCD3D_SM2_POOR)
			{
				//<@ ava specific
				// sm2 poor 라서 특별히 처리해야 될 일은 없습니다.. 2007. 7. 5 changmin
				////check if MANUAL_FP_BLEND is already defined
				//UBOOL bManualFPBlendAlreadyDefined = FALSE;
				//for (INT i = 0; i < Macros.Num(); i++)
				//{
				//	if (strcmp(Macros(i).Name, "MANUAL_FP_BLEND") == 0)
				//	{
				//		bManualFPBlendAlreadyDefined = TRUE;
				//	}
				//}

				////set the MANUAL_FP_BLEND shader define
				//if (!bManualFPBlendAlreadyDefined)
				//{
				//	D3DXMACRO* MacroManualFPBlend = new(Macros) D3DXMACRO;
				//	MacroManualFPBlend->Name = strcpy(new ANSICHAR[strlen("MANUAL_FP_BLEND") + 1], "MANUAL_FP_BLEND");
				//	MacroManualFPBlend->Definition = strcpy(new ANSICHAR[2], "1");
				//}
				//>@ ava
			}

			if (Target.Frequency == SF_Pixel)
			{
				appStrcpy(ShaderProfile, TEXT("ps_2_0"));
			}
			else
			{
				appStrcpy(ShaderProfile, TEXT("vs_2_0"));
			}
		}

		// Terminate the Macros list.
		D3DXMACRO* TerminatorMacro = new(Macros) D3DXMACRO;
		TerminatorMacro->Name = NULL;
		TerminatorMacro->Definition = NULL;

		if (bSilent)
		{
			//temporarily turn off d3dx debug messages
			D3DXDebugMute(TRUE);
		}			

		/*const TCHAR PixelTargets[][32] =
		{
			//TEXT("ps_3_0"),
			TEXT("ps_2_0"),
			TEXT("ps_1_4"),
			TEXT("ps_1_1"),
		};

		INT PixelTargetIndex = -1;

		for (INT i=0; i<ARRAY_COUNT(PixelTargets); ++i)
		{
			if (!appStrcmp( PixelTargets[i], ShaderProfile ))
			{
				PixelTargetIndex = i;
				break;
			}
		}*/

	    HRESULT Result = D3DXCompileShader(
		    TCHAR_TO_ANSI(*SourceFile),
		    SourceFile.Len(),			
		    &Macros(0),
		    &IncludeEnvironment,
		    TCHAR_TO_ANSI(FunctionName),
		    TCHAR_TO_ANSI(ShaderProfile),
		    CompileFlags,
		    Shader.GetInitReference(),
		    Errors.GetInitReference(),
		    ConstantTable.GetInitReference()
		    );

		if (bSilent)
		{
			D3DXDebugMute(FALSE);
		}

		if(FAILED(Result))
		{
			// Copy the error text to the output.
			FString* ErrorString = new(Output.Errors) FString(ANSI_TO_TCHAR(Errors->GetBufferPointer()));

			if (!bSilent)
			{
				// Log the compilation error.
				warnf(TEXT("Shader compile error: %s/%s"),SourceFilename,**ErrorString);

				// Write out everything to file.
				GSaveShaderIncludes = TRUE;
				appSaveStringToFile( SourceFile, *(FString(ShaderPath) + PATH_SEPARATOR + TEXT("CompilingShader-") + SourceFilename + TEXT(".usf")) );
				D3DXCompileShader(
					TCHAR_TO_ANSI(*SourceFile),
					SourceFile.Len(),			
					&Macros(0),
					&IncludeEnvironment,
					TCHAR_TO_ANSI(FunctionName),
					TCHAR_TO_ANSI(ShaderProfile),
					CompileFlags,
					Shader.GetInitReference(),
					Errors.GetInitReference(),
					ConstantTable.GetInitReference()
					);
				GSaveShaderIncludes = FALSE;
			}

			bShaderCompileFailed = TRUE;
		}
		else
		{
			/*if (PixelTargetIndex != -1)
			{
				for (INT i=PixelTargetIndex+1; i<ARRAY_COUNT(PixelTargets); ++i)
				{
					TD3DRef<ID3DXBuffer> OptShader;
					TD3DRef<ID3DXBuffer> OptErrors;
					TD3DRef<ID3DXConstantTable> OptConstantTable;

					Result = D3DXCompileShader(
						TCHAR_TO_ANSI(*SourceFile),
						SourceFile.Len(),			
						&Macros(0),
						&IncludeEnvironment,
						TCHAR_TO_ANSI(FunctionName),
						TCHAR_TO_ANSI(PixelTargets[i]),
						CompileFlags,
						OptShader.GetInitReference(),
						OptErrors.GetInitReference(),
						OptConstantTable.GetInitReference()
						);					

					if (FAILED(Result))
						break;

					debugf( NAME_Log, TEXT( "Shader %s was optimized successfully to target %s" ), FunctionName, PixelTargets[i] );

					Shader = OptShader;
					Errors = OptErrors;
					ConstantTable = OptConstantTable;
				}
			}*/
			// The shader compiled successfully, break out of the retry loop.
			bShaderCompileFailed = FALSE;
		}
	}

	// Free temporary strings allocated for the macros.
	for(INT MacroIndex = 0;MacroIndex < Macros.Num();MacroIndex++)
	{
		delete Macros(MacroIndex).Name;
		delete Macros(MacroIndex).Definition;
	}

	// if the shader failed to compile we need to tell the caller that Compiling the shader failed
	if( bShaderCompileFailed == TRUE )
	{
		return FALSE;
	}


	UINT NumShaderBytes = Shader->GetBufferSize();
	Output.Code.Empty(NumShaderBytes);
	Output.Code.Add(NumShaderBytes);
	appMemcpy(&Output.Code(0),Shader->GetBufferPointer(),NumShaderBytes);

	// Read the constant table description.
	D3DXCONSTANTTABLE_DESC	Desc;
	VERIFYD3DRESULT(ConstantTable->GetDesc(&Desc));

	// Map each constant in the table.
	for(UINT ConstantIndex = 0;ConstantIndex < Desc.Constants;ConstantIndex++)
	{
		// Read the constant description.
		D3DXCONSTANT_DESC ConstantDesc;
		UINT NumConstants = 1;
		VERIFYD3DRESULT(ConstantTable->GetConstantDesc(ConstantTable->GetConstant(NULL,ConstantIndex),&ConstantDesc,&NumConstants));

		Output.ParameterMap.AddParameterAllocation(
			ANSI_TO_TCHAR(ConstantDesc.Name),
			ConstantDesc.RegisterIndex,
			ConstantDesc.RegisterCount
			);
	}

	// Pass the target through to the output.
	Output.Target = Target;

	// Disassemble the shader to determine the instruction count.
	TD3DRef<ID3DXBuffer> DisassemblyBuffer;
	VERIFYD3DRESULT(D3DXDisassembleShader((DWORD*)Shader->GetBufferPointer(),FALSE,NULL,DisassemblyBuffer.GetInitReference()));

	// Extract the instruction count from the disassembly text.
	Parse(
		ANSI_TO_TCHAR((ANSICHAR*)DisassemblyBuffer->GetBufferPointer()),
		TEXT("// approximately "),
		(DWORD&)Output.NumInstructions
		);

	return TRUE;
}

#endif
