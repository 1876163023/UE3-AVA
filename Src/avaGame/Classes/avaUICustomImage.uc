/*
	avaUIComp_DrawCustomImage를 사용하는 공지, 배너용 클래스.

	2007/04/02	고광록
*/
class avaUICustomImage extends UIImage
	native;

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
native virtual function bool RefreshSubscriberValue( optional int BindingIndex=INDEX_NONE );

DefaultProperties
{
	Begin Object Class=avaUIComp_DrawCustomImage Name=CustomImageComponentTemplate
		ImageStyle=(DefaultStyleTag="DefaultImageStyle",RequiredStyleClass=class'Engine.UIStyle_Image')
	End Object
	ImageComponent=CustomImageComponentTemplate
}