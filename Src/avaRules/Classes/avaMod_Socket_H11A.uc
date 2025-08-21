class avaMod_Socket_H11A extends avaCharacterModifier;

// 나이트 비전
defaultproperties
{
	id = 7937
	SLOT=CHAR_SLOT_H1_1
//	EUPointHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
//	EURifleHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
//	NRFPointHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
//	NRFRifleHELMETAttachedItems[0]=(PrimarySocket=NV,		MeshName="WP_MVPVS_7b.MS_MVPVS_7b")
}

static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.bEnableNightvision = TRUE;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


