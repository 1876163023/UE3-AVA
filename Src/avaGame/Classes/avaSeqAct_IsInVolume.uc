/*
	Actor 가 Volume 안에 있는지 Check 한다.
*/
class avaSeqAct_IsInVolume extends SequenceAction;

var Actor	actor;
var Volume	volume;

event Activated()
{
	local bool	bMatched;
	local Actor	check_actor;
	local Volume V;
	bMatched = false;
	if ( actor != None && volume != None )
	{
		check_actor = actor;
		if ( Controller( actor ) != None )
			check_actor = Controller( actor ).Pawn;

		bMatched = check_actor.IsInVolume( volume );

		ForEach check_actor.TouchingActors(class'Volume',V)
		{
			`log( "avaSeqAct_IsInVolume" @check_actor @volume @bMatched @V );
		}
	}
	GenerateImpulse( bMatched );
}

function GenerateImpulse(bool bMatched)
{
	if( bAutoActivateOutputLinks )	bAutoActivateOutputLinks = false;
	OutputLinks[ int( !bMatched ) ].bHasImpulse = true;
}

defaultproperties
{
	bCallHandler=false
	ObjCategory="Actor"
	ObjName="IsInVolume"
	bAutoActivateOutputLinks=false
	
	OutputLinks(0)=(LinkDesc="yes")
	OutputLinks(1)=(LinkDesc="no")
	
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object', LinkDesc="actor",PropertyName=actor, MaxVars=1)
	VariableLinks(1)=(ExpectedType=class'SeqVar_Object', LinkDesc="volume",PropertyName=volume, MaxVars=1)
}