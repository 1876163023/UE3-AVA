/*=============================================================================
	MaterialInstanceEditor.h: Material instance editor class.
	Copyright © 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __MATERIALINSTANCEEDITOR_H__
#define __MATERIALINSTANCEEDITOR_H__

#include "Properties.h"
#include "MaterialEditorBase.h"
#include "UnUIEditorProperties.h"

// Forward declarations.
class WxPropertyWindow;
class WxMaterialInstanceEditorToolBar;

/**
 * Custom property window class for displaying material instance parameters.
 */
class WxCustomPropertyItem_MaterialInstanceParameter : public WxCustomPropertyItem_ConditonalItem
{
public:
	DECLARE_DYNAMIC_CLASS(WxCustomPropertyItem_MaterialInstanceParameter);

	/** Whether or not to allow editing of properties. */
	UBOOL bAllowEditing;

	/** Name of the struct that holds this property. */
	FName PropertyStructName;

	/** Name to display on the left side of the property instead of the normal property name. */
	FName DisplayName;

	// Constructor
	WxCustomPropertyItem_MaterialInstanceParameter();

	/**
	 * Toggles the value of the property being used as the condition for editing this property.
	 *
	 * @return	the new value of the condition (i.e. TRUE if the condition is now TRUE)
	 */
	virtual UBOOL ToggleConditionValue();

	/**
	 * Returns TRUE if the value of the conditional property matches the value required.  Indicates whether editing or otherwise interacting with this item's
	 * associated property should be allowed.
	 */
	virtual UBOOL IsConditionMet();

	/**
	 * @return TRUE if the property is overridden, FALSE otherwise.
	 */
	virtual UBOOL IsOverridden();

	/** @return Returns the instance object this property is associated with. */
	UMaterialEditorInstance* GetInstanceObject();

	/**
	 * Called when an property window item receives a left-mouse-button press which wasn't handled by the input proxy.  Typical response is to gain focus
	 * and (if the property window item is expandable) to toggle expansion state.
	 *
	 * @param	Event	the mouse click input that generated the event
	 *
	 * @return	TRUE if this property window item should gain focus as a result of this mouse input event.
	 */
	UBOOL ClickedPropertyItem( wxMouseEvent& Event );

	/**
	 * Renders the left side of the property window item.
	 *
	 * This version is responsible for rendering the checkbox used for toggling whether this property item window should be enabled.
	 *
	 * @param	RenderDeviceContext		the device context to use for rendering the item name
	 * @param	ClientRect				the bounding region of the property window item
	 */
	virtual void RenderItemName( wxBufferedPaintDC& RenderDeviceContext, const wxRect& ClientRect );
};

/**
 * Custom property window item class for displaying material instance parameter arrays unwrapped.
 */
class WxPropertyWindow_MaterialInstanceParameters : public WxPropertyWindow_Item
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_MaterialInstanceParameters);

	virtual void CreateChildItems();
private:
};


/**
 * Main material instance editor window class.
 */
class WxMaterialInstanceEditor : public WxMaterialEditorBase, public FSerializableObject, public FDockingParent, FNotifyHook
{
public:
	WxMaterialInstanceEditor(wxWindow* parent, wxWindowID id, UMaterialInstance* InMaterialInstance);
	virtual ~WxMaterialInstanceEditor();

	WxPropertyWindow* PropertyWindow;					/** Property window to display instance parameters. */
	UMaterialEditorInstance* MaterialEditorInstance;	/** Object that stores all of the possible parameters we can edit. */

	/** The material editor's toolbar. */
	WxMaterialInstanceEditorToolBar*		ToolBar;

	virtual void Serialize(FArchive& Ar);

	/** Post edit change notify for properties. */
	void NotifyPostChange(void* Src, UProperty* PropertyThatChanged);

protected:

	/** Saves editor settings. */
	void SaveSettings();

	/** Loads editor settings. */
	void LoadSettings();

	/**
	 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
	 *  @return A string representing a name to use for this docking parent.
	 */
	virtual const TCHAR* GetDockingParentName() const;

	/**
	 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
	 */
	virtual const INT GetDockingParentVersion() const;

private:
	DECLARE_EVENT_TABLE()
};

#endif // __MATERIALINSTANCEEDITOR_H__

