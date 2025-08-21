/*
	UI Scene의 SceneInputMode의 값을 변경해 준다.

	2007/02/23	고광록
		ReadyRoom에서 WeaponInventory창이 위에 뜰 때 입력모드를 변경하기 위해서 추가.
*/
class avaUIAction_SetSceneInputMode extends UIAction;

// UI Scene의 입력모드를 
var() EScreenInputMode		SceneInputMode;

event Activated()
{
	local int i;
	local UIScene Scene;

	// 연결된 모든 대상에 대해서 입력모드를 변경해 준다.
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