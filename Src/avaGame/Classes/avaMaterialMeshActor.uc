/*
	기존의 StaticMeshActor에 Material을 선택적으로 적용 가능하게 추가한 Actor

	2007/01/03	고광록
		StaticMeshActor와 별반 다른 기능은 없으며 StaticMeshComponent를
		avaMaterialMeshComponent로 선언한 부분만 다르다.
*/

class avaMaterialMeshActor extends Actor
	native
	placeable;

var() const editconst avaMaterialMeshComponent	MaterialMeshComponent;

defaultproperties
{
	// 이 부분에서 StaticMeshComponent를 새로 정의해서 사용 가능?

	Begin Object Class=avaMaterialMeshComponent Name=MaterialMeshComponent0
		bAllowApproximateOcclusion=TRUE
		bCastDynamicShadow=FALSE
		bForceDirectLightMap=TRUE
	End Object
	CollisionComponent=MaterialMeshComponent0
	MaterialMeshComponent=MaterialMeshComponent0
	Components.Add(MaterialMeshComponent0)

	bEdShouldSnap=TRUE
	bStatic=TRUE
	bMovable=FALSE
	bCollideActors=TRUE
	bBlockActors=TRUE
	bWorldGeometry=TRUE
	bGameRelevant=TRUE
	bRouteBeginPlayEvenIfStatic=FALSE
	bCollideWhenPlacing=FALSE
}
