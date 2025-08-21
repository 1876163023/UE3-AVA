/***

	Copyright (c) 2006 Redduck Inc. All rights reserved.

	Project: AVA

	Name: UIDataStore_AvaNet.uc

	Description: Supplys information handled by avaNet module.

***/
class avaUIDataStore_AvaGame extends UIDataStore
	native(inherit)
	implements(UIListElementProvider)
	transient;

`include(Core/Globals.uci)

var avaUIDataProvider_AvaNetChatMsgs		AvaNetChatMsgProvider;

var avaUIDataProvider_PlayerSkill			PlayerSkillProvider;
var avaUIDataProvider_PlayerAward			PlayerAwardProvider;
var avaUIDataProvider_LastResultAwardSkill	LastResultAwardSkillProvider;

var avaUIDataProvider_KeyMap				KeyMapProvider;

var avaUIDataProvider_OptionValueList		OptionValueProvider;
var avaUIDataProvider_GroupValueList		GroupValueProvider;

var avaUIDataProvider_ChannelListField		ChannelListFieldProvider;

var avaUIDataProvider_GameProperty			GamePropertyProvider;

cpptext
{

	/**
	 * Hook for performing any initialization required for this data store
	 */
	virtual void InitializeDataStore();

	/**
	 * Resolves the value of the data field specified and stores it in the output parameter.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see ParseDataStoreReference for additional notes
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL GetFieldValue( const FString& FieldName, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE );

	/**
	 * Resolves the value of the data field specified and stores the value specified to the appropriate location for that field.
	 *
	 * @param	FieldName		the data field to resolve the value for;  guaranteed to correspond to a property that this provider
	 *							can resolve the value for (i.e. not a tag corresponding to an internal provider, etc.)
	 * @param	FieldValue		the value to store for the property specified.
	 * @param	ArrayIndex		optional array index for use with data collections
	 */
	virtual UBOOL SetFieldValue( const FString& FieldName, const struct FUIProviderScriptFieldValue& FieldValue, INT ArrayIndex=INDEX_NONE );

	/**
	 * Gets the list of data fields exposed by this data provider.
	 *
	 * @param	out_Fields	will be filled in with the list of tags which can be used to access data in this data provider.
	 *						Will call GetScriptDataTags to allow script-only child classes to add to this list.
	 */
	virtual void GetSupportedDataFields( TArray<struct FUIDataProviderField>& out_Fields );

	/**
	 * Generates filler data for a given tag.  This is used by the editor to generate a preview that gives the
	 * user an idea as to what a bound datastore will look like in-game.
	 *
 	 * @param		DataTag		the tag corresponding to the data field that we want filler data for
 	 *
	 * @return		a string of made-up data which is indicative of the typical [resolved] value for the specified field.
	 */
	virtual FString GenerateFillerData( const FString& DataTag );

/* === IUIListElementProvider interface === */

	/**
	 * Retrieves the list of all data tags contained by this element provider which correspond to list element data.
	 *
	 * @return	the list of tags supported by this element provider which correspond to list element data.
	 */
	virtual TArray<FName> GetElementProviderTags();

	/**
	 * Returns the number of list elements associated with the data tag specified.
	 *
	 * @param	FieldName	the name of the property to get the element count for.  guaranteed to be one of the values returned
	 *						from GetElementProviderTags.
	 *
	 * @return	the total number of elements that are required to fully represent the data specified.
	 */
	virtual INT GetElementCount( FName FieldName );

	/**
	 * Retrieves the list elements associated with the data tag specified.
	 *
	 * @param	FieldName		the name of the property to get the element count for.  guaranteed to be one of the values returned
	 *							from GetElementProviderTags.
	 * @param	out_Elements	will be filled with the elements associated with the data specified by DataTag.
	 *
	 * @return	TRUE if this data store contains a list element data provider matching the tag specified.
	 */
	virtual UBOOL GetListElements( FName FieldName, TArray<INT>& out_Elements );

	/**
	 * Retrieves a list element for the specified data tag that can provide the list with the available cells for this list element.
	 * Used by the UI editor to know which cells are available for binding to individual list cells.
	 *
	 * @param	FieldName		the tag of the list element data provider that we want the schema for.
	 *
	 * @return	a pointer to some instance of the data provider for the tag specified.  only used for enumerating the available
	 *			cell bindings, so doesn't need to actually contain any data (i.e. can be the CDO for the data provider class, for example)
	 */
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellSchemaProvider( FName FieldName );

	/**
	 * Retrieves a UIListElementCellProvider for the specified data tag that can provide the list with the values for the cells
	 * of the list element indicated by CellValueProvider.DataSourceIndex
	 *
	 * @param	FieldName		the tag of the list element data field that we want the values for
	 * @param	ListIndex		the list index for the element to get values for
	 *
	 * @return	a pointer to an instance of the data provider that contains the value for the data field and list index specified
	 */
	virtual TScriptInterface<class IUIListElementCellProvider> GetElementCellValueProvider( FName FieldName, INT ListIndex );

	/**
	 * Resolves PropertyName into a list element provider that provides list elements for the property specified.
	 *
	 * @param	PropertyName	the name of the property that corresponds to a list element provider supported by this data store
	 *
	 * @return	a pointer to an interface for retrieving list elements associated with the data specified, or NULL if
	 *			there is no list element provider associated with the specified property.
	 */
	virtual TScriptInterface<class IUIListElementProvider> ResolveListElementProvider( const FString& PropertyName );

	/**
	 * Determines whether a member of a collection should be considered "enabled" by subscribed lists.  Disabled elements will still be displayed in the list
	 * but will be drawn using the disabled state.
	 *
	 * @param	FieldName			the name of the collection data field that CollectionIndex indexes into.
	 * @param	CollectionIndex		the index into the data field collection indicated by FieldName to check
	 *
	 * @return	TRUE if FieldName doesn't correspond to a valid collection data field, CollectionIndex is an invalid index for that collection,
	 *			or the item is actually enabled; FALSE only if the item was successfully resolved into a data field value, but should be considered disabled.
	 */
	virtual UBOOL IsElementEnabled( FName FieldName, INT CollectionIndex );

	/**
	 * Allows list element providers the chance to perform custom sorting of a collection of list elements.  Implementors should implement this
	 * method if they desire to perform complex sorting behavior, such as considering additional data when evaluting the order to sort the elements into.
	 *
	 * @param	CollectionDataFieldName		the name of a collection data field inside this UIListElementProvider associated with the
	 *										list items provided.  Guaranteed to one of the values returned from GetElementProviderTags.
	 * @param	ListItems					the array of list items that need sorting.
	 * @param	OutputType	determines the format of the result.
	 *						EVALPOS_None:
	 *							return value is formatted using this screen position's ScaleType for the specified face
	 *						EVALPOS_PercentageOwner:
	 * @param	SortParameters				the parameters to use for sorting
	 *										PrimaryIndex:
	 *											the index [into the ListItems' Cells array] for the cell which the user desires to perform primary sorting with.
	 *										SecondaryIndex:
	 *											the index [into the ListItems' Cells array] for the cell which the user desires to perform secondary sorting with.  Not guaranteed
	 *											to be a valid value; Comparison should be performed using the value of the field indicated by PrimarySortIndex, then when these
	 *											values are identical, the value of the cell field indicated by SecondarySortIndex should be used.
	 *
	 * @return	TRUE to indicate that custom sorting was performed by this UIListElementProvider.  Custom sorting is not required - if this method returns FALSE,
	 *			the list bound to this UIListElementProvider will perform its default sorting behavior (alphabetical sorting of the desired cell values)
	 */
	virtual UBOOL SortListElements( FName CollectionDataFieldName, TArray<struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters );
}


/**
 * Called when this data store is added to the data store manager's list of active data stores.
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
event Registered( LocalPlayer PlayerOwner )
{
	`log( PlayerOwner @ ": UIDataStore_AvaGame registered" );
}

/**
 * Called when this data store is removed from the data store manager's list of active data stores.
 *
 * @param	PlayerOwner		the player that will be associated with this DataStore.  Only relevant if this data store is
 *							associated with a particular player; NULL if this is a global data store.
 */
event Unregistered( LocalPlayer PlayerOwner )
{
	`log( PlayerOwner @ ": UIDataStore_AvaGame unregistered" );
}


defaultproperties
{
	Tag=avaGame
}
