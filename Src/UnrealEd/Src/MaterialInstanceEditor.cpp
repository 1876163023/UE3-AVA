/*=============================================================================
MaterialInstanceEditor.cpp: Material instance editor class.
Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
#include "EngineMaterialClasses.h"
#include "Properties.h"
#include "MaterialEditorBase.h"
#include "MaterialEditorToolBar.h"
#include "MaterialInstanceEditor.h"
#include "MaterialEditorPreviewScene.h"
#include "PropertyWindowManager.h"	// required for access to GPropertyWindowManager

/**
 * Versioning Info for the Docking Parent layout file.
 */
namespace
{
	static const TCHAR* DockingParent_Name = TEXT("MaterialInstanceEditor");
	static const INT DockingParent_Version = 0;		//Needs to be incremented every time a new dock window is added or removed from this docking parent.
}


//////////////////////////////////////////////////////////////////////////
// UMaterialEditorInstance
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS(UMaterialEditorInstance);

void UMaterialEditorInstance::PostEditChange(UProperty* Property)
{
	if(Property->GetName()==TEXT("Parent"))
	{
		SourceInstance->SetParent(Parent);
		RegenerateArrays();

		// Reattach components.
		FGlobalComponentReattachContext RecreateComponents;
	}

	CopyToSourceInstance();

	// Tell our source instance to update itself so the preview updates.
	SourceInstance->PostEditChange(Property);
}


/** Regenerates the parameter arrays. */
void UMaterialEditorInstance::RegenerateArrays()
{
	// Clear out existing parameters.
	VectorParameterValues.Empty();
	ScalarParameterValues.Empty();
	TextureParameterValues.Empty();

	if(Parent)
	{
		UMaterial* ParentMaterial = Parent->GetMaterial();

	

		// Loop through all types of parameters for this material and add them to the parameter arrays.
		TArray<FName> ParameterNames;

		// Vector Parameters.
		ParentMaterial->GetAllVectorParameterNames(ParameterNames);
		VectorParameterValues.AddZeroed(ParameterNames.Num());

		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorVectorParameterValue &ParameterValue = VectorParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			FLinearColor Value;
			SourceInstance->GetVectorParameterValue(ParameterName, Value);

			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ParameterValue = Value;

			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->VectorParameterValues.Num(); ParameterIdx++)
			{
				FVectorParameterValue &SourceParam = SourceInstance->VectorParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;
				}
			}
		}

		// Scalar Parameters.
		ParentMaterial->GetAllScalarParameterNames(ParameterNames);
		ScalarParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorScalarParameterValue &ParameterValue = ScalarParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			FLOAT Value;
			SourceInstance->GetScalarParameterValue(ParameterName, Value);

			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ParameterValue = Value;


			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->ScalarParameterValues.Num(); ParameterIdx++)
			{
				FScalarParameterValue &SourceParam = SourceInstance->ScalarParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;
				}
			}
		}

		// Texture Parameters.
		ParentMaterial->GetAllTextureParameterNames(ParameterNames);
		TextureParameterValues.AddZeroed(ParameterNames.Num());
		for(INT ParameterIdx=0; ParameterIdx<ParameterNames.Num(); ParameterIdx++)
		{
			FEditorTextureParameterValue &ParameterValue = TextureParameterValues(ParameterIdx);
			FName ParameterName = ParameterNames(ParameterIdx);
			UTexture* Value;
			SourceInstance->GetTextureParameterValue(ParameterName, Value);

			ParameterValue.bOverride = FALSE;
			ParameterValue.ParameterName = ParameterName;
			ParameterValue.ParameterValue = Value;


			// @todo: This is kind of slow, maybe store these in a map for lookup?
			// See if this keyname exists in the source instance.
			for(INT ParameterIdx=0; ParameterIdx<SourceInstance->NewTextureParameterValues.Num(); ParameterIdx++)
			{
				FTextureParameterValue &SourceParam = SourceInstance->NewTextureParameterValues(ParameterIdx);
				if(ParameterName==SourceParam.ParameterName)
				{
					ParameterValue.bOverride = TRUE;
					ParameterValue.ParameterValue = SourceParam.ParameterValue;
				}
			}
		}
	}
}

/** Copies the parameter array values back to the source instance. */
void UMaterialEditorInstance::CopyToSourceInstance()
{
	SourceInstance->VectorParameterValues.Empty();
	SourceInstance->ScalarParameterValues.Empty();
	SourceInstance->NewTextureParameterValues.Empty();

	// Scalar Parameters
	for(INT ParameterIdx=0; ParameterIdx<ScalarParameterValues.Num(); ParameterIdx++)
	{
		FEditorScalarParameterValue &ParameterValue = ScalarParameterValues(ParameterIdx);

		if(ParameterValue.bOverride)
		{
			SourceInstance->SetScalarParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);
		}
	}

	// Texture Parameters
	for(INT ParameterIdx=0; ParameterIdx<TextureParameterValues.Num(); ParameterIdx++)
	{
		FEditorTextureParameterValue &ParameterValue = TextureParameterValues(ParameterIdx);

		if(ParameterValue.bOverride)
		{
			SourceInstance->SetTextureParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);
		}
	}

	// Vector Parameters
	for(INT ParameterIdx=0; ParameterIdx<VectorParameterValues.Num(); ParameterIdx++)
	{
		FEditorVectorParameterValue &ParameterValue = VectorParameterValues(ParameterIdx);

		if(ParameterValue.bOverride)
		{
			SourceInstance->SetVectorParameterValue(ParameterValue.ParameterName, ParameterValue.ParameterValue);
		}
	}

	// Copy phys material back to source instance
	SourceInstance->PhysMaterial = PhysMaterial;	
	SourceInstance->ReflectivityScale = ReflectivityScale;
	SourceInstance->ReflectivityColor = ReflectivityColor;
	SourceInstance->bOverrideReflectivityColor = bOverrideReflectivityColor;
	SourceInstance->BrightnessScale = BrightnessScale;
	SourceInstance->BrightnessColor = BrightnessColor;
	SourceInstance->bOverrideBrightnessColor = bOverrideBrightnessColor;
}

/** 
 * Sets the source instance for this object and regenerates arrays. 
 *
 * @param MaterialInstance		Instance to use as the source for this material editor instance.
 */
void UMaterialEditorInstance::SetSourceInstance(UMaterialInstanceConstant* MaterialInstance)
{
	check(MaterialInstance);
	SourceInstance = MaterialInstance;
	Parent = SourceInstance->Parent;
	PhysMaterial = SourceInstance->PhysMaterial;
	ReflectivityScale = SourceInstance->ReflectivityScale;
	ReflectivityColor = SourceInstance->ReflectivityColor;
	bOverrideReflectivityColor = SourceInstance->bOverrideReflectivityColor;
	BrightnessScale = SourceInstance->BrightnessScale;
	BrightnessColor = SourceInstance->BrightnessColor;
	bOverrideBrightnessColor = SourceInstance->bOverrideBrightnessColor;
	RegenerateArrays();
}


//////////////////////////////////////////////////////////////////////////
// WxCustomPropertyItem_MaterialInstanceParameter
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC_CLASS(WxCustomPropertyItem_MaterialInstanceParameter, WxCustomPropertyItem_ConditonalItem);

WxCustomPropertyItem_MaterialInstanceParameter::WxCustomPropertyItem_MaterialInstanceParameter() : 
WxCustomPropertyItem_ConditonalItem()
{
	bAllowEditing = FALSE;
}

/**
 * Toggles the value of the property being used as the condition for editing this property.
 *
 * @return	the new value of the condition (i.e. TRUE if the condition is now TRUE)
 */
UBOOL WxCustomPropertyItem_MaterialInstanceParameter::ToggleConditionValue()
{	
	UMaterialEditorInstance* Instance = GetInstanceObject();

	if(Instance)
	{
		FName PropertyName = PropertyStructName;
		FName ScalarArrayName(TEXT("ScalarParameterValues"));
		FName TextureArrayName(TEXT("TextureParameterValues"));
		FName VectorArrayName(TEXT("VectorParameterValues"));

		if(PropertyName==ScalarArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->ScalarParameterValues.Num();ParamIdx++)
			{
				FEditorScalarParameterValue& Param = Instance->ScalarParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==TextureArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->TextureParameterValues.Num();ParamIdx++)
			{
				FEditorTextureParameterValue& Param = Instance->TextureParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==VectorArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->VectorParameterValues.Num();ParamIdx++)
			{
				FEditorVectorParameterValue& Param = Instance->VectorParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					Param.bOverride = !Param.bOverride;
					break;
				}
			}
		}

		// Notify the instance that we modified an override so it needs to update itself.
		Instance->PostEditChange(Property);
	}

	// Always allow editing even if we aren't overriding values.
	return TRUE;
}


/**
 * Returns TRUE if the value of the conditional property matches the value required.  Indicates whether editing or otherwise interacting with this item's
 * associated property should be allowed.
 */
UBOOL WxCustomPropertyItem_MaterialInstanceParameter::IsOverridden()
{
	UMaterialEditorInstance* Instance = GetInstanceObject();

	if(Instance)
	{
		FName PropertyName = PropertyStructName;
		FName ScalarArrayName(TEXT("ScalarParameterValues"));
		FName TextureArrayName(TEXT("TextureParameterValues"));
		FName VectorArrayName(TEXT("VectorParameterValues"));

		if(PropertyName==ScalarArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->ScalarParameterValues.Num();ParamIdx++)
			{
				FEditorScalarParameterValue& Param = Instance->ScalarParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==TextureArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->TextureParameterValues.Num();ParamIdx++)
			{
				FEditorTextureParameterValue& Param = Instance->TextureParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
		else if(PropertyName==VectorArrayName)
		{
			for(INT ParamIdx=0; ParamIdx<Instance->VectorParameterValues.Num();ParamIdx++)
			{
				FEditorVectorParameterValue& Param = Instance->VectorParameterValues(ParamIdx);

				if(Param.ParameterName == DisplayName)
				{
					bAllowEditing = Param.bOverride;
					break;
				}
			}
		}
	}

	return bAllowEditing;
}


/**
 * Returns TRUE if the value of the conditional property matches the value required.  Indicates whether editing or otherwise interacting with this item's
 * associated property should be allowed.
 */
UBOOL WxCustomPropertyItem_MaterialInstanceParameter::IsConditionMet()
{
	return TRUE;
}

/** @return Returns the instance object this property is associated with. */
UMaterialEditorInstance* WxCustomPropertyItem_MaterialInstanceParameter::GetInstanceObject()
{
	WxPropertyWindow_Objects* ItemParent = FindObjectItemParent();
	UMaterialEditorInstance* MaterialInstance = NULL;

	if(ItemParent)
	{
		for(WxPropertyWindow_Objects::TObjectIterator It(ItemParent->ObjectIterator()); It; ++It)
		{
			MaterialInstance = Cast<UMaterialEditorInstance>(*It);
			break;
		}
	}

	return MaterialInstance;
}

/**
 * Renders the left side of the property window item.
 *
 * This version is responsible for rendering the checkbox used for toggling whether this property item window should be enabled.
 *
 * @param	RenderDeviceContext		the device context to use for rendering the item name
 * @param	ClientRect				the bounding region of the property window item
 */
void WxCustomPropertyItem_MaterialInstanceParameter::RenderItemName( wxBufferedPaintDC& RenderDeviceContext, const wxRect& ClientRect )
{
	const UBOOL bItemEnabled = IsOverridden();

	// determine which checkbox image to render
	const WxMaskedBitmap& bmp = bItemEnabled
		? GPropertyWindowManager->CheckBoxOnB
		: GPropertyWindowManager->CheckBoxOffB;

	// render the checkbox bitmap
	RenderDeviceContext.DrawBitmap( bmp, ClientRect.x + IndentX - PROP_Indent, ClientRect.y + ((PROP_DefaultItemHeight - bmp.GetHeight()) / 2), 1 );

	INT W, H;
	RenderDeviceContext.GetTextExtent( *DisplayName.ToString(), &W, &H );

	const INT YOffset = (PROP_DefaultItemHeight - H) / 2;
	RenderDeviceContext.DrawText( *DisplayName.ToString(), ClientRect.x+IndentX+( bCanBeExpanded ? 16 : 0 ),ClientRect.y+YOffset );

	RenderDeviceContext.DestroyClippingRegion();
}


/**
 * Called when an property window item receives a left-mouse-button press which wasn't handled by the input proxy.  Typical response is to gain focus
 * and (if the property window item is expandable) to toggle expansion state.
 *
 * @param	Event	the mouse click input that generated the event
 *
 * @return	TRUE if this property window item should gain focus as a result of this mouse input event.
 */
UBOOL WxCustomPropertyItem_MaterialInstanceParameter::ClickedPropertyItem( wxMouseEvent& Event )
{
	UBOOL bShouldGainFocus = TRUE;

	// if this property is edit-const, it can't be changed
	// or if we couldn't find a valid condition property, also use the base version
	if ( Property == NULL || (Property->PropertyFlags & CPF_EditConst) != 0 )
	{
		bShouldGainFocus = WxPropertyWindow_Item::ClickedPropertyItem(Event);
	}

	// if they clicked on the checkbox, toggle the edit condition
	else if ( ClickedCheckbox(Event.GetX(), Event.GetY()) )
	{
		
		NotifyPreChange(Property);
		bShouldGainFocus = !bCanBeExpanded;
		if ( ToggleConditionValue() == FALSE )
		{
			bShouldGainFocus = FALSE;

			// if we just disabled the condition which allows us to edit this control
			// collapse the item if this is an expandable item
			if ( bCanBeExpanded )
			{
				Collapse();
			}
		}

		if ( !bCanBeExpanded && ParentItem != NULL )
		{
			ParentItem->Refresh();
		}
		else
		{
			Refresh();
		}


		// Note the current property window so that CALLBACK_ObjectPropertyChanged
		// doesn't destroy the window out from under us.
		WxPropertyWindow* PreviousPropertyWindow = NULL;
		if ( GApp )
		{
			PreviousPropertyWindow = GApp->CurrentPropertyWindow;
			GApp->CurrentPropertyWindow = GetPropertyWindow();
		}
		NotifyPostChange(Property);

		// Unset, effectively making this property window updatable by CALLBACK_ObjectPropertyChanged.
		if ( GApp )
		{
			GApp->CurrentPropertyWindow = PreviousPropertyWindow;
		}
	}
	// if the condition for editing this control has been met (i.e. the checkbox is checked), pass the event back to the base version, which will do the right thing
	// based on where the user clicked
	else if ( IsConditionMet() )
	{
		bShouldGainFocus = WxPropertyWindow_Item::ClickedPropertyItem(Event);
	}
	else
	{
		// the condition is false, so this control isn't allowed to do anything - swallow the event.
		bShouldGainFocus = FALSE;
	}

	return bShouldGainFocus;
}


//////////////////////////////////////////////////////////////////////////
// WxPropertyWindow_MaterialInstanceParameters
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC_CLASS(WxPropertyWindow_MaterialInstanceParameters, WxPropertyWindow_Item);

// Called by Expand(), creates any child items for any properties within this item.
void WxPropertyWindow_MaterialInstanceParameters::CreateChildItems()
{
	FName PropertyName = Property->GetFName();
	UStructProperty* StructProperty = Cast<UStructProperty>(Property,CLASS_IsAUStructProperty);
	UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property);
	UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property,CLASS_IsAUObjectProperty);

	// Copy IsSorted() flag from parent.  Default sorted to TRUE if no parent exists.
	SetSorted( ParentItem ? ParentItem->IsSorted() : 1 );

	if( Property->ArrayDim > 1 && ArrayIndex == -1 )
	{
		// Expand array.
		SetSorted( 0 );
		for( INT i = 0 ; i < Property->ArrayDim ; i++ )
		{
			WxPropertyWindow_Item* pwi = CreatePropertyItem(Property,i,this);
			pwi->Create( this, this, TopPropertyWindow, Property, i*Property->ElementSize, i, bSupportsCustomControls );
			ChildItems.AddItem(pwi);
		}
	}
	else if( ArrayProperty )
	{
		// Expand array.
		SetSorted( 0 );

		FArray* Array = NULL;
		TArray<BYTE*> Addresses;
		if ( GetReadAddress( this, bSingleSelectOnly, Addresses ) )
		{
			Array = (FArray*)Addresses(0);
		}

		WxPropertyWindow_Objects* ItemParent = FindObjectItemParent();
		UMaterialEditorInstance* MaterialInstance = NULL;
		UMaterial* Material = NULL;


		if(ItemParent)
		{
			for(WxPropertyWindow_Objects::TObjectIterator It(ItemParent->ObjectIterator()); It; ++It)
			{
				MaterialInstance = Cast<UMaterialEditorInstance>(*It);
				Material = MaterialInstance->SourceInstance->GetMaterial();
				break;
			}
		}

		if( Array && Material )
		{
			FName ParameterValueName(TEXT("ParameterValue"));
			FName ScalarArrayName(TEXT("ScalarParameterValues"));
			FName TextureArrayName(TEXT("TextureParameterValues"));
			FName VectorArrayName(TEXT("VectorParameterValues"));

			// Make sure that the inner of this array is a material instance parameter struct.
			UStructProperty* StructProperty = Cast<UStructProperty>(ArrayProperty->Inner);
		
			if(StructProperty)
			{	
				// Iterate over all possible fields of this struct until we find the value field, we want to combine
				// the name and value of the parameter into one property window item.  We do this by adding a item for the value
				// and overriding the name of the item using the name from the parameter.
				for( TFieldIterator<UProperty,CLASS_IsAUProperty> It(StructProperty->Struct); It; ++It )
				{
					UProperty* StructMember = *It;
					if( GetPropertyWindow()->ShouldShowNonEditable() || (StructMember->PropertyFlags&CPF_Edit) )
					{
						// Loop through all elements of this array and add properties for each one.
						for( INT ArrayIdx = 0 ; ArrayIdx < Array->Num() ; ArrayIdx++ )
						{	
							WxCustomPropertyItem_MaterialInstanceParameter* PropertyWindowItem = wxDynamicCast(CreatePropertyItem(StructMember,INDEX_NONE,this), 
								WxCustomPropertyItem_MaterialInstanceParameter);
							if(StructMember->GetFName() == ParameterValueName)
							{
								// Find a name for the parameter property we are adding.
								FName OverrideName = NAME_None;
								BYTE* ElementData = ((BYTE*)Array->GetData())+ArrayIdx*ArrayProperty->Inner->ElementSize;

								if(PropertyName==ScalarArrayName)
								{
									FEditorScalarParameterValue* Param = (FEditorScalarParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
								}
								else if(PropertyName==TextureArrayName)
								{
									FEditorTextureParameterValue* Param = (FEditorTextureParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
								}
								else if(PropertyName==VectorArrayName)
								{
									FEditorVectorParameterValue* Param = (FEditorVectorParameterValue*)(ElementData);
									OverrideName = (Param->ParameterName);
								}

								// Add the property.
								PropertyWindowItem->Create( this, this, TopPropertyWindow, StructMember, ArrayIdx*ArrayProperty->Inner->ElementSize+StructMember->Offset, 
									INDEX_NONE, bSupportsCustomControls );
								PropertyWindowItem->PropertyStructName = PropertyName;
								PropertyWindowItem->DisplayName = OverrideName;

								ChildItems.AddItem(PropertyWindowItem);
							}
						}
					}
				}
			}
		}
	}

	SortChildren();
}


//////////////////////////////////////////////////////////////////////////
//
//	WxMaterialInstanceEditor
//
//////////////////////////////////////////////////////////////////////////

/**
 * wxWidgets Event Table
 */
BEGIN_EVENT_TABLE(WxMaterialInstanceEditor, WxMaterialEditorBase)

END_EVENT_TABLE()


WxMaterialInstanceEditor::WxMaterialInstanceEditor( wxWindow* Parent, wxWindowID id, UMaterialInstance* InMaterialInstance ) :	
        WxMaterialEditorBase( Parent, id, InMaterialInstance ),   
		FDockingParent(this)
{
	// Set the static mesh editor window title to include the static mesh being edited.
	SetTitle( *FString::Printf( *LocalizeUnrealEd("MaterialInstanceEditorCaption_F"), *MaterialInstance->GetPathName() ) );

	// Construct a temp holder for our instance parameters.
	MaterialEditorInstance = ConstructObject<UMaterialEditorInstance>(UMaterialEditorInstance::StaticClass());
	MaterialEditorInstance->SetSourceInstance(Cast<UMaterialInstanceConstant>(InMaterialInstance));
	
	// Create toolbar
	ToolBar = new WxMaterialInstanceEditorToolBar( this, -1 );
	SetToolBar( ToolBar );

	// Create property window
	PropertyWindow = new WxPropertyWindow;
	PropertyWindow->Create( this, this );
	PropertyWindow->SetCustomControlSupport( TRUE );
	PropertyWindow->SetObject( MaterialEditorInstance, 1,1, 1 );	//<@ ava specific ; category 보이도록 call parameter 변경
	FWindowUtil::LoadPosSize( TEXT("MaterialInstanceEditor"), this, 64,64,800,450 );

	// Add docking windows.
	{
		AddDockingWindow(PropertyWindow, FDockingParent::DH_Right, *FString::Printf(*LocalizeUnrealEd("PropertiesCaption_F"), *MaterialInstance->GetPathName()), *LocalizeUnrealEd("Properties"));
		SetDockHostSize(FDockingParent::DH_Right, 300);

		wxPane* PreviewPane = new wxPane( this );
		{
			PreviewPane->ShowHeader(false);
			PreviewPane->ShowCloseButton( false );
			PreviewPane->SetClient((wxWindow*)PreviewWin);
		}
		LayoutManager->SetLayout( wxDWF_SPLITTER_BORDERS, PreviewPane );

		// Try to load a existing layout for the docking windows.
		LoadDockingLayout();
	}

	// Add docking menu
	wxMenuBar* MenuBar = new wxMenuBar();
	AppendDockingMenu(MenuBar);
	SetMenuBar(MenuBar);

	// Set the preview mesh for the material.  This call must occur after the toolbar is initialized.
	if ( !SetPreviewMesh( *InMaterialInstance->PreviewMesh ) )
	{
		// The material preview mesh couldn't be found or isn't loaded.  Default to the sphere.
		SetPreviewMesh( GUnrealEd->GetThumbnailManager()->TexPropSphere, NULL );
	}

	// Load editor settings.
	LoadSettings();
}

WxMaterialInstanceEditor::~WxMaterialInstanceEditor()
{
	SaveSettings();

	PropertyWindow->SetObject( NULL, 0,0,0 );
	delete PropertyWindow;
}

void WxMaterialInstanceEditor::Serialize(FArchive& Ar)
{
	// Just call super for now.
	WxMaterialEditorBase::Serialize(Ar);
	Ar<<MaterialEditorInstance;
}

/** Post edit change notify for properties. */
void WxMaterialInstanceEditor::NotifyPostChange(void* Src, UProperty* PropertyThatChanged)
{
	// Update the preview window when the user changes a property.
	RefreshPreviewViewport();
}

/** Saves editor settings. */
void WxMaterialInstanceEditor::SaveSettings()
{
	FWindowUtil::SavePosSize( TEXT("MaterialInstanceEditor"), this );

	SaveDockingLayout();

	GConfig->SetBool(TEXT("MaterialInstanceEditor"), TEXT("bShowGrid"), bShowGrid, GEditorUserSettingsIni);
	GConfig->SetBool(TEXT("MaterialInstanceEditor"), TEXT("bDrawGrid"), PreviewVC->IsRealtime(), GEditorUserSettingsIni);
}

/** Loads editor settings. */
void WxMaterialInstanceEditor::LoadSettings()
{
	UBOOL bRealtime=FALSE;

	GConfig->GetBool(TEXT("MaterialInstanceEditor"), TEXT("bShowGrid"), bShowGrid, GEditorUserSettingsIni);
	GConfig->GetBool(TEXT("MaterialInstanceEditor"), TEXT("bDrawGrid"), bRealtime, GEditorUserSettingsIni);

	if(PreviewVC)
	{
		PreviewVC->SetShowGrid(bShowGrid);
		PreviewVC->SetRealtime(bRealtime);
	}
}

/**
 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
 *  @return A string representing a name to use for this docking parent.
 */
const TCHAR* WxMaterialInstanceEditor::GetDockingParentName() const
{
	return DockingParent_Name;
}

/**
 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
 */
const INT WxMaterialInstanceEditor::GetDockingParentVersion() const
{
	return DockingParent_Version;
}


