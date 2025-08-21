/*=============================================================================
	UnCanvas.h: Unreal canvas definition.
	Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

struct FCanvasIcon
{
	class UTexture2D *Texture;
	FLOAT U, V, UL, VL;
};

class FBatchedElements;


/**
 * Line Style (LS_DASH,LS_DOT,LS_DASHDOT,LS_DASHDOTDOT) for DrawLine2DStyled
 */
enum EDrawLineStyle
{
	DLS_DASH,
	DLS_DOT,
	DLS_DASHDOT,
	DLS_DASHDOTDOT,
	DLS_MAX
};

/**
 * Encapsulates the canvas state.
 */
class FCanvas
{
public:	

	/** 
	 * Enum that describes what type of element we are currently batching.
	 */
	enum EElementType
	{
		ET_Line,
		ET_Triangle,
		ET_MAX
	};

	/** 
	* Constructor.
	*/
	FCanvas(FRenderTarget* InRenderTarget,FHitProxyConsumer* InHitProxyConsumer);

	/** 
	* Destructor.
	*/
	~FCanvas();

	/**
	* Returns a FBatchedElements pointer to be used for adding vertices and primitives for rendering.
	* Adds a new render item to the sort element entry based on the current sort key.
	*
	* @param InElementType - Type of element we are going to draw.
	* @param InTexture - New texture that will be set.
	* @param InBlendMode - New blendmode that will be set.	* 
	* @return Returns a pointer to a FBatchedElements object.
	*/
	FBatchedElements* GetBatchedElements(EElementType InElementType, const FTexture* Texture=NULL, EBlendMode BlendMode=BLEND_MAX);
	/** 
	* Sends a message to the rendering thread to draw the batched elements. 
	*/
	void Flush();

	/**
	 * Pushes a transform onto the canvas's transform stack, multiplying it with the current top of the stack.
	 * @param Transform - The transform to push onto the stack.
	 */
	void PushRelativeTransform(const FMatrix& Transform);

	/**
	 * Pushes a transform onto the canvas's transform stack.
	 * @param Transform - The transform to push onto the stack.
	 */
	void PushAbsoluteTransform(const FMatrix& Transform);

	/**
	 * Removes the top transform from the canvas's transform stack.
	 */
	void PopTransform();

	/**
	* Replace the base (ie. TransformStack(0)) transform for the canvas with the given matrix
	*
	* @param Transform - The transform to use for the base
	*/
	void SetBaseTransform(const FMatrix& Transform);

	/**
	* Generate a 2D projection for the canvas. Use this if you only want to transform in 2D on the XY plane
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcBaseTransform2D(UINT ViewSizeX, UINT ViewSizeY);

	/**
	* Generate a 3D projection for the canvas. Use this if you want to transform in 3D 
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @param NearPlane - Distance to the near clip plane
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcBaseTransform3D(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane);
	
	/**
	* Generate a view matrix for the canvas. Used for CalcBaseTransform3D
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @return Matrix for canvas view orientation
	*/
	static FMatrix CalcViewMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV);
	
	/**
	* Generate a projection matrix for the canvas. Used for CalcBaseTransform3D
	*
	* @param ViewSizeX - Viewport width
	* @param ViewSizeY - Viewport height
	* @param fFOV - Field of view for the projection
	* @param NearPlane - Distance to the near clip plane
	* @return Matrix for canvas projection
	*/
	static FMatrix CalcProjectionMatrix(UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane);

	/**
	* Get the current top-most transform entry without the canvas projection
	* @return matrix from transform stack. 
	*/
	FMatrix GetTransform() const 
	{ 
		return TransformStack.Top() * TransformStack(0).Inverse(); 
	}

	/** 
	* Get the bottom-most element of the transform stack. 
	* @return matrix from transform stack. 
	*/
	const FMatrix& GetBottomTransform() const 
	{ 
		return TransformStack(0); 
	}

	/**
	* Get the current top-most transform entry 
	* @return matrix from transform stack. 
	*/
	const FMatrix& GetFullTransform() const 
	{ 
		return TransformStack.Top(); 
	}

	/**
	 * Sets the render target which will be used for subsequent canvas primitives.
	 */
	void SetRenderTarget(FRenderTarget* NewRenderTarget);	

	/**
	* Get the current render target for the canvas
	*/	
	FRenderTarget* GetRenderTarget() const 
	{ 
		return RenderTarget; 
	}
	
	/**
	* Sets the hit proxy which will be used for subsequent canvas primitives.
	*/ 
	void SetHitProxy(HHitProxy* HitProxy);

	// HitProxy Accessors.	

	FHitProxyId GetHitProxyId() const { return CurrentHitProxy ? CurrentHitProxy->Id : FHitProxyId(); }
	FHitProxyConsumer* GetHitProxyConsumer() const { return HitProxyConsumer; }
	UBOOL IsHitTesting() const { return HitProxyConsumer != NULL; }			


public:
	FLOAT AlphaModulate;


	//! 폰트의 해상도에 따른 스케일링 계수를 설정하는 함수(2007/01/11 고광록).
	void	SetFontRatioScaling(bool bEnabled, float RatioX=1.0f, float RatioY=1.0f);

	float	FontRatioFactorX;	//!< 폰트 가로 비율계수.
	float	FontRatioFactorY;	//!< 폰트 세로 비율계수.

private:
	/** Current render target used by the canvas */
	FRenderTarget* RenderTarget;
	/** Current hit proxy consumer */
	FHitProxyConsumer* HitProxyConsumer;
	/** Current hit proxy object */
	TRefCountPtr<HHitProxy> CurrentHitProxy;
	/** Stack of matrices. Bottom most entry is the canvas projection */
	TArray<FMatrix> TransformStack;	

	/** Current batched elements, created and destroyed as needed. */
	FBatchedElements* BatchedElements;

	/** Current texture being used for batching, set to NULL if it hasn't been used yet. */
	FTexture* Texture;

	/** Current blend mode being used for batching, set to BLEND_MAX if it hasn't been used yet. */
	EBlendMode BlendMode;

	/** Current element type being used for batching, set to ET_MAX if it hasn't been used yet. */
	EElementType ElementType;
};

extern void Clear(FCanvas* Canvas,const FLinearColor& Color);

/**
 *	Draws a line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
extern void DrawLine(FCanvas* Canvas,const FVector& StartPos,const FVector& EndPos,const FLinearColor& Color);

/**
 * Draws a 2D line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
extern void DrawLine2D(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos,const FLinearColor& Color);

extern void DrawLine2DStyled(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos, const FLinearColor& Color, EDrawLineStyle LineStyle, INT LineOffset = 0);

extern void DrawBox2D(FCanvas* Canvas,const FVector2D& StartPos,const FVector2D& EndPos,const FLinearColor& Color);

extern void DrawTile(
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
	const FTexture* Texture = NULL,
	UBOOL AlphaBlend = TRUE
	);

extern void DrawTile(
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
	);

extern void DrawTriangle2D(
	FCanvas* Canvas,
	const FVector2D& Position0,
	const FVector2D& TexCoord0,
	const FVector2D& Position1,
	const FVector2D& TexCoord1,
	const FVector2D& Position2,
	const FVector2D& TexCoord2,
	const FLinearColor& Color,
	const FTexture* Texture = NULL,
	UBOOL AlphaBlend = 1
	);

extern void DrawTriangle2D(
	FCanvas* Canvas,
	const FVector2D& Position0,
	const FVector2D& TexCoord0,
	const FVector2D& Position1,
	const FVector2D& TexCoord1,
	const FVector2D& Position2,
	const FVector2D& TexCoord2,
	const FMaterialInstance* MaterialInstance
	);

extern INT DrawStringCentered(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color);

extern INT DrawString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color, FLOAT XScale=1.0, FLOAT YScale=1.0);

extern INT DrawShadowedString(FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color);

extern void StringSize(UFont* Font,INT& XL,INT& YL,const TCHAR* Format,...);

// Kismet Sequence Variable 등 vargs가 필요없는 것들을 그리기 위해 추가.
extern void StringSize(UFont* Font,const TCHAR* Format,INT& XL,INT& YL);

/*
	UCanvas
	A high-level rendering interface used to render objects on the HUD.
*/

class UCanvas : public UObject
{
	DECLARE_CLASS(UCanvas,UObject,CLASS_Transient|CLASS_NoExport,Engine);
	NO_DEFAULT_CONSTRUCTOR(UCanvas);
public:

	// Variables.
	UFont*			Font;
	FLOAT			SpaceX, SpaceY;
	FLOAT			OrgX, OrgY;
	FLOAT			ClipX, ClipY;
	FLOAT			CurX, CurY;
	FLOAT			CurYL;
	FColor			DrawColor;
	BITFIELD		bCenter:1;
	BITFIELD		bNoSmooth:1;
	INT				SizeX, SizeY;
	FCanvas*		Canvas;
	FSceneView*		SceneView;
	FPlane			ColorModulate;
	UTexture2D*		DefaultTexture;

	// UCanvas interface.
	void Init();
	void Update();
	void DrawTile( UTexture2D* Tex, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& Color );
	void DrawMaterialTile( UMaterialInstance* Tex, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL );
	void DrawTileRotate( UTexture2D* Tex, FLOAT AxisX, FLOAT AxisY, FLOAT Angle, FLOAT X, FLOAT Y, FLOAT XL, FLOAT YL, FLOAT U, FLOAT V, FLOAT UL, FLOAT VL, const FLinearColor& Color);
	static void ClippedStrLen( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Text );
	void VARARGS WrappedStrLenf( UFont* Font, FLOAT ScaleX, FLOAT ScaleY, INT& XL, INT& YL, const TCHAR* Fmt, ... );
	
	void WrappedPrint( UBOOL Draw, INT& XL, INT& YL, UFont* Font, FLOAT ScaleX, FLOAT ScaleY, UBOOL Center, const TCHAR* Text ); 
	

	// Natives.
	DECLARE_FUNCTION(execDrawTile);
	DECLARE_FUNCTION(execDrawMaterialTile);

	DECLARE_FUNCTION(execDrawText);
	DECLARE_FUNCTION(execDrawTextClipped);

	/** 2006/12/11 윤태식, DrawTileRotate */
	DECLARE_FUNCTION(execDrawTileRotate);

	DECLARE_FUNCTION(execStrLen);
	DECLARE_FUNCTION(execTextSize);
	DECLARE_FUNCTION(execProject);

    void eventReset()
    {
        ProcessEvent(FindFunctionChecked(TEXT("Reset")),NULL);
    }
};

