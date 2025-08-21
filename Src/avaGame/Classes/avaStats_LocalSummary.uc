/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */

class avaStats_LocalSummary extends avaStatsSummary;

const	TABSPACE=8;

function GenerateSummary()
{
	if ( bKeepLocalStatsLog )
	{
		GenerateGameStats();
		//GenerateIndPlayerStat();
	}
}

/**
 * Output a header to the stats file
 */

function HeaderText(FileWriter Output, string Text, optional bool bLesser)
{
	local int i,l;

    Output.Logf("");
    if (bLesser)
    {
		Output.Logf("------------------------------------------------------");
	}
	else
	{
		Output.Logf("============================================================");
	}

	L = Len(Text);
	if ( L < 60 )
	{
		L = int( (60.0 - L) * 0.5);
		for (i=0;i<L;i++)
			Text = " "$Text;
	}
	Output.Logf(Text);
    if (bLesser)
    {
		Output.Logf("------------------------------------------------------");
	}
	else
	{
		Output.Logf("============================================================");
	}
    Output.Logf("");
}

/**
 * The function will take a string and backfill the end to fit a given number of tabs
 */

function string TFormatStr(coerce string s, int NoTabs)
{
	local int ln,x,dif,n;
	ln = len(s);
	x = NoTabs * TABSPACE;
	if (x>ln)
	{
		dif = x-ln;
		n = int( float(dif) / TABSPACE);
		if (n * tabspace < dif)
		{
			n++;
		}
		for (x=0;x<n;x++)
			s = s $ chr(9);
	}
	return s;

}
//
///**
// * Left Pads a number with 0's
// */
//
//function string IndexText(int Index)
//{
//	local int i;
//	local string s;
//
//	s = ""$Index;
//	for (i=0;i<6-len(s);i++)
//	{
//		S = "0"$s;
//	}
//
//	return s;
//}
//
///****************************************************************************
//  General Gameplay Stats
//****************************************************************************/
//
//function GenerateMiscGameStats(FileWriter Output)
//{
//	local Info.ServerResponseLine ServerData;
//	local int i;
//	local string ip;
//
//    HeaderText(Output,"Unreal Tournament 2007 General Gameplay Stats");
//
//	// Collect all of the information about the server.
//
//	WorldInfo.Game.GetServerInfo(ServerData);
//	WorldInfo.Game.GetServerDetails(ServerData);
//
//	ip = ServerData.IP;
//	if (ServerData.IP != "")
//	{
//		ip = " ["$ip$":"$ServerData.Port$"]";
//	}
//
//	Output.Logf("ServerName:"$CHR(9)$ServerData.ServerName@ip);
//	Output.Logf("ServerID  :"$CHR(9)$ServerData.ServerID);
//	Output.Logf("");
//    Output.Logf("Game: "@WorldInfo.Game.GameName@"match on"@GetMapFilename());
//    Output.Logf("");
//    Output.Logf(":: Settings ::");
//    Output.Logf("");
//
//	for (i=0;i<ServerData.ServerInfo.Length;i++)
//	{
//		Output.Logf( ""$CHR(9)$ ServerData.ServerInfo[i].Key$ CHR(9)$ CHR(9)$ ServerData.ServerInfo[i].Value);
//	}
//    Output.Logf("");
//}

function GeneratePlayerWeaponSummaries( FileWriter Output, int StatsID, int RoundID, int SpawnID )
{
	local int WeaponID;
	local WeaponStat	WeaponStats;
	local string temp;
	local int ValidWeaponCnt;

	for ( WeaponID = 0 ; WeaponID < PlayerStats[StatsID].RoundStats[RoundID].SpawnStats[SpawnID].WeaponStats.length ; ++ WeaponID )
	{
		WeaponStats = PlayerStats[StatsID].RoundStats[RoundID].SpawnStats[SpawnID].WeaponStats[WeaponID];

		// 사용하지 않은 무기는 `log 에 남기지 말자...
		if (   WeaponStats.FireStats.Kill		== 0 &&
			   WeaponStats.FireStats.TeamKill	== 0 &&
			   WeaponStats.FireStats.HeadShot	== 0 &&
			   WeaponStats.FireStats.Fire		== 0 &&
			   WeaponStats.FireStats.Hit		== 0 &&
			   WeaponStats.FireStats.Damage		== 0 &&
			   WeaponStats.FireStats.DoubleKill	== 0 &&
			   WeaponStats.FireStats.MultiKill	== 0 )	continue;

		++ ValidWeaponCnt;
	}

	if ( ValidWeaponCnt == 0 )	return;

	// Header
	temp = TFormatStr("",3);
	temp = temp$TFormatStr("Name",2);
	temp = temp$TFormatStr("K",1);
	temp = temp$TFormatStr("TK",1);
	temp = temp$TFormatStr("HS",1);
	temp = temp$TFormatStr("F",1);
	temp = temp$TFormatStr("H",1);
	temp = temp$TFormatStr("D",1);
	temp = temp$TFormatStr("DK",1);
	temp = temp$TFormatStr("MK",1);
	Output.Logf(Temp);

	temp = TFormatStr("",3);
	temp = temp$"=========================================================================================";
	Output.Logf(Temp);

	for ( WeaponID = 0 ; WeaponID < PlayerStats[StatsID].RoundStats[RoundID].SpawnStats[SpawnID].WeaponStats.length ; ++ WeaponID )
	{
		WeaponStats = PlayerStats[StatsID].RoundStats[RoundID].SpawnStats[SpawnID].WeaponStats[WeaponID];

		// 사용하지 않은 무기는 `log 에 남기지 말자...
		if (   WeaponStats.FireStats.Kill		== 0 &&
			   WeaponStats.FireStats.TeamKill	== 0 &&
			   WeaponStats.FireStats.HeadShot	== 0 &&
			   WeaponStats.FireStats.Fire		== 0 &&
			   WeaponStats.FireStats.Hit		== 0 &&
			   WeaponStats.FireStats.Damage		== 0 &&
			   WeaponStats.FireStats.DoubleKill	== 0 &&
			   WeaponStats.FireStats.MultiKill	== 0 )	continue;

		temp = TFormatStr("",3);
		temp = temp$TFormatStr(WeaponStats.WeaponType.default.ItemName,2);
		temp = temp$TFormatStr(WeaponStats.FireStats.Kill		,1);
		temp = temp$TFormatStr(WeaponStats.FireStats.TeamKill	,1);
		temp = temp$TFormatStr(WeaponStats.FireStats.HeadShot	,1);
		temp = temp$TFormatStr(WeaponStats.FireStats.Fire		,1);
		temp = temp$TFormatStr(WeaponStats.FireStats.Hit		,1);
		temp = temp$TFormatStr(WeaponStats.FireStats.Damage		,1);
		temp = temp$TFormatStr(WeaponStats.FireStats.DoubleKill	,1);
		temp = temp$TFormatStr(WeaponStats.FireStats.MultiKill	,1);
		Output.Logf(Temp);
	}
	Output.Logf("");
}

function GeneratePlayerSpawnSummaries(FileWriter Output, int StatsID, int RoundID )
{
	local int		SpawnID;
	local SpawnStat	SpawnStats;
	local string	temp;

	for ( SpawnID = 0 ; SpawnID < PlayerStats[StatsID].RoundStats[RoundID].SpawnStats.length ; ++SpawnID )
	{
		temp = TFormatStr("",2);
		temp = temp$TFormatStr("SpawnStart",3);
		temp = temp$TFormatStr("SpawnEnd",3);
		temp = temp$TFormatStr("SpawnTime",3);
		temp = temp$TFormatStr("Dmg",3);
		temp = temp$TFormatStr("DmgCnt",3);
		temp = temp$TFormatStr("Team",3);
		temp = temp$TFormatStr("SprintTime",3);
		Output.Logf(Temp);

		SpawnStats = PlayerStats[StatsID].RoundStats[RoundID].SpawnStats[SpawnID];
		temp = TFormatStr("",2);
		temp = temp$TFormatStr(TimeStr(SpawnStats.SpawnTime),3);
		temp = temp$TFormatStr(TimeStr(SpawnStats.SpawnEndTime),3);
		if ( SpawnStats.SpawnEndTime == 0 )		temp = temp$TFormatStr(TimeStr(GameRoundStats[RoundID].EndTime-SpawnStats.SpawnTime),3);
		else									temp = temp$TFormatStr(TimeStr(SpawnStats.SpawnEndTime-SpawnStats.SpawnTime),3);
		temp = temp$TFormatStr(SpawnStats.Damaged,3);
		temp = temp$TFormatStr(SpawnStats.DamagedCount,3);
		temp = temp$TFormatStr(SpawnStats.nTeam,3);
		temp = temp$TFormatStr(TimeStr(SpawnStats.SprintTime),3);

		Output.Logf(Temp);

		Output.Logf("");
		GeneratePlayerWeaponSummaries( Output, StatsID, RoundID, SpawnID );
	}
}

function GeneratePlayerRoundSummaries(FileWriter Output, int StatsID )
{
	local int		RoundID;
	local RoundStat	RoundStats;
	local string	temp;

	for ( RoundID = 0 ; RoundID < PlayerStats[StatsID].RoundStats.length ; ++RoundID )
	{
		RoundStats = PlayerStats[StatsID].RoundStats[RoundID];
		
		// Spawn 하지 않았으면 Play 를 하지 않은것이기 때문에 `log 에 남기지 않는다.
		if ( RoundStats.SpawnCount == 0 )		continue;


		temp = TFormatStr("",1);
		temp = temp$TFormatStr("Round",2);
		temp = temp$TFormatStr("SpawnCnt",2);
		temp = temp$TFormatStr("Killed",2);
		temp = temp$TFormatStr("HeadS",2);
		temp = temp$TFormatStr("Suicide",2);
		temp = temp$TFormatStr("Unluck",2);
		temp = temp$TFormatStr("Trainee",2);
		temp = temp$TFormatStr("R_Rank",2);
		temp = temp$TFormatStr("R_HRank",2);
		temp = temp$TFormatStr("R_Score",2);
		temp = temp$TFormatStr("XP",2);
		Output.Logf(Temp);

		temp = TFormatStr("",1);
		temp = temp$TFormatStr(RoundID,2);
		temp = temp$TFormatStr(RoundStats.SpawnCount,2);
		temp = temp$TFormatStr(RoundStats.DeathStats.Killed,2);
		temp = temp$TFormatStr(RoundStats.DeathStats.HeadShoted,2);
		temp = temp$TFormatStr(RoundStats.DeathStats.Suicide,2);
		temp = temp$TFormatStr(RoundStats.DeathStats.UnluckDeath,2);
		temp = temp$TFormatStr(RoundStats.DeathStats.Trainee,2);
		temp = temp$TFormatStr(RoundStats.KillRank,2);
		temp = temp$TFormatStr(RoundStats.HeadShotRank,2);
		temp = temp$TFormatStr(RoundStats.RoundScore,2);
		temp = temp$TFormatStr(RoundStats.RoundExp,2);
		Output.Logf(Temp);

		Output.Logf("");
		GeneratePlayerSpawnSummaries( Output, StatsID, RoundID );
	}
}

function string GetShotInfoName( int nShotInfo )
{
		 if ( nShotInfo == SI_Head )		return "Head";
	else if ( nShotInfo == SI_Stomach )		return "Stomach";
	else if ( nShotInfo == SI_Chest )		return "Chest";
	else if ( nShotInfo == SI_LeftArm )		return "LeftArm";
	else if ( nShotInfo == SI_RightArm )	return "RightArm";
	else if ( nShotInfo == SI_LeftLeg )		return "LeftLeg";
	else if ( nShotInfo == SI_RightLeg )	return "RightLeg";
	else									return "Generic";
}

function GeneratePlayerHitSummaries(FileWriter Output, int StatsID )
{
	local int		HitID;
	local HitStat	HitStats;
	local string	temp;

	for ( HitID = 0 ; HitID < PlayerStats[StatsID].HitStats.length ; ++HitID )
	{
		HitStats = PlayerStats[StatsID].HitStats[HitID];
		temp = TFormatStr(GetDisplayName(StatsID),2);
		temp = temp$TFormatStr(HitStats.Weapon.default.ItemName,2);
		temp = temp$TFormatStr(GetShotInfoName(HitStats.ShotInfo),2);
		temp = temp$TFormatStr(GetDisplayName(HitStats.Victim),2);
		temp = temp$TFormatStr(HitStats.Distance,2);
		temp = temp$TFormatStr(HitStats.Damage,2);
		if ( HitStats.bKill )	temp = temp$TFormatStr("O",2);
		Output.Logf(Temp);
	}
	Output.Logf("");
}

function string GetTeamName( int nTeam )
{
	return nTeam == 0 ? "EU" : "NRF";
}

function GenerateGameSummaries( FileWriter Output )
{
	local int		RoundID;
	local int		PlayerID;
	local string	temp;
	local SpawnStat PlayerLastSpawnStat;
	local RoundStat PlayerRoundStat;
	HeaderText(Output,"Game Summary");

	for ( RoundID = 0 ; RoundID < GameRoundStats.length ; RoundID ++ )
	{
		if ( GameRoundStats[RoundID].Winner == -1 )	continue;

		temp = TFormatStr("",1);
		temp = temp$TFormatStr("Round"@RoundID,2);
		temp = temp$TFormatStr("StartTime"@TimeStr(GameRoundStats[RoundID].StartTime),2);
		temp = temp$TFormatStr("EndTime"@TimeStr(GameRoundStats[RoundID].EndTime),2);
		temp = temp$TFormatStr("Winner"@GetTeamName( GameRoundStats[RoundID].Winner ),2);
		temp = temp$TFormatStr("Desc:"@GameRoundStats[RoundID].EndRoundDesc,2);
		Output.Logf(Temp);

		// 이긴팀...
		temp = TFormatStr("",1);
		temp = temp$TFormatStr( GetTeamName( GameRoundStats[RoundID].Winner ), 2 );
		temp = temp$TFormatStr("<Player>",3);
		temp = temp$TFormatStr("<Kill>",3);
		temp = temp$TFormatStr("<Score>",3);
		temp = temp$TFormatStr("<Death>",3);
		temp = temp$TFormatStr("<HeadShot>",3);
		temp = temp$TFormatStr("<StartTime>",3);
		temp = temp$TFormatStr("<P/R/S>",3);

		Output.Logf(Temp);

		for ( PlayerID = 0 ; PlayerID < PlayerStats.length ; PlayerID++ )
		{
			PlayerRoundStat = PlayerStats[PlayerID].RoundStats[RoundID];
			if ( PlayerRoundStat.SpawnStats.length <= 0 )	continue;
			PlayerLastSpawnStat = PlayerRoundStat.SpawnStats[PlayerRoundStat.SpawnStats.length-1];
			if ( PlayerLastSpawnStat.nTeam == GameRoundStats[RoundID].Winner )
			{
				temp = TFormatStr( "", 3 );
				temp = temp$TFormatStr(GetDisplayName(PlayerID),3);
				temp = temp$TFormatStr(PlayerRoundStat.RoundFireStats.Kill,3);
				temp = temp$TFormatStr(int(PlayerRoundStat.RoundScore),3);
				temp = temp$TFormatStr(PlayerRoundStat.DeathStats.Killed,3);
				temp = temp$TFormatStr(PlayerRoundStat.RoundFireStats.HeadShot,3);
				temp = temp$TFormatStr(TimeStr(PlayerRoundStat.SpawnStats[0].SpawnTime),3);
				temp = temp$TFormatStr(PlayerRoundStat.RolePointManCnt$"/"$PlayerRoundStat.RoleRifleManCnt$"/"$PlayerRoundStat.RoleSniperCnt,3);
				Output.Logf(temp);
			}
		}

		// 진팀...
		temp = TFormatStr("",1);
		temp = temp$TFormatStr( GetTeamName( GameRoundStats[RoundID].Winner == 0 ? 1 : 0 ), 2 );
		temp = temp$TFormatStr("<Player>",3);
		temp = temp$TFormatStr("<Kill>",3);
		temp = temp$TFormatStr("<Score>",3);
		temp = temp$TFormatStr("<Death>",3);
		temp = temp$TFormatStr("<HeadShot>",3);
		temp = temp$TFormatStr("<StartTime>",3);
		temp = temp$TFormatStr("<P/R/S>",3);
		Output.Logf(Temp);

		for ( PlayerID = 0 ; PlayerID < PlayerStats.length ; PlayerID++ )
		{
			PlayerRoundStat = PlayerStats[PlayerID].RoundStats[RoundID];
			if ( PlayerRoundStat.SpawnStats.length <= 0 )	continue;
			PlayerLastSpawnStat = PlayerRoundStat.SpawnStats[PlayerRoundStat.SpawnStats.length-1];
			if ( PlayerLastSpawnStat.nTeam != GameRoundStats[RoundID].Winner )
			{
				temp = TFormatStr( "", 3 );
				temp = temp$TFormatStr(GetDisplayName(PlayerID),3);
				temp = temp$TFormatStr(PlayerRoundStat.RoundFireStats.Kill,3);
				temp = temp$TFormatStr(int(PlayerRoundStat.RoundScore),3);
				temp = temp$TFormatStr(PlayerRoundStat.DeathStats.Killed,3);
				temp = temp$TFormatStr(PlayerRoundStat.RoundFireStats.HeadShot,3);
				temp = temp$TFormatStr(TimeStr(PlayerRoundStat.SpawnStats[0].SpawnTime),3);
				temp = temp$TFormatStr(PlayerRoundStat.RolePointManCnt$"/"$PlayerRoundStat.RoleRifleManCnt$"/"$PlayerRoundStat.RoleSniperCnt,3);
				Output.Logf(temp);
			}
		}

		Output.Logf("");
	}
}

function GeneratePlayerSummaries(FileWriter Output)
{
	local int	StatID;
	local string temp;

	HeaderText(Output,"Player Summary");
	for ( StatID = 0 ; StatID < PlayerStats.length ; StatID++ )
	{
		// 기본 Data... 
		Output.Logf("#"$chr(9)$"Total Time"$chr(9)$"Name");
		Temp = TFormatStr(StatID,1);
		Temp = Temp$TFormatStr(TimeStr(GetPlayerConnectTime(StatID)),2);
		Temp = Temp$GetDisplayName(StatID);
		Output.Logf(Temp);
		Output.Logf("");
		GeneratePlayerRoundSummaries( Output, StatID );
		Output.Logf("");
	}

	HeaderText(Output,"Hit Summary");
	for ( StatID = 0 ; StatID < PlayerStats.length ; StatID++ )
	{
		GeneratePlayerHitSummaries( Output, StatID );
	}

	GenerateGameSummaries( Output );

	//for (StatID=0;StatID<PlayerStats.Length;StatID++)
	//{
	//	Temp = ""$StatID$chr(9);
	//	Temp = Temp$TFormatStr(TimeStr(GetPlayerConnectTime(StatID)),2);
	//	Temp = Temp$TFormatStr(GetPlayerScore(StatID),2);
	//	Temp = Temp$TFormatStr(PlayerStats[StatID].KillStats.Length,1);
	//	Temp = Temp$TFormatStr(PlayerStats[StatID].DeathStats.Length - PlayerStats[StatID].NoSuicides,1);
	//	Temp = Temp$GetDisplayName(StatID);

	//	Output.Logf(Temp);
	//}

	//Output.Logf("");
	//Output.Logf(":: Kill Matrix ::");
	//Output.Logf("");

	//Temp = "";

	//// Generate Header

	//for (i=0;i<PlayerStats.Length;i++)
	//{
	//	Temp=Temp$chr(9)$i;
	//}

	//Output.Logf(Temp);
	//Output.Logf("");

	//KillTally.Length = PlayerStats.Length;

	//for (i=0;i<PlayerStats.Length;i++)
	//{
	//	for (j=0;j<PlayerStats[i].KillStats.Length;j++)
	//	{
	//		if ( GameStats[PlayerStats[i].KillStats[j]].InstigatorID == i )
	//		{
	//			KillTally[j] = PlayerStats[i].NoSuicides;
	//		}
	//		else if ( GameStats[PlayerStats[i].KillStats[j]].InstigatorID >= 0 )
	//		{
	//			KillTally[ GameStats[PlayerStats[i].KillStats[j]].InstigatorID ]++;
	//		}
	//	}

	//	Temp = ""$i;
	//	for (j=0;j<KillTally.Length;j++)
	//	{
	//		Temp = Temp$chr(9)$KillTally[j];
	//		KillTally[j] = 0;
	//	}

	//	Output.Logf(Temp);
	//}

    Output.Logf("");
}

//
//
//function GenerateEventSummaries(FileWriter Output, int StatID)
//{
//	local int EventIndex;
//	local string Temp, DName1, DName2;
//	local avaGame GI;
//
//	HeaderText(Output,"Event Summary");
//
//	GI = avaGame(WorldInfo.Game);
//	if (GI!=none)
//	{
//		for (EventIndex=0;EventIndex<GameStats.Length;EventIndex++)
//		{
//
//			if (StatID<0 || GameStats[EventIndex].InstigatorID == StatID)
//			{
//
//				DName1 = GameStats[EventIndex].InstigatorID>=0?PlayerStats[GameStats[EventIndex].InstigatorID].DisplayName:"";
//				DName2 = GameStats[EventIndex].AdditionalID>=0?PlayerStats[GameStats[EventIndex].AdditionalID].DisplayName:"";
//
//				Temp = GI.DecodeEvent(GameStats[EventIndex].GameStatType, GameStats[EventIndex].Team, DName1, DName2, GameStats[EventIndex].AdditionalData);
//
//				if (Temp!="")
//				{
//					Output.Logf(IndexText(EventIndex)$CHR(9)$TimeStr(GameStats[EventIndex].TimeStamp)$CHR(9)$CHR(9)$Temp);
//				}
//			}
//		}
//	}
//	else
//	{
//		Output.Logf("Stats not available on gametypes who are not children of avaGAME");
//	}
//
//	Output.Logf("");
//}
//
//
function GenerateGameStats()
{
	Local FileWriter Output;

	Output = Spawn(class'FileWriter');
	if (Output!=none)
	{
		if ( Output.OpenFile( GetMapFilename(), FWFT_Stats,,,true) )
		{
			//GenerateMiscGameStats(Output);
			GeneratePlayerSummaries(Output);
			//GenerateEventSummaries(Output,-1);

			`log(" GenerateGameStats to:"@Output.Filename);
		}
		else
		{
			StatLog("Could Not Create local Game Summary!",""$self);
		}
	}
}

///****************************************************************************
//  Individual Player Stats
//****************************************************************************/
//
//function GeneratePlayerStats(FileWriter Output, int StatID)
//{
//	local string temp;
//	local int i,j,shots,hits;
//
//	HeaderText(Output,"Player Summary");
//
//	Output.Logf("Player Name:"$CHR(9)$GetDisplayName(StatID));
//	Output.Logf("Global ID  :"$CHR(9)$PlayerStats[StatID].GlobalID);
//	Output.Logf("");
//
//	Temp = "Connect Time:"@CHR(9)$TimeStr(GetPlayerConnectTime(StatId));
//	if (PlayerStats[StatID].NoConnects>1)
//	{
//		Temp = Temp @ "(# of Reconnects:"@(PlayerStats[StatID].NoConnects-1)$")";
//	}
//
//	Output.Logf(Temp);
//	Output.Logf("Final Score:"@CHR(9)$GetPlayerScore(StatID));
//	Output.Logf("# of Kills:"@CHR(9)$PlayerStats[StatID].KillStats.Length);
//	Output.Logf("# of Deaths:"@CHR(9)$PlayerStats[StatID].DeathStats.Length);
//	Output.Logf("# of Suicides:"@CHR(9)$PlayerStats[StatID].NoSuicides);
//
//	shots = 0;
//	hits = 0;
//	for (i=0;i<PlayerStats[StatID].WeaponStats.Length;i++)
//	{
//		for (j=0;j<PlayerStats[StatID].WeaponStats[i].FireStats.Length;j++)
//		{
//			shots += PlayerStats[StatID].WeaponStats[i].FireStats[j].ShotsFired;
//			hits  += PlayerStats[StatID].WeaponStats[i].FireStats[j].ShotsDamaged + PlayerStats[StatID].WeaponStats[i].FireStats[j].ShotsDirectHit;
//		}
//	}
//
//	Output.Logf("# of Shots:"@CHR(9)$Shots);
//	Output.logf("# of Hits:"@CHR(9)$Hits);
//	Output.Logf("");
//	Output.Logf("General Accuracy:"@int(float(hits)/float(Shots)*100.0)$"%");
//	Output.Logf("");
//
//	Output.Logf(":: Bonuses ::");
//
//	for (i=0;i<PlayerStats[StatID].BonusStats.Length;i++)
//	{
//		Output.Logf("x"$PlayerStats[StatID].BonusStats[i].NoReceived@PlayerStats[StatID].BonusStats[i].BonusType);
//	}
//
//
//	HeaderText(Output,"Weapon Stats");
//
//	for (i=0;i<PlayerStats[StatID].WeaponStats.Length;i++)
//	{
//		GenerateSingleWeaponStats(Output,StatID, PlayerStats[StatID].WeaponStats[i]);
//	}
//
//}
//
//function string FireDesc(int FireMode)
//{
//	if (FireMode==0)
//		return "Primary Fire";
//	else if (FireMode==1)
//		return "Alternate Fire";
//	else
//		return "Unknown Fire";
//}
//
//function GenerateSingleWeaponStats(FileWriter Output, int StatID, WeaponStat WeapStat)
//{
//	local int i, cnt, Shots;
//	local float a,e;
//	local array<float> Accuracy;
//	local array<float> Efficiency;
//	local string temp;
//	local GameStat GS;
//	local class<avaDamageType> DT;
//
//   	HeaderText(Output,""$WeapStat.WeaponType,true);
//
//	if (WeapStat.FireStats.Length < 2)
//	{
//		WeapStat.FireStats.Length = 2;
//	}
//
//	Accuracy.Length = WeapStat.FireStats.Length;
//	Efficiency.Length = WeapStat.FireStats.Length;
//
//    // Force Fire and Alt Fire
//
//	Cnt = 0;
//	for (i=0;i<WeapStat.FireStats.Length;i++)
//	{
//		Shots = WeapStat.FireStats[i].ShotsFired;
//		if (Shots>0)
//		{
//			Accuracy[i]   = float(WeapStat.FireStats[i].ShotsDirectHit) / float(Shots) * 100.0;
//			Efficiency[i] = float( (WeapStat.FireStats[i].ShotsDirectHit + WeapStat.FireStats[i].ShotsDamaged)) / float(Shots) * 100.0;
//			a += Accuracy[i];
//			e += Efficiency[i];
//			cnt++;
//		}
//		else
//		{
//			Accuracy[i]   = 0;
//			Efficiency[i] = 0;
//		}
//	}
//
//
//	if (Cnt>0)
//	{
//		a = a / Cnt;
//		e = e / Cnt;
//	}
//
//	Temp = "Average Accuracy:"@a$"%"$CHR(9)$"Average Efficiency:"@e$"%";
//	Output.Logf(Temp);
//	Output.Logf("");
//
//	Output.Logf("FIREMODE"$Chr(9)$"SHOTS"$CHR(9)$"DIRECT"$CHR(9)$"SPLASH"$CHR(9)$"ACCURACY"$CHR(9)$"EFFICIENCY");
//	Output.Logf("");
//
//	for (i=0;i<Accuracy.Length;i++)
//	{
//		Temp = "";
//		Temp = Temp$TFormatStr(FireDesc(i),2);
//		Temp = Temp$TFormatStr(WeapStat.FireStats[i].ShotsFired,1);
//		Temp = Temp$TFormatStr(WeapStat.FireStats[i].ShotsDirectHit,1);
//		Temp = Temp$TFormatStr(WeapStat.FireStats[i].ShotsDamaged,1);
//		Temp = Temp$TFormatStr(Accuracy[i]$"%",2);
//		Temp = Temp$Efficiency[i]$"%";
//
//		Output.Logf(Temp);
//	}
//
//	if (WeapStat.Combos>0)
//	{
//		Output.logf("");
//		Output.logf("Combos x"$WeapStat.Combos);
//		Output.logf("");
//	}
//
//	Output.logf("");
//	Output.Logf(":: Kill List ::");
//	Output.logf("");
//
//
//	for (i=0;i<PlayerStats[StatId].KillStats.Length;i++)
//	{
//		GS = GameStats[ PlayerStats[StatId].KillStats[i] ];
//		if (GS.AdditionalData != none)
//		{
//			DT = class<avaDamageType>(GS.AdditionalData);
//			if (DT!=none && DT.default.DamageWeaponClass != none && DT.default.DamageWeaponClass==WeapStat.WeaponType)
//			{
//				Output.Logf(TimeStr(GS.TimeStamp)$Chr(9)$"Killed "@GetDisplayName(GS.InstigatorID));
//			}
//		}
//	}
//
//	Output.Logf("");
//
//}
//
//function GeneratePlayerInvStats(FileWriter Output, int StatID)
//{
//	local int i;
//	local string temp;
//
//	HeaderText(Output,"Inventory Stats");
//
//	Output.Logf("ITEM TYPE"$chr(9)$chr(9)$"PICKUPS"$CHR(9)$"DROPS"$CHR(9)$"GIVES");
//	Output.Logf("");
//
//	for (i=0;i<PlayerStats[StatID].InventoryStats.Length;i++)
//	{
//		Temp = TFormatStr(PlayerStats[StatID].InventoryStats[i].InventoryType,3);
//		Temp = Temp $ TFormatStr(PlayerStats[StatID].InventoryStats[i].NoPickups,1);
//		Temp = Temp $ TFormatStr(PlayerStats[StatID].InventoryStats[i].NoDrops,1);
//		Temp = Temp $ TFormatStr(PlayerStats[StatID].InventoryStats[i].NoIntentionalDrops,1);
//		Output.Logf(Temp);
//	}
//
//	Output.Logf("");
//}
//
//function GenerateIndPlayerStat()
//{
//	Local FileWriter Output;
//	Local string FName;
//	local int StatID;
//
//	for (StatID=0;StatID<PlayerStats.Length;StatID++)
//	{
//		FName = GetDisplayName(StatID);
//		Output = Spawn(class'FileWriter');
//		if (Output!=none)
//		{
//			if ( Output.OpenFile( FName, FWFT_Stats,,,true) )
//			{
//				GeneratePlayerStats(Output,StatID);
//				GeneratePlayerInvStats(Output,StatID);
//				GenerateEventSummaries(Output,StatID);
//			}
//			else
//			{
//				StatLog("Could Not Create local Player Stats!",""$self);
//			}
//		}
//	}
//}
//
//
//defaultproperties
//{
//}
