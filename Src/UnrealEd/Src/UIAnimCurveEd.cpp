/*=============================================================================
	CurveEd.cpp: Specified FInterpCurve editor for UIAnimation

=============================================================================*/

#include "UnrealEd.h"
#include "UIAnimCurveEd.h"

BEGIN_EVENT_TABLE(WxUIAnimCurveEd, WxCurveEditor)
	EVT_MENU_RANGE(IDM_UIANIMCURVE_BASE,IDM_UIANIMCURVE_END,WxUIAnimCurveEd::OnCreateAnimCurve)
	EVT_MENU(IDM_CURVE_REMOVECURVE, WxUIAnimCurveEd::OnContextCurveRemove)
END_EVENT_TABLE()

WxUIAnimCurveEd::WxUIAnimCurveEd( WxUIEditorBase* InUIEditor, wxWindowID InID, class UInterpCurveEdSetup* InEdSetup, UUIAnimationSeq *InAnimSeq ) : WxCurveEditor( InUIEditor, InID, InEdSetup ), AnimSeq(InAnimSeq), UIEditor(InUIEditor)
{
	EnumAnimType = FindObject<UEnum>(ANY_PACKAGE, TEXT("EUIAnimType"), TRUE);
	check(AnimSeq && EnumAnimType && InUIEditor);

	RefreshAllCurves();
	GCallbackEvent->Register( CALLBACK_SelectObject, this );
	GCallbackEvent->Register( CALLBACK_SelChange, this );
}

WxUIAnimCurveEd::~WxUIAnimCurveEd()
{
	GCallbackEvent->Unregister( CALLBACK_SelectObject, this );
	GCallbackEvent->Unregister( CALLBACK_SelChange, this );
	EdSetup->ResetTabs();
}

void WxUIAnimCurveEd::PreEditCurve(TArray<UObject*> CurvesAboutToChange)
{
}

void WxUIAnimCurveEd::PostEditCurve()
{
	AnimSeq->RecalcInTimeRange();
	AnimSeq->Modify();
}

UBOOL WxUIAnimCurveEd::OnRightMouseButtonReleased( const HHitProxy* HHitProxy )
{
	UBOOL bResult = FALSE;

	if( HHitProxy && HHitProxy->IsA(HCurveEdLabelBkgProxy::StaticGetType()) )
	{
		WxMBAnimCurveMenu AnimCurveMenu( this );
		FTrackPopupMenu tpm( this, &AnimCurveMenu );
		tpm.Show();
		bResult = TRUE;
	}

	return bResult;
}

void WxUIAnimCurveEd::Send(ECallbackEventType InType, UObject* InObj)
{
	if( InType == CALLBACK_SelectObject && Cast<UUIObject>(InObj) )
	{
		ResetAllAnimation();
	}
}

void WxUIAnimCurveEd::Send(ECallbackEventType InType)
{
	if( InType == CALLBACK_SelChange )
	{
		ResetAllAnimation();
	}
}


void WxUIAnimCurveEd::OnCreateAnimCurve(wxCommandEvent& In)
{
	check(AnimSeq);

	if( !(IDM_UIANIMCURVE_BASE <= In.GetId() && In.GetId() < IDM_UIANIMCURVE_END) )
		return;

	BYTE AnimType = In.GetId() - IDM_UIANIMCURVE_BASE;

	INT TrackIndex = AnimSeq->GetAnimTrack( AnimType );

	if( TrackIndex != INDEX_NONE )
	{
		RefreshAllCurves();
		AnimSeq->Modify();
	}
}

void WxUIAnimCurveEd::OnContextCurveRemove(wxCommandEvent& In)
{
	FName CurveName = *EdSetup->Tabs(EdSetup->ActiveTab).Curves(RightClickCurveIndex).CurveName;
	INT RemoveAnimType = INDEX_NONE;

	for( INT EnumIndex = 0 ; EnumIndex < EnumAnimType->NumEnums() ; EnumIndex++  )
	{
		if( EnumAnimType->GetEnum(EnumIndex) == CurveName )
		{
			RemoveAnimType = EnumIndex;
			break;
		}
	}

	if( RemoveAnimType != INDEX_NONE )
	{
		for( INT TrackIndex = AnimSeq->Tracks.Num() - 1 ; TrackIndex >= 0 ; TrackIndex-- )
		{
			if( AnimSeq->Tracks(TrackIndex).TrackType == RemoveAnimType )
			{
				AnimSeq->Tracks.Remove(TrackIndex);
			}
		}
		ResetAllAnimation();
	}

	AnimSeq->Modify();
	In.Skip();
}


void WxUIAnimCurveEd::RefreshAllCurves()
{
	// Clear All Curves
	for( INT TabIndex = 0 ; TabIndex < EdSetup->Tabs.Num() ; TabIndex++ )
	{
		for( INT CurveIndex = EdSetup->Tabs(TabIndex).Curves.Num() - 1 ; CurveIndex	 >= 0 ; CurveIndex-- )
		{
			EdSetup->Tabs(TabIndex).Curves.Remove(CurveIndex);
		}
	}

	// Get the newest curve setup
	for( INT TrackIndex = 0 ; TrackIndex < AnimSeq->Tracks.Num() ; TrackIndex++ )
	{
		FUIAnimTrack& AnimTrack = AnimSeq->Tracks(TrackIndex);
		UClass* ProperCurveClass = AnimSeq->GetProperDistributionClass( AnimTrack.TrackType );
		if( ProperCurveClass->IsChildOf(UDistributionFloat::StaticClass()) )
		{
			EdSetup->AddCurveToCurrentTab( AnimTrack.DistFloat.Distribution, EnumAnimType->GetEnum(AnimTrack.TrackType).GetName(), FLinearColor::FGetHSV( AnimTrack.TrackType*255/EAT_MAX,255,255).ToRGBE(), TRUE );
		}
		else if( ProperCurveClass->IsChildOf(UDistributionVector::StaticClass()) )
		{
			EdSetup->AddCurveToCurrentTab( AnimTrack.DistVector.Distribution, EnumAnimType->GetEnum(AnimTrack.TrackType).GetName(), FLinearColor::FGetHSV( AnimTrack.TrackType*255/EAT_MAX,255,255).ToRGBE(), TRUE );
		}
		else
		{
			checkf(FALSE, TEXT("no match found between Distribution and AnimType"));
		}
	}
}

void WxUIAnimCurveEd::ResetAllAnimation()
{
	TArray<UUIObject*> SelectedWidgets;
	UIEditor->GetSelectedWidgets( SelectedWidgets );
	if( SelectedWidgets.Num() == AnimAppliedObjs.Num() )
		return;

	for( INT ObjIndex = 0 ; ObjIndex < AnimAppliedObjs.Num() ; ObjIndex++)
	{
		AnimAppliedObjs(ObjIndex)->eventStopUIAnimation(NAME_None, AnimSeq);
	}

	AnimAppliedObjs = SelectedWidgets;

	for( INT ObjIndex = 0 ; ObjIndex < AnimAppliedObjs.Num() ; ObjIndex++ )
	{
		AnimAppliedObjs(ObjIndex)->eventPlayUIAnimation(NAME_None,AnimSeq,1.f,TRUE);
	}
}

/*-----------------------------------------------------------------------------
WxMBAnimCurveMenu.
-----------------------------------------------------------------------------*/

WxMBAnimCurveMenu::WxMBAnimCurveMenu(WxCurveEditor* CurveEd)
{
	UEnum* EnumAnimType = FindObject<UEnum>(ANY_PACKAGE, TEXT("EUIAnimType"), TRUE);
	check(EnumAnimType);

	for( INT i = 0 ; i < EAT_MAX ; i++ )
		Append( IDM_UIANIMCURVE_BASE + i, EnumAnimType->GetEnum(i).GetName(), TEXT("") );

}

WxMBAnimCurveMenu::~WxMBAnimCurveMenu()
{

}
