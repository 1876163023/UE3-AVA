/*=============================================================================
  avaUIObject
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/08/21 by OZ

		UIObject 를 확장한 AVA Game 을 위한 UI Object 이다.
		UT Game 을 참조하여 만들었다...
		
=============================================================================*/
class avaUIObject extends UIObject
	abstract
	native;

/** If true, this object will render it's bounds */
var(Widget) transient bool bShowBounds;

cpptext
{
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );	
	virtual void Render_Widget(FCanvas* Canvas);		

	/**
	  * WARNING: This function does not check the destination and assumes it is valid.
	  *
	  * LookupProperty - Finds a property of a source actor and returns it's value.
	  *
	  * @param		SourceActor			The actor to search
	  * @param		SourceProperty		The property to look up
	  * @out param 	DestPtr				A Point to the storgage of the value
	  *
	  * @Returns true if the look up succeeded
	  */
	virtual UBOOL LookupProperty(AActor* SourceActor, FName SourceProperty, BYTE* DestPtr);


}

/**
 * Script side version of GetOwner() but returns UTGame specific
 */

native function avaUIObject GetAvaWidgetOwner();

/**
 * Get the avaPlayerController that is associated with this hud
 */
native function avaPlayerController GetAvaPlayerOwner(optional int Index);

/**
 * Returns the Pawn associated with this hud
 */
native function Pawn GetPawnOwner();

/** 
 * Returns the PRI associated with this hud 
 */
 
native function avaPlayerReplicationInfo GetPRIOwner();

/**
 * Returns a font from the font pool
 */
native function Font GetFont(name FontName);

/**
 * Returns the WorldInfo 
 */
native function WorldInfo GetWorldInfo();




defaultproperties
{
}


