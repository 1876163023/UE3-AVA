class avaMod_Mark_NRF extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	Slot = CHAR_SLOT_MARK
				
	EURifleExtraMeshes(0) = (MeshName="AVA_CH_Mark.MS_EU_Mark01")
	EUPointExtraMeshes(0) = (MeshName="AVA_CH_Mark.MS_EU_Mark01")
	EUSniperExtraMeshes(0) = (MeshName="AVA_CH_Mark.MS_EU_Mark01")
	NRFRifleExtraMeshes(0) = (MeshName="AVA_CH_Mark.MS_NRF_Mark01")
	NRFPointExtraMeshes(0) = (MeshName="AVA_CH_Mark.MS_NRF_Mark01")
	NRFSniperExtraMeshes(0) = (MeshName="AVA_CH_Mark.MS_NRF_Mark01")
}

