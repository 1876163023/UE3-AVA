#include "avaGame.h"

IMPLEMENT_CLASS(AavaMaterialMeshActor);
IMPLEMENT_CLASS(UavaMaterialMeshActorFactory);

/*! @brief
		단순히 위의 IMPLEMENT_CLASS를 위한 파일이며 아래 함수에 특별한 기능은 없다.
		(2007/01/03 고광록)
*/

AActor* UavaMaterialMeshActorFactory::CreateActor( const FVector* const Location, const FRotator* const Rotation, const class USeqAct_ActorFactory* const ActorFactoryData )
{
	return Super::CreateActor( Location, Rotation, ActorFactoryData );
}
