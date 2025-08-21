/*
	재질 변환용 MeshComponent

	2007/01/03	고광록
		MaterialExtIndex값에 따라서 MeshComponent::Materials를 
		GetMaterial함수로 호출하여 사용할 때, 오프셋을 변경하여 적용한다.
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
	// 기본값은 0이며, 0일 경우에는 StaticMeshComponent와 동일하게 처리된다.
	MaterialExtIndex = 0
}
