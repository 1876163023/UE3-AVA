/*
	기존의 기능에서 원하는 애니메이션으로 초기화 하는 루틴 추가.

	2007/06/26 고광록
*/
class avaAnimNodeRandom extends AnimNodeRandom
	native;

/*! @brief 애니메이션 해당 ChildIndex로 초기화 해준다.
	@param ChildIndex
		PendingChildIndex로 설정되어 진다.
	@param BlendTime
		기존의 BlendTime과 동일하게 사용된다.
*/
native function ResetActiveChild( INT ChildIndex, FLOAT BlendTime );

defaultproperties
{
}