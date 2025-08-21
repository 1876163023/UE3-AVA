/*=============================================================================
	UnMaterialCompiler.cpp: Unreal base HLSL material compiler.
	Copyright ?2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"

TMap<FGuid, FHLSLMaterialCompiler::FShaderCachedCode> FHLSLMaterialCompiler::CachedMaterialCode;

struct FMaterialUserInputTime: FMaterialUserInput
{
	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		OutValue.R = GWorld->GetTimeSeconds();
	}
};

struct FMaterialUserInputParameter: FMaterialUserInput
{
	FName	ParameterName;
	UBOOL	Vector;

	FMaterialUserInputParameter(FName InParameterName,UBOOL InVector):
		ParameterName(InParameterName),
		Vector(InVector)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		OutValue.R = OutValue.G = OutValue.B = OutValue.A = 0;

		if(MaterialInstance)
		{
			if(Vector)
			{
				MaterialInstance->GetVectorValue(ParameterName, &OutValue);
			}
			else
			{
				MaterialInstance->GetScalarValue(ParameterName, &OutValue.R);
			}
		}
	}
	virtual UBOOL ShouldEmbedCode() { return 0; }
};

struct FMaterialUserInputSine: FMaterialUserInput
{
	FMaterialUserInput*		X;
	UBOOL					Cosine;

	FMaterialUserInputSine(FMaterialUserInput* InX,UBOOL InCosine):
		X(InX),
		Cosine(InCosine)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{ 
		// OutValue.GBA ! 0
		X->GetValue(MaterialInstance, OutValue);
		OutValue.R = Cosine ? appCos(OutValue.R) : appSin(OutValue.R);
	}
	virtual UBOOL ShouldEmbedCode() { return X->ShouldEmbedCode(); }
};


struct FMaterialUserInputSquareRoot: FMaterialUserInput
{
	FMaterialUserInput*	X;

	FMaterialUserInputSquareRoot(FMaterialUserInput* InX):
		X(InX)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{ 
		// OutValue.GBA ! 0
		X->GetValue(MaterialInstance, OutValue);
		OutValue.R = appSqrt(OutValue.R);
	}
	virtual UBOOL ShouldEmbedCode() { return X->ShouldEmbedCode(); }
};

enum EFoldedMathOperation
{
	FMO_Add,
	FMO_Sub,
	FMO_Mul,
	FMO_Div,
	FMO_Dot
};

struct FMaterialUserInputFoldedMath: FMaterialUserInput
{
	FMaterialUserInput*		A;
	FMaterialUserInput*		B;
	EFoldedMathOperation	Op;

	FMaterialUserInputFoldedMath(FMaterialUserInput* InA,FMaterialUserInput* InB,EFoldedMathOperation InOp):
		A(InA),
		B(InB),
		Op(InOp)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		FLinearColor ValueA, ValueB;
		A->GetValue(MaterialInstance, ValueA);
		B->GetValue(MaterialInstance, ValueB);

		switch(Op)
		{
			case FMO_Add: OutValue = ValueA + ValueB; break;
			case FMO_Sub: OutValue = ValueA - ValueB; break;
			case FMO_Mul: OutValue = ValueA * ValueB; break;
			case FMO_Div: 
				OutValue.R = ValueA.R / ValueB.R;
				OutValue.G = ValueA.G / ValueB.G;
				OutValue.B = ValueA.B / ValueB.B;
				OutValue.A = ValueA.A / ValueB.A;
				break;
			case FMO_Dot: 
				{
					FLOAT DotProduct = ValueA.R * ValueB.R + ValueA.G * ValueB.G + ValueA.B * ValueB.B + ValueA.A * ValueB.A;
					OutValue.R = OutValue.G = OutValue.B = OutValue.A = DotProduct;
				}
				break;
			default: appErrorf(TEXT("Unknown folded math operation: %08x"),(INT)Op);
		};
	}
	virtual UBOOL ShouldEmbedCode() 
	{ 
		return A->ShouldEmbedCode() && B->ShouldEmbedCode(); 
	}
};

//
//	FMaterialUserInputPeriodic - A hint that only the fractional part of this input matters.
//

struct FMaterialUserInputPeriodic: FMaterialUserInput
{
	FMaterialUserInput*	A;

	FMaterialUserInputPeriodic(FMaterialUserInput* InA):
		A(InA)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		A->GetValue(MaterialInstance, OutValue);

		OutValue.R = OutValue.R - appFloor(OutValue.R);
		OutValue.G = OutValue.G - appFloor(OutValue.G);
		OutValue.B = OutValue.B - appFloor(OutValue.B);
		OutValue.A = OutValue.A - appFloor(OutValue.A);
	}
};

struct FMaterialUserInputAppendVector: FMaterialUserInput
{
	FMaterialUserInput*	A;
	FMaterialUserInput*	B;
	UINT				NumComponentsA;

	FMaterialUserInputAppendVector(FMaterialUserInput* InA,FMaterialUserInput* InB,UINT InNumComponentsA):
		A(InA),
		B(InB),
		NumComponentsA(InNumComponentsA)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		FLinearColor	ValueA, ValueB;
		A->GetValue(MaterialInstance, ValueA);
		B->GetValue(MaterialInstance, ValueB);

		OutValue.R = NumComponentsA >= 1 ? ValueA.R : (&ValueB.R)[0 - NumComponentsA];
		OutValue.G = NumComponentsA >= 2 ? ValueA.G : (&ValueB.R)[1 - NumComponentsA];
		OutValue.B = NumComponentsA >= 3 ? ValueA.B : (&ValueB.R)[2 - NumComponentsA];
		OutValue.A = NumComponentsA >= 4 ? ValueA.A : (&ValueB.R)[3 - NumComponentsA];
	}
	virtual UBOOL ShouldEmbedCode() { return A->ShouldEmbedCode() && B->ShouldEmbedCode(); }
};

struct FMaterialUserInputMin: FMaterialUserInput
{
	FMaterialUserInput*	A;
	FMaterialUserInput*	B;

	FMaterialUserInputMin(FMaterialUserInput* InA,FMaterialUserInput* InB):
		A(InA),
		B(InB)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		FLinearColor	ValueA, ValueB;
		A->GetValue(MaterialInstance, ValueA);
		B->GetValue(MaterialInstance, ValueB);

		OutValue.R = Min(ValueA.R, ValueB.R);
		OutValue.G = Min(ValueA.G, ValueB.G);
		OutValue.B = Min(ValueA.B, ValueB.B);
		OutValue.A = Min(ValueA.A, ValueB.A);
	}
	virtual UBOOL ShouldEmbedCode() { return A->ShouldEmbedCode() && B->ShouldEmbedCode(); }
};

struct FMaterialUserInputMax: FMaterialUserInput
{
	FMaterialUserInput*	A;
	FMaterialUserInput*	B;

	FMaterialUserInputMax(FMaterialUserInput* InA,FMaterialUserInput* InB):
		A(InA),
		B(InB)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		FLinearColor	ValueA, ValueB;
		A->GetValue(MaterialInstance, ValueA);
		B->GetValue(MaterialInstance, ValueB);

		OutValue.R = Max(ValueA.R, ValueB.R);
		OutValue.G = Max(ValueA.G, ValueB.G);
		OutValue.B = Max(ValueA.B, ValueB.B);
		OutValue.A = Max(ValueA.A, ValueB.A);
	}
	virtual UBOOL ShouldEmbedCode() { return A->ShouldEmbedCode() && B->ShouldEmbedCode(); }
};

struct FMaterialUserInputClamp: FMaterialUserInput
{
	FMaterialUserInput*	Input;
	FMaterialUserInput*	Min;
	FMaterialUserInput*	Max;

	FMaterialUserInputClamp(FMaterialUserInput* InInput,FMaterialUserInput* InMin,FMaterialUserInput* InMax):
		Input(InInput),
		Min(InMin),
		Max(InMax)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		FLinearColor	ValueMin, ValueMax, ValueInput;
		Min->GetValue(MaterialInstance, ValueMin);
		Max->GetValue(MaterialInstance, ValueMax);
		Input->GetValue(MaterialInstance, ValueInput);

		OutValue.R = Clamp(ValueInput.R, ValueMin.R, ValueMax.R);
		OutValue.G = Clamp(ValueInput.G, ValueMin.G, ValueMax.G);
		OutValue.B = Clamp(ValueInput.B, ValueMin.B, ValueMax.B);
		OutValue.A = Clamp(ValueInput.A, ValueMin.A, ValueMax.A);
	}
	virtual UBOOL ShouldEmbedCode() { return Input->ShouldEmbedCode() && Min->ShouldEmbedCode() && Max->ShouldEmbedCode(); }
};

struct FMaterialUserInputFloor: FMaterialUserInput
{
	FMaterialUserInput*	X;

	FMaterialUserInputFloor(FMaterialUserInput* InX):
		X(InX)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		X->GetValue(MaterialInstance, OutValue);

		OutValue.R = appFloor(OutValue.R);
		OutValue.G = appFloor(OutValue.G);
		OutValue.B = appFloor(OutValue.B);
		OutValue.A = appFloor(OutValue.A);
	}
	virtual UBOOL ShouldEmbedCode() { return X->ShouldEmbedCode(); }
};

struct FMaterialUserInputCeil: FMaterialUserInput
{
	FMaterialUserInput*	X;

	FMaterialUserInputCeil(FMaterialUserInput* InX):
		X(InX)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		X->GetValue(MaterialInstance, OutValue);

		OutValue.R = appCeil(OutValue.R);
		OutValue.G = appCeil(OutValue.G);
		OutValue.B = appCeil(OutValue.B);
		OutValue.A = appCeil(OutValue.A);
	}
	virtual UBOOL ShouldEmbedCode() { return X->ShouldEmbedCode(); }
};

struct FMaterialUserInputFrac: FMaterialUserInput
{
	FMaterialUserInput*	X;

	FMaterialUserInputFrac(FMaterialUserInput* InX):
		X(InX)
	{}

	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		X->GetValue(MaterialInstance, OutValue);

		OutValue.R = OutValue.R - appFloor(OutValue.R);
		OutValue.G = OutValue.G - appFloor(OutValue.G);
		OutValue.B = OutValue.B - appFloor(OutValue.B);
		OutValue.A = OutValue.A - appFloor(OutValue.A);
	}
	virtual UBOOL ShouldEmbedCode() { return X->ShouldEmbedCode(); }
};

/**
 * Absolute value evaluator for a given input expression
 */
struct FMaterialUserInputAbs: FMaterialUserInput
{
	/**
	 * Input expression to be evaluated
	 */
	FMaterialUserInput*	X;

	/**
	 * Constructor
	 *
	 * @param	InX - input expression
	 */
	FMaterialUserInputAbs( FMaterialUserInput* InX ):
		X(InX)
	{}

	/**
	* Creates the new shader code chunk needed for the Abs expression
	*
	* @param	MaterialInstance - access to the value for the user input instance
	* @return	Resulting color
	*/	
	virtual void GetValue(FMaterialInstance* MaterialInstance, FLinearColor& OutValue)
	{
		X->GetValue(MaterialInstance, OutValue);
		OutValue.R = Abs<FLOAT>(OutValue.R);
		OutValue.G = Abs<FLOAT>(OutValue.G);
		OutValue.B = Abs<FLOAT>(OutValue.B);
		OutValue.A = Abs<FLOAT>(OutValue.A);
	}

	/**
	* Returns true if the code chunk for this user input should be added
	*/	
	virtual UBOOL ShouldEmbedCode() { return X->ShouldEmbedCode(); }
};


const TCHAR* FHLSLMaterialCompiler::DescribeType(EMaterialCodeType Type) const
{
	switch(Type)
	{
	case MCT_Float1:		return TEXT("float1");
	case MCT_Float2:		return TEXT("float2");
	case MCT_Float3:		return TEXT("float3");
	case MCT_Float4:		return TEXT("float4");
	case MCT_Float:			return TEXT("float");
	case MCT_Texture2D:		return TEXT("texture2D");
	case MCT_TextureCube:	return TEXT("textureCube");
	case MCT_Texture3D:		return TEXT("texture3D");
	default: appErrorf(TEXT("Trying to describe invalid type: %u"),(UINT)Type);
	};
	return NULL;
}

// AddCodeChunk - Adds a formatted code string to the Code array and returns its index.
INT FHLSLMaterialCompiler::AddCodeChunk(EMaterialCodeType Type,DWORD Flags,const TCHAR* Format,...)
{
	INT		BufferSize		= 256;
	TCHAR*	FormattedCode	= NULL;
	INT		Result			= -1;

	if(bGenerateCode)
	{
		while(Result == -1)
		{
			FormattedCode = (TCHAR*) appRealloc( FormattedCode, BufferSize * sizeof(TCHAR) );
			GET_VARARGS_RESULT(FormattedCode,BufferSize - 1,Format,Format,Result);
			BufferSize *= 2;
		};
		FormattedCode[Result] = 0;
	}
	else
	{
		FormattedCode = TEXT("");
	}

	INT	CodeIndex = CodeChunks.Num();
	new(CodeChunks) FShaderCodeChunk(FormattedCode,Type,Flags);

	if( bGenerateCode )
	{
		appFree(FormattedCode);
	}
	return CodeIndex;
}

// AddUserInput - Adds an input to the Code array and returns its index.
INT FHLSLMaterialCompiler::AddUserInput(FMaterialUserInput* UserInput,EMaterialCodeType Type,const TCHAR* Format,...)
{
	INT		BufferSize		= 256;
	TCHAR*	FormattedCode	= NULL;
	INT		Result			= -1;

	if(bGenerateCode)
	{
		while(Result == -1)
		{
			FormattedCode = (TCHAR*) appRealloc( FormattedCode, BufferSize * sizeof(TCHAR) );
			GET_VARARGS_RESULT(FormattedCode,BufferSize - 1,Format,Format,Result);
			BufferSize *= 2;
		};
		FormattedCode[Result] = 0;
	}
	else
	{
		FormattedCode = TEXT("");
	}

	UserInputs.AddItem(UserInput);

	INT	CodeIndex = CodeChunks.Num();
	new(CodeChunks) FShaderCodeChunk(UserInput,FormattedCode,Type,0);

	if( bGenerateCode )
	{
		appFree(FormattedCode);
	}
	return CodeIndex;
}

// AddUserConstant - Adds an input for a constant, reusing existing instances of the constant.
INT FHLSLMaterialCompiler::AddUserConstant(const FLinearColor& Constant,EMaterialCodeType Type,const TCHAR* Format,...)
{
	INT		BufferSize		= 256;
	TCHAR*	FormattedCode	= NULL;
	INT		Result			= -1;

	if(bGenerateCode)
	{
		while(Result == -1)
		{
			FormattedCode = (TCHAR*) appRealloc( FormattedCode, BufferSize * sizeof(TCHAR) );
			GET_VARARGS_RESULT(FormattedCode,BufferSize - 1,Format,Format,Result);
			BufferSize *= 2;
		};
		FormattedCode[Result] = 0;
	}
	else
	{
		FormattedCode = TEXT("");
	}

	FMaterialUserInputConstant*	UserInput = NULL;
	for(UINT ConstantIndex = 0;ConstantIndex < (UINT)UserConstants.Num();ConstantIndex++)
	{
		if(UserConstants(ConstantIndex)->Value == Constant)
		{
			UserInput = UserConstants(ConstantIndex);
			break;
		}
	}

	if(!UserInput)
	{
		UserInput = new FMaterialUserInputConstant(Constant,Type == MCT_Float4 ? 4 : (Type == MCT_Float3 ? 3 : (Type == MCT_Float2 ? 2 : 1)));
		UserInputs.AddItem(UserInput);
		UserConstants.AddItem(UserInput);
	}

	INT	CodeIndex = CodeChunks.Num();
	new(CodeChunks) FShaderCodeChunk(UserInput,FormattedCode,Type,0);
	
	if( bGenerateCode )
	{
		appFree(FormattedCode);
	}
	return CodeIndex;
}

// AccessUserInput - Adds code to access an input to the Code array and returns its index.
INT FHLSLMaterialCompiler::AccessUserInput(INT Index)
{
	check(Index >= 0 && Index < CodeChunks.Num());

	const FShaderCodeChunk&	CodeChunk = CodeChunks(Index);

	check(CodeChunk.Input);

	TCHAR* FormattedCode = NULL;
	if( bGenerateCode )
	{
		// This malloc assumes that the below appSprintf won't use more.
		FormattedCode = (TCHAR*) appMalloc( 50 * sizeof(TCHAR) );
	}
	else
	{
		FormattedCode = TEXT("");
	}

	if(CodeChunk.Type == MCT_Float)
	{
		INT	ScalarInputIndex = UserScalarInputs.AddUniqueItem(CodeChunk.Input);
		if( bGenerateCode )
		{
			// Update the above appMalloc if this appSprintf grows in size, e.g. %s, ...
			appSprintf(FormattedCode,TEXT("UserScalarInputs[%u][%u]"),ScalarInputIndex / 4,ScalarInputIndex & 3);
		}
	}
	else if(CodeChunk.Type & MCT_Float)
	{
		INT				VectorInputIndex = UserVectorInputs.AddUniqueItem(CodeChunk.Input);
		const TCHAR*	Mask;
		switch(CodeChunk.Type)
		{
		case MCT_Float:
		case MCT_Float1: Mask = TEXT(".r"); break;
		case MCT_Float2: Mask = TEXT(".rg"); break;
		case MCT_Float3: Mask = TEXT(".rgb"); break;
		default: Mask = TEXT(""); break;
		};
		if( bGenerateCode )
		{
			appSprintf(FormattedCode,TEXT("UserVectorInputs[%u]%s"),VectorInputIndex,Mask);
		}
	}
	else
	{
		appErrorf(TEXT("User input of unknown type: %s"),DescribeType(CodeChunk.Type));
	}

	INT	CodeIndex = CodeChunks.Num();
	new(CodeChunks) FShaderCodeChunk(FormattedCode,CodeChunks(Index).Type,0);

	if( bGenerateCode )
	{
		appFree(FormattedCode);
	}
	return CodeIndex;
}

// GetParameterCode

const TCHAR* FHLSLMaterialCompiler::GetParameterCode(INT Index)
{
	check(Index >= 0 && Index < CodeChunks.Num());
	if(!CodeChunks(Index).Input || CodeChunks(Index).Input->ShouldEmbedCode())
	{
		return *CodeChunks(Index).Code;
	}
	else
	{
		return *CodeChunks(AccessUserInput(Index)).Code;
	}
}

// CoerceParameter
FString FHLSLMaterialCompiler::CoerceParameter(INT Index,EMaterialCodeType DestType)
{
	check(Index >= 0 && Index < CodeChunks.Num());
	const FShaderCodeChunk&	CodeChunk = CodeChunks(Index);
	if( CodeChunk.Type == DestType )
	{
		return GetParameterCode(Index);
	}
	else
	if( (CodeChunk.Type & DestType) && (CodeChunk.Type & MCT_Float) )
	{
		switch( DestType )
		{
		case MCT_Float1:
			return FString::Printf( TEXT("float(%s)"), GetParameterCode(Index) );
		case MCT_Float2:
			return FString::Printf( TEXT("float2(%s,%s)"), GetParameterCode(Index), GetParameterCode(Index) );
		case MCT_Float3:
			return FString::Printf( TEXT("float3(%s,%s,%s)"), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index) );
		case MCT_Float4:
			return FString::Printf( TEXT("float4(%s,%s,%s,%s)"), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index), GetParameterCode(Index) );
		default: 
			return FString::Printf( TEXT("%s"), GetParameterCode(Index) );
		}
	}
	else
	{
		Errorf(TEXT("Coercion failed: %s: %s -> %s"),*CodeChunk.Code,DescribeType(CodeChunk.Type),DescribeType(DestType));
		return TEXT("");
	}
}

// GetFixedParameterCode
const TCHAR* FHLSLMaterialCompiler::GetFixedParameterCode(INT Index) const
{
	check(Index >= 0 && Index < CodeChunks.Num());
	check(!CodeChunks(Index).Input || CodeChunks(Index).Input->ShouldEmbedCode());
	return *CodeChunks(Index).Code;
}

// GetParameterType
EMaterialCodeType FHLSLMaterialCompiler::GetParameterType(INT Index) const
{
	check(Index >= 0 && Index < CodeChunks.Num());
	return CodeChunks(Index).Type;
}

// GetParameterInput
FMaterialUserInput* FHLSLMaterialCompiler::GetParameterInput(INT Index) const
{
	check(Index >= 0 && Index < CodeChunks.Num());
	return CodeChunks(Index).Input;
}

// GetParameterFlags
DWORD FHLSLMaterialCompiler::GetParameterFlags(INT Index) const
{
	check(Index >= 0 && Index < CodeChunks.Num());
	return CodeChunks(Index).Flags;
}

// GetArithmeticResultType
EMaterialCodeType FHLSLMaterialCompiler::GetArithmeticResultType(EMaterialCodeType TypeA,EMaterialCodeType TypeB)
{
	if(!(TypeA & MCT_Float) || !(TypeB & MCT_Float))
	{
		Errorf(TEXT("Attempting to perform arithmetic on non-numeric types: %s %s"),DescribeType(TypeA),DescribeType(TypeB));
	}

	if(TypeA == TypeB)
	{
		return TypeA;
	}
	else if(TypeA & TypeB)
	{
		if(TypeA == MCT_Float)
		{
			return TypeB;
		}
		else
		{
			check(TypeB == MCT_Float);
			return TypeA;
		}
	}
	else
	{
		Errorf(TEXT("Arithmetic between types %s and %s are undefined"),DescribeType(TypeA),DescribeType(TypeB));
		return MCT_Float;
	}
}

EMaterialCodeType FHLSLMaterialCompiler::GetArithmeticResultType(INT A,INT B)
{
	check(A >= 0 && A < CodeChunks.Num());
	check(B >= 0 && B < CodeChunks.Num());

	EMaterialCodeType	TypeA = CodeChunks(A).Type,
						TypeB = CodeChunks(B).Type;
	
	return GetArithmeticResultType(TypeA,TypeB);
}

// GetNumComponents - Returns the number of components in a vector type.
UINT FHLSLMaterialCompiler::GetNumComponents(EMaterialCodeType Type)
{
	switch(Type)
	{
		case MCT_Float:
		case MCT_Float1: return 1;
		case MCT_Float2: return 2;
		case MCT_Float3: return 3;
		case MCT_Float4: return 4;
		default: Errorf(TEXT("Attempting to get component count of non-vector type %s"),DescribeType(Type));
	}
	return 0;
}

// GetVectorType - Returns the vector type containing a given number of components.
EMaterialCodeType FHLSLMaterialCompiler::GetVectorType(UINT NumComponents)
{
	switch(NumComponents)
	{
		case 1: return MCT_Float;
		case 2: return MCT_Float2;
		case 3: return MCT_Float3;
		case 4: return MCT_Float4;
		default: Errorf(TEXT("Requested %u component vector type does not exist"),NumComponents);
	};
	return MCT_Unknown;
}

// FMaterialCompiler interface.

INT FHLSLMaterialCompiler::Error(const TCHAR* Text)
{
	if(GuardStack.Num())
	{
		GuardStack(GuardStack.Num() - 1)->Error(Text);
	}
#if EXCEPTIONS_DISABLED
	debugf(TEXT("Compiler error: %s"), Text);
	appDebugBreak();
	return INDEX_NONE;
#else
	throw Text;
	return INDEX_NONE;
#endif
}

void FHLSLMaterialCompiler::EnterGuard(FMaterialCompilerGuard* Guard)
{
	GuardStack.AddItem(Guard);
}

void FHLSLMaterialCompiler::ExitGuard()
{
	check(GuardStack.Num());
	GuardStack.Remove(GuardStack.Num() - 1);
}

/**
 * Returns the index of the selection color vector parameter.
 *
 * @return index of selection color vector parameter
 */
INT FHLSLMaterialCompiler::GetSelectionColorIndex()
{
	if( SelectionColorIndex == INDEX_NONE )
	{
		SelectionColorIndex = ComponentMask(VectorParameter(NAME_SelectionColor),1,1,1,0);
	}
	return SelectionColorIndex;
}

EMaterialCodeType FHLSLMaterialCompiler::GetType(INT Code)
{
	return GetParameterType(Code);
}

INT FHLSLMaterialCompiler::ForceCast(INT Code,EMaterialCodeType DestType)
{
	if(GetParameterInput(Code) && !GetParameterInput(Code)->ShouldEmbedCode())
	{		
		return ForceCast(AccessUserInput(Code),DestType);
	}

	EMaterialCodeType	SourceType = GetParameterType(Code);

	if(SourceType & DestType)
	{
		return Code;
	}
	else if((SourceType & MCT_Float) && (DestType & MCT_Float))
	{
		UINT	NumSourceComponents = GetNumComponents(SourceType),
				NumDestComponents = GetNumComponents(DestType);

		if(NumSourceComponents > NumDestComponents) // Use a mask to select the first NumDestComponents components from the source.
		{
			const TCHAR*	Mask;
			switch(NumDestComponents)
			{
				case 1: Mask = TEXT(".r"); break;
				case 2: Mask = TEXT(".rg"); break;
				case 3: Mask = TEXT(".rgb"); break;
				default: appErrorf(TEXT("Should never get here!")); return INDEX_NONE;
			};

			return AddCodeChunk(DestType,0,TEXT("%s%s"),GetParameterCode(Code),Mask);
		}
		else if(NumSourceComponents < NumDestComponents) // Pad the source vector up to NumDestComponents.
		{
			UINT	NumPadComponents = NumDestComponents - NumSourceComponents;
			return AddCodeChunk(
				DestType,
				0,
				TEXT("%s(%s%s%s%s)"),
				DescribeType(DestType),
				GetParameterCode(Code),
				NumPadComponents >= 1 ? TEXT(",0") : TEXT(""),
				NumPadComponents >= 2 ? TEXT(",0") : TEXT(""),
				NumPadComponents >= 3 ? TEXT(",0") : TEXT("")
				);
		}
		else
		{
			return Code;
		}
	}
	else
	{
		return Errorf(TEXT("Cannot force a cast between non-numeric types."));
	}
}

INT FHLSLMaterialCompiler::VectorParameter(FName ParameterName)
{
	return AddUserInput(new FMaterialUserInputParameter(ParameterName,1),MCT_Float4,TEXT(""));
}

INT FHLSLMaterialCompiler::ScalarParameter(FName ParameterName)
{
	return AddUserInput(new FMaterialUserInputParameter(ParameterName,0),MCT_Float,TEXT(""));
}

INT FHLSLMaterialCompiler::Constant(FLOAT X)
{
	return AddUserConstant(FLinearColor(X,0,0,0),MCT_Float,TEXT("(%0.8f)"),X);
}

INT FHLSLMaterialCompiler::Constant2(FLOAT X,FLOAT Y)
{
	return AddUserConstant(FLinearColor(X,Y,0,0),MCT_Float2,TEXT("float2(%0.8f,%0.8f)"),X,Y);
}

INT FHLSLMaterialCompiler::Constant3(FLOAT X,FLOAT Y,FLOAT Z)
{
	return AddUserConstant(FLinearColor(X,Y,Z,0),MCT_Float3,TEXT("float3(%0.8f,%0.8f,%0.8f)"),X,Y,Z);
}

INT FHLSLMaterialCompiler::Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W)
{
	return AddUserConstant(FLinearColor(X,Y,Z,W),MCT_Float4,TEXT("float4(%0.8f,%0.8f,%0.8f,%0.8f)"),X,Y,Z,W);
}

void FHLSLMaterialCompiler::AddTexture(FTextureBase* Texture, INT& TextureIndex)
{
	TextureIndex = Textures.AddUniqueItem(Texture);
	
	if (TextureNames.Num() <= TextureIndex)
	{
		TextureNames.Insert(TextureIndex);
	}
	TextureNames(TextureIndex) = NAME_None;

	if (UsingTextureParameters.Num() <= TextureIndex)
	{
		UsingTextureParameters.Insert(TextureIndex);
	}
	UsingTextureParameters(TextureIndex) = 0;
}

INT FHLSLMaterialCompiler::Texture(FTextureBase* Texture, INT& TextureIndex)
{
	AddTexture(Texture, TextureIndex);

	FResourceType*		ResourceType = Texture->Type();
	EMaterialCodeType	ShaderType;
	if(ResourceType->IsA(FTexture2D::StaticType))
	{
		ShaderType = MCT_Texture2D;
	}
	else if(ResourceType->IsA(FTextureCube::StaticType))
	{
		ShaderType = MCT_TextureCube;
	}
	else if(ResourceType->IsA(FTexture3D::StaticType))
	{
		ShaderType = MCT_Texture3D;
	}
	else if( ResourceType->IsA(FRenderTargetCube::StaticType) )
	{
		ShaderType = MCT_TextureCube;
	}
	else if( ResourceType->IsA(FRenderTarget::StaticType) )
	{
		ShaderType = MCT_Texture2D;
	}	
	else
	{
		Errorf(TEXT("Unsupported texture type used: %s"),ResourceType->Name);
		return INDEX_NONE;
	}

	DWORD Flags = 0;
	if( Texture->IsRGBE() )
	{
		Flags |= Texture->Format == PF_A8R8G8B8 ? SCF_RGBE_8BIT_EXPONENT : SCF_RGBE_4BIT_EXPONENT;
	}
	if( (GPixelFormats[Texture->Format].Flags & PF_REQUIRES_GAMMA_CORRECTION) && Texture->IsGammaCorrected() )
	{
		Flags |= SCF_REQUIRES_GAMMA_CORRECTION;
	}
	return AddCodeChunk(ShaderType,Flags,TEXT("UserTextures%u"),TextureIndex);
}

INT FHLSLMaterialCompiler::TextureUnpackMin(INT TextureIndex)
{
	return AddCodeChunk(MCT_Float4, 0, TEXT("UserTexturesMins[%u]"), TextureIndex);
}

INT FHLSLMaterialCompiler::TextureUnpackMaxMinusMin(INT TextureIndex)
{
	return AddCodeChunk(MCT_Float4, 0, TEXT("UserTexturesMaxMinusMins[%u]"), TextureIndex);
}

INT FHLSLMaterialCompiler::TextureOffsetParameter()
{
	UsingTextureOffset = true;
	return AddCodeChunk(MCT_Float4, 0, TEXT("UserTextureOffset"));
}

INT	FHLSLMaterialCompiler::TextureScaleParameter()
{
	UsingTextureScale = true;
	return AddCodeChunk(MCT_Float4, 0, TEXT("UserTextureScale"));
}

INT FHLSLMaterialCompiler::FlipBookOffset(INT TextureIndex)
{
	return AddCodeChunk(MCT_Float4, 0, TEXT("UserFlipBookOffset[%u]"), TextureIndex);
}

INT FHLSLMaterialCompiler::FlipBookScale(INT TextureIndex)
{
	return AddCodeChunk(MCT_Float4, 0, TEXT("UserFlipBookScale[%u]"), TextureIndex);
}

INT FHLSLMaterialCompiler::SceneTime()
{
	return AddUserInput(new FMaterialUserInputTime(),MCT_Float,TEXT("SceneTime"));
}

INT FHLSLMaterialCompiler::PeriodicHint(INT PeriodicCode)
{
	if(GetParameterInput(PeriodicCode))
	{
		return AddUserInput(new FMaterialUserInputPeriodic(GetParameterInput(PeriodicCode)),GetParameterType(PeriodicCode),TEXT("%s"),GetParameterCode(PeriodicCode));
	}
	else
	{
		return PeriodicCode;
	}
}

INT FHLSLMaterialCompiler::Sine(INT X)
{
	if(GetParameterInput(X))
	{
		return AddUserInput(new FMaterialUserInputSine(GetParameterInput(X),0),MCT_Float,TEXT("sin(%s)"),*CoerceParameter(X,MCT_Float));
	}
	else
	{
		return AddCodeChunk(MCT_Float,0,TEXT("sin(%s)"),*CoerceParameter(X,MCT_Float));
	}
}

INT FHLSLMaterialCompiler::Cosine(INT X)
{
	if(GetParameterInput(X))
	{
		return AddUserInput(new FMaterialUserInputSine(GetParameterInput(X),1),MCT_Float,TEXT("cos(%s)"),*CoerceParameter(X,MCT_Float));
	}
	else
	{
		return AddCodeChunk(MCT_Float,0,TEXT("cos(%s)"),*CoerceParameter(X,MCT_Float));
	}
}

INT FHLSLMaterialCompiler::Floor(INT X)
{
	if(GetParameterInput(X))
	{
		return AddUserInput(new FMaterialUserInputFloor(GetParameterInput(X)),GetParameterType(X),TEXT("floor(%s)"),GetParameterCode(X));
	}
	else
	{
		return AddCodeChunk(GetParameterType(X),0,TEXT("floor(%s)"),GetParameterCode(X));
	}
}

INT FHLSLMaterialCompiler::Ceil(INT X)
{
	if(GetParameterInput(X))
	{
		return AddUserInput(new FMaterialUserInputCeil(GetParameterInput(X)),GetParameterType(X),TEXT("ceil(%s)"),GetParameterCode(X));
	}
	else
	{
		return AddCodeChunk(GetParameterType(X),0,TEXT("ceil(%s)"),GetParameterCode(X));
	}
}

INT FHLSLMaterialCompiler::Frac(INT X)
{
	if(GetParameterInput(X))
	{
		return AddUserInput(new FMaterialUserInputFrac(GetParameterInput(X)),GetParameterType(X),TEXT("frac(%s)"),GetParameterCode(X));
	}
	else
	{
		return AddCodeChunk(GetParameterType(X),0,TEXT("frac(%s)"),GetParameterCode(X));
	}
}

/**
 * Creates the new shader code chunk needed for the Abs expression
 *
 * @param	X - Index to the FMaterialCompiler::CodeChunk entry for the input expression
 * @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
 */	
INT FHLSLMaterialCompiler::Abs( INT X )
{
	// get the user input struct for the input expression
	FMaterialUserInput* pInputParam = GetParameterInput(X);
	if( pInputParam )
	{
		FMaterialUserInputAbs* pMatUserInput = new FMaterialUserInputAbs( pInputParam );
		return AddUserInput( pMatUserInput, GetParameterType(X), TEXT("abs(%s)"), GetParameterCode(X) );
	}
	else
	{
		return AddCodeChunk( GetParameterType(X), 0, TEXT("abs(%s)"), GetParameterCode(X) );
	}
}

INT FHLSLMaterialCompiler::ReflectionVector()
{
	return AddCodeChunk(MCT_Float3,0,TEXT("Input.TangentReflectionVector"));
}

INT FHLSLMaterialCompiler::CameraVector()
{
	return AddCodeChunk(MCT_Float3,0,TEXT("Input.TangentCameraVector"));
}

INT FHLSLMaterialCompiler::LightVector()
{
	return AddCodeChunk(MCT_Float3,0,TEXT("Input.TangentLightVector"));
}

INT FHLSLMaterialCompiler::ScreenPosition(  UBOOL bScreenAlign )
{
	if( bScreenAlign )
	{
		return AddCodeChunk(MCT_Float4,0,TEXT("screenAlignedPosition(Input.ScreenPosition)"));		
	}
	else
	{
		return AddCodeChunk(MCT_Float4,0,TEXT("Input.ScreenPosition"));		
	}	
}

INT FHLSLMaterialCompiler::If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB)
{
	EMaterialCodeType ResultType = GetArithmeticResultType(GetParameterType(AGreaterThanB),GetArithmeticResultType(AEqualsB,ALessThanB));

	INT CoercedAGreaterThanB = ForceCast(AGreaterThanB,ResultType);
	INT CoercedAEqualsB = ForceCast(AEqualsB,ResultType);
	INT CoercedALessThanB = ForceCast(ALessThanB,ResultType);

	return AddCodeChunk(
		ResultType,
		0,
		TEXT("((%s >= %s) ? (%s > %s ? %s : %s) : %s)"),
		GetParameterCode(A),
		GetParameterCode(B),
		GetParameterCode(A),
		GetParameterCode(B),
		GetParameterCode(CoercedAGreaterThanB),
		GetParameterCode(CoercedAEqualsB),
		GetParameterCode(CoercedALessThanB)
		);
}

INT FHLSLMaterialCompiler::TextureCoordinate(UINT CoordinateIndex)
{
	NumUserTexCoords = ::Max(CoordinateIndex + 1,NumUserTexCoords);
	return AddCodeChunk(MCT_Float2,0,TEXT("Input.UserTexCoords[%u].xy"),CoordinateIndex);
}

INT FHLSLMaterialCompiler::TextureSample(INT TextureIndex,INT CoordinateIndex)
{
	EMaterialCodeType	TextureType = GetParameterType(TextureIndex);
	DWORD				Flags		= GetParameterFlags(TextureIndex);

	FString				SampleCode;

	switch(TextureType)
	{
	case MCT_Texture2D:
		SampleCode = TEXT("tex2D(%s,%s)");
		break;
	case MCT_TextureCube:
		SampleCode = TEXT("texCUBE(%s,%s)");
		break;
	case MCT_Texture3D:
		SampleCode = TEXT("tex3D(%s,%s)");
		break;		
	}

	if( Flags & SCF_RGBE_4BIT_EXPONENT )
	{
		SampleCode = FString::Printf( TEXT("expandCompressedRGBE(%s)"), *SampleCode );
	}

	if( Flags & SCF_RGBE_8BIT_EXPONENT )
	{
		SampleCode = FString::Printf( TEXT("expandRGBE(%s)"), *SampleCode );
	}

	if( Flags & SCF_REQUIRES_GAMMA_CORRECTION )
	{
		SampleCode = FString::Printf( TEXT("gammaCorrect(%s)"), *SampleCode );
	}

	switch(TextureType)
	{
	case MCT_Texture2D:
		return AddCodeChunk(
				MCT_Float4,
				0,
				*SampleCode,
				*CoerceParameter(TextureIndex,MCT_Texture2D),
				*CoerceParameter(CoordinateIndex,MCT_Float2)
				);
	case MCT_TextureCube:
		return AddCodeChunk(
				MCT_Float4,
				0,
				*SampleCode,
				*CoerceParameter(TextureIndex,MCT_TextureCube),
				*CoerceParameter(CoordinateIndex,MCT_Float3)
				);
	case MCT_Texture3D:
		return AddCodeChunk(
				MCT_Float4,
				0,
				*SampleCode,
				*CoerceParameter(TextureIndex,MCT_Texture3D),
				*CoerceParameter(CoordinateIndex,MCT_Float3)
				);
	default:
		Errorf(TEXT("Sampling unknown texture type: %s"),DescribeType(TextureType));
		return INDEX_NONE;
	};
}

/**
* Add the shader code for sampling from the scene texture
*
* @param	CoordinateIdx - index of shader code for user specified tex coords
*/
INT FHLSLMaterialCompiler::SceneTextureSample( BYTE TexType, INT CoordinateIdx )
{
	// use the scene texture type
	FString SceneTexCode;
	switch(TexType)
	{
	case SceneTex_Lighting:
		SceneTexCode = FString(TEXT("PreviousLightingTexture"));
		break;
	default:
		Errorf(TEXT("Scene texture type not supported."));
		return INDEX_NONE;
	}
	// sampler
	FString	SampleCode( TEXT("tex2D(%s,%s)") );
	// replace default tex coords with user specified coords if available
	FString TexCoordCode( CoerceParameter( (CoordinateIdx != INDEX_NONE) ? CoordinateIdx : TextureCoordinate(0), MCT_Float2 ) );
	// add the code string
	return AddCodeChunk(
		MCT_Float4,
		0,
		*SampleCode,
		*SceneTexCode,
		*TexCoordCode
		);
}

/**
* Add the shader code for sampling the scene depth
*
* @param	CoordinateIdx - index of shader code for user specified tex coords
*/
INT FHLSLMaterialCompiler::SceneTextureDepth( UBOOL bNormalize, INT CoordinateIdx)
{
	// sampler
	FString	UserDepthCode( TEXT("calcSceneDepth(%s)") );

	if( bNormalize )
	{
		//@todo sz
	}

	// replace default tex coords with user specified coords if available
	FString TexCoordCode( CoerceParameter( (CoordinateIdx != INDEX_NONE) ? CoordinateIdx : TextureCoordinate(0), MCT_Float2 ) );
	// add the code string
	return AddCodeChunk(
		MCT_Float1,
		0,
		*UserDepthCode,
		*TexCoordCode
		);
}

INT FHLSLMaterialCompiler::PixelDepth(UBOOL bNormalize)
{
	return AddCodeChunk(MCT_Float1, 0, TEXT("Input.ScreenPosition.w"));		
}

INT FHLSLMaterialCompiler::DestColor()
{
	FString	UserColorCode(TEXT("previousLighting(%s)"));
	FString	ScreenPosCode(TEXT("Input.ScreenPosition"));
	// add the code string
	return AddCodeChunk(
		MCT_Float4,
		0,
		*UserColorCode,
		*ScreenPosCode
		);
}

INT FHLSLMaterialCompiler::DestDepth(UBOOL bNormalize)
{
	FString	UserDepthCode(TEXT("previousDepth(%s)"));
	FString	ScreenPosCode(TEXT("Input.ScreenPosition"));
	if( bNormalize )
	{
		//@todo sz
	}

	// add the code string
	return AddCodeChunk(
		MCT_Float1,
		0,
		*UserDepthCode,
		*ScreenPosCode
		);
}

void FHLSLMaterialCompiler::AddTextureParameter(FTextureBase* DefaultTexture, FName ParameterName, INT& TextureIndex)
{
	// See if the texture is already in the array
	INT FindIndex = -1;
	UBOOL bFound = FALSE;

	for (INT CheckIndex = 0; CheckIndex < Textures.Num(); CheckIndex++)
	{
		FTextureBase* CheckTexture = Textures(CheckIndex);
		if (CheckTexture == DefaultTexture)
		{
			FindIndex	= CheckIndex;
			if (TextureNames(FindIndex) == ParameterName)
			{
				// The same texture with the same parameter name is in the array... Don't re-add it.
				TextureIndex = FindIndex;
				bFound = TRUE;
			}
		}
	}

	if (bFound == FALSE)
	{
		// The texture was not found in any form
		// OR
		// The same texture with a different parameter name is in the array... Re-add it.
		TextureIndex = Textures.AddItem(DefaultTexture);
		TextureNames.Insert(TextureIndex);
		TextureNames(TextureIndex) = ParameterName;
	}

	if (UsingTextureParameters.Num() <= TextureIndex)
	{
		UsingTextureParameters.Insert(TextureIndex);
	}
	UsingTextureParameters(TextureIndex) = 1;
}

INT FHLSLMaterialCompiler::TextureParameter(FTextureBase* DefaultTexture, FName ParameterName, INT& TextureIndex)
{
	AddTextureParameter(DefaultTexture, ParameterName, TextureIndex);

	FResourceType*		ResourceType = DefaultTexture->Type();
	EMaterialCodeType	ShaderType;
	if (ResourceType->IsA(FTexture2D::StaticType))
	{
		ShaderType = MCT_Texture2D;
	}
	else
	if (ResourceType->IsA(FTextureCube::StaticType))
	{
		ShaderType = MCT_TextureCube;
	}
	else
	if (ResourceType->IsA(FTexture3D::StaticType))
	{
		ShaderType = MCT_Texture3D;
	}
	else if( ResourceType->IsA(FRenderTargetCube::StaticType) )
	{
		ShaderType = MCT_TextureCube;
	}
	else if( ResourceType->IsA(FRenderTarget::StaticType) )
	{
		ShaderType = MCT_Texture2D;
	}	
	else
	{
		Errorf(TEXT("Unsupported texture type used: %s"), ResourceType->Name);
		return INDEX_NONE;
	}

	DWORD Flags = 0;
	if (DefaultTexture->IsRGBE())
	{
		Flags |= (DefaultTexture->Format == PF_A8R8G8B8) ? SCF_RGBE_8BIT_EXPONENT : SCF_RGBE_4BIT_EXPONENT;
	}

	if ((GPixelFormats[DefaultTexture->Format].Flags & PF_REQUIRES_GAMMA_CORRECTION) && 
		DefaultTexture->IsGammaCorrected())
	{
		Flags |= SCF_REQUIRES_GAMMA_CORRECTION;
	}

	return AddCodeChunk(ShaderType,Flags,TEXT("UserTextures%u"),TextureIndex);
}

INT FHLSLMaterialCompiler::VertexColor()
{
	return AddCodeChunk(MCT_Float4,0,TEXT("Input.UserVertexColor"));
}

INT FHLSLMaterialCompiler::MeshEmitterVertexColor()
{
	UsingMeshEmitterVertexColor = true;
	return AddCodeChunk(MCT_Float4,0,TEXT("UserMeshEmitterVertexColor"));
}

INT FHLSLMaterialCompiler::Add(INT A,INT B)
{
	if(GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputFoldedMath(GetParameterInput(A),GetParameterInput(B),FMO_Add),GetArithmeticResultType(A,B),TEXT("(%s + %s)"),GetParameterCode(A),GetParameterCode(B));
	}
	else
	{
		return AddCodeChunk(GetArithmeticResultType(A,B),0,TEXT("(%s + %s)"),GetParameterCode(A),GetParameterCode(B));
	}
}

INT FHLSLMaterialCompiler::Sub(INT A,INT B)
{
	if(GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputFoldedMath(GetParameterInput(A),GetParameterInput(B),FMO_Sub),GetArithmeticResultType(A,B),TEXT("(%s - %s)"),GetParameterCode(A),GetParameterCode(B));
	}
	else
	{
		return AddCodeChunk(GetArithmeticResultType(A,B),0,TEXT("(%s - %s)"),GetParameterCode(A),GetParameterCode(B));
	}
}

INT FHLSLMaterialCompiler::Mul(INT A,INT B)
{
	if(GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputFoldedMath(GetParameterInput(A),GetParameterInput(B),FMO_Mul),GetArithmeticResultType(A,B),TEXT("(%s * %s)"),GetParameterCode(A),GetParameterCode(B));
	}
	else
	{
		return AddCodeChunk(GetArithmeticResultType(A,B),0,TEXT("(%s * %s)"),GetParameterCode(A),GetParameterCode(B));
	}
}

INT FHLSLMaterialCompiler::Div(INT A,INT B)
{
	if(GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputFoldedMath(GetParameterInput(A),GetParameterInput(B),FMO_Div),GetArithmeticResultType(A,B),TEXT("(%s / %s)"),GetParameterCode(A),GetParameterCode(B));
	}
	else
	{
		return AddCodeChunk(GetArithmeticResultType(A,B),0,TEXT("(%s / %s)"),GetParameterCode(A),GetParameterCode(B));
	}
}

INT FHLSLMaterialCompiler::Dot(INT A,INT B)
{
	if(GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputFoldedMath(GetParameterInput(A),GetParameterInput(B),FMO_Dot),MCT_Float,TEXT("dot(%s,%s)"),GetParameterCode(A),GetParameterCode(B));
	}
	else
	{
		return AddCodeChunk(MCT_Float,0,TEXT("dot(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
	}
}

INT FHLSLMaterialCompiler::Cross(INT A,INT B)
{
	return AddCodeChunk(MCT_Float3,0,TEXT("cross(%s,%s)"),*CoerceParameter(A,MCT_Float3),*CoerceParameter(B,MCT_Float3));
}

INT FHLSLMaterialCompiler::Power(INT Base,INT Exponent)
{
	return AddCodeChunk(GetParameterType(Base),0,TEXT("pow(%s,%s)"),GetParameterCode(Base),*CoerceParameter(Exponent,MCT_Float));;
}

INT FHLSLMaterialCompiler::SquareRoot(INT X)
{
	if(GetParameterInput(X))
	{
		return AddUserInput(new FMaterialUserInputSquareRoot(GetParameterInput(X)),MCT_Float,TEXT("sqrt(%s)"),*CoerceParameter(X,MCT_Float1));
	}
	else
	{
		return AddCodeChunk(MCT_Float,0,TEXT("sqrt(%s)"),*CoerceParameter(X,MCT_Float1));
	}
}

INT FHLSLMaterialCompiler::Lerp(INT X,INT Y,INT A)
{
	return AddCodeChunk(GetArithmeticResultType(X,Y),0,TEXT("lerp(%s,%s,%s)"),GetParameterCode(X),GetParameterCode(Y),*CoerceParameter(A,MCT_Float1));
}

INT FHLSLMaterialCompiler::Min(INT A,INT B)
{
	if(GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputMin(GetParameterInput(A),GetParameterInput(B)),GetParameterType(A),TEXT("min(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
	}
	else
	{
		return AddCodeChunk(GetParameterType(A),0,TEXT("min(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
	}
}

INT FHLSLMaterialCompiler::Max(INT A,INT B)
{
	if(GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputMax(GetParameterInput(A),GetParameterInput(B)),GetParameterType(A),TEXT("max(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
	}
	else
	{
		return AddCodeChunk(GetParameterType(A),0,TEXT("max(%s,%s)"),GetParameterCode(A),*CoerceParameter(B,GetParameterType(A)));
	}
}

INT FHLSLMaterialCompiler::Clamp(INT X,INT A,INT B)
{
	if(GetParameterInput(X) && GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputClamp(GetParameterInput(X),GetParameterInput(A),GetParameterInput(B)),GetParameterType(X),TEXT("clamp(%s,%s,%s)"),GetParameterCode(X),*CoerceParameter(A,GetParameterType(X)),*CoerceParameter(B,GetParameterType(X)));
	}
	else
	{
		return AddCodeChunk(GetParameterType(X),0,TEXT("clamp(%s,%s,%s)"),GetParameterCode(X),*CoerceParameter(A,GetParameterType(X)),*CoerceParameter(B,GetParameterType(X)));
	}
}

INT FHLSLMaterialCompiler::ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A)
{
	EMaterialCodeType	VectorType = GetParameterType(Vector);

	if(	A && (VectorType & MCT_Float) < MCT_Float4 ||
		B && (VectorType & MCT_Float) < MCT_Float3 ||
		G && (VectorType & MCT_Float) < MCT_Float2 ||
		R && (VectorType & MCT_Float) < MCT_Float1)
		Errorf(TEXT("Not enough components in (%s: %s) for component mask %u%u%u%u"),GetParameterCode(Vector),DescribeType(GetParameterType(Vector)),R,G,B,A);

	EMaterialCodeType	ResultType;
	switch((R ? 1 : 0) + (G ? 1 : 0) + (B ? 1 : 0) + (A ? 1 : 0))
	{
		case 1: ResultType = MCT_Float; break;
		case 2: ResultType = MCT_Float2; break;
		case 3: ResultType = MCT_Float3; break;
		case 4: ResultType = MCT_Float4; break;
		default: Errorf(TEXT("Couldn't determine result type of component mask %u%u%u%u"),R,G,B,A); return INDEX_NONE;
	};

	return AddCodeChunk(
		ResultType,
		0,
		TEXT("%s.%s%s%s%s"),
		GetParameterCode(Vector),
		R ? TEXT("r") : TEXT(""),
		G ? TEXT("g") : TEXT(""),
		B ? TEXT("b") : TEXT(""),
		A ? TEXT("a") : TEXT("")
		);
}

INT FHLSLMaterialCompiler::AppendVector(INT A,INT B)
{
	INT					NumResultComponents = GetNumComponents(GetParameterType(A)) + GetNumComponents(GetParameterType(B));
	EMaterialCodeType	ResultType = GetVectorType(NumResultComponents);

	if(GetParameterInput(A) && GetParameterInput(B))
	{
		return AddUserInput(new FMaterialUserInputAppendVector(GetParameterInput(A),GetParameterInput(B),GetNumComponents(GetParameterType(A))),ResultType,TEXT("float%u(%s,%s)"),NumResultComponents,GetParameterCode(A),GetParameterCode(B));
	}
	else
	{
		return AddCodeChunk(ResultType,0,TEXT("float%u(%s,%s)"),NumResultComponents,GetParameterCode(A),GetParameterCode(B));
	}
}

/**
* Generate shader code for transforming a vector
*
* @param	CoordType - type of transform to apply. see EMaterialVectorCoordTransform 
* @param	A - index for input vector parameter's code
*/
INT FHLSLMaterialCompiler::TransformVector(BYTE CoordType,INT A)
{
	INT Result = INDEX_NONE;

	INT NumInputComponents = GetNumComponents(GetParameterType(A));
	// only allow float3/float4 transforms
	if( NumInputComponents < 3 )
	{
		Result = Errorf(TEXT("input must be a vector (%s: %s)"),GetParameterCode(A),DescribeType(GetParameterType(A)));
	}
	else
	{
		// code string to transform the input vector
		FString CodeStr;
		switch( CoordType )
		{
		case TRANSFORM_World:
			// transform from tangent to world space
			UsingTransforms |= UsedCoord_World;
			CodeStr = FString(TEXT("mul(LocalToWorldMatrix,mul(Input.TangentBasisInverse,%s))"));			
			break;
		case TRANSFORM_View:
			// transform from tangent to view space
			UsingTransforms |= UsedCoord_View;
			CodeStr = FString(TEXT("mul(LocalToViewMatrix,mul(Input.TangentBasisInverse,%s))"));
			break;
		case TRANSFORM_Local:
			// transform from tangent to local space
			UsingTransforms |= UsedCoord_Local;
			CodeStr = FString(TEXT("mul(Input.TangentBasisInverse,%s)"));
			break;
		default:
			appErrorf( TEXT("Invalid CoordType. See EMaterialVectorCoordTransform") );
		}

		// we are only transforming vectors (not points) so only return a float3
		Result = AddCodeChunk(
				MCT_Float3,
				0,
				*CodeStr,
				*CoerceParameter(A,MCT_Float3)
				);
	}

	return Result;    
}

FString FHLSLMaterialCompiler::GenerateMaterialCode( FMaterial* Material, const TCHAR* UserShaderTemplate, TArray<FMaterialError>& OutErrors )
{
#if !EXCEPTIONS_DISABLED
	try
#endif
	{
		if (!GIsEditor && Material->ShouldCacheCode())
		{
			const FShaderCachedCode* CachedCode = CachedMaterialCode.Find(Material->GetPersistentId());
			if (CachedCode != NULL)
			{
				/** special compiler that adds the textures and parameters used by a material and does nothing else */
				struct FShaderTextureMaterialCompiler : public FMaterialCompiler
				{
					FHLSLMaterialCompiler* Compiler;
					
					// Constructor
					FShaderTextureMaterialCompiler(FHLSLMaterialCompiler* InCompiler)
					: Compiler(InCompiler)
					{}

					// add any textures or parameters encountered to the base compiler's list
					virtual INT Texture(FTextureBase* Texture, INT& TextureIndex) 
					{ 
						Compiler->AddTexture(Texture, TextureIndex);
						return 0; 
					}
					virtual INT TextureParameter(FTextureBase* DefaultTexture, FName ParameterName, INT& TextureIndex)
					{
						Compiler->AddTextureParameter(DefaultTexture, ParameterName, TextureIndex);
						return 0;
					}
					virtual	INT SceneTextureSample(BYTE /*TexType*/,INT /*CoordinateIdx*/) { return 0; }
					virtual	INT SceneTextureDepth( UBOOL /*bNormalize*/, INT /*CoordinateIdx*/) { return 0; }
					virtual	INT PixelDepth(UBOOL /*bNormalize*/) { return 0; }
					virtual	INT DestColor() { return 0; }
					virtual	INT DestDepth(UBOOL /*bNormalize*/) { return 0; }

					// Not implemented (don't need to).
					virtual INT TextureUnpackMin(INT TextureIndex)			{ return 0;	}
					virtual INT TextureUnpackMaxMinusMin(INT TextureIndex)	{ return 0; }
					virtual INT TextureOffsetParameter()					{ return 0; }
					virtual INT TextureScaleParameter()						{ return 0; }
					virtual INT FlipBookOffset(INT TextureIndex)			{ return 0; }
					virtual INT FlipBookScale(INT TextureIndex)				{ return 0; }
#if !EXCEPTIONS_DISABLED
					virtual INT Error(const TCHAR* Text) { throw Text; return 0; }
#else
					virtual INT Error(const TCHAR* Text) { return 0; }
#endif
					virtual void EnterGuard(FMaterialCompilerGuard* Guard) {}
					virtual void ExitGuard() {}
					virtual INT GetSelectionColorIndex() { return 0; }
					virtual EMaterialCodeType GetType(INT Code) { return MCT_Unknown; }
					virtual INT ForceCast(INT Code,EMaterialCodeType DestType) { return 0; }
					virtual INT VectorParameter(FName ParameterName) { return 0; }
					virtual INT ScalarParameter(FName ParameterName) { return 0; }
					virtual INT Constant(FLOAT X) { return 0; }
					virtual INT Constant2(FLOAT X,FLOAT Y) { return 0; }
					virtual INT Constant3(FLOAT X,FLOAT Y,FLOAT Z) { return 0; }
					virtual INT Constant4(FLOAT X,FLOAT Y,FLOAT Z,FLOAT W) { return 0; }
					virtual INT SceneTime() { return 0; }
					virtual INT Sine(INT X) { return 0; }
					virtual INT Cosine(INT X) { return 0; }
					virtual INT Floor(INT X) { return 0; }
					virtual INT Ceil(INT X) { return 0; }
					virtual INT Frac(INT X) { return 0; }
					virtual INT Abs(INT X) { return 0; }
					virtual INT ReflectionVector() { return 0; }
					virtual INT CameraVector() { return 0; }
					virtual INT LightVector() { return 0; }
					virtual INT ScreenPosition( UBOOL bScreenAlign ) { return 0; }
					virtual INT If(INT A,INT B,INT AGreaterThanB,INT AEqualsB,INT ALessThanB) { return 0; }
					virtual INT TextureCoordinate(UINT CoordinateIndex) { return 0; }
					virtual INT TextureSample(INT Texture,INT Coordinate) { return 0; }
					virtual INT VertexColor() { return 0; }
					virtual INT MeshEmitterVertexColor() { return 0; }
					virtual INT Add(INT A,INT B) { return 0; }
					virtual INT Sub(INT A,INT B) { return 0; }
					virtual INT Mul(INT A,INT B) { return 0; }
					virtual INT Div(INT A,INT B) { return 0; }
					virtual INT Dot(INT A,INT B) { return 0; }
					virtual INT Cross(INT A,INT B) { return 0; }
					virtual INT Power(INT Base,INT Exponent) { return 0; }
					virtual INT SquareRoot(INT X) { return 0; }
					virtual INT Lerp(INT X,INT Y,INT A) { return 0; }
					virtual INT Min(INT A,INT B) { return 0; }
					virtual INT Max(INT A,INT B) { return 0; }
					virtual INT Clamp(INT X,INT A,INT B) { return 0; }
					virtual INT ComponentMask(INT Vector,UBOOL R,UBOOL G,UBOOL B,UBOOL A) { return 0; }
					virtual INT AppendVector(INT A,INT B) { return 0; }
					virtual INT TransformVector(BYTE /*CoordType*/,INT /*A*/) { return 0; }
				} Compiler( this );

				// gather the textures and texture parameters used by the material
				// we can't cache these because the FTextureBase aren't guaranteed to stay around
				//@warning: the order of the calls here MUST match the order in the un-cached path below
				Material->CompileProperty(MP_Normal, &Compiler);
				Material->CompileProperty(MP_EmissiveColor, &Compiler);
				Material->CompileProperty(MP_DiffuseColor, &Compiler);
				Material->CompileProperty(MP_SpecularColor, &Compiler);
				Material->CompileProperty(MP_SpecularPower, &Compiler);
				Material->CompileProperty(MP_Opacity, &Compiler);
				Material->CompileProperty(MP_OpacityMask, &Compiler);
				Material->CompileProperty(MP_Distortion, &Compiler);
				Material->CompileProperty(MP_TwoSidedLightingMask, &Compiler);
				Material->CompileProperty(MP_SHM, &Compiler);
				Material->CompileProperty(MP_CustomLighting, &Compiler);
				Material->CompileProperty(MP_AmbientMask, &Compiler);

				// we found cached code for this material - use that instead
				NumUserTexCoords			= CachedCode->NumUserTexCoords;
				UsingTextureOffset			= CachedCode->UsingTextureOffset;
				UsingTextureScale			= CachedCode->UsingTextureScale;
				UsingMeshEmitterVertexColor = CachedCode->UsingMeshEmitterVertexColor;
				UsingTransforms				= CachedCode->UsingTransforms;
				UserInputs					= CachedCode->UserInputs;
				UserConstants				= CachedCode->UserConstants;
				UserVectorInputs			= CachedCode->UserVectorInputs;
				UserScalarInputs			= CachedCode->UserScalarInputs;
				return CachedCode->Code;
			}
		}

		INT		NormalChunk					= ForceCast(Material->CompileProperty(MP_Normal					,this),MCT_Float3);
		INT		EmissiveColorChunk			= ForceCast(Material->CompileProperty(MP_EmissiveColor			,this),MCT_Float3);
		INT		DiffuseColorChunk			= ForceCast(Material->CompileProperty(MP_DiffuseColor			,this),MCT_Float3);
		INT		SpecularColorChunk			= ForceCast(Material->CompileProperty(MP_SpecularColor			,this),MCT_Float3);
		INT		SpecularPowerChunk			= ForceCast(Material->CompileProperty(MP_SpecularPower			,this),MCT_Float1);
		INT		OpacityChunk				= ForceCast(Material->CompileProperty(MP_Opacity				,this),MCT_Float1);
		INT		MaskChunk					= ForceCast(Material->CompileProperty(MP_OpacityMask			,this),MCT_Float1);
		INT		DistortionChunk				= ForceCast(Material->CompileProperty(MP_Distortion				,this),MCT_Float2);
		INT		TwoSidedLightingMaskChunk	= ForceCast(Material->CompileProperty(MP_TwoSidedLightingMask	,this),MCT_Float3);
		INT		SHMChunk					= ForceCast(Material->CompileProperty(MP_SHM					,this),MCT_Float4);
		INT		CustomLightingChunk			= ForceCast(Material->CompileProperty(MP_CustomLighting			,this),MCT_Float3);
		INT		AmbientMaskChunk			= ForceCast(Material->CompileProperty(MP_AmbientMask			,this),MCT_Float1);

		FString Code;
		if( bGenerateCode )
		{
			FString	UserTexturesString	= TEXT("");
			for(INT TextureIndex=0; TextureIndex<Textures.Num(); TextureIndex++)
			{
				FTextureBase*	Texture			= Textures(TextureIndex);
				TCHAR*			SamplerType		= NULL;
				if( Texture )
				{
					FResourceType*	ResourceType = Texture->Type();
					if(ResourceType->IsA(FTexture2D::StaticType))
					{
						SamplerType = TEXT("2D");
					}
					else if(ResourceType->IsA(FTextureCube::StaticType))
					{
						SamplerType = TEXT("CUBE");
					}
					else if(ResourceType->IsA(FTexture3D::StaticType))
					{
						SamplerType = TEXT("3D");
					}
					else if( ResourceType->IsA(FRenderTargetCube::StaticType) )
					{
						SamplerType = TEXT("CUBE");
					}
					else if( ResourceType->IsA(FRenderTarget::StaticType) )
					{
						SamplerType = TEXT("2D");
					}
				}
				else
				{
					SamplerType = TEXT("");
				}

				check(SamplerType);
				UserTexturesString += FString::Printf(TEXT("sampler%s UserTextures%i;\r\n"),SamplerType,TextureIndex);
			}

			Code = FString::Printf(
									UserShaderTemplate,
									Textures.Num(),
									NumUserTexCoords,
									UserVectorInputs.Num(),
									UserScalarInputs.Num(),
									*UserTexturesString,
									GetFixedParameterCode(NormalChunk),
									GetFixedParameterCode(EmissiveColorChunk),
									GetFixedParameterCode(DiffuseColorChunk),
									GetFixedParameterCode(SpecularColorChunk),
									GetFixedParameterCode(SpecularPowerChunk),
									GetFixedParameterCode(OpacityChunk),
									GetFixedParameterCode(MaskChunk),
									*FString::Printf(TEXT("%.5f"),Material->OpacityMaskClipValue),
									GetFixedParameterCode(DistortionChunk),
									GetFixedParameterCode(TwoSidedLightingMaskChunk),
									GetFixedParameterCode(SHMChunk),
									GetFixedParameterCode(CustomLightingChunk),
									GetFixedParameterCode(AmbientMaskChunk)
									);

			if (!GIsEditor && Material->ShouldCacheCode())
			{
				// mark user inputs as persistent so they aren't deleted when the shader is (we will delete them in ClearCodeCache())
				for (INT i = 0; i < UserInputs.Num(); i++)
				{
					UserInputs(i)->bPersistent = true;
				}

				CachedMaterialCode.Set( Material->GetPersistentId(), FShaderCachedCode( Code, NumUserTexCoords, UsingTextureOffset, UsingTextureScale, UsingMeshEmitterVertexColor, 
																						UsingTransforms, UserInputs,	UserConstants, UserVectorInputs, UserScalarInputs )
				);
			}
		}
		else
		{
			static INT SkipCount;
			SkipCount++;
		}

		return Code;
	}
#if !EXCEPTIONS_DISABLED
	catch(const TCHAR* ErrorText)
	{
		SET_WARN_COLOR(COLOR_RED);
		warnf(TEXT("Failed to generate material code for %s: %s"),*Material->DescribeResource(),ErrorText);
		CLEAR_WARN_COLOR();

		// If any errors occured while generating the code, log them and use the next fallback material.
		FMaterialError	Error;
		Error.Description = ErrorText;
		new(OutErrors) FMaterialError(Error);
		return TEXT("");
	}
#endif
}

/** clears the code cache
 * @warning: this function assumes that any FMaterialUserInputs used by the cached code are unreferenced elsewhere and should therefore only be called after flushing the renderer
 */
void FHLSLMaterialCompiler::ClearCodeCache()
{
	// delete any user inputs that were referenced by the cached code
	for (TMap<FGuid, FShaderCachedCode>::TIterator It(CachedMaterialCode); It; ++It)
	{
		const FShaderCachedCode& CachedCode = It.Value();
		for (INT i = 0; i < CachedCode.UserInputs.Num(); i++)
		{
			if (CachedCode.UserInputs(i) != NULL)
			{
				delete CachedCode.UserInputs(i);
			}
		}
	}

	// clear the cache
	CachedMaterialCode.Empty();
}
