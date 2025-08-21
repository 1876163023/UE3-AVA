class avaLocalizedTeamPack extends Object
	abstract;

//	Stress Level별 SoundCue Data....
struct PlayerVoiceData
{
	var	SoundCue	snd[3];
};

var PlayerVoiceData	LeaderSound[20];		// avaLeaderMessage 의 Index 와 맞춰야 함
var PlayerVoiceData	MemberSound[20];		// avaMemberMessage 의 Index 와 맞춰야 함
var	PlayerVoiceData	CommandSound[8];		// avaRadioCommandMessage 의 Index 와 맞춰야 함
var	PlayerVoiceData	ResponseSound[8];		// avaRadioResponseMessage 의 Index 와 맞춰야 함
var PlayerVoiceData	AutoSound[12];			// avaRadioAutoMessage 의 Index 와 맞춰야 함


