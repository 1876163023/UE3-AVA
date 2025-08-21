class avaMod_BootsC extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.ChrSprintSpeedPct += 0.1;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 8963
	Slot = CHAR_SLOT_BT
	EURifleExtraMeshes(0) = (MeshName="CH_EU.EU_Foot.MS_EU_Foot01")
	EUPointExtraMeshes(0) = (MeshName="CH_EU.EU_Foot.MS_EU_Foot01")
	EUSniperExtraMeshes(0) = (MeshName="CH_EU.EU_Foot.MS_EU_Foot01")
						
	NRFRifleExtraMeshes(0) = (MeshName="CH_NRF.NRF_Foot.MS_NRF_Foot")
	NRFPointExtraMeshes(0) = (MeshName="CH_NRF.NRF_Foot.MS_NRF_Foot")
	NRFSniperExtraMeshes(0) = (MeshName="CH_NRF.NRF_Foot.MS_NRF_Foot")
}