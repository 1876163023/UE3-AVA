/*
	������ ��ɿ��� ���ϴ� �ִϸ��̼����� �ʱ�ȭ �ϴ� ��ƾ �߰�.

	2007/06/26 ����
*/
class avaAnimNodeRandom extends AnimNodeRandom
	native;

/*! @brief �ִϸ��̼� �ش� ChildIndex�� �ʱ�ȭ ���ش�.
	@param ChildIndex
		PendingChildIndex�� �����Ǿ� ����.
	@param BlendTime
		������ BlendTime�� �����ϰ� ���ȴ�.
*/
native function ResetActiveChild( INT ChildIndex, FLOAT BlendTime );

defaultproperties
{
}