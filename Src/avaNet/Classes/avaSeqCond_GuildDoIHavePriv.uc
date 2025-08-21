class avaSeqCond_GuildDoIHavePriv extends avaSeqCondition;


enum EGuildPriv
{
	EGP_Invite,
	EGP_Kick,
	EGP_SetMotd,
	EGP_SendNotice,
	EGP_CreateMatch,
	EGP_JoinMatch,
};


var() EGuildPriv Priv;



event Activated()
{
	if ( class'avaNetRequest'.static.GetAvaNetRequest().GuildDoIHavePriv(Priv) )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}


defaultproperties
{
	ObjCategory="avaNet"
	ObjName="(Guild) Do I Have Privilege"

	bAutoActivateOutputLinks=false

	InputLinks(0)=(LinkDesc="In")

	OutputLinks(0)=(LinkDesc="Yes")
	OutputLinks(1)=(LinkDesc="No");

	VariableLinks.Empty
	VariableLinks(0)=(ExpectedType=class'SeqVar_Int',LinkDesc="Privilege",PropertyName=Priv)
}

