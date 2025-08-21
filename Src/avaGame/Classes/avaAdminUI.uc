/**
 *	AdminUI : 운영자를 위한 키입력 Interaction
 *
 *	avaNetHandler에서 AmIAdmin()이 아니면 키입력을 받지 않는다. (운영자에게만 생성되는 Interaction)
 *
 *	현재는 Console키만 Tilde로 엮어놓았음
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