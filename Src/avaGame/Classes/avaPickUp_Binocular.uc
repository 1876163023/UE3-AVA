class avaPickUp_Binocular extends avaPickup;

simulated event FellOutOfWorld(class<DamageType> dmgType)
{
	local avaPlayerReplicationInfo	avaPRI;
	local avaGame					avaGame;

	avaGame = avaGame( WorldInfo.Game );
	if ( avaGame.HasSquadLeader( TeamIdx ) == None )
	{
		avaPRI = avaGame.GetSquadLeader( TeamIdx );
		avaGame.SetSquadLeader( avaPRI );
	}
	Super.FellOutOfWorld( dmgType );
}

defaultproperties
{
	DrawScale	= 1.5
	LifeTime	= 0
	bDrawInRadar= true
	IconCode	= 58
}