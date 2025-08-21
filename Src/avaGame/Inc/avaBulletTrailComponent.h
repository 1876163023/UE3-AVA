#pragma once

class UavaBulletTrailComponent;

struct FBulletTrailSegment
{
	FBulletTrailSegment( const FVector& InP0, const FVector& InP1, FLOAT InU0, FLOAT InU1, FLOAT InS0, FLOAT InS1 )
		: P0( InP0 ), P1( InP1 ), U0( InU0 ), U1( InU1 ), S0( InS0 ), S1( InS1 )
	{
	}

	FVector			P0, P1;
	FLOAT			U0, U1, S0, S1;
};

struct FBulletTrailEntry
{
	FBulletTrailEntry( const FVector& Source, const FVector& Destination, const FVector& InitVel, FLOAT Speed, FLOAT Size, FLOAT AddDeltaTime )
		: Source( Source ), Destination( Destination ), InitVel(InitVel), Speed( Speed ), FiredTime( GWorld->GetTimeSeconds() ), Size( Size ), AddDeltaTime( AddDeltaTime )
	{
		Direction = Destination - Source;

		Length = Direction.Size();

		Direction /= Length;

		checkSlow( Speed != 0 );

		TTL = Length / Speed;
	}

	FVector										Source, Destination, Direction, InitVel;
	FLOAT										Speed;
	FLOAT										FiredTime, TTL, Size;
	FLOAT										AddDeltaTime;
	FLOAT										Length;
};

struct FBulletTrailRenderContext
{
	FBulletTrailRenderContext( UavaBulletTrailComponent* InComponent )
		: Component( InComponent )
	{
	}

	UavaBulletTrailComponent*					Component;
	TArray<FBulletTrailSegment>					Segments;
	TArray<WORD>								Indices;	

	UBOOL AddEntry( const FBulletTrailEntry& );		
};

class UavaBulletTrailComponent : public UPrimitiveComponent
{
	DECLARE_CLASS(UavaBulletTrailComponent,UPrimitiveComponent,CLASS_NoExport,avaGame);
public :
	UMaterialInstance*	Material;	
	FLOAT Intensity;
	FLOAT HalfLife;
	FLOAT Size;
	FLOAT Speed;
	FLOAT AddDeltaTime;
	TArray<FBulletTrailEntry>					BulletTrails;
	FBulletTrailRenderContext*					Context;

	DECLARE_FUNCTION(execFire)
	{
		P_GET_VECTOR(Source);
		P_GET_VECTOR(Destination);
		P_GET_VECTOR(InitVel);
		P_FINISH;
		Fire( Source, Destination, InitVel );
	}

	UavaBulletTrailComponent();
	virtual void FinishDestroy();

	void Fire( const FVector& Source, const FVector& Destination, const FVector& InitVel );	

	/* Render interface */
	virtual UBOOL HasUnlitDistortion();	
	virtual UBOOL HasUnlitTranslucency();
	/*	virtual void Render(const FSceneContext& Context,FPrimitiveRenderInterface* PRI);	*/
	virtual void UpdateBounds();
	virtual void Precache();
	virtual UBOOL IsValidComponent() const;

	virtual FPrimitiveSceneProxy* CreateSceneProxy();

	virtual void Tick( FLOAT DeltaTime );

	void UpdateDynamicData();	
	/* End of render interface */
};