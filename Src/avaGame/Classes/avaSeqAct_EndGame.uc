// 게임을 종료하고자할 때 사용한다.
// 게임 클래스가 무엇이든, GameInfo 를 상속 받은 것은 모두 다 쓸 수 있다.

class avaSeqAct_EndGame extends SequenceAction;

/// 종료 원인을 의미하는 문자열.
/// TimeLimit 나 Massacre 같은 것을 세팅해주면 되겠다.
var() string	EndDesc;
var() int		WinType;

event Activated()
{
    local avaGame     game;
	///@todo reset 하기 위해 대기하는 동안 focus를 누구에게 맞출 것인지.
	/// avaPlayerController 에서 이 값을 SetViewTarget() 함수로 세팅하게 되는데
	/// 꼭 controller로 해야하는지, 아니면 pawn 으로 해야하는지 혹은 EndGame 처럼
	/// PRI 로 해야하는지 명확하지 않다. 일단 actor라고 가정한다.
	local Actor         InstigatorActor;

	game = avaGame( GetWorldInfo().Game );

	// WarmUp 중에라도 EndGame 은 불려야 된다!!!

	// Warmup Round 라면 End Round 불가능
	//if ( avaGameReplicationInfo( game.GameReplicationInfo ).bWarmupRound == true )
	//	return;
 
    if ( game == none )
    {
        `log( "game is not avaGame",, 'error' );
    }
    else
    {
        if ( VariableLinks.Length >= 1 && VariableLinks[0].LinkedVariables.Length > 0 )
        {
            InstigatorActor = Actor( SeqVar_Object(VariableLinks[0].LinkedVariables[0] ).GetObjectValue());
        }
		game.EndGameEx( avaPawn(InstigatorActor).PlayerReplicationInfo, EndDesc, WinType );
    }
}


defaultproperties
{
	ObjCategory = "Game"
	ObjName     = "End Game"
	bCallHandler = false

	EndDesc = "kismet"

	VariableLinks(0)=(LinkDesc="InstigatorPRI",MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_String',LinkDesc="Desc",  PropertyName=EndDesc, MinVars=0, MaxVars=1 )
	VariableLinks(2)=(ExpectedType=class'SeqVar_Int',LinkDesc="WinType",  PropertyName=WinType, MinVars=0, MaxVars=1 )
}

