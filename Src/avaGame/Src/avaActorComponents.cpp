#include "avaGame.h"
#include "avaActorComponents.h"
#include "LevelUtils.h"
#if WITH_NOVODEX
#include "../../engine/src/UnNovodexSupport.h"
#endif // WITH_NOVODEX


IMPLEMENT_CLASS(UavaFlashlightComponent);
IMPLEMENT_CLASS(UavaCCDStaticMeshComponent);
IMPLEMENT_CLASS(UavaMaterialMeshComponent);

void UavaFlashlightComponent::SetTransformedToWorld()
{
	LightToWorld = FMatrix(
		FPlane(+0,+0,+1,+0),
		FPlane(+0,+1,+0,+0),
		FPlane(+1,+0,+0,+0),
		FPlane(+0,+0,+0,+1)
		) *
		FRotationTranslationMatrix(Rotation,Translation) * 
		CachedParentToWorld;
	LightToWorld.RemoveScaling();
	WorldToLight = LightToWorld.Inverse();
}

void UavaFlashlightComponent::SetAttachmentInfo( float y, int pitch )
{
	Translation.Z = y;
	Rotation.Pitch = pitch;
	BeginDeferredReattach();
}

void UavaSkeletalMeshComponent::UpdateBounds()
{
	Super::UpdateBounds();
	if ( bNoForceUpdateBound )	return;
	
	FBox BoundingBox;
	BoundingBox.Init();

	BoundingBox += Bounds.Origin + Bounds.BoxExtent + FVector( 100, 100, 100 );
	BoundingBox += Bounds.Origin - Bounds.BoxExtent - FVector( 100, 100, 100 );	

	Bounds = FBoxSphereBounds(BoundingBox);	
}

void UavaSkeletalMeshComponent::Tick(FLOAT DeltaTime)
{
	Super::Tick( DeltaTime );
	//// Parent �� Child ���ٴ� LOD �ܰ谡 ���ƾ� �Ѵ�.
	//for(INT AttachmentIndex = 0;AttachmentIndex < Attachments.Num();AttachmentIndex++)
	//{
	//	USkeletalMeshComponent* childSkeletalComponent = Cast<USkeletalMeshComponent>(Attachments(AttachmentIndex).Component);
	//	if ( childSkeletalComponent && 
	//		 childSkeletalComponent->ParentAnimComponent == this && 
	//		 childSkeletalComponent->SkeletalMesh != NULL && 
	//		 PreviousMinLODLevel > childSkeletalComponent->PredictedLODLevel )
	//		PreviousMinLODLevel = childSkeletalComponent->PredictedLODLevel;
	//}
}

// Collision Group �� �ٲ��ش�.
void UavaSkeletalMeshComponent::SetPhysicsAssetCollisionGroup(INT GroupIdx)
{
	if ( PhysicsAssetInstance == NULL )	return;
	PhysicsAssetCollisionGroupIdx = GroupIdx;
	for ( int i = 0 ; i <  PhysicsAssetInstance->Bodies.Num() ; ++ i )
	{
		URB_BodyInstance* BodyInst = PhysicsAssetInstance->Bodies(i);
		if ( BodyInst == NULL ) continue;
		NxActor* pNxActor = BodyInst->GetNxActor();
		if ( pNxActor == NULL ) continue;

		int				nbShapes	= pNxActor->getNbShapes();    
		NxShape *const* shapeArray	= pNxActor->getShapes();

		for( int j=0;j < nbShapes ;j++ )
		{
			NxShape *shape=shapeArray[j];
			shape->setGroup( GroupIdx );
		}
	}
}

	;
void UavaSkeletalMeshComponent::InitComponentRBPhys(UBOOL bFixed)
{
	Super::InitComponentRBPhys( bFixed );
	// CollisionGroup ��default �� �����Ǿ� �ֱ� ������ Modify �� �ʿ䰡 ����.
	if ( PhysicsAssetCollisionGroupIdx == 0 )	return;
	SetPhysicsAssetCollisionGroup( PhysicsAssetCollisionGroupIdx );
}


// Continuous Collision Detection �� �̿��ϱ� ���� Component
void UavaCCDStaticMeshComponent::InitComponentRBPhys(UBOOL bFixed)
{
	Super::InitComponentRBPhys( bFixed );

	NxActor* pNxActor = BodyInstance->GetNxActor();
	if ( pNxActor )
	{
		int				nbShapes	= pNxActor->getNbShapes();    
		NxShape *const* shapeArray	= pNxActor->getShapes();

		NxCCDSkeleton*			ccds;
		NxSimpleTriangleMesh	stm;
		NxVec3					points[8];
		NxU32 triangles[3 * 12] = { 0,1,3, 0,3,2, 3,7,6, 3,6,2,
									1,5,7, 1,7,3, 4,6,7, 4,7,5,
									1,0,4, 5,1,4, 4,0,2, 4,2,6 };

		if ( nbShapes > 0 )
		{
			stm.numVertices			= 8;
			stm.numTriangles		= 6*2;
			stm.pointStrideBytes	= sizeof(NxVec3);
			stm.triangleStrideBytes = sizeof(NxU32)*3;

			for (NxU32 i = 0; i < 8; i++)
				points[i].arrayMultiply(points[i], NxVec3(0.2f, 0.2f, 0.2f));

			stm.points = points;
			stm.triangles = triangles;
			stm.flags |= NX_MF_FLIPNORMALS;
		}

		for( int i=0;i < nbShapes ;i++ )
		{
			NxShape *shape=shapeArray[i];
			ccds = GNovodexSDK->createCCDSkeleton(stm);
			shape->setCCDSkeleton(ccds);
		}

		pNxActor->setCCDMotionThreshold( 16 * U2PScale );
	}

}

/*! @brief
		������ �Լ����� MaterialExtIndex�� ���� �⺻ ������ �������� �����ؼ� ����ش�.
		(2007/01/03 ����)
*/
UMaterialInstance* UavaMaterialMeshComponent::GetMaterial(INT MaterialIndex, INT LOD) const
{
	// If we have a base materials array, use that
	if(MaterialIndex < Materials.Num() && Materials(MaterialIndex))
	{
		// MaterialExtIndex�� ���� �ٸ� ������ ����ǵ��� ���ش�.
		INT ExtIndex = StaticMesh->LODModels(0).Elements.Num() * MaterialExtIndex + MaterialIndex;

		if( ExtIndex < Materials.Num() && Materials(ExtIndex) )
			return Materials(ExtIndex);

		return Materials(MaterialIndex);
	}
	// Otherwise get from static mesh lod
	else if(StaticMesh && MaterialIndex < StaticMesh->LODModels(LOD).Elements.Num() && StaticMesh->LODModels(LOD).Elements(MaterialIndex).Material)
	{
		return StaticMesh->LODModels(LOD).Elements(MaterialIndex).Material;
	}
	else
		return NULL;
}
