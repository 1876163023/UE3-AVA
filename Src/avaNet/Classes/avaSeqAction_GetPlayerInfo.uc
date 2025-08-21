class avaSeqAction_GetPlayerInfo extends avaSeqAction;

var() int	LastLevel;
var() int	LastClass;
var() int	LastTeam;
var() int	PlayerExp;
var() int	PlayerSupply;
var() int	PlayerCash;
var() int	PlayerMoney;
var() int	ClanMarkID;

event Activated()
{
	Local bool bResult;
	Local byte byteLevel, byteClass, byteTeam;

	bResult = class'avaNetHandler'.static.GetAvaNetHandler().GetPlayerInfo( byteLevel, byteClass, byteTeam, PlayerExp, PlayerSupply, PlayerCash, PlayerMoney, ClanMarkID );

	LastLevel = int(byteLevel);
	LastClass = int(byteClass);
	LastTeam = int(byteTeam);

	if( bResult )
		OutputLinks[0].bHasImpulse = true;
	else
		OutputLinks[1].bHasImpulse = true;
}


defaultproperties
{
	ObjName="GetPlayerInfo"

	bAutoActivateOutputLinks=false

	OutputLinks(0)=(LinkDesc="Accept")
	OutputLinks(1)=(LinkDesc="Failed")

    VariableLinks.Empty
	
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Level",PropertyName=LastLevel))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Class",PropertyName=LastClass))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Team",PropertyName=LastTeam))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Exp",PropertyName=PlayerExp))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Supply",PropertyName=PlayerSupply))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Cash",PropertyName=PlayerCash))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Money",PropertyName=PlayerMoney))
	VariableLinks.Add((ExpectedType=class'SeqVar_Int',LinkDesc="Clan Mark ID",PropertyName=ClanMarkID))

	ObjClassVersion=2
}