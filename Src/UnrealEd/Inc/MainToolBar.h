/*=============================================================================
	Copyright ?2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __MAINTOOLBAR_H__
#define __MAINTOOLBAR_H__

class wxComboBox;
class wxWindow;
class WxAlphaBitmap;

/**
 * The toolbar that sits at the top of the main editor frame.
 */
class WxMainToolBar : public wxToolBar
{
public:
	WxMainToolBar(wxWindow* InParent, wxWindowID InID);

	enum EPlatformButtons { B_PC=0, B_PS3, B_XBox360, B_MAX };

private:
	WxMaskedBitmap NewB, OpenB, SaveB, SaveAllB, UndoB, RedoB, CutB, CopyB, PasteB, SearchB, FullScreenB, GenericB, KismetB, TranslateB,
		ShowWidgetB, RotateB, ScaleB, ScaleNonUniformB, BrushPolysB, PrefabLockB, CamSlowB, CamNormalB, CamFastB, ViewPushStartB, ViewPushStopB, ViewPushSyncB,
		DistributionToggleB;
	WxAlphaBitmap *BuildGeomB, *BuildLightingB, *BuildPathsB, *BuildCoverNodesB, *BuildAllB, *PlayB[B_MAX];
	//<@ ava specific ; 2006. 1. 12 changmin
	//WxAlphaBitmap *ComputePVSB;
	WxAlphaBitmap *FullBuildGeomB;
	WxAlphaBitmap *BuildSolidGeomB;
	WxAlphaBitmap *BuildSolidGeomAndComputePvsB;	// 2008. 2. 22 changmin
	WxAlphaBitmap *CheckBrushIntersectionB;			// 2008. 3. 6 changmin
	WxAlphaBitmap *CheckSolidBrushIntersectionB;			// 2008. 3. 7 changmin
	//>@ ava
	WxMenuButton MRUButton, PasteSpecialButton;
	wxMenu PasteSpecialMenu;

public:
	wxComboBox* CoordSystemCombo;
};

#endif // __MAINTOOLBAR_H__
