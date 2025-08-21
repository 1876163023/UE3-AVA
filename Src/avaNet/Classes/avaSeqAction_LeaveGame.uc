class avaSeqAction_LeaveGame extends avaSeqAction;

event Activated()
{
	local PlayerController			PC;
	local WorldInfo					WorldInfo;

	if ( class'avaNetHandler'.static.GetAvaNetHandler().AmIHost() )
	{
		WorldInfo = GetWorldInfo();
		foreach WorldInfo.LocalPlayerControllers(PC)
		{
			avaGameReplicationInfo ( WorldInfo.Game.GameReplicationInfo ).PlayerOut( avaPlayerController( PC ) );
			break;
		}
	}

	class'avaNet.avaNetRequest'.static.GetAvaNetRequest().LeaveGame();
}

defaultproperties
{
	ObjName="(Game) Leave"

    VariableLinks.Empty
}

