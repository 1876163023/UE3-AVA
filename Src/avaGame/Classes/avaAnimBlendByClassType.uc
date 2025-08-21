/*
	������ �ִϸ��̼��� ������ �ִ� Ŭ����.

	2007/01/24	����

	@original avaAnimBlendByWeaponType.uc����.
*/
class avaAnimBlendByClassType extends avaAnimBlendBase
	native;

enum EBlendClassType
{
	BCT_PointMan,
	BCT_RifleMan,
	BCT_Sniper,
	BCT_Max
};

cpptext
{
	virtual	void PlayAnim( UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */ );
	virtual	void TickAnim( float DeltaSeconds, float TotalWeight  );
/*
	virtual FLOAT GetSliderPosition(INT SliderIndex, INT ValueIndex);
	virtual void HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue);
	virtual FString GetSliderDrawValue(INT SliderIndex);
*/
}

defaultproperties
{
	Children(0)=(Name="PointMan",Weight=1.0)
	Children(1)=(Name="RifleMan")
	Children(2)=(Name="Sniper")

	bFixNumChildren=true
}
