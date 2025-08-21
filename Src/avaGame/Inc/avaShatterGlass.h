#pragma once

//
//	UavaShatterGlassComponent
//

enum WinSide_t 
{
	WIN_SIDE_BOTTOM,
	WIN_SIDE_RIGHT,
	WIN_SIDE_TOP,
	WIN_SIDE_LEFT,
};

enum WinEdge_t 
{
	EDGE_NOT	= -1,		// No edge
	EDGE_NONE,				// No edge on both sides	/##\  
	EDGE_FULL,				// Edge on both sides		|##|
	EDGE_LEFT,				// Edge is on left only		|##\ 
	EDGE_RIGHT,				// Edge is on right only	/##|
};

#define MAX_NUM_PANELS 16
#define BITS_PANEL_IS_SOLID		(1<<0)
#define BITS_PANEL_IS_STALE		(1<<1)

#define NUM_EDGE_TYPES		4
#define NUM_EDGE_STYLES		3

struct FEdgeRenderInfo
{
	char			x;
	char			z;
	char			Side;
	char			EdgeType;
	char			Style;	
};

struct FSolidRenderInfo
{
	FSolidRenderInfo( const FVector& Base, const FVector& X, const FVector& Z )
		: Base(Base), X(X), Z(Z)
	{
	}

	FVector			Base;
	FVector			X;
	FVector			Z;
};

struct FShatterGlassVertex
{
	FVector			Position;	// 12
	FPackedNormal	TangentX;	// 12
	FPackedNormal	TangentY;
	FPackedNormal	TangentZ;

	/** Decal mesh texture coordinates. */
	FVector2D		UV;			// 8 (32)

	/** Transforms receiver tangent basis into decal tangent basis for normal map lookup. */
	FVector2D		DetailUV;	// 8 (40)

	BYTE			Padding[24];

	FShatterGlassVertex() {}
	FShatterGlassVertex(const FVector& InPosition,
		const FPackedNormal& InTangentX,
		const FPackedNormal& InTangentY,
		const FPackedNormal& InTangentZ,
		const FVector2D& InUV,
		const FVector2D& InDetailUV)
		:	Position( InPosition )
		,	TangentX( InTangentX )
		,	TangentY( InTangentY )
		,	TangentZ( InTangentZ )
		,	UV( InUV )
		,	DetailUV( InDetailUV )		
	{}
};

struct FShatterGlassSceneProxy;





class UavaShatterGlassComponent;

struct FShatterGlassRenderContext
{
	FShatterGlassRenderContext( UavaShatterGlassComponent* Component )
		: Component( Component )
	{
	}

	UavaShatterGlassComponent*					Component;
	TArray<FShatterGlassVertex>					Vertices[13];
	TArray<WORD>								Indices[13];	

	FVector Origin, X, UnitZ;
	FPackedNormal TangentX, TangentY, TangentZ;

	void Reset();
	void AddSolidBlock( int width, int height, int nHCount );
	void AddEdge( int edgeType, int style, int width, int height, WinSide_t nEdge );
};




class UavaShatterGlassComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UavaShatterGlassComponent,UPrimitiveComponent,CLASS_NoExport,avaGame);
public:
	DECLARE_FUNCTION(execBreakAll)
	{		
		P_FINISH;
		BreakAll();
	}

	DECLARE_FUNCTION(execBreak)
	{		
		P_GET_INT(x);
		P_GET_INT(z);
		P_FINISH;
		Break(x,z);
	}

	DECLARE_FUNCTION(execBreakBitmap)
	{		
		P_GET_INT(x);
		P_GET_INT(z);
		P_FINISH;
		BreakBitmap(x,(UINT)z);
	}

	DECLARE_FUNCTION(execUpdateEdges)
	{		
		P_FINISH;
		UpdateEdges();
	}

	DECLARE_FUNCTION(execReset)
	{		
		P_FINISH;
		Reset();
	}

	DECLARE_FUNCTION(execGlassTick)
	{	
		P_FINISH;

		GlassTick();
	}		

	DECLARE_FUNCTION(execTakeDamage)
	{
		P_GET_INT(Damage);
		P_GET_OBJECT(AController,EventInstigator);
		P_GET_VECTOR(HitLocation);
		P_GET_VECTOR(Momentum);
		P_GET_OBJECT(UClass,DamageType);
		P_GET_STRUCT_INIT(FTraceHitInfo,HitInfo);
		P_GET_OBJECT(AActor,DamageCauser);

		P_FINISH;

		TakeDamage( Damage, EventInstigator, HitLocation, Momentum, DamageType, HitInfo, DamageCauser );
	}

	UMaterialInstance*	Material;
	UMaterialInstance*	BrokenMaterial;
	INT NumWide;
	INT NumHigh;		
	INT Fragility;	
	BITFIELD bIsBroken:1;
	BITFIELD bNeedsUpdateSupport:1;
	BITFIELD bSolidRenderListUpdated:1;
	float Support[16][16];
	byte State[16][16];
	INT NumBrokenPanes;
	INT NumEdgeTrisToRender, NumSolidTrisToRender;

	FShatterGlassRenderContext* RenderContext;

	UMaterialInstanceConstant* Style[12];	

	FName DetailParameter;

	UTexture2D* Texture_Style[12];	

	TArray<FEdgeRenderInfo> EdgeRenderList;
	TArray<FSolidRenderInfo> SolidRenderList;	

	INT LastRound;

	// UPrimitiveComponent interface

	/** Returns true if the prim is using a material with unlit distortion */
	virtual UBOOL HasUnlitDistortion();
	/** Returns true if the prim is using a material with unlit translucency */
	virtual UBOOL HasUnlitTranslucency();	
	
	virtual void UpdateBounds();

	virtual void Precache();

	virtual UBOOL IsValidComponent() const;

	void DiscardMaterialInstances();

	inline bool InLegalRange(int x, int z)				{ return (x < NumWide && z < NumHigh && x >=0 && z >= 0); }
	inline bool	IsPanelSolid(int x, int z)				{ return (BITS_PANEL_IS_SOLID & State[x][z])!=0; }
	inline bool	IsPanelStale(int x, int z)				{ return (BITS_PANEL_IS_STALE & State[x][z])!=0; }
	inline void	SetPanelSolid(int x, int z, bool value)	{ if (InLegalRange(x,z)) (value ? (State[x][z] |= BITS_PANEL_IS_SOLID) : (State[x][z] &= ~BITS_PANEL_IS_SOLID)); }
	inline void	SetPanelStale(int x, int z, bool value)	{ if (InLegalRange(x,z)) (value ? (State[x][z] |= BITS_PANEL_IS_STALE) : (State[x][z] &= ~BITS_PANEL_IS_STALE)); }

	inline void SetStyleType( int w, int h, int type )
	{
		checkSlow( type < NUM_EDGE_STYLES );
		checkSlow( type >= 0 );
		// Clear old value
		State[ w ][ h ] &= ( ~0x03 << 2 );
		// Insert new value
		State[ w ][ h ] |= ( type << 2 );
	}

	inline int GetStyleType( int w, int h )
	{
		int value = State[ w ][ h ];
		value = ( value >> 2 ) & 0x03;
		checkSlow( value < NUM_EDGE_STYLES );
		return value;
	}

	void PanePos(const FVector &vPos, float *flWidth, float *flHeight);
	void UpdateEdgeType(int nWidth, int nHeight, int forceStyle =-1 );
	bool HavePanel(int nWidth, int nHeight);
	void AddToEdgeRenderList(int nWidth, int nHeight, WinSide_t nSide, WinEdge_t nEdgeType, int forceStyle );
	int FindRenderPanel(int nWidth, int nHeight, WinSide_t nWinSide);
	int FindFirstRenderTexture(WinEdge_t nEdgeType, int nStyle);

	void BreakAll();
	void Break( int x, int z );
	void BreakBitmap( int x, UINT z );
	void UpdateEdges();

	void UpdateSolidRenderList();	
	void RenderEdges(FPrimitiveDrawInterface* PDI,UINT InDepthPriorityGroup);
	void RenderOneBlock(FPrimitiveDrawInterface* PDI,UINT InDepthPriorityGroup,const FVector& Base,const FVector& X,const FVector& Z);

	void Reset();

	UavaShatterGlassComponent();

	virtual void PostEditChange(UProperty* PropertyThatChanged);
	virtual void PostLoad();

	float GetSupport( int x, int z );
	float RecalcSupport( int x, int z );

	void GlassTick();

	bool IsBroken(int x, int z);
	void DropPane(int x, int z);
	UBOOL ShatterPane(int x, int z, const FVector&, const FVector&);
	void BreakPane(int x, int z);
	void BreakAllPanes();
	
	virtual UBOOL PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags);
	virtual UBOOL LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags);
	void TakeDamage( INT Damage, AController* EventInstigator, const FVector& HitLocation, const FVector& Momentum, UClass* DamageType, const FTraceHitInfo& HitInfo, class AActor* DamageCauser );
	void Die();

	void SurfaceTouch( AActor* );			
	
	void UpdateTransform();

	virtual void InitComponentRBPhys(UBOOL bFixed);
	virtual void TermComponentRBPhys(FRBPhysScene* Scene);

	void PrepareMaterialInstances();	

	void PrepareMaterialInstance( UMaterialInstanceConstant*& Result, UTexture2D* Texture );

	void CreateShards( int x, int z, const FVector& Force, const FVector& ForcePos );	

	virtual void Attach();

	virtual FPrimitiveSceneProxy* CreateSceneProxy();

	void UpdateDynamicData();
	void UpdateDynamicData( FShatterGlassSceneProxy* );

	virtual void FinishDestroy();

	virtual void Tick( FLOAT DeltaTime );

	FVector CoordToWorld( INT x, INT z ) const;
};