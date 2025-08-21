/** 
 * 현재 동작중인 모든 LocalPlayers.PlayerInput 의 입력키를 해제한다
 *
 * 원래는 입력을 받는 UIScene이 없다가 생겼을때 기존의 입력을 모두 해제하고 UIScene의 입력을 받기 위해 사용한다
 * ( UGameUISceneClient::UpdateInputProcessingStatus 참고)
 * 
 * @TODO : 각각의 Player에 대한 FlushPlayerInput이 필요할 수 있다 (분활하면 다중 LocalPlayer)
 */
class avaUIAction_FlushPlayerInput extends UIAction;

event Activated()
{
	Local GameUISceneClient SceneClient;
	Local UIScene OwnerScene;

	OwnerScene = GetOwnerScene();

	SceneClient = OwnerScene != none ? OwnerScene.GetSceneClient() : none;
	if( SceneClient != none )
	{
		SceneClient.FlushPlayerInput();
	}
}

defaultproperties
{
	ObjName="FlushPlayerInput"
	ObjCategory="Input"

	VariableLinks.Empty
}