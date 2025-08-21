#include "avaGame.h"

IMPLEMENT_CLASS(AavaMaterialMeshActor);
IMPLEMENT_CLASS(UavaMaterialMeshActorFactory);

/*! @brief
		�ܼ��� ���� IMPLEMENT_CLASS�� ���� �����̸� �Ʒ� �Լ��� Ư���� ����� ����.
		(2007/01/03 ����)
*/

AActor* UavaMaterialMeshActorFactory::CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData )
{
	return Super::CreateActor( Location, Rotation, ActorFactoryData );
}
