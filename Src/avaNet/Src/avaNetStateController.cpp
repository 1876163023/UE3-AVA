/***

Copyright (c) 2006 Redduck Inc. All rights reserved.

Project: avaNet

Name: avaNetStateController.cpp

Description: Implementation of avaNetStateController

***/
#include "avaNet.h"
#include "avaNetStateController.h"

#ifndef ASSERT
#define ASSERT check
#endif

#include "ComDef/MsgDef.h"
#include "ComDef/Inventory.h"
#include "ComDef/Version.h"
#include "ComDef/avaAES.h"
#include "ComDef/compress.h"

#include "avaStaticData.h"
#include "avaMsgSend.h"
#include "avaCommunicator.h"

// {{ 20070214 dEAthcURe|HM
#ifdef EnableHostMigration
#include "hostMigration.h"
#endif
// }} 20070214 dEAthcURe|HM

#include "avaWebInClient.h"

#include "avaConnection.h"

#include "UnUIMarkupResolver.h"

using namespace Def;



//////////////////////////////////////////////////////////////////////////////////////////
// Locals

enum EavaChatCommandType
{
	ECmd_None,

	ECmd_Buddy,
	ECmd_BuddyList,
	ECmd_BuddyAdd,
	ECmd_BuddyAddYes,
	ECmd_BuddyAddNo,
	ECmd_BuddyDelete,
	ECmd_BuddyInfo,

	ECmd_Block,
	ECmd_BlockList,
	ECmd_BlockAdd,
	ECmd_BlockDelete,

	ECmd_Guild,
	ECmd_GuildList,
	ECmd_GuildInvite,
	ECmd_GuildInviteYes,
	ECmd_GuildInviteNo,
	ECmd_GuildNotice,
	ECmd_GuildMotd,
	ECmd_GuildLeave,
	ECmd_GuildKick,

	ECmd_GuildChat,

	ECmd_InviteGame,
	ECmd_InviteGameYes,
	ECmd_InviteGameNo,
	ECmd_Follow,

	ECmd_Whisper,
	ECmd_Reply,
	ECmd_Help,
	ECmd_SwapTeam,
	ECmd_RoomKick,

	ECmd_GMWhisper,

	ECmd_Quit,

	ECmd_ClaimHost,
	ECmd_MatchNotice,
	ECmd_MatchChatOff,
};

#define _CASE_CMDTYPE(v, s) if (v == TEXT(#s)) return s

struct FavaChatCommandInfo
{
	EavaChatCommandType CmdType;
	EavaChatCommandType ParentCmd;
	TArray<FString> CmdChar;

	FavaChatCommandInfo() : CmdType(ECmd_None) {}
	UBOOL HasCmd(const FString &InCmdChar) const { return CmdChar.ContainsItem(InCmdChar); }

	static EavaChatCommandType GetCmdType(const FString &Cmd)
	{
		_CASE_CMDTYPE(Cmd, ECmd_Buddy);
		_CASE_CMDTYPE(Cmd, ECmd_BuddyList);
		_CASE_CMDTYPE(Cmd, ECmd_BuddyAdd);
		_CASE_CMDTYPE(Cmd, ECmd_BuddyAddYes);
		_CASE_CMDTYPE(Cmd, ECmd_BuddyAddNo);
		_CASE_CMDTYPE(Cmd, ECmd_BuddyDelete);
		_CASE_CMDTYPE(Cmd, ECmd_BuddyInfo);

		_CASE_CMDTYPE(Cmd, ECmd_Block);
		_CASE_CMDTYPE(Cmd, ECmd_BlockList);
		_CASE_CMDTYPE(Cmd, ECmd_BlockAdd);
		_CASE_CMDTYPE(Cmd, ECmd_BlockDelete);

		_CASE_CMDTYPE(Cmd, ECmd_Guild);
		_CASE_CMDTYPE(Cmd, ECmd_GuildList);
		_CASE_CMDTYPE(Cmd, ECmd_GuildInvite);
		_CASE_CMDTYPE(Cmd, ECmd_GuildInviteYes);
		_CASE_CMDTYPE(Cmd, ECmd_GuildInviteNo);
		_CASE_CMDTYPE(Cmd, ECmd_GuildNotice);
		_CASE_CMDTYPE(Cmd, ECmd_GuildMotd);
		_CASE_CMDTYPE(Cmd, ECmd_GuildLeave);
		_CASE_CMDTYPE(Cmd, ECmd_GuildKick);

		_CASE_CMDTYPE(Cmd, ECmd_GuildChat);

		_CASE_CMDTYPE(Cmd, ECmd_InviteGame);
		_CASE_CMDTYPE(Cmd, ECmd_InviteGameYes);
		_CASE_CMDTYPE(Cmd, ECmd_InviteGameNo);
		_CASE_CMDTYPE(Cmd, ECmd_Follow);

		_CASE_CMDTYPE(Cmd, ECmd_Whisper);
		_CASE_CMDTYPE(Cmd, ECmd_Reply);
		_CASE_CMDTYPE(Cmd, ECmd_Help);
		_CASE_CMDTYPE(Cmd, ECmd_SwapTeam);
		_CASE_CMDTYPE(Cmd, ECmd_RoomKick);

		_CASE_CMDTYPE(Cmd, ECmd_GMWhisper);

		_CASE_CMDTYPE(Cmd, ECmd_Quit);

		_CASE_CMDTYPE(Cmd, ECmd_ClaimHost);
		_CASE_CMDTYPE(Cmd, ECmd_MatchNotice);
		_CASE_CMDTYPE(Cmd, ECmd_MatchChatOff);

		return ECmd_None;
	}
};

#undef _CASE_CMDTYPE

struct FavaNetStateControllerLocal
{
	TArray<FString> UnreachablePlayerList;
	TArray<FavaChatCommandInfo> ChatCommandList;

	INT GetCmdTypeFromChar(const FString &CmdChar, INT ParentCmd = ECmd_None)
	{
		if (CmdChar.Len() > 0)
		{
			for (INT i = 0; i < ChatCommandList.Num(); ++i)
			{
				FavaChatCommandInfo &Info = ChatCommandList(i);

				if (Info.ParentCmd == ParentCmd && Info.HasCmd(CmdChar))
				{
					return Info.CmdType;
				}
			}
		}
		return ECmd_None;
	}
};


FavaNetStateControllerLocal _Locals;


//////////////////////////////////////////////////////////////////////////////////////////
// FPlayerInfo

void FPlayerInfo::SetItemInfo(Def::ITEM_INFO *WeaponInven, WORD WeaponCount,
							  Def::ITEM_INFO *EquipInven, WORD EquipCount,
							  Def::CUSTOM_ITEM_INFO *CustomInven, WORD CustomCount,
							  Def::EFFECT_ITEM_INFO *EffectInven, WORD EffectCount,
							  Def::TSN_ITEM *WeaponSet, Def::TSN_ITEM *EquipSet, Def::TSN_ITEM *EffectSet)
{
	WeaponCount = Clamp<WORD>(WeaponCount, 0, MAX_INVENTORY_SIZE);
	if (WeaponCount > 0)
	{
		appMemzero(PlayerInfo.itemInfo.weaponInven, MAX_INVENTORY_SIZE * sizeof(ITEM_INFO));
		appMemcpy(PlayerInfo.itemInfo.weaponInven, WeaponInven, WeaponCount * sizeof(ITEM_INFO));
	}
	EquipCount = Clamp<WORD>(EquipCount, 0, MAX_INVENTORY_SIZE);
	if (EquipCount > 0)
	{
		appMemzero(PlayerInfo.itemInfo.equipInven, MAX_INVENTORY_SIZE * sizeof(ITEM_INFO));
		appMemcpy(PlayerInfo.itemInfo.equipInven, EquipInven, EquipCount * sizeof(ITEM_INFO));
	}
	CustomCount = Clamp<WORD>(CustomCount, 0, MAX_CUSTOM_INVENTORY_SIZE);
	if (CustomCount > 0)
	{
		appMemzero(PlayerInfo.itemInfo.customWeapon, MAX_CUSTOM_INVENTORY_SIZE * sizeof(CUSTOM_ITEM_INFO));
		appMemcpy(PlayerInfo.itemInfo.customWeapon, CustomInven, CustomCount * sizeof(CUSTOM_ITEM_INFO));
	}
	EffectCount = Clamp<WORD>(EffectCount, 0, MAX_EFFECT_INVENTORY_SIZE);
	if (EffectCount > 0)
	{
		appMemzero(PlayerInfo.itemInfo.effectInven, MAX_EFFECT_INVENTORY_SIZE * sizeof(EFFECT_ITEM_INFO));
		appMemcpy(PlayerInfo.itemInfo.effectInven, EffectInven, EffectCount * sizeof(EFFECT_ITEM_INFO));
	}

	appMemzero(PlayerInfo.itemInfo.weaponSet, MAX_WEAPONSET_SIZE * sizeof(TSN_ITEM));
	appMemcpy(PlayerInfo.itemInfo.weaponSet, WeaponSet, MAX_WEAPONSET_SIZE * sizeof(TSN_ITEM));
	appMemzero(PlayerInfo.itemInfo.equipSet, MAX_EQUIPSET_SIZE * sizeof(TSN_ITEM));
	appMemcpy(PlayerInfo.itemInfo.equipSet, EquipSet, MAX_EQUIPSET_SIZE * sizeof(TSN_ITEM));
	appMemzero(PlayerInfo.itemInfo.effectSet, MAX_EFFECTSET_SIZE * sizeof(TSN_ITEM));
	appMemcpy(PlayerInfo.itemInfo.effectSet, EffectSet, MAX_EFFECTSET_SIZE * sizeof(TSN_ITEM));

	for (INT i = 0; i < Def::MAX_INVENTORY_SIZE; ++i)
	{
		ITEM_INFO &item = PlayerInfo.itemInfo.weaponInven[i];
		if (item.id == Def::ID_INVALID_ITEM)
			continue;

		ITEM_DESC *Desc = _ItemDesc().GetItem(item.id);
		if (Desc)
		{
			if ((Desc->gaugeType == _IGT_MAINTENANCE || Desc->gaugeType == _IGT_DURABILITY))
			{
				item.limit = Clamp<LONG>(item.limit, 0, ITEM_LIMIT_INITED);
			}
		}
	}
	for (INT i = 0; i < Def::MAX_INVENTORY_SIZE; ++i)
	{
		ITEM_INFO &item = PlayerInfo.itemInfo.equipInven[i];
		if (item.id == Def::ID_INVALID_ITEM)
			continue;

		ITEM_DESC *Desc = _ItemDesc().GetItem(item.id);
		if (Desc)
		{
			if ((Desc->gaugeType == _IGT_MAINTENANCE || Desc->gaugeType == _IGT_DURABILITY))
			{
				item.limit = Clamp<LONG>(item.limit, 0, ITEM_LIMIT_INITED);
			}
		}
	}
	for (INT i = 0; i < Def::MAX_CUSTOM_INVENTORY_SIZE; ++i)
	{
		CUSTOM_ITEM_INFO &item = PlayerInfo.itemInfo.customWeapon[i];
		if (item.id == Def::ID_INVALID_ITEM)
			continue;

		CUSTOM_ITEM_DESC *Desc = _ItemDesc().GetCustomItem(item.id);
		if (Desc)
		{
			if ((Desc->gaugeType == _IGT_MAINTENANCE || Desc->gaugeType == _IGT_DURABILITY))
			{
				item.limit = Clamp<LONG>(item.limit, 0, ITEM_LIMIT_INITED);
			}
		}
	}
	for (INT i = 0; i < Def::MAX_EFFECT_INVENTORY_SIZE; ++i)
	{
		EFFECT_ITEM_INFO &item = PlayerInfo.itemInfo.effectInven[i];
		if (item.id == Def::ID_INVALID_ITEM)
			continue;

		EFFECT_ITEM_DESC *Desc = _ItemDesc().GetEffectItem(item.id);
		if (Desc)
		{
			if ((Desc->gaugeType == _IGT_MAINTENANCE || Desc->gaugeType == _IGT_DURABILITY))
			{
				item.limit = Clamp<LONG>(item.limit, 0, ITEM_LIMIT_INITED);
			}
		}
	}

	Inven.RebuildEffect();
	CheckGrenadeSlots();

	BackupItemInfo();

	bItemInfo = TRUE;
}

void FPlayerInfo::BackupItemInfo()
{
	appMemcpy(&OldItemInfo, &PlayerInfo.itemInfo, sizeof(OldItemInfo));
}

void FPlayerInfo::CheckGrenadeSlots()
{
	// 군장 아이템들을 체크해서 열려있는 투척무기 슬롯 개수 얻어냄
	INT GRSlotCount = Inven.GetUsedEffect(_IET_GR) + 1;
	_LOG(TEXT("Count = %d"), GRSlotCount);
	// 필요 없는 슬롯 막음
	switch (GRSlotCount)
	{
	case 1:
		PlayerInfo.itemInfo.weaponSet[10] = SN_INVALID_ITEM;
		PlayerInfo.itemInfo.weaponSet[13] = SN_INVALID_ITEM;
		PlayerInfo.itemInfo.weaponSet[16] = SN_INVALID_ITEM;
		// continue to case 2
	case 2:
		PlayerInfo.itemInfo.weaponSet[11] = SN_INVALID_ITEM;
		PlayerInfo.itemInfo.weaponSet[14] = SN_INVALID_ITEM;
		PlayerInfo.itemInfo.weaponSet[17] = SN_INVALID_ITEM;
		break;
	case 3:
		break;
	default:
		break;
	}
}

void FPlayerInfo::SetSkillInfo(Def::PLAYER_SKILL_INFO &skill)
{
	appMemcpy(&PlayerInfo.skillInfo, &skill, sizeof(PLAYER_SKILL_INFO));
	bSkillInfo = TRUE;
}

void FPlayerInfo::SetScoreInfo(Def::PLAYER_SCORE_INFO &score)
{
	appMemcpy(&PlayerInfo.scoreInfo, &score, sizeof(PLAYER_SCORE_INFO));
	bScoreInfo = TRUE;
}

void FPlayerInfo::SetAwardInfo(Def::PLAYER_AWARD_INFO &award)
{
	appMemcpy(&PlayerInfo.awardInfo, &award, sizeof(PLAYER_AWARD_INFO));
	bAwardInfo = TRUE;
}

void FPlayerInfo::SetGuildInfo(Def::PLAYER_GUILD_INFO &guild)
{
	appMemcpy(&PlayerInfo.guildInfo, &guild, sizeof(PLAYER_GUILD_INFO));
	bGuildInfo = TRUE;
}

INT FPlayerInfo::GetAwardProgress(BYTE idAward)
{
	if (idAward < 0 || idAward >= MAX_AWARD_PER_PLAYER)
		return -1;

	FAwardInfo *AwardInfo = _AwardDesc().GetAwardInfo(idAward);
	if (!AwardInfo)
		return -1;
	if (AwardInfo->Type != 0 && AwardInfo->Type != 2)
		return -1;

	if (PlayerInfo.awardInfo.info[idAward] > 0)
		return 100;

	if (AwardInfo->CheckValue <= 0)
		return -1;

	INT CheckValue = 0;
	switch (AwardInfo->CheckProperty)
	{
	case CavaAwardDesc::_Check_None:
		return -1;
	case CavaAwardDesc::_Check_Level:
		CheckValue = PlayerInfo.level;
		break;
	case CavaAwardDesc::_Check_XP:
		CheckValue = PlayerInfo.xp;
		break;
	case CavaAwardDesc::_Check_GameWin:
		CheckValue = PlayerInfo.scoreInfo.gameWin;
		break;
	case CavaAwardDesc::_Check_GameDefeat:
		CheckValue = PlayerInfo.scoreInfo.gameDefeat;
		break;
	case CavaAwardDesc::_Check_RoundWin:
		CheckValue = PlayerInfo.scoreInfo.roundWin;
		break;
	case CavaAwardDesc::_Check_RoundDefeat:
		CheckValue = PlayerInfo.scoreInfo.roundDefeat;
		break;
	case CavaAwardDesc::_Check_DisconnectCount:
		CheckValue = PlayerInfo.scoreInfo.disconnectCount;
		break;
	case CavaAwardDesc::_Check_DeathCount:
		CheckValue = PlayerInfo.scoreInfo.deathCount;
		break;
	case CavaAwardDesc::_Check_StraightWinCount:
		CheckValue = PlayerInfo.straightWin;
		break;
	case CavaAwardDesc::_Check_TeamKillCount:
		CheckValue = PlayerInfo.scoreInfo.teamKillCount;
		break;
	case CavaAwardDesc::_Check_PlayTime:
		{
			for (INT i = 0; i < _CLASS_MAX; ++i)
				CheckValue += PlayerInfo.scoreInfo.classScoreInfo[i].playTime;
		}
		break;

	case CavaAwardDesc::_Check_Score_Attacker:
		CheckValue = PlayerInfo.scoreInfo.score.attacker;
		break;
	case CavaAwardDesc::_Check_Score_Defender:
		CheckValue = PlayerInfo.scoreInfo.score.defender;
		break;
	case CavaAwardDesc::_Check_Score_Leader:
		CheckValue = PlayerInfo.scoreInfo.score.leader;
		break;
	case CavaAwardDesc::_Check_Score_Tactic:
		CheckValue = PlayerInfo.scoreInfo.score.tactic;
		break;

	case CavaAwardDesc::_Check_P_PlayRound:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].playRound;
		break;
	case CavaAwardDesc::_Check_P_HeadshotCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].headshotCount;
		break;
	case CavaAwardDesc::_Check_P_HeadshotKillCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].headshotKillCount;
		break;
	case CavaAwardDesc::_Check_P_PlayTime:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].playTime;
		break;
	case CavaAwardDesc::_Check_P_SprintTime:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].sprintTime;
		break;
	case CavaAwardDesc::_Check_P_TakenDamage:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].takenDamage;
		break;
	case CavaAwardDesc::_Check_P_KillCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].killCount;
		break;
	case CavaAwardDesc::_Check_P_WeaponKillCount_Pistol:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].weaponKillCount[Def::_WEAPON_PISTOL];
		break;
	case CavaAwardDesc::_Check_P_WeaponKillCount_Knife:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].weaponKillCount[Def::_WEAPON_KNIFE];
		break;
	case CavaAwardDesc::_Check_P_WeaponKillCount_Grenade:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].weaponKillCount[Def::_WEAPON_GRENADE];
		break;
	case CavaAwardDesc::_Check_P_WeaponKillCount_Primary:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].weaponKillCount[Def::_WEAPON_PRIMARY];
		break;
	case CavaAwardDesc::_Check_P_WeaponDamage_Pistol:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].weaponDamage[Def::_WEAPON_PISTOL];
		break;
	case CavaAwardDesc::_Check_P_WeaponDamage_Knife:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].weaponDamage[Def::_WEAPON_KNIFE];
		break;
	case CavaAwardDesc::_Check_P_WeaponDamage_Grenade:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].weaponDamage[Def::_WEAPON_GRENADE];
		break;
	case CavaAwardDesc::_Check_P_WeaponDamage_Primary:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_POINTMAN].weaponDamage[Def::_WEAPON_PRIMARY];
		break;

	case CavaAwardDesc::_Check_R_PlayRound:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].playRound;
		break;
	case CavaAwardDesc::_Check_R_HeadshotCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].headshotCount;
		break;
	case CavaAwardDesc::_Check_R_HeadshotKillCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].headshotKillCount;
		break;
	case CavaAwardDesc::_Check_R_PlayTime:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].playTime;
		break;
	case CavaAwardDesc::_Check_R_SprintTime:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].sprintTime;
		break;
	case CavaAwardDesc::_Check_R_TakenDamage:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].takenDamage;
		break;
	case CavaAwardDesc::_Check_R_KillCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].killCount;
		break;
	case CavaAwardDesc::_Check_R_WeaponKillCount_Pistol:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].weaponKillCount[Def::_WEAPON_PISTOL];
		break;
	case CavaAwardDesc::_Check_R_WeaponKillCount_Knife:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].weaponKillCount[Def::_WEAPON_KNIFE];
		break;
	case CavaAwardDesc::_Check_R_WeaponKillCount_Grenade:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].weaponKillCount[Def::_WEAPON_GRENADE];
		break;
	case CavaAwardDesc::_Check_R_WeaponKillCount_Primary:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].weaponKillCount[Def::_WEAPON_PRIMARY];
		break;
	case CavaAwardDesc::_Check_R_WeaponDamage_Pistol:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].weaponDamage[Def::_WEAPON_PISTOL];
		break;
	case CavaAwardDesc::_Check_R_WeaponDamage_Knife:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].weaponDamage[Def::_WEAPON_KNIFE];
		break;
	case CavaAwardDesc::_Check_R_WeaponDamage_Grenade:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].weaponDamage[Def::_WEAPON_GRENADE];
		break;
	case CavaAwardDesc::_Check_R_WeaponDamage_Primary:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_RIFLEMAN].weaponDamage[Def::_WEAPON_PRIMARY];
		break;

	case CavaAwardDesc::_Check_S_PlayRound:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].playRound;
		break;
	case CavaAwardDesc::_Check_S_HeadshotCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].headshotCount;
		break;
	case CavaAwardDesc::_Check_S_HeadshotKillCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].headshotKillCount;
		break;
	case CavaAwardDesc::_Check_S_PlayTime:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].playTime;
		break;
	case CavaAwardDesc::_Check_S_SprintTime:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].sprintTime;
		break;
	case CavaAwardDesc::_Check_S_TakenDamage:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].takenDamage;
		break;
	case CavaAwardDesc::_Check_S_KillCount:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].killCount;
		break;
	case CavaAwardDesc::_Check_S_WeaponKillCount_Pistol:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].weaponKillCount[Def::_WEAPON_PISTOL];
		break;
	case CavaAwardDesc::_Check_S_WeaponKillCount_Knife:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].weaponKillCount[Def::_WEAPON_KNIFE];
		break;
	case CavaAwardDesc::_Check_S_WeaponKillCount_Grenade:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].weaponKillCount[Def::_WEAPON_GRENADE];
		break;
	case CavaAwardDesc::_Check_S_WeaponKillCount_Primary:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].weaponKillCount[Def::_WEAPON_PRIMARY];
		break;
	case CavaAwardDesc::_Check_S_WeaponDamage_Pistol:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].weaponDamage[Def::_WEAPON_PISTOL];
		break;
	case CavaAwardDesc::_Check_S_WeaponDamage_Knife:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].weaponDamage[Def::_WEAPON_KNIFE];
		break;
	case CavaAwardDesc::_Check_S_WeaponDamage_Grenade:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].weaponDamage[Def::_WEAPON_GRENADE];
		break;
	case CavaAwardDesc::_Check_S_WeaponDamage_Primary:
		CheckValue = PlayerInfo.scoreInfo.classScoreInfo[_CLASS_SNIPER].weaponDamage[Def::_WEAPON_PRIMARY];
		break;
	case CavaAwardDesc::_Check_P_SkillMaster:
		{
			INT SkillCount = 0;
			for (INT i = 0; i < MAX_SKILL_PER_CLASS; ++i)
			{
				SkillCount += (PlayerInfo.skillInfo.skill[_CLASS_POINTMAN] & (1 << i)) ? 1 : 0;
			}
			return (SkillCount > 12 ? 100 : 100 * SkillCount / 12);
		}
		break;
	case CavaAwardDesc::_Check_R_SkillMaster:
		{
			INT SkillCount = 0;
			for (INT i = 0; i < MAX_SKILL_PER_CLASS; ++i)
			{
				SkillCount += (PlayerInfo.skillInfo.skill[_CLASS_RIFLEMAN] & (1 << i)) ? 1 : 0;
			}
			return (SkillCount > 12 ? 100 : 100 * SkillCount / 12);
		}
		break;
	case CavaAwardDesc::_Check_S_SkillMaster:
		{
			INT SkillCount = 0;
			for (INT i = 0; i < MAX_SKILL_PER_CLASS; ++i)
			{
				SkillCount += (PlayerInfo.skillInfo.skill[_CLASS_SNIPER] & (1 << i)) ? 1 : 0;
			}
			return (SkillCount > 11 ? 100 : 100 * SkillCount / 11);
		}
		break;
	default:
		return -1;
	}

	INT Progress = 100 * CheckValue / AwardInfo->CheckValue;
	return Progress > 100 ? 100 : Progress;
}

void FPlayerInfo::UpdateCurrentClass()
{
	if (OldCurrentClass != PlayerInfo.currentClass)
	{
		OldCurrentClass = PlayerInfo.currentClass;
		PM::INVENTORY::CHANGE_CLASS_NTF::Send(OldCurrentClass);
	}
}

INT FPlayerInfo::GetBoostXPPerc()
{
	INT Perc = 0;
	
	//if (!_StateController->ChannelInfo.IsMyClanChannel())
	if (_StateController->GetChannelSetting(EChannelSetting_AllowBoostItem) > 0)
	{
		Perc = Inven.GetUsedEffect(_IET_EXP_BOOST);
	}
	return Perc;
}

INT FPlayerInfo::GetBonusXPPerc()
{
	INT Perc = GetBoostXPPerc();
	
#ifdef _HAPPY_NEW_YEAR_EVENT
	//if (!_StateController->ChannelInfo.IsMyClanChannel())
	if (_StateController->GetChannelSetting(EChannelSetting_AllowEventBonus) > 0)
	{
		Perc += 30;
	}
#endif
	return Perc;
}


INT FPlayerInfo::GetPcBangBonusXPPerc()
{
	INT Perc = 0;
	if (_StateController->GetChannelSetting(EChannelSetting_AllowPCBangBonus) > 0)
	{
		switch (PcBangServiceType)
		{
		case _PCB_AVA:
			Perc = 30;
			break;
		}
	}
	return Perc;
}

INT FPlayerInfo::GetBoostSupplyPerc()
{
	INT Perc = 0;
	//if (!_StateController->ChannelInfo.IsMyClanChannel())
	if (_StateController->GetChannelSetting(EChannelSetting_AllowBoostItem) > 0)
	{
		Perc = Inven.GetUsedEffect(_IET_SP_BOOST);
	}
	return Perc;
}

INT FPlayerInfo::GetBonusSupplyPerc()
{
	INT Perc = GetBoostSupplyPerc();
	return Perc;
}

INT FPlayerInfo::GetBoostMoneyPerc()
{
	INT Perc = 0;
	//if (!_StateController->ChannelInfo.IsMyClanChannel())
	if (_StateController->GetChannelSetting(EChannelSetting_AllowBoostItem) > 0)
	{
		Perc = Inven.GetUsedEffect(_IET_MONEY_BOOST);
	}
	return Perc;
}

INT FPlayerInfo::GetBonusMoneyPerc()
{
	INT Perc = GetBoostMoneyPerc();
	return Perc;
}

INT FPlayerInfo::GetPcBangBonusMoneyPerc()
{
	INT Perc = 0;
	if (_StateController->GetChannelSetting(EChannelSetting_AllowPCBangBonus) > 0)
	{
		switch (PcBangServiceType)
		{
		case _PCB_AVA:
			Perc = 6;
			break;
		}
	}
	return Perc;
}

INT FPlayerInfo::GetBonusBIAPerc()
{
	INT Perc = 20;
	return Perc;
}

INT FPlayerInfo::GetClanMarkID()
{
	return PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD ? PlayerInfo.guildInfo.idGuildMark : -1;
}

INT FPlayerInfo::GetCurrentChannelMaskLevel()
{
	if (!_StateController->ChannelInfo.IsValid() || !_StateController->ChannelInfo.IsMaskTurnedOn())
		return _CML_NONE;
	INT MaskLevel = PlayerInfo.channelMaskList[_StateController->ChannelInfo.Mask];
	if (IsAdmin() && MaskLevel == _CML_NONE)
		MaskLevel = _CML_REFREE;
	return MaskLevel;
}

void FPlayerInfo::DumpPlayerInfo()
{
#if !FINAL_RELEASE
	if (PlayerInfo.idAccount != ID_INVALID_ACCOUNT)
	{
		//FString Str;
		//for (INT m = 0; m < Def::MAX_CLASS_TYPE; ++m)
		//{
		//	if (m > 0)
		//		Str += TEXT("/");
		//	for (INT n = 0; n < Def::MAX_SKILL_PER_CLASS; ++n)
		//	{
		//		if (n > 0)
		//			Str += TEXT(",");
		//		Str += appItoa(PlayerInfo.skill[m][n]);
		//	}
		//}

#pragma warning(disable:4548)

		_DUMP(TEXT("[%d] %s; Level/XP = <%d/%d(%d%%)>; Money/Supply = <%d/%d>; Face = %d; BIA XP = %d"),
			PlayerInfo.idAccount,
			PlayerInfo.nickname,
			PlayerInfo.level,
			PlayerInfo.xp,
			PlayerInfo.xpProgress,
			PlayerInfo.money,
			PlayerInfo.supplyPoint,
			PlayerInfo.faceType,
			PlayerInfo.biaXP);

		FString SkillP, SkillR, SkillS;
		for (INT i = 15; i >= 0; --i)
		{
			SkillP += PlayerInfo.skillInfo.skill[0] & (1 << i) ? TEXT("1") : TEXT("0");
			SkillR += PlayerInfo.skillInfo.skill[1] & (1 << i) ? TEXT("1") : TEXT("0");
			SkillS += PlayerInfo.skillInfo.skill[2] & (1 << i) ? TEXT("1") : TEXT("0");
		}
		_DUMP(TEXT("SkillP = %s"), *SkillP);
		_DUMP(TEXT("SkillR = %s"), *SkillR);
		_DUMP(TEXT("SkillS = %s"), *SkillS);

		_DUMP(TEXT("GameWin = %d, RoundWin = %d, RoundDefeat = %d, DeathCount = %d, Score = %d/%d/%d/%d"),
					PlayerInfo.scoreInfo.gameWin, PlayerInfo.scoreInfo.roundWin, PlayerInfo.scoreInfo.roundDefeat, PlayerInfo.scoreInfo.deathCount,
					PlayerInfo.scoreInfo.score.attacker, PlayerInfo.scoreInfo.score.defender, PlayerInfo.scoreInfo.score.leader, PlayerInfo.scoreInfo.score.tactic);
		_DUMP(TEXT("StraightWin = %d, TeamKill = %d"), PlayerInfo.scoreInfo.straightWinCount, PlayerInfo.scoreInfo.teamKillCount);
		_DUMP(TEXT("Pointman;"));
		_DUMP(TEXT("    PlayTime = %d, PlayRound = %d, SprintTime = %d, KillCount = %d, TakenDamage = %d, HeadshotCount = %d, HeadshotKillCount = %d"),
					PlayerInfo.scoreInfo.classScoreInfo[0].playTime, PlayerInfo.scoreInfo.classScoreInfo[0].playRound, PlayerInfo.scoreInfo.classScoreInfo[0].sprintTime,
					PlayerInfo.scoreInfo.classScoreInfo[0].killCount, PlayerInfo.scoreInfo.classScoreInfo[0].takenDamage,
					PlayerInfo.scoreInfo.classScoreInfo[0].headshotCount, PlayerInfo.scoreInfo.classScoreInfo[0].headshotKillCount);
		_DUMP(TEXT("    SMGDamage = %d, SMGKillCount = %d, PistolDamage = %d, PistolKillCount = %d, KnifeDamage = %d, KnifeKillCount = %d, GrenadeDamage = %d, GrenadeKillCount = %d"),
					PlayerInfo.scoreInfo.classScoreInfo[0].weaponDamage[3], PlayerInfo.scoreInfo.classScoreInfo[0].weaponKillCount[3],
					PlayerInfo.scoreInfo.classScoreInfo[0].weaponDamage[1], PlayerInfo.scoreInfo.classScoreInfo[0].weaponKillCount[1],
					PlayerInfo.scoreInfo.classScoreInfo[0].weaponDamage[0], PlayerInfo.scoreInfo.classScoreInfo[0].weaponKillCount[0],
					PlayerInfo.scoreInfo.classScoreInfo[0].weaponDamage[2], PlayerInfo.scoreInfo.classScoreInfo[0].weaponKillCount[2]);
		_DUMP(TEXT("Rifleman;"));
		_DUMP(TEXT("    PlayTime = %d, PlayRound = %d, SprintTime = %d, KillCount = %d, TakenDamage = %d, HeadshotCount = %d, HeadshotKillCount = %d"),
					PlayerInfo.scoreInfo.classScoreInfo[1].playTime, PlayerInfo.scoreInfo.classScoreInfo[1].playRound, PlayerInfo.scoreInfo.classScoreInfo[1].sprintTime,
					PlayerInfo.scoreInfo.classScoreInfo[1].killCount, PlayerInfo.scoreInfo.classScoreInfo[1].takenDamage,
					PlayerInfo.scoreInfo.classScoreInfo[1].headshotCount, PlayerInfo.scoreInfo.classScoreInfo[1].headshotKillCount);
		_DUMP(TEXT("    ARDamage = %d, ARKillCount = %d, PistolDamage = %d, PistolKillCount = %d, KnifeDamage = %d, KnifeKillCount = %d, GrenadeDamage = %d, GrenadeKillCount = %d"),
					PlayerInfo.scoreInfo.classScoreInfo[1].weaponDamage[3], PlayerInfo.scoreInfo.classScoreInfo[1].weaponKillCount[3],
					PlayerInfo.scoreInfo.classScoreInfo[1].weaponDamage[1], PlayerInfo.scoreInfo.classScoreInfo[1].weaponKillCount[1],
					PlayerInfo.scoreInfo.classScoreInfo[1].weaponDamage[0], PlayerInfo.scoreInfo.classScoreInfo[1].weaponKillCount[0],
					PlayerInfo.scoreInfo.classScoreInfo[1].weaponDamage[2], PlayerInfo.scoreInfo.classScoreInfo[1].weaponKillCount[2]);
		_DUMP(TEXT("Sniper;"));
		_DUMP(TEXT("    PlayTime = %d, PlayRound = %d, SprintTime = %d, KillCount = %d, TakenDamage = %d, HeadshotCount = %d, HeadshotKillCount = %d"),
					PlayerInfo.scoreInfo.classScoreInfo[2].playTime, PlayerInfo.scoreInfo.classScoreInfo[2].playRound, PlayerInfo.scoreInfo.classScoreInfo[2].sprintTime,
					PlayerInfo.scoreInfo.classScoreInfo[2].killCount, PlayerInfo.scoreInfo.classScoreInfo[2].takenDamage,
					PlayerInfo.scoreInfo.classScoreInfo[2].headshotCount, PlayerInfo.scoreInfo.classScoreInfo[2].headshotKillCount);
		_DUMP(TEXT("    SRDamage = %d, SRKillCount = %d, PistolDamage = %d, PistolKillCount = %d, KnifeDamage = %d, KnifeKillCount = %d, GrenadeDamage = %d, GrenadeKillCount = %d"),
					PlayerInfo.scoreInfo.classScoreInfo[2].weaponDamage[3], PlayerInfo.scoreInfo.classScoreInfo[2].weaponKillCount[3],
					PlayerInfo.scoreInfo.classScoreInfo[2].weaponDamage[1], PlayerInfo.scoreInfo.classScoreInfo[2].weaponKillCount[1],
					PlayerInfo.scoreInfo.classScoreInfo[2].weaponDamage[0], PlayerInfo.scoreInfo.classScoreInfo[2].weaponKillCount[0],
					PlayerInfo.scoreInfo.classScoreInfo[2].weaponDamage[2], PlayerInfo.scoreInfo.classScoreInfo[2].weaponKillCount[2]);
	}
#endif
}

void FPlayerInfo::DumpItemInfo()
{
#if !FINAL_RELEASE
	FString Str;
	Def::ITEM_DESC *Desc;

	_DUMP(TEXT("Weapon Inventory = "));
	for (INT i = 0; i < Def::MAX_INVENTORY_SIZE; ++i)
	{
		if (PlayerInfo.itemInfo.weaponInven[i].id == Def::ID_INVALID_ITEM)
			continue;

		Desc = _ItemDesc().GetItem(PlayerInfo.itemInfo.weaponInven[i].id);
		if (Desc)
		{
			_DUMP(*FString::Printf(TEXT("[%d]<%d:%I64d>%s (gaugeType = %d, limit = %d)"),
									i,
									PlayerInfo.itemInfo.weaponInven[i].id,
									PlayerInfo.itemInfo.weaponInven[i].sn,
									Desc->GetName(),
									Desc->gaugeType,
									PlayerInfo.itemInfo.weaponInven[i].limit));
		}
	}

	_DUMP(TEXT("Equip Inventory = "));
	for (INT i = 0; i < Def::MAX_INVENTORY_SIZE; ++i)
	{
		if (PlayerInfo.itemInfo.equipInven[i].id == Def::ID_INVALID_ITEM)
			continue;
		Desc = _ItemDesc().GetItem(PlayerInfo.itemInfo.equipInven[i].id);
		if (Desc)
		{
			_DUMP(*FString::Printf(TEXT("[%d]<%d:%I64d>%s (gaugeType = %d, limit = %d)"),
									i,
									PlayerInfo.itemInfo.equipInven[i].id,
									PlayerInfo.itemInfo.equipInven[i].sn,
									Desc->GetName(),
									Desc->gaugeType,
									PlayerInfo.itemInfo.equipInven[i].limit));
		}
	}

	_DUMP(TEXT("Effect Inventory = "));
	for (INT i = 0; i < Def::MAX_EFFECT_INVENTORY_SIZE; ++i)
	{
		if (PlayerInfo.itemInfo.effectInven[i].id == Def::ID_INVALID_ITEM)
			continue;

		EFFECT_ITEM_DESC *Desc = _ItemDesc().GetEffectItem(PlayerInfo.itemInfo.effectInven[i].id);
		if (Desc)
		{
			_DUMP(*FString::Printf(TEXT("[%d]<%d:%I64d>%s (gaugeType = %d, limit = %d)"),
									i,
									PlayerInfo.itemInfo.effectInven[i].id,
									PlayerInfo.itemInfo.effectInven[i].item_sn,
									Desc->GetName(),
									Desc->gaugeType,
									PlayerInfo.itemInfo.effectInven[i].limit));
		}
	}

	Str = TEXT("Weapon Set = ");
	for (INT i = 0; i < Def::MAX_WEAPONSET_SIZE; ++i)
	{
		TSN_ITEM sn = PlayerInfo.itemInfo.weaponSet[i];
		if (sn == Def::SN_INVALID_ITEM)
		{
			Str += FString::Printf(TEXT("[%d]**Empty** "), i);
		}
		else
		{
			for (INT j = 0; j < Def::MAX_INVENTORY_SIZE; ++j)
				if (PlayerInfo.itemInfo.weaponInven[j].sn == sn)
				{
					Desc = _ItemDesc().GetItem(PlayerInfo.itemInfo.weaponInven[j].id);
					if (Desc)
						Str += FString::Printf(TEXT("[%d]<%d:%I64d>%s (gaugeType = %d, limit = %d)"),
												i,
												PlayerInfo.itemInfo.weaponInven[j].id,
												PlayerInfo.itemInfo.weaponInven[j].sn,
												Desc->GetName(),
												Desc->gaugeType,
												PlayerInfo.itemInfo.weaponInven[i].limit);
					break;
				}
		}
	}
	_DUMP(*Str);

	Str = TEXT("Equip Set = ");
	for (INT i = 0; i < Def::MAX_EQUIPSET_SIZE; ++i)
	{
		TSN_ITEM sn = PlayerInfo.itemInfo.equipSet[i];
		if (sn == Def::SN_INVALID_ITEM)
		{
			Str += FString::Printf(TEXT("[%d]**Empty** "), i);
		}
		else
		{
			for (INT j = 0; j < Def::MAX_INVENTORY_SIZE; ++j)
				if (PlayerInfo.itemInfo.equipInven[j].sn == sn)
				{
					Desc = _ItemDesc().GetItem(PlayerInfo.itemInfo.equipInven[j].id);
					if (Desc)
						Str += FString::Printf(TEXT("[%d]<%d:%I64d>%s (gaugeType = %d, limit = %d)"),
												i,
												PlayerInfo.itemInfo.equipInven[j].id,
												PlayerInfo.itemInfo.equipInven[j].sn,
												Desc->GetName(),
												Desc->gaugeType,
												PlayerInfo.itemInfo.equipInven[i].limit);
					break;
				}
		}
	}
	_DUMP(*Str);

	Str = TEXT("Effect Set = ");
	for (INT i = 0; i < Def::MAX_EFFECTSET_SIZE; ++i)
	{
		TSN_ITEM sn = PlayerInfo.itemInfo.effectSet[i];
		if (sn == Def::SN_INVALID_ITEM)
		{
			Str += FString::Printf(TEXT("[%d]**Empty** "), i);
		}
		else
		{
			for (INT j = 0; j < Def::MAX_EFFECT_INVENTORY_SIZE; ++j)
				if (PlayerInfo.itemInfo.effectInven[j].item_sn == sn)
				{
					EFFECT_ITEM_DESC *Desc = _ItemDesc().GetEffectItem(PlayerInfo.itemInfo.effectInven[j].id);
					if (Desc)
						Str += FString::Printf(TEXT("[%d]<%d:%I64d>%s (gaugeType = %d, limit = %d)"),
												i,
												PlayerInfo.itemInfo.effectInven[j].id,
												PlayerInfo.itemInfo.effectInven[j].item_sn,
												Desc->GetName(),
												Desc->gaugeType,
												PlayerInfo.itemInfo.effectInven[i].limit);
					break;
				}
		}
	}
	_DUMP(*Str);
#endif
}


void FPlayerInfo::DumpGuildInfo()
{
#if !FINAL_RELEASE
	if (bGuildInfo)
	{
		_DUMP(TEXT("Guild = [%d][%s]%s"), PlayerInfo.guildInfo.idGuild, PlayerInfo.guildInfo.strGuildID, PlayerInfo.guildInfo.guildName);
	}
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// FPlayerDispInfo

void FPlayerDispInfo::DumpPlayerInfo()
{
#if !FINAL_RELEASE
	_DUMP(TEXT("[%d] %s; Level = %d, idGuildMark = %d"),
		PlayerInfo.idAccount,
		PlayerInfo.nickname,
		PlayerInfo.level,
		PlayerInfo.idGuildMark);
#endif
}

void FPlayerDispInfo::Set(PLAYER_INFO &Info)
{
	if (Info.idAccount != ID_INVALID_ACCOUNT)
	{
		PlayerInfo = Info;
		SetFullInfo(TRUE);
	}
}

void FPlayerDispInfo::Set(PLAYER_DISP_INFO &Info)
{
	if (Info.idAccount != ID_INVALID_ACCOUNT)
	{
		appMemcpy(&PlayerInfo, &Info, sizeof(PLAYER_DISP_INFO));
		SetFullInfo(TRUE);
	}
}

INT FPlayerDispInfo::GetClanMarkID()
{
	return (IsFullInfo() && appStrlen(PlayerInfo.guildName) > 0) ? PlayerInfo.idGuildMark : -1;
}

void FPlayerDispInfo::SyncPlayerInfo( FGuildPlayerInfo &Player )
{
	if (InfoTimeStamp > 0.0 && InfoTimeStamp > Player.InfoTimeStamp)
		return;

	appStrcpy(PlayerInfo.nickname, Player.GuildPlayerInfo.nickname);
	appStrcpy(PlayerInfo.guildName, _StateController->GuildInfo.GuildInfo.name);
	PlayerInfo.idGuildMark = _StateController->GuildInfo.GuildInfo.idGuildMark;
	PlayerInfo.gameWin = Player.GameWin;
	PlayerInfo.gameDefeat = Player.GameDefeat;
	PlayerInfo.scoreInfo = Player.Score;
	PlayerInfo.killCount = Player.KillCount;
	PlayerInfo.deathCount = Player.DeathCount;
	PlayerInfo.disconnectCount = Player.DisconnectCount;
	PlayerInfo.level = Player.GuildPlayerInfo.level;
	PlayerInfo.scoreInfo = Player.Score;
	SetFullInfo(TRUE);}


//////////////////////////////////////////////////////////////////////////////////////////
// FLobbyPlayerInfo

void FLobbyPlayerInfo::Set(PLAYER_INFO &Info)
{
	if (Info.idAccount != ID_INVALID_ACCOUNT)
	{
		PlayerInfo = Info;
		LobbyPlayerInfo.idAccount = Info.idAccount;
		LobbyPlayerInfo.level = Info.level;
		appStrcpy(LobbyPlayerInfo.nickname, Info.nickname);

		SetFullInfo(TRUE);
	}
}

void FLobbyPlayerInfo::Set(PLAYER_DISP_INFO &Info)
{
	if (Info.idAccount == LobbyPlayerInfo.idAccount)
	{
		appMemcpy(&PlayerInfo, &Info, sizeof(PLAYER_DISP_INFO));
		LobbyPlayerInfo.idAccount = Info.idAccount;
		LobbyPlayerInfo.level = Info.level;
		appStrcpy(LobbyPlayerInfo.nickname, Info.nickname);

		SetFullInfo(TRUE);

		//LobbyPlayerInfo.level = Info.level;
		//appStrcpy(LobbyPlayerInfo.nickname, Info.nickname);
	}
}

void FLobbyPlayerInfo::Set(LOBBY_PLAYER_INFO &Info)
{
	if (Info.idAccount != ID_INVALID_ACCOUNT)
	{
		appMemcpy(&LobbyPlayerInfo, &Info, sizeof(LOBBY_PLAYER_INFO));
		PlayerInfo.idAccount = Info.idAccount;
		PlayerInfo.level = Info.level;
		appStrcpy(PlayerInfo.nickname, Info.nickname);

		// LOBBY_PLAYER_INFO 만으로는 FullInfo 설정하지 않음
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
// FRoomPlayerInfo

void FRoomPlayerInfo::DumpPlayerInfo()
{
#if !FINAL_RELEASE
	TID_ITEM WeapID = GetPrimaryWeaponID();
	FString WeapStr = _ItemDesc().GetItem(WeapID) ? _ItemDesc().GetItem(WeapID)->GetName() : *appItoa(WeapID);
	for (INT i = 0; i < _CSI_MAX; ++i)
	{
		TID_ITEM CustID = RoomPlayerInfo.customItem[RoomPlayerInfo.currentClass][i];
		if (CustID != ID_INVALID_ITEM)
			WeapStr += FString::Printf(TEXT("@%d"), CustID);
	}

	_DUMP(TEXT("[%02d] %s; [%d]%s; Level = <%d>; <%s|%s> %s, hostRating = %d, rtt = %.2f, Weapon = %s"),
		RoomPlayerInfo.idSlot,
		RoomPlayerInfo.nickname,
		RoomPlayerInfo.idGuildMark,
		RoomPlayerInfo.guildName,
		RoomPlayerInfo.level,
		GetTeamID() == RT_EU ? TEXT("EU") : TEXT("NRF"),
		RoomPlayerInfo.currentClass == _CLASS_POINTMAN ? TEXT("P") : RoomPlayerInfo.currentClass == _CLASS_RIFLEMAN ? TEXT("R") : TEXT("S"),
		RoomPlayerInfo.bReady == _READY_NONE ? TEXT("") : RoomPlayerInfo.bReady == _READY_WAIT ? TEXT("Ready") : RoomPlayerInfo.bReady == _READY_LOADING ? TEXT("Loading") : TEXT("Playing"),
		RoomPlayerInfo.hostRating, RoomPlayerInfo.rttScore,
		*WeapStr);
#endif
}

void FRoomPlayerInfo::Set(PLAYER_INFO &Info)
{
	if (Info.idAccount != ID_INVALID_ACCOUNT)
	{
		PlayerInfo = Info;
		RoomPlayerInfo = Info;

		//RoomPlayerInfo.bReady = _READY_NONE;
		RoomPlayerInfo.rttScore = -1;
		RttValue = -1;
		SetFullInfo(TRUE);
	}
}

void FRoomPlayerInfo::Set(PLAYER_DISP_INFO &Info)
{
	if (Info.idAccount == RoomPlayerInfo.idAccount)
	{
		appMemcpy(&PlayerInfo, &Info, sizeof(PLAYER_DISP_INFO));

		RoomPlayerInfo.idAccount = Info.idAccount;
		appStrcpy(RoomPlayerInfo.nickname, Info.nickname);
		RoomPlayerInfo.level = Info.level;
		//RoomPlayerInfo.faceType = Info.faceType;
		//RoomPlayerInfo.currentClass = Info.currentClass;
		//appMemcpy(RoomPlayerInfo.equipItem, Info.equipItem, MAX_EQUIPSET_SIZE * sizeof(TID_ITEM));
		//appMemcpy(RoomPlayerInfo.weaponItem, Info.weaponItem, MAX_WEAPONSET_SIZE * sizeof(TID_ITEM));

		SetFullInfo(TRUE);

		//RoomPlayerInfo.level = Info.level;
	}
}

void FRoomPlayerInfo::Set(ROOM_PLAYER_INFO &Info)
{
	if (Info.idAccount != ID_INVALID_ACCOUNT)
	{
		//RoomPlayerInfo = Info;
		appMemcpy(&RoomPlayerInfo, &Info, sizeof(ROOM_PLAYER_INFO));

		PlayerInfo.idAccount = Info.idAccount;
		appStrcpy(PlayerInfo.nickname, Info.nickname);
		//PlayerInfo.faceType = Info.faceType;
		PlayerInfo.level = Info.level;
		//PlayerInfo.currentClass = Info.currentClass;
		//appMemcpy(PlayerInfo.weaponItem, Info.weaponItem, sizeof(Info.weaponItem));
		//appMemcpy(PlayerInfo.equipItem, Info.equipItem, sizeof(Info.equipItem));

		RttValue = Info.rttScore;

		// ROOM_PLAYER_INFO 만으로는 FullInfo 설정하지 않음
	}
}

FString FRoomPlayerInfo::GetPrimaryWeaponIconCode(INT idClass)
{
	INT SlotID = GetPrimaryWeaponSlotIdx(idClass);
	if (SlotID == ID_INVALID_EQUIP_SLOT)
		return TEXT("");
	else
		return GetAvaNetHandler()->GetWeaponIconCode(RoomPlayerInfo.weaponItem[SlotID].id);
}

DWORD FRoomPlayerInfo::GetPrimaryWeaponSlotIdx(INT idClass)
{
	INT CurrClass = (idClass >= 0 && idClass < _CLASS_MAX ? idClass : RoomPlayerInfo.currentClass);
	if (CurrClass < 0 || idClass >= _CLASS_MAX)
		CurrClass = _CLASS_POINTMAN;
	INT SlotType = (CurrClass == _CLASS_POINTMAN ? _EP_P1 : CurrClass == _CLASS_RIFLEMAN ? _EP_R1 : _EP_S1);
	SLOT_DESC *pSlotDesc = _ItemDesc().GetWeaponSlotByType(SlotType);

	return pSlotDesc ? pSlotDesc->index : ID_INVALID_EQUIP_SLOT;
}

DWORD FRoomPlayerInfo::GetPrimaryWeaponID(INT idClass)
{
	DWORD SlotIdx = GetPrimaryWeaponSlotIdx(idClass);
	return (SlotIdx != ID_INVALID_EQUIP_SLOT ? RoomPlayerInfo.weaponItem[SlotIdx].id : ID_INVALID_ITEM);
}

FString FRoomPlayerInfo::GetCurrentWeaponIconCode()
{
	return GetAvaNetHandler()->GetWeaponIconCode(GetPrimaryWeaponID());
}

BYTE FRoomPlayerInfo::GetTeamID()
{
	return RoomPlayerInfo.idSlot != ID_INVALID_ROOM_SLOT ? FRoomInfo::SlotToTeam(RoomPlayerInfo.idSlot) : 255;
}

INT FRoomPlayerInfo::GetClanMarkID()
{
	return appStrlen(RoomPlayerInfo.guildName) > 0 ? RoomPlayerInfo.idGuildMark : -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// FLobbyPlayerList

FLobbyPlayerInfo* FLobbyPlayerList::Add(PLAYER_INFO &Player)
{
	FLobbyPlayerInfo *Info = NULL;

	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).PlayerInfo.idAccount == Player.idAccount)
		{
			Info = &(PlayerList(i));
			break;
		}
	}

	if (!Info)
		Info = new(PlayerList) FLobbyPlayerInfo();

	check(Info);

	Info->Set(Player);

	return Info;
}

FLobbyPlayerInfo* FLobbyPlayerList::Add(PLAYER_DISP_INFO &Player)
{
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).PlayerInfo.idAccount == Player.idAccount)
		{
			PlayerList(i).Set(Player);
			return &(PlayerList(i));
		}
	}

	FLobbyPlayerInfo* Info = new(PlayerList) FLobbyPlayerInfo();
	check(Info);

	if (Player.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		Info->Set(_StateController->PlayerInfo.PlayerInfo);
	}

	Info->Set(Player);

	return Info;
}

FLobbyPlayerInfo* FLobbyPlayerList::Add(LOBBY_PLAYER_INFO &Player)
{
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).PlayerInfo.idAccount == Player.idAccount)
		{
			PlayerList(i).Set(Player);
			return &(PlayerList(i));
		}
	}

	FLobbyPlayerInfo* Info = new(PlayerList) FLobbyPlayerInfo();
	check(Info);

	if (Player.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		Info->Set(_StateController->PlayerInfo.PlayerInfo);
	}

	Info->Set(Player);

	return Info;
}

UBOOL FLobbyPlayerList::Remove(LONG idAccount)
{
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).PlayerInfo.idAccount == idAccount)
		{
			PlayerList.Remove(i);
			return TRUE;
		}
	}

	return FALSE;
}

FLobbyPlayerInfo* FLobbyPlayerList::Find(LONG idAccount)
{
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).PlayerInfo.idAccount == idAccount)
		{
			return &(PlayerList(i));
		}
	}

	return NULL;
}

FLobbyPlayerInfo* FLobbyPlayerList::Find(const FString &PlayerName)
{
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).PlayerInfo.idAccount != ID_INVALID_ACCOUNT && PlayerName == PlayerList(i).PlayerInfo.nickname)
		{
			return &(PlayerList(i));
		}
	}

	return NULL;
}



//////////////////////////////////////////////////////////////////////////////////////////
// FRoomPlayerList

FRoomPlayerInfo* FRoomPlayerList::Add(PLAYER_INFO &Player)
{
	FRoomPlayerInfo *Info = Find(Player.idAccount);

	if (!Info)
	{
		for (INT i = 0; i < MAX_ALL_PLAYER_PER_ROOM; ++i)
		{
			if (IsEmpty(i))
			{
				Info = &(PlayerList[i]);
				break;
			}
		}
	}

	if (Info)
		Info->Set(Player);

	return Info;
}

FRoomPlayerInfo* FRoomPlayerList::Add(PLAYER_DISP_INFO &Player)
{
	FRoomPlayerInfo *Info = Find(Player.idAccount);

	if (!Info)
	{
		for (INT i = 0; i < MAX_ALL_PLAYER_PER_ROOM; ++i)
		{
			if (IsEmpty(i))
			{
				Info = &(PlayerList[i]);
			}
		}
	}

	if (Info)
		Info->Set(Player);

	return Info;
}

FRoomPlayerInfo* FRoomPlayerList::Add(ROOM_PLAYER_INFO &Player)
{
	if (Player.idSlot >= MAX_ALL_PLAYER_PER_ROOM)
		return NULL;

	//if (!IsEmpty(Player.idSlot))
	//	return NULL;

	PlayerList[Player.idSlot].Set(Player);
	if (Player.idAccount == _StateController->PlayerInfo.PlayerInfo.idAccount)
	{
		// it's me
		PlayerList[Player.idSlot].Set(_StateController->PlayerInfo.PlayerInfo);
	}
	return &(PlayerList[Player.idSlot]);
}

FRoomPlayerInfo* FRoomPlayerList::Find(const FString &PlayerName)
{
	for (INT i = 0; i < MAX_ALL_PLAYER_PER_ROOM; ++i)
	{
		if (!IsEmpty(i) && PlayerName == PlayerList[i].PlayerInfo.nickname)
			return &(PlayerList[i]);
	}

	return NULL;
}

FRoomPlayerInfo* FRoomPlayerList::Find(LONG idAccount)
{
	for (INT i = 0; i < MAX_ALL_PLAYER_PER_ROOM; ++i)
	{
		if (PlayerList[i].PlayerInfo.idAccount == idAccount)
			return &(PlayerList[i]);
	}

	return NULL;
}

BOOL FRoomPlayerList::Empty(BYTE idSlot)
{
	if (idSlot < MAX_ALL_PLAYER_PER_ROOM)
	{
		if (!IsEmpty(idSlot))
		{
			//INT Index = StartPlayerList.FindItemIndex(PlayerList[idSlot].PlayerInfo.idAccount);
			//if (Index != INDEX_NONE)
			//	StartPlayerList.Remove(Index);
			//for (INT i = 0; i < StartingPlayerList.Num(); ++i)
			//{
			//	if (StartingPlayerList(i).AccountID == PlayerList[idSlot].PlayerInfo.idAccount)
			//	{
			//		StartingPlayerList.Remove(i);
			//		break;
			//	}
			//}
			PlayerList[idSlot].Clear();
			return TRUE;
		}
	}

	return FALSE;
}



//////////////////////////////////////////////////////////////////////////////////////////
// FChannelInfo


void FChannelInfo::DumpChannelInfo()
{
#if !FINAL_RELEASE
	_DUMP(TEXT("[%d] %s (%d/%d) [%d]"), idChannel, *ChannelName, Count, MaxPlayers, Flag);
#endif
}

// 내 클랜 홈 채널로 설정
void FChannelInfo::SetFromGuild(const FGuildInfo &Guild)
{
	//Idx = (INT)Guild.GuildInfo.idGuild;
	idChannel = ID_MY_CLAN_HOME;//(INT)Guild.GuildInfo.idGuild;
	ChannelName = Guild.GuildInfo.name;
	MaxPlayers = 100;
	Flag = EChannelFlag_MyClan;
	Count = Guild.PlayerList.PlayerList.Num();
	Mask = -1;

	FString NameList = Localize(TEXT("Channel"), *FString::Printf(TEXT("Text_ChannelName[%d]"), (INT)EChannelFlag_MyClan), TEXT("AVANET"));
	FString ShortName, LongName;
	NameList.Split(TEXT("|"), &ShortName, &LongName);
	ChannelNameShort = ShortName;
}

void FChannelInfo::SetChannelName(const TCHAR *InName, INT Idx)
{
	FString NameList = InName;
	if (NameList.Len() == 0)
		NameList = Localize(TEXT("Channel"), *FString::Printf(TEXT("Text_ChannelName[%d]"), Flag), TEXT("AVANET"));

	FString ShortName, LongName;
	NameList.Split(TEXT("|"), &ShortName, &LongName);

	if (Idx > 0)
	{
		ChannelName = LongName + FString::Printf(TEXT(" %d"), Idx);
		ChannelNameShort = ShortName + appItoa(Idx);
	}
	else
	{
		ChannelName = LongName;
		ChannelNameShort = ShortName;
	}
}

FString FChannelInfo::IsJoinableLevel(BYTE ChannelFlag, BYTE Level)
{
	if (!_StateController->PlayerInfo.IsValid())
		return TEXT("not available");

	// 운영자는 제한 없음
	if (_StateController->PlayerInfo.IsAdmin())
		return TEXT("ok");

	switch (ChannelFlag)
	{
	case EChannelFlag_Trainee:
		if (Level >= CHANNEL_LEVEL_THRESHOLD_TRAINEE)
		{
			return TEXT("trainee only");
		}
		break;
	//case EChannelFlag_Sergeant:
	//	if (Level < _LEV_SERGEANT)
	//	{
	//		return TEXT("sergeant required");
	//	}
	//	break;
	//case EChannelFlag_Lieutenant:
	//	if (Level < _LEV_LIEUTENANT)
	//	{
	//		return TEXT("lieutenant required");
	//	}
	//	break;
	//case EChannelFlag_Officer:
	//	if (Level < _LEV_OFFICER)
	//	{
	//		return TEXT("officer required");
	//	}
	//	break;
	//case EChannelFlag_Commander:
	//	if (Level < _LEV_COMMANDER)
	//	{
	//		return TEXT("commander required");
	//	}
	//	break;
	}

	return TEXT("ok");
}

FString FChannelInfo::IsJoinable(BYTE ChannelFlag)
{
	if (!_StateController->PlayerInfo.IsValid())
		return TEXT("not available");

	// 운영자는 제한 없음
	if (_StateController->PlayerInfo.IsAdmin())
		return TEXT("ok");

	FString Result = IsJoinableLevel(ChannelFlag, _StateController->PlayerInfo.PlayerInfo.level);
	if (Result != TEXT("ok"))
		return Result;

	switch (ChannelFlag)
	{
	case EChannelFlag_Newbie:
		if (_StateController->PlayerInfo.PlayerInfo.scoreInfo.GetSDRatio() >= CHANNEL_SDRATIO_THRESHOLD_NEWBIE)
		{
			return TEXT("newbie only");
		}
		break;
	case EChannelFlag_Clan:
		if (!_StateController->GuildInfo.IsValid())
		{
			return TEXT("guild only");
		}
		if (!_StateController->GuildInfo.IsRegularGuild())
		{
			return TEXT("regular guild only");
		}
		break;
	case EChannelFlag_PCBang:
		if (!_StateController->PlayerInfo.IsPcBang())
		{
			return TEXT("pcbang only");
		}
		break;
	case EChannelFlag_Match:
		break;
	case EChannelFlag_Event:
		break;
	case EChannelFlag_Temp:
		break;
	}

	return TEXT("ok");
}

FString FChannelInfo::IsJoinable()
{
	if (!IsValid())
		return TEXT("not available");

	if (!IsJoinableMask())
		return TEXT("no priv");

	return IsJoinable(Flag);
}

UBOOL FChannelInfo::IsJoinableMask()
{
	if (!_StateController->PlayerInfo.IsValid())
		return FALSE;

	// 운영자는 제한 없음
	if (_StateController->PlayerInfo.IsAdmin())
		return TRUE;

	if (!IsMaskTurnedOn())
		return TRUE;
	return (_StateController->PlayerInfo.PlayerInfo.channelMaskList[Mask] > _CML_NONE);
}

//////////////////////////////////////////////////////////////////////////////////////////
// FChannelList

// config에서 채널 목록을 읽어들임.
// 서버에게서 CHANNEL::CHANNEL_DESC_NTF를 받기 전까지 (최초 접속 채널을 결정하기 위해서) 임시로 사용하는 목록임.
INT FChannelList::Set()
{
	_LOG(TEXT("%d channels found from config"), GetAvaNetRequest()->ChannelList.Num());

	if (GetAvaNetRequest()->ChannelList.Num() > 0)
	{
		ChannelList.Empty();

		for (INT i = 0; i < GetAvaNetRequest()->ChannelList.Num(); ++i)
		{
			FavaNetChannelInfo &ChannelInfo = GetAvaNetRequest()->ChannelList(i);
			FChannelInfo *Info = new(ChannelList) FChannelInfo();
			check(Info);

			Info->idChannel = ChannelInfo.idChannel;
			Info->Flag = (INT)ChannelInfo.Flag;
			Info->MaxPlayers = 1000;
			Info->SetChannelName(TEXT(""), ChannelInfo.ChIdx);
		}

		return ChannelList.Num();
	}

	return 0;
}

FChannelInfo* FChannelList::Update(const CHANNEL_DESC &Desc, const FString &InName)
{
	if (Desc.maxPlayers == 0)
	{
		// maxPlayers == 0 이면 목록에서 삭제
		for (INT i = 0; i < ChannelList.Num(); ++i)
		{
			if (ChannelList(i).idChannel == Desc.idChannel)
			{
				ChannelList.Remove(i);
				return NULL;
			}
		}
	}

	FChannelInfo *Info = Find(Desc.idChannel);
	if (!Info)
	{
		Info = new(ChannelList) FChannelInfo();
		check(Info);
		Info->idChannel = Desc.idChannel;
	}

	Info->Flag = Desc.flag;
	Info->Mask = (Desc.mask != UCHAR_MAX ? Desc.mask : -1);
	Info->MaxPlayers = Desc.maxPlayers;
	Info->SetChannelName(*InName, Desc.idx);

	return Info;
}

//FChannelInfo* FChannelList::Add(Def::CHANNEL_INFO &Channel)
//{
//	FChannelInfo *Info = new(ChannelList) FChannelInfo();
//	Info->Idx = Channel.idChannel;
//	Info->Name = FString::Printf(TEXT("A.V.A Channel %d"), Channel.idChannel);
//	Info->Count = Channel.cnt_player;
//
//	return Info;
//}

FChannelInfo* FChannelList::Find(TID_CHANNEL idChannel)
{
	for (INT i = 0; i < ChannelList.Num(); ++i)
		if (ChannelList(i).idChannel == idChannel)
		{
			return &(ChannelList(i));
		}

	return NULL;
}

FChannelInfo* FChannelList::RandomSelect()
{
	if (ChannelList.Num() > 0)
		return &(ChannelList(appRand() % ChannelList.Num()));
	else
		return NULL;
}




//////////////////////////////////////////////////////////////////////////////////////////
// FRoomDispInfo

void FRoomDispInfo::DumpRoomInfo()
{
#if !FINAL_RELEASE
	if (IsValid())
	{
		_DUMP(TEXT("<%d> %s%s (%d/%d) <Host = %s> <Map = %d>%s%s"),
			RoomInfo.idRoom,
			RoomInfo.roomName,
			RoomInfo.bPassword == 0 ? TEXT("") : TEXT(" <*>"),
			RoomInfo.state.numCurr,
			RoomInfo.setting.numMax,
			RoomInfo.hostName,
			RoomInfo.setting.idMap,
			RoomInfo.state.playing == RIP_PLAYING ? TEXT(" <Playing>") : TEXT(""),
			RoomInfo.state.bMatch ? TEXT(" <Match>") : TEXT(""));
	}
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// FRoomInfo

void FRoomInfo::DumpClanInfo()
{
#if !FINAL_RELEASE
	if (IsValid())
	{
		FString ClanName = TEXT("None");
		for (INT i = 0; i < MAX_PLAYER_PER_TEAM; ++i)
		{
			if (PlayerList.PlayerList[i].IsValid())
			{
				if ( appStrlen(PlayerList.PlayerList[i].RoomPlayerInfo.guildName) > 0)
				{
					ClanName = PlayerList.PlayerList[i].RoomPlayerInfo.guildName;
					break;
				}
			}
		}
		_DUMP(TEXT("EU = %s"), *ClanName);

		ClanName = TEXT("None");
		for (INT i = MAX_PLAYER_PER_TEAM; i < MAX_PLAYER_PER_ROOM; ++i)
		{
			if (PlayerList.PlayerList[i].IsValid())
			{
				if ( appStrlen(PlayerList.PlayerList[i].RoomPlayerInfo.guildName) > 0)
				{
					ClanName = PlayerList.PlayerList[i].RoomPlayerInfo.guildName;
					break;
				}
			}
		}
		_DUMP(TEXT("NRF = %s"), *ClanName);
	}
#endif
}

FRoomPlayerInfo* FRoomInfo::AddPlayer(Def::ROOM_PLAYER_INFO &Player)
{
	FRoomPlayerInfo *Info = PlayerList.Add(Player);
	if (Info)
		RefreshPlayerCount();

	return Info;
}

FRoomPlayerInfo* FRoomInfo::AddPlayer(Def::PLAYER_DISP_INFO &Player)
{
	FRoomPlayerInfo *Info = PlayerList.Add(Player);
	if (Info)
		RefreshPlayerCount();

	return Info;
}

FRoomPlayerInfo* FRoomInfo::AddPlayer(Def::PLAYER_INFO &Player)
{
	FRoomPlayerInfo *Info = PlayerList.Add(Player);
	if (Info)
		RefreshPlayerCount();

	return Info;
}

void FRoomInfo::Remove(TID_ACCOUNT idAccount)
{
	FRoomPlayerInfo *Info = PlayerList.Find(idAccount);
	if (Info)
	{
		if ( PlayerList.Empty(Info->RoomPlayerInfo.idSlot) )
		{
			--RoomInfo.state.numCurr;
		}
	}
}

void FRoomInfo::RemoveSlot(BYTE idSlot)
{
	if ( PlayerList.Empty(idSlot) )
	{
		--RoomInfo.state.numCurr;
	}
}

void FRoomInfo::RefreshPlayerCount()
{
	if ( !IsValid() )
		return;

	RoomInfo.state.numCurr = 0;

	INT From = 0;
	INT To = RoomInfo.setting.numMax / 2;

	for (INT i = From; i < To; ++i)
	{
		if (PlayerList.PlayerList[i].IsValid())
			++RoomInfo.state.numCurr;
	}

	From = MAX_PLAYER_PER_TEAM;
	To = From + RoomInfo.setting.numMax / 2;

	for (INT i = From; i < To; ++i)
	{
		if (PlayerList.PlayerList[i].IsValid())
			++RoomInfo.state.numCurr;
	}

}

void FRoomInfo::RefreshClanInfo()
{
	// --> deprecated
	//INT Cnt[RT_MAX];
	//appMemzero(Cnt, RT_MAX * sizeof(INT));

	//// 양 진영의 클랜원 수를 새로 셈
	//for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
	//{
	//	if (!PlayerList.IsEmpty(i))
	//		++Cnt[FRoomInfo::SlotToTeam(i)];
	//}

	//// 한쪽 진영이 비었다면, 그 진영의 클랜 이름을 삭제
	//for (INT i = 0; i < 2; ++i)
	//{
	//	if (Cnt[i] == 0)
	//	{
	//		RoomClanInfo[i].idGuild = ID_INVALID_GUILD;
	//		appStrcpy(RoomClanInfo[i].guildName, TEXT(""));
	//	}
	//}
}

INT FRoomInfo::FindEmptySlot(BYTE idTeam)
{
	if (idTeam == RT_SPECTATOR && RoomInfo.setting.allowSpectator == 0)
		return -1;

	INT from = (idTeam == RT_EU ? 0 :
				idTeam == RT_NRF ? MAX_PLAYER_PER_TEAM :
				idTeam == RT_SPECTATOR ? MAX_PLAYER_PER_ROOM : 0);
	INT to = (idTeam == RT_EU || idTeam == RT_NRF ?
				from + _StateController->RoomInfo.RoomInfo.setting.numMax / 2 - 1 :
				_StateController->ChannelInfo.IsMatchChannel() ? MAX_ALL_PLAYER_PER_ROOM - 1 : MAX_PLAYER_PER_ROOM + 4 - 1);

	for (INT i = from; i <= to; ++i)
	{
		if (_StateController->RoomInfo.PlayerList.IsEmpty(i))
		{
			return i;
		}
	}

	return -1;
}


//////////////////////////////////////////////////////////////////////////////////////////
// FRoomList


FRoomDispInfo* FRoomList::Add(Def::ROOM_INFO &Room)
{
#ifdef _SERVER_PUSH
	if (Room.idRoom < MAX_ROOMS_PER_CHANNEL)
		ListBuf[Room.idRoom] = Room;
#endif

	for (INT i = 0; i < RoomList.Num(); ++i)
	{
		if (RoomList(i).RoomInfo.idRoom == Room.idRoom)
		{
			RoomList(i).RoomInfo = Room;
			return &(RoomList(i));
		}
	}

	FRoomDispInfo* Info = new(RoomList) FRoomDispInfo();
	Info->RoomInfo = Room;

	if (ListIndex < 0)
		ListIndex = 1;
	else if (ListIndex >= RoomList.Num())
		ListIndex = RoomList.Num() - 1;

	return Info;
}

BOOL FRoomList::Remove(WORD idRoom)
{
#ifdef _SERVER_PUSH
	// 서버와 똑같이 동작해야 함
	if (idRoom < MAX_ROOMS_PER_CHANNEL)
		ListBuf[idRoom].Clear();
#endif

	for (INT i = 0; i < RoomList.Num(); ++i)
	{
		if (RoomList(i).RoomInfo.idRoom == idRoom)
		{
			RoomList.Remove(i);

			if (ListIndex < 0)
				ListIndex = 1;
			else if (ListIndex >= RoomList.Num())
				ListIndex = RoomList.Num() - 1;

			return TRUE;
		}
	}

	return FALSE;
}

void FRoomList::Clear()
{
	ListIndex = -1;
	RoomList.Empty();

#ifdef _SERVER_PUSH
	// 서버와 똑같이 동작해야 함
	appMemzero(ListBuf, MAX_ROOMS_PER_CHANNEL * sizeof(ROOM_INFO));
	DiffIndex = 0;
#endif
}

FRoomDispInfo* FRoomList::Find(WORD idRoom)
{
	for (INT i = 0; i < RoomList.Num(); ++i)
	{
		if (RoomList(i).RoomInfo.idRoom == idRoom)
		{
			return &(RoomList(i));
		}
	}

	return NULL;
}

void FRoomList::MergeFromDiff(BYTE *DiffBuf, INT BufSize)
{
	if (BufSize != BUF_MAX_SIZE)
		return;

	// ListBuf를 diff patch
	xor_extract((BYTE*)ListBuf, DiffBuf, BufSize, (BYTE*)ListBuf);

	// state.playing == RIP_NONE 이라면 유효하지 않은 방이다.
	for (INT i = 0; i < MAX_ROOMS_PER_CHANNEL; ++i)
	{
		if ( ((ROOM_INFO*)ListBuf)[i].state.playing == RIP_NONE )
			// 유효하지 않은 방은 삭제
			_StateController->RoomList.Remove(i);
		else
			// 유효한 방은 최신 정보로 업데이트
			_StateController->RoomList.Add( ((ROOM_INFO*)ListBuf)[i] );
	}
}



//////////////////////////////////////////////////////////////////////////////////////////
// FLastResultInfo

void FLastResultInfo::FPlayerResultInfo::Clear()
{
	idSlot = -1;
	Level = -1;
	LastLevel = -1;
	Nickname = TEXT("???");
	Score = 1;
	Death = 1;
	xp = 1;
	SupplyPoint = 1;
	bLeader = FALSE;
	bPcBang = FALSE;
	biaXPFlag = _BIAXP_NONE;
	EffectList.Empty();
}

void FLastResultInfo::FPlayerResultInfo::AddEffect(TID_ITEM idItem)
{
	if (idItem == ID_INVALID_ITEM)
		return;

	EFFECT_ITEM_DESC *pDesc = _ItemDesc().GetEffectItem(idItem);
	if (!pDesc)
	{
		_LOG(TEXT("Effect info of the item[%d] is not found."), idItem);
	}

	INT Index = 0;
	for (INT i = 0; i < EffectList.Num(); ++i)
	{
		TID_ITEM id = EffectList(i);
		if (idItem == id)
		{
			_LOG(TEXT("Same item already exists in the list!"));
			return;
		}
		if (idItem < id)
		{
			Index = i;
			break;
		}

	}

	if (Index >= EffectList.Num())
		EffectList.Push(idItem);
	else
		EffectList.InsertItem(idItem, Index);
}

INT FLastResultInfo::FPlayerResultInfo::GetBonusXPPerc()
{
	INT Perc = 0;
	//if (!_StateController->ChannelInfo.IsMyClanChannel())
	if (_StateController->GetChannelSetting(EChannelSetting_AllowBoostItem) > 0)
	{
		for (INT x = 0; x < EffectList.Num(); ++x)
		{
			EFFECT_ITEM_DESC *pDesc = _ItemDesc().GetEffectItem(EffectList(x));
			if (pDesc && pDesc->effectInfo.effectType == _IET_EXP_BOOST)
			{
				Perc += pDesc->effectInfo.effectValue;
			}
		}
#ifdef _HAPPY_NEW_YEAR_EVENT
		if (_StateController->GetChannelSetting(EChannelSetting_AllowEventBonus) > 0)
			Perc += 30;
#endif
	}
	return Perc;
}

INT FLastResultInfo::FPlayerResultInfo::GetPcBangBonusXPPerc()
{
	return _StateController->GetChannelSetting(EChannelSetting_AllowPCBangBonus) > 0 && bPcBang ? 30 : 0;
}

INT FLastResultInfo::FPlayerResultInfo::GetBonusSupplyPerc()
{
	INT Perc = 0;
	//if (!_StateController->ChannelInfo.IsMyClanChannel())
	if (_StateController->GetChannelSetting(EChannelSetting_AllowBoostItem) > 0)
	{
		for (INT x = 0; x < EffectList.Num(); ++x)
		{
			EFFECT_ITEM_DESC *pDesc = _ItemDesc().GetEffectItem(EffectList(x));
			if (pDesc && pDesc->effectInfo.effectType == _IET_SP_BOOST)
			{
				Perc += pDesc->effectInfo.effectValue;
			}
		}
	}
	return Perc;
}

FLastResultInfo::FPlayerResultInfo* FLastResultInfo::FindPlayer(INT Slot)
{
	for (INT i = 0; i < RoomResultInfo.Num(); ++i)
	{
		if (RoomResultInfo(i).idSlot == Slot)
			return &RoomResultInfo(i);
	}

	return NULL;
}

void FLastResultInfo::Clear()
{
	_LOG(TEXT(">>>>>>>>>>>>>>>>>>> Clearing last result info"));
	InfoLevel = 0;
	//bNeedInfo = FALSE;
	appMemzero(&PlayerResultInfo, sizeof(PlayerResultInfo));
	PlayerResultInfo.idAccount = ID_INVALID_ACCOUNT;
	PlayerResultInfo.gameWin = 1;
	Level = 0;
	xp = 0;
	SupplyPoint = 0;
	SupplyCount = 0;
	Money = 0;
	BIAFlag = _BIAXP_NONE;
	for (INT i = 0; i < _CLASS_MAX; ++i)
		SkillList[i].Empty();
	AwardList.Empty();
	RoomResultInfo.Empty();
	LastResultMsgData.Empty();
}

void FLastResultInfo::Sort()
{
	if (RoomResultInfo.Num() == 0)
		return;

	TArray<FPlayerResultInfo> Orig = RoomResultInfo;
	RoomResultInfo.Empty();

	for (INT i = 0; i < Orig.Num(); ++i)
	{
		INT Index = 0;
		for ( ; Index < RoomResultInfo.Num(); ++Index)
		{
			// score를 기준으로 정렬
			if (Orig(i).Score > RoomResultInfo(Index).Score)
				break;
			if (Orig(i).Score == RoomResultInfo(Index).Score)
			{
				// score가 같을 경우 death를 기준으로 2차 정렬
				if (Orig(i).Death < RoomResultInfo(Index).Death)
					break;
				if (Orig(i).Death == RoomResultInfo(Index).Death)
				{
					if (Orig(i).idSlot < RoomResultInfo(Index).idSlot)
						break;
				}
			}
		}

		FPlayerResultInfo *Result = NULL;
		if (Index >= RoomResultInfo.Num())
			Result = new(RoomResultInfo) FPlayerResultInfo;
		else
			Result = new(RoomResultInfo, Index) FPlayerResultInfo;
		check(Result);

		(*Result) = Orig(i);
	}
}

void FLastResultInfo::Dump()
{
#if !FINAL_RELEASE
	if (InfoLevel == 0)
	{
		_DUMP(TEXT("LastResultInfo: Nothing to dump"));
		return;
	}

	_DUMP(TEXT("Dumping LastResultInfo..."));
	_DUMP(TEXT("EU = %d, NRF = %d"), TeamScore[0], TeamScore[1]);
	_DUMP(TEXT("Level = %+d; XP = %+d; SupplyPoint = %+d, SupplyCount = %+d, Money = %+d, BIAFlag = %d"), Level, xp, SupplyPoint, SupplyCount, Money, BIAFlag);
	for (INT i = 0; i < LastResultMsgData.Num(); ++i)
	{
		_DUMP(TEXT("[%d]ResultMsgType = %d, Value = %d"), (INT)LastResultMsgData(i).MsgType, LastResultMsgData(i).Variation);
	}

	if (InfoLevel > 1)
	{
		for (INT i = 0; i < RoomResultInfo.Num(); ++i)
		{
			if (RoomResultInfo(i).IsValid())
			{
				FString EffItems;
				for (INT j = 0; j < RoomResultInfo(i).EffectList.Num(); ++j)
				{
					EFFECT_ITEM_DESC *Desc = _ItemDesc().GetEffectItem(RoomResultInfo(i).EffectList(j));
					if (Desc)
					{
						if (EffItems.Len() > 0)
							EffItems += TEXT(",");
						EffItems += Desc->GetName();
					}
				}

				_DUMP(TEXT("[%d] NickName = %s; Score = %d; Death = %d; XP = %+d, SupplyPoint = %+d, PCBang = %d, BIAFlag = %d, EffectItems = %s"),
					RoomResultInfo(i).idSlot, *RoomResultInfo(i).Nickname, RoomResultInfo(i).Score, RoomResultInfo(i).Death,
					RoomResultInfo(i).xp, RoomResultInfo(i).SupplyPoint, RoomResultInfo(i).bPcBang, RoomResultInfo(i).biaXPFlag, *EffItems);
			}
		}
	}
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// FChatMsgList

FString FChatMsg::GetAddedTime(UBOOL bAddDate)
{
	return _StateController->GetTimeFromAppSec(AddedTime, bAddDate);
}

void FChatMsgList::Dump()
{
#if !FINAL_RELEASE
	for (TStringList::TIterator It(ChatList.GetHead()); It; ++It)
	{
		_DUMP(**It);
	}
#endif
}


void FChatMsgList::Add(const FString &Msg, const INT MsgType)
{
	FString TypedMsg = FString::Printf(TEXT("[%d]%s"), MsgType, *Msg);

	FChatMsg MsgElem;
	MsgElem.Msg = TypedMsg;
	MsgElem.AddedTime = appSeconds();

	ChatList.AddTail(MsgElem);
	if (ChatList.Num() > 200)
		ChatList.RemoveNode(ChatList.GetHead());
}

void FChatMsgList::Add(const TCHAR *Msg, const INT MsgType)
{
	FString Str(Msg);
	Add(Str, MsgType);
}

void FChatMsgList::AddWithPrefix(const FString &Msg, const INT MsgType)
{
	FString Prefix = Localize(TEXT("ChatTypeLabel"), *FString::Printf(TEXT("Text_ChatPrefix[%d]"), MsgType), TEXT("AVANET"));
	Add(Prefix + Msg, MsgType);
}

void FChatMsgList::AddWithPrefix(const TCHAR *Msg, const INT MsgType)
{
	FString Prefix = Localize(TEXT("ChatTypeLabel"), *FString::Printf(TEXT("Text_ChatPrefix[%d]"), MsgType), TEXT("AVANET"));
	Add(Prefix + Msg, MsgType);
}

void FChatMsgList::Clear()
{
	ChatList.Clear();
}



//////////////////////////////////////////////////////////////////////////////////////////
// FGuildPlayerInfo

void FGuildPlayerInfo::Set(Def::GUILD_PLAYER_INFO &Player)
{
	if (Player.idAccount != ID_INVALID_ACCOUNT)
	{
		appMemcpy(&GuildPlayerInfo, &Player, sizeof(GUILD_PLAYER_INFO));
		idChannel = Player.idChannel;
	}
}

void FGuildPlayerInfo::Set(Def::PLAYER_INFO &Player)
{
	if (Player.idAccount != ID_INVALID_ACCOUNT)
	{
		GuildPlayerInfo = Player;

		GameWin = Player.scoreInfo.gameWin;
		GameDefeat = Player.scoreInfo.gameDefeat;
		DisconnectCount = Player.scoreInfo.disconnectCount;
		KillCount = Player.scoreInfo.GetKillCount();
		DeathCount = Player.scoreInfo.deathCount;
		Score = Player.scoreInfo.score;
		SetFullInfo(TRUE);
	}
}

void FGuildPlayerInfo::Set(Def::PLAYER_DISP_INFO &Player)
{
	if (Player.idAccount != ID_INVALID_ACCOUNT)
	{
		GuildPlayerInfo.idAccount = Player.idAccount;
		appStrcpy(GuildPlayerInfo.nickname, Player.nickname);
		GuildPlayerInfo.level = Player.level;

		idRoom = Player.idRoom;

		GameWin = Player.gameWin;
		GameDefeat = Player.gameDefeat;
		DisconnectCount = Player.disconnectCount;
		Score = Player.scoreInfo;
		KillCount = Player.killCount;
		DeathCount = Player.deathCount;
		SetFullInfo(TRUE);
	}
}

void FGuildPlayerInfo::UpdateNickname(const FString &Nickname, UBOOL bShowMsg)
{
	if (Nickname != GuildPlayerInfo.nickname)
	{
		_LOG(TEXT("Nickname changed; [%d]%s -> %s"), GuildPlayerInfo.idAccount, GuildPlayerInfo.nickname, *Nickname);

		if (bShowMsg)
			_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Buddy_UpdateNickname"), TEXT("AVANET")),
																GuildPlayerInfo.nickname, *Nickname),
											EChat_GuildSystem);
		appStrncpy(GuildPlayerInfo.nickname, *Nickname, SIZE_NICKNAME+1);

		GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_MemberList, TEXT("ok"), TEXT(""), 0, 0);
	}
}

FString FGuildPlayerInfo::GetLocation()
{
	if (!IsValid())
		return TEXT("-");
	return _StateController->GetLocationString(IsOnline(), idChannel, idRoom);
}

void FGuildPlayerInfo::DumpPlayerInfo()
{
#if !FINAL_RELEASE
	if (IsFullInfo())
	{
		_DUMP(TEXT("[%d][%d]%s; Rank/Group = <%d/%d> %s, %d/%d/%d, %d/%d"),
			GuildPlayerInfo.idAccount,
			GuildPlayerInfo.level,
			GuildPlayerInfo.nickname,
			GuildPlayerInfo.idRank,
			GuildPlayerInfo.idGroup,
			IsOnline() ? TEXT("Online") : TEXT("Offline"),
			GameWin,
			GameDefeat,
			DisconnectCount,
			KillCount,
			DeathCount
			);
	}
	else
	{
		_DUMP(TEXT("[%d]%s; Rank/Group = <%d/%d> %s, --- "),
			GuildPlayerInfo.level,
			GuildPlayerInfo.nickname,
			GuildPlayerInfo.idRank,
			GuildPlayerInfo.idGroup,
			IsOnline() ? TEXT("Online") : TEXT("Offline"));
	}
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////
// FGuildPlayerList

FGuildPlayerInfo* FGuildPlayerList::Add(Def::GUILD_PLAYER_INFO &Player)
{
	FGuildPlayerInfo *Info = NULL;

	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).GuildPlayerInfo.idAccount == Player.idAccount)
		{
			Info = &(PlayerList(i));
			break;
		}
	}

	if (!Info)
		Info = new(PlayerList) FGuildPlayerInfo();

	check(Info);

	Info->Set(Player);

	return Info;
}

FGuildPlayerInfo* FGuildPlayerList::Add(Def::PLAYER_INFO &Player)
{
	FGuildPlayerInfo *Info = NULL;

	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).GuildPlayerInfo.idAccount == Player.idAccount)
		{
			Info = &(PlayerList(i));
			break;
		}
	}

	if (!Info)
		Info = new(PlayerList) FGuildPlayerInfo();

	check(Info);

	Info->Set(Player);

	return Info;
}

FGuildPlayerInfo* FGuildPlayerList::Add(Def::PLAYER_DISP_INFO &Player)
{
	FGuildPlayerInfo *Info = NULL;

	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).GuildPlayerInfo.idAccount == Player.idAccount)
		{
			Info = &(PlayerList(i));
			break;
		}
	}

	if (!Info)
		Info = new(PlayerList) FGuildPlayerInfo();

	check(Info);

	Info->Set(Player);

	return Info;
}

UBOOL FGuildPlayerList::Remove(LONG idAccount)
{
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).GuildPlayerInfo.idAccount == idAccount)
		{
			PlayerList.Remove(i);
			return TRUE;
		}
	}

	return FALSE;
}

FGuildPlayerInfo* FGuildPlayerList::Find(LONG idAccount)
{
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerList(i).GuildPlayerInfo.idAccount == idAccount)
		{
			return &PlayerList(i);
		}
	}

	return NULL;
}

FGuildPlayerInfo* FGuildPlayerList::Find(const FString &PlayerName)
{
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		if (PlayerName == PlayerList(i).GuildPlayerInfo.nickname)
		{
			return &PlayerList(i);
		}
	}

	return NULL;
}

void FGuildPlayerList::DumpToConsole(INT DumpFlag)
{
	INT Cnt = 0;
	for (INT i = 0; i < PlayerList.Num(); ++i)
	{
		FGuildPlayerInfo &Info = PlayerList(i);

		if ( (DumpFlag == _DF_ON && !Info.IsOnline()) ||
			(DumpFlag == _DF_OFF && Info.IsOnline()) )
			continue;

		_StateController->LogChatConsole( *FString::Printf(TEXT("%s - %s"),
			PlayerList(i).GuildPlayerInfo.nickname,
			PlayerList(i).IsOnline() ? TEXT("Online") : TEXT("Offline")),
			EChat_GuildSystem
			);

		++Cnt;
	}

	_StateController->LogChatConsole(*FString::Printf(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_MemberList"), TEXT("AVANET")), Cnt),
									EChat_GuildSystem);
}


//////////////////////////////////////////////////////////////////////////////////////////
// FGuildInfo

void FGuildInfo::Clear()
{
	GuildInfo.idGuild = ID_INVALID_GUILD;
	SetFullInfo(FALSE);
	PlayerList.Clear();
}

void FGuildInfo::Set(PLAYER_GUILD_INFO &Info)
{
	GuildInfo.idGuild = Info.idGuild;
	GuildInfo.level = Info.guildLevel;
	appStrcpy(GuildInfo.name, Info.guildName);
}

void FGuildInfo::SetMotd(const TCHAR *Motd)
{
	appStrncpy(GuildInfo.motd, Motd, SIZE_GUILD_MOTD + 1);
}

void FGuildInfo::SyncPlayerInfo(FPlayerDispInfo &Player)
{
	if (!IsValid())
		return;

	FGuildPlayerInfo *Info = PlayerList.Find(Player.PlayerInfo.idAccount);
	if (Info)
	{
		if (Info->InfoTimeStamp > 0.0 && Info->InfoTimeStamp > Player.InfoTimeStamp)
			return;

		Info->Set(Player.PlayerInfo);

		//Info->idRoom = Player.idRoom;
		//Info->GameWin = Player.PlayerInfo.gameWin;
		//Info->GameDefeat = Player.PlayerInfo.gameDefeat;
		//Info->Score = Player.PlayerInfo.scoreInfo;
		//Info->KillCount = Player.PlayerInfo.killCount;
		//Info->DeathCount = Player.PlayerInfo.deathCount;
		//Info->DisconnectCount = Player.PlayerInfo.disconnectCount;
		//Info->GuildPlayerInfo.level = Player.PlayerInfo.level;
		Info->SetFullInfo(TRUE);
	}
}

void FGuildInfo::SyncPlayerInfo(PLAYER_DISP_INFO &Player)
{
	if (!IsValid())
		return;

	FGuildPlayerInfo *Info = PlayerList.Find(Player.idAccount);
	if (Info)
	{
		// Player의 타임스탬프를 알 수 없으므로, Player가 무조건 최신이라고 믿고 sync 맞춤
		Info->Set(Player);

		//Info->idRoom = Player.idRoom;
		//Info->GameWin = Player.gameWin;
		//Info->GameDefeat = Player.gameDefeat;
		//Info->Score = Player.scoreInfo;
		//Info->KillCount = Player.killCount;
		//Info->DeathCount = Player.deathCount;
		//Info->DisconnectCount = Player.disconnectCount;
		//Info->GuildPlayerInfo.level = Player.level;
		Info->SetFullInfo(TRUE);
	}
}

void FGuildInfo::SyncPlayerInfo(Def::TID_ACCOUNT idAccount, Def::TID_CHANNEL idChannel)
{
	if (!IsValid())
		return;

	FGuildPlayerInfo *Info = PlayerList.Find(idAccount);
	if (Info)
	{
		Info->GuildPlayerInfo.idChannel = idChannel;
		Info->idChannel = idChannel;
	}
}

void FGuildInfo::SyncPlayerInfo(Def::TID_ACCOUNT idAccount, Def::TID_CHANNEL idChannel, Def::TID_ROOM idRoom)
{
	if (!IsValid())
		return;

	FGuildPlayerInfo *Info = PlayerList.Find(idAccount);
	if (Info)
	{
		Info->GuildPlayerInfo.idChannel = idChannel;
		Info->idChannel = idChannel;
		Info->idRoom = idRoom;
		Info->SetLocationUpdated();
	}
}

INT FGuildInfo::GetClanMarkID()
{
	if ( !IsValid() )
		return -1;

	return appStrlen(GuildInfo.name) > 0 ? GuildInfo.idGuildMark : -1;
}

void FGuildInfo::Dump()
{
#if !FINAL_RELEASE
	if (IsValid())
	{
		_DUMP(TEXT("Guild Info ="));
		_DUMP(TEXT("[%d] %s <%d/%d> %d points, %d members. Master = [%d]%s; RegDate = %s"),
			GuildInfo.idGuild,
			GuildInfo.name,
			GuildInfo.level,
			GuildInfo.xp,
			GuildInfo.income,
			GuildInfo.cntMember,
			GuildInfo.idMaster,
			*GetMasterName(),
			GuildInfo.regdate);
		_DUMP(TEXT("MOTD = %s"), GuildInfo.motd);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// FMapInfo

void FMapInfo::Dump()
{
#if !FINAL_RELEASE
	_DUMP(TEXT("[%d]%s = {"),idMap, *MapName);
	_DUMP(TEXT("InFileName = %s"),*FileName);
	_DUMP(TEXT("bHidden = %s"),bHidden ? GTrue : GFalse);
	_DUMP(TEXT("bStatLog = %s"),bStatLog ? GTrue : GFalse);
	_DUMP(TEXT("MissionType = %d"),MissionType);
	_DUMP(TEXT("}"));
#endif
}

FMapInfo::FMapInfo( FavaNetMapInfo* MapInfo, FavaNetMissionInfo* MissionInfo )
{
	extern UBOOL GIsRedduckInternal;

	check(MapInfo);
	idMap = MapInfo->MapID;
	ImagePathName = MapInfo->ImagePathName;
	MapName = MapInfo->MapName;
	Description = MapInfo->Description;
	FileName = MapInfo->MapFileName;
	ExclChannelGroups = MapInfo->ExclChannelGroups;
	bStatLog = MapInfo->bStatLog;
	AllowMapOption = MapInfo->AllowMapOption;
	bHidden = MapInfo->bRedduckInternalOnly && !GIsRedduckInternal;

	UBOOL bHasDefaultMaxPlayer = FALSE;;
	TArray<FString> ParsedStrings;
	// 맵정보에 MaxPlayers가 없을 경우 맵정보에서 MaxPlayers를 꺼내온다
	FString MaxPlayers = MapInfo->MaxPlayers.Len() == 0 && MissionInfo != NULL ? MissionInfo->MaxPlayers : MapInfo->MaxPlayers;
	MaxPlayers.ParseIntoArrayWS( &ParsedStrings, TEXT(","));
	for( INT i = 0 ; i < ParsedStrings.Num() ; i++ )
	{
		FString MaxPlayerStr = ParsedStrings(i);
		if( MaxPlayerStr.InStr(TEXT("*")) >= 0)
		{
			MaxPlayerStr = MaxPlayerStr.Replace(TEXT("*"),TEXT(""));
			DefaultMaxPlayer = appAtoi(*MaxPlayerStr);
			bHasDefaultMaxPlayer = TRUE;
		}
		MaxPlayerList.AddItem(appAtoi(*MaxPlayerStr));
	}
	if( ! bHasDefaultMaxPlayer && MaxPlayerList.Num() > 0)
		DefaultMaxPlayer = MaxPlayerList( MaxPlayerList.Num() / 2 );

	DefaultWinCond = 0;
	UBOOL bHasDefaultWinCond = FALSE;

	// 맵정보에 WinCondition이 없을경우 미션정보에서 WinCondition을 꺼내온다
	FString WinCondition = MapInfo->WinCondition.Len() == 0 && MissionInfo != NULL ? MissionInfo->WinCondition : MapInfo->WinCondition;
	WinCondition.ParseIntoArrayWS( &ParsedStrings, TEXT(","));
	for( INT i = 0 ; i < ParsedStrings.Num() ; i++ )
	{
		FString WinCondStr = ParsedStrings(i);
		if( WinCondStr.InStr(TEXT("*")) >= 0 )
		{
			WinCondStr = WinCondStr.Replace(TEXT("*"),TEXT(""));
			DefaultWinCond = appAtoi(*WinCondStr);
			bHasDefaultWinCond = TRUE;
		}
		WinCondList.AddItem(appAtoi(*WinCondStr));
	}
	if( ! bHasDefaultWinCond && WinCondList.Num() > 0 )
		DefaultWinCond = WinCondList( WinCondList.Num() / 2 );

	WinCondType = MapInfo->WinConditionType;
	MissionType = MapInfo->MissionType;
}

//////////////////////////////////////////////////////////////////////////////////////////
// FMapList

INT FMapList::Set()
{
	MapList.Empty();

	if (GetAvaNetRequest()->MapList.Num() > 0)
	{
		for (INT i = 0; i < GetAvaNetRequest()->MapList.Num(); ++i)
		{
			FavaNetMapInfo* MapInfo = &GetAvaNetRequest()->MapList(i);
			FavaNetMissionInfo* MissionInfo = NULL;
			for( INT MissionIndex = 0 ; MissionIndex < GetAvaNetRequest()->MissionList.Num() ; MissionIndex++ )
			{
				FavaNetMissionInfo& Mission = GetAvaNetRequest()->MissionList(MissionIndex);
				if( Mission.MissionType == MapInfo->MissionType )
				{
					MissionInfo = &Mission;
					break;
				}
			}

			//UBOOL bHidden = MapInfo.bRedduckInternalOnly && !GIsRedduckInternal;
			Add(MapInfo,MissionInfo);
		}

		return MapList.Num();
	}

	return 0;
}

FMapInfo* FMapList::Add( FavaNetMapInfo* MapInfo, FavaNetMissionInfo* MissionInfo )
{
	check(MapInfo);
	FMapInfo *Info = Find(MapInfo->MapID);
	if (Info)
		return Info;

	Info = new(MapList) FMapInfo( MapInfo, MissionInfo);

	return Info;
}


//////////////////////////////////////////////////////////////////////////////////////////
// FPopUpMsgList

//void FPopUpMsgList::PushPopUpMsg(BYTE MsgType, const FString &PopUpMsg, const FString &NextScene, FName NextUIEventName )
//{
//	FPopUpMsgInfo *Info = new(Msgs) FPopUpMsgInfo;
//	check(Info);
//
//	Info->MsgType = MsgType;
//	appStrncpy(Info->PopUpMsg, *PopUpMsg, 128);
//	appStrncpy(Info->NextScene, *NextScene, 32);
//	Info->NextUIEventName = NextUIEventName;
//}
//
//UBOOL FPopUpMsgList::PopFirstPopUpMsg(FPopUpMsgInfo &Info)
//{
//	if (Msgs.Num() > 0)
//	{
//		Info.MsgType = Msgs(0).MsgType;
//		appStrcpy(Info.PopUpMsg, Msgs(0).PopUpMsg);
//		appStrcpy(Info.NextScene, Msgs(0).NextScene);
//		Info.NextUIEventName = Msgs(0).NextUIEventName;
//		Msgs.Remove(0);
//
//		return TRUE;
//	}
//	else
//	{
//		return FALSE;
//	}
//}
//
//UBOOL FPopUpMsgList::PopFirstPopUpMsg(BYTE& MsgType, FString& PopUpMsg, FString& NextScene, FName& NextUIEventName )
//{
//	if (Msgs.Num() > 0)
//	{
//		MsgType = Msgs(0).MsgType;
//		PopUpMsg = Msgs(0).PopUpMsg;
//		NextScene = Msgs(0).NextScene;
//		NextUIEventName = Msgs(0).NextUIEventName;
//		Msgs.Remove(0);
//
//
//		return TRUE;
//	}
//	else
//	{
//		return FALSE;
//	}
//}



//////////////////////////////////////////////////////////////////////////////////////////
// CavaNetStateController


void CavaNetStateController::Init()
{
	BaseSec = appSeconds();
	_time32(&BaseTime);

	if (ChannelList.Set() == 0)
	{
		_LOG(TEXT("Error! Failed to load channel list"));
		LastConnectResult = TEXT("no resource");
	}

	if (MapList.Set() == 0)
	{
		_LOG(TEXT("Error! Failed to load map list"));
		LastConnectResult = TEXT("no resource");
	}

	MapList.Dump();

	_ItemDesc();
	_ShopDesc();
	_WordCensor();
	_TickerMsg();
	_RoomName();
	_AwardDesc();
	_SkillDesc();
	GUILD_INFO::InitPrivInfo();

	if (!_ItemDesc().IsInit() || !_ShopDesc().IsInit() || !_WordCensor().IsInit() || /*!_TickerMsg().IsInit() ||*/
		 !_RoomName().IsInit() ||!_AwardDesc().IsInit() || !_SkillDesc().IsInit())
	{
		_LOG(TEXT("Failed to read resource files."));
		LastConnectResult = TEXT("no resource");
	}

	//-- chatting commands list
	// 엔진이 ANSICHAR로 저장되어 있는 DefaultNet.ini를 유니코드(한 character를 unsigned short로 취급)인 것처럼 읽어들이면서
	// 모든 2바이트 한글이 4바이트로 저장됨.
	// 이를 다시 2바이트 유니코드로 변환해서 저장해둠.
	TArray<FString> CmdList;
	GConfig->GetArray(TEXT("ChatCommandList"), TEXT("ChatCommand"), CmdList, GNetIni);
	for (INT i = 0; i < CmdList.Num(); ++i)
	{
		FavaChatCommandInfo CmdInfo;
		FString &Cmd = CmdList(i);
		INT Pos = 0, Len = Cmd.Len();

		if (Cmd[0] == TEXT('('))
		{
			++Pos;
			--Len;
		}
		if (Cmd[Cmd.Len()-1] == TEXT(')'))
		{
			--Len;
		}
		if ((Pos > 0 || Len < Cmd.Len()) && Len > 0)
			Cmd = Cmd.Mid(Pos, Len);

		TArray<FString> ParamList;
		Cmd.ParseIntoArray(&ParamList, TEXT(","), TRUE);
		for (INT n = 0; n < ParamList.Num(); ++n)
		{
			FString &Param = ParamList(n);
			FString Key, Val;
			Param = Param.Trim().TrimTrailing();
			if (!Param.Split(TEXT("="), &Key, &Val))
				continue;
			Key = Key.Trim().TrimTrailing();
			Val = Val.Trim().TrimTrailing().TrimQuotes();
			if (Key == TEXT("CmdType"))
			{
				CmdInfo.CmdType = FavaChatCommandInfo::GetCmdType(Val);
			}
			else if (Key == TEXT("ParentCmd"))
			{
				CmdInfo.ParentCmd = FavaChatCommandInfo::GetCmdType(Val);
			}
			else if (Key == TEXT("CmdChar"))
			{
				BYTE buf[16];
				Val.ParseIntoArray(&CmdInfo.CmdChar, TEXT("|"), TRUE);
				for (INT k = 0; k < CmdInfo.CmdChar.Num(); ++k)
				{
					appMemzero(buf, 16);
					FString &Str = CmdInfo.CmdChar(k);
					BYTE *tr = (BYTE*)*Str;
					for (INT x = 0; x < Str.Len() && x < 16; ++x)
					{
						buf[x] = *tr;
						tr += 2;
					}
					Str = ANSI_TO_TCHAR((TCHAR*)buf);
				}
			}
		}

		if (CmdInfo.CmdType != ECmd_None && CmdInfo.CmdChar.Num() > 0)
			_Locals.ChatCommandList.Push(CmdInfo);

		//FString CmdCharList = (CmdInfo.CmdChar.Num() > 0 ? CmdInfo.CmdChar(0) : TEXT(""));
		//for (INT x = 1; x < CmdInfo.CmdChar.Num(); ++x)
		//	CmdCharList += FString::Printf(TEXT(",%s"), *CmdInfo.CmdChar(x));
		//_LOG(TEXT("CmdType = %d, ParentCmd = %d, CmdChar = %s"), (INT)CmdInfo.CmdType, (INT)CmdInfo.ParentCmd, *CmdCharList);
	}

	//for (INT i = 0; i < GetAvaNetRequest()->ChannelSettingList.Num(); ++i)
	//{
	//	FavaNetChannelSettingInfo &Info = GetAvaNetRequest()->ChannelSettingList(i);
	//	_LOG(TEXT("ChannelSettingInfo(%d): Flag = %d, SettingAutoBalance = %d, DefaultAutoBalance = %d, SettingMaxPlayer = %d, DefaultMaxPlayer = %d"),
	//			i, Info.Flag, Info.EnableAutoBalance, Info.DefaultAutoBalance, Info.EnableMaxPlayers, Info.DefaultMaxPlayers);
	//}
}

void CavaNetStateController::Final()
{
	//if ( !TickerSave() )
	//{
	//	_LOG(TEXT("Error! Failed to save ticker message list"));
	//}
	_WebInClient().ChannelUnset();
}

// 1초에 한 번씩 불림
void CavaNetStateController::Tick()
{
	// 대기실에서의 강퇴 조건 체크
	if (!RoomInfo.IsValid())
		return;
	if (AmISpectator() || AmIStealthMode())
		return;

	DOUBLE Now = appSeconds();

	if (RoomHostKickCount >= 0)
	{
		if (NetState == _AN_ROOM && !IsCountingDown())
		{
			switch (RoomHostKickCount)
			{
			case 20:
				_LOG(TEXT("Host-kick count down is triggered."));
				break;
			case 10:
				if ( AmIHost() )
				{
					LogChatConsole(*Localize(TEXT("UIRoomScene"), TEXT("Msg_HostKickIn10"), TEXT("AVANET")), EChat_PlayerSystem);
				}
				break;
			case 5:
				{
					FString Nickname;
					if (!RoomInfo.PlayerList.IsEmpty(RoomInfo.HostIdx))
						Nickname = RoomInfo.PlayerList.PlayerList[RoomInfo.HostIdx].RoomPlayerInfo.nickname;
					else
						Nickname = TEXT("????");
					LogChatConsole(*FString::Printf(*Localize(TEXT("UIRoomScene"), TEXT("Msg_HostKickIn5"), TEXT("AVANET")), *Nickname), EChat_PlayerSystem);
				}
				break;
			case 4:
			case 3:
			case 2:
			case 1:
				{
					LogChatConsole(*FString::Printf(*Localize(TEXT("UIRoomScene"), TEXT("Msg_HostKickCount"), TEXT("AVANET")), RoomHostKickCount), EChat_PlayerSystem);
				}
				break;
			case 0:
				if (AmIHost())
				{
					_LOG(TEXT("You are host and didn't start the game while all are ready, so leaving the room"));
					GetAvaNetRequest()->LeaveRoom(ELR_HostDidntStart);
				}
				break;
			}
			--RoomHostKickCount;
		}
		else
		{
			RoomHostKickCount = -1;
		}
	}

	if (RoomReadyDue > 0.0)
	{
		if (NetState == _AN_ROOM && !AmIHost())
		{
			// 난입 허용되지 않은 상태에서 게임이 진행 중이라면 패스
			if ( !(!RoomInfo.RoomInfo.setting.allowInterrupt && RoomInfo.IsPlaying()) )
			{
				FRoomPlayerInfo *Info = GetMyRoomPlayerInfo();
				if (Info && !Info->IsReady() && !Info->IsPlaying())
				{
					DOUBLE Remain = RoomReadyDue - Now;
					if (Remain <= 0.0)
					{
						_LOG(TEXT("You didn't ready in 30 seconds, so leaving the room"));

						GetAvaNetRequest()->LeaveRoom(ELR_DidntReady);
					}
					else if (Remain <= 30.0 && RoomReadyKickCount == 2)
					{
						_LOG(TEXT("If you don't ready in 30 seconds, you will leave the room"));

						RoomReadyKickCount = 1;
						//GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Kick, TEXT("warning"), TEXT("ready in 30sec"), 0, 0);
						LogChatConsole(*Localize(TEXT("UIPopUpMsg"), TEXT("Msg_Room_Kicked_ReadyIn30"), TEXT("AVANET")), EChat_PlayerSystem);

						return;
					}
					else
					{
						return;
					}
				}
			}
		}

		RoomReadyDue = 0.0;
		RoomReadyKickCount = 0;
	}
}

UBOOL CavaNetStateController::LeavingState(INT NewState)
{
	switch (NetState)
	{
	case _AN_DISCONNECTED:
		break;
	case _AN_CONNECTING:
		break;
	case _AN_CHANNELLIST:
		break;
	case _AN_LOBBY:
		ClearLobbyInfo();
		_Communicator().EndBuddyList();
		break;
	case _AN_ROOM:
		ChatMsgList.Clear();
		_Communicator().EndBuddyList();
		RoomReadyDue = 0.0;
		RoomReadyKickCount = 0;
		if (NewState == _AN_LOBBY)
		{
			ClearRoomInfo();
#ifdef EnableRttTest
			//GavaNetClient->chsImpl.StopTest();
			_LOG(TEXT("[dEAthcURe] deactivating RTT test..."));
			GavaNetClient->p2pNetTester.activateRttTest(false); // 20070517 dEAthcURe|RTT // GavaNetClient->p2pNetTester.setActive(false);
#endif
		}
		break;
	case _AN_INGAME:
		if (NewState == _AN_LOBBY)
		{
			ClearRoomInfo();
#ifdef EnableRttTest
			//GavaNetClient->chsImpl.StopTest();
			_LOG(TEXT("[dEAthcURe] deactivating RTT test..."));
			GavaNetClient->p2pNetTester.activateRttTest(false); // 20070517 dEAthcURe|RTT // GavaNetClient->p2pNetTester.setActive(false);
#endif
		}
		break;
	case _AN_NEWCHARACTER:
		break;
	case _AN_INVENTORY:
		break;
	}

	return TRUE;
}

UBOOL CavaNetStateController::EnteringState(INT NewState)
{
	switch (NewState)
	{
	case _AN_DISCONNECTED:
		if (NetState >= _AN_CHANNELLIST)
		{
			ClearRoomInfo();
			ClearLobbyInfo();
			ClearChannelInfo();
			ClearPlayerInfo();
			ChatMsgList.Clear();
		}
		break;
	case _AN_CONNECTING:
		break;
	case _AN_CHANNELLIST:
		if (NetState >= _AN_LOBBY)
		{
			ClearChannelInfo();
			ChatMsgList.Clear();
			PlayerInfo.RoomKickedReason = 255;
			_WebInClient().ChannelSet(Localize(TEXT("Channel"), TEXT("Text_Loc_ChannelList"), TEXT("AVANET")));
		}
		break;
	case _AN_LOBBY:
		ClearLobbyInfo();
		break;
	case _AN_ROOM:
		if (NetState == _AN_LOBBY)
		{
			ChatMsgList.Clear();
			LastResultInfo.Clear();
		}

		{
			FRoomPlayerInfo *MyInfo = GetMyRoomPlayerInfo();
			if (MyInfo)
				MyInfo->RoomPlayerInfo.bReady = _READY_NONE;
		}
		break;
	case _AN_INGAME:
		// {{ 20071024 dEAthcURe|HM 연습채널의 check all out 기능 비활성화
		//if(EChannelFlag_Practice == _StateController->ChannelInfo.Flag) {
		//	g_hostMigration.bEnableCheckAllOut = false;
		//	//debugf(TEXT("[dEAthcURe|HM] Checking all out after HM DISABLED"));
		//}
		//else {
		//	g_hostMigration.bEnableCheckAllOut = true;
		//	//debugf(TEXT("[dEAthcURe|HM] Checking all out after HM ENABLED"));
		//}
		g_hostMigration.bEnableCheckAllOut = (GetChannelSetting(EChannelSetting_GameCheckAllOut) > 0);
		// }} 20071024 dEAthcURe|HM 연습채널의 check all out 기능 비활성화

		// {{ 20071018 dEAthcURe|HM FHM 제어
		#ifdef EnableHmFastLoading
		// {{ 20071123 dEAthcURe|HM HmFastLoading이 항상 켜지도록
		/*debugf(TEXT("Testing channel %d"), _StateController->ChannelInfo.idChannel);
		if(g_hostMigration.isChannelToLoadFast(_StateController->ChannelInfo.idChannel)) {
			GEnableHmFastLoading = true;
			debugf(TEXT("[dEAthcURe|HM] HmFastLoading enabled"));
		}
		else {
			GEnableHmFastLoading = false;
			debugf(TEXT("[dEAthcURe|HM] HmFastLoading disabled"));
		}*/
		GEnableHmFastLoading = true;
		//debugf(TEXT("[dEAthcURe|HM] always enable HmFastLoading"));
		// }} 20071123 dEAthcURe|HM HmFastLoading이 항상 켜지도록
		#endif
		// }} 20071018 dEAthcURe|HM FHM 제어

		// {{ 20070828 dEAthcURe|HM allowed player정보가 남지 않도록
#ifdef EnableHostMigration
		g_hostMigration.clearAllowedPlayersOnNextRound();
		g_hostMigration.clear();
#endif
		// }} 20070828 dEAthcURe|HM allowed player정보가 남지 않도록
		LastResultInfo.Clear();
//		if (NetState == _AN_ROOM)
//		{
//			// {{ 20070525 dEAthcURe|RTT
//#ifdef EnableRttTest
//			if (GIsGame && !GIsEditor)
//			{	
//				debugf(TEXT("[dEAthcURe] deactivating RTT test..."));
//				GavaNetClient->p2pNetTester.activateRttTest(false); // GavaNetClient->p2pNetTester.setActive();
//			}
//#endif
//			// }} 20070525 dEAthcURe|RTT
//		}
		break;
	case _AN_NEWCHARACTER:
		break;
	case _AN_INVENTORY:
		break;
	}

	return TRUE;
}

// appSeconds() 를 날짜 및 시간 문자열로 변환
FString CavaNetStateController::GetTimeFromAppSec(DOUBLE Sec, UBOOL bAddDate)
{
	if (Sec < BaseSec)
		return TEXT("");

	INT Now = 0;
#ifdef _WEB_IN_CLIENT
	Now = _WebInClient().GetNow();
#endif
	INT Curr = 0;

	if (Now > 0)
	{
		DOUBLE CurrSec = appSeconds();
		Curr = Now + (INT)(Sec - CurrSec);
	}
	else
	{
		Curr = (INT)BaseTime + (INT)(Sec - BaseSec);
	}

	tm *_t = _localtime32((__time32_t*)&Curr);
	if (!_t)
		return TEXT("");

	if (bAddDate)
		return FString::Printf(TEXT("%d/%02d/%02d %02d:%02d:%02d"), _t->tm_year + 1900, _t->tm_mon, _t->tm_mday,
																	_t->tm_hour, _t->tm_min, _t->tm_sec);
	else
		return FString::Printf(TEXT("%02d:%02d:%02d"), _t->tm_hour, _t->tm_min, _t->tm_sec);
}

INT CavaNetStateController::GetNow()
{
	if ( BaseSec == 0 || (GIsEditor && !GIsGame) )
		return 0;

	INT Now = 0;
#ifdef _WEB_IN_CLIENT
	Now = _WebInClient().GetNow();
#endif

	if (Now > 0)
	{
		return Now;
	}
	else
	{
		DOUBLE Sec = appSeconds();
		return (INT)BaseTime + (INT)(Sec - BaseSec);
	}
}

void CavaNetStateController::SetLastConnectResult(const FString &Result)
{
	if (LastConnectResult == TEXT("connecting"))
	{
		LastConnectResult = Result;
	}
	else
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Client, EMsg_Client_Connect, *Result, TEXT(""), 0, 0);
	}

	_LOG(TEXT("LastConnectResult = %s"), *Result);
}

// 클라이언트가 대기실에서 30초 이내에 준비하는지 여부 체크
void CavaNetStateController::SetRoomReadyDue()
{
	if (NetState == _AN_ROOM)
	{
		if ( !AmIHost() && !AmISpectator() && !AmIStealthMode() && !IsCountingDown() && !(!RoomInfo.RoomInfo.setting.allowInterrupt && RoomInfo.IsPlaying()) )
		{
			if (RoomReadyDue == 0.0 && RoomReadyKickCount == 0)
			{
				RoomReadyDue = appSeconds() + 180.0;
				RoomReadyKickCount = 2;
			}
			return;
		}
	}

	RoomReadyDue = 0.0;
	RoomReadyKickCount = 0;
}

// 대기실에서 전원이 준비한 상태에서 방장이 게임을 시작하지 않는 상황 체크
void CavaNetStateController::CheckRoomHostKickCondition()
{
	RoomHostKickCount = -1;

	// 대회 채널 등, 채널 마스크가 설정된 채널에서는 체크하지 않음
	if (!_StateController->ChannelInfo.IsValid() || _StateController->ChannelInfo.IsMaskTurnedOn())
		return;

	if (RoomInfo.IsValid() && NetState == _AN_ROOM && !IsCountingDown() && RoomInfo.IsFull())
	{
		UBOOL bCheck = TRUE;
		for (INT i = 0; i < MAX_PLAYER_PER_ROOM; ++i)
		{
			if (RoomInfo.HostIdx != i && !RoomInfo.PlayerList.IsEmpty(i)/* && FRoomInfo::IsOpenSlot(i)*/)
			{
				if (!RoomInfo.PlayerList.PlayerList[i].IsReady())
				{
					bCheck = FALSE;
					break;
				}
			}
		}
		if (bCheck)
			//if (GetAvaNetHandler()->IsGameStartable(FALSE) && RoomInfo.RoomInfo.state.numCurr > 1)
		{
			RoomHostKickCount = 20;
			//GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_StartCount, TEXT(""), TEXT(""), 0, 0);
		}
	}
}

// 카운트 다운 시작
void CavaNetStateController::StartCountDown()
{
	CountDown = 6;

	FString Msg = Localize(TEXT("UIRoomScene"), TEXT("Msg_StartCountDown"), TEXT("AVANET"));
	ChatMsgList.Add( Msg, EChat_ReadyRoom);

	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Chat, TEXT(""), *Msg, 0, 0);
	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_StartCount, TEXT(""), TEXT(""), 0, 0);
}

// 카운트 다운 처리
UBOOL CavaNetStateController::ProcCountDown()
{
	// 게임시작 카운트 중이거나 호스트 강퇴조건에 해당할때
	// 게임시작이 불가능한 조건(3명이상 인원차이)이거나 난입이 불가능한 조건(2명이상 게임중인원 차이)이라면 에러 메세지를 넘기고 리셋
	FString	ErrorTypeStr;
	if( (CountDown > 0 || RoomHostKickCount > 0) &&
		!GetAvaNetHandler()->IsBalancedRoomPlayers( ErrorTypeStr ) )
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_Start, "failed", *ErrorTypeStr, 0, 0);
		CountDown = -1;
		RoomHostKickCount = -1;
	}

	if (CountDown > 0)
	{
		--CountDown;

		if (CountDown > 0)
		{
			FString Fmt = Localize(TEXT("UIRoomScene"), TEXT("Msg_CountDown"), TEXT("AVANET"));
			if (Fmt.Len() == 0)
				Fmt = TEXT("Starting the game...... %d");
			FString Str = FString::Printf(*Fmt, CountDown);
			ChatMsgList.Add(Str, EChat_ReadyRoom);
		}
		else if (CountDown == 0)
		{
			FString Msg = Localize(TEXT("UIRoomScene"), TEXT("Msg_StartGame"), TEXT("AVANET"));
			ChatMsgList.Add(Msg, EChat_ReadyRoom);

			GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Chat, TEXT(""), *Msg, 0, 0);

			if (AmIHost())
			{
				PM::GAME::START_NTF::Send();
			}
			else if (RoomInfo.RoomInfo.state.playing == RIP_PLAYING && RoomInfo.RoomInfo.setting.allowInterrupt > 0)
			{
				PM::GAME::START_NTF::Send();
			}
		}
	}
	else
	{
		GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_CancelCount, TEXT(""), TEXT(""), 0, 0);
	}

	return (CountDown > 0) || (RoomHostKickCount >= 0);
}

// 카운트 다운 취소
void CavaNetStateController::CancelCountDown()
{
	CountDown = -1;

	FString Msg = Localize(TEXT("UIRoomScene"), TEXT("Msg_CancelCountDown"), TEXT("AVANET"));
	ChatMsgList.Add( Msg, EChat_ReadyRoom);

	GetAvaNetHandler()->ProcMessage(EMsg_Room, EMsg_Room_Chat, TEXT(""), *Msg, 0, 0);
	GetAvaNetHandler()->ProcMessage(EMsg_Game, EMsg_Game_CancelCount, TEXT(""), TEXT(""), 0, 0);
}

UBOOL CavaNetStateController::IsVoteAvailable()
{
	// 게임 중에는 강퇴 비용이 있어야 함
#ifdef _APPLY_VOTE_FEE
	return (NetState == _AN_INGAME && PlayerInfo.PlayerInfo.money >= VOTE_FEE) || (NetState == _AN_ROOM && AmIHost());
	//return PlayerInfo.PlayerInfo.money >= VOTE_FEE &&
	//		((NetState == _AN_ROOM && AmIHost()) || NetState == _AN_INGAME);
#else
	return NetState == _AN_INGAME || (NetState == _AN_ROOM && AmIHost());
#endif
}

// 채널 목록에서 채널을 하나 선택
FChannelInfo* CavaNetStateController::SelectChannel(UBOOL bPreferLastChannel)
{
	FChannelInfo *Info = NULL;
	if (bPreferLastChannel)
	{
		// 마지막으로 접속한 채널을 우선 선택

		FString *Channel = NULL;
		Channel = GavaNetClient->Settings.Find(CFG_LASTCHANNEL);
		if (Channel && *Channel != TEXT("") && *Channel != appItoa(ID_MY_CLAN_HOME))
		{
			Info = ChannelList.Find(appAtoi(**Channel));
			//if (Info && Info->Flag != _CF_NORMAL)
			//{
			//	if (Info->Flag != _CF_PCBANG || !PcBangFlag)
			//	{
			//		Info = NULL;
			//	}
			//}
			if (!Info)
			{
				CHANNEL_DESC Desc;
				Desc.idChannel = appAtoi(**Channel);
				Desc.flag = _CF_NORMAL;
				Desc.idx = Desc.idChannel;
				Desc.maxPlayers = 1000;
				Info = ChannelList.Update(Desc, TEXT(""));
			}
		}
	}

	if (!Info)
		Info = ChannelList.RandomSelect();

	return Info;
}

void CavaNetStateController::ClearPlayerInfo()
{
	ClearChannelInfo();

	if (PlayerInfo.IsValid())
	{
		PlayerInfo.PlayerInfo.idAccount = ID_INVALID_ACCOUNT;
	}
}

void CavaNetStateController::ClearChannelInfo()
{
	ClearRoomInfo();
	CountDown = -1;		//!< IsCountingDown()에서 -1보다 큰 경우라 0에서 -1로 수정.(2007/03/05 고광록)

	if (ChannelInfo.IsValid())
	{
		ChannelInfo.idChannel = ID_INVALID_CHANNEL;
	}

	RoomList.Clear();
	LobbyPlayerList.Clear();

	QuickJoinChannelList.Empty();
}

void CavaNetStateController::ClearLobbyInfo()
{
	RoomList.Clear();
	LobbyPlayerList.Clear();
}

void CavaNetStateController::ClearRoomInfo()
{
	RoomInfo.Clear();
	if (IsCountingDown())
		CancelCountDown();
	RoomHostKickCount = -1;
	RoomReadyDue = 0.0;
	RoomReadyKickCount = 0;
}


void CavaNetStateController::RefreshCurrentRoom()
{
	if (NetState == _AN_ROOM && RoomInfo.IsValid())
	{
		//RoomInfo.RoundCount = 0;
		RoomInfo.RoomInfo.state.currRound = 0;
		RoomInfo.RoomInfo.state.playing = RIP_WAIT;

		for (INT i = 0; i < MAX_ALL_PLAYER_PER_ROOM; ++i)
		{
			if (RoomInfo.PlayerList.PlayerList[i].IsValid())
			{
				RoomInfo.PlayerList.PlayerList[i].RoomPlayerInfo.bReady = (i == RoomInfo.HostIdx ? _READY_WAIT : _READY_NONE);
			}
		}

		//PM::ROOM::INFO_REQ::Send();

//#ifdef EnableRttTest
//		if (GIsGame && !GIsEditor && (!AmIAdmin() || !StealthMode))
//		{
//			if ( AmIHost() )
//			{
//				GavaNetClient->p2pNetTester.initRttTest();
//			}
//			else
//			{
//				if ( !RoomInfo.PlayerList.IsEmpty(RoomInfo.HostIdx) )
//				{
//					FRoomPlayerInfo &Host = RoomInfo.PlayerList.PlayerList[RoomInfo.HostIdx];
//					if (Host.RttValue < 0)
//					{
//						Host.RttValue = -1;
//						Host.RoomPlayerInfo.rttScore = -1;
//
//						_LOG(TEXT("Starting rtt test with the host. idAccount = %d"), Host.RoomPlayerInfo.idAccount);
//						GavaNetClient->chsImpl.AddPlayer(Host.RoomPlayerInfo.idAccount);
//
//						GavaNetClient->p2pNetTester.activateRttTest(true, AmIHost()==TRUE?true:false); // 20070517 dEAthcURe|RTT // GavaNetClient->p2pNetTester.setActive();
//					}
//				}
//			}
//		}
//#endif
	}
}

void CavaNetStateController::SetCurrentHostUnreachable()
{
	if (!RoomInfo.IsValid())
		return;

	if (RoomInfo.HostIdx >= 0 && RoomInfo.HostIdx < Def::MAX_ALL_PLAYER_PER_ROOM)
	{
		FRoomPlayerInfo &HostInfo = RoomInfo.PlayerList.PlayerList[RoomInfo.HostIdx];
		_Locals.UnreachablePlayerList.AddUniqueItem(HostInfo.PlayerInfo.nickname);
	}
}

UBOOL CavaNetStateController::IsHostUnreachable(TCHAR *HostName)
{
	return _Locals.UnreachablePlayerList.FindItemIndex(HostName) != INDEX_NONE;
}

UBOOL CavaNetStateController::IsHostUnreachable(INT RoomIndex)
{
	return IsHostUnreachable( RoomList.RoomList(RoomIndex).RoomInfo.hostName );
}


#define _CASE_CHANNELSETTING(v) case EChannelSetting_##v: return (INT)Info.v;

INT CavaNetStateController::GetChannelSetting(BYTE Setting)
{
	if (!ChannelInfo.IsValid())
		return -1;

	FavaNetChannelSettingInfo &Info = GetChannelSettingInfo();

	switch (Setting)
	{
	_CASE_CHANNELSETTING(EnableAutoBalance)
	_CASE_CHANNELSETTING(EnableSpectator)
	_CASE_CHANNELSETTING(EnableInterrupt)
	_CASE_CHANNELSETTING(EnableBackView)
	_CASE_CHANNELSETTING(EnableGhostChat)
	_CASE_CHANNELSETTING(EnableAutoSwapTeam)
	_CASE_CHANNELSETTING(EnableMaxPlayers)
	_CASE_CHANNELSETTING(EnableTKLevel)
	_CASE_CHANNELSETTING(MissionTypeSpecial)
	_CASE_CHANNELSETTING(MissionTypeWarfare)
	_CASE_CHANNELSETTING(MissionTypeTraining)
	_CASE_CHANNELSETTING(DefaultAutoBalance)
	_CASE_CHANNELSETTING(DefaultSpectator)
	_CASE_CHANNELSETTING(DefaultInterrupt)
	_CASE_CHANNELSETTING(DefaultBackView)
	_CASE_CHANNELSETTING(DefaultGhostChat)
	_CASE_CHANNELSETTING(DefaultAutoSwapTeam)
	_CASE_CHANNELSETTING(DefaultTKLevel)
	_CASE_CHANNELSETTING(DefaultMaxPlayers)
	_CASE_CHANNELSETTING(RoomChangeTeam)
	_CASE_CHANNELSETTING(RoomSkipMinPlayerCheck)
	_CASE_CHANNELSETTING(RoomSkipBalanceCheck)
	_CASE_CHANNELSETTING(GameIdleCheck)
	_CASE_CHANNELSETTING(GameFreeCam)
	_CASE_CHANNELSETTING(GameCheckAllOut)
	_CASE_CHANNELSETTING(UpdatePlayerScore)
	_CASE_CHANNELSETTING(UpdateStatLog)
	_CASE_CHANNELSETTING(InvenCashItem)
	_CASE_CHANNELSETTING(AllowPCBangBonus)
	_CASE_CHANNELSETTING(AllowBoostItem)
	_CASE_CHANNELSETTING(AllowEventBonus)
	}

	return -1;
}

#undef _CASE_CHANNELSETTING

FavaNetChannelSettingInfo& CavaNetStateController::GetChannelSettingInfo()
{
	UavaNetRequest *pRequest = GetAvaNetRequest();
	check(pRequest);
	check(pRequest->ChannelSettingList.Num() > 0);

	if (!ChannelInfo.IsValid())
		return pRequest->ChannelSettingList(0);

	for (INT i = 0; i < pRequest->ChannelSettingList.Num(); ++i)
	{
		if (pRequest->ChannelSettingList(i).Flag == ChannelInfo.Flag)
		{
			return pRequest->ChannelSettingList(i);
		}
	}

	return pRequest->ChannelSettingList(0);
}

FPlayerDispInfo* CavaNetStateController::FindPlayerFromList(const FString &Nickname)
{
	switch (NetState)
	{
	case _AN_LOBBY:
		return LobbyPlayerList.Find(Nickname);
	case _AN_ROOM:
	case _AN_INGAME:
		return RoomInfo.PlayerList.Find(Nickname);
	default:
		return NULL;
	}
}

FPlayerDispInfo* CavaNetStateController::FindPlayerFromList(Def::TID_ACCOUNT idAccount)
{
	switch (NetState)
	{
	case _AN_LOBBY:
	case _AN_INVENTORY:
		return LobbyPlayerList.Find(idAccount);
	case _AN_ROOM:
	case _AN_INGAME:
		return RoomInfo.PlayerList.Find(idAccount);
	default:
		return NULL;
	}
}


UBOOL CavaNetStateController::ProcAutoMove()
{
	// 자동 이동 처리
	if (!AutoMoveDest.IsMoving())
		return FALSE;

	_LOG(TEXT("idChannel = %d, idRoom = %d, RoomPassword = %s, idAccount = %d"),
			AutoMoveDest.idChannel, AutoMoveDest.idRoom, *AutoMoveDest.RoomPassword, AutoMoveDest.idAccount);

	UBOOL Result = FALSE;

	switch (NetState)
	{
	case _AN_CHANNELLIST:
		if (AutoMoveDest.IsNormalChannel())
		{
			// 목적지가 일반 채널일 때
			// 채널 조인
			GetAvaNetRequest()->JoinChannelByID(AutoMoveDest.idChannel, AutoMoveDest.IsFollowing());
			Result = TRUE;
		}
		else if (AutoMoveDest.IsGuildChannel())
		{
			// 목적지가 클랜 홈일 때
			GetAvaNetRequest()->GuildJoinChannel();
			Result = TRUE;
		}
		else
		{
			// arrived
			AutoMoveDest.Clear();
			GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_EndFollow, TEXT("channel list"), TEXT(""), 0, 0);
			Result = TRUE;
		}
		break;
	case _AN_INVENTORY:
		GetAvaNetRequest()->LeaveInven();
		// 인벤토리 나간 후 즉시 case _AN_LOBBY 실행
	case _AN_LOBBY:
		if (AutoMoveDest.idChannel == ChannelInfo.idChannel)
		{
			// 목적지와 같은 채널임
			if (AutoMoveDest.idRoom != ID_INVALID_ROOM)
			{
				// 들어가야할 방이 있음
				PM::ROOM::JOIN_REQ::Send(AutoMoveDest.idRoom, *AutoMoveDest.RoomPassword);
				Result = TRUE;
			}
			else
			{
				// arrived
				AutoMoveDest.Clear();
				GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_EndFollow, TEXT("lobby"), TEXT(""), 0, 0);
				Result = TRUE;
			}
		}
		else
		{
			// 목적지는 다른 채널 임
			// 채널 나감
			GetAvaNetRequest()->LeaveChannel();
			Result = TRUE;
		}
		break;
	case _AN_ROOM:
		if (AutoMoveDest.idChannel != ChannelInfo.idChannel || AutoMoveDest.idRoom != RoomInfo.RoomInfo.idRoom)
		{
			// 목적지는 다른 채널이거나 다른 방
			// 방 나감
			GetAvaNetRequest()->LeaveRoom();
			Result = TRUE;
		}
		else
		{
			// 목적지와 같은 채널, 같은 방
			if (!AmIStealthMode() && AutoMoveDest.idAccount != ID_INVALID_ACCOUNT)
			{
				FRoomPlayerInfo *MyPlayer = GetMyRoomPlayerInfo();
				FRoomPlayerInfo *Buddy = RoomInfo.PlayerList.Find(AutoMoveDest.idAccount);
				if (MyPlayer && Buddy)
				{
					// 따라가고자하는 플레이어가 방에 있음
					BYTE idTeam = Buddy->GetTeamID();
					if (idTeam != RT_SPECTATOR && idTeam != MyPlayer->GetTeamID())
					{
						// 해당 플레이어와 팀이 다름; 빈 자리가 있으면 같은 팀으로 이동
						INT Slot = RoomInfo.FindEmptySlot(idTeam);
						if (Slot != -1)
						{
							PM::ROOM::CHANGE_SLOT_NTF::Send(Slot);
						}
					}
				}
			}

			// arrived anyway
			AutoMoveDest.Clear();
			GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_EndFollow, TEXT("room"), TEXT(""), 0, 0);
			Result = TRUE;
		}
		break;
	default:
		AutoMoveDest.Clear();
		break;
	}

	return Result;
}

void CavaNetStateController::StopAutoMove()
{
	UBOOL WasMoving = AutoMoveDest.IsMoving();
	AutoMoveDest.Clear();
	if (WasMoving)
	{
		switch (NetState)
		{
		case _AN_CHANNELLIST:
			GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_EndFollow, TEXT("channel list"), TEXT(""), 0, 0);
			break;
		case _AN_LOBBY:
			GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_EndFollow, TEXT("lobby"), TEXT(""), 0, 0);
			break;
		case _AN_ROOM:
			//GetAvaNetHandler()->ProcMessage(EMsg_Buddy, EMsg_Buddy_EndFollow, TEXT("room"), TEXT(""), 0, 0);
			break;
		}
	}
}

// 정렬된 채널 퀵 조인 목록에서 가장 앞의 채널로 조인 시도. 성공할 때까지 반복됨.
void CavaNetStateController::ProcQuickJoinChannel()
{
	if (QuickJoinChannelList.Num() > 0)
	{
		INT id = QuickJoinChannelList(0);
		QuickJoinChannelList.Remove(0);

		GetAvaNetRequest()->JoinChannel(id);
	}
}

void CavaNetStateController::ParseConsoleCommand(const TCHAR *Cmd)
{
	if ( ParseCommand(&Cmd, TEXT("TEST")) )
	{
		if ( ParseCommand(&Cmd, TEXT("STAT")) )
		{
			_LOG(TEXT("Sending test stat log message"));

			STAT_GAME_SCORE_LOG statGameScore[2];
			TArray<STAT_ROUND_PLAY_LOG> statRoundPlay;
			TArray<STAT_WEAPON_LOG> statRoundWeapon;
			TArray<STAT_KILL_LOG> statKill;

			statGameScore[0].killCount = 0;
			statGameScore[0].suicideCount = 0;
			statGameScore[0].headshotKillCount = 0;
			statGameScore[0].score.attacker = 0;
			statGameScore[0].score.defender = 0;
			statGameScore[0].score.leader = 2;
			statGameScore[0].score.tactic = 3;
			statGameScore[0].friendlyFireCount = 0;
			statGameScore[0].friendlyKillCount = 0;
			statGameScore[0].spawnCount[0] = 71;
			statGameScore[0].spawnCount[1] = 62;
			statGameScore[0].spawnCount[2] = 46;
			statGameScore[1].killCount = 0;
			statGameScore[1].suicideCount = 0;
			statGameScore[1].headshotKillCount = 0;
			statGameScore[1].score.attacker = 0;
			statGameScore[1].score.defender = 0;
			statGameScore[1].score.leader = 0;
			statGameScore[1].score.tactic = 2;
			statGameScore[1].friendlyFireCount = 0;
			statGameScore[1].friendlyKillCount = 0;
			statGameScore[1].spawnCount[0] = 57;
			statGameScore[1].spawnCount[1] = 85;
			statGameScore[1].spawnCount[2] = 29;

			STAT_ROUND_PLAY_LOG *pStat = new(statRoundPlay) STAT_ROUND_PLAY_LOG;
			check(pStat);

			pStat->winTeam = 1;
			pStat->winType = 0;
			pStat->startTime = 32;
			pStat->roundTime = 1031;
			pStat->playerCount[0] = 8;
			pStat->playerCount[1] = 8;

			for (INT i = 0; i < 17; ++i)
			{
				STAT_WEAPON_LOG *pStatWeapon = new(statRoundWeapon) STAT_WEAPON_LOG;
				check(pStatWeapon);

				pStatWeapon->idWeapon = 776;
				pStatWeapon->round = 0;
				pStatWeapon->usedCount = 44;
				pStatWeapon->fireCount = 1050;
				pStatWeapon->hitCount_Head = 10;
				pStatWeapon->hitCount_Body = 22;
				pStatWeapon->hitCount_Stomach = 44;
				pStatWeapon->hitCount_LeftArm = 12;
				pStatWeapon->hitCount_RightArm = 15;
				pStatWeapon->hitCount_LeftLeg = 24;
				pStatWeapon->hitCount_RightLeg = 31;
				pStatWeapon->hitDistance = 104.4;
				pStatWeapon->hitDamage = 55;
				pStatWeapon->killCount[0] = 24;
				pStatWeapon->killCount[1] = 15;
				pStatWeapon->killCount[2] = 36;
				pStatWeapon->headshotKillCount = 12;
				pStatWeapon->multiKillCount = 2;
			}

			for (INT i = 0; i < 334; ++i)
			{
				STAT_KILL_LOG *pStat = new(statKill) STAT_KILL_LOG;
				check(pStat);

				pStat->idWeapon = 776;

				pStat->killTime = 47;
				pStat->killerLocX = 294.68;
				pStat->killerLocY = -1105.8;
				pStat->killerLocZ = -543.85;
				pStat->victimLocX = -47.53;
				pStat->victimLocY = -1483.22;
				pStat->victimLocZ = -676.99;
			}

			PM::GAME::REPORT_STAT_NTF::Send(statGameScore, statRoundPlay, statRoundWeapon, statKill);
		}
		else if ( ParseCommand(&Cmd, TEXT("NOW")) )
		{
			INT Now = _StateController->GetNow();
			TCHAR *StrTime = _wctime32((__time32_t*)&Now);
			_LOG(TEXT("Now = %d -> %s"), Now, StrTime);

			//tm *TM = localtime((time_t*)&Now);
			//_LOG(TEXT("3"));
			//if (TM)
			//	_LOG(TEXT("Now = %d -> %d/%d/%d %02d:%02d:%02d"), Now, TM->tm_year, TM->tm_mon, TM->tm_mday, TM->tm_hour, TM->tm_min, TM->tm_sec);
			//else
			//	_LOG(TEXT("Now = %d"), Now);
		}
	}
	else if ( ParseCommand(&Cmd, TEXT("VERSION")) )
	{
		_CNLOG(TEXT("avaNet protocol version = %d, client version = %d"), Def::VERSION_PROTOCOL, Def::VERSION_CLIENT);
	}
	else if ( ParseCommand(&Cmd, TEXT("MYINFO")) )
	{
		FInternetIpAddr Addr;
		GSocketSubsystem->GetLocalHostAddr(*GLog, Addr);
		//((SOCKADDR_IN*)((SOCKADDR*)Addr))->sin_addr.S_un.S_addr;
		_CLOG(TEXT("My IP address is %s"), *(Addr.ToString(FALSE)));
		_CLOG(TEXT("My net state is %s(%d)"), (NetState == 0 ? TEXT("DISCONNECTED") : NetState == 1 ? TEXT("CONNECTING") : NetState == 2 ? TEXT("CHANNELLIST") :
			NetState == 3 ? TEXT("LOBBY") : NetState == 4 ? TEXT("ROOM") : NetState == 5 ? TEXT("INGAME") : NetState == 6 ? TEXT("NEWCHARACTEER") : TEXT("UNKNOWN")), NetState);
	}
	else if ( ParseCommand(&Cmd, TEXT("SERVERADDRESS")) )
	{
		FString Addr = ParseToken(Cmd, FALSE);

		if (Addr.Len() > 0)
			GavaNetClient->Settings.Set(CFG_NEWSERVERADDRESS, *Addr);
	}
	else if ( ParseCommand(&Cmd, TEXT("CONNECT")) )
	{
		FString USN = ParseToken(Cmd, FALSE);
		FString ID = ParseToken(Cmd, FALSE);

		//GavaNetClient->Settings.Set(CFG_USERID, *ID);
		//GavaNetClient->Settings.Set(CFG_USERPASSWORD, *Pwd);

		//if ( !GavaNetClient->AutoConnect() )
		if ( !GetAvaNetRequest()->AutoConnect(FALSE, USN, ID) )
			_CLOG(TEXT("Connect failed"));
	}
	else if ( ParseCommand(&Cmd, TEXT("FORCECONNECT")) )
	{
		FString USN = ParseToken(Cmd, FALSE);
		FString ID = ParseToken(Cmd, FALSE);

		//GavaNetClient->Settings.Set(CFG_USERID, *ID);
		//GavaNetClient->Settings.Set(CFG_USERPASSWORD, *Pwd);

		if ( !GetAvaNetRequest()->AutoConnect(TRUE, USN, ID) )
			_CLOG(TEXT("Connect failed"));
	}
	else if ( ParseCommand(&Cmd, TEXT("DISCONNECT")) )
	{
		GetAvaNetRequest()->CloseConnection(EXIT_UNKNOWN);
	}
	else if ( ParseCommand(&Cmd, TEXT("RESET")) )
	{
		GetAvaNetRequest()->CloseConnection(EXIT_UNKNOWN);
		// {{ 20070214 dEAthcURe|HM
#ifdef EnableHostMigration
		g_hostMigration.clear();
#endif
		// }} 20070214 dEAthcURe|HM
		GEngine->SetClientTravel(TEXT("avaEntry.ut3?closed"), TRAVEL_Absolute);
	}
	else if ( ParseCommand(&Cmd, TEXT("QUIT")) )
	{
		UBOOL bGraceful = TRUE;
		if (ParseCommand(&Cmd, TEXT("NOW")) )
			bGraceful = FALSE;
		GetAvaNetRequest()->Quit(bGraceful);
	}
	else if ( ParseCommand(&Cmd, TEXT("NEWCHAR")) )
	{
		FString Nickname = ParseToken(Cmd, FALSE);
		FString Face = ParseToken(Cmd, FALSE);

		//PM::CLIENT::CHECK_NICK_REQ::Send(Nickname, appAtoi(*Face));
		GetAvaNetRequest()->CreateCharacter(Nickname, appAtoi(*Face));
	}
	else if ( ParseCommand(&Cmd, TEXT("POPUP")) )
	{
		GetAvaNetHandler()->eventPopUpMessage(EPT_Notice, Cmd, TEXT("None"));
	}
	else if ( ParseCommand(&Cmd, TEXT("CHANNEL")) )
	{
		ParseChannelCommand(Cmd);
	}
	else if ( ParseCommand(&Cmd, TEXT("ROOM")) )
	{
		ParseRoomCommand(Cmd);
	}
	else if ( ParseCommand(&Cmd, TEXT("GAME")) )
	{
		ParseGameCommand(Cmd);
	}
	else if ( ParseCommand(&Cmd, TEXT("INVEN")) )
	{
		ParseInventoryCommand(Cmd);
	}
	else if ( ParseCommand(&Cmd, TEXT("GUILD")) )
	{
		ParseGuildCommand(Cmd);
	}
#ifdef _WEB_IN_CLIENT
	else if ( ParseCommand(&Cmd, TEXT("WIC")) )
	{
		if ( ParseCommand(&Cmd, TEXT("CHARGE")) )
		{
			GetAvaNetRequest()->WICOpenChargeWindow();
		}
		else if ( ParseCommand(&Cmd, TEXT("CASH")) )
		{
			GetAvaNetRequest()->WICGetCash();
		}
		else if ( ParseCommand(&Cmd, TEXT("BUY")) )
		{
			FString Item = ParseToken(Cmd, FALSE);
			if (Item.Len() > 0)
			{
				GetAvaNetRequest()->WICBuyItem(appAtoi(*Item));
			}
		}
		else if ( ParseCommand(&Cmd, TEXT("SENDGIFT")) )
		{
			FString To = ParseToken(Cmd, FALSE);
			FString Item = ParseToken(Cmd, FALSE);
			if (To.Len() > 0 && Item.Len() > 0)
			{
				GetAvaNetRequest()->WICSendGift(appAtoi(*Item), appAtoi(*To));
			}
		}
		else if ( ParseCommand(&Cmd, TEXT("GIFT")) )
		{
			FString Item = ParseToken(Cmd, FALSE);
			GetAvaNetRequest()->WICOpenGiftWindow(appAtoi(*Item));
		}
		else if ( ParseCommand(&Cmd, TEXT("CHANNEL")) )
		{
			if ( ParseCommand(&Cmd, TEXT("SET")) )
			{
				if (appStrlen(Cmd) > 0)
					_WebInClient().ChannelSet(Cmd);
			}
			else if ( ParseCommand(&Cmd, TEXT("UNSET")) )
			{
				_WebInClient().ChannelUnset();
			}

		}
		else if ( ParseCommand(&Cmd, TEXT("GUILD")) )
		{
			if ( ParseCommand(&Cmd, TEXT("JOIN")) )
			{
				FString Guild = ParseToken(Cmd, FALSE);
				FString From = ParseToken(Cmd, FALSE);
				FString To = ParseToken(Cmd, FALSE);
				if (Guild.Len() > 0 && From.Len() > 0 && To.Len() > 0)
					_WebInClient().GuildJoin( Guild, appAtoi(*From), appAtoi(*To) );
			}
			else if ( ParseCommand(&Cmd, TEXT("KICK")) )
			{
				FString Guild = ParseToken(Cmd, FALSE);
				FString From = ParseToken(Cmd, FALSE);
				FString To = ParseToken(Cmd, FALSE);
				if (Guild.Len() > 0 && From.Len() > 0 && To.Len() > 0)
					_WebInClient().GuildKick( Guild, appAtoi(*From), appAtoi(*To) );
			}
			else if ( ParseCommand(&Cmd, TEXT("LEAVE")) )
			{
				FString Guild = ParseToken(Cmd, FALSE);
				FString From = ParseToken(Cmd, FALSE);
				if (Guild.Len() > 0 && From.Len() > 0)
					_WebInClient().GuildLeave( Guild, appAtoi(*From) );
			}
		}
	}
#endif
}

void CavaNetStateController::ParseChannelCommand(const TCHAR *Cmd)
{
	if (NetState <= _AN_CONNECTING)
		return;

	if (NetState < _AN_ROOM)
	{
		if ( ParseCommand(&Cmd, TEXT("CHAT")) )
		{
			PM::CHANNEL::LOBBY_CHAT_NTF::Send(Cmd);
		}
		else if ( ParseCommand(&Cmd, TEXT("ROOM")) )
		{
			GetAvaNetRequest()->ListRoom();
		}
		else if ( ParseCommand(&Cmd, TEXT("PLAYER")) )
		{
			GetAvaNetRequest()->ListLobbyPlayer();
		}
		else if ( ParseCommand(&Cmd, TEXT("LIST")) )
		{
			PM::CHANNEL::CHANNEL_LIST_REQ::Send();
		}
		else if ( ParseCommand(&Cmd, TEXT("JOIN")) )
		{
			BYTE id = (BYTE)appAtoi(Cmd);
			if (id == 0)
			{
				_LOG(TEXT("Joining default channel. (id = 111)"));
				id = 111;
			}

			//PM::CHANNEL::CHANNEL_JOIN_REQ::Send(id);
			GetAvaNetRequest()->JoinChannelByID(id);
		}
		else if ( ParseCommand(&Cmd, TEXT("LEAVE")) )
		{
			//PM::CHANNEL::CHANNEL_LEAVE_REQ::Send();
			GetAvaNetRequest()->LeaveChannel();
		}
	}
}

void CavaNetStateController::ParseRoomCommand(const TCHAR *Cmd)
{
	if (NetState < _AN_LOBBY)
		return;

	if ( ParseCommand(&Cmd, TEXT("CREATE")) )
	{
		FString RoomName = ParseToken(Cmd, FALSE);
		FString MapID = ParseToken(Cmd, FALSE);
		FString TKLevel = ParseToken(Cmd, FALSE);
		FString AllowSpectator = ParseToken(Cmd, FALSE);
		FString AutoBalance = ParseToken(Cmd, FALSE);
		FString AllowInterrupt = ParseToken(Cmd, FALSE);
		FString RoundToWin = ParseToken(Cmd, FALSE);
		FString MaxPlayer = ParseToken(Cmd, FALSE);
		FString Password = ParseToken(Cmd, FALSE);

		FavaRoomSetting Setting;
		Setting.idMap = appAtoi(*MapID);
		Setting.tkLevel = appAtoi(*TKLevel);
		Setting.allowSpectator = appAtoi(*AllowSpectator);
		Setting.autoBalance = appAtoi(*AutoBalance);
		Setting.allowInterrupt = appAtoi(*AllowInterrupt);
		Setting.roundToWin = appAtoi(*RoundToWin);
		Setting.MaxPlayer = appAtoi(*MaxPlayer);
		if (Setting.MaxPlayer == 0)
			Setting.MaxPlayer = MAX_PLAYER_PER_ROOM;

		GetAvaNetRequest()->CreateRoom(RoomName, Password, Setting);
	}
	else if ( ParseCommand(&Cmd, TEXT("JOIN")) )
	{
		FString ListIndex = ParseToken(Cmd, FALSE);
		FString Password = ParseToken(Cmd, FALSE);

		GetAvaNetRequest()->JoinRoom(appAtoi(*ListIndex), Password);
	}

	if (NetState < _AN_ROOM)
		return;

	if ( ParseCommand(&Cmd, TEXT("LEAVE")) )
	{
		FString Reason = ParseToken(Cmd, FALSE);

		_LOG(TEXT("Leaving room, caused by console command; Reason = %d"), appAtoi(*Reason));
		GetAvaNetRequest()->LeaveRoom(appAtoi(*Reason));
	}
	else if ( ParseCommand(&Cmd, TEXT("READY")) )
	{
		GetAvaNetRequest()->RoomReadyToggle();
	}
	else if ( ParseCommand(&Cmd, TEXT("KICK")) )
	{
		GetAvaNetRequest()->RoomKickPlayer(Cmd);
	}
	else if ( ParseCommand(&Cmd, TEXT("SETTING")) )
	{
		FString MapID = ParseToken(Cmd, FALSE);
		FString TKLevel = ParseToken(Cmd, FALSE);
		FString AutoBalance = ParseToken(Cmd, FALSE);
		FString AllowInterrupt = ParseToken(Cmd, FALSE);
		FString RoundToWin = ParseToken(Cmd, FALSE);
		FString MaxPlayer = ParseToken(Cmd, FALSE);

		FavaRoomSetting Setting;
		Setting.idMap = appAtoi(*MapID);
		Setting.tkLevel = appAtoi(*TKLevel);
		Setting.autoBalance = appAtoi(*AutoBalance);
		Setting.allowInterrupt = appAtoi(*AllowInterrupt);
		Setting.roundToWin = appAtoi(*RoundToWin);
		Setting.MaxPlayer = appAtoi(*MaxPlayer);
		if (Setting.MaxPlayer == 0)
			Setting.MaxPlayer = MAX_PLAYER_PER_ROOM;
		if (Setting.roundToWin == 0)
			Setting.roundToWin = 5;

		GetAvaNetRequest()->RoomSetting(Setting);
	}
	else if ( ParseCommand(&Cmd, TEXT("CHAT")) )
	{
		PM::ROOM::CHAT_NTF::Send(Cmd);
	}
	else if ( ParseCommand(&Cmd, TEXT("TEAM")) )
	{
		FString TeamID = ParseToken(Cmd, FALSE);

		GetAvaNetRequest()->RoomChangeTeam(appAtoi(*TeamID));
	}
	else if ( ParseCommand(&Cmd, TEXT("SWAPTEAM")) )
	{
		GetAvaNetRequest()->RoomSwapTeam();
	}
	else if ( ParseCommand(&Cmd, TEXT("CLAIMHOST")) )
	{
		GetAvaNetRequest()->RoomClaimHost();
	}
}

void CavaNetStateController::ParseGameCommand(const TCHAR *Cmd)
{
	if ( ParseCommand(&Cmd, TEXT("START")) )
	{
		PM::GAME::START_NTF::Send();
	}
	else if ( ParseCommand(&Cmd, TEXT("LEAVE")) )
	{
		GetAvaNetRequest()->LeaveGame();
	}
	else if ( ParseCommand(&Cmd, TEXT("SWAPTEAM")) )
	{
		GetAvaNetHandler()->SwapTeamInGame();
	}
}

void CavaNetStateController::ParseGuildCommand(const TCHAR *Cmd)
{
	if ( ParseCommand(&Cmd, TEXT("JOINCHANNEL")) )
	{
		GetAvaNetRequest()->GuildJoinChannel();
	}
}

void CavaNetStateController::ParseInventoryCommand(const TCHAR *Cmd)
{
	if ( ParseCommand(&Cmd, TEXT("LIST")) )
	{
		PlayerInfo.DumpItemInfo();
	}
	else if ( ParseCommand(&Cmd, TEXT("ENTER")) )
	{
		PM::INVENTORY::ENTER_NTF::Send();
		GoToState(_AN_INVENTORY);
	}
	else if ( ParseCommand(&Cmd, TEXT("LEAVE")) )
	{
		PM::INVENTORY::LEAVE_NTF::Send();
		GoToState(_AN_LOBBY);
	}
	else if ( ParseCommand(&Cmd, TEXT("EQUIPSET")) )
	{
		FString EquipSlot = ParseToken(Cmd, FALSE);
		FString ItemSN = ParseToken(Cmd, FALSE);
		if (ItemSN.Len() == 0)
			return;
		PM::INVENTORY::EQUIPSET_REQ::Send(appAtoi(*EquipSlot), appAtoi(*ItemSN));
	}
	else if ( ParseCommand(&Cmd, TEXT("EQUIPUNSET")) )
	{
		FString EquipSlot = ParseToken(Cmd, FALSE);
		PM::INVENTORY::EQUIPSET_REQ::Send(appAtoi(*EquipSlot), SN_INVALID_ITEM);
	}
	else if ( ParseCommand(&Cmd, TEXT("WEAPONSET")) )
	{
		FString WeaponSlot = ParseToken(Cmd, FALSE);
		FString ItemSN = ParseToken(Cmd, FALSE);
		if (ItemSN.Len() == 0)
			return;
		PM::INVENTORY::WEAPONSET_REQ::Send(appAtoi(*WeaponSlot), appAtoi(*ItemSN));
	}
	else if ( ParseCommand(&Cmd, TEXT("WEAPONUNSET")) )
	{
		FString WeaponSlot = ParseToken(Cmd, FALSE);
		PM::INVENTORY::WEAPONSET_REQ::Send(appAtoi(*WeaponSlot), SN_INVALID_ITEM);
	}
	else if ( ParseCommand(&Cmd, TEXT("CUSTOMSET")) )
	{
		FString InvenSlot = ParseToken(Cmd, FALSE);
		FString CustomPart = ParseToken(Cmd, FALSE);
		FString Pos = ParseToken(Cmd, FALSE);
		if (Pos.Len() == 0)
			return;
		PM::INVENTORY::CUSTOMSET_REQ::Send(appAtoi(*InvenSlot), appAtoi(*CustomPart), appAtoi(*Pos));
	}
	else if ( ParseCommand(&Cmd, TEXT("CUSTOMUNSET")) )
	{
		FString InvenSlot = ParseToken(Cmd, FALSE);
		FString Pos = ParseToken(Cmd, FALSE);
		PM::INVENTORY::CUSTOMSET_REQ::Send(appAtoi(*InvenSlot), ID_INVALID_ITEM, appAtoi(*Pos));
	}
	else if ( ParseCommand(&Cmd, TEXT("EFFECTSET")) )
	{
		FString EffectSlot = ParseToken(Cmd, FALSE);
		FString ItemSN = ParseToken(Cmd, FALSE);
		if (ItemSN.Len() == 0)
			return;
		PM::INVENTORY::EFFSET_REQ::Send(appAtoi(*EffectSlot), appAtoi(*ItemSN));
	}
	else if ( ParseCommand(&Cmd, TEXT("EFFECTUNSET")) )
	{
		FString EffectSlot = ParseToken(Cmd, FALSE);
		PM::INVENTORY::EFFSET_REQ::Send(appAtoi(*EffectSlot), SN_INVALID_ITEM);
	}
	else if ( ParseCommand(&Cmd, TEXT("BUY")) )
	{
		FString ItemID = ParseToken(Cmd, FALSE);
		PM::INVENTORY::ITEM_BUY_REQ::Send(appAtoi(*ItemID));
	}
	else if ( ParseCommand(&Cmd, TEXT("GIFT")) )
	{
		FString ItemID = ParseToken(Cmd, FALSE);
		FString Expire = ParseToken(Cmd, FALSE);
		FString AccountID = ParseToken(Cmd, FALSE);
		PM::INVENTORY::ITEM_GIFT_REQ::Send(appAtoi(*ItemID), appAtoi(*Expire), appAtoi(*AccountID));
	}
	else if ( ParseCommand(&Cmd, TEXT("DUMP")) )
	{
		_StateController->PlayerInfo.DumpItemInfo();
	}
}


inline void _ShowHelp(const TCHAR *Sect)
{
	if (!Sect)
		return;

	for (INT i = 0; i < 30; ++i)
	{
		FString Help = Localize(TEXT("ChatConsoleMessage"), *FString::Printf(TEXT("Text_Help_%s[%d]"), Sect, i), TEXT("AVANET"));
		if (Help.Len() == 0)
			continue;
		_StateController->LogChatConsole(*Help, EChat_PlayerSystem);
	}
}

UBOOL CavaNetStateController::ParseChatCommand(const TCHAR *Cmd, UBOOL Team)
{
	_LOG(TEXT("Parsing %s"), Cmd);

	if (Cmd[0] == TEXT('/'))
	{
		if (IsMatchProcessing() && _StateController->PlayerInfo.GetCurrentChannelMaskLevel() <= _CML_PLAYER)
		{
			// 대회 중에는 콘솔 명령어 사용 불가
			_StateController->LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Help_NoConsoleWhileMatch"), TEXT("AVANET")), EChat_PlayerSystem);
			return TRUE;
		}

		TCHAR *Str = (TCHAR*)Cmd + 1;
		FString NextCmd = ParseToken((const TCHAR*&)Str, FALSE).Trim().TrimTrailing();

		switch ( _Locals.GetCmdTypeFromChar(NextCmd) )
		{
		case ECmd_Buddy:
			NextCmd = ParseToken((const TCHAR*&)Str, FALSE).Trim().TrimTrailing();
			switch ( _Locals.GetCmdTypeFromChar(NextCmd, ECmd_Buddy) )
			{
			case ECmd_BuddyAdd:
				{
					FString Who = ParseToken((const TCHAR*&)Str, FALSE);
					if (Who.Len() > 0)
					{
						switch ( _Locals.GetCmdTypeFromChar(Who, ECmd_BuddyAdd) )
						{
						case ECmd_BuddyAddYes:
							Who = ParseToken((const TCHAR*&)Str, FALSE);
							_Communicator().AddBuddyAns(TRUE, Who);
							break;
						case ECmd_BuddyAddNo:
							_Communicator().AddBuddyAns(FALSE, Who);
							break;
						default:
							{
								UBOOL bForce = FALSE;
								if (Who[0] == TEXT('!'))
								{
									bForce = TRUE;
									Who = Who.Right(Who.Len() - 1);
								}

								if (Who.Len() > 0)
								{
									FPlayerDispInfo *Player = FindPlayerFromList(Who);
									if (Player)
									{
										_Communicator().AddBuddy(bForce, Player->PlayerInfo.idAccount, Who);
									}
									else
									{
										FGuildPlayerInfo *GuildPlayer = GuildInfo.PlayerList.Find(Who);
										if (GuildPlayer)
										{
											_Communicator().AddBuddy(bForce, GuildPlayer->GuildPlayerInfo.idAccount, Who);
										}
										else
										{
											FBuddyInfo *Block = _Communicator().BlockList.Find(Who);
											if (Block)
												_Communicator().AddBuddy(bForce, Block->idAccount, Who);
											else
												_Communicator().AddBuddy(bForce, ID_INVALID_ACCOUNT, Who);
										}
									}
								}
							}
							break;
						}
					}
				}
				break;
			case ECmd_BuddyDelete:
				{
					FString Who = ParseToken((const TCHAR*&)Str, FALSE);
					if (Who.Len() > 0)
					{
						_Communicator().DeleteBuddy(Who);
					}
				}
				break;
			case ECmd_BuddyInfo:
				{
					FString Who = ParseToken((const TCHAR*&)Str, FALSE);
					if (Who.Len() > 0)
					{
						for (INT i = 0; i < _Communicator().BuddyList.Num(); ++i)
						{
							if (_Communicator().BuddyList(i).Nickname == Who)
							{
								GetAvaNetRequest()->SelectBuddy(i);
								break;
							}
						}
					}
				}
				break;
			case ECmd_BuddyList:
				{
					FString Flag = ParseToken((const TCHAR*&)Str, FALSE).ToUpper();
					_Communicator().DumpBuddyListToConsole( Flag == TEXT("ON") ? FGuildPlayerList::_DF_ON :
															Flag == TEXT("OFF") ? FGuildPlayerList::_DF_OFF :
															FGuildPlayerList::_DF_ALL );
				}
				break;
			default:
				_ShowHelp(TEXT("Buddy"));
				break;
			}
			break;

		case ECmd_Block:
			NextCmd = ParseToken((const TCHAR*&)Str, FALSE).Trim().TrimTrailing();
			switch ( _Locals.GetCmdTypeFromChar(NextCmd, ECmd_Block) )
			{
			case ECmd_BlockAdd:
				{
					FString Who = ParseToken((const TCHAR*&)Str, FALSE);
					if (Who.Len() > 0)
					{
						UBOOL bForce = FALSE;
						if (Who[0] == TEXT('!'))
						{
							bForce = TRUE;
							Who = Who.Right(Who.Len() - 1);
						}

						if (Who.Len() > 0)
						{
							FPlayerDispInfo *Player = FindPlayerFromList(Who);
							_Communicator().AddBlock(bForce, Player ? Player->PlayerInfo.idAccount : ID_INVALID_ACCOUNT, Who);
						}
					}
				}
				break;
			case ECmd_BlockDelete:
				{
					FString Who = ParseToken((const TCHAR*&)Str, FALSE);
					if (Who.Len() > 0)
					{
						_Communicator().DeleteBlock(Who);
					}
				}
				break;
			case ECmd_BlockList:
				_Communicator().DumpBlockListToConsole();
				break;
			default:
				_ShowHelp(TEXT("Block"));
				break;
			}
			break;

		case ECmd_Guild:
			NextCmd = ParseToken((const TCHAR*&)Str, FALSE).Trim().TrimTrailing();
			switch ( _Locals.GetCmdTypeFromChar(NextCmd, ECmd_Guild) )
			{
			case ECmd_GuildInvite:
				{
					FString Who = ParseToken((const TCHAR*&)Str, FALSE);
					if (Who.Len() > 0)
					{
						switch ( _Locals.GetCmdTypeFromChar(Who, ECmd_GuildInvite) )
						{
						case ECmd_GuildInviteYes:
							Who = ParseToken((const TCHAR*&)Str, FALSE);
							_Communicator().InviteGuildAns(TRUE, Who);
							break;
						case ECmd_GuildInviteNo:
							Who = ParseToken((const TCHAR*&)Str, FALSE);
							_Communicator().InviteGuildAns(FALSE, Who);
							break;
						default:
							_Communicator().InviteGuild(Who);
						}
					}
				}
				break;
			case ECmd_GuildNotice:
				{
					FString NoticeMsg = Str;
					NoticeMsg = NoticeMsg.Trim().TrimTrailing();
					if (NoticeMsg.Len() > 0)
					{
						GetAvaNetRequest()->GuildNotice(NoticeMsg);
					}
				}
				break;
			case ECmd_GuildMotd:
				{
					FString Motd = Str;
					Motd = Motd.Trim().TrimTrailing();
					if (Motd.Len() > 0)
					{
						GetAvaNetRequest()->GuildSetMotd(Motd);
					}
				}
				break;
			case ECmd_GuildLeave:
				{
					GetAvaNetRequest()->GuildLeave();
				}
				break;
			case ECmd_GuildKick:
				{
					FString Who = ParseToken((const TCHAR*&)Str, FALSE);
					if (Who.Len() > 0)
					{
						GetAvaNetRequest()->GuildKick(Who);
					}
				}
				break;
			case ECmd_GuildList:
				{
					if (!GuildInfo.IsValid())
					{
						_LOG(TEXT("Error! You have no guild information."));
						LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoClan"), TEXT("AVANET")), EChat_GuildSystem);
						GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("no guild"), TEXT(""), 0, 0);
						break;
					}
					if (!GuildInfo.IsChannelConnected())
					{
						_LOG(TEXT("Error! Guild channel is not connected."));
						LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Clan_NoConnection"), TEXT("AVANET")), EChat_GuildSystem);
						GetAvaNetHandler()->ProcMessage(EMsg_Guild, EMsg_Guild_Kick, TEXT("not connected"), TEXT(""), 0, 0);
						break;
					}

					FString Flag = ParseToken((const TCHAR*&)Str, FALSE).ToUpper();
					GuildInfo.PlayerList.DumpToConsole( Flag == TEXT("ON") ? FGuildPlayerList::_DF_ON :
														Flag == TEXT("OFF") ? FGuildPlayerList::_DF_OFF :
														FGuildPlayerList::_DF_ALL );
				}
				break;
			default:
				_ShowHelp(TEXT("Clan"));
				break;
			}
			break;

		case ECmd_Follow:
			if (!_StateController->AutoMoveDest.IsMoving())
			{
				FString Who = ParseToken((const TCHAR*&)Str, FALSE);
				if (Who.Len() > 0)
				{
					FString Password = Str;
					Password = Password.Trim().TrimTrailing();
					if (Password.Len() > 0)
						_StateController->AutoMoveDest.RoomPassword = Password;
					_LOG(TEXT("ECmd_Folloer; Who = %s, Password %s"), *Who, *Password);
					GetAvaNetRequest()->FollowPlayer(Who);
				}
			}
			break;

		case ECmd_InviteGame:
			{
				FString Who = ParseToken((const TCHAR*&)Str, FALSE);
				if (Who.Len() > 0)
				{
					switch ( _Locals.GetCmdTypeFromChar(Who, ECmd_InviteGame) )
					{
					case ECmd_InviteGameYes:
						Who = ParseToken((const TCHAR*&)Str, FALSE);
						_Communicator().InviteGameAns(TRUE, Who);
						break;
					case ECmd_InviteGameNo:
						Who = ParseToken((const TCHAR*&)Str, FALSE);
						_Communicator().InviteGameAns(FALSE, Who);
						break;
					default:
						_Communicator().InviteGame(Who);
					}
				}
			}
			break;

		case ECmd_Whisper:
			{
				FString Who = ParseToken((const TCHAR*&)Str, FALSE);
				FString ChatMsg = Str;
				ChatMsg = ChatMsg.Trim().TrimTrailing();
				if (Who.Len() > 0 && ChatMsg.Len() > 0)
				{
					_Communicator().Whisper(Who, ChatMsg);
				}
			}
			break;

		case ECmd_Reply:
			{
				FString ChatMsg = Str;
				ChatMsg = ChatMsg.Trim().TrimTrailing();
				if (ChatMsg.Len() > 0)
				{
					_Communicator().Reply(ChatMsg);
				}
			}
			break;

		case ECmd_GuildChat:
			{
				FString ChatMsg = Str;
				ChatMsg = ChatMsg.Trim().TrimTrailing();
				if (ChatMsg.Len() > 0)
				{
					GetAvaNetRequest()->GuildChat(ChatMsg, FALSE);
				}
			}
			break;

		case ECmd_Help:
			{
				_ShowHelp(TEXT("Console"));
				if (NetState == _AN_ROOM)
				{
					_ShowHelp(TEXT("Room"));
				}
				if (_StateController->PlayerInfo.GetCurrentChannelMaskLevel() >= _CML_REFREE)
				{
					_ShowHelp(TEXT("Match"));
				}
			}
			break;

		case ECmd_SwapTeam:
			GetAvaNetRequest()->RoomSwapTeam();
			break;

		case ECmd_RoomKick:
			if (NetState == _AN_ROOM)
			{
				FString Who = ParseToken((const TCHAR*&)Str, FALSE);
				if (Who.Len() > 0)
					GetAvaNetRequest()->RoomKickPlayer(Who, KR_BAN);
			}
			break;

		case ECmd_GMWhisper:
			{
				FString Who = ParseToken((const TCHAR*&)Str, FALSE);
				FString ChatMsg = Str;
				ChatMsg = ChatMsg.Trim().TrimTrailing();
				if (Who.Len() > 0 && ChatMsg.Len() > 0)
				{
					_Communicator().GMWhisper(Who, ChatMsg);
				}
			}
			break;

		case ECmd_Quit:
			if (!IsStatePlaying())
			{
				//GEngine->Exec(TEXT("QUIT"));
				GetAvaNetRequest()->Quit(TRUE);
			}
			break;

		case ECmd_ClaimHost:
			GetAvaNetRequest()->RoomClaimHost();
			break;

		case ECmd_MatchNotice:
			_LOG(TEXT("ChannelMaskLevel = %d"), _StateController->PlayerInfo.GetCurrentChannelMaskLevel());
			if (_StateController->PlayerInfo.GetCurrentChannelMaskLevel() >= _CML_REFREE)
			{
				// 대회 채널에서 심판 이상만
				TID_CHANNEL idChannel = _StateController->ChannelInfo.idChannel;
				if (idChannel == ID_INVALID_CHANNEL)
					break;

				while (appIsWhitespace(*Str))
					++Str;
				if (*Str == 0)
				{
					_LOG(TEXT("Sending empty notice to channel %d"), idChannel);
					PM::ADMIN::NOTICE_NTF::Send(idChannel, TEXT(""));
					break;
				}

				_LOG(TEXT("Sending notice to channel %d; %s"), idChannel, Str);

				PM::ADMIN::NOTICE_NTF::Send(idChannel, Str);
			}
			break;

		case ECmd_MatchChatOff:
			_LOG(TEXT("ChannelMaskLevel = %d"), _StateController->PlayerInfo.GetCurrentChannelMaskLevel());
			if (_StateController->PlayerInfo.GetCurrentChannelMaskLevel() >= _CML_REFREE)
			{
				// 대회 채널에서 심판 이상만
				FString Who = ParseToken((const TCHAR*&)Str, FALSE);
				while (Who.Len() > 0)
				{
					if (_StateController->PlayerInfo.IsValid() && Who == _StateController->PlayerInfo.PlayerInfo.nickname)
					{
						continue;
					}
					else
					{
						_LOG(TEXT("Blocking chatting of the player '%s'"), *Who);
						PM::ADMIN::CHATOFF_REQ::Send(Who);
						Who = ParseToken((const TCHAR*&)Str, FALSE);
					}
				}
			}
			break;
		}

		return TRUE;
	}

	// 스텔스 상태인 GM은 채팅 불가
	if (AmIStealthMode())
		return TRUE;

	if (RoomInfo.IsValid() && RoomInfo.IsPlaying() && !RoomInfo.PlayerList.IsEmpty(MyRoomSlotIdx) &&
		(RoomInfo.PlayerList.PlayerList[MyRoomSlotIdx].IsPlaying() || AmIHost()))
	{
		if (GetAvaNetHandler()->LoadingCheckTime > 0.0 ||
			GetAvaNetHandler()->RoomStartingPlayerList.Num() > 0 ||
			g_hostMigration.state == hmsNewClientPrepare)
		{
			_LOG(TEXT("GAME::CHAT_NTF: LoadingCheckTime = %.2f, RoomStartingPlayerList.Num = %d, g_hostMigration.state = %d"),
								GetAvaNetHandler()->LoadingCheckTime, GetAvaNetHandler()->RoomStartingPlayerList.Num(), (INT)g_hostMigration.state);
			if (CheckChatBlocked(Cmd))
				return TRUE;

			TCHAR *Msg = (TCHAR*)Cmd;
			_WordCensor().ReplaceChatMsg(Msg);
			if (appStrlen(Msg) == 0)
				return TRUE;

			PM::GAME::CHAT_NTF::Send(Msg, Team);

			SentChatMsgList.Add(Msg);
			while (SentChatMsgList.Num() > 5)
				SentChatMsgList.ChatList.RemoveNode(SentChatMsgList.ChatList.GetHead());

			CheckChatProhibition();

			return TRUE;
		}
	}

	return FALSE;
}

UBOOL CavaNetStateController::CheckChatBlocked(const TCHAR *ChatMsg)
{
	// check if chatting is blocked
	if (appSeconds() < ChatOffDue)
	{
		_LOG(TEXT("Blocked to chat."));
		LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Chat_CannotChatBecauseBlocked"), TEXT("AVANET")), EChat_PlayerSystem);
		return TRUE;
	}

	if (SentChatMsgList.Num() > 0 && appStrlen(ChatMsg) >= 20)
	{
		FChatMsg *Last = SentChatMsgList.At(0);
		if (Last)
		{
			TCHAR *Check = appStrchr(*Last->Msg, TEXT(']'));
			if (Check)
				++Check;
			else
				Check = (TCHAR*)*Last->Msg;
			_LOG(TEXT("%s <-> %s"), Check, ChatMsg);
			if (Last && appSeconds() - Last->AddedTime <= 5.0 && appStrcmp(Check, ChatMsg) == 0)
			{
				LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Chat_CannotChatLongAndSameMsg"), TEXT("AVANET")), EChat_PlayerSystem);
				return TRUE;
			}
		}
	}

	return FALSE;
}

UBOOL CavaNetStateController::CheckChatProhibition()
{
	// check chat prohibition
	UBOOL bChatOff = FALSE;
	if (SentChatMsgList.Num() >= 4)
	{
		// 1초 내 채팅글을 4회 이상 입력했을 경우
		FChatMsg *Last = SentChatMsgList.At(0);
		FChatMsg *First = SentChatMsgList.At(3);
		if (Last && First && Last->AddedTime - First->AddedTime <= 1.0)
		{
			//_LOG(TEXT("Chat blocked; Last = %.2f, First = %.2f"), Last->AddedTime, First->AddedTime);
			bChatOff = TRUE;
		}
	}
	if (!bChatOff && SentChatMsgList.Num() >= 5)
	{
		// 5초 내에 동일한 문장을 5회 이상 입력 했을 경우
		FChatMsg *Last = NULL, *First = NULL;
		INT SameMsgCount = 0;
		for (FChatMsgList::TIterator It(SentChatMsgList.GetHead()); It; ++It)
		{
			FChatMsg &Elem = *It;
			if (It.GetNode() == SentChatMsgList.GetHead())
			{
				First = &Elem;
				continue;
			}
			if (It.GetNode() == SentChatMsgList.GetTail())
				Last = &Elem;

			_LOG(TEXT("%s <-> %s"), *First->Msg, *Elem.Msg);

			if (First && First->Msg == Elem.Msg)
				++SameMsgCount;
		}
		if (SameMsgCount >= 4 && Last->AddedTime - First->AddedTime <= 5.0)
		{
			bChatOff = TRUE;
		}
	}
	if (bChatOff)
	{
		_LOG(TEXT("Chat blocked"));
		LogChatConsole(*Localize(TEXT("ChatConsoleMessage"), TEXT("Text_Chat_Blocked"), TEXT("AVANET")), EChat_PlayerSystem);
		ChatOffDue = appSeconds() + 60.0;
	}

	return bChatOff;
}


void CavaNetStateController::LogChatConsole(const TCHAR *Msg, INT MsgType)
{
	GetAvaNetHandler()->eventChatMessage(Msg, (BYTE)MsgType);
}

void CavaNetStateController::LogChatConsoleNoMatch(const TCHAR *Msg, INT MsgType)
{
	if (_StateController->IsMatchProcessing())
	{
		// 대회 채널에서 게임 중일 때에는 메시지를 화면에 찍지 않고 대기실 채팅 메시지 리스트에 쌓아둔다.
		ChatMsgList.AddWithPrefix(Msg, MsgType);
	}
	else
	{
		GetAvaNetHandler()->eventChatMessage(Msg, (BYTE)MsgType);
	}
}

INT CavaNetStateController::CheckMyLocation()
{
	//if ( !GIsEditor && GIsGame )
	//	_LOG(TEXT("NetState = %d"), NetState);

	if (NetState == _AN_INGAME /*&& RoomInfo.IsValid()*/)
	{
		return 3;
	}
	else if (NetState == _AN_ROOM /*&& RoomInfo.IsValid()*/)
	{
		return 2;
	}
	else if (NetState == _AN_LOBBY /*&& ChannelInfo.IsValid()*/)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

FString CavaNetStateController::GetURLString(INT idAccount, const FString &Option)
{
	if (idAccount == ID_INVALID_ACCOUNT)
	{
		if (PlayerInfo.IsValid())
		{
			if (Option == TEXT("Name"))
			{
				return PlayerInfo.PlayerInfo.nickname;
			}
			else if (Option == TEXT("Team"))
			{
				return appItoa(PlayerInfo.PlayerInfo.lastTeam);
			}
			else if (Option == TEXT("Class") || Option == TEXT("LastClass"))
			{
				return appItoa(PlayerInfo.PlayerInfo.currentClass);
			}
			else if (Option == TEXT("LastWeapon"))
			{
				return appItoa(PlayerInfo.Inven.GetWeaponSet(PlayerInfo.PlayerInfo.currentClass)->id);
			}
			else if (Option == TEXT("LastWeaponInvenIndex"))
			{
				PlayerInfo.Inven.Init(&_ItemDesc(), &PlayerInfo.PlayerInfo.itemInfo);
				TID_ITEM currentWeaponId = PlayerInfo.Inven.GetWeaponSet(PlayerInfo.PlayerInfo.currentClass)->id;

				INT resultInventoryIndex = -1;
				for (INT Index = 0; Index < MAX_INVENTORY_SIZE; Index++)
				{
					if (!PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].IsEmpty())
					{
						ITEM_DESC *ItemDesc = _ItemDesc().GetItem(PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].id);
						if (ItemDesc && ItemDesc->id == currentWeaponId )
						{
							resultInventoryIndex = Index;
							break;
						}
					}
				}
				debugf(TEXT("resultInventoryIndex = %d"),resultInventoryIndex);
				return appItoa( resultInventoryIndex );
			}
			else if (Option == TEXT("ChLevel"))
			{
				return appItoa(PlayerInfo.PlayerInfo.level);
			}
			else if (Option == TEXT("Guild"))
			{
				return PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD ? PlayerInfo.PlayerInfo.guildInfo.guildName : TEXT("");
			}
			else if (Option == TEXT("ChItem"))
			{
				FString Str;
				Str = appItoa((PlayerInfo.PlayerInfo.faceType < 1 || PlayerInfo.PlayerInfo.faceType > 5 ? 1 : PlayerInfo.PlayerInfo.faceType));
				for (INT i = 0; i < MAX_EQUIPSET_SIZE; ++i)
				{
					ITEM_INFO *pInfo = PlayerInfo.Inven.GetEquipSet(i);
					if (pInfo)
						Str += FString::Printf(TEXT(";%d"), pInfo->id);
				}

				return Str;
			}
			else if (Option == TEXT("WeaponP") || Option == TEXT("WeaponR") || Option == TEXT("WeaponS"))
			{
				FString WeapStr;
				INT Mask = (Option == TEXT("WeaponP") ? _EP_WEAP_POINTMAN : Option == TEXT("WeaponR") ? _EP_WEAP_RIFLEMAN : _EP_WEAP_SNIPER);
				for (INT i = 0; i < MAX_WEAPONSET_SIZE; ++i)
				{
					SLOT_DESC *Desc = _ItemDesc().GetWeaponSlot(i);
					if (Desc && (Desc->slotType & Mask))
					{
						ITEM_INFO *pInfo = PlayerInfo.Inven.GetWeaponSet(i);
						if (pInfo)
						{
							WeapStr += FString::Printf(TEXT(";%d"), pInfo->id);

							if (Desc->slotType & _EP_WEAP_CUSTOMIZABLE)
							{
								for (INT j = 0; j < MAX_CUSTOM_INVENTORY_SIZE; ++j)
								{
									CUSTOM_ITEM_INFO &CustInfo = PlayerInfo.PlayerInfo.itemInfo.customWeapon[j];
									if (CustInfo.item_sn == pInfo->sn && CustInfo.slot != _CSI_NONE)
									{
										WeapStr += FString::Printf(TEXT("@%d"), CustInfo.id);
									}
								}
							}

							ITEM_DESC *pItemDesc = _ItemDesc().GetItem(pInfo->id);
							if (!pItemDesc)
								_LOG(TEXT("ItemDesc not found! id = %d"), pInfo->id);
							if (pItemDesc && pItemDesc->gaugeType == _IGT_MAINTENANCE)
							{
								WeapStr += FString::Printf(TEXT("*%d"), (100 * pInfo->limit / ITEM_LIMIT_INITED));
							}
						}
					}
				}

				if (WeapStr.Len() > 0)
					return *WeapStr + 1;
			}
			else if (Option == TEXT("SkillP"))
			{
				FString SkillStr(TEXT("8449"));
				for (INT i = 0; i < MAX_SKILL_PER_CLASS; ++i)
				{
					if (PlayerInfo.PlayerInfo.skillInfo.skill[_CLASS_POINTMAN] & (1 << i))
					{
						SkillStr += FString::Printf(TEXT(";%d"), 51201 + i);
					}
				}
				return *SkillStr;
			}
			else if (Option == TEXT("SkillR"))
			{
				FString SkillStr(TEXT("8450"));
				for (INT i = 0; i < MAX_SKILL_PER_CLASS; ++i)
				{
					if (PlayerInfo.PlayerInfo.skillInfo.skill[_CLASS_RIFLEMAN] & (1 << i))
					{
						SkillStr += FString::Printf(TEXT(";%d"), 51457 + i);
					}
				}
				return *SkillStr;
			}
			else if (Option == TEXT("SkillS"))
			{
				FString SkillStr(TEXT("8451"));
				for (INT i = 0; i < MAX_SKILL_PER_CLASS; ++i)
				{
					if (PlayerInfo.PlayerInfo.skillInfo.skill[_CLASS_SNIPER] & (1 << i))
					{
						SkillStr += FString::Printf(TEXT(";%d"), 51713 + i);
					}
				}
				return *SkillStr;
			}
			else if (Option == TEXT("LeaderScore"))
			{
				return *appItoa(PlayerInfo.PlayerInfo.scoreInfo.score.leader);
			}
			else if (Option == TEXT("ClanMarkID"))
			{
				return *appItoa(PlayerInfo.GetClanMarkID());
			}
			else if ( appStrnicmp(*Option, TEXT("CustomWeapon"), 12) == 0 )
			{
				// 소지하는 무기 아이템의 장착 무기 ID 리스트를 얻어온다.(2007/02/09 고광록)
				FString			CustomWeaponStr;
				const TCHAR*	Code   = (const TCHAR*)(*Option) + 12;
				INT				ItemID = appAtoi(Code);
				TSN_ITEM		sn     = SN_INVALID_ITEM;

				// 인벤토리를 찾아서 해당 ItemID와 같은 무기를 찾는다.
				for (INT Index = 0; Index < MAX_INVENTORY_SIZE; Index++)
				{
					if ( PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].IsEmpty() )
						continue;

					// 해당무기를 찾아서 Global Unique Item ID(GUII)를 얻어온다.
					if ( PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].id == ItemID )
					{
						sn = PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].sn;
						break;
					}
				}

				// 만약 찾았다면.
				if ( sn != SN_INVALID_ITEM )
				{
					for (INT j = 0; j < MAX_CUSTOM_INVENTORY_SIZE; ++j)
					{
						CUSTOM_ITEM_INFO &customWeapon = PlayerInfo.PlayerInfo.itemInfo.customWeapon[j];

						// 해당 무기의 장착 아이템인지 확인해서
						if ( customWeapon.item_sn == sn )
							CustomWeaponStr += FString::Printf(TEXT(";%d"), customWeapon.id);
					}
				}

				return CustomWeaponStr.Len() > 0 ? (*CustomWeaponStr) + 1 : CustomWeaponStr;
			}
			else if ( appStrnicmp(*Option, TEXT("WeaponSN"), 8) == 0 )
			{
				// 소지하는 무기 아이템의 장착 무기 ID 리스트를 얻어온다.(2007/02/27 고광록)
				FString			WeaponStr;
				const TCHAR*	Code   = (const TCHAR*)(*Option) + 8;
				__int64			ItemSN = _tstoi64(Code);
				TSN_ITEM		sn     = ItemSN;
				INT				ItemID;

				// 인벤토리를 찾아서 해당 Serial Number와 같은 무기를 찾는다.
				for (INT Index = 0; Index < MAX_INVENTORY_SIZE; Index++)
				{
					if ( PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].IsEmpty() )
						continue;

					// 해당무기를 Global Unique Item ID(GUII)를 해당무기를 얻어온다.
					if ( PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].sn == sn )
					{
						ItemID = PlayerInfo.PlayerInfo.itemInfo.weaponInven[Index].id;

						WeaponStr += FString::Printf(TEXT("%d"), ItemID);
						break;
					}
				}

				// 만약 찾았다면.
				if ( sn != SN_INVALID_ITEM )
				{
					for (INT j = 0; j < MAX_CUSTOM_INVENTORY_SIZE; ++j)
					{
						CUSTOM_ITEM_INFO &customWeapon = PlayerInfo.PlayerInfo.itemInfo.customWeapon[j];

						// 해당 무기의 장착 아이템인지 확인해서
						if ( customWeapon.item_sn == sn )
							WeaponStr += FString::Printf(TEXT(";%d"), customWeapon.id);
					}
				}

				return WeaponStr;
			}
		}
	}
	else
	{
		FRoomPlayerInfo *Info = RoomInfo.PlayerList.Find(idAccount);
		if (Info != NULL)
		{
			if (Option == TEXT("Name"))
			{
				return Info->PlayerInfo.nickname;
			}
			else if (Option == TEXT("Team"))
			{
				return appItoa(Info->GetTeamID());
			}
			else if (Option == TEXT("Class"))
			{
				return appItoa(Info->RoomPlayerInfo.currentClass);
			}
			else if (Option == TEXT("ChLevel"))
			{
				return appItoa(Info->PlayerInfo.level);
			}
			else if (Option == TEXT("Guild"))
			{
				return Info->PlayerInfo.guildName;
			}
			else if (Option == TEXT("ChItem"))
			{
				FString Str;
				Str = appItoa((Info->RoomPlayerInfo.faceType < 1 || Info->RoomPlayerInfo.faceType > 5 ? 1 : Info->RoomPlayerInfo.faceType));
				for (INT i = 0; i < MAX_EQUIPSET_SIZE; ++i)
				{
					TID_ITEM ItemID = Info->RoomPlayerInfo.equipItem[i];
					if (ItemID != ID_INVALID_ITEM)
					{
						Str += FString::Printf(TEXT(";%d"), ItemID);
					}
					else
					{
						// slot is empty; check if default item exists and add its id to the list
						SLOT_DESC *Desc = _ItemDesc().GetEquipSlot(i);
						if (Desc)
						{
							if (Desc->defaultItem != ID_INVALID_ITEM)
							{
								Str += FString::Printf(TEXT(";%d"), Desc->defaultItem);
							}
						}
					}
				}

				_LOG(TEXT("%s=%s"), *Option, *Str);

				return Str;
			}
			else if (Option == TEXT("WeaponP") || Option == TEXT("WeaponR") || Option == TEXT("WeaponS"))
			{
				FString WeapStr;
				INT Mask = (Option == TEXT("WeaponP") ? _EP_WEAP_POINTMAN : Option == TEXT("WeaponR") ? _EP_WEAP_RIFLEMAN : _EP_WEAP_SNIPER);
				INT ClassID = (Option == TEXT("WeaponP") ? _CLASS_POINTMAN : Option == TEXT("WeaponR") ? _CLASS_RIFLEMAN : _CLASS_SNIPER);
				for (INT i = 0; i < MAX_WEAPONSET_SIZE; ++i)
				{
					SLOT_DESC *Desc = _ItemDesc().GetWeaponSlot(i);
					if (Desc && (Desc->slotType & Mask))
					{
						ROOM_ITEM_INFO &ItemInfo = Info->RoomPlayerInfo.weaponItem[i];
						if (!ItemInfo.IsEmpty()/*Info->RoomPlayerInfo.weaponItem[i] != ID_INVALID_ACCOUNT*/)
						{
							WeapStr += FString::Printf(TEXT(";%d"), /*Info->RoomPlayerInfo.weaponItem[i]*/ItemInfo.id);

							if (Desc->slotType & _EP_WEAP_PRIMARY)
							{
								for (INT csi = 0; csi < _CSI_MAX; ++csi)
								{
									if (Info->RoomPlayerInfo.customItem[ClassID][csi] != ID_INVALID_ITEM)
									{
										CUSTOM_ITEM_DESC *CustDesc = _ItemDesc().GetCustomItem(Info->RoomPlayerInfo.customItem[ClassID][csi]);
										if (CustDesc && CustDesc->item_id == ItemInfo.id/*Info->RoomPlayerInfo.weaponItem[i]*/)
											WeapStr += FString::Printf(TEXT("@%d"), Info->RoomPlayerInfo.customItem[ClassID][csi]);
									}
								}
							}

							ITEM_DESC *pItemDesc = _ItemDesc().GetItem(ItemInfo.id);
							if (pItemDesc && pItemDesc->gaugeType == _IGT_MAINTENANCE)
							{
								WeapStr += FString::Printf(TEXT("*%d"), ItemInfo.limitPerc);
							}
						}
						else
						{
							// slot is empty; check if default item exists and add its id to the list
							if (Desc->defaultItem != ID_INVALID_ITEM)
							{
								WeapStr += FString::Printf(TEXT(";%d"), Desc->defaultItem);
							}
						}
					}
				}

				if (WeapStr.Len() > 0)
				{
					_LOG(TEXT("%s=%s"), *Option, *WeapStr + 1);
					return *WeapStr + 1;
				}
			}
			else if (Option == TEXT("SkillP"))
			{
				FString SkillStr(TEXT("8449"));
				for (INT i = 0; i < 16; ++i)
				{
					if (Info->RoomPlayerInfo.skillInfo.skill[_CLASS_POINTMAN] & (1 << i))
					{
						SkillStr += FString::Printf(TEXT(";%d"), 51201 + i);
					}
				}
				_LOG(TEXT("%s=%s"), *Option, *SkillStr);
				return *SkillStr;
			}
			else if (Option == TEXT("SkillR"))
			{
				FString SkillStr(TEXT("8450"));
				for (INT i = 0; i < 16; ++i)
				{
					if (Info->RoomPlayerInfo.skillInfo.skill[_CLASS_RIFLEMAN] & (1 << i))
					{
						SkillStr += FString::Printf(TEXT(";%d"), 51457 + i);
					}
				}
				_LOG(TEXT("%s=%s"), *Option, *SkillStr);
				return *SkillStr;
			}
			else if (Option == TEXT("SkillS"))
			{
				FString SkillStr(TEXT("8451"));
				for (INT i = 0; i < 16; ++i)
				{
					if (Info->RoomPlayerInfo.skillInfo.skill[_CLASS_SNIPER] & (1 << i))
					{
						SkillStr += FString::Printf(TEXT(";%d"), 51713 + i);
					}
				}
				_LOG(TEXT("%s=%s"), *Option, *SkillStr);
				return *SkillStr;
			}
			else if (Option == TEXT("LeaderScore"))
			{
				return *appItoa(Info->RoomPlayerInfo.leaderScore);
			}
			else if (Option == TEXT("ClanMarkID"))
			{
				return *appItoa(Info->GetClanMarkID());
			}
		}
	}

	return TEXT("");
}

void CavaNetStateController::GetOpenURL(FURL &OpenURL)
{
	OpenURL = FURL(0x0); // 20070213 dEAthcURe|HM

	FRoomPlayerInfo *MyInfo = GetMyRoomPlayerInfo();
	if (!RoomInfo.IsValid() || !MyInfo)
		return;

	//OpenURL.AddOption(TEXT("LAN"));

	if (AmIHost())
	{
		FMapInfo *Info = GetCurrentMap();
		if (Info)
		{
			OpenURL.Map = Info->FileName;
		}
		else
		{
			_LOG(TEXT("Map not found; loading default map"));
			OpenURL.Map = TEXT("SW-BO.ut3");
		}

		OpenURL.AddOption(TEXT("Listen"));

		//OpenURL.AddOption(TEXT("MinNetPlayers=2"));
		OpenURL.AddOption(*FString::Printf(TEXT("WinCondition=%d"), RoomInfo.RoomInfo.setting.roundToWin));
		OpenURL.AddOption(*FString::Printf(TEXT("FFType=%d"), RoomInfo.RoomInfo.setting.tkLevel));
		OpenURL.AddOption(*FString::Printf(TEXT("TPCam=%d"), RoomInfo.RoomInfo.setting.allowBackView ? 1 : 0));
		OpenURL.AddOption(*FString::Printf(TEXT("GhostChat=%d"), RoomInfo.RoomInfo.setting.allowGhostChat ? 1 : 0));
		OpenURL.AddOption(*FString::Printf(TEXT("SwapRule=%d"), RoomInfo.RoomInfo.setting.mapOption > 0 ? 1 : 0));
	}
	else
	{
		FString *HostAddress = GavaNetClient->Settings.Find(CFG_HOSTADDRESS);

		if (HostAddress)
			OpenURL.Host = *HostAddress;
		else
			OpenURL.Host = TEXT("127.0.0.1");

		//#ifdef EnableP2pConn
		//// {{ 20070124 dEAthcURe
		//if(GavaNetClient->pp2pConn) {
		//	SOCKADDR_IN* pAddr = (SOCKADDR_IN*)&GavaNetClient->pp2pConn->hostSockAddr;
		//	if(pAddr->sin_addr.S_un.S_addr) {			
		//		OpenURL.Host = FString::Printf(TEXT("%d.%d.%d.%d"), pAddr->sin_addr.S_un.S_un_b.s_b1,pAddr->sin_addr.S_un.S_un_b.s_b2,pAddr->sin_addr.S_un.S_un_b.s_b3,pAddr->sin_addr.S_un.S_un_b.s_b4);
		//	}
		//	else {
		//		OpenURL.Host = TEXT("127.0.0.1");
		//	}
		//}
		//// }} 20070124 dEAthcURe
		//#endif

		OpenURL.Map = TEXT("");

#ifdef _AVANET_PRELOAD
		FMapInfo *Info = GetCurrentMap();
		if (Info)
		{
			OpenURL.AddOption(*FString::Printf(TEXT("PreLoad=%s"), *Info->FileName));
		}
		else
		{
			_LOG(TEXT("Map not found; loading default map"));
		}
#endif
	}

	// add option strings
	OpenURL.AddOption(*FString::Printf(TEXT("Name=%s"), PlayerInfo.PlayerInfo.nickname));
	OpenURL.AddOption(*FString::Printf(TEXT("AID=%d"), PlayerInfo.PlayerInfo.idAccount));
	OpenURL.AddOption(*FString::Printf(TEXT("ChLevel=%d"), PlayerInfo.PlayerInfo.level));

	//if (PlayerInfo.PlayerInfo.guildInfo.idGuild != ID_INVALID_GUILD)
	//{
	//	OpenURL.AddOption(*FString::Printf(TEXT("Guild=%s"), PlayerInfo.PlayerInfo.guildInfo.guildName));
	//}

	if (AmIStealthMode())
	{
		OpenURL.AddOption(TEXT("SpectatorOnly=1"));
		OpenURL.AddOption(TEXT("bSilentLogIn=1"));
		return;
	}

	if (MyInfo->GetTeamID() == RT_SPECTATOR)
	{
		OpenURL.AddOption(TEXT("SpectatorOnly=1"));
		return;
	}

	OpenURL.AddOption(*FString::Printf(TEXT("Team=%d"), MyInfo->GetTeamID()));
	OpenURL.AddOption(*FString::Printf(TEXT("Class=%d"), MyInfo->RoomPlayerInfo.currentClass));

	OpenURL.AddOption(*FString::Printf(TEXT("LeaderScore=%d"), MyInfo->RoomPlayerInfo.leaderScore));

	OpenURL.AddOption(*FString::Printf(TEXT("SDR=%.2f"), PlayerInfo.PlayerInfo.scoreInfo.GetSDRatio()));

	// {{ 20070509 dEAthcURe|HM HM접속인지 구분
	if(g_hostMigration.state == hmsNewClientPrepare) {
		OpenURL.AddOption(TEXT("HM"));
		debugf(TEXT("[GetOpenURL] Add option 'HM' to OpenURL."));
	}
	// }} 20070509 dEAthcURe|HM HM접속인지 구분
}


UBOOL CavaNetStateController::ChatSave()
{
	if (ChatMsgList.ChatList.Num() == 0)
		return FALSE;

	FString FileName;
	FString *UserID = GavaNetClient->Settings.Find(CFG_USERID);
	if (UserID)
		FileName = FString::Printf(TEXT("%sChat-%s-%s.log"), *appGameLogDir(), *appSystemTimeString(), **UserID);
	else
		FileName = FString::Printf(TEXT("%sChat-%s-unknown.log"), *appGameLogDir(), *appSystemTimeString());

	FILE *fp = _tfopen(*FileName, TEXT("w"));
	if (!fp)
		return FALSE;

	for (FChatMsgList::TIterator It(ChatMsgList.GetHead()); It; ++It)
	{
//#if UNICODE
//		char buf[1024];
//		WideCharToMultiByte(CP_ACP, 0, **It, -1, buf, 1024, NULL, NULL);
//		fputs(buf, fp);
//#else
//		fputs(**It, fp);
//#endif
		fputs(TCHAR_TO_ANSI(*FString::Printf(TEXT("(%s) %s"), *(*It).GetAddedTime(TRUE), **It)), fp);
		fputs("\n", fp);
	}

	fclose(fp);

	return TRUE;
}


void CavaNetStateController::SyncPlayerLevel(Def::TID_ACCOUNT idAccount, INT Level)
{
	if (GuildInfo.IsValid())
	{
		FGuildPlayerInfo *Info = GuildInfo.PlayerList.Find(idAccount);
		if (Info)
		{
			if (Info->GuildPlayerInfo.level < Level)
			{
				Info->GuildPlayerInfo.level = Level;
				Info->SetFullInfo(FALSE);
			}
		}
	}

	{
		FBuddyInfo *Info = _Communicator().BuddyList.Find(idAccount);
		if (Info)
		{
			if (Info->Level < Level)
			{
				Info->Level = Level;
				Info->SetFullInfo(FALSE);
			}
		}
	}
}

FString CavaNetStateController::GetLocationString( UBOOL bOnline, Def::TID_CHANNEL idChannel, Def::TID_ROOM idRoom/* = Def::ID_INVALID_ROOM*/ )
{
	if (!bOnline)
		return TEXT("Offline");

	if (idRoom != USHRT_MAX)
	{
		if (idChannel == ID_MY_CLAN_HOME)
		{
			FString NameList = Localize(TEXT("Channel"), *FString::Printf(TEXT("Text_ChannelName[%d]"), (INT)EChannelFlag_MyClan), TEXT("AVANET"));
			FString ShortName, LongName;
			NameList.Split(TEXT("|"), &ShortName, &LongName);
			return ShortName;
		}
		else
		{
			FChannelInfo *Channel = ChannelList.Find(idChannel);
			if (Channel)
			{
				FString LocStr(Channel->ChannelName);
				if (idRoom != ID_INVALID_ROOM)
					LocStr += FString::Printf(TEXT(" / %d"), idRoom);
				else
					LocStr += FString::Printf(TEXT(" / %s"), *Localize(TEXT("UILobbyScene"), TEXT("Text_To_Lobby"), TEXT("AVANET")));
				return LocStr;
			}
		}
	}

	// 채널 선택 중
	return Localize(TEXT("Channel"), TEXT("Text_Loc_ChannelList"), TEXT("AVANET"));
	//return TEXT("-");
}
