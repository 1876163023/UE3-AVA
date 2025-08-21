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
*	와 같은 struct 안의 delegate를 쓸 수 없기 때문에 Class를 하나 새로 생성하였다.
*	위에서 예시한 Struct의 구조와 avaStateUIData.uc는 대동소이하다.
*
************************************************************************************/

class avaStateUIData extends Object;

var string StateName;

delegate OnBeginState(string PrevStateName);
delegate OnEndState(string NextStateName);
delegate bool OnInputKey(int ControllerID, name Key, int number ,EInputEvent Event);