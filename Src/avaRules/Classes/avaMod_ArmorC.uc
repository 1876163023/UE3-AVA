class avaMod_ArmorC extends avaCharacterModifier;

defaultproperties
{
	id = 5891
	Slot = CHAR_SLOT_BD
	NRFRifleExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.MS_NRF_Ammor01")
	NRFPointExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.MS_NRF_Ammor01")
	NRFSniperExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.MS_NRF_Ammor01")

	EURifleExtraMeshes(0) = (MeshName="AVA_EU_Ammor.MS_EU_Ammor01")
	EUPointExtraMeshes(0) = (MeshName="AVA_EU_Ammor.MS_EU_Ammor01")
	EUSniperExtraMeshes(0) = (MeshName="AVA_EU_Ammor.MS_EU_Ammor01")

}

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.Absorb_Stomach		+= 0.25;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
	Pawn.Armor_Stomach		+= 15;
	Pawn.ArmorMax			+= 15;
}

