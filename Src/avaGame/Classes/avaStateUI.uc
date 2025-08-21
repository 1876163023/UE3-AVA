/*****************************************************************************
*	
*	avaStateUI.uc
*	
*
*	2006/08/16, YTS
*
*
*	 UIInteraction�� �⺻���� ����±�ɸ� �����Ѵ�. 
*	�������� UIInteraction���� MenuState�� Multiline Message�� ���� 
*	���������� ����� ���� ����� �����ִ�.
*	
*	 MenuState�� ( ��. �д��� ��ê > ���� > ��������Ʈ1 ) �� ���� ����� �� �ִ�.
*	�޴����� ��Ҹ� ������ �Ѵܰ辿 ���� �ö� �� �־���ϱ⵵ �ϴ�. 
*
*	 Multiline Message����� HUD�� Console�޼����� ����� ����. (�� �ڸ��� �ٸ���)
*	Color�� LifeTime�� �����ؼ� AddMessage(Ȥ�� AppendMessage)�ϸ� �޼����� ���ӽð���
*	������ ���ؼ� �Է��� �� �ִ�.
*
*
*	
*	* �������
*	EmptyState( StateStack�� ����ִ� ���¿����� �Է��� OnInputKey()�� ���´�.
*
*
*	������ avaUIInteraction.uc���� State�� ����� ��ɵ��� �����Ǿ������� 
*	������ ���� �ʾ� ���� �ʾҴ�.
*
********************************************************************************/


class avaStateUI extends UIInteraction
	within avaGameViewportClient	
	native
	dependson (avaStateUIData)
	config(Game);

// avaHUD�� �Űܰ��� ( native struct �϶� ���� Ŭ�������� ������ �����ϹǷ� )
//struct StateUIMessage
//{
//	var string	Text;
//	var float	Life;
//	var Color	Color;
//};


var bool					bOpened;

/* State*/
var array<string>			StateStack;
var array<avaStateUIData>	StateData;

/* Message*/
var array<StateUIMessage>	Messages;
var int						RenderPosX, RenderPosY;
var bool					bLineDown;

/* Key */
var bool					bCtrlPressed;
var bool					bAltPressed;

var bool					bNoRepeat;


/* Mouse */
var	bool				bMouseLPressed;
var bool				bMouseRPressed;

var bool				bCaptureMouse;

var native pointer pMessage{TArray<struct FStateUIMessage>};


function bool OnInputKey( int ControllerID, Name Key, int Number ,EInputEvent Event)
{
	if(!bOpened)
		return false;

	return false;
}

event bool InputKey( int ControllerId, name Key, EInputEvent Event, float AmountDepressed = 1.f , bool bGamepad = FALSE)
{
	Local int Number;

	if(!bOpened)
		return false;

	if(Key == 'LeftControl' || Key == 'RightControl')
	{	
		if(Event == IE_Pressed)
			bCtrlPressed = true;
		else if(Event == IE_Released)
			bCtrlPressed = false;
	}
	else if(Key == 'LeftAlt' || Key == 'RightAlt')
	{
		if(Event == IE_Pressed)
			bCtrlPressed = true;
		else if(Event == IE_Released)
			bCtrlPressed = false;
	}
	else if( Key == 'LeftMouseButton')
	{
		if( Event == IE_Pressed )
			bMouseLPressed = true;
		else if ( Event == IE_Released)
			bMouseLPressed = false;
	}
	else if ( Key == 'RightMouseButton' )
	{
		if( Event == IE_Pressed )
			bMouseRPressed = true;
		else if ( Event == IE_Released )
			bMouseRPressed = false;
	}

	if( bNoRepeat && Event != IE_Pressed)
		return false;

	if(!CheckInputCondition())
		return false;

	Number = GetNumberKey(Key);
	return GetCurrentState().OnInputKey(ControllerID, Key, Number, Event);
}

function AppendMessage(string Msg, float LifeTime, Color TextColor)
{
	Local float fCurrentTime;
	Local StateUIMessage UIMsg;
	
	fCurrentTime = GetLocalPC().WorldInfo.TimeSeconds;
	UIMsg.Text = Msg;
	UIMsg.Life = LifeTime == 0 ? fCurrentTime + 24*60*60 : fCurrentTime + LifeTime;
	UIMsg.Color = TextColor;

	NativeAppendMessage(UIMsg);
	//	NativeAppendMessage(Msg, LifeTime == 0 ? fCurrentTime + 24*60*60 : fCurrentTime + LifeTime, TextColor);
	//GetMessages().Length = Messages.Length + 1;
	//GetMessages()[ Messages.Length - 1 ].Text = Msg;
	//GetMessages()[ Messages.Length - 1 ].Life = LifeTime == 0 ? fCurrentTime + 24*60*60 : fCurrentTime + LifeTime;
	//GetMessages()[ Messages.Length - 1 ].Color = TextColor;
}

function AddMessage(array<string> MsgString, float LifeTime, Color TextColor, bool bClear = true)
{
	Local int i;

	if(bClear)
		ClearMessage();

	for(i = 0 ; i < MsgString.Length ; i++)
		AppendMessage(MsgString[i], LifeTime, TextColor);
}

//function ClearMessage()
//{
//	Messages.Length = 0;
//}

//function RenderMenu( Canvas Canvas )
//{
//	local int Idx, XPos, YPos;
//    local float XL, YL;
//
//	if(!bOpened)
//		return;
//	
//    for (Idx = 0; Idx < Messages.Length; Idx++)
//    {
//		if ( Messages[Idx].Text == "" || Messages[Idx].Life < GetLocalPC().WorldInfo.TimeSeconds )
//			Messages.Remove(Idx--,1);
//    }
//
//	XPos = RenderPosX;
//	Ypos = RenderPosY;
////		XPos = (0.1 *  Canvas.SizeX);
////		YPos = (0.1 * Canvas.SizeY);
//
//    Canvas.Font = class'Engine'.static.GetSmallFont();
//    Canvas.TextSize ("A", XL, YL);
//
//	if(!bLineDown)
//		YPos -= YL * Messages.Length;
//
//    for (Idx = 0; Idx < Messages.Length; Idx++)
//    {
//		if (Messages[Idx].Text == "")
//			continue;
//	
//		Canvas.StrLen( Messages[Idx].Text, XL, YL );
//		Canvas.SetPos( XPos, YPos );
//		Canvas.DrawColor = Messages[Idx].Color;
//		Canvas.DrawText( Messages[Idx].Text, false );
//		YPos += YL;
//    }
//}

function RenderMenu( Canvas Canvas )
{
	if( !bOpened )
		return;

}

/* State functions ( Pop, Push, Clear, Peek State) */
function bool PushMenu(string StateName)
{
	Local int NewIndex, OldIndex;
	Local string PrevStateName;
	
	NewIndex = GetIndex(StateName);
	/* Couldn't find it */
	if( NewIndex < 0 )
	{
		`Warn("Fail to Push"@StateName);
		ClearMenu();
		return false;
	}

	OldIndex = GetIndex(PeekMenuName());
	if(OldIndex >= 0)
	{
		StateData[Oldindex].OnEndState(StateName);
		PrevStateName = PeekMenuName();
	}

	StateStack.Length = StateStack.Length + 1;
	StateStack[StateStack.Length - 1] = StateName;

	StateData[NewIndex].OnBeginState(PrevStateName);

	return true;
}

function PopMenu()
{
	Local int NewIndex, OldIndex;

	OldIndex = GetIndex(PeekMenuName());
	if(StateStack.Length > 0)
		StateStack.Length = StateStack.Length - 1;
	NewIndex = GetIndex(PeekMenuName());
	
	if(OldIndex >= 0)
		StateData[OldIndex].OnEndState( StateData[OldIndex].StateName );
	
	if(NewIndex >= 0)
		StateData[NewIndex].OnBeginState( StateData[NewIndex].StateName);
}

function ClearMenu()
{
	Local int OldIndex;
	
	OldIndex = GetIndex(PeekMenuName());
	if(OldIndex >= 0)
		StateData[OldIndex].OnEndState("");

	StateStack.Length = 0;
}

function string PeekMenuName()
{
	if(StateStack.Length > 0)
		return StateStack[StateStack.Length - 1];
	else
		return "";
}

function bool IsEmptyState()
{
	return PeekMenuName() == "";
}

function SetRenderPos(int PosX, int PosY)
{
	RenderPosX = PosX;
	RenderPosY = PosY;
}

function SetLineDown(bool bDown)
{
	bLineDown = bDown;
}

/* Getters Here */
function int GetIndex( string StateName)
{
	Local int i;

	for(i =  0 ; i < StateData.Length ; i++)
		if(StateData[i].StateName == StateName)
			return i;

	return -1;
}

function AvaStateUIData GetCurrentState()
{
	return GetStateData(PeekMenuName());
}

function avaStateUIData GetStateData( string StateName )
{
	Local int i;
	i = GetIndex(StateName);
	if( i >= 0)
		return StateData[i];
	else
		return None;
}

function LocalPlayer GetLocalPlayer()
{
	Local int i;
	
	for(i = 0 ; i < GamePlayers.Length ; i++)
	{
		if( GamePlayers[i] != None && GamePlayers[i].Actor != None && NetConnection(GamePlayers[i].Actor.Player) == None)
			return GamePlayers[i];
	}
	
	return None;
}

function PlayerController GetLocalPC()
{
	Local LocalPlayer P;
	P = GetLocalPlayer();
	return  P == None ? None : P.Actor;
}


function AddState(string StateName, optional delegate<avaStateUIData.OnInputKey> InputKey, optional delegate<avaStateUIData.OnBeginState> BeginState, optional delegate<avaStateUIData.OnEndState> EndState)
{
	if(GetIndex(StateName) >= 0)
		return;

	StateData.Length = StateData.Length + 1;
	StateData[StateData.Length - 1] = new(Self) class'avaStateUIData';
	StateData[StateData.Length - 1].StateName = StateName;
	StateData[StateData.Length - 1].OnBeginState = BeginState;
	StateData[StateData.Length - 1].OnEndState = EndState;
	StateData[StateData.Length - 1].OnInputKey = InputKey;
}

function Initialized()
{
	Super.Initialized();
	RedirectMessages(Messages);
	AddState("", OnInputKey);		//  DummyState for Empty
}

function bool CheckInputCondition()
{
	return true;
}

// QuickChat, Vote ���� ������ Message�� �ٸ������� ����� �� �ֵ��� Redirection.
// ���� avaQuickChatUI�� avaHUD.QuickChatMessages�� 
// avaVoteUI�� avaHUD.VoteMessages�� �����ؼ� ���� ����.

native function RedirectMessages( out array<StateUIMessage> NewMessageRef );
//native function NativeAppendMessage(string Text, float LifeTime, Color TextColor);
native function NativeAppendMessage(StateUIMessage NewMessage);
native function ClearMessage();

defaultproperties
{
	bOpened=true
	bNoRepeat=true

	RenderPosX=20
	RenderPosY=20

	bLineDown=true
}