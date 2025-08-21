/*=============================================================================
	UnCanvas.cpp: Unreal canvas rendering.
	Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineUserInterfaceClasses.h"
#include "ScenePrivate.h"
#include "TileRendering.h"

IMPLEMENT_CLASS(UCanvas);

FCanvas::FCanvas(FRenderTarget* InRenderTarget,FHitProxyConsumer* InHitProxyConsumer):
	RenderTarget(InRenderTarget),
	HitProxyConsumer(InHitProxyConsumer),
	BatchedElements(NULL), 
	Texture(NULL), 
	BlendMode(BLEND_MAX),
	ElementType(ET_MAX)
{
	check(RenderTarget);
	// Push the viewport transform onto the stack.  Default to using a 2D projection. 
	new(TransformStack) FMatrix( CalcBaseTransform2D(RenderTarget->GetSizeX(),RenderTarget->GetSizeY()) );
	// init alpha to 1
	AlphaModulate=1.0;

	//! 폰트의 해상도에 따른 스케일링 계수(2007/01/11 고광록).
	// 기본값으로 1을 넣어주면 크기에 변동이 없다.
	FontRatioFactorX = 1.0f;
	FontRatioFactorY = 1.0f;
}

/**
* Replace the base (ie. TransformStack(0)) transform for the canvas with the given matrix
*
* @param Transform - The transform to use for the base
*/
void FCanvas::SetBaseTransform(const FMatrix& Transform)
{
	// set the base transform
	if( TransformStack.Num() > 0 )
	{
		TransformStack(0) = Transform;
	}
	else
	{
		new(TransformStack) FMatrix(Transform);
	}
}

/**
* Generate a 2D projection for the canvas. Use this if you only want to transform in 2D on the XY plane
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcBaseTransform2D(UINT ViewSizeX, UINT ViewSizeY)
{
	return 
		FTranslationMatrix(FVector(-GPixelCenterOffset,-GPixelCenterOffset,0)) *
		FMatrix(
			FPlane(	1.0f / (ViewSizeX / 2.0f),	0.0,										0.0f,	0.0f	),
			FPlane(	0.0f,						-1.0f / (ViewSizeY / 2.0f),					0.0f,	0.0f	),
			FPlane(	0.0f,						0.0f,										1.0f,	0.0f	),
			FPlane(	-1.0f,						1.0f,										0.0f,	1.0f	)
			);
}

/**
* Generate a 3D projection for the canvas. Use this if you want to transform in 3D 
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @param NearPlane - Distance to the near clip plane
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcBaseTransform3D(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane)
{
	FMatrix ViewMat(CalcViewMatrix(ViewSizeX,ViewSizeY,fFOV));
	FMatrix ProjMat(CalcProjectionMatrix(ViewSizeX,ViewSizeY,fFOV,NearPlane));
	return ViewMat * ProjMat;
}

/**
* Generate a view matrix for the canvas. Used for CalcBaseTransform3D
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @return Matrix for canvas view orientation
*/
FMatrix FCanvas::CalcViewMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV)
{
	// convert FOV to randians
	FLOAT FOVRad = fFOV * (FLOAT)PI / 360.0f;
	// move camera back enough so that the canvas items being rendered are at the same screen extents as regular canvas 2d rendering	
	FTranslationMatrix CamOffsetMat(-FVector(0,0,-appTan(FOVRad)*ViewSizeX/2));
	// adjust so that canvas items render as if they start at [0,0] upper left corner of screen 
	// and extend to the lower right corner [ViewSizeX,ViewSizeY]. 
	FMatrix OrientCanvasMat(
		FPlane(	1.0f,				0.0f,				0.0f,	0.0f	),
		FPlane(	0.0f,				-1.0f,				0.0f,	0.0f	),
		FPlane(	0.0f,				0.0f,				1.0f,	0.0f	),
		FPlane(	ViewSizeX * -0.5f,	ViewSizeY * 0.5f,	0.0f, 1.0f		)
		);
	return 
		// also apply screen offset to align to pixel centers
		FTranslationMatrix(FVector(-GPixelCenterOffset,-GPixelCenterOffset,0)) * 
		OrientCanvasMat * 
		CamOffsetMat;
}

/**
* Generate a projection matrix for the canvas. Used for CalcBaseTransform3D
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @param NearPlane - Distance to the near clip plane
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcProjectionMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane)
{
	// convert FOV to randians
	FLOAT FOVRad = fFOV * (FLOAT)PI / 360.0f;
	// project based on the FOV and near plane given
	return FPerspectiveMatrix(
		FOVRad,
		ViewSizeX,
		ViewSizeY,
		NearPlane
		);
}

FCanvas::~FCanvas()
{
	Flush();
}

/** Sends a message to the rendering thread to draw the batched elements. */
void FCanvas::Flush()
{
	FCommandContextRHI* GlobalContext = RHIGetGlobalContext();
	if(BatchedElements)
	{
		// this allows us to use FCanvas operations from the rendering thread (ie, render subtitles
		// on top of a movie that is rendered completely in rendering thread)
		if (IsInRenderingThread())
		{
			void UpdateTrueTypeFontMips();
			UpdateTrueTypeFontMips();

			BatchedElements->Draw(
				GlobalContext,
				GetFullTransform(),
				GetRenderTarget()->GetSizeX(),
				GetRenderTarget()->GetSizeY(),
				IsHitTesting(),
				1.0f / GetRenderTarget()->GetDisplayGamma()
				);
			delete BatchedElements;
		}
		else
		{
			// Render the batched elements.
			struct FFlushParameters
			{
				FBatchedElements* BatchedElements;
				FMatrix Transform;
				UINT ViewportSizeX;
				UINT ViewportSizeY;
				BITFIELD bHitTesting : 1;
				FLOAT DisplayGamma;
			};
			FFlushParameters FlushParameters =
			{
				BatchedElements,
					GetFullTransform(),
					GetRenderTarget()->GetSizeX(),
					GetRenderTarget()->GetSizeY(),
					IsHitTesting(),
					GetRenderTarget()->GetDisplayGamma()
			};
			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				FlushCommand,
				FFlushParameters,Parameters,FlushParameters,
			{
				void UpdateTrueTypeFontMips();
				UpdateTrueTypeFontMips();

				FCommandContextRHI* GlobalContext = RHIGetGlobalContext();
				Parameters.BatchedElements->Draw(
					GlobalContext,
					Parameters.Transform,
					Parameters.ViewportSizeX,
					Parameters.ViewportSizeY,
					Parameters.bHitTesting,
					1.0f / Parameters.DisplayGamma
					);
				delete Parameters.BatchedElements;
			});
		}
	}

	// Release our pointer to the batched elements.
	BatchedElements = NULL;
	Texture = NULL;
	BlendMode = BLEND_MAX;
	ElementType = ET_MAX;
}

/**
* Returns a FBatchedElements pointer to be used for adding vertices and primitives for rendering.
* Adds a new render item to the sort element entry based on the current sort key.
*
* @param InElementType - Type of element we are going to draw.
* @param InTexture - New texture that will be set.
* @param InBlendMode - New blendmode that will be set.	* 
* @return Returns a pointer to a FBatchedElements object.
*/
FBatchedElements* FCanvas::GetBatchedElements(EElementType InElementType, const FTexture* InTexture, EBlendMode InBlendMode)
{
	// If we already have a CanvasBatch, check to see if the parameters passed in are 
	if(BatchedElements)
	{
		UBOOL bFlush = FALSE;

		if(ElementType != ET_MAX && ElementType != InElementType)
		{
			bFlush = TRUE;
		}
		else if(Texture && InTexture && Texture != InTexture)
		{
			bFlush = TRUE;
		}
		else if(BlendMode != BLEND_MAX && InBlendMode != BLEND_MAX && BlendMode != InBlendMode)
		{
			bFlush = TRUE;
		}

		if(bFlush)
		{
			Flush();
			BatchedElements = new FBatchedElements;
		}
	}
	else
	{
		BatchedElements = new FBatchedElements;
	}

	// Set the properties for this batch.
	Texture = const_cast<FTexture*>(InTexture);
	BlendMode = InBlendMode;
	ElementType = InElementType;

	return BatchedElements;
}

void FCanvas::PushRelativeTransform(const FMatrix& Transform)
{
	Flush();

	INT PreviousTopIndex = TransformStack.Num() - 1;
#if 0
	static UBOOL DEBUG_NoRotation=1;
	if( DEBUG_NoRotation )
	{
		FMatrix TransformNoRotation(FMatrix::Identity);
		TransformNoRotation.SetOrigin(Transform.GetOrigin());
		new(TransformStack) FMatrix(TransformNoRotation * TransformStack(PreviousTopIndex));
	}
	else
#endif
	{
		new(TransformStack) FMatrix(Transform * TransformStack(PreviousTopIndex));
	}
}

void FCanvas::PushAbsoluteTransform(const FMatrix& Transform) 
{
	Flush();

	new(TransformStack) FMatrix(Transform * TransformStack(0));
}

void FCanvas::PopTransform()
{
	Flush();

	TransformStack.Pop();
}

void FCanvas::SetHitProxy(HHitProxy* HitProxy)
{
	// Change the current hit proxy.
	CurrentHitProxy = HitProxy;

	if(HitProxyConsumer && HitProxy)
	{
		// Notify the hit proxy consumer of the new hit proxy.
		HitProxyConsumer->AddHitProxy(HitProxy);
	}
}

void FCanvas::SetRenderTarget(FRenderTarget* NewRenderTarget)
{
	FCommandContextRHI* GlobalContext = RHIGetGlobalContext();

	// Set the RHI render target.
	RHISetRenderTarget(GlobalContext,NewRenderTarget->RenderTargetSurfaceRHI, FSurfaceRHIRef());

	// Change the current render target.
	RenderTarget = NewRenderTarget;
}

//! 폰트의 해상도에 따른 스케일링 계수를 설정하는 함수(2007/01/11 고광록).
void FCanvas::SetFontRatioScaling(bool bEnabled, float RatioX, float RatioY)
{
	if(!bEnabled)
	{
		FontRatioFactorX = 1.0f;
		FontRatioFactorY = 1.0f;
		return ;
	}
	else
	{
		// 대상이 없다면 아무 소용 없다.
		if(RenderTarget == NULL)
			return ;

		// 폰트의 비율계수 = 현재 해상도 / 폰트의 기본 해상도.
		FontRatioFactorX = RatioX;
		FontRatioFactorY = RatioY;
	}
}

void Clear(FCanvas* Canvas,const FLinearColor& Color)
{
	// desired display gamma space
	const FLOAT DisplayGamma = (GEngine && GEngine->Client) ? GEngine->Client->DisplayGamma : 2.2f;
	// render target gamma space expected
	FLOAT RenderTargetGamma = DisplayGamma;
	if( Canvas->GetRenderTarget() )
	{
		RenderTargetGamma = Canvas->GetRenderTarget()->GetDisplayGamma();
	}
	// assume that the clear color specified is in 2.2 gamma space
	// so convert to the render target's color space 
	FLinearColor GammaCorrectedColor(Color);
	GammaCorrectedColor.R = appPow(Clamp<FLOAT>(GammaCorrectedColor.R,0.0f,1.0f), DisplayGamma / RenderTargetGamma);
	GammaCorrectedColor.G = appPow(Clamp<FLOAT>(GammaCorrectedColor.G,0.0f,1.0f), DisplayGamma / RenderTargetGamma);
	GammaCorrectedColor.B = appPow(Clamp<FLOAT>(GammaCorrectedColor.B,0.0f,1.0f), DisplayGamma / RenderTargetGamma);

	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		ClearCommand,
		FColor,Color,GammaCorrectedColor,
		FRenderTarget*,CanvasRenderTarget,Canvas->GetRenderTarget(),
	{
		FCommandContextRHI* Context = RHIGetGlobalContext();		
		RHIClear(Context,TRUE,Color,FALSE,0.0f,FALSE,0);
	});
}

/**
 *	Draws a line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
void DrawLine(FCanvas* Canvas,const FVector& StartPos,const FVector& EndPos,const FLinearColor& Color)
{
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Line);
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	BatchedElements->AddLine(StartPos,EndPos,Color,HitProxyId);
}

/**
 *	Draws a 2D line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
void DrawLine2D(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos,const FLinearColor& Color)
{
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Line);
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	BatchedElements->AddLine(FVector(StartPos.X,StartPos.Y,0),FVector(EndPos.X,EndPos.Y,0),Color,HitProxyId);
}

void DrawBox2D(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos,const FLinearColor& Color)
{
	DrawLine2D(Canvas,FVector2D(StartPos.X,StartPos.Y),FVector2D(StartPos.X,EndPos.Y),Color);
	DrawLine2D(Canvas,FVector2D(StartPos.X,EndPos.Y),FVector2D(EndPos.X,EndPos.Y),Color);
	DrawLine2D(Canvas,FVector2D(EndPos.X,EndPos.Y),FVector2D(EndPos.X,StartPos.Y),Color);
	DrawLine2D(Canvas,FVector2D(EndPos.X,StartPos.Y),FVector2D(StartPos.X,StartPos.Y),Color);
}

/**
 *  Draws a modified line.
 */
void DrawLine2DStyled(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos, const FLinearColor& Color, EDrawLineStyle LineStyle, INT LineOffset )
{
	// 짝수 : 선의 픽셀수
	// 홀수 : 공백 픽셀수
	// 예외 : 0일경우 패턴을 맨앞부터 다시
	static INT LinePattern[DLS_MAX][6] = 
	{
		{3,1,0,0,0,0},
		{1,2,0,0,0,0},
		{3,2,1,2,0,0},
		{3,2,1,2,1,2},
	};

	static const INT PatternSize = ARRAY_COUNT(LinePattern[0]);
	check( 0 <= LineStyle && LineStyle < DLS_MAX);

	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Line);
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	FVector Direction(EndPos - StartPos,0.f);
	FVector EndPosVect = FVector(EndPos, 0.f);
	Direction.Normalize();

	// 패턴의 갯수를 구하고 (예를 들어 DLS_DASH의 경우 2개, DLS_DASHDOT의 경우 4개)
	INT Mod = 0;
	INT PeriodicPixel = 0;
	for( INT i = 0 ; i < PatternSize && LinePattern[LineStyle][i] > 0 ; i++ )
	{
		PeriodicPixel += LinePattern[LineStyle][i];
		Mod = i + 1;
	}
	
	// 맨 앞에 그려야할 패턴을 정한다. (라인패턴에 오프셋값이 있다면)
	INT FirstOffset = LineOffset % PeriodicPixel;
	INT PatternOffset = 0;
	for( INT i = 0 ; i < Mod ; i++ )
	{
		if( FirstOffset - LinePattern[LineStyle][i] > 0 )
			FirstOffset -= LinePattern[LineStyle][i];
		else
			PatternOffset = i;
	}

	INT Count = 0;
	INT CurrentPattern = 0;
	for( FVector NewStartPos = FVector(StartPos,0.f) ; ((EndPosVect - NewStartPos) | Direction) > 0 ; )
	{
		INT PattenIndex = (Count + PatternOffset) % Mod;
		INT CurrentPattern = LinePattern[LineStyle][PattenIndex] - (Count == 0 ? FirstOffset : 0);

		FVector NewEndPos = NewStartPos + Direction * CurrentPattern;
		if( PattenIndex % 2 == 0)
		{
			BatchedElements->AddLine(NewStartPos,NewEndPos,Color,HitProxyId);
		}

		NewStartPos = NewEndPos;
		Count++;
	}
}

void DrawTile(
	FCanvas* Canvas,
	FLOAT X,
	FLOAT Y,
	FLOAT SizeX,
	FLOAT SizeY,
	FLOAT U,
	FLOAT V,
	FLOAT SizeU,
	FLOAT SizeV,
	const FLinearColor& Color,
	const FTexture* Texture,
	UBOOL AlphaBlend
	)
{
	if (AlphaBlend && Color.A == 0)
		return;

	FLinearColor ActualColor = Color;
	ActualColor.A *= Canvas->AlphaModulate;

	const FTexture* FinalTexture = Texture ? Texture : GWhiteTexture;
	const EBlendMode BlendMode = AlphaBlend ? BLEND_Translucent : BLEND_Opaque;
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Triangle, FinalTexture, BlendMode);	
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	INT V00 = BatchedElements->AddVertex(FVector4(X,		Y,			0,1),FVector2D(U,			V),			ActualColor,HitProxyId);
	INT V10 = BatchedElements->AddVertex(FVector4(X + SizeX,Y,			0,1),FVector2D(U + SizeU,	V),			ActualColor,HitProxyId);
	INT V01 = BatchedElements->AddVertex(FVector4(X,		Y + SizeY,	0,1),FVector2D(U,			V + SizeV),	ActualColor,HitProxyId);
	INT V11 = BatchedElements->AddVertex(FVector4(X + SizeX,Y + SizeY,	0,1),FVector2D(U + SizeU,	V + SizeV),	ActualColor,HitProxyId);

	BatchedElements->AddTriangle(V00,V10,V11,FinalTexture,BlendMode);
	BatchedElements->AddTriangle(V00,V11,V01,FinalTexture,BlendMode);
}

void DrawTile(
	FCanvas* Canvas,
	FLOAT X,
	FLOAT Y,
	FLOAT SizeX,
	FLOAT SizeY,
	FLOAT U,
	FLOAT V,
	FLOAT SizeU,
	FLOAT SizeV,
	const FMaterialInstance* MaterialInstance
	)
{
	// we must flush because materials aren't batched, so to preserve drawing order, we kick all tile renders so far
	Canvas->Flush();

	FSceneViewFamily* ViewFamily = new FSceneViewFamily(
		Canvas->GetRenderTarget(),
		NULL,
		SHOW_DefaultGame,
		GWorld->GetTimeSeconds(),
		GWorld->GetRealTimeSeconds(),
		NULL
		);

	// make a temporary view
	FViewInfo* View = new FViewInfo(ViewFamily, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, Canvas->GetRenderTarget()->GetSizeX(), Canvas->GetRenderTarget()->GetSizeY(), FMatrix::Identity, Canvas->GetFullTransform(), Canvas->GetFullTransform(), 90, FLinearColor::Black, FLinearColor::White, FLinearColor::White, TArray<FPrimitiveSceneInfo*>());

	// Render the batched elements.
	struct FDrawTileParameters
	{
		FViewInfo* View;
		FLOAT X;
		FLOAT Y;
		FLOAT SizeX;
		FLOAT SizeY;
		FLOAT U;
		FLOAT V;
		FLOAT SizeU;
		FLOAT SizeV;
		const FMaterialInstance* MaterialInstance;
		UBOOL bIsHitTesting;
		FHitProxyId HitProxyId;
	};
	FDrawTileParameters DrawTileParameters =
	{
		View,
		X,
		Y,
		SizeX,
		SizeY,
		U,
		V,
		SizeU,
		SizeV,
		MaterialInstance,
		Canvas->IsHitTesting(),
		Canvas->GetHitProxyId()
	};
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		DrawTileCommand,
		FDrawTileParameters,Parameters,DrawTileParameters,
		{
			FTileRenderer TileRenderer;
			FCommandContextRHI* GlobalContext = RHIGetGlobalContext();
			TileRenderer.DrawTile(GlobalContext, *Parameters.View, Parameters.MaterialInstance, 
				Parameters.X, Parameters.Y, Parameters.SizeX, Parameters.SizeY, 
				Parameters.U, Parameters.V, Parameters.SizeU, Parameters.SizeV,
				Parameters.bIsHitTesting, Parameters.HitProxyId);

			delete Parameters.View->Family;
			delete Parameters.View;
		});
}

void DrawTriangle2D(
	FCanvas* Canvas,
	const FVector2D& Position0,
	const FVector2D& TexCoord0,
	const FVector2D& Position1,
	const FVector2D& TexCoord1,
	const FVector2D& Position2,
	const FVector2D& TexCoord2,
	const FLinearColor& Color,
	const FTexture* Texture,
	UBOOL AlphaBlend
	)
{
	if (AlphaBlend && Color.A == 0)
		return;

	const EBlendMode BlendMode = AlphaBlend ? BLEND_Translucent : BLEND_Opaque;
	const FTexture* FinalTexture = Texture ? Texture : GWhiteTexture;
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Triangle, FinalTexture, BlendMode);
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	INT V0 = BatchedElements->AddVertex(FVector4(Position0.X,Position0.Y,0,1),TexCoord0,Color,HitProxyId);
	INT V1 = BatchedElements->AddVertex(FVector4(Position1.X,Position1.Y,0,1),TexCoord1,Color,HitProxyId);
	INT V2 = BatchedElements->AddVertex(FVector4(Position2.X,Position2.Y,0,1),TexCoord2,Color,HitProxyId);

	BatchedElements->AddTriangle(V0,V1,V2,FinalTexture, BlendMode);
}

void DrawTriangle2D(
	FCanvas* Canvas,
	const FVector2D& Position0,
	const FVector2D& TexCoord0,
	const FVector2D& Position1,
	const FVector2D& TexCoord1,
	const FVector2D& Position2,
	const FVector2D& TexCoord2,
	const FMaterialInstance* MaterialInstance
	)
{}

INT DrawStringCentered(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color)
{
	INT XL, YL;
	StringSize( Font, XL, YL, Text );

	return DrawString(Canvas, StartX-(XL/2), StartY, Text, Font, Color );
}

INT DrawString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color,FLOAT XScale, FLOAT YScale)
{
	if(Font == NULL || Text == NULL)
	{
		return FALSE;
	}

	XScale *= Canvas->FontRatioFactorX;
	YScale *= Canvas->FontRatioFactorY;

	StartX = appTrunc(StartX);
	StartY = appTrunc(StartY);

	// Draw all characters in string.
	FLOAT LineX = 0;
	WCHAR PrevChar = 0;
	for( INT i=0; Text[i]; i++ )
	{
		INT Ch = (TCHARU)Font->RemapChar(Text[i]);

		// Process character if it's valid.
		if( Ch < Font->Characters.Num() )
		{
			const FFontCharacter& Char = Font->Characters(Ch);
			UTexture2D* Tex;
			// Font->Texture.Num() == 0, Font로딩하는 부분이 현재 엔진과 다른것으로 보임. 원래대로 롤백
//			if( Char.TextureIndex < Font->Textures.Num() && (Tex=Font->Textures(Char.TextureIndex))!=NULL )
			if( (Tex=Font->GetTexture(Char.TextureIndex))!=NULL )
			{
				INT Kerning = Char.CalculateKerning( Font->Kerning, PrevChar );

				PrevChar = Ch;
				//LineX += (Char.A + Kerning) * XScale * Canvas->FontRatioFactorX;
				LineX += (Char.A + Kerning) * XScale;
				
				const FLOAT X      = LineX + StartX;
				const FLOAT Y      = StartY;
				const FLOAT CU     = Char.StartU;
				const FLOAT CV     = Char.StartV;
				const FLOAT CUSize = Char.USize;
				const FLOAT CVSize = Char.VSize;
				const FLOAT ScaledSizeU = CUSize * XScale;
				const FLOAT ScaledSizeV = CVSize * YScale;

				//! 폰트 비율 계수값을 곱하도록 수정(2007/01/11 고광록).
				//! Canvas->FontRatioFactorX, FontRatioFactorY를 사용.

				// Draw.
				DrawTile(
					Canvas,
					X,
					Y,
					ScaledSizeU,
					ScaledSizeV,
					CU		/ (FLOAT)Tex->SizeX,
					CV		/ (FLOAT)Tex->SizeY,
					CUSize	/ (FLOAT)Tex->SizeX,
					CVSize	/ (FLOAT)Tex->SizeY,
					Color,
					Tex->Resource
					);


				// Update the current rendering position

				// Update underline status.
				// Char.C는 아바에서 구현한것으로 케릭터 공간을 나타낸다.
				LineX += (ScaledSizeU + (Char.C * XScale));
			}
		}
		else
		{
			PrevChar = 0;
		}
	}

	return appTrunc(LineX);
}

INT DrawShadowedString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color)
{
	// Draw a shadow of the text offset by 1 pixel in X and Y.
	DrawString(Canvas,StartX + 1,StartY + 1,Text,Font,FLinearColor::Black);

	// Draw the text.
	return DrawString(Canvas,StartX,StartY,Text,Font,Color);
}

void StringSize(UFont* Font,INT& XL,INT& YL,const TCHAR* Format,...)
{
	TCHAR Text[4096];
	GET_VARARGS( Text, ARRAY_COUNT(Text), Format, Format );

	// this functionality has been moved to a static function in UIString
	FRenderParameters Parameters(Font,1.f,1.f);
	UUIString::StringSize(Parameters, Text);

	XL = appTrunc(Parameters.DrawXL);
	YL = appTrunc(Parameters.DrawYL);
}

void StringSize(UFont* Font,const TCHAR* Format,INT& XL,INT& YL)
{
	// this functionality has been moved to a static function in UIString
	FRenderParameters Parameters(Font,1.f,1.f);
	UUIString::StringSize(Parameters, Format);

	XL = appTrunc(Parameters.DrawXL);
	YL = appTrunc(Parameters.DrawYL);
}

void SetRenderTarget(FRenderTarget* RenderTarget)
{}

/*-----------------------------------------------------------------------------
	UCanvas object functions.
-----------------------------------------------------------------------------*/

void UCanvas::Init()
{
}

void UCanvas::Update()
{
	// Call UnrealScript to reset.
	eventReset();

	// Copy size parameters from viewport.
	ClipX = SizeX;
	ClipY = SizeY;

}

/*-----------------------------------------------------------------------------
	UCanvas scaled sprites.
-----------------------------------------------------------------------------*/

//
// Draw arbitrary aligned rectangle.
//
void UCanvas::DrawTile
(
	UTexture2D*			Tex,
	FLOAT				X,
	FLOAT				Y,
	FLOAT				XL,
	FLOAT				YL,
	FLOAT				U,
	FLOAT				V,
	FLOAT				UL,
	FLOAT				VL,
	const FLinearColor&	Color
)
{
    if ( !Canvas || !Tex ) 
        return;

	FLOAT MyClipX = OrgX + ClipX;
	FLOAT MyClipY = OrgY + ClipY;
	FLOAT w = X + XL > MyClipX ? MyClipX - X : XL;
	FLOAT h = Y + YL > MyClipY ? MyClipY - Y : YL;
	if (XL > 0.f &&
		YL > 0.f)
	{
		::DrawTile(
			Canvas,
			appTrunc(X),
			appTrunc(Y),
			appTrunc(w),
			appTrunc(h),
			U/Tex->SizeX,
			V/Tex->SizeY,
			UL/Tex->SizeX * w/XL,
			VL/Tex->SizeY * h/YL,
			Color,
			Tex->Resource
			);
	}
}

void UCanvas::DrawTileRotate( UTexture2D *Tex, FLOAT AxisX, FLOAT AxisY, FLOAT Angle,FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& Color)
{
	FMatrix TransMat = FRotationMatrix( FRotator(0, appTrunc(Angle) ,0))
		* FTranslationMatrix( FVector(AxisX, AxisY, 0));

	Canvas->PushRelativeTransform(TransMat);
	DrawTile(Tex, X,Y, XL, YL, U, V, UL, VL, Color );
	Canvas->PopTransform();
}

void UCanvas::DrawMaterialTile(
	UMaterialInstance*	Material,
	FLOAT				X,
	FLOAT				Y,
	FLOAT				XL,
	FLOAT				YL,
	FLOAT				U,
	FLOAT				V,
	FLOAT				UL,
	FLOAT				VL
	)
{
    if ( !Canvas || !Material ) 
        return;

	::DrawTile(
		Canvas,
		appTrunc(X),
		appTrunc(Y),
		appTrunc(XL),
		appTrunc(YL),
		U,
		V,
		UL,
		VL,
		Material->GetInstanceInterface(0)
		);
}

void UCanvas::ClippedStrLen( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Text )
{
	XL = 0;
	YL = 0;
	if (Font != NULL)
	{
		FRenderParameters Parameters(Font,ScaleX,ScaleY);
		UUIString::StringSize(Parameters, Text);

		XL = appTrunc(Parameters.DrawXL);
		YL = appTrunc(Parameters.DrawYL);
		/*
		for( INT i=0; Text[i]; i++)
		{
			INT W, H;
			Font->GetCharSize( Text[i], W, H );
			if (Text[i + 1])
			{
				W = appTrunc((FLOAT)(W / *+ SpaceX + Font->Kerning* /) * ScaleX);
			}
			else
			{
				W = appTrunc((FLOAT)(W) * ScaleX);
			}
			H = appTrunc((FLOAT)(H) * ScaleY);
			XL += W;
			if(YL < H)
			{
				YL = H;	
			}
		}
		*/
	}
}

//
// Calculate the size of a string built from a font, word wrapped
// to a specified region.
//
void VARARGS UCanvas::WrappedStrLenf( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Fmt, ... ) 
{
	TCHAR Text[4096];
	GET_VARARGS( Text, ARRAY_COUNT(Text), Fmt, Fmt );

	WrappedPrint( 0, XL, YL, Font, ScaleX, ScaleY, 0, Text ); 
}

//
// Compute size and optionally print text with word wrap.
//!!For the next generation, redesign to ignore CurX,CurY.
//
void UCanvas::WrappedPrint( UBOOL Draw, INT& out_XL, INT& out_YL, UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Text ) 
{
	// FIXME: Wrapped Print is screwed which kills the hud.. fix later

	if( ClipX<0 || ClipY<0 )
		return;
	check(Font);

	// Process each word until the current line overflows.
	FLOAT XL=0.f, YL=0.f;
	do
	{
		INT iCleanWordEnd=0, iTestWord;
		FLOAT TestXL= CurX, CleanXL=0;
		FLOAT TestYL=0,    CleanYL=0;
		UBOOL GotWord=0;
		TCHAR PrevChar = 0;
		for( iTestWord=0; Text[iTestWord]!=0 && Text[iTestWord]!='\n'; )
		{
			FLOAT ChW, ChH;
			Font->GetCharSize(Text[iTestWord], ChW, ChH, 0, PrevChar);
			PrevChar = Text[iTestWord];
			TestXL              += (ChW + SpaceX /*+ Font->Kerning*/) * ScaleX; 
			TestYL               = Max(TestYL, (ChH + SpaceY) * ScaleY);
			if( TestXL>ClipX )
				break;
			iTestWord++;
			UBOOL WordBreak = Text[iTestWord]==' ' || Text[iTestWord]=='\n' || Text[iTestWord]==0;
			if( WordBreak || !GotWord )
			{
				iCleanWordEnd = iTestWord;
				CleanXL       = TestXL;
				CleanYL       = TestYL;
				GotWord       = GotWord || WordBreak;				
			}
		}
		if( iCleanWordEnd==0 )
			break;

		// Sucessfully split this line, now draw it.
		if (Draw && CurY < ClipY && CurY + CleanYL > 0)
		{
			FString TextLine(Text);
			FLOAT LineX = Center ? CurX+(ClipX-CleanXL)/2 : CurX;
			LineX += DrawString(Canvas,OrgX + LineX, OrgY + CurY,*(TextLine.Left(iCleanWordEnd)),Font,DrawColor);
			CurX = LineX;
		}

		// Update position.
		CurX  = 0;
		CurY += CleanYL;
		YL   += CleanYL;
		XL    = Max(XL,CleanXL);
		Text += iCleanWordEnd;

		// Skip whitespace after word wrap.
		while( *Text==' '|| *Text=='\n' )
			Text++;
	}
	while( *Text );

	out_XL = appTrunc(XL);
	out_YL = appTrunc(YL);
}


/*-----------------------------------------------------------------------------
	UCanvas natives.
-----------------------------------------------------------------------------*/
void UCanvas::execDrawTile _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture2D,Tex);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_FINISH;
	if( !Tex )
		return;

	DrawTile
	(
		Tex,
		OrgX+CurX,
		OrgY+CurY,
		XL,
		YL,
		U,
		V,
		UL,
		VL,
		DrawColor
	);
	CurX += XL + SpaceX;
	CurYL = Max(CurYL,YL);
}
IMPLEMENT_FUNCTION( UCanvas, 466, execDrawTile );

void UCanvas::execDrawTileRotate _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture2D,Tex);
	P_GET_FLOAT(AxisX);
	P_GET_FLOAT(AxisY);
	P_GET_FLOAT(Angle);
	P_GET_FLOAT(X);
	P_GET_FLOAT(Y);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_FINISH;

	if( !Tex )
		return;
	DrawTileRotate(Tex, AxisX, AxisY, Angle * 65535.0f / 360.0f, X, Y, XL, YL, U, V, UL, VL, DrawColor);
}

void UCanvas::execDrawMaterialTile _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UMaterialInstance,Material);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT_OPTX(U,0.f);
	P_GET_FLOAT_OPTX(V,0.f);
	P_GET_FLOAT_OPTX(UL,1.f);
	P_GET_FLOAT_OPTX(VL,1.f);
	P_FINISH;
	if(!Material)
		return;
	DrawMaterialTile
	(
		Material,
		OrgX+CurX,
		OrgY+CurY,
		XL,
		YL,
		U,
		V,
		UL,
		VL
	);
	CurX += XL + SpaceX;
	CurYL = Max(CurYL,YL);
}
IMPLEMENT_FUNCTION( UCanvas, INDEX_NONE, execDrawMaterialTile );

void UCanvas::execDrawText _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(InText);
	P_GET_UBOOL_OPTX(CR,1);
	P_FINISH;

	if( !Font )
	{
		Stack.Logf( NAME_Warning, TEXT("DrawText: No font") ); 
		return;
	}
	INT		XL		= 0;
	INT		YL		= 0; 
	FLOAT	OldCurX	= CurX;
	FLOAT	OldCurY	= CurY;
	WrappedPrint( 1, XL, YL, Font, 1.f, 1.f, bCenter, *InText ); 
    
	CurX += XL;
	CurYL = Max(CurYL,(FLOAT)YL);
	if( CR )
	{
		CurX	= OldCurX;
		CurY	= OldCurY + CurYL;
		CurYL	= 0;
	}

}
IMPLEMENT_FUNCTION( UCanvas, 465, execDrawText );

void UCanvas::execDrawTextClipped _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(InText);
	P_GET_UBOOL_OPTX(CheckHotKey, 0);
	P_FINISH;

	if( !Font )
	{
		Stack.Logf( TEXT("DrawTextClipped: No font") ); 
		return;
	}

	check(Font);

	DrawString(Canvas,appTrunc(OrgX + CurX), appTrunc(OrgY + CurY), *InText, Font, DrawColor);

}
IMPLEMENT_FUNCTION( UCanvas, 469, execDrawTextClipped );

void UCanvas::execStrLen _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL ) // wrapped 
{
	P_GET_STR(InText);
	P_GET_FLOAT_REF(XL);
	P_GET_FLOAT_REF(YL);
	P_FINISH;

	INT XLi, YLi;
	INT OldCurX, OldCurY;

	OldCurX = appTrunc(CurX);
	OldCurY = appTrunc(CurY);
	CurX = 0;
	CurY = 0;

	WrappedStrLenf( Font, 1.f, 1.f, XLi, YLi, TEXT("%s"), *InText );

	CurY = OldCurY;
	CurX = OldCurX;
	XL = XLi;
	YL = YLi;

}
IMPLEMENT_FUNCTION( UCanvas, 464, execStrLen );

void UCanvas::execTextSize _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL ) // clipped
{
	P_GET_STR(InText);
	P_GET_FLOAT_REF(XL);
	P_GET_FLOAT_REF(YL);
	P_FINISH;

	INT XLi, YLi;

	if( !Font )
	{
		Stack.Logf( TEXT("TextSize: No font") ); 
		return;
	}

	ClippedStrLen( Font, 1.f, 1.f, XLi, YLi, *InText );

	XL = XLi;
	YL = YLi;

}
IMPLEMENT_FUNCTION( UCanvas, 470, execTextSize );

void UCanvas::execProject _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_VECTOR(Location);
	P_FINISH;

	FVector ViewLoc;
	FRotator ViewRot;
	FPlane V(0,0,0,0);
	FSceneView *LocalSceneView;

	// [ 8/31/2006 otterrrr ] 
	// SceneView가 없는 경우에는 LocalPlayer의 Viewport를 얻어와 LocalSceneView를 생성한다.
	ULocalPlayer* Player = GEngine->GamePlayers(0);				// Assume that the index of LocalPlayer is '0'
	FViewport* Viewport = Player->ViewportClient->Viewport;
	QWORD ShowFlags = Player->ViewportClient->ShowFlags;

	if( SceneView == NULL )
	{
		FSceneViewFamilyContext ViewFamily(Viewport ,GWorld->Scene, ShowFlags ,GWorld->GetTimeSeconds(),GWorld->GetRealTimeSeconds(), GWorld->bGatherDynamicShadowStatsSnapshot ? &GWorld->DynamicShadowStats : NULL, TRUE );
		LocalSceneView = Player->CalcSceneView( &ViewFamily, ViewLoc, ViewRot, Viewport);
		V = LocalSceneView->Project(Location);
	}
	else
		V = SceneView->Project(Location);
	//  [8/31/2006 otterrrr]

	FVector resultVec(V);
	resultVec.X = (ClipX/2.f) + (resultVec.X*(ClipX/2.f));
	resultVec.Y *= -1.f;
	resultVec.Y = (ClipY/2.f) + (resultVec.Y*(ClipY/2.f));
	*(FVector*)Result =	resultVec;
}
IMPLEMENT_FUNCTION( UCanvas, -1, execProject);

/* unused
//
// Wrapped printf.
//
void VARARGS UCanvas::WrappedPrintf( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Fmt, ... ) 
{
	TCHAR Text[4096];
	GET_VARARGS( Text, ARRAY_COUNT(Text), Fmt, Fmt );

	INT XL=0, YL=0;
	WrappedPrint( 1, XL, YL, Font, ScaleX, ScaleY, Center, Text ); 
}

void UCanvas::ClippedPrint( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Text )
{
	DrawString(Canvas,appTrunc(OrgX+CurX), appTrunc(OrgY+CurY), Text, Font, DrawColor);
}

//
// Draw titling pattern.
//
void UCanvas::DrawPattern
(
	UTexture2D*			Tex,
	FLOAT				X,
	FLOAT				Y,
	FLOAT				XL,
	FLOAT				YL,
	FLOAT				Scale,
	FLOAT				OrgX,
	FLOAT				OrgY,
	const FLinearColor&	Color
)
{
	DrawTile( Tex, X, Y, XL, YL, (X-OrgX)*Scale + Tex->SizeX, (Y-OrgY)*Scale + Tex->SizeY, XL*Scale, YL*Scale, Color );
}


//
// Draw a scaled sprite.  Takes care of clipping.
// XSize and YSize are in pixels.
//
void UCanvas::DrawIcon
(
	UTexture2D*			Tex,
	FLOAT				ScreenX, 
	FLOAT				ScreenY, 
	FLOAT				XSize, 
	FLOAT				YSize,
	const FLinearColor&	Color
)
{
	DrawTile( Tex, ScreenX, ScreenY, XSize, YSize, 0, 0, 1, 1, Color );
}

// Notice: This will actually append to the array for continous wrapping, does not support tabs yet
void UCanvas::WrapStringToArray( const TCHAR *Text, TArray<FString> *OutArray, float Width, UFont *pFont, TCHAR EOL)
{
	check(Text != NULL);
	check(OutArray != NULL);

	const TCHAR *LastText;

	if (*Text == '\0')
		return;

	if (pFont == NULL)
		pFont = this->Font;

	do
	{
		while (*Text == EOL)
		{
		INT iAdded = OutArray->AddZeroed();

			(*OutArray)(iAdded) = FString(TEXT(""));
			Text++;
		}
		if (*Text==0)
			break;

		LastText = Text;

		INT iCleanWordEnd=0, iTestWord;
		INT TestXL=0, CleanXL=0;
		UBOOL GotWord=0;
		for( iTestWord=0; Text[iTestWord]!=0 && Text[iTestWord]!=EOL; )
		{
			INT ChW, ChH;
			pFont->GetCharSize(Text[iTestWord], ChW, ChH);
			TestXL              += appTrunc(((FLOAT)(ChW + SpaceX / *+ pFont->Kerning* /))); 
			if( TestXL>Width )
				break;
			iTestWord++;
			UBOOL WordBreak = Text[iTestWord]==' ' || Text[iTestWord]==EOL || Text[iTestWord]==0;
			if( WordBreak || !GotWord )
			{
				iCleanWordEnd = iTestWord;
				CleanXL       = TestXL;
				GotWord       = GotWord || WordBreak;
			}
		}
		if( iCleanWordEnd==0 )
		{
			if (*Text != 0)
			{
			INT iAdded = OutArray->AddZeroed();

				if (Text == LastText)
				{
					(*OutArray)(iAdded) = FString::Printf(TEXT("%c"), Text);
					Text++;
				}
				else
					(*OutArray)(iAdded) = FString(TEXT(""));
			}
		}
		else
		{
		FString TextLine(Text);
		INT iAdded = OutArray->AddZeroed();

			(*OutArray)(iAdded) = TextLine.Left(iCleanWordEnd);
		}
		Text += iCleanWordEnd;

		// Skip whitespace after word wrap.
		while( *Text==' ' )
			Text++;

		if (*Text == EOL)
			Text++;
	}
	while( *Text );
}

void UCanvas::execWrapStringToArray _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(InText);
	P_GET_TARRAY_REF(FString,OutArray);
	P_GET_FLOAT(Width);
	P_GET_STR_OPTX(EOL, TEXT("|"));
	P_FINISH;

	WrapStringToArray(*InText, &OutArray, Width, NULL, EOL[0]);

}
IMPLEMENT_FUNCTION( UCanvas, -1, execWrapStringToArray );

void UCanvas::execDrawTileClipped _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture2D,Tex);
	P_GET_FLOAT(XL);
	P_GET_FLOAT(YL);
	P_GET_FLOAT(U);
	P_GET_FLOAT(V);
	P_GET_FLOAT(UL);
	P_GET_FLOAT(VL);
	P_FINISH;

	if( !Tex )
	{
		Stack.Logf( TEXT("DrawTileClipped: Missing Material") );
		return;
	}


	// Clip to ClipX and ClipY
	if( XL > 0 && YL > 0 )
	{		
		if( CurX<0 )
			{FLOAT C=CurX*UL/XL; U-=C; UL+=C; XL+=CurX; CurX=0;}
		if( CurY<0 )
			{FLOAT C=CurY*VL/YL; V-=C; VL+=C; YL+=CurY; CurY=0;}
		if( XL>ClipX-CurX )
			{UL+=(ClipX-CurX-XL)*UL/XL; XL=ClipX-CurX;}
		if( YL>ClipY-CurY )
			{VL+=(ClipY-CurY-YL)*VL/YL; YL=ClipY-CurY;}
	
		DrawTile
		(
			Tex,
			OrgX+CurX,
			OrgY+CurY,
			XL,
			YL,
			U,
			V,
			UL,
			VL,
			DrawColor
		);

		CurX += XL + SpaceX;
		CurYL = Max(CurYL,YL);
	}

}
IMPLEMENT_FUNCTION( UCanvas, 468, execDrawTileClipped );
*/
/* unused
void UCanvas::execDrawTileStretched _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture2D,Tex);
	P_GET_FLOAT(AWidth);
	P_GET_FLOAT(AHeight);
	P_FINISH;

	DrawTileStretched(Tex,CurX, CurY, AWidth, AHeight);

}
IMPLEMENT_FUNCTION( UCanvas, -1, execDrawTileStretched );

void UCanvas::DrawTileStretched(UTexture2D* Tex, FLOAT Left, FLOAT Top, FLOAT AWidth, FLOAT AHeight)
{

	CurX = Left;
	CurY = Top;

	if( !Tex )
		return;

	// Get the size of the image

	FLOAT mW = Tex->SizeX;
	FLOAT mH = Tex->SizeY;

	// Get the midpoints of the image

	FLOAT MidX = appFloor(mW/2);
	FLOAT MidY = appFloor(mH/2);

	// Grab info about the scaled image

	FLOAT SmallTileW = AWidth - mW;
	FLOAT SmallTileH = AHeight - mH;
	FLOAT fX, fY;		// Used to shrink

	// Draw the spans first

		// Top and Bottom

	if (mW<AWidth)
	{
		fX = MidX;

		if (mH>AHeight)
			fY = AHeight/2;
		else
			fY = MidY;

		DrawTile(Tex, CurX+fX,	CurY,					SmallTileW,		fY,		MidX,	0,			1,		fY,	DrawColor);
		DrawTile(Tex, CurX+fX,	CurY+AHeight-fY,		SmallTileW,		fY,		MidX,	mH-fY,		1,		fY,	DrawColor);
	}
	else
		fX = AWidth / 2;

		// Left and Right

	if (mH<AHeight)
	{

		fY = MidY;

		DrawTile(Tex, CurX,				CurY+fY,	fX,	SmallTileH,		0,		fY, fX,	1, DrawColor);
		DrawTile(Tex, CurX+AWidth-fX,	CurY+fY,	fX,	SmallTileH,		mW-fX,	fY, fX,	1, DrawColor);

	}
	else
		fY = AHeight / 2; 

		// Center

	if ( (mH<AHeight) && (mW<AWidth) )
		DrawTile(Tex, CurX+fX, CurY+fY, SmallTileW, SmallTileH, fX, fY, 1, 1, DrawColor);

	// Draw the 4 corners.

	DrawTile(Tex, CurX,				CurY,				fX, fY, 0,		0,		fX, fY, DrawColor);
	DrawTile(Tex, CurX+AWidth-fX,	CurY,				fX, fY,	mW-fX,	0,		fX, fY, DrawColor);
	DrawTile(Tex, CurX,				CurY+AHeight-fY,	fX, fY, 0,		mH-fY,	fX, fY, DrawColor);
	DrawTile(Tex, CurX+AWidth-fX,	CurY+AHeight-fY,	fX, fY,	mW-fX,	mH-fY,	fX, fY, DrawColor);

}
void UCanvas::execDrawTileScaled _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture2D,Tex);
	P_GET_FLOAT(NewXScale);
	P_GET_FLOAT(NewYScale);
	P_FINISH;

	DrawTileScaled(Tex,CurX,CurY,NewXScale,NewYScale);

}

IMPLEMENT_FUNCTION( UCanvas, -1, execDrawTileScaled );


void UCanvas::DrawTileBound(UTexture2D* Tex, FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height)
{
	if( Tex )
	{
		DrawTile( Tex, Left, Top, Width, Height, 0, 0, Width, Height, DrawColor );
	}
	else
	{
		debugf(TEXT("DrawTileBound: Missing texture"));
	}
}

void UCanvas::DrawTileScaleBound(UTexture2D* Tex, FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height)
{
	if( Tex )
	{
		FLOAT mW = Tex->SizeX;
		FLOAT mH = Tex->SizeY;
		DrawTile( Tex, Left, Top, Width, Height, 0, 0, mW, mH, DrawColor);
	}
	else
	{
		debugf(TEXT("DrawTileScaleBound: Missing Material"));
	}
}

IMPLEMENT_FUNCTION( UCanvas, -1, execDrawTileJustified );

void UCanvas::execDrawTileJustified _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UTexture2D,Tex);
	P_GET_BYTE(Just);
	P_GET_FLOAT(AWidth);
	P_GET_FLOAT(AHeight);
	P_FINISH;

	DrawTileJustified(Tex, CurX, CurY, AWidth, AHeight, Just);
}


// Justification is: 0 = left/top, 1 = Center, 2 = bottom/right
void UCanvas::DrawTileJustified(UTexture2D* Tex, FLOAT Left, FLOAT Top, FLOAT Width, FLOAT Height, BYTE Justification)
{
	if( Tex )
	{
		FLOAT mW = Tex->SizeX;
		FLOAT mH = Tex->SizeY;

		if (mW <= 0.0 || mH <= 0.0)
			return;

		// Find scaling proportions
		FLOAT MatProps = mH/mW;
		FLOAT BndProps = Height/Width;

		if (MatProps == BndProps)
		{
			DrawTile( Tex, Left, Top, Width, Height, 0, 0, mW, mH, DrawColor);
		}
		else if (MatProps > BndProps)	// Stretch to fit Height
		{
			FLOAT NewWidth = Width * BndProps / MatProps;
			FLOAT NewLeft = Left;

			if (Justification == 1)			// Centered
			{
				NewLeft += ((Width - NewWidth) / 2.0);
			}
			else if (Justification == 2)	// RightBottom
			{
				NewLeft += Width - NewWidth;
			}
			DrawTile( Tex, NewLeft, Top, NewWidth, Height, 0, 0, mW, mH, DrawColor);
		}
		else							// Stretch to fit Width
		{
			FLOAT NewHeight = Height * MatProps / BndProps;
			FLOAT NewTop = Top;

			if (Justification == 1)			// Centered
			{
				NewTop += ((Height - NewHeight) / 2.0);
			}
			else if (Justification == 2)	// RightBottom
			{
				NewTop += Height - NewHeight;
			}
			DrawTile( Tex, Left, NewTop, Width, NewHeight, 0, 0, mW, mH, DrawColor);
		}
	}
	else
	{
		debugf(TEXT("DrawTileScaleBound: Missing texture"));
	}
}

void UCanvas::DrawTileScaled(UTexture2D* Tex, FLOAT Left, FLOAT Top, FLOAT NewXScale, FLOAT NewYScale)
{
	if( Tex )
	{
		FLOAT mW = Tex->SizeX;
		FLOAT mH = Tex->SizeY;
		DrawTile( Tex, Left, Top, mW*NewXScale, mH*NewYScale, 0, 0, mW, mH, DrawColor);
	}
	else
	{
		debugf(TEXT("DrawTileScaled: Missing texture"));
	}
}

IMPLEMENT_FUNCTION( UCanvas, -1, execDrawTextJustified );

void UCanvas::execDrawTextJustified _ParamList_ScriptFunctionImpl // 20080221 dEAthcURe|CD // ( FFrame& Stack, RESULT_DECL )
{
	P_GET_STR(InText);
	P_GET_BYTE(Just);
	P_GET_FLOAT(X1);
	P_GET_FLOAT(Y1);
	P_GET_FLOAT(X2);
	P_GET_FLOAT(Y2);
	P_FINISH;

	DrawTextJustified(Just,X1,Y1,X2,Y2, TEXT("%s"),*InText);

}


void VARARGS UCanvas::DrawTextJustified(BYTE Justification, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, const TCHAR* Fmt, ... )
{
	TCHAR Text[4096];
	GET_VARARGS( Text, ARRAY_COUNT(Text), Fmt, Fmt );

	INT XL,YL;
	CurX = 0;
	CurY = 0;
	ClippedStrLen(  Font, 1.0f,1.0f, XL, YL, Text );

	// Adjust the Y so the Font is centered in the bounding box
	CurY = ((y2-y1) / 2) - (YL/2);

	if (Justification == 0)						// Left
		CurX = 0;
	else if (Justification == 1)				// Center
	{
		if( XL > x2-x1 ) // align left when there's no room
			CurX = 0;
		else
			CurX = ((x2-x1) / 2) - (XL/2);
	}
	else if (Justification == 2)				// Right
		CurX = (x2-x1) - XL;

	FLOAT OldClipX = ClipX;
	FLOAT OldClipY = ClipY;
	FLOAT OldOrgX = OrgX;
	FLOAT OldOrgY = OrgY;

	// Clip to box
	OrgX = x1;
	OrgY = y1;
	ClipX = x2-x1;
	ClipY = y2-y1;

	ClippedPrint(Font, 1.0f, 1.0f, false, Text);

	ClipX = OldClipX;
	ClipY = OldClipY;
	OrgX = OldOrgX;
	OrgY = OldOrgY;
}
*/

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
