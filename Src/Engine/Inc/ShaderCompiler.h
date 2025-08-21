/*=============================================================================
	ShaderCompiler.h: Platform independent shader compilation definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

enum EShaderFrequency
{
	SF_Vertex		= 0,
	SF_Pixel		= 1,
	SF_NumBits		= 1
};

enum EShaderPlatform
{
	SP_PCD3D		= 0,
	SP_PS3			= 1,
	SP_XBOXD3D		= 2,
	SP_PCD3D_SM2	= 3,
	SP_PCD3D_SM2_POOR = 4, // NO FP BLEND, No HW PCF, AVA Specific -> HDR Integer Path, No Shadow

	SP_NumPlatforms = 5,
	SP_NumBits		= 3,
	SP_AllPlatforms	= SP_NumPlatforms,
	SP_MaxPlatforms	= 16,	// 이보다 AllPlatforms가 크면 안됨!! 여기 저기 문제 생길 수 있음
};

FORCEINLINE UBOOL IsPCPlatform( EShaderPlatform Platform )
{
	return Platform == SP_PCD3D || Platform == SP_PCD3D_SM2 || Platform == SP_PCD3D_SM2_POOR;
}

FORCEINLINE UBOOL IsSM2Platform( EShaderPlatform Platform )
{
	return Platform == SP_PCD3D_SM2 || Platform == SP_PCD3D_SM2_POOR;
}

FORCEINLINE UBOOL IsHDRIntegerPath( EShaderPlatform Platform )
{
	return Platform == SP_PCD3D_SM2_POOR;
}

FORCEINLINE INT GetExtShaderNum( EShaderPlatform ShaderPlatform )
{
	switch( ShaderPlatform )
	{
	case SP_PCD3D:	return 3;
	case SP_PCD3D_SM2: return 2;
	case SP_PCD3D_SM2_POOR: return 1;
	default: return 0;
	}
}

FORCEINLINE EShaderPlatform GetIntShaderPlatform( INT ExtShaderNum )
{
	for( INT i = 0 ; i < SP_NumPlatforms ; i++ )
		if( GetExtShaderNum( (EShaderPlatform)i ) == ExtShaderNum )
			return (EShaderPlatform)i;

	return SP_AllPlatforms;
}


extern INT GetDeviceShaderVersion();
FORCEINLINE UBOOL IsAvailShaderPlatform( EShaderPlatform ShaderPlatform )
{
// 다른 환경(엑박, 플스3)등이 필요할까 ?
	return IsPCPlatform(ShaderPlatform) && ( GetDeviceShaderVersion() > 2 || ShaderPlatform == SP_PCD3D_SM2_POOR );
}

/**
* Converts shader platform to human readable string. 
*
* @param ShaderPlatform	Shader platform enum
* @return text representation of enum
*/
extern const TCHAR* ShaderPlatformToText( EShaderPlatform ShaderPlatform );

struct FShaderTarget
{
	BITFIELD Frequency : SF_NumBits;
	BITFIELD Platform : SP_NumBits;
};

enum ECompilerFlags
{
	CFLAG_PreferFlowControl = 0,
	CFLAG_Debug = 1,
	CFLAG_AvoidFlowControl = 2
};

/**
 * A map of shader parameter names to registers allocated to that parameter.
 */
class FShaderParameterMap
{
public:
	UBOOL FindParameterAllocation(const TCHAR* ParameterName,WORD& OutBaseRegisterIndex,WORD& OutNumRegisters) const;
	void AddParameterAllocation(const TCHAR* ParameterName,WORD BaseRegisterIndex,WORD NumRegisters);
private:
	struct FParameterAllocation
	{
		WORD BaseRegisterIndex;
		WORD NumRegisters;
	};
	TMap<FString,FParameterAllocation> ParameterMap;
};

/**
 * The environment used to compile a shader.
 */
struct FShaderCompilerEnvironment
{
	TMap<FString,FString> IncludeFiles;
	TMap<FName,FString> Definitions;
	TArray<ECompilerFlags> CompilerFlags;
};

/**
 * The output of the shader compiler.
 */
struct FShaderCompilerOutput
{
	FShaderParameterMap ParameterMap;
	TArray<FString> Errors;
	FShaderTarget Target;
	TArray<BYTE> Code;
	UINT NumInstructions;
};

/**
 * Loads the shader file with the given name.
 * @return The contents of the shader file.
 */
extern FString LoadShaderSourceFile(const TCHAR* Filename);

/**
 * Compiles a shader by invoking the platform-specific shader compiler for the given platform.
 */
extern UBOOL CompileShader(
	const TCHAR* SourceFilename,
	const TCHAR* FunctionName,
	FShaderTarget Target,
	const FShaderCompilerEnvironment& Environment,
	FShaderCompilerOutput& Output,
	UBOOL bSilent
	);

/** The shader precompilers for each platform.  These are only set during the console shader compilation while cooking or in the PrecompileShaders commandlet. */
extern class FConsoleShaderPrecompiler* GConsoleShaderPrecompilers[SP_NumPlatforms];
