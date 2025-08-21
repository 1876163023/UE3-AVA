
// ***  avaGameSkillMessage is expired!!!

class avaGameSkillMessage extends avaLocalMessage;



//var localized string Skill_Acquire;
//var localized string Skill_Cancel;
//var localized string SkillName[10];
//
//// avaGame.kor의 Localized String들과 
//// 갯수, 순서가 일치해야함.
//enum EGameSkillType
//{
//	GAMESKILL_PistolPinPoint,
//	GAMESKILL_SMGPinPoint,
//	GAMESKILL_RiflePinPoint,
//	GAMESKILL_SniperPinPoint,
//	GAMESKILL_GoldenBullet,
//	GAMESKILL_Trainee,			//훈련병 보호
//	GAMESKILL_BetterArmor,		//신형 방탄복
//	GAMESKILL_TightNeck,
//	GAMESKILL_HardLife,			//구사일생
//	GAMESKILL_GreenBeret,
//};
//
//
//static function ClientReceive(
//	PlayerController P,
//	optional int Switch,
//	optional PlayerReplicationInfo RelatedPRI_1,
//	optional PlayerReplicationInfo RelatedPRI_2,
//	optional Object OptionalObject
//	)
//{
//	Local avaHUD avaHUD;
//	Local string Msg, Key;
//	Local avaMsgParam Param;
//	Local bool bCancel;
//	
//	Param = avaMsgParam(OptionalObject);
//	avaHUD = avaHUD(P.myHUD);
//	Assert( avaHUD != None && Param != None);
//	if( avaHUD == None || Param == None )
//		return;
//
//	Key = Switch$"";
//	Play(Key, RelatedPRI_1, P);
//
//	bCancel = Param.BoolParam1;
//	Msg = bCancel ? default.Skill_Cancel : default.Skill_Acquire;
//	Msg = Msg $ default.SkillName[Switch];
//	avaHUD.GameInfoMessage(Msg);
//}
//
//
//
//defaultproperties
//{
//	MessageData.Add((Key="0", MsgIndex=GAMESKILL_PistolPinPoint, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="1", MsgIndex=GAMESKILL_SMGPinPoint, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="2", MsgIndex=GAMESKILL_RiflePinPoint, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="3", MsgIndex=GAMESKILL_SniperPinPoint, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="4", MsgIndex=GAMESKILL_GoldenBullet, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_GoldenBullet', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="5", MsgIndex=GAMESKILL_Trainee, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_Newbie', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="6", MsgIndex=GAMESKILL_BetterArmor, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_BetterArmor', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="7", MsgIndex=GAMESKILL_TightNeck, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_TightNeck', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="8", MsgIndex=GAMESKILL_HardLife, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_941', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//	MessageData.Add((Key="9", MsgIndex=GAMESKILL_GreenBeret, SoundCue[0]=SoundCue'avaGameVoices.SkillVoices.SKL_GreenBeret', SoundCue[1]=SoundCue'avaGameVoices.SkillVoices.SKL_PinPoint'))
//
//	// INDISPENSIBLE FOR BINARY SEARCH ( ref. avaLocalMessage ). DO NOT DELETE
//	MessageData.Sort(Key)
//}