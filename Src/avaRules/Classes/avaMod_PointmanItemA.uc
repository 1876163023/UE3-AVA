class avaMod_PointmanItemA extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	//if ( Pawn.TypeID == 0 )			// PointMan		avaCharacter( Pawn ).EnableTakeOffHelmet = true;
	//else if ( Pawn.TypeID == 1 )	// RifleMan		avaCharacter( Pawn ).EnableTakeOffHelmet = true;
	//else if ( Pawn.TypeID == 2 )	// Sniper		avaCharacter( Pawn ).EnableTakeOffHelmet = false;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
//	avaCharacter( Pawn ).EnableTakeOffHelmet = true;
}

defaultproperties
{
	id = 8449
	SLOT = CHAR_SLOT_U

	EUPointExtraMeshes(0) = (MeshName="AVA_EU_Body.MS_EU_Body01")
//	EURifleExtraMeshes(0) = (MeshName="AVA_EU_Body.MS_EU_Body01")
//	EUSniperExtraMeshes(0) = (MeshName="AVA_EU_Body.MS_EU_Body01")

	EUPointExtraMeshes(1) = (MeshName="AVA_EU_Item.MS_Pointman_Item01",MaxVisibleDistance = 400)
//	EURifleExtraMeshes(1) = (MeshName="AVA_EU_Item.MS_Pointman_Item01")
//	EUSniperExtraMeshes(1) = (MeshName="AVA_EU_Item.MS_Pointman_Item01")

	NRFPointExtraMeshes(0) = (MeshName="AVA_NRF_Body.MS_NRF_body01")
//	NRFRifleExtraMeshes(0) = (MeshName="AVA_NRF_Body.MS_NRF_body01")
//	NRFSniperExtraMeshes(0) = (MeshName="AVA_NRF_Body.MS_NRF_body01")

	NRFPointExtraMeshes(1) = (MeshName="AVA_NRF_Item.MS_Pointman_Item01",MaxVisibleDistance = 400)
//	NRFRifleExtraMeshes(1) = (MeshName="AVA_NRF_Item.MS_Pointman_Item01")
//	NRFSniperExtraMeshes(1) = (MeshName="AVA_NRF_Item.MS_Pointman_Item01")

}