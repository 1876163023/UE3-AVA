struct FShatterGlassRenderData
{
	struct FInner
	{
		FInner()
			: Vertices(NULL), Indices(NULL), NumVertices(0), NumIndices(0)
		{}

		~FInner()
		{
			appFree( Vertices );
			appFree( Indices );			
		}

		DWORD GetAllocatedSize() const
		{
			return sizeof(WORD) * NumIndices + sizeof(FShatterGlassVertex) * NumVertices;
		}

		void CopyData( FShatterGlassRenderContext* Context, INT Index )
		{
			TArray<FShatterGlassVertex>& SrcVertices = Context->Vertices[Index];
			TArray<WORD>& SrcIndices = Context->Indices[Index];

			NumVertices = SrcVertices.Num();
			NumIndices = SrcIndices.Num();

			if ( NumIndices > 0)
			{
				Vertices = (FShatterGlassVertex*)appRealloc( Vertices, sizeof(FShatterGlassVertex) * NumVertices );
				Indices = (WORD*)appRealloc( Indices, sizeof(WORD) * NumIndices );

				appMemcpy( Vertices, &SrcVertices(0), sizeof(FShatterGlassVertex) * NumVertices );
				appMemcpy( Indices, &SrcIndices(0), sizeof(WORD) * NumIndices );

				UMaterialInstance* MaterialInstance;

				UavaShatterGlassComponent* Component = Context->Component;

				UBOOL bSelected = Component->IsOwnerSelected();

				if (Index == 0)
				{
					MaterialInstance = Component->bIsBroken ? Component->BrokenMaterial : Component->Material;
				}
				else
				{
					MaterialInstance = Cast<UMaterialInstance>( Component->Style[ Index - 1 ] );
				}				

				if (MaterialInstance)
				{
					MaterialResource = MaterialInstance->GetInstanceInterface( bSelected );
				}
				else
				{
					MaterialResource = NULL;
				}
			}			
		}

		FShatterGlassVertex*						Vertices;	
		WORD*										Indices;	
		INT											NumVertices;
		INT											NumIndices;
		const FMaterialInstance*					MaterialResource;
	};

	FShatterGlassRenderData( FShatterGlassRenderContext* Context )
	{
		for (INT i=0; i<13; ++i)
		{
			Inner[i].CopyData( Context, i );
		}
	}

	~FShatterGlassRenderData()
	{
	}

	FInner										Inner[13];	

	DWORD GetMemoryFootprint( void ) const 
	{ 
		DWORD Size = sizeof( *this );
		
		for (INT i=0; i<13; ++i)
			Size += Inner[i].GetAllocatedSize();

		return Size;
	}
};

struct FShatterGlassSceneProxy : FPrimitiveSceneProxy
{
	UavaShatterGlassComponent*					Component;	
	FShatterGlassRenderData*					DynamicData;
	BITFIELD									bHasTranslucency : 1;
	BITFIELD									bHasDistortion : 1;

	virtual void DrawDynamicElements( FPrimitiveDrawInterface* PDI, const FSceneView* View, UINT DepthPriorityGroup );

	FShatterGlassSceneProxy( UavaShatterGlassComponent* Component );
	~FShatterGlassSceneProxy();

	void UpdateData(FShatterGlassRenderData* NewDynamicData);

	void UpdateData_RenderThread(FShatterGlassRenderData* NewDynamicData);	

	//	void Render(const FSceneContext& Context,FPrimitiveRenderInterface* PRI, const FColor* LevelColor);

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocSkMSP ); }
	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() + (DynamicData ? DynamicData->GetMemoryFootprint() : 0 ) ); }
};