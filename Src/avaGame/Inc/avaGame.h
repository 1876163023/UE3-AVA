//=============================================================================
// Copyright 2005 Epic Games - All Rights Reserved.
// Confidential.
//=============================================================================

#ifndef _INC_ENGINE

#include "Engine.h"
#include "EngineAnimClasses.h"
#include "EngineAIClasses.h"
#include "EnginePhysicsClasses.h"
#include "EngineDecalClasses.h"
#include "EngineDSPClasses.h"
#include "UnTerrain.h"
#include "EngineSequenceClasses.h"
#include "GameFrameworkClasses.h"
#include "EngineParticleClasses.h"
#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"
#include "EngineUISequenceClasses.h"

class wxMenu;

struct FGenericBrowserTypeInfo
{
	class UClass* Class;
	FColor BorderColor;
	QWORD RequiredFlags;
	wxMenu* ContextMenu;
	class UGenericBrowserType* BrowserType;
	FPointer IsSupportedCallback;

	typedef UBOOL (*GenericBrowserSupportCallback)(UObject* Object);

	FGenericBrowserTypeInfo( 
		UClass* InClass, 
		const FColor& InBorderColor, 
		wxMenu* InContextMenu, 
		QWORD InRequiredFlags = 0, 
		UGenericBrowserType* InBrowserType = NULL, 
		GenericBrowserSupportCallback InIsSupportedCallback = NULL
		)
		:	Class(InClass)
		,	ContextMenu(InContextMenu)
		,	RequiredFlags(InRequiredFlags)
		,	BorderColor(InBorderColor)
		,	BrowserType(InBrowserType)
		,	IsSupportedCallback(InIsSupportedCallback)
	{}

	UBOOL Supports( UObject* Object ) const
	{
		UBOOL bResult = FALSE;
		if ( Object->IsA(Class) )
		{
			bResult = TRUE;
			if ( RequiredFlags != 0 )
			{
				bResult = Object->HasAllFlags(RequiredFlags);
				if ( RequiredFlags != 0 )
				{
					bResult = Object->HasAllFlags(RequiredFlags);
				}
			}
			if( bResult && IsSupportedCallback )
			{
				GenericBrowserSupportCallback Callback = (GenericBrowserSupportCallback) IsSupportedCallback;
				bResult = Callback( Object );
			}
		}
		return bResult;
	}

	inline UBOOL operator==( const FGenericBrowserTypeInfo& Other ) const
	{
		return ( Class == Other.Class && RequiredFlags == Other.RequiredFlags );
	}

};

class UGenericBrowserType : public UObject
{
public:
	//## BEGIN PROPS GenericBrowserType
	FStringNoInit Description;
	TArrayNoInit<struct FGenericBrowserTypeInfo> SupportInfo;
	FColor BorderColor;
	//## END PROPS GenericBrowserType

	DECLARE_ABSTRACT_CLASS(UGenericBrowserType,UObject,0,UnrealEd)
	/**
	* @return Returns the browser type description string.
	*/
	const FString& GetBrowserTypeDescription() const
	{ 
		return Description; 
	}

	FColor GetBorderColor( UObject* InObject );

	/**
	* Does any initial set up that the type requires.
	*/
	virtual void Init() {}

	/**
	* Checks to see if the specified class is handled by this type.
	*
	* @param	InObject	The object we need to check if we support
	*/
	UBOOL Supports( UObject* InObject );

	/**
	* Creates a context menu specific to the type of object passed in.
	*
	* @param	InObject	The object we need the menu for
	*/
	wxMenu* GetContextMenu( UObject* InObject );

	/**
	* Invokes the editor for an object.  The default behaviour is to
	* open a property window for the object.  Dervied classes can override
	* this with eg an editor which is specialized for the object's class.
	*
	* @param	InObject	The object to invoke the editor for.
	*/
	virtual UBOOL ShowObjectEditor( UObject* InObject )
	{
		return ShowObjectProperties( InObject );
	}

	/**
	* Opens a property window for the specified object.  By default, GEditor's
	* notify hook is used on the property window.  Derived classes can override
	* this method in order to eg provide their own notify hook.
	*
	* @param	InObject	The object to invoke the property window for.
	*/
	virtual UBOOL ShowObjectProperties( UObject* InObject );

	/**
	* Opens a property window for the specified objects.  By default, GEditor's
	* notify hook is used on the property window.  Derived classes can override
	* this method in order to eg provide their own notify hook.
	*
	* @param	InObjects	The objects to invoke the property window for.
	*/
	virtual UBOOL ShowObjectProperties( const TArray<UObject*>& InObjects );

	/**
	* Invokes the editor for all selected objects.
	*/
	virtual UBOOL ShowObjectEditor();

	/**
	* Displays the object properties window for all selected objects that this
	* GenericBrowserType supports.
	*/
	UBOOL ShowObjectProperties();

	/**
	* Invokes a custom menu item command for every selected object
	* of a supported class.
	*
	* @param InCommand		The command to execute
	*/

	virtual void InvokeCustomCommand( INT InCommand );

	/**
	* Invokes a custom menu item command.
	*
	* @param InCommand		The command to execute
	* @param InObject		The object to invoke the command against
	*/

	virtual void InvokeCustomCommand( INT InCommand, UObject* InObject ) {}

	/**
	* Calls the virtual "DoubleClick" function for each object
	* of a supported class.
	*/

	virtual void DoubleClick();

	/**
	* Allows each type to handle double clicking as they see fit.
	*/

	virtual void DoubleClick( UObject* InObject );

	/**
	* Retrieves a list of objects supported by this browser type which
	* are currently selected in the generic browser.
	*/
	void GetSelectedObjects( TArray<UObject*>& Objects );

	/**
	* Called when the user chooses to delete objects from the generic browser.  Gives the resource type the opportunity
	* to perform any special logic prior to the delete.
	*
	* @param	ObjectToDelete	the object about to be deleted.
	*
	* @return	TRUE to allow the object to be deleted, FALSE to prevent the object from being deleted.
	*/
	virtual UBOOL NotifyDeletingObject( UObject* ObjectToDelete ) { return TRUE; }
};

#endif

#include "avaGameClasses.h"
#if !FINAL_RELEASE
#include "avaGameEditorClasses.h"
#endif
#include "avaTrail.h"
#include "avaGameParticleClasses.h"
#include "avaGameSequenceClasses.h"
#include "avaGameNetClasses.h"
#include "avaGameCameraClasses.h"
#include "avaGameVideoClasses.h"
#include "avaGameUIPrivateClasses.h"

#include "avaCommandlets.h"
#include "AvaTemplate.h"

#ifdef EnableHostMigration
#include "hmDataTypes.h" // 20061122 dEAthcURe|HM
#endif

#ifndef _UT_INTRINSIC_CLASSES
#define _UT_INTRINSIC_CLASSES

BEGIN_COMMANDLET(avaLevelCheck,avaGame)
	void StaticInitialize()
	{
		IsEditor = FALSE;
	}
END_COMMANDLET

BEGIN_COMMANDLET(avaReplaceActor,avaGame)
	void StaticInitialize()
	{
		IsEditor = TRUE;
	}
END_COMMANDLET

/** factory for UTWarfareTerrainMaterial */
class UavaOnslaughtTerrainLayerSetupFactoryNew : public UFactory
{
	DECLARE_CLASS(UavaOnslaughtTerrainLayerSetupFactoryNew, UFactory, CLASS_CollapseCategories, UTGame);

public:
	void StaticConstructor();
	/**
	* Initializes property values for intrinsic classes.  It is called immediately after the class default object
	* is initialized against its archetype, but before any objects of this class are created.
	*/
	void InitializeIntrinsicPropertyValues();
	UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn);
};

FLOAT static CalcRequiredAngle(FVector Start, FVector Hit, INT Length)
{
	FLOAT Dist = (Hit - Start).Size();
	FLOAT Angle = 90 - ( appAcos( Dist / FLOAT(Length) ) * 57.2957795);

	return Clamp<FLOAT>(Angle,0,90);
}

#endif