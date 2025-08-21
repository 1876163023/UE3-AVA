//=============================================================================
//  avaLocalizedMessage
// 
//  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
//  
//  게임내 Localized Message 를 모아둔 Class 이다.
//=============================================================================

class avaLocalizedMessage extends avaLocalMessage;

var localized string Not_In_Bomb_Area;				// 폭탄 설치 지역이 아닙니다.

var	localized string NextPage;						// "다음"
var localized string PrevPage;						// "이전"
var localized string Cancel;						// "취소"

var localized string TargetCancel;					// "설정취소"
var localized string CommandCenterLabel;			// 지휘통제소.

var localized string NoInputKickWarning;			// "몇초동안 입력이 없으면 방에서 강제퇴장 당합니다."
var localized string TakeScreenShot;				// "
var localized string strChangeSpectatorSpeed;
var localized string strChangeFOV;

var localized string NotifySavedCamPos;				// 카메라 위치 저장 안내
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