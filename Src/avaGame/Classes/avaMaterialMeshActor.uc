/*
	������ StaticMeshActor�� Material�� ���������� ���� �����ϰ� �߰��� Actor

	2007/01/03	����
		StaticMeshActor�� ���� �ٸ� ����� ������ StaticMeshComponent��
		avaMaterialMeshComponent�� ������ �κи� �ٸ���.
*/

class avaMaterialMeshActor extends Actor
	native
	placeable;

var() const editconst avaMaterialMeshComponent	MaterialMeshComponent;

defaultproperties
{
	// �� �κп��� StaticMeshComponent�� ���� �����ؼ� ��� ����?

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
