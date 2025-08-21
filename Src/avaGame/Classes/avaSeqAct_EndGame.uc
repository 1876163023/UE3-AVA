// ������ �����ϰ����� �� ����Ѵ�.
// ���� Ŭ������ �����̵�, GameInfo �� ��� ���� ���� ��� �� �� �� �ִ�.

class avaSeqAct_EndGame extends SequenceAction;

/// ���� ������ �ǹ��ϴ� ���ڿ�.
/// TimeLimit �� Massacre ���� ���� �������ָ� �ǰڴ�.
var() string	EndDesc;
var() int		WinType;

event Activated()
{
    local avaGame     game;
	///@todo reset �ϱ� ���� ����ϴ� ���� focus�� �������� ���� ������.
	/// avaPlayerController ���� �� ���� SetViewTarget() �Լ��� �����ϰ� �Ǵµ�
	/// �� controller�� �ؾ��ϴ���, �ƴϸ� pawn ���� �ؾ��ϴ��� Ȥ�� EndGame ó��
	/// PRI �� �ؾ��ϴ��� ��Ȯ���� �ʴ�. �ϴ� actor��� �����Ѵ�.
	local Actor         InstigatorActor;

	game = avaGame( GetWorldInfo().Game );

	// WarmUp �߿��� EndGame �� �ҷ��� �ȴ�!!!

	// Warmup Round ��� End Round �Ұ���
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

