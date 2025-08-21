class avaMod_ArmorE extends avaCharacterModifier;

defaultproperties
{
	id = 5893
	Slot = CHAR_SLOT_BD
	NRFRifleExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.Light_ammor.MS_NRF_Light01")
	NRFPointExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.Light_ammor.MS_NRF_Light01")
	NRFSniperExtraMeshes(0) = (MeshName="AVA_NRF_Ammor.Light_ammor.MS_NRF_Light01")

	EURifleExtraMeshes(0) = (MeshName="AVA_EU_Ammor.Light_ammor.MS_EU_Light01")
	EUPointExtraMeshes(0) = (MeshName="AVA_EU_Ammor.Light_ammor.MS_EU_Light01")
	EUSniperExtraMeshes(0) = (MeshName="AVA_EU_Ammor.Light_ammor.MS_EU_Light01")

}

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.Absorb_Stomach		+= 0.80;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
	Pawn.Armor_Stomach		+= 15;
	Pawn.ArmorMax			+= 15;
}

