/**
 *	AdminUI : ��ڸ� ���� Ű�Է� Interaction
 *
 *	avaNetHandler���� AmIAdmin()�� �ƴϸ� Ű�Է��� ���� �ʴ´�. (��ڿ��Ը� �����Ǵ� Interaction)
 *
 *	����� ConsoleŰ�� Tilde�� ���������
 */
class avaAdminUI extends Interaction
	within avaGameViewportClient
	config(Game);


event bool InputKey( int ControllerId, name Key, EInputEvent Event, float AmountDepressed = 1.f , bool bGamepad = FALSE)
{
	if(Key == 'Tilde' && Event == IE_Pressed)
	{
		if( ViewportConsole.IsInState('Open') || ViewportConsole.IsInState('Typing') )
		{
			ViewportConsole.GotoState('');
			return true;
		}
		else
		{
			ViewportConsole.GotoState('Open');
			return true;
		}
	}
	return false;
}