class avaMod_HelmetA extends avaCharacterModifier;

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	if ( Pawn.TypeID == 0 )			// PointMan
		{	
			avaCharacter( Pawn ).EnableTakeOffHelmet = true;
		}
	else if ( Pawn.TypeID == 1 )	// RifleMan
		{
			avaCharacter( Pawn ).EnableTakeOffHelmet = true;
		}
	else if ( Pawn.TypeID == 2 )	// Sniper
		{	
			avaCharacter( Pawn ).EnableTakeOffHelmet = false;
		}
	Pawn.Absorb_Head			+= 0.2;
	Pawn.Helmet_DamageThreshold += 3;		// 맞았을때 Helmet 이 벗겨지는...
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
	if ( Pawn.TypeID == 0 )			// PointMan
		{	
			avaCharacter( Pawn ).EnableTakeOffHelmet = true;
		}
	else if ( Pawn.TypeID == 1 )	// RifleMan
		{
			avaCharacter( Pawn ).EnableTakeOffHelmet = true;
		}
	else if ( Pawn.TypeID == 2 )	// Sniper
		{	
			avaCharacter( Pawn ).EnableTakeOffHelmet = false;
		}
	Pawn.Armor_Head				= 9999;
}

defaultproperties
{
	id = 5377
	Slot = CHAR_SLOT_H1
	EURifleHELMETMesh	= (MeshName="AVA_EU_Helmet.MS_Rifleman_Helmet01")
	EUPointHELMETMesh	= (MeshName="AVA_EU_Helmet.MS_Pointman_Helmet01")
	EUSniperExtraMeshes(0) = (MeshName="AVA_EU_Helmet.MS_Sniper_Helmet01")
	
						
	NRFRifleHELMETMesh	= (MeshName="AVA_NRF_Helmet.MS_Rifleman_Helmet01")
	NRFPointHELMETMesh	= (MeshName="AVA_NRF_Helmet.MS_Pointman_Helmet01")
	NRFSniperExtraMeshes(0)	= (MeshName="AVA_NRF_Helmet.MS_Sniper_Helmet01")

//	EUPointHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
//	EURifleHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
//	EUSniperHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
//	NRFPointHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
//	NRFRifleHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
//	NRFSniperHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
}