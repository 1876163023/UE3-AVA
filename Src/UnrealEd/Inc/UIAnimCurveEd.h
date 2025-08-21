/*=============================================================================
	UIAnimCurveEd.h: Specified FInterpCurve Editor for UIAnimation

=============================================================================*/

#ifndef __UIANIMCURVEED_H__
#define __UIANIMCURVEED_H__


#include "CurveEd.h"
#include "UnObjectEditor.h"
#include "UnUIEditor.h"


class WxUIAnimCurveEd : public WxCurveEditor, public FCurveEdNotifyInterface, public FCallbackEventDevice
{
public:
	WxUIAnimCurveEd( WxUIEditorBase* InUIEditor, wxWindowID InID, class UInterpCurveEdSetup* InEdSetup, UUIAnimationSeq* InAnimSeq );
	~WxUIAnimCurveEd();

	/* FCurveEdNotifyInterface */
	virtual void PreEditCurve(TArray<UObject*> CurvesAboutToChange);
	virtual void PostEditCurve();

	virtual void MovedKey() {}

	virtual void DesireUndo() {}
	virtual void DesireRedo() {}

	virtual UBOOL OnRightMouseButtonReleased( const HHitProxy* HHitProxy );

	/** FCallbackEventDevice Interface */
	virtual void Send(ECallbackEventType InType, UObject* InObj);
	virtual void Send(ECallbackEventType InType);

protected:
	void OnCreateAnimCurve( wxCommandEvent& In );
	void OnContextCurveRemove(wxCommandEvent& In);
	
	void RefreshAllCurves();
	void ResetAllAnimation();

	UUIAnimationSeq* AnimSeq;
	UEnum* EnumAnimType;
	WxUIEditorBase* UIEditor;

	TArray<UUIObject*> AnimAppliedObjs;
public:
	DECLARE_EVENT_TABLE()
};

/*-----------------------------------------------------------------------------
WxMBAnimCurveMenu
-----------------------------------------------------------------------------*/

class WxMBAnimCurveMenu : public wxMenu
{
public:
	WxMBAnimCurveMenu(WxCurveEditor* CurveEd);
	~WxMBAnimCurveMenu();
};


#endif