class avaSeqAct_GetChannelFlag extends SequenceAction;

event Activated()
{
	local int nChannelFlag;
	nChannelFlag = class'avaNetHandler'.static.GetAvaNetHandler().GetCurrentChannelFlag();
	GenerateImpulse( nChannelFlag );
}

function GenerateImpulse( int nChannel )
{
	if ( OutputLinks.length <= nChannel || nChannel < 0 )
	{
		`log( "avaSeqAct_GetChannelFlag.GenerateImpulse Invalid ChannelFlag" @nChannel );
	}
	if( bAutoActivateOutputLinks )	bAutoActivateOutputLinks = false;
	OutputLinks[ nChannel ].bHasImpulse = true;
}


defaultproperties
{
	bCallHandler	=	false
	ObjCategory		=	"avaNet"
	ObjName			=	"Get Channel Flag"

	OutputLinks(0)	=(LinkDesc="EChannelFlag_Normal")
	OutputLinks(1)	=(LinkDesc="EChannelFlag_Trainee")
	OutputLinks(2)	=(LinkDesc="EChannelFlag_Match")
	OutputLinks(3)	=(LinkDesc="EChannelFlag_Reserve2")
	OutputLinks(4)	=(LinkDesc="EChannelFlag_Reserve3")
	OutputLinks(5)	=(LinkDesc="EChannelFlag_Newbie")
	OutputLinks(6)	=(LinkDesc="EChannelFlag_Clan")
	OutputLinks(7)	=(LinkDesc="EChannelFlag_PCBang")
	OutputLinks(8)	=(LinkDesc="EChannelFlag_Event")
	OutputLinks(9)	=(LinkDesc="EChannelFlag_MyClan")
	OutputLinks(10)	=(LinkDesc="EChannelFlag_Practice")
	OutputLinks(11)	=(LinkDesc="EChannelFlag_Broadcast")
	OutputLinks(12)	=(LinkDesc="EChannelFlag_AutoBalance")
	OutputLinks(13)	=(LinkDesc="EChannelFlag_Temp")

	VariableLinks(0)=(MaxVars=1)
}
