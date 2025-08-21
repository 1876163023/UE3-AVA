//=============================================================================
// Copyright 2004-2005 Epic Games - All Rights Reserved.
// Confidential.
//
// This holds all of the base UI Objects
//=============================================================================

#include "avaGame.h"

IMPLEMENT_CLASS(UavaUIObject);
IMPLEMENT_CLASS(UavaUIScene);
IMPLEMENT_CLASS(UavaUICharacterPIP);
IMPLEMENT_CLASS(UavaUICustomImage);

void UavaUICharacterPIP::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize( inOwnerScene, inOwner );
}

void UavaUICharacterPIP::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* OutermostProperty = PropertyThatChanged.GetHead()->GetValue();
		if ( OutermostProperty != NULL )
		{
			FName PropertyName = OutermostProperty->GetFName();
			if ( PropertyName == TEXT("Template") || PropertyName == TEXT("WeaponTemplate") )
			{
				eventRefresh();
			}
		}
	}

	Super::PostEditChange( PropertyThatChanged );
}

void UavaUICharacterPIP::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );
}

void UavaUIScene::Initialize(UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);

	// Iterate over all properties and auto-add any instanced objects

	for( TFieldIterator<UObjectProperty,CLASS_IsAUObjectProperty> It( GetClass() ); It; ++It)
	{
		UObjectProperty* P = *It;

		//@todo xenon: fix up with May XDK
		DWORD PropertyFlagMask = P->PropertyFlags&(CPF_EditInline | CPF_ExportObject);
		if (PropertyFlagMask != 0)
		{
			for (INT Index=0; Index < It->ArrayDim; Index++)
			{
				UObject* HudObject = *((UObject**)((BYTE*)this + It->Offset + (Index * It->ElementSize) ));
				if ( HudObject )
				{
					UUIObject* UIObject = Cast<UUIObject>(HudObject);
					if ( UIObject )
					{
						// Should not be trying to attach a default object!
						if(UIObject->IsTemplate())
						{
							appErrorf( TEXT("UUTUIObject::Initialize : Trying to insert Default Object %s (from property %s) into %s."), *UIObject->GetName(), *P->GetName(), *GetName() );
						}

						//@HACK - Big hack time.  The current UI doesn't support any type of render ordering
						// so rendering is determined solely by the order in which the components are added to
						// the children array.  To kind of account for this, we insert each child in to the 
						// array at the head. 
						//
						// This means you need to be careful with how you layout your member variables.
						// Members in a child will be found before members in the parent.  Otherwise they are in the 
						// order as defined by the code (top down).

						// Once a render ordering scheme is in place, remove the forced index and just let nature do the work.

						InsertChild(UIObject, 0);
					}
				}
			}
		}
		else
			debugf(TEXT("Failed!"));
	}
}

/*=========================================================================================
UTUIObject - Our UIObjects support a PreRender and Tick pass as well as hold several 
support functions for getting quick access to owners.  The biggest addition is that UTUIObjects
will attempt to auto-seed their children stack during the initialization phase.
========================================================================================= */

/** 
* Automatically add any children defined within the object to the array.
*
* @see UUIObject.Intiailize
*/

void UavaUIObject::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	Super::Initialize(inOwnerScene, inOwner);

	// Iterate over all properties and auto-add any instanced objects

	for( TFieldIterator<UObjectProperty,CLASS_IsAUObjectProperty> It( GetClass() ); It; ++It)
	{
		UObjectProperty* P = *It;

		//@todo xenon: fix up with May XDK
		DWORD PropertyFlagMask = P->PropertyFlags&(CPF_EditInline | CPF_ExportObject);
		if (PropertyFlagMask != 0)
		{
			for (INT Index=0; Index < It->ArrayDim; Index++)
			{
				UObject* HudObject = *((UObject**)((BYTE*)this + It->Offset + (Index * It->ElementSize) ));
				if ( HudObject )
				{
					UUIObject* UIObject = Cast<UUIObject>(HudObject);
					if ( UIObject )
					{
						// Should not be trying to attach a default object!
						if(UIObject->IsTemplate())
						{
							appErrorf( TEXT("UUTUIObject::Initialize : Trying to insert Default Object %s (from property %s) into %s."), *UIObject->GetName(), *P->GetName(), *GetName() );
						}

						//@HACK - Big hack time.  The current UI doesn't support any type of render ordering
						// so rendering is determined solely by the order in which the components are added to
						// the children array.  To kind of account for this, we insert each child in to the 
						// array at the head. 
						//
						// This means you need to be careful with how you layout your member variables.
						// Members in a child will be found before members in the parent.  Otherwise they are in the 
						// order as defined by the code (top down).

						// Once a render ordering scheme is in place, remove the forced index and just let nature do the work.

						InsertChild(UIObject, 0);
					}
				}
			}
		}
	}
}


/**
* @Returns the Owner of the widget.  For some reason there isn't a script native for this.  Returns it precasted to
* a UTUIObject
*/
UavaUIObject* UavaUIObject::GetAvaWidgetOwner()
{
	return Cast<UavaUIObject>( GetOwner() );
}

/**
* This function will attempt to find the UTPlayerController associated with an Index.  If that Index is < 0 then it will find the first 
* available UTPlayerController.  CurrentPlayer in the UIClient isn't valid yet.
*
* @Param	Index		This is the index in to the GamePlayers array.  If -1, the function will attempt to find the first player
*/
AavaPlayerController* UavaUIObject::GetAvaPlayerOwner(INT Index)
{
	// Attempt to find the first available
	if (Index < 0)
	{
		for (INT i=0;i<GEngine->GamePlayers.Num(); i++)
		{
			// {{ 20071005 dEAthcURe|HM check valid
			ULocalPlayer* pLocalPlayer = GEngine->GamePlayers(i);
			if(0x0 != pLocalPlayer && FALSE == pLocalPlayer->IsValid()) {
				debugf(TEXT("invalid local player"));
			}

			if(0x0 != pLocalPlayer && TRUE == pLocalPlayer->IsValid())
			//if ( GEngine->GamePlayers(i) )	
			// }} 20071005 dEAthcURe|HM check valid
			{
				Index = i;
				break;
			}
		}
	}

	// Cast and return

	ULocalPlayer* CurrentPlayer = GEngine->GamePlayers.Num() > 0 ? GEngine->GamePlayers(Index) : NULL;
	if ( CurrentPlayer )
	{
		AavaPlayerController* avaPC = Cast<AavaPlayerController>( CurrentPlayer->Actor );
		return avaPC;
	}
	return NULL;
}

/**
* This function will attempt to resolve the pawn that is associated with this scene
*/
APawn* UavaUIObject::GetPawnOwner()
{
	AavaPlayerController* PlayerOwner = GetAvaPlayerOwner();
	if ( PlayerOwner )
	{
		AActor* ViewTarget = PlayerOwner->ViewTarget;
		if ( !ViewTarget )
		{
			ViewTarget = PlayerOwner->Pawn;
		}

		return Cast<APawn>(ViewTarget);
	}
	return NULL;
}

AavaPlayerReplicationInfo* UavaUIObject::GetPRIOwner()
{
	APawn* PawnOwner = GetPawnOwner();
	if ( !PawnOwner )
	{
		AavaPlayerController* PlayerOwner = GetAvaPlayerOwner();
		if ( PlayerOwner )
		{
			return Cast<AavaPlayerReplicationInfo>(PlayerOwner->PlayerReplicationInfo);
		}
	}
	else
	{
		return Cast<AavaPlayerReplicationInfo>(PawnOwner->PlayerReplicationInfo);
	}
	return NULL;
}

AWorldInfo* UavaUIObject::GetWorldInfo()
{
	return GWorld ? GWorld->GetWorldInfo() : NULL;
}


/** 
* !!! WARNING !!! This function does not check the destination and assumes it is valid.
*
* LookupProperty - Finds a property of a source actor and returns it's value.
*
* @param		SourceActor			The actor to search
* @param		SourceProperty		The property to look up
* @out param 	DestPtr				A Point to the storgage of the value
*
* @Returns true if the look up succeeded
*/

UBOOL UavaUIObject::LookupProperty(AActor *SourceActor, FName SourceProperty, BYTE* DestPtr)
{
	if ( SourceActor && SourceProperty != NAME_None )
	{
		UProperty* Prop = FindField<UProperty>( SourceActor->GetClass(), SourceProperty);
		if ( Prop )
		{
			BYTE* PropLoc = (BYTE*) SourceActor + Prop->Offset;
			if ( Cast<UIntProperty>(Prop) )
			{
				UIntProperty* IntProp = Cast<UIntProperty>(Prop);
				IntProp->CopySingleValue( (INT*)(DestPtr), PropLoc);
				return true;
			}
			else if ( Cast<UBoolProperty>(Prop) )
			{
				UBoolProperty* BoolProp = Cast<UBoolProperty>(Prop);
				BoolProp->CopySingleValue( (UBOOL*)(DestPtr), PropLoc);
				return true;
			}
			else if (Cast<UFloatProperty>(Prop) )
			{
				UFloatProperty* FloatProp = Cast<UFloatProperty>(Prop);
				FloatProp->CopySingleValue( (FLOAT*)(DestPtr), PropLoc);
				return true;
			}
			else if (Cast<UByteProperty>(Prop) )
			{
				UByteProperty* ByteProp = Cast<UByteProperty>(Prop);
				ByteProp->CopySingleValue( (BYTE*)(DestPtr), PropLoc);
				return true;
			}
			else
			{
				debugf(TEXT("Unhandled Property Type (%s)"),*Prop->GetName());
				return false;
			}
		}
	}
	debugf(TEXT("Illegal Proptery Operation (%s) %s"),*GetName(), *SourceProperty.ToString());
	return false;
}		


/** 
* If bShowBounds is true, this control will always render it's bounds 
*/

void UavaUIObject::Render_Widget(FCanvas* Canvas)
{
	if ( bShowBounds ) 
	{
		// Render the bounds 
		FLOAT X  = RenderBounds[UIFACE_Left];
		FLOAT Y  = RenderBounds[UIFACE_Top];
		FLOAT tW = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
		FLOAT tH = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

		DrawTile(Canvas, X,Y, tW, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,FLinearColor(1.0,1.0,1.0,1.0));
		DrawTile(Canvas, X,Y, 1.0f, tH, 0.0f, 0.0f, 1.0f, 1.0f,FLinearColor(1.0,1.0,1.0,1.0));

		DrawTile(Canvas, X,Y+tH, tW, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,FLinearColor(1.0,1.0,1.0,1.0));
		DrawTile(Canvas, X+tW,Y, 1.0f, tH, 0.0f, 0.0f, 1.0f, 1.0f,FLinearColor(1.0,1.0,1.0,1.0));
	}
}

/** 
* GetFontName is a helper function that will return a font using the font pool.  It uses the current scene's dims
*
* @Param	FontName	Name of the font to load
*/

UFont* UavaUIObject::GetFont(FName FontName)
{
	//UUTUIScene* UTScene = Cast<UUTUIScene>( GetSceneClient() );
	//if ( UTScene )
	//{
	//	UUTFontPool* FontPool = UTScene->GetFontPool(FontName);
	//	if ( FontPool )
	//	{
	//		FLOAT SceneHeight = GetScene()->Position.GetBoundsExtent( GetScene(), UIORIENT_Vertical, EVALPOS_PixelViewport);
	//		return FontPool->GetFont( appTrunc(SceneHeight) );
	//	}
	//}

	return NULL;
}

//-----------------------------------------------------------------------------
//	class UavaUICustomImage
//-----------------------------------------------------------------------------

UBOOL UavaUICustomImage::RefreshSubscriberValue(INT BindingIndex)
{
	// 추가작업.
	UavaUIComp_DrawCustomImage *pUIComp = Cast<UavaUIComp_DrawCustomImage>(ImageComponent);
	if ( pUIComp != NULL )
	{
		pUIComp->eventOnChangeIniName();
	}

	return UUIImage::RefreshSubscriberValue(BindingIndex);
}
