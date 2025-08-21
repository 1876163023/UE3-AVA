class avaSeqAct_CheckChannelFlag extends SequenceAction;

var() array< EChannelFlag >		ChannelFlagToCheck;

event Activated()
{
	local int nChannelFlag;
	local int i;
	local bool bMatch;
	
	nChannelFlag = class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag();

	bMatch = false;
	for ( i = 0 ; i < ChannelFlagToCheck.length ; ++ i )
	{
		if ( ChannelFlagToCheck[i] == nChannelFlag )
		{
			bMatch = true;
			break;
		}
	}

	GenerateImpulse( bMatch );
}

function GenerateImpulse( bool bMatch )
{
	if( bAutoActivateOutputLinks )	bAutoActivateOutputLinks = false;
	OutputLinks[ int( !bMatch ) ].bHasImpulse = true;
}

defaultproperties
{
	bCallHandler	=	false
	ObjCategory		=	"avaNet"
	ObjName			=	"Check Channel Flag"

	OutputLinks(0)	=(LinkDesc="yes")
	OutputLinks(1)	=(LinkDesc="no")

	VariableLinks(0)=(MaxVars=1)


	ChannelFlagToCheck(0) = EChannelFlag_Normal
}
