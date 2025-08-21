/**
 * Provides an interface for objects which provide data for list element cells.
 *
 * Copyright ?1998-2007 Epic Games, Inc. All Rights Reserved
 */
interface UIListElementCellProvider
	native(UIPrivate);

cpptext
{
	/**
	 * Retrieves the list of tags that can be bound to individual cells in a single list element, along with the human-readable,
	 * localized string that should be used in the header for each cell tag (in lists which have column headers enabled).
	 *
	 * @param	out_CellTags	receives the list of tag/column headers that can be bound to element cells for the specified property.
	 */
	virtual void GetElementCellTags( TMap<FName,FString>& out_CellTags )=0;

	/**
	 * Retrieves the field type for the specified cell.
	 *
	 * @param	CellTag				the tag for the element cell to get the field type for
	 * @param	out_CellFieldType	receives the field type for the specified cell; should be a EUIDataProviderFieldType value.
	 *
	 * @return	TRUE if this element cell provider contains a cell with the specified tag, and out_CellFieldType was changed.
	 */
	virtual UBOOL GetCellFieldType( const FName& CellTag, BYTE& out_CellFieldType )=0;

	/**
	 * Resolves the value of the cell specified by CellTag and stores it in the output parameter.
	 *
	 * @param	CellTag			the tag for the element cell to resolve the value for
	 * @param	ListIndex		the UIList's item index for the element that contains this cell.  Useful for data providers which
	 *							do not provide unique UIListElement objects for each element.
	 * @param	out_FieldValue	receives the resolved value for the property specified.
	 *							@see GetDataStoreValue for additional notes
	 * @param	ArrayIndex		optional array index for use with cell tags that represent data collections.  Corresponds to the
	 *							ArrayIndex of the collection that this cell is bound to, or INDEX_NONE if CellTag does not correspond
	 *							to a data collection.
	 */
	virtual UBOOL GetCellFieldValue( const FName& CellTag, INT ListIndex, struct FUIProviderFieldValue& out_FieldValue, INT ArrayIndex=INDEX_NONE )=0;

	/**
	 * Allows list element providers the chance to perform custom sorting of a collection of list elements.  Implementors should implement this
	 * method if they desire to perform complex sorting behavior, such as considering additional data when evaluting the order to sort the elements into.
	 *
	 * @param	PrimaryCellTag				정렬을 하기위한 기본 필드
	 * @param	SecondaryCellTag			PrimaryCellTag를 기준으로 정렬할시에 같은 값이 생길경우 적용할 2번째 정렬 필드
	 * @param	ListItems					정렬할 대상 아이템들
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
	virtual UBOOL SortListElements( const FName& PrimaryCellTag, const FName& SecondaryCellTag, TArray< /*const*/ struct FUIListItem>& ListItems, const struct FUIListSortingParameters& SortParameters ){ return FALSE; }
}

DefaultProperties
{

}
