/**
 * Represents the "active" widget state.  This state indicates that the widget is currently being moused over or
 * is otherwise selected without necessarily having focus.
 *
 * Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
 */
class UIState_Active extends UIState
	native(UIPrivate);

cpptext
{
	/**
	 * Notification that Target has made this state its active state.
	 *
	 * @param	Target			the widget that activated this state.
	 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
	 * @param	bPushState		TRUE if this state needs to be added to the state stack for the owning widget; FALSE if this state was already
	 *							in the state stack of the owning widget and is being activated for additional split-screen players.
	 */
	virtual void OnActivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPushState );

	/**
	 * Notification that Target has just deactivated this state.
	 *
	 * @param	Target			the widget that deactivated this state.
	 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
	 * @param	bPopState		TRUE if this state needs to be removed from the owning widget's StateStack; FALSE if this state is
	 *							still active for at least one player (i.e. in splitscreen)
	 */
	virtual void OnDeactivate( UUIScreenObject* Target, INT PlayerIndex, UBOOL bPopState );
}

/**
 * Deactivate this state for the specified target.
 * This version prevents the active state from being deactivated if the widget is currently in the pressed state.
 *
 * @param	Target			the widget that is deactivating this state.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated this call
 *
 * @return	TRUE to allow this state to be deactivated for the specified Target.
 */
event bool DeactivateState( UIScreenObject Target, int PlayerIndex )
{
	local bool bResult;
	local UIObject Owner;
	local UIScene OwnerScene;
	local int PressedStateIndex;
//	Local UIScreenObject ParentObject;

	bResult = Super.DeactivateState(Target,PlayerIndex);

	// if nothing in our super class's DeactivateState wants to prevent this state from being deactivated
	if ( Target != None && bResult )
	{
		// Find out if the widget is currently in the pressed state.
		if ( Target.HasActiveStateOfClass(class'UIState_Pressed',PlayerIndex,PressedStateIndex) )
		{
			// unless the widget's scene has been deactivated, keep the widget in the active state until it's no longer in the pressed state.  This is so that
			// the widget remains the scene client's ActiveControl as long as the user is still holding e.g. the mouse button.
			Owner = UIObject(Target);
			if ( Owner != None )
				OwnerScene = Owner.GetScene();
			else
				OwnerScene = UIScene(Target);

			if( OwnerScene != None )
			{
				if( OwnerScene.IsSceneReachable() )
				{
					//버튼들은 풀어준다(버튼은 일반적으로 마우스를 캡쳐하지 않는다)
					if ( UIButton(Target) != None && UIScrollBarMarkerButton(Target) == none )
						bResult = true;
					else
						bResult = false;
				}
			}
			//else
			//{
			//	ParentObject = Target.GetParent();
			//	if ( ParentObject != None && class<UIScrollBar>(ParentObject.Class) != None  )
			//		bResult = false;
			//}
		}
	}

	if( bResult == true )
		Target.DeactivateStateByClass(class'UIState_Pressed', PlayerIndex );

	return bResult;
}

DefaultProperties
{

}
