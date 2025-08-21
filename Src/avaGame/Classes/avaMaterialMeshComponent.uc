/*
	���� ��ȯ�� MeshComponent

	2007/01/03	����
		MaterialExtIndex���� ���� MeshComponent::Materials�� 
		GetMaterial�Լ��� ȣ���Ͽ� ����� ��, �������� �����Ͽ� �����Ѵ�.
*/
class avaMaterialMeshComponent extends StaticMeshComponent
	native;

var(Rendering) int		MaterialExtIndex;

cpptext
{
	/**
	*	UStaticMeshComponent::GetMaterial
	* @param MaterialIndex Index of material
	* @param LOD Lod level to query from
	* @return Material instance for this component at index
	*/
	virtual UMaterialInstance* GetMaterial(INT MaterialIndex, INT LOD) const;
}

defaultproperties
{
	// �⺻���� 0�̸�, 0�� ��쿡�� StaticMeshComponent�� �����ϰ� ó���ȴ�.
	MaterialExtIndex = 0
}
