//=============================================================================
//  avaLocalizedMessage
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//  ���ӳ� Localized Message �� ��Ƶ� Class �̴�.
//=============================================================================

class avaLocalizedMessage extends avaLocalMessage;

var localized string Not_In_Bomb_Area;				// ��ź ��ġ ������ �ƴմϴ�.

var	localized string NextPage;						// "����"
var localized string PrevPage;						// "����"
var localized string Cancel;						// "���"

var localized string TargetCancel;					// "�������"
var localized string CommandCenterLabel;			// ����������.

var localized string NoInputKickWarning;			// "���ʵ��� �Է��� ������ �濡�� �������� ���մϴ�."
var localized string TakeScreenShot;				// "
var localized string strChangeSpectatorSpeed;
var localized string strChangeFOV;

var localized string NotifySavedCamPos;				// ī�޶� ��ġ ���� �ȳ�
var localized string NotifyCameraCollisionOn;
var localized string NotifyCameraCollisionOff;
var localized string NotifyDisableCameraEffect;
var localized string NotifyEnableCameraEffect;





struct	LocationNameData
{
	var	int					id;
	var localized string	LocationName;
};

var array<LocationNameData>		LocationNameList;