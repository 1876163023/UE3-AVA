/*
	avaUICharacter�� ĳ������ ���������� �����ϱ� ���� �׼�.

	2007/02/01	����
		bForceLocal�� �߰��Ͽ� ����â���� ������ �⺻ ���������� ĳ���͸� ������ �� ���.s

	2007/01/25	����
*/
class avaSeqAct_UICharacterEvent extends SequenceAction;

/*! @brief avaUICharacter���� ����ϴ� �̺�Ʈ.
	@note
		StrValue�� �Բ� ����Ͽ� �������� �̺�Ʈ�� �����ؼ� ó���Ѵ�.
*/
enum EUICharEvent
{
	UICE_None,

	UICE_ChangeCharacter,				//!< ĳ���� ����.
	UICE_ChangeCharacterMod,			//!< ĳ���� ������ ����.
	UICE_ChangeWeapon,					//!< ���� ����(with Default-CustomWeapons).
	UICE_ChangeWeaponMod,				//!< ���� ���� ������ ����(Only CustomWeapons).

	UICE_SetChannelAnimTree,			//!< Create, Channel���� �⺻ �ִϸ��̼� Ʈ�� ����.
	UICE_SetReadyAnimTree,				//!< Lobby, Shop, Room���� ������ �ִϸ��̼� Ʈ�� ����.

	UICE_PlayReadyTurn,					//!< ���â �ڵ��� ���� ����(������ �ڵ� ��⵿���� �ȴ�).

	// ��, ���� ���� �߰�.(2007/01/30)
	UICE_ChangePointMan,				//!< PointMan ���� ����.
	UICE_ChangeRifleMan,				//!< RifleMan ���� ����.
	UICE_ChangeSniper,					//!< Sniper ���� ����.

	UICE_ChangeTeamEU,					//!< EU �� ����.
	UICE_ChangeTeamNRF,					//!< NRF �� ����.

	// ��Ȯ���� LocalPlayerController�� PlayerModifierInfo�� ������ ����.(2007/02/06)
	UICE_ChangeServerCharacter,			//!< ������ ���� �������� ĳ���ͷ� ����(���� ����).
	UICE_ChangeServerWeapon,			//!< ������ ���� �������� Primary����� ����.

	// ���� ���⿡ ���� ���� ��� �߰�.(2007/02/09)
//	UICE_ChangeWeaponWithDefaultMod,	//!< '����� �⺻ ���� ������'���� ����.
	UICE_ChangeInventoryWeapon,			//!< '����� �����ϴ� ���� ������'���� ����.

	UICE_ChangeServerTeamClass,			//!< �������� ���� �������� ���� ������ �ش��ϴ� (�⺻) ĳ���ͷ� ����.

	UICE_InitReadyTurn,					//!< �ڵ��� ���� �ִٸ� ReadyTurn�������� �ʱ�ȭ.

	UICE_RemoveCharacterHelmet,			//!< Remove Helmet

//	UICE_ChangeClassByWeapon,			//!< ���� ������ ĳ���� ���� ����.
//	UICE_PlayReadyIdle,					//!< ���â ��� ����.
//	UICE_PlayCreateIdle,				//!< ����â ��� ����.
};


//! �̺�Ʈ�� ����.
var() EUICharEvent				Event;

//! �̺�Ʈ�� �ش��ϴ� ���ڿ� ��.
var string						StrValue;

defaultproperties
{
	bCallHandler = true
	ObjCategory="Actor"
	ObjName="ava UICharacterEvent"
	
//	VariableLinks(0)=(ExpectedType=class'SeqVar_Object', LinkDesc="Actor", PropertyName=Actor, MaxVars=1)

	// 0���� Actor�� ����Ǿ� �ִ�.
	VariableLinks(1)=(ExpectedType=class'SeqVar_String', LinkDesc="StrValue", PropertyName=StrValue, MaxVars=1)

	Event = UICE_None
}