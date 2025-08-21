/*
	avaUICharacter의 캐릭터의 여러가지를 변경하기 위한 액션.

	2007/02/01	고광록
		bForceLocal을 추가하여 생성창에서 로컬의 기본 정보만으로 캐릭터를 구성할 때 사용.s

	2007/01/25	고광록
*/
class avaSeqAct_UICharacterEvent extends SequenceAction;

/*! @brief avaUICharacter에서 사용하는 이벤트.
	@note
		StrValue와 함께 사용하여 여러가지 이벤트를 통합해서 처리한다.
*/
enum EUICharEvent
{
	UICE_None,

	UICE_ChangeCharacter,				//!< 캐릭터 변경.
	UICE_ChangeCharacterMod,			//!< 캐릭터 아이템 변경.
	UICE_ChangeWeapon,					//!< 무기 변경(with Default-CustomWeapons).
	UICE_ChangeWeaponMod,				//!< 무기 장착 아이템 변경(Only CustomWeapons).

	UICE_SetChannelAnimTree,			//!< Create, Channel에서 기본 애니메이션 트리 설정.
	UICE_SetReadyAnimTree,				//!< Lobby, Shop, Room에서 대기상태 애니메이션 트리 설정.

	UICE_PlayReadyTurn,					//!< 대기창 뒤돌기 동작 시작(끝나고 자동 대기동작이 된다).

	// 팀, 병과 변경 추가.(2007/01/30)
	UICE_ChangePointMan,				//!< PointMan 병과 변경.
	UICE_ChangeRifleMan,				//!< RifleMan 병과 변경.
	UICE_ChangeSniper,					//!< Sniper 병과 변경.

	UICE_ChangeTeamEU,					//!< EU 팀 변경.
	UICE_ChangeTeamNRF,					//!< NRF 팀 변경.

	// 정확히는 LocalPlayerController의 PlayerModifierInfo의 정보로 변경.(2007/02/06)
	UICE_ChangeServerCharacter,			//!< 서버에 받은 정보에서 캐릭터로 변경(무기 포함).
	UICE_ChangeServerWeapon,			//!< 서버에 받은 정보에서 Primary무기로 변경.

	// 장착 무기에 대한 적용 방법 추가.(2007/02/09)
//	UICE_ChangeWeaponWithDefaultMod,	//!< '무기와 기본 장착 아이템'으로 변경.
	UICE_ChangeInventoryWeapon,			//!< '무기와 소유하는 장착 아이템'으로 변경.

	UICE_ChangeServerTeamClass,			//!< 서버에서 받은 정보에서 팀과 병과에 해당하는 (기본) 캐릭터로 변경.

	UICE_InitReadyTurn,					//!< 뒤돌아 보고 있다면 ReadyTurn동작으로 초기화.

	UICE_RemoveCharacterHelmet,			//!< Remove Helmet

//	UICE_ChangeClassByWeapon,			//!< 무기 정보로 캐릭터 병과 변경.
//	UICE_PlayReadyIdle,					//!< 대기창 대기 동작.
//	UICE_PlayCreateIdle,				//!< 생성창 대기 동작.
};


//! 이벤트를 저장.
var() EUICharEvent				Event;

//! 이벤트에 해당하는 문자열 값.
var string						StrValue;

defaultproperties
{
	bCallHandler = true
	ObjCategory="Actor"
	ObjName="ava UICharacterEvent"
	
//	VariableLinks(0)=(ExpectedType=class'SeqVar_Object', LinkDesc="Actor", PropertyName=Actor, MaxVars=1)

	// 0번은 Actor로 예약되어 있다.
	VariableLinks(1)=(ExpectedType=class'SeqVar_String', LinkDesc="StrValue", PropertyName=StrValue, MaxVars=1)

	Event = UICE_None
}