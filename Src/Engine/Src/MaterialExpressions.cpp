/*=============================================================================
	MaterialExpressions.cpp - Material expressions implementation.
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineMaterialClasses.h"

IMPLEMENT_CLASS(UMaterialExpression);
IMPLEMENT_CLASS(UMaterialExpressionTextureSample);
IMPLEMENT_CLASS(UMaterialExpressionMeshEmitterVertexColor);
IMPLEMENT_CLASS(UMaterialExpressionMultiply);
IMPLEMENT_CLASS(UMaterialExpressionDivide);
IMPLEMENT_CLASS(UMaterialExpressionSubtract);
IMPLEMENT_CLASS(UMaterialExpressionLinearInterpolate);
IMPLEMENT_CLASS(UMaterialExpressionAdd);
IMPLEMENT_CLASS(UMaterialExpressionTextureCoordinate);
IMPLEMENT_CLASS(UMaterialExpressionComponentMask);
IMPLEMENT_CLASS(UMaterialExpressionDotProduct);
IMPLEMENT_CLASS(UMaterialExpressionCrossProduct);
IMPLEMENT_CLASS(UMaterialExpressionClamp);
IMPLEMENT_CLASS(UMaterialExpressionConstant);
IMPLEMENT_CLASS(UMaterialExpressionConstant2Vector);
IMPLEMENT_CLASS(UMaterialExpressionConstant3Vector);
IMPLEMENT_CLASS(UMaterialExpressionConstant4Vector);
IMPLEMENT_CLASS(UMaterialExpressionTime);
IMPLEMENT_CLASS(UMaterialExpressionCameraVector);
IMPLEMENT_CLASS(UMaterialExpressionReflectionVector);
IMPLEMENT_CLASS(UMaterialExpressionPanner);
IMPLEMENT_CLASS(UMaterialExpressionRotator);
IMPLEMENT_CLASS(UMaterialExpressionUserTransform);
IMPLEMENT_CLASS(UMaterialExpressionFallback);
IMPLEMENT_CLASS(UMaterialExpressionSine);
IMPLEMENT_CLASS(UMaterialExpressionCosine);
IMPLEMENT_CLASS(UMaterialExpressionBumpOffset);
IMPLEMENT_CLASS(UMaterialExpressionAppendVector);
IMPLEMENT_CLASS(UMaterialExpressionFloor);
IMPLEMENT_CLASS(UMaterialExpressionCeil);
IMPLEMENT_CLASS(UMaterialExpressionFrac);
IMPLEMENT_CLASS(UMaterialExpressionAbs);
IMPLEMENT_CLASS(UMaterialExpressionDepthBiasBlend);
IMPLEMENT_CLASS(UMaterialExpressionDepthBiasedAlpha);
IMPLEMENT_CLASS(UMaterialExpressionDepthBiasedBlend);
IMPLEMENT_CLASS(UMaterialExpressionDesaturation);
IMPLEMENT_CLASS(UMaterialExpressionVectorParameter);
IMPLEMENT_CLASS(UMaterialExpressionScalarParameter);
IMPLEMENT_CLASS(UMaterialExpressionNormalize);
IMPLEMENT_CLASS(UMaterialExpressionVertexColor);
IMPLEMENT_CLASS(UMaterialExpressionParticleSubUV);
IMPLEMENT_CLASS(UMaterialExpressionMeshSubUV);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameter);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameter2D);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameterCube);
IMPLEMENT_CLASS(UMaterialExpressionTextureSampleParameterMovie);
IMPLEMENT_CLASS(UMaterialExpressionFlipBookSample);
IMPLEMENT_CLASS(UMaterialExpressionLensFlareIntensity);
IMPLEMENT_CLASS(UMaterialExpressionLensFlareRadialDistance);
IMPLEMENT_CLASS(UMaterialExpressionLensFlareRayDistance);
IMPLEMENT_CLASS(UMaterialExpressionLensFlareSourceDistance);
IMPLEMENT_CLASS(UMaterialExpressionLightVector);
IMPLEMENT_CLASS(UMaterialExpressionScreenPosition);
IMPLEMENT_CLASS(UMaterialExpressionPixelDepth);
IMPLEMENT_CLASS(UMaterialExpressionDestColor);
IMPLEMENT_CLASS(UMaterialExpressionDestDepth);
IMPLEMENT_CLASS(UMaterialExpressionPower);
IMPLEMENT_CLASS(UMaterialExpressionSquareRoot);
IMPLEMENT_CLASS(UMaterialExpressionIf);
IMPLEMENT_CLASS(UMaterialExpressionOneMinus);
IMPLEMENT_CLASS(UMaterialExpressionSceneTexture);
IMPLEMENT_CLASS(UMaterialExpressionEnvCube);
IMPLEMENT_CLASS(UMaterialExpressionSceneDepth);
IMPLEMENT_CLASS(UMaterialExpressionTransform);
IMPLEMENT_CLASS(UMaterialExpressionComment);
IMPLEMENT_CLASS(UMaterialExpressionCompound);
IMPLEMENT_CLASS(UMaterialExpressionFresnel);

#define REMOVE_REFERENCE_TO( ExpressionInput, ToBeRemovedExpression )		\
if( ExpressionInput.Expression == ToBeRemovedExpression )					\
{																			\
	ExpressionInput.Expression = NULL;										\
}

//
//	UMaterialExpression::PostEditChange
//

void UMaterialExpression::PostEditChange(UProperty* PropertyThatChanged)
{
	Super::PostEditChange(PropertyThatChanged);

	check(GetOuterUMaterial());
	GetOuterUMaterial()->PreEditChange(NULL);
	GetOuterUMaterial()->PostEditChange(NULL);
}

//
//	UMaterialExpression::GetOutputs
//

void UMaterialExpression::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(0);
}

//
//	UMaterialExpression::GetWidth
//

INT UMaterialExpression::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

//
//	UMaterialExpression::GetHeight
//

INT UMaterialExpression::GetHeight() const
{
	TArray<FExpressionOutput>	Outputs;
	GetOutputs(Outputs);
	return Max(ME_CAPTION_HEIGHT + (Outputs.Num() * ME_STD_TAB_HEIGHT),ME_CAPTION_HEIGHT+ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2));
}

//
//	UMaterialExpression::UsesLeftGutter
//

UBOOL UMaterialExpression::UsesLeftGutter() const
{
	return 0;
}

//
//	UMaterialExpression::UsesRightGutter
//

UBOOL UMaterialExpression::UsesRightGutter() const
{
	return 0;
}

//
//	UMaterialExpression::GetCaption
//

FString UMaterialExpression::GetCaption() const
{
	return TEXT("Expression");
}

INT UMaterialExpression::CompilerError(FMaterialCompiler* Compiler, const TCHAR* pcMessage)
{
	return Compiler->Errorf(TEXT("%s> %s"), Desc.Len() > 0 ? *Desc : *GetCaption(), pcMessage);
}

//
//	UMaterialExpressionTextureSample::Compile
//
INT UMaterialExpressionTextureSample::Compile(FMaterialCompiler* Compiler)
{
#if EXCEPTIONS_DISABLED
	// if we can't throw the error below, attempt to thwart the error by using the default texture
	// @todo: handle missing cubemaps and 3d textures?
	if (!Texture)
	{
		debugf(TEXT("Using default texture instead of real texture!"));
		Texture = GWorld->GetWorldInfo()->DefaultTexture;
	}
#endif

	if (Texture)
	{
		INT TextureCodeIndex = Compiler->Texture(Texture);

		INT ArgA = Compiler->TextureSample(
						TextureCodeIndex,
						Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0)
						);
		INT ArgB = Compiler->Constant4(
						Texture->UnpackMax[0] - Texture->UnpackMin[0],
						Texture->UnpackMax[1] - Texture->UnpackMin[1],
						Texture->UnpackMax[2] - Texture->UnpackMin[2],
						Texture->UnpackMax[3] - Texture->UnpackMin[3]
						);
		INT ArgC =  Compiler->Constant4(
						Texture->UnpackMin[0],
						Texture->UnpackMin[1],
						Texture->UnpackMin[2],
						Texture->UnpackMin[3]
						);

		return Compiler->Add(Compiler->Mul(ArgA, ArgB), ArgC);
	}
	else
	{
		if (Desc.Len() > 0)
		{
			return Compiler->Errorf(TEXT("%s> Missing input texture"), *Desc);
		}
		else
		{
			return Compiler->Errorf(TEXT("TextureSample> Missing input texture"));
		}
	}
}

//
//	UMaterialExpressionTextureSample::GetOutputs
//

void UMaterialExpressionTextureSample::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

INT UMaterialExpressionTextureSample::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionTextureSample::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinates, Expression );
}

//
//	UMaterialExpressionTextureSample::GetCaption
//

FString UMaterialExpressionTextureSample::GetCaption() const
{
	return TEXT("Texture Sample");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionAdd::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//  UMaterialExpressionTextureSampleParameter
//
INT UMaterialExpressionTextureSampleParameter::Compile(FMaterialCompiler* Compiler)
{
	if (Texture == NULL)
	{
		return CompilerError(Compiler, GetRequirements());
	}

    if (Texture)
    {
        if (!TextureIsValid(Texture))
        {
            return CompilerError(Compiler, GetRequirements());
        }
    }

	if (!ParameterName.IsValid() || (ParameterName.GetIndex() == NAME_None))
	{
		return UMaterialExpressionTextureSample::Compile(Compiler);
	}

	INT TextureCodeIndex = Compiler->TextureParameter(ParameterName, Texture);

	INT ArgA = Compiler->TextureSample(
					TextureCodeIndex,
					Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0)
					);
#if GEMINI_TODO
	// Should we handle texture parameter UnpackMin/UnpackMax, or just use the default texture's unpack settings?
#else
	INT ArgB = Compiler->Constant4(
					Texture->UnpackMax[0] - Texture->UnpackMin[0],
					Texture->UnpackMax[1] - Texture->UnpackMin[1],
					Texture->UnpackMax[2] - Texture->UnpackMin[2],
					Texture->UnpackMax[3] - Texture->UnpackMin[3]
					);
	INT ArgC =  Compiler->Constant4(
					Texture->UnpackMin[0],
					Texture->UnpackMin[1],
					Texture->UnpackMin[2],
					Texture->UnpackMin[3]
					);
#endif

	return Compiler->Add(Compiler->Mul(ArgA,ArgB),ArgC);
}

FString UMaterialExpressionTextureSampleParameter::GetCaption() const
{
	return FString::Printf( TEXT("Parm '%s'"), *ParameterName.ToString() ); 
}

UBOOL UMaterialExpressionTextureSampleParameter::TextureIsValid( UTexture* /*InTexture*/ )
{
    return FALSE;
}

const TCHAR* UMaterialExpressionTextureSampleParameter::GetRequirements()
{
    return TEXT("Invalid texture type");
}

/**
 *	Sets the default texture if none is set
 */
void UMaterialExpressionTextureSampleParameter::SetDefaultTexture()
{
	// Does nothing in the base case...
}

//
//  UMaterialExpressionTextureSampleParameter2D
//
FString UMaterialExpressionTextureSampleParameter2D::GetCaption() const
{
	return FString::Printf( TEXT("Parm2D '%s'"), *ParameterName.ToString() ); 
}

UBOOL UMaterialExpressionTextureSampleParameter2D::TextureIsValid( UTexture* InTexture )
{
	UBOOL Result=FALSE;
	if (InTexture)		
    {
        if( InTexture->GetClass() == UTexture2D::StaticClass() ) 
		{
			Result = TRUE;
		}
		if( InTexture->IsA(UTextureRenderTarget2D::StaticClass()) )	
		{
			Result = TRUE;
		}
	}
    return Result;
}

const TCHAR* UMaterialExpressionTextureSampleParameter2D::GetRequirements()
{
    return TEXT("Requires Texture2D");
}

/**
 *	Sets the default texture if none is set
 */
void UMaterialExpressionTextureSampleParameter2D::SetDefaultTexture()
{
	Texture = LoadObject<UTexture2D>(NULL, TEXT("EngineResources.DefaultTexture"), NULL, LOAD_None, NULL);
}

//
//  UMaterialExpressionTextureSampleParameterCube
//
FString UMaterialExpressionTextureSampleParameterCube::GetCaption() const
{
	return FString::Printf( TEXT("ParmCube'%s'"), *ParameterName.ToString() ); 
}

UBOOL UMaterialExpressionTextureSampleParameterCube::TextureIsValid( UTexture* InTexture )
{
	UBOOL Result=false;
    if (InTexture)
    {
		if( InTexture->GetClass() == UTextureCube::StaticClass() ) {
			Result = true;
		}
		if( InTexture->IsA(UTextureRenderTargetCube::StaticClass()) ) {
			Result = true;
		}
    }
    return Result;
}

const TCHAR* UMaterialExpressionTextureSampleParameterCube::GetRequirements()
{
    return TEXT("Requires TextureCube");
}

/**
 *	Sets the default texture if none is set
 */
void UMaterialExpressionTextureSampleParameterCube::SetDefaultTexture()
{
	Texture = LoadObject<UTextureCube>(NULL, TEXT("EngineResources.DefaultTextureCube"), NULL, LOAD_None, NULL);
}

/**
* Textual description for this material expression
*
* @return	Caption text
*/	
FString UMaterialExpressionTextureSampleParameterMovie::GetCaption() const
{
	return FString::Printf( TEXT("ParmMovie '%s'"), *ParameterName.ToString() ); 
}

/**
* Return true if the texture is a movie texture
*
* @return	true/false
*/	
UBOOL UMaterialExpressionTextureSampleParameterMovie::TextureIsValid( UTexture* InTexture )
{
	UBOOL Result=false;
    if (InTexture)
    {
		Result = (InTexture->GetClass() == UTextureMovie::StaticClass());
    }
    return Result;
}

/**
* Called when TextureIsValid==false
*
* @return	Descriptive error text
*/	
const TCHAR* UMaterialExpressionTextureSampleParameterMovie::GetRequirements()
{
    return TEXT("Requires TextureMovie");
}

//
//	UMaterialExpressionFlipBookSample
//
INT UMaterialExpressionFlipBookSample::Compile(FMaterialCompiler* Compiler)
{
	if (Texture)
	{
		if (!Texture->IsA(UTextureFlipBook::StaticClass()))
		{
			return Compiler->Errorf(TEXT("FlipBookSample> Texture is not a FlipBook"));
		}

#if 1//GEMINI_TODO
		//                                 | Mul |
		//	FlipBookScale-->CompMask(XY)-->|A    |----\      | Add |   |Sample|
		//	TextureCoord(0)--------------->|B    |     \---->|A    |-->|Coord |
		//	FlipBookOffset->CompMask(XY)-------------------->|B    |   |      |
		// Out	 = sub-UV sample of the input texture
		//
		UTextureFlipBook* FlipBook = CastChecked<UTextureFlipBook>(Texture);

		// Compile the texture itself
		INT TextureCodeIndex	= Compiler->Texture(Texture);
		// The scale is constant per texture, and we don't allow for parameterized flipbooks...
		FVector Scale;
		FlipBook->GetFlipBookScale(Scale);
		INT FBScale = Compiler->Constant2(Scale.X, Scale.Y);
		// The input coordinates are like any other texture-expression
		INT TextureCoord = Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0);
		// Scale the UV
		INT ScaledUV = Compiler->Mul(FBScale, TextureCoord);
		// The offset is dynamic... we need an uniform expression parameter
		INT FBOffset = Compiler->FlipBookOffset(FlipBook);
		INT FBOffset_Masked = Compiler->ComponentMask(FBOffset,1,1,0,0);
		// Apply the offset
		INT FinalUV = Compiler->Add(ScaledUV, FBOffset_Masked);

		INT ArgA = Compiler->TextureSample(
						TextureCodeIndex, 
						FinalUV
						);
		INT ArgB = Compiler->Constant4(
						Texture->UnpackMax[0] - Texture->UnpackMin[0],
						Texture->UnpackMax[1] - Texture->UnpackMin[1],
						Texture->UnpackMax[2] - Texture->UnpackMin[2],
						Texture->UnpackMax[3] - Texture->UnpackMin[3]
						);
		INT ArgC =  Compiler->Constant4(
						Texture->UnpackMin[0],
						Texture->UnpackMin[1],
						Texture->UnpackMin[2],
						Texture->UnpackMin[3]
						);

		return Compiler->Add(Compiler->Mul(ArgA, ArgB), ArgC);
/***
		INT TextureIndex;
		INT TextureCodeIndex	= Compiler->Texture(Texture, TextureIndex);
		return Compiler->Add(
				Compiler->Mul(
					Compiler->TextureSample(
						TextureCodeIndex,
						Compiler->Add(
							Compiler->Mul(
								Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0),
								Compiler->ComponentMask(
									Compiler->FlipBookScale(TextureIndex),
									1, 
									1, 
									0, 
									0
									)
								),
							Compiler->ComponentMask(
								Compiler->FlipBookOffset(TextureIndex),
								1, 
								1, 
								0, 
								0
								)
							)
						),
					Compiler->Constant4(
						Texture->UnpackMax[0] - Texture->UnpackMin[0],
						Texture->UnpackMax[1] - Texture->UnpackMin[1],
						Texture->UnpackMax[2] - Texture->UnpackMin[2],
						Texture->UnpackMax[3] - Texture->UnpackMin[3]
						)
					),
				Compiler->Constant4(
					Texture->UnpackMin[0],
					Texture->UnpackMin[1],
					Texture->UnpackMin[2],
					Texture->UnpackMin[3]
					)
				);
***/
#else
		return INDEX_NONE;
#endif
	}
	else
	{
		if (Desc.Len() > 0)
		{
			return Compiler->Errorf(TEXT("%s> Missing input texture"), *Desc);
		}
		else
		{
			return Compiler->Errorf(TEXT("TextureSample> Missing input texture"));
		}
	}
}

FString UMaterialExpressionFlipBookSample::GetCaption() const
{
    return TEXT("FlipBook");
}

//
//	UMaterialExpressionAdd::Compile
//

INT UMaterialExpressionAdd::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing Add input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing Add input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Add(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionAdd::GetCaption
//

FString UMaterialExpressionAdd::GetCaption() const
{
	return TEXT("Add");
}


//
//	UMaterialExpressionMeshEmitterVertexColor::Compile
//
INT UMaterialExpressionMeshEmitterVertexColor::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->VectorParameter(FName(TEXT("MeshEmitterVertexColor")),FLinearColor::White);
}

//
//	UMaterialExpressionMeshEmitterVertexColor::GetCaption
//
FString UMaterialExpressionMeshEmitterVertexColor::GetCaption() const
{
	return TEXT("MeshEmit VertColor");
}

//
//	UMaterialExpressionMeshEmitterVertexColor::GetOutputs
//
void UMaterialExpressionMeshEmitterVertexColor::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

//
//	UMaterialExpressionMultiply::Compile
//

INT UMaterialExpressionMultiply::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing Multiply input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing Multiply input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Mul(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionMultiply::GetCaption
//

FString UMaterialExpressionMultiply::GetCaption() const
{
	return TEXT("Multiply");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionMultiply::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionDestColor
///////////////////////////////////////////////////////////////////////////////
/**
 *	Compile the expression
 *
 *	@param	Compiler	The compiler to utilize
 *
 *	@return				The index of the resulting code chunk.
 *						INDEX_NONE if error.
 */
INT UMaterialExpressionDestColor::Compile(FMaterialCompiler* Compiler)
{
	// resulting index to compiled code chunk
	// add the code chunk for the scene color sample        
	INT Result = Compiler->DestColor();
	return Result;
}

/**
 *	Retrieve the outputs this expression supplies.
 *
 *	@param	Outputs		The array to insert the available outputs in.
 */
void UMaterialExpressionDestColor::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// RGB
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	// R
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	// G
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	// B
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	// A
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

/**
 *	Get the caption to display on this material expression
 *
 *	@return			An FString containing the display caption
 */
FString UMaterialExpressionDestColor::GetCaption() const
{
	return TEXT("DestColor");
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionDestDepth
///////////////////////////////////////////////////////////////////////////////
/**
 *	Compile the expression
 *
 *	@param	Compiler	The compiler to utilize
 *
 *	@return				The index of the resulting code chunk.
 *						INDEX_NONE if error.
 */
INT UMaterialExpressionDestDepth::Compile(FMaterialCompiler* Compiler)
{
	// resulting index to compiled code chunk
	// add the code chunk for the scene depth sample        
	INT Result = Compiler->DestDepth(bNormalize);
	return Result;
}

/**
 *	Retrieve the outputs this expression supplies.
 *
 *	@param	Outputs		The array to insert the available outputs in.
 */
void UMaterialExpressionDestDepth::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// Depth
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
 *	Get the caption to display on this material expression
 *
 *	@return			An FString containing the display caption
 */
FString UMaterialExpressionDestDepth::GetCaption() const
{
	return TEXT("DestDepth");
}

//
//	UMaterialExpressionDivide::Compile
//

INT UMaterialExpressionDivide::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing Divide input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing Divide input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Div(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionDivide::GetCaption
//

FString UMaterialExpressionDivide::GetCaption() const
{
	return TEXT("Divide");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDivide::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionSubtract::Compile
//

INT UMaterialExpressionSubtract::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing Subtract input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing Subtract input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Sub(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionSubtract::GetCaption
//

FString UMaterialExpressionSubtract::GetCaption() const
{
	return TEXT("Subtract");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionSubtract::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionLinearInterpolate::Compile
//

INT UMaterialExpressionLinearInterpolate::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing LinearInterpolate input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing LinearInterpolate input B"));
	else if(!Alpha.Expression)
		return Compiler->Errorf(TEXT("Missing LinearInterpolate input Alpha"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		INT Arg3 = Alpha.Compile(Compiler);
		return Compiler->Lerp(
			Arg1,
			Arg2,
			Arg3
			);
	}
}

//
//	UMaterialExpressionLinearInterpolate::GetCaption
//

FString UMaterialExpressionLinearInterpolate::GetCaption() const
{
	return TEXT("Lerp");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionLinearInterpolate::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
	REMOVE_REFERENCE_TO( Alpha, Expression );
}

//
//	UMaterialExpressionConstant::Compile
//

INT UMaterialExpressionConstant::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Constant(R);
}

//
//	UMaterialExpressionConstant::GetCaption
//

FString UMaterialExpressionConstant::GetCaption() const
{
	return FString::Printf( TEXT("%.4g"), R );
}

//
//	UMaterialExpressionConstant2Vector::Compile
//

INT UMaterialExpressionConstant2Vector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Constant2(R,G);
}

//
//	UMaterialExpressionConstant2Vector::GetCaption
//

FString UMaterialExpressionConstant2Vector::GetCaption() const
{
	return FString::Printf( TEXT("%.3g,%.3g"), R, G );
}

//
//	UMaterialExpressionConstant3Vector::Compile
//

INT UMaterialExpressionConstant3Vector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Constant3(R,G,B);
}

//
//	UMaterialExpressionConstant3Vector::GetCaption
//

FString UMaterialExpressionConstant3Vector::GetCaption() const
{
	return FString::Printf( TEXT("%.3g,%.3g,%.3g"), R, G, B );
}

//
//	UMaterialExpressionConstant4Vector::Compile
//

INT UMaterialExpressionConstant4Vector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Constant4(R,G,B,A);
}

//
//	UMaterialExpressionConstant4Vector::GetCaption
//

FString UMaterialExpressionConstant4Vector::GetCaption() const
{
	return FString::Printf( TEXT("%.2g,%.2g,%.2g,%.2g"), R, G, B, A );
}

//
//	UMaterialExpressionClamp::Compile
//

INT UMaterialExpressionClamp::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Clamp input"));
	else
	{
		if(!Min.Expression && !Max.Expression)
			return Input.Compile(Compiler);
		else if(!Min.Expression)
		{
			INT Arg1 = Input.Compile(Compiler);
			INT Arg2 = Max.Compile(Compiler);
			return Compiler->Min(
				Arg1,
				Arg2
				);
		}
		else if(!Max.Expression)
		{
			INT Arg1 = Input.Compile(Compiler);
			INT Arg2 = Min.Compile(Compiler);
			return Compiler->Max(
				Arg1,
				Arg2
				);
		}
		else
		{
			INT Arg1 = Input.Compile(Compiler);
			INT Arg2 = Min.Compile(Compiler);
			INT Arg3 = Max.Compile(Compiler);
			return Compiler->Clamp(
				Arg1,
				Arg2,
				Arg3
				);
		}
	}
}

//
//	UMaterialExpressionClamp::GetCaption
//

FString UMaterialExpressionClamp::GetCaption() const
{
	return TEXT("Clamp");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionClamp::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
	REMOVE_REFERENCE_TO( Min, Expression );
	REMOVE_REFERENCE_TO( Max, Expression );
}

//
//	UMaterialExpressionTextureCoordinate::Compile
//

INT UMaterialExpressionTextureCoordinate::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->Mul(Compiler->TextureCoordinate(CoordinateIndex),Compiler->Constant(Tiling));
}

//
//	UMaterialExpressionTextureCoordinate::GetCaption
//

FString UMaterialExpressionTextureCoordinate::GetCaption() const
{
	return TEXT("TexCoord");
}

//
//	UMaterialExpressionDotProduct::Compile
//

INT UMaterialExpressionDotProduct::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing DotProduct input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing DotProduct input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Dot(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionDotProduct::GetCaption
//

FString UMaterialExpressionDotProduct::GetCaption() const
{
	return TEXT("Dot");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDotProduct::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionCrossProduct::Compile
//

INT UMaterialExpressionCrossProduct::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing CrossProduct input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing CrossProduct input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->Cross(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionCrossProduct::GetCaption
//

FString UMaterialExpressionCrossProduct::GetCaption() const
{
	return TEXT("Cross");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionCrossProduct::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionComponentMask::Compile
//

INT UMaterialExpressionComponentMask::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing ComponentMask input"));
	else
		return Compiler->ComponentMask(
			Input.Compile(Compiler),
			R,
			G,
			B,
			A
			);
}

//
//	UMaterialExpressionComponentMask::GetCaption
//

FString UMaterialExpressionComponentMask::GetCaption() const
{
	FString Str(TEXT("Mask ("));
	if ( R ) Str += TEXT(" R");
	if ( G ) Str += TEXT(" G");
	if ( B ) Str += TEXT(" B");
	if ( A ) Str += TEXT(" A");
	Str += TEXT(" )");
	return Str;
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionComponentMask::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionTime::Compile
//

INT UMaterialExpressionTime::Compile(FMaterialCompiler* Compiler)
{
	return bIgnorePause ? Compiler->RealTime() : Compiler->GameTime();
}

//
//	UMaterialExpressionTime::GetCaption
//

FString UMaterialExpressionTime::GetCaption() const
{
	return TEXT("Time");
}

//
//	UMaterialExpressionCameraVector::Compile
//

INT UMaterialExpressionCameraVector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->CameraVector();
}

//
//	UMaterialExpressionCameraVector::GetCaption
//

FString UMaterialExpressionCameraVector::GetCaption() const
{
	return TEXT("Camera Vector");
}

//
//	UMaterialExpressionReflectionVector::Compile
//

INT UMaterialExpressionReflectionVector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->ReflectionVector();
}

//
//	UMaterialExpressionReflectionVector::GetCaption
//

FString UMaterialExpressionReflectionVector::GetCaption() const
{
	return TEXT("Reflection Vector");
}

//
//	UMaterialExpressionPanner::Compile
//

INT UMaterialExpressionPanner::Compile(FMaterialCompiler* Compiler)
{
	INT Arg1 = Compiler->PeriodicHint(Compiler->Mul(Time.Expression ? Time.Compile(Compiler) : Compiler->GameTime(),Compiler->Constant(SpeedX)));
	INT Arg2 = Compiler->PeriodicHint(Compiler->Mul(Time.Expression ? Time.Compile(Compiler) : Compiler->GameTime(),Compiler->Constant(SpeedY)));
	INT Arg3 = Coordinate.Expression ? Coordinate.Compile(Compiler) : Compiler->TextureCoordinate(0);
	return Compiler->Add(
			Compiler->AppendVector(
				Arg1,
				Arg2
				),
			Arg3
			);
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionPanner::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinate, Expression );
	REMOVE_REFERENCE_TO( Time, Expression );
}

//
//	UMaterialExpressionPanner::GetCaption
//

FString UMaterialExpressionPanner::GetCaption() const
{
	return TEXT("Panner");
}

//
//	UMaterialExpressionRotator::Compile
//

INT UMaterialExpressionRotator::Compile(FMaterialCompiler* Compiler)
{
	INT	Cosine = Compiler->Cosine(Compiler->Mul(Time.Expression ? Time.Compile(Compiler) : Compiler->GameTime(),Compiler->Constant(Speed))),
		Sine = Compiler->Sine(Compiler->Mul(Time.Expression ? Time.Compile(Compiler) : Compiler->GameTime(),Compiler->Constant(Speed))),
		RowX = Compiler->AppendVector(Cosine,Compiler->Mul(Compiler->Constant(-1.0f),Sine)),
		RowY = Compiler->AppendVector(Sine,Cosine),
		Origin = Compiler->Constant2(CenterX,CenterY),
		BaseCoordinate = Coordinate.Expression ? Coordinate.Compile(Compiler) : Compiler->TextureCoordinate(0);

	INT Arg1 = Compiler->Dot(RowX,Compiler->Sub(Compiler->ComponentMask(BaseCoordinate,1,1,0,0),Origin));
	INT Arg2 = Compiler->Dot(RowY,Compiler->Sub(Compiler->ComponentMask(BaseCoordinate,1,1,0,0),Origin));

	if(Compiler->GetType(BaseCoordinate) == MCT_Float3)
		return Compiler->AppendVector(
				Compiler->Add(
					Compiler->AppendVector(
						Arg1,
						Arg2
						),
					Origin
					),
				Compiler->ComponentMask(BaseCoordinate,0,0,1,0)
				);
	else
	{
		INT Arg1 = Compiler->Dot(RowX,Compiler->Sub(BaseCoordinate,Origin));
		INT Arg2 = Compiler->Dot(RowY,Compiler->Sub(BaseCoordinate,Origin));

		return Compiler->Add(
				Compiler->AppendVector(
					Arg1,
					Arg2
					),
				Origin
				);
	}
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionRotator::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinate, Expression );
	REMOVE_REFERENCE_TO( Time, Expression );
}

//
//	UMaterialExpressionRotator::GetCaption
//

FString UMaterialExpressionRotator::GetCaption() const
{
	return TEXT("Rotator");
}

//
//	UMaterialExpressionUserTransform::Compile
//

INT UMaterialExpressionUserTransform::Compile(FMaterialCompiler* Compiler)
{
	INT BaseCoordinate = Coordinate.Expression ? Coordinate.Compile(Compiler) : Compiler->TextureCoordinate(0);

	INT ScaledCoordinate = Scaling.Expression ? Compiler->Mul( Scaling.Compile(Compiler), BaseCoordinate ) : BaseCoordinate;
	INT TranslatedCoordinate = Translation.Expression ? Compiler->Add( Translation.Compile(Compiler), ScaledCoordinate ) : ScaledCoordinate;

	if (RowX.Expression || RowY.Expression)
	{
		INT	RowX0 = RowX.Expression ? RowX.Compile(Compiler) : Compiler->Constant3(0,0,0),
			RowY0 = RowY.Expression ? RowY.Compile(Compiler) : Compiler->Constant3(0,0,0);

		INT Arg1 = Compiler->Dot(RowX0,Compiler->AppendVector(Compiler->ComponentMask(TranslatedCoordinate,1,1,0,0),Compiler->Constant(1)));
		INT Arg2 = Compiler->Dot(RowY0,Compiler->AppendVector(Compiler->ComponentMask(TranslatedCoordinate,1,1,0,0),Compiler->Constant(1)));

		if(Compiler->GetType(BaseCoordinate) == MCT_Float3)
			return Compiler->AppendVector(Compiler->AppendVector(Arg1,Arg2),Compiler->ComponentMask(BaseCoordinate,0,0,1,0));
		else	
			return Compiler->AppendVector(Arg1,Arg2);	
	}	
	else
	{
		return TranslatedCoordinate;
	}
}

/**
* Removes references to the passed in expression.
*
* @param	Expression		Expression to remove reference to
*/
void UMaterialExpressionUserTransform::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinate, Expression );	
	REMOVE_REFERENCE_TO( RowX, Expression );	
	REMOVE_REFERENCE_TO( RowY, Expression );	
	REMOVE_REFERENCE_TO( Translation, Expression );	
	REMOVE_REFERENCE_TO( Scaling, Expression );	
}

//
//	UMaterialExpressionUserTransform::GetCaption
//

FString UMaterialExpressionUserTransform::GetCaption() const
{
	return TEXT("UserTransform");
}

//
//	UMaterialExpressionFallback::Compile
//

INT UMaterialExpressionFallback::Compile(FMaterialCompiler* Compiler)
{
	check( SP_NumPlatforms == 5 );

#define HANDLE_FALLBACK(Platform)	case SP_##Platform : if (Platform.Expression) return Platform.Compile(Compiler); else break;

	switch (GShaderCompilePlatform)
	{
	HANDLE_FALLBACK(PCD3D);
	HANDLE_FALLBACK(PS3);
	HANDLE_FALLBACK(XBOXD3D);
	case SP_PCD3D_SM2_POOR :
	{
		if( PCD3D_SM2_POOR.Expression )
		{
			return PCD3D_SM2_POOR.Compile(Compiler);
		}
		else
		{
			if( PCD3D_SM2.Expression )
			{
				return PCD3D_SM2.Compile(Compiler);
			}
			else
			{
				break;
			}
		}
	}
	HANDLE_FALLBACK(PCD3D_SM2);	
	}
	
	if (DefaultCase.Expression) return DefaultCase.Compile(Compiler);

	return INDEX_NONE;
}

/**
* Removes references to the passed in expression.
*
* @param	Expression		Expression to remove reference to
*/
void UMaterialExpressionFallback::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );

	check( SP_NumPlatforms == 5 );

	REMOVE_REFERENCE_TO( DefaultCase, Expression );	
	REMOVE_REFERENCE_TO( PCD3D, Expression );	
	REMOVE_REFERENCE_TO( PS3, Expression );	
	REMOVE_REFERENCE_TO( XBOXD3D, Expression );	
	REMOVE_REFERENCE_TO( PCD3D_SM2, Expression );
	REMOVE_REFERENCE_TO( PCD3D_SM2_POOR, Expression );
}

//
//	UMaterialExpressionUserTransform::GetCaption
//

FString UMaterialExpressionFallback::GetCaption() const
{
	return TEXT("Fallback");
}

//
//	UMaterialExpressionSine::Compile
//

INT UMaterialExpressionSine::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Sine input"));
	return Compiler->Sine(Period > 0.0f ? Compiler->Mul(Input.Compile(Compiler),Compiler->Constant(2.0f * (FLOAT)PI / Period)) : Input.Compile(Compiler));
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionSine::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionSine::GetCaption
//

FString UMaterialExpressionSine::GetCaption() const
{
	return TEXT("Sine");
}

//
//	UMaterialExpressionCosine::Compile
//

INT UMaterialExpressionCosine::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
	{
		return Compiler->Errorf(TEXT("Missing Cosine input"));
	}
	return Compiler->Cosine(Compiler->Mul(Input.Compile(Compiler),Period > 0.0f ? Compiler->Constant(2.0f * (FLOAT)PI / Period) : 0));
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionCosine::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionCosine::GetCaption
//

FString UMaterialExpressionCosine::GetCaption() const
{
	return TEXT("Cosine");
}

//
//	UMaterialExpressionBumpOffset::Compile
//

INT UMaterialExpressionBumpOffset::Compile(FMaterialCompiler* Compiler)
{
	if(!Height.Expression)
		return Compiler->Errorf(TEXT("Missing Height input"));

	return Compiler->Add(
			Compiler->Mul(
				Compiler->ComponentMask(Compiler->CameraVector(),1,1,0,0),
				Compiler->Add(
					Compiler->Mul(
						Compiler->Constant(HeightRatio),
						Compiler->ForceCast(Height.Compile(Compiler),MCT_Float1)
						),
					Compiler->Constant(-ReferencePlane * HeightRatio)
					)
				),
			Coordinate.Expression ? Coordinate.Compile(Compiler) : Compiler->TextureCoordinate(0)
			);
}

//
//	UMaterialExpressionBumpOffset::GetCaption
//

FString UMaterialExpressionBumpOffset::GetCaption() const
{
	return TEXT("BumpOffset");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionBumpOffset::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Height, Expression );
	REMOVE_REFERENCE_TO( Coordinate, Expression );
}

//
//	UMaterialExpressionAppendVector::Compile
//

INT UMaterialExpressionAppendVector::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
		return Compiler->Errorf(TEXT("Missing AppendVector input A"));
	else if(!B.Expression)
		return Compiler->Errorf(TEXT("Missing AppendVector input B"));
	else
	{
		INT Arg1 = A.Compile(Compiler);
		INT Arg2 = B.Compile(Compiler);
		return Compiler->AppendVector(
			Arg1,
			Arg2
			);
	}
}

//
//	UMaterialExpressionAppendVector::GetCaption
//

FString UMaterialExpressionAppendVector::GetCaption() const
{
	return TEXT("Append");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionAppendVector::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
}

//
//	UMaterialExpressionFloor::Compile
//

INT UMaterialExpressionFloor::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Floor input"));
	return Compiler->Floor(Input.Compile(Compiler));
}

//
//	UMaterialExpressionFloor::GetCaption
//

FString UMaterialExpressionFloor::GetCaption() const
{
	return TEXT("Floor");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionFloor::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionCeil::Compile
//

INT UMaterialExpressionCeil::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Ceil input"));
	return Compiler->Ceil(Input.Compile(Compiler));
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionCeil::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionCeil::GetCaption
//

FString UMaterialExpressionCeil::GetCaption() const
{
	return TEXT("Ceil");
}

//
//	UMaterialExpressionFrac::Compile
//

INT UMaterialExpressionFrac::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Frac input"));
	return Compiler->Frac(Input.Compile(Compiler));
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionFrac::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

//
//	UMaterialExpressionFrac::GetCaption
//

FString UMaterialExpressionFrac::GetCaption() const
{
	return TEXT("Frac");
}

///////////////////////////////////////////////////////////
// UMaterialExpressionDepthBiasBlend
///////////////////////////////////////////////////////////
#define _DEPTHBIAS_USE_ONE_MINUS_BIAS_
/**
 *	Compile the material expression
 *
 *	@param	Compiler	Pointer to the material compiler to use
 *
 *	@return	INT			The compiled code index
 */	
INT UMaterialExpressionDepthBiasBlend::Compile(FMaterialCompiler* Compiler)
{
	//   +----------------+
	//   | DepthBiasBlend |
	//   +----------------+
	// --| RGB       Bias |--       BiasCalc = (1 - Bias) or -Bias... still determining.
	// --| R    BiasScale |--
	// --| G              |
	// --| B              |
	// --| Alpha          |
	//   +----------------+                     +-----+     +----------+
	//                      +------------+      | Sub |  /--| DstDepth |
	//        +-----+    /--| PixelDepth |      |    A|-/   +----------+-----------+
	//        | If  |   /   +------------+  /---|    B|-----| BiasCalc * BiasScale |
	// RGB ---|    A|--/  /----------------/    +-----+     +----------------------+
	//        |    B|----/  +-----+     +------------+                    +----------+
	//        |    >|--\    | If  |  /--| DstDepth   |     +--------+   /-| DstColor |
	//        |    =|-------|    A|-/   +------------+     | Lerp   |  /  +----------+
	//        |    <|-\     |    B|-----| PixelDepth |     |       A|-/ /-| SrcColor |   +----------+      +------------+
	//        +-----+ |     |    >|-\   +------------+     |       B|--/  +----------+   | Subtract |   /--| DstDepth   |
	//                |     |    =|------------------------|   Alpha|-\                  |         A|--/   +------------+
	//                |     |    <|--\                     +--------+  \              /--|         B|------| PixelDepth |
	//                |     +-----+   \                                 \   +-----+  /   +----------+      +------------+
	//                |  +----------+  \   +----------+                  \--| /  A|-/   +----------------------+
	//                \--| SrcColor |   \--| DstColor |                     |    B|-----| BiasCalc * BiasScale |
	//                   +----------+      +----------+                     +-----+     +----------------------+
	//
    //@todo. Add support for this
    //BITFIELD bNormalize:1;

#if EXCEPTIONS_DISABLED
	// if we can't throw the error below, attempt to thwart the error by using the default texture
	// @todo: handle missing cubemaps and 3d textures?
	if (!Texture)
	{
		debugf(TEXT("Using default texture instead of real texture!"));
		Texture = GWorld->GetWorldInfo()->DefaultTexture;
	}
#endif

	if (Texture)
	{
		INT SrcTextureCodeIndex = Compiler->Texture(Texture);

		INT ArgA	=	Compiler->TextureSample(
							SrcTextureCodeIndex,
							Coordinates.Expression ? 
								Coordinates.Compile(Compiler) : 
								Compiler->TextureCoordinate(0)
							);
		INT ArgB	=	Compiler->Constant4(
							Texture->UnpackMax[0] - Texture->UnpackMin[0],
							Texture->UnpackMax[1] - Texture->UnpackMin[1],
							Texture->UnpackMax[2] - Texture->UnpackMin[2],
							Texture->UnpackMax[3] - Texture->UnpackMin[3]
							);
		INT ArgC	=	Compiler->Constant4(
							Texture->UnpackMin[0],
							Texture->UnpackMin[1],
							Texture->UnpackMin[2],
							Texture->UnpackMin[3]
							);

		INT	Arg_SrcSample	=	Compiler->Add(Compiler->Mul(ArgA, ArgB), ArgC);
		INT Arg_DstSample	=	Compiler->DestColor();
		INT	Arg_SrcDepth	=	Compiler->PixelDepth(bNormalize);
		INT	Arg_DstDepth	=	Compiler->DestDepth(bNormalize);
		INT	Arg_Constant_0	=	Compiler->Constant(0.0f);
		INT	Arg_Constant_1	=	Compiler->Constant(1.0f);

#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
		INT	Arg_Bias		=	(Bias.Expression) ? 
									Compiler->Sub(Arg_Constant_1, Bias.Compile(Compiler)) : 
									Arg_Constant_1;
#else	//#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
		INT	Arg_Bias		=	(Bias.Expression) ? 
									Compiler->Sub(Arg_Constant_0, Bias.Compile(Compiler)) : 
									Arg_Constant_0;
#endif	//#if defined(_DEPTHBIAS_USE_ONE_MINUS_BIAS_)
		
		INT	Arg_BiasScaleConstant		=	Compiler->Constant(BiasScale);
		INT	Arg_ScaledBias				=	Compiler->Mul(Arg_Bias, Arg_BiasScaleConstant);
		INT	Arg_Sub_DstDepth_Bias		=	Compiler->Sub(Arg_DstDepth, Arg_ScaledBias);
		INT	Arg_Sub_DstDepth_SrcDepth	=	Compiler->Sub(Arg_DstDepth, Arg_SrcDepth);
		INT	Arg_Div_DZ_Sub_SZ_Bias		=	Compiler->Div(Arg_Sub_DstDepth_SrcDepth, Arg_ScaledBias);
		INT Arg_ClampedLerpBias			=	Compiler->Clamp(Arg_Div_DZ_Sub_SZ_Bias, Arg_Constant_0, Arg_Constant_1);

		INT	Arg_Lerp_Dst_Src_Color		=	
								Compiler->Lerp(
									Arg_DstSample, 
									Arg_SrcSample, 
									Arg_ClampedLerpBias
									);

		INT	Arg_If_DZ_SZ	=	Compiler->If(
									Arg_DstDepth, 
									Arg_SrcDepth, 
									Arg_Lerp_Dst_Src_Color, 
									Arg_Lerp_Dst_Src_Color, 
									Arg_DstSample
									);

		INT	Arg_If_SZ_DZ_Add_Bias	= Compiler->If(
										Arg_SrcDepth,
										Arg_Sub_DstDepth_Bias,
										Arg_If_DZ_SZ,
										Arg_If_DZ_SZ,
										Arg_SrcSample
										);

		return Arg_If_SZ_DZ_Add_Bias;
	}
	else
	{
		if (Desc.Len() > 0)
		{
			return Compiler->Errorf(TEXT("%s> Missing input texture"), *Desc);
		}
		else
		{
			return Compiler->Errorf(TEXT("TextureSample> Missing input texture"));
		}
	}
}

/**
 *	Get the outputs associated with the expression
 *
 *	@param	Outputs		The array that contains the output expression
 */	
void UMaterialExpressionDepthBiasBlend::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
}

/**
 */	
INT UMaterialExpressionDepthBiasBlend::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
 */	
FString UMaterialExpressionDepthBiasBlend::GetCaption() const
{
	return TEXT("DepthBiasBlend");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDepthBiasBlend::RemoveReferenceTo(UMaterialExpression* Expression)
{
	Super::RemoveReferenceTo(Expression);
	REMOVE_REFERENCE_TO(Bias, Expression);
	REMOVE_REFERENCE_TO(Coordinates, Expression);
}

///////////////////////////////////////////////////////////
// UMaterialExpressionDepthBiasedAlpha
///////////////////////////////////////////////////////////
    //## BEGIN PROPS MaterialExpressionDepthBiasedAlpha
//    BITFIELD bNormalize:1;
//    FLOAT BiasScale;
//    FExpressionInput Alpha;
//    FExpressionInput Bias;
    //## END PROPS MaterialExpressionDepthBiasedAlpha

/**
 *	Compile the material expression
 *
 *	@param	Compiler	Pointer to the material compiler to use
 *
 *	@return	INT			The compiled code index
 */	
INT UMaterialExpressionDepthBiasedAlpha::Compile(FMaterialCompiler* Compiler)
{
	INT ResultIdx = INDEX_NONE;

	// source alpha input
	INT Arg_SourceA = Alpha.Expression ? Alpha.Compile(Compiler) : Compiler->Constant(1.0f);
	// bias input
	INT Arg_Bias = Bias.Expression ? Bias.Compile(Compiler) : Compiler->Constant(0.5f);
	// bias scale
	INT Arg_BiasScale = Compiler->Constant(BiasScale);

	// make sure source alpha is 1 component
	EMaterialValueType Type_SourceA = Compiler->GetType(Arg_SourceA);
	if (!(Type_SourceA & MCT_Float1))
	{
		Arg_SourceA = Compiler->ComponentMask(Arg_SourceA, 1, 0, 0, 0);
	}

	// add depth bias alpha code chunk
	ResultIdx = Compiler->DepthBiasedAlpha(Arg_SourceA, Arg_Bias, Arg_BiasScale);

	return ResultIdx;
}

/**
 */	
INT UMaterialExpressionDepthBiasedAlpha::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
 */	
FString UMaterialExpressionDepthBiasedAlpha::GetCaption() const
{
	return TEXT("DepthBiasedAlpha");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDepthBiasedAlpha::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo(Expression);
	REMOVE_REFERENCE_TO(Bias, Expression);
	REMOVE_REFERENCE_TO(Alpha, Expression);
}

///////////////////////////////////////////////////////////
// UMaterialExpressionDepthBiasedBlend
///////////////////////////////////////////////////////////

//## BEGIN PROPS MaterialExpressionDepthBiasedBlend
//BITFIELD bNormalize:1;
//FLOAT BiasScale;
//FExpressionInput SourceRGB;
//FExpressionInput SourceA;
//FExpressionInput Bias;
//## END PROPS MaterialExpressionDepthBiasedBlend

/**
 *	Compile the material expression
 *
 *	@param	Compiler	Pointer to the material compiler to use
 *
 *	@return	INT			The compiled code index
 */	
INT UMaterialExpressionDepthBiasedBlend::Compile(FMaterialCompiler* Compiler)
{
	INT ResultIdx = INDEX_NONE;

	// source RGB color input 
	INT	Arg_SourceRGB	= RGB.Expression	? RGB.Compile( Compiler ) : Compiler->Constant3( 0.f, 0.f, 0.f );
	// source Alpha input 
	INT	Arg_SourceA		= Alpha.Expression	? Alpha.Compile( Compiler )	: Compiler->Constant( 1.0f );
	// bias input
	INT Arg_Bias = Bias.Compile( Compiler );
	// bias scale property
	INT Arg_BiasScale = Compiler->Constant( BiasScale );

	// make sure source Alpha is 1 component
	EMaterialValueType Type_SourceA = Compiler->GetType( Arg_SourceA );
	if( !(Type_SourceA & MCT_Float1) )
	{
		return Compiler->Errorf( TEXT("Alpha input must be float1") );
	}
	// make sure source RGB is 3 components
	EMaterialValueType Type_SourceRGB = Compiler->GetType( Arg_SourceRGB );
	if( Type_SourceRGB == MCT_Float4 )
	{
		Arg_SourceRGB = Compiler->ComponentMask( Arg_SourceRGB, 1, 1, 1, 0 );
	}

	// add depth bias blend code chunk
	INT	Arg_RGB_Component	= Compiler->DepthBiasedBlend( Arg_SourceRGB, Arg_Bias, Arg_BiasScale );
	// append the alpha to RGB for a 4 component result
	ResultIdx =  Compiler->AppendVector(Arg_RGB_Component, Arg_SourceA);

	return ResultIdx;
}


/**
 *	Get the outputs associated with the expression
 *
 *	@param	Outputs		The array that contains the output expression
 */	
void UMaterialExpressionDepthBiasedBlend::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

/**
 */	
INT UMaterialExpressionDepthBiasedBlend::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
 */	
FString UMaterialExpressionDepthBiasedBlend::GetCaption() const
{
	return TEXT("DepthBiasedBlend");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDepthBiasedBlend::RemoveReferenceTo(UMaterialExpression* Expression)
{
	Super::RemoveReferenceTo(Expression);
	REMOVE_REFERENCE_TO(Bias, Expression);
	REMOVE_REFERENCE_TO(Alpha, Expression);
	REMOVE_REFERENCE_TO(RGB, Expression);
}

//
//	UMaterialExpressionDesaturation::Compile
//

INT UMaterialExpressionDesaturation::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
		return Compiler->Errorf(TEXT("Missing Desaturation input"));

	INT	Color = Input.Compile(Compiler),
		Grey = Compiler->Dot(Color,Compiler->Constant3(LuminanceFactors.R,LuminanceFactors.G,LuminanceFactors.B));

	if(Percent.Expression)
		return Compiler->Lerp(Color,Grey,Percent.Compile(Compiler));
	else
		return Grey;
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionDesaturation::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
	REMOVE_REFERENCE_TO( Percent, Expression );
}

//
//	UMaterialExpressionVectorParameter::Compile
//

INT UMaterialExpressionVectorParameter::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->VectorParameter(ParameterName,DefaultValue);
}

//
//	UMaterialExpressionVectorParameter::GetOutputs
//

void UMaterialExpressionVectorParameter::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

FString UMaterialExpressionVectorParameter::GetCaption() const
{
	return FString::Printf(
		 TEXT("Parm '%s' (%.3g,%.3g,%.3g,%.3g)"),
		 *ParameterName.ToString(),
		 DefaultValue.R,
		 DefaultValue.G,
		 DefaultValue.B,
		 DefaultValue.A );
}
 
//
//	UMaterialExpressionScalarParameter::Compile
//

INT UMaterialExpressionScalarParameter::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->ScalarParameter(ParameterName,DefaultValue);
}

FString UMaterialExpressionScalarParameter::GetCaption() const
{
	 return FString::Printf(
		 TEXT("Parm '%s' (%.4g)"),
		 *ParameterName.ToString(),
		 DefaultValue ); 
}

INT UMaterialExpressionNormalize::Compile(FMaterialCompiler* Compiler)
{
	if(!VectorInput.Expression)
		return Compiler->Errorf(TEXT("Missing Normalize input"));

	INT	V = VectorInput.Compile(Compiler);

	return Compiler->Div(V,Compiler->SquareRoot(Compiler->Dot(V,V)));
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionNormalize::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( VectorInput, Expression );
}

INT UMaterialExpressionVertexColor::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->VertexColor();
}

void UMaterialExpressionVertexColor::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

FString UMaterialExpressionVertexColor::GetCaption() const
{
	return TEXT("Vertex Color");
}

//
//	MaterialExpressionParticleSubUV
//
INT UMaterialExpressionParticleSubUV::Compile(FMaterialCompiler* Compiler)
{
	if (Texture)
	{
		// Out	 = linear interpolate... using 2 sub-images of the texture
		// A	 = RGB sample texture with TexCoord0
		// B	 = RGB sample texture with TexCoord1
		// Alpha = x component of TexCoord2
		//
		INT TextureCodeIndexA = Compiler->Texture(Texture);
		INT TextureCodeIndexB = Compiler->Texture(Texture);

		return 
			Compiler->Lerp(
				// Lerp_A, 
				Compiler->Add(
					Compiler->Mul(
						Compiler->TextureSample(
							TextureCodeIndexA,  
							Compiler->TextureCoordinate(0)
						),
						Compiler->Constant4(
							Texture->UnpackMax[0] - Texture->UnpackMin[0],
							Texture->UnpackMax[1] - Texture->UnpackMin[1],
							Texture->UnpackMax[2] - Texture->UnpackMin[2],
							Texture->UnpackMax[3] - Texture->UnpackMin[3]
						)
					),
					Compiler->Constant4(
						Texture->UnpackMin[0],
						Texture->UnpackMin[1],
						Texture->UnpackMin[2],
						Texture->UnpackMin[3]
					)
				),
				// Lerp_B, 
				Compiler->Add(
					Compiler->Mul(
						Compiler->TextureSample(
							TextureCodeIndexB, 
							Compiler->TextureCoordinate(1)
						),
						Compiler->Constant4(
							Texture->UnpackMax[0] - Texture->UnpackMin[0],
							Texture->UnpackMax[1] - Texture->UnpackMin[1],
							Texture->UnpackMax[2] - Texture->UnpackMin[2],
							Texture->UnpackMax[3] - Texture->UnpackMin[3]
						)
					),
					Compiler->Constant4(
						Texture->UnpackMin[0],
						Texture->UnpackMin[1],
						Texture->UnpackMin[2],
						Texture->UnpackMin[3]
					)
				),
				// Lerp 'alpha' comes from the 3rd texture set...
				Compiler->ComponentMask(
					Compiler->TextureCoordinate(2), 
					1, 0, 0, 0
				)
			);

	}
	else
	{
		return Compiler->Errorf(TEXT("Missing ParticleSubUV input texture"));
	}
}

void UMaterialExpressionParticleSubUV::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

INT UMaterialExpressionParticleSubUV::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

FString UMaterialExpressionParticleSubUV::GetCaption() const
{
	return TEXT("Particle SubUV");
}

//
//	UMaterialExpressionMeshSubUV
//
INT UMaterialExpressionMeshSubUV::Compile(FMaterialCompiler* Compiler)
{
	if (Texture)
	{
		//                         | Mul |
		//	VectorParam(Scale)---->|A    |----\           | Add |   |Sample|
		//	TextureCoord(0)------->|B    |     \--------->|A    |-->|Coord |
		//	VectorParam(SubUVMeshOffset)-->CompMask(XY)-->|B    |   |      |

		// Out	 = sub-UV sample of the input texture
		//
		INT TextureCodeIndex	= Compiler->Texture(Texture);
		return Compiler->Add(
				Compiler->Mul(
					Compiler->TextureSample(
						TextureCodeIndex,
						Compiler->Add(
							Compiler->Mul(
								Coordinates.Expression ? Coordinates.Compile(Compiler) : Compiler->TextureCoordinate(0),
								Compiler->ComponentMask(
									Compiler->VectorParameter(FName(TEXT("TextureScaleParameter")),FLinearColor::White),
									1, 
									1, 
									0, 
									0
									)
								),
							Compiler->ComponentMask(
								Compiler->VectorParameter(FName(TEXT("TextureOffsetParameter")),FLinearColor::Black),
								1, 
								1, 
								0, 
								0
								)
							)
						),
					Compiler->Constant4(
						Texture->UnpackMax[0] - Texture->UnpackMin[0],
						Texture->UnpackMax[1] - Texture->UnpackMin[1],
						Texture->UnpackMax[2] - Texture->UnpackMin[2],
						Texture->UnpackMax[3] - Texture->UnpackMin[3]
						)
					),
				Compiler->Constant4(
					Texture->UnpackMin[0],
					Texture->UnpackMin[1],
					Texture->UnpackMin[2],
					Texture->UnpackMin[3]
					)
				);
	}
	else
	{
		return Compiler->Errorf(TEXT("%s missing texture"), *GetCaption());
	}
}

void UMaterialExpressionMeshSubUV::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

INT UMaterialExpressionMeshSubUV::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

FString UMaterialExpressionMeshSubUV::GetCaption() const
{
	return TEXT("Mesh SubUV");
}

/**
*	LensFlareIntensity
*/
/**
*	Compile this expression with the given compiler.
*	
*	@return	INT			The code index for this expression.
*/
INT UMaterialExpressionLensFlareIntensity::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->LensFlareIntesity();
}

/**
*	Get the outputs supported by this expression.
*
*	@param	Outputs		The TArray of outputs to fill in.
*/
void UMaterialExpressionLensFlareIntensity::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
*	Get the width required by this expression (in the material editor).
*
*	@return	INT			The width in pixels.
*/
INT UMaterialExpressionLensFlareIntensity::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
*	Returns the text to display on the material expression (in the material editor).
*
*	@return	FString		The text to display.
*/
FString UMaterialExpressionLensFlareIntensity::GetCaption() const
{
	return TEXT("LensFlare Intensity");
}

/**
*	LensFlareRadialDistance
*/
/**
*	Compile this expression with the given compiler.
*	
*	@return	INT			The code index for this expression.
*/
INT UMaterialExpressionLensFlareRadialDistance::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->LensFlareRadialDistance();
}

/**
*	Get the outputs supported by this expression.
*
*	@param	Outputs		The TArray of outputs to fill in.
*/
void UMaterialExpressionLensFlareRadialDistance::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
*	Get the width required by this expression (in the material editor).
*
*	@return	INT			The width in pixels.
*/
INT UMaterialExpressionLensFlareRadialDistance::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
*	Returns the text to display on the material expression (in the material editor).
*
*	@return	FString		The text to display.
*/
FString UMaterialExpressionLensFlareRadialDistance::GetCaption() const
{
	return TEXT("LensFlare RadialDist");
}

/**
*	LensFlareRayDistance
*/
/**
*	Compile this expression with the given compiler.
*	
*	@return	INT			The code index for this expression.
*/
INT UMaterialExpressionLensFlareRayDistance::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->LensFlareRayDistance();
}

/**
*	Get the outputs supported by this expression.
*
*	@param	Outputs		The TArray of outputs to fill in.
*/
void UMaterialExpressionLensFlareRayDistance::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
*	Get the width required by this expression (in the material editor).
*
*	@return	INT			The width in pixels.
*/
INT UMaterialExpressionLensFlareRayDistance::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
*	Returns the text to display on the material expression (in the material editor).
*
*	@return	FString		The text to display.
*/
FString UMaterialExpressionLensFlareRayDistance::GetCaption() const
{
	return TEXT("LensFlare RayDist");
}	

/**
*	LensFlareSourceDistance
*/
/**
*	Compile this expression with the given compiler.
*	
*	@return	INT			The code index for this expression.
*/
INT UMaterialExpressionLensFlareSourceDistance::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->LensFlareSourceDistance();
}	

/**
*	Get the outputs supported by this expression.
*
*	@param	Outputs		The TArray of outputs to fill in.
*/
void UMaterialExpressionLensFlareSourceDistance::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
*	Get the width required by this expression (in the material editor).
*
*	@return	INT			The width in pixels.
*/
INT UMaterialExpressionLensFlareSourceDistance::GetWidth() const
{
	return ME_STD_THUMBNAIL_SZ+(ME_STD_BORDER*2);
}

/**
*	Returns the text to display on the material expression (in the material editor).
*
*	@return	FString		The text to display.
*/
FString UMaterialExpressionLensFlareSourceDistance::GetCaption() const
{
	return TEXT("LensFlare SourceDist");
}

INT UMaterialExpressionLightVector::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->LightVector();
}

FString UMaterialExpressionLightVector::GetCaption() const
{
	return TEXT("Light Vector");
}

INT UMaterialExpressionScreenPosition::Compile(FMaterialCompiler* Compiler)
{
	return Compiler->ScreenPosition( ScreenAlign );
}

FString UMaterialExpressionScreenPosition::GetCaption() const
{
	return TEXT("ScreenPos");
}

INT UMaterialExpressionSquareRoot::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
	{
		return Compiler->Errorf(TEXT("Missing square root input"));
	}
	return Compiler->SquareRoot(Input.Compile(Compiler));
}

FString UMaterialExpressionSquareRoot::GetCaption() const
{
	return TEXT("Sqrt");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionSquareRoot::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionPixelDepth
///////////////////////////////////////////////////////////////////////////////
/**
 *	Compile the expression
 *
 *	@param	Compiler	The compiler to utilize
 *
 *	@return				The index of the resulting code chunk.
 *						INDEX_NONE if error.
 */
INT UMaterialExpressionPixelDepth::Compile(FMaterialCompiler* Compiler)
{
	// resulting index to compiled code chunk
	// add the code chunk for the scene depth sample        
	INT Result = Compiler->PixelDepth(bNormalize);
	return Result;
}

/**
 *	Retrieve the outputs this expression supplies.
 *
 *	@param	Outputs		The array to insert the available outputs in.
 */
void UMaterialExpressionPixelDepth::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// Depth
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
 *	Get the caption to display on this material expression
 *
 *	@return			An FString containing the display caption
 */
FString UMaterialExpressionPixelDepth::GetCaption() const
{
	return TEXT("PixelDepth");
}

//
INT UMaterialExpressionPower::Compile(FMaterialCompiler* Compiler)
{
	if(!Base.Expression)
	{
		return Compiler->Errorf(TEXT("Missing Power Base input"));
	}
	if(!Exponent.Expression)
	{
		return Compiler->Errorf(TEXT("Missing Power Exponent input"));
	}

	INT Arg1 = Base.Compile(Compiler);
	INT Arg2 = Exponent.Compile(Compiler);
	return Compiler->Power(
		Arg1,
		Arg2
		);
}

FString UMaterialExpressionPower::GetCaption() const
{
	return TEXT("Power");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionPower::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Base, Expression );
	REMOVE_REFERENCE_TO( Exponent, Expression );
}

INT UMaterialExpressionIf::Compile(FMaterialCompiler* Compiler)
{
	if(!A.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If A input"));
	}
	if(!B.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If B input"));
	}
	if(!AGreaterThanB.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If AGreaterThanB input"));
	}
	if(!AEqualsB.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If AEqualsB input"));
	}
	if(!ALessThanB.Expression)
	{
		return Compiler->Errorf(TEXT("Missing If ALessThanB input"));
	}

	INT CompiledA = A.Compile(Compiler);
	INT CompiledB = B.Compile(Compiler);

	if(Compiler->GetType(CompiledA) != MCT_Float)
	{
		return Compiler->Errorf(TEXT("If input A must be of type float."));
	}

	if(Compiler->GetType(CompiledB) != MCT_Float)
	{
		return Compiler->Errorf(TEXT("If input B must be of type float."));
	}

	INT Arg3 = AGreaterThanB.Compile(Compiler);
	INT Arg4 = AEqualsB.Compile(Compiler);
	INT Arg5 = ALessThanB.Compile(Compiler);

	return Compiler->If(CompiledA,CompiledB,Arg3, Arg4, Arg5);
}

FString UMaterialExpressionIf::GetCaption() const
{
	return TEXT("If");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionIf::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( A, Expression );
	REMOVE_REFERENCE_TO( B, Expression );
	REMOVE_REFERENCE_TO( AGreaterThanB, Expression );
	REMOVE_REFERENCE_TO( AEqualsB, Expression );
	REMOVE_REFERENCE_TO( ALessThanB, Expression );
}

INT UMaterialExpressionOneMinus::Compile(FMaterialCompiler* Compiler)
{
	if(!Input.Expression)
	{
		return Compiler->Errorf(TEXT("Missing 1-x input"));
	}
	return Compiler->Sub(Compiler->Constant(1.0f),Input.Compile(Compiler));
}

FString UMaterialExpressionOneMinus::GetCaption() const
{
	return TEXT("1-x");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionOneMinus::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

/**
 * Creates the new shader code chunk needed for the Abs expression
 *
 * @param	Compiler - Material compiler that knows how to handle this expression
 * @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
 */	
INT UMaterialExpressionAbs::Compile( FMaterialCompiler* Compiler )
{
	INT Result=INDEX_NONE;

	if( !Input.Expression )
	{
		// an input expression must exist
		Result = Compiler->Errorf( TEXT("Missing Abs input") );
	}
	else
	{
		// evaluate the input expression first and use that as
		// the parameter for the Abs expression
		Result = Compiler->Abs( Input.Compile(Compiler) );
	}

	return Result;
}

/**
 * Textual description for this material expression
 *
 * @return	Caption text
 */	
FString UMaterialExpressionAbs::GetCaption() const
{
	return TEXT("Abs");
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionAbs::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionSceneTexture
///////////////////////////////////////////////////////////////////////////////

/**
* Create the shader code chunk for sampling the scene's lighting texture
*
* @param	Compiler - Material compiler that knows how to handle this expression
* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
*/	
INT UMaterialExpressionSceneTexture::Compile( FMaterialCompiler* Compiler )
{
	// resulting index to compiled code chunk
	INT Result=INDEX_NONE;
	// resulting index to compiled code for the tex coordinates if available
	INT CoordIdx = INDEX_NONE;
	// if there are valid texture coordinate inputs then compile them
	if( Coordinates.Expression )
	{
		CoordIdx = Coordinates.Compile(Compiler);
	}
    // add the code chunk for the scene texture sample        
	Result = Compiler->SceneTextureSample( SceneTextureType, CoordIdx );
	return Result;
}

/**
* Fill in the array of valid outputs for this material expression
*
* @param	Outputs - array of this expression's valid outputs
*/	
void UMaterialExpressionSceneTexture::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// RGB
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	// R
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	// G
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	// B
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	// A
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

/**
* Removes references to the passed in expression.
*
* @param	Expression - expression to remove reference to
*/
void UMaterialExpressionSceneTexture::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinates, Expression );
}

/**
* Text description of this expression
*/
FString UMaterialExpressionSceneTexture::GetCaption() const
{
	return TEXT("Scene Texture Sample");
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionEnvCube
///////////////////////////////////////////////////////////////////////////////

/**
* Create the shader code chunk for sampling the scene's lighting texture
*
* @param	Compiler - Material compiler that knows how to handle this expression
* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
*/	
INT UMaterialExpressionEnvCube::Compile( FMaterialCompiler* Compiler )
{
	// resulting index to compiled code chunk
	INT Result=INDEX_NONE;
	// resulting index to compiled code for the tex coordinates if available
	INT CoordIdx = INDEX_NONE;
	// if there are valid texture coordinate inputs then compile them
	if( Coordinates.Expression )
	{
		CoordIdx = Coordinates.Compile(Compiler);
	}
	// add the code chunk for the scene texture sample        
	Result = Compiler->EnvCube( CoordIdx );
	return Result;
}

/**
* Fill in the array of valid outputs for this material expression
*
* @param	Outputs - array of this expression's valid outputs
*/	
void UMaterialExpressionEnvCube::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	// RGB
	new(Outputs) FExpressionOutput(1,1,1,1,0);
	// R
	new(Outputs) FExpressionOutput(1,1,0,0,0);
	// G
	new(Outputs) FExpressionOutput(1,0,1,0,0);
	// B
	new(Outputs) FExpressionOutput(1,0,0,1,0);
	// A
	new(Outputs) FExpressionOutput(1,0,0,0,1);
}

/**
* Removes references to the passed in expression.
*
* @param	Expression - expression to remove reference to
*/
void UMaterialExpressionEnvCube::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinates, Expression );
}

/**
* Text description of this expression
*/
FString UMaterialExpressionEnvCube::GetCaption() const
{
	return TEXT("EnvCube");
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionSceneDepth
///////////////////////////////////////////////////////////////////////////////

/**
* Create the shader code chunk for sampling the scene's depth
*
* @param	Compiler - Material compiler that knows how to handle this expression
* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
*/	
INT UMaterialExpressionSceneDepth::Compile( FMaterialCompiler* Compiler )
{
	// resulting index to compiled code chunk
	INT Result=INDEX_NONE;
	// resulting index to compiled code for the tex coordinates if available
	INT CoordIdx = INDEX_NONE;
	// if there are valid texture coordinate inputs then compile them
	if( Coordinates.Expression )
	{
		CoordIdx = Coordinates.Compile(Compiler);
	}
	// add the code chunk for the scene depth sample        
	Result = Compiler->SceneTextureDepth( bNormalize, CoordIdx );
	return Result;
}

/**
* Fill in the array of valid outputs for this material expression
*
* @param	Outputs - array of this expression's valid outputs
*/	
void UMaterialExpressionSceneDepth::GetOutputs(TArray<FExpressionOutput>& Outputs) const
{
	new(Outputs) FExpressionOutput(1,1,0,0,0);
}

/**
* Removes references to the passed in expression.
*
* @param	Expression - expression to remove reference to
*/
void UMaterialExpressionSceneDepth::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Coordinates, Expression );
}

/**
* Text description of this expression
*/
FString UMaterialExpressionSceneDepth::GetCaption() const
{
	return TEXT("Scene Depth");
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionTransform
///////////////////////////////////////////////////////////////////////////////

/**
* Create the shader code chunk for transforming an input vector to a new coordinate space
*
* @param	Compiler - Material compiler that knows how to handle this expression
* @return	Index to the new FMaterialCompiler::CodeChunk entry for this expression
*/	
INT UMaterialExpressionTransform::Compile(FMaterialCompiler* Compiler)
{
	INT Result=INDEX_NONE;

	if( !Input.Expression )
	{
		Result = Compiler->Errorf(TEXT("Missing Transform input vector"));
	}
	else
	{
		INT VecInputIdx = Input.Compile(Compiler);
		Result = Compiler->TransformVector( TransformType, VecInputIdx );
	}

	return Result;
}

/**
* Text description of this expression
*/
FString UMaterialExpressionTransform::GetCaption() const
{
	return TEXT("Coordinate Transform");
}

/**
* Removes references to the passed in expression.
*
* @param	Expression		Expression to remove reference to
*/
void UMaterialExpressionTransform::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Input, Expression );
}

///////////////////////////////////////////////////////////////////////////////
// UMaterialExpressionComment
///////////////////////////////////////////////////////////////////////////////

/**
 * Text description of this expression.
 */
FString UMaterialExpressionComment::GetCaption() const
{
	return TEXT("Comment");
}

///////////////////////////////////////////////////////////////////////////////
// MaterialExpressionCompound
///////////////////////////////////////////////////////////////////////////////

/**
 * Text description of this expression.
 */
FString UMaterialExpressionCompound::GetCaption() const
{
	return Caption.Len() > 0 ? *Caption : TEXT("Compound Expression");
}

/**
 * Recursively gathers all UMaterialExpression objects referenced by this expression.
 * Including self.
 *
 * @param	Expressions		Reference to array of material expressions to add to
 */	
void UMaterialExpressionCompound::GetExpressions( TArray<const UMaterialExpression*>& Expressions ) const
{
	//Super::GetExpressions( Expressions );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionCompound::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	MaterialExpressions.RemoveItem( Expression );
}

/**
 * Removes references to the passed in expression.
 *
 * @param	Expression		Expression to remove reference to
 */
void UMaterialExpressionFresnel::RemoveReferenceTo( UMaterialExpression* Expression )
{
	Super::RemoveReferenceTo( Expression );
	REMOVE_REFERENCE_TO( Normal, Expression );
}

/**
 * Spits out the proper shader code for the approximate Fresnel term
 *
 * @param Compiler the compiler compiling this expression
 */
INT UMaterialExpressionFresnel::Compile(FMaterialCompiler* Compiler)
{
	// pow(1 - max(0,Normal dot Camera),Exponent)
	//
	INT NormalArg = Normal.Expression ? Normal.Compile(Compiler) : Compiler->Constant3(0.f,0.f,1.f);
	INT DotArg = Compiler->Dot(NormalArg,Compiler->CameraVector());
	INT MaxArg = Compiler->Max(Compiler->Constant(0.f),DotArg);
	INT MinusArg = Compiler->Sub(Compiler->Constant(1.f),MaxArg);
	INT PowArg = Compiler->Power(MinusArg,Compiler->Constant(Exponent));
	return PowArg;
}
