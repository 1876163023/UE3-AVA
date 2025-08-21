/** 
 * ���� �������� ��� LocalPlayers.PlayerInput �� �Է�Ű�� �����Ѵ�
 *
 * ������ �Է��� �޴� UIScene�� ���ٰ� �������� ������ �Է��� ��� �����ϰ� UIScene�� �Է��� �ޱ� ���� ����Ѵ�
 * ( UGameUISceneClient::UpdateInputProcessingStatus ����)
 * 
 * @TODO : ������ Player�� ���� FlushPlayerInput�� �ʿ��� �� �ִ� (��Ȱ�ϸ� ���� LocalPlayer)
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