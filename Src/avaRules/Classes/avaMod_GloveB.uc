class avaMod_GloveB extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAdd[WEAPON_RIFLE].ZoomSpreadAdd -=0.005;
	Pawn.WeapTypeAdd[WEAPON_SNIPER].ZoomSpreadAdd -=0.005;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}

defaultproperties
{
	id = 8453
	Slot = CHAR_SLOT_G
	EURifleExtraMeshes(0) = (MeshName="CH_NRF.NRF_Hand.MS_NRF_Hand")
	EUPointExtraMeshes(0) = (MeshName="CH_NRF.NRF_Hand.MS_NRF_Hand")
	EUSniperExtraMeshes(0) = (MeshName="CH_NRF.NRF_Hand.MS_NRF_Hand")
//손은 공통으로 사용한다.						

	NRFRifleExtraMeshes(0) = (MeshName="CH_NRF.NRF_Hand.MS_NRF_Hand")
	NRFPointExtraMeshes(0) = (MeshName="CH_NRF.NRF_Hand.MS_NRF_Hand")
	NRFSniperExtraMeshes(0) = (MeshName="CH_NRF.NRF_Hand.MS_NRF_Hand")
}