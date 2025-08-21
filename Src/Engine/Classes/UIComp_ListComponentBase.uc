/**
 * Base class for all UIList-related component classes.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class UIComp_ListComponentBase extends UIComponent
//	within UIList
	native(UIPrivate)
	HideCategories(Object)
	abstract;

// within UIList가 컴파일되지 않아 임시로 사용
function final native UIList GetOuterUUIList() const;

defaultproperties
{

}
