/*=============================================================================
	ShaderCompiler.cpp: Platform independent shader compilation.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"

/**
* Converts shader platform to human readable string. 
*
* @param ShaderPlatform	Shader platform enum
* @return text representation of enum
*/
const TCHAR* ShaderPlatformToText( EShaderPlatform ShaderPlatform )
{
	switch( ShaderPlatform )
	{
	case SP_PCD3D_SM2_POOR:
		return TEXT("PC-D3D-SM2P");
		break;
	case SP_PCD3D_SM2:
		return TEXT("PC-D3D-SM2");
		break;
	case SP_PCD3D :
		return TEXT("PC-D3D");
		break;
	case SP_XBOXD3D:
		return TEXT("Xbox360");
		break;
	case SP_PS3:
		return TEXT("PS3");
		break;
	default:
		return TEXT("Unknown");
		break;
	}
}

FConsoleShaderPrecompiler* GConsoleShaderPrecompilers[SP_NumPlatforms] = { NULL, NULL, NULL, NULL, NULL };

extern UBOOL D3DCompileShader(
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	UBOOL bSilent
	);

extern UBOOL XeD3DCompileShader(
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output
	);

UBOOL CompileShader(
					const TCHAR* SourceFilename,
					const TCHAR* FunctionName,
					FShaderTarget Target,
					const FShaderCompilerEnvironment& InEnvironment,
					FShaderCompilerOutput& Output,
					UBOOL bSilent = FALSE
					)
{
	STAT(DOUBLE RHICompileTime = 0);

	UBOOL bResult = FALSE;
	UBOOL bRetry;

	FShaderCompilerEnvironment Environment( InEnvironment );

	// #define PIXELSHADER and VERTEXSHADER accordingly
	{
		Environment.Definitions.Set( TEXT("PIXELSHADER"),  (Target.Frequency == SF_Pixel) ? TEXT("1") : TEXT("0") );
		Environment.Definitions.Set( TEXT("VERTEXSHADER"), (Target.Frequency == SF_Vertex) ? TEXT("1") : TEXT("0") );
	}

	do 
	{
		bRetry = FALSE;

#if defined(_COMPILESHADER_DEBUG_DUMP_)
		warnf(TEXT("   Compiling shader %s[%s] for platform %d"), SourceFilename, FunctionName, Target.Platform);
#endif	//#if defined(_COMPILESHADER_DEBUG_DUMP_)

		if(Target.Platform == GRHIShaderPlatform)
		{
#if XBOX
			check(Target.Platform == SP_XBOXD3D);
			bResult = XeD3DCompileShader(SourceFilename,FunctionName,Target,Environment,Output);
#elif !CONSOLE
			check(IsPCPlatform( (EShaderPlatform)Target.Platform ));
			bResult = D3DCompileShader(SourceFilename,FunctionName,Target,Environment,Output,bSilent);
#else
			appErrorf(TEXT("Attempted to compile \'%s\' shader on platform that doesn't support dynamic shader compilation."),SourceFilename);
#endif
		}
		// If we're compiling for a different platform than we're running on, use the external precompiler.
		else
		{
#if !CONSOLE
			if( IsPCPlatform( (EShaderPlatform)Target.Platform ))
			{
				bResult = D3DCompileShader(SourceFilename,FunctionName,Target,Environment,Output,bSilent);
			}
			else
			{
				// if we have a global shader precompiler object, use that to compile the shader instead
				check(GConsoleShaderPrecompilers[Target.Platform]);
				bResult = GConsoleShaderPrecompilers[Target.Platform]->CallPrecompiler(SourceFilename, FunctionName, Target, Environment, Output);
			}
#else
			appErrorf(TEXT("Attempted to compile \'%s\' shader for non-native platform %d on console."),SourceFilename,Target.Platform);
#endif
		}

#if defined(_DEBUG) && !CONSOLE
		// Give the user a chance to fix the shader error.
		if (!bSilent)
		{
			if ( !bResult && GWarn->YesNof( TEXT("Failed to compile %s:\r\n%s\r\n\r\nWould you like to try again?"), SourceFilename, Output.Errors.Num() ? *Output.Errors(0) : TEXT("<unknown error>") ) )
			{
				bRetry = TRUE;
			}
		}
		
#endif
	} while ( bRetry );

	return bResult;
}
