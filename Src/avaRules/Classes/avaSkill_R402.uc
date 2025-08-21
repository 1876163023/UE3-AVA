class avaSkill_R402 extends avaCharacterModifier;

//51466		고급 조준사격술		Advanced AimedShot
static function ApplyToCharacter_Client( avaPawn Pawn )
{
	Pawn.WeapTypeAdd[WEAPON_RIFLE].ZoomSpreadAdd	-=0.005;
}

static function ApplyToCharacter_Server( avaPawn Pawn )
{
}


defaultproperties
{
	id = 51466
}
