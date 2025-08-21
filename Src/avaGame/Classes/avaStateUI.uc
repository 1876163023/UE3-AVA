/*****************************************************************************
*	
*	avaStateUI.uc
*	
*
*	2006/08/16, YTS
*
*
*	 UIInteraction은 기본적인 입출력기능만 지원한다. 
*	불행히도 UIInteraction에는 MenuState나 Multiline Message와 같은 
*	공통적으로 사용할 만한 기능이 빠져있다.
*	
*	 MenuState는 ( 예. 분대장 퀵챗 > 공격 > 웨이포인트1 ) 와 같이 사용할 수 있다.
*	메뉴에서 취소를 누르면 한단계씩 위로 올라갈 수 있어야하기도 하다. 
*
*	 Multiline Message기능은 HUD의 Console메세지와 기능이 같다. (단 자리가 다르다)
*	Color와 LifeTime을 지정해서 AddMessage(혹은 AppendMessage)하면 메세지의 지속시간과
*	색깔을 정해서 입력할 수 있다.
*
*
*	
*	* 참고사항
*	EmptyState( StateStack이 비어있는 상태에서의 입력은 OnInputKey()로 들어온다.
*
*
*	실제로 avaUIInteraction.uc에도 State에 기반한 기능들이 구현되어있으나 
*	실정에 맞지 않아 쓰지 않았다.
*
********************************************************************************/


class avaStateUI extends UIInteraction
	within avaGameViewportClient	
	native
	dependson (avaStateUIData)
	config(Game);

// avaHUD로 옮겨갔음 ( native struct 일때 여러 클래스에서 접근이 가능하므로 )
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

// QuickChat, Vote 등의 각각의 Message가 다른곳에서 연결될 수 있도록 Redirection.
// 현재 avaQuickChatUI는 avaHUD.QuickChatMessages를 
// avaVoteUI는 avaHUD.VoteMessages를 연결해서 쓰고 있음.

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