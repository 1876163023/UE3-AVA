/*=============================================================================
	RenderResource.h: Render resource definitions.
	Copyright 2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/**
 * A rendering resource which is owned by the rendering thread.
 */
class FRenderResource
{
public:

	/**
	 * @return The global initialized resource list.
	 */
	static TLinkedList<FRenderResource*>*& GetResourceList();

	/**
	 * Minimal initialization constructor.
	 */
	FRenderResource():
		bInitialized(FALSE)
	{}

	/**
	* Destructor used to catch unreleased resources.
	*/
	virtual ~FRenderResource();

	/**
	 * Initializes the dynamic RHI resource and/or RHI render target used by this resource.
	 * Called when the resource is initialized, or when reseting all RHI resources.
	 * This is only called by the rendering thread.
	 */
	virtual void InitDynamicRHI() {}

	/**
	 * Releases the dynamic RHI resource and/or RHI render target resources used by this resource.
	 * Called when the resource is released, or when reseting all RHI resources.
	 * This is only called by the rendering thread.
	 */
	virtual void ReleaseDynamicRHI() {}

	/**
	 * Initializes the RHI resources used by this resource.
	 * Called when the resource is initialized, or when reseting all RHI resources.
	 * This is only called by the rendering thread.
	 */
	virtual void InitRHI() {}

	/**
	 * Releases the RHI resources used by this resource.
	 * Called when the resource is released, or when reseting all RHI resources.
	 * This is only called by the rendering thread.
	 */
	virtual void ReleaseRHI() {}

	/**
	 * Initializes the resource.
	 * This is only called by the rendering thread.
	 */
	virtual void Init();

	/**
	 * Prepares the resource for deletion.
	 * This is only called by the rendering thread.
	 */
	virtual void Release();

	/**
	 * If the resource's RHI has been initialized, then release and reinitialize it.  Otherwise, do nothing.
	 * This is only called by the rendering thread.
	 */
	void UpdateRHI();

	/**
	 * @return The resource's friendly name.  Typically a UObject name.
	 */
	virtual FString GetFriendlyName() const { return TEXT("undefined"); }

	// Assignment operator.
	void operator=(const FRenderResource& OtherResource) {}

	// Accessors.
	UBOOL IsInitialized() const { return bInitialized; }

protected:

	/** This resource's link in the global resource list. */
	TLinkedList<FRenderResource*> ResourceLink;

	/** True if the resource has been initialized. */
	BITFIELD bInitialized : 1;
};

/**
 * Sends a message to the rendering thread to initialize a resource.
 * This is called in the game thread.
 */
extern void BeginInitResource(FRenderResource* Resource);

/**
 * Sends a message to the rendering thread to update a resource.
 * This is called in the game thread.
 */
extern void BeginUpdateResourceRHI(FRenderResource* Resource);

/**
 * Sends a message to the rendering thread to release a resource.
 * This is called in the game thread.
 */
extern void BeginReleaseResource(FRenderResource* Resource);

/**
 * Sends a message to the rendering thread to release a resource, and spins until the rendering thread has processed the message.
 * This is called in the game thread.
 */
extern void ReleaseResourceAndFlush(FRenderResource* Resource);

/**
 * A context object used to construct a message with new data to send to a resource.
 * This is used in the game thread.
 * Note: this doesn't work with GIsThreadedRendering yet.
 */
template<typename ResourceType>
class TSetResourceDataContext
{
public:
	TSetResourceDataContext(ResourceType* Resource)
	{
		if( GIsThreadedRendering && IsInGameThread() )
		{
			DataContext = new FRingBufferContext(Resource);
		}
		else
		{
			DataContext = new FInPlaceContext(Resource);
		}
		PendingValue = DataContext->GetPendingValue();
	}

	~TSetResourceDataContext()
	{
		delete DataContext;
		DataContext = NULL;
	}

	void Commit()
	{
		if(PendingValue && DataContext)
		{
			delete DataContext;
			DataContext = NULL;
			PendingValue = NULL;
		}
	}

	typename ResourceType::DataType* operator->() { return PendingValue; }

private:
	class FDataContext
	{
	public:
		virtual ~FDataContext() {}
		virtual typename ResourceType::DataType* GetPendingValue()=0;
	};

	class FRingBufferContext : public FDataContext
	{
	public:
		FRingBufferContext(ResourceType* InResource) 
		:	AllocationContext(GRenderCommandBuffer,sizeof(FSetDataCommand))
		{
			Command = new(AllocationContext) FSetDataCommand(InResource);
		}
		virtual ~FRingBufferContext()
		{
			AllocationContext.Commit();
		}
		virtual typename ResourceType::DataType* GetPendingValue()
		{
			return &Command->PendingValue;
		}
	private:
		class FSetDataCommand : public FRenderCommand
		{
		public:
			FSetDataCommand(ResourceType* InResource):
				Resource(InResource)
			{}
			virtual UINT Execute()
			{
				Resource->SetData(PendingValue);
				return sizeof(*this);
			}
			virtual TCHAR* DescribeCommand()
			{
				return TEXT("FSetDataCommand");
			}

			ResourceType* Resource;
			typename ResourceType::DataType PendingValue;
		};
		FSetDataCommand* Command;
		FRingBuffer::AllocationContext AllocationContext;
	};

	class FInPlaceContext : public FDataContext
	{
	public:
		FInPlaceContext(ResourceType* InResource)
		:	Resource(InResource)
		{
		}
		virtual ~FInPlaceContext()
		{
			Resource->SetData(PendingValue);
		}
		virtual typename ResourceType::DataType* GetPendingValue()
		{
			return &PendingValue;
		}
	private:
		ResourceType* Resource;
		typename ResourceType::DataType PendingValue;
	};
	
	typename ResourceType::DataType* PendingValue;
	FDataContext* DataContext; 
};

/**
 * A texture.
 */
class FTexture : public FRenderResource
{
public:
	/** The texture's RHI resource. */
	FTextureRHIRef		TextureRHI;

	/** The sampler state to use for the texture. */
	FSamplerStateRHIRef SamplerStateRHI;

	/** The last time the texture has been bound */
	mutable DOUBLE		LastRenderTime;

	/**
	 * Default constructor, initializing last render time to 0.
	 */
	FTexture()
	: LastRenderTime(0.0)
	{}

	// FRenderResource interface.
	virtual void ReleaseRHI()
	{
		TextureRHI.Release();
		SamplerStateRHI.Release();
	}
	/** Returns the width of the texture in pixels. */
	virtual UINT GetSizeX() const
	{
		return 0;
	}
	/** Returns the height of the texture in pixels. */
	virtual UINT GetSizeY() const
	{
		return 0;
	}
};

/**
 * A vertex buffer.
 */
class FVertexBuffer : public FRenderResource
{
public:
	FVertexBufferRHIRef VertexBufferRHI;

	// FRenderResource interface.
	virtual void ReleaseRHI()
	{
		VertexBufferRHI.Release();
	}
};

/**
 * An index buffer.
 */
class FIndexBuffer : public FRenderResource
{
public:
	FIndexBufferRHIRef IndexBufferRHI;

	// FRenderResource interface.
	virtual void ReleaseRHI()
	{
		IndexBufferRHI.Release();
	}
};
