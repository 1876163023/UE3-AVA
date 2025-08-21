class avaUIGameString extends avaUISimpleText native;

enum avaUIPlayerSummaryType
{
	AVAUISummary_Level,
	AVAUISummary_Name,
	AVAUISummary_GuildName,
	AVAUISummary_Class,
	AVAUISummary_Score,
	AVAUISummary_Kill,
	AVAUISummary_Death,
	AVAUISummary_Exp,
	AVAUISummary_Status,
	AVAUISummary_Supply,
	AVAUISummary_Bonus,
	AVAUISummary_NextClass,
	AVAUISummary_Ping,
	AVAUISummary_Host,
	AVAUISummary_Help,
	AVAUISummary_Leader,
	AVAUISummary_SpectatorHelp,
	AVAUISummary_CurrentWeapon,
	AVAUISummary_PracticeHelp,
	AVAUISummary_ClanMarkIcon,
	AVAUISummary_SlotNum,
	AVAUISummary_Rank,
};

var()	avaUIPlayerSummaryType	SummaryType;

cpptext
{
	virtual UBOOL UpdateString();		
}
