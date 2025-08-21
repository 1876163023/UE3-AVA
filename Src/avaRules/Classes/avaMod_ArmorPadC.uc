class avaMod_ArmorPadC extends avaCharacterModifier;

defaultproperties
{
	id = 5896
	SLOT=CHAR_SLOT_E
	NRFRifleExtraMeshes(0) = (MeshName="Ch_USSR_Item.Common.MS_NRF_Protector01")
	NRFPointExtraMeshes(0) = (MeshName="Ch_USSR_Item.Common.MS_NRF_Protector01")
	NRFSniperExtraMeshes(0) = (MeshName="Ch_USSR_Item.Common.MS_NRF_Protector01")

	EURifleExtraMeshes(0) = (MeshName="Ch_EU_Item.Common.MS_EU_Protector01")
	EUPointExtraMeshes(0) = (MeshName="Ch_EU_Item.Common.MS_EU_Protector01")
	EUSniperExtraMeshes(0) = (MeshName="Ch_EU_Item.Common.MS_EU_Protector01")
}

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.Absorb_Stomach		+= 0.15;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
	Pawn.Armor_Stomach		+= 0;
	Pawn.ArmorMax			+= 0;
}

