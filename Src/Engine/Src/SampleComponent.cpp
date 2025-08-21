#include "EnginePrivate.h"

IMPLEMENT_CLASS(USampleComponent);

///** Represents a sample to the scene manager. */
//class FSampleSceneProxy : public FPrimitiveSceneProxy
//{
//private:
//	FPrimitiveViewRelevance ViewRelevance;
//
//public:
//	const USampleComponent* SampleComponent;
//
//	/** Every derived class should override these functions */
//	virtual EMemoryStats GetMemoryStatType( void ) const { return( STAT_GameToRendererMallocSkMSP ); }
//	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
//	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() ); }
//
//	/** Initialization constructor. */
//	FSampleSceneProxy(const USampleComponent* InComponent):
//	FPrimitiveSceneProxy(InComponent), SampleComponent( InComponent )
//	{
//		ViewRelevance.bDynamicRelevance = TRUE;
//		ViewRelevance.SetDPG(SDPG_Foreground, TRUE);
//	}
//
//	/**
//	*  Returns a struct that describes to the renderer when to draw this proxy.
//	*	@param		Scene view to use to determine our relevence.
//	*  @return		View relevance struct
//	*/
//	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
//	{
//		return ViewRelevance;
//	}
//
//	// FPrimitiveSceneProxy interface.
//	virtual void DrawDynamicElements(
//		FPrimitiveDrawInterface* PDI,
//		const FSceneView* View,
//		UINT InDepthPriorityGroup
//		)
//	{
//		if (View->Family->ShowFlags & SHOW_Samples )
//		{
//			FMatrix ViewToWorld = View->ViewMatrix.Inverse();
//
//			FVector WorldCameraZ = ViewToWorld.TransformNormal(FVector( 0, 0, 1));
//
//			const FColor WindingColor( 255, 0, 0 );
//			const FColor NormalColor( 0, 255, 0 );
//
//			for( INT iSample = 0; iSample < SampleComponent->Samples.Num(); ++iSample )
//			{
//				const AvaSample& Sample = SampleComponent->Samples(iSample);
//				FVector Normal = Sample.NormalEnd - Sample.Center;
//				FColor SampleColor = FColor(FLinearColor(Sample.Color.X, Sample.Color.Y, Sample.Color.Z));
//
//				// back face culling
//				if( (WorldCameraZ | Normal) > 0.0f )
//				{
//					continue;
//				}
//
//
//
//				for( INT p=0; p < Sample.NumPoints; ++p)
//				{
//					const FVector& start_point = Sample.Positions(p);
//					const FVector& end_point = Sample.Positions((p+1) %Sample.NumPoints);
//					PDI->DrawLine( start_point, end_point, SampleColor, SDPG_Foreground );
//				}
//
//				if( Sample.NumPoints < 3 )
//				{
//					PDI->DrawLine( Sample.Center, Sample.NormalEnd, FColor(255, 0, 0), SDPG_Foreground );
//				}
//				else
//				{
//					PDI->DrawLine( Sample.Center, Sample.NormalEnd, NormalColor, SDPG_Foreground );
//				}
//
//
//			}
//		}
//	}
//};

FPrimitiveSceneProxy* USampleComponent::CreateSceneProxy()
{
	return NULL;//new FSampleSceneProxy(this);
}