class avaMod_ArmorD extends avaCharacterModifier;

defaultproperties
{
	id = 5892
	Slot = CHAR_SLOT_BD
	NRFRifleExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.Heavy_ammor.MS_NRF_Heavy01")
	NRFPointExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.Heavy_ammor.MS_NRF_Heavy01")
	NRFSniperExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.Heavy_ammor.MS_NRF_Heavy01")

	EURifleExtraMeshes(0) = (MeshName="AVA_EU_Ammor.Heavy_ammor.MS_EU_Heavy01")
	EUPointExtraMeshes(0) = (MeshName="AVA_EU_Ammor.Heavy_ammor.MS_EU_Heavy01")
	EUSniperExtraMeshes(0) = (MeshName="AVA_EU_Ammor.Heavy_ammor.MS_EU_Heavy01")

}

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.Absorb_Stomach		+= 0.25;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
	Pawn.Armor_Stomach		+= 30;
	Pawn.ArmorMax			+= 30;
}

