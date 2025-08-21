/*********************************************************************************
*	
*	avaStateUIData.uc
*	
*
*	2006/08/16, YTS
*
*	struct StateUIData
*	{
*		var delegate<OnBeginState>
*		var delegate<OnEndState>
*		var delegate<OnInputKey>
*	}
*	
*	�� ���� struct ���� delegate�� �� �� ���� ������ Class�� �ϳ� ���� �����Ͽ���.
*	������ ������ Struct�� ������ avaStateUIData.uc�� �뵿�����ϴ�.
*
************************************************************************************/

class avaStateUIData extends Object;

var string StateName;

delegate OnBeginState(string PrevStateName);
delegate OnEndState(string NextStateName);
delegate bool OnInputKey(int ControllerID, name Key, int number ,EInputEvent Event);