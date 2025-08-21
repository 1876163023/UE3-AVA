/*
	UI Scene�� SceneInputMode�� ���� ������ �ش�.

	2007/02/23	����
		ReadyRoom���� WeaponInventoryâ�� ���� �� �� �Է¸�带 �����ϱ� ���ؼ� �߰�.
*/
class avaUIAction_SetSceneInputMode extends UIAction;

// UI Scene�� �Է¸�带 
var() EScreenInputMode		SceneInputMode;

event Activated()
{
	local int i;
	local UIScene Scene;

	// ����� ��� ��� ���ؼ� �Է¸�带 ������ �ش�.
	for( i = 0 ; i < Targets.Length ; i++ )
	{
		Scene = UIScene(Targets[i]);
		if( Scene == None )
			continue;

		Scene.SetSceneInputMode( SceneInputMode );
	}
}

defaultproperties
{
	ObjName="ava Set SceneInputMode"

	SceneInputMode = INPUTMODE_ActiveOnly;
}