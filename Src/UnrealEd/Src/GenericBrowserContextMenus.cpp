/*=============================================================================
	Copyright 2005-2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "UnrealEd.h"
/*
#include "..\..\Launch\Resources\resource.h"
#include "EngineSequenceClasses.h"
#include "EnginePhysicsClasses.h"
#include "EnginePrefabClasses.h"
*/
#include "GenericBrowserContextMenus.h"

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext.
-----------------------------------------------------------------------------*/

void WxMBGenericBrowserContext::AppendObjectMenu()
{
	AppendSeparator();
	Append(IDMN_ObjectContext_CopyReference,*LocalizeUnrealEd("CopyReference"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_Duplicate,*LocalizeUnrealEd("Duplicate"),TEXT(""));
	Append(IDMN_ObjectContext_Rename,*LocalizeUnrealEd("Rename"),TEXT(""));
	Append(IDMN_ObjectContext_Delete,*LocalizeUnrealEd("Delete"),TEXT(""));
	Append(IDMN_ObjectContext_Export,*LocalizeUnrealEd("ExportFileE"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_ShowRefs,*LocalizeUnrealEd("ShowReferencers"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_ShowRefObjs, *LocalizeUnrealEd("ShowReferences"),TEXT(""));
	Append(IDMN_ObjectContext_DuplicateWithRefs,*LocalizeUnrealEd("DuplicateWithReferences"),TEXT(""));
	Append(IDMN_ObjectContext_RenameWithRefs,*LocalizeUnrealEd("MoveWithReferences"),TEXT(""));
	Append(IDMN_ObjectContext_DeleteWithRefs,*LocalizeUnrealEd("DeleteWithReferences"),TEXT(""));
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Object.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Object::WxMBGenericBrowserContext_Object()
{
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Archetype.
-----------------------------------------------------------------------------*/
WxMBGenericBrowserContext_Archetype::WxMBGenericBrowserContext_Archetype()
{
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Material.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Material::WxMBGenericBrowserContext_Material()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("MaterialEditor"),TEXT(""));
	//<@ ava specific ; 2007. 8. 24 changmin
	Append(IDMN_AVA_SelectSurfaceMatchingSelectedMaterial, *LocalizeUnrealEd("SelectSurfaceMatchingSelectedMaterial"),TEXT(""));
	//>@ ava
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_MaterialInstance.
-----------------------------------------------------------------------------*/
WxMBGenericBrowserContext_MaterialInstance::WxMBGenericBrowserContext_MaterialInstance()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("MaterialInstanceEditor"),TEXT(""));
	//<@ ava specific ; 2007. 8. 24 changmin
	Append(IDMN_AVA_SelectSurfaceMatchingSelectedMaterial, *LocalizeUnrealEd("SelectSurfaceMatchingSelectedMaterial"),TEXT(""));
	//>@ ava
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Texture.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Texture::WxMBGenericBrowserContext_Texture()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("TextureViewer"),TEXT(""));
	Append(IDMN_ObjectContext_Reimport,*LocalizeUnrealEd(TEXT("ReimportTexture")),TEXT(""));
	Append(IDMN_ObjectContext_CreateNewMaterial,*LocalizeUnrealEd(TEXT("CreateNewMaterial")),TEXT(""));
	AppendCheckItem(IDMN_TextureContext_ShowStreamingBounds,*LocalizeUnrealEd(TEXT("ShowStreamingBounds")),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_RenderTexture.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_RenderTexture::WxMBGenericBrowserContext_RenderTexture()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("TextureViewer"),TEXT(""));
	Append(IDMN_ObjectContext_RTCreateStaticTexture,*LocalizeUnrealEd(TEXT("CreateStaticTextureE")),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_StaticMesh.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_StaticMesh::WxMBGenericBrowserContext_StaticMesh()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("StaticMeshEditor"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_Sound.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Sound::WxMBGenericBrowserContext_Sound()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("SoundNodeWavePreview"),TEXT(""));
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_Sound_Play,*LocalizeUnrealEd("Play"),TEXT(""));
	Append(IDMN_ObjectContext_Sound_Stop,*LocalizeUnrealEd("Stop"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_SoundCue.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_SoundCue::WxMBGenericBrowserContext_SoundCue()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("SoundCueEditor"),TEXT(""));
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	Append(IDMN_ObjectContext_EditNodes,*LocalizeUnrealEd("EditNodesE"),TEXT(""));
	AppendSeparator();	
	Append(IDMN_ObjectContext_Sound_Play,*LocalizeUnrealEd("Play"),TEXT(""));
	Append(IDMN_ObjectContext_Sound_Stop,*LocalizeUnrealEd("Stop"),TEXT(""));
	AppendSeparator();

	// Add sound group menu & submenu.
	wxMenu*			SoundGroupMenu	= new wxMenu();
	UAudioDevice*	AudioDevice		= GEditor->Client->GetAudioDevice();
	if( AudioDevice )
	{
		// Retrieve sound groups from audio device.
		TArray<FName> SoundGroupNames = AudioDevice->GetSoundGroupNames();
		for( INT NameIndex=0; NameIndex<SoundGroupNames.Num(); NameIndex++ )
		{
			// Add sound group to submenu.
			FName GroupName = SoundGroupNames(NameIndex);
			SoundGroupMenu->Append(IDMN_ObjectContext_SoundCue_SoundGroups_START+NameIndex,*GroupName.ToString());
		}
		// Add menu.
		Append(IDMN_ObjectContext_SoundCue_SoundGroups,*LocalizeUnrealEd("SoundGroups"),SoundGroupMenu);
	}

	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_PhysicsAsset.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_PhysicsAsset::WxMBGenericBrowserContext_PhysicsAsset()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("PhysicsAssetEditor"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_FaceFXAsset.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_FaceFXAsset::WxMBGenericBrowserContext_FaceFXAsset()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("FaceFXStudio"),TEXT(""));
    AppendSeparator();
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_CreateNewFaceFXAnimSet,*LocalizeUnrealEd("CreateFaceFXAnimSet"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_ImportFaceFXAsset,*LocalizeUnrealEd("ImportFromFXA"),TEXT(""));
	Append(IDMN_ObjectContext_ExportFaceFXAsset,*LocalizeUnrealEd("ExportToFXA"),TEXT(""));
	// Don't want to show the "Export to File..." option as it may be confusing
	// since there is already an "Export to ..." option, so rather than call 
	// AppendObjectMenu() simply do it in-place.
	AppendSeparator();
	Append(IDMN_ObjectContext_CopyReference,*LocalizeUnrealEd("CopyReference"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_Duplicate,*LocalizeUnrealEd("Duplicate"),TEXT(""));
	Append(IDMN_ObjectContext_Rename,*LocalizeUnrealEd("Rename"),TEXT(""));
	Append(IDMN_ObjectContext_Delete,*LocalizeUnrealEd("Delete"),TEXT(""));
	Append(IDMN_ObjectContext_ShowRefs,*LocalizeUnrealEd("ShowReferencers"),TEXT(""));
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_FaceFXAnimSet.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_FaceFXAnimSet::WxMBGenericBrowserContext_FaceFXAnimSet()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("FaceFXStudio"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	// Don't want to show the "Export to File..." option as it may be confusing
	// since there is already an "Export to ..." option, so rather than call 
	// AppendObjectMenu() simply do it in-place.
	AppendSeparator();
	Append(IDMN_ObjectContext_CopyReference,*LocalizeUnrealEd("CopyReference"),TEXT(""));
	AppendSeparator();
	Append(IDMN_ObjectContext_Duplicate,*LocalizeUnrealEd("Duplicate"),TEXT(""));
	Append(IDMN_ObjectContext_Rename,*LocalizeUnrealEd("Rename"),TEXT(""));
	Append(IDMN_ObjectContext_Delete,*LocalizeUnrealEd("Delete"),TEXT(""));
	Append(IDMN_ObjectContext_ShowRefs,*LocalizeUnrealEd("ShowReferencers"),TEXT(""));
}

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_Skeletal.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_Skeletal::WxMBGenericBrowserContext_Skeletal()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("AnimSetViewer"),TEXT("")); 
	Append(IDMN_ObjectContext_CreateNewPhysicsAsset,*LocalizeUnrealEd("CreateNewPhysicsAsset"),TEXT("")); 
#if WITH_FACEFX
	Append(IDMN_ObjectContext_CreateNewFaceFXAsset, *LocalizeUnrealEd("CreateNewFaceFXAsset"), TEXT(""));
#endif // WITH_FACEFX
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
WxMBGenericBrowserContext_LensFlare
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_LensFlare::WxMBGenericBrowserContext_LensFlare()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("EditorLensFlare"),TEXT("")); 
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_ParticleSystem.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_ParticleSystem::WxMBGenericBrowserContext_ParticleSystem()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("EditorCascade"),TEXT("")); 
	Append(IDMN_ObjectContext_Properties,*LocalizeUnrealEd("PropertiesE"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_AnimSet.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_AnimSet::WxMBGenericBrowserContext_AnimSet()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("AnimSetViewer"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	WxMBGenericBrowserContext_MorphTargetSet.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_MorphTargetSet::WxMBGenericBrowserContext_MorphTargetSet()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("AnimSetViewer"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	XMBGenericBrowserContext_AnimTree.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_AnimTree::WxMBGenericBrowserContext_AnimTree()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("AnimTreeEditor"),TEXT(""));
	AppendObjectMenu();
}

/*-----------------------------------------------------------------------------
	XMBGenericBrowserContext_AnimTree.
-----------------------------------------------------------------------------*/

WxMBGenericBrowserContext_PostProcess::WxMBGenericBrowserContext_PostProcess()
{
	Append(IDMN_ObjectContext_Editor,*LocalizeUnrealEd("PostProcessEditor"),TEXT(""));
	AppendObjectMenu();
}
