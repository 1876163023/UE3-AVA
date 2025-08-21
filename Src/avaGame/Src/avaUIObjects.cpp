#include "PrecompiledHeaders.h"

// Includes.
#include "avaGame.h"
#include "avatemplate.h"

IMPLEMENT_CLASS(UavaUICheckLabelButton);
IMPLEMENT_CLASS(UavaUIEditBoxMisc);
IMPLEMENT_CLASS(UavaUIOutfitter);
IMPLEMENT_CLASS(UavaUIMouseTracker);
IMPLEMENT_CLASS(UavaUIChart);
IMPLEMENT_CLASS(UavaUIFloatingPanel);

/**
* Render this checkbox.
*
* @param	Canvas	the FCanvas to use for rendering this widget
*/
void UavaUICheckLabelButton::Render_Widget( FCanvas* Canvas )
{
	if( (!bDrawExclusive) || 
		(bDrawExclusive && !bIsChecked) )
		Super::Render_Widget( Canvas );

	if( bIsChecked )
	{
		FRenderParameters Parameters(
			RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top],
			RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left],
			RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top]
		);

		if ( CheckedImageComponent != NULL )
		{
			CheckedImageComponent->RenderComponent( Canvas, Parameters );
		}
		if ( CheckedStringComponent != NULL )
		{
			CheckedStringComponent->Render_String( Canvas );
		}
	}
}

/**
* Changes the checked image for this checkbox, creating the wrapper UITexture if necessary.
*
* @param	NewImage		the new surface to use for this UIImage
*/
void UavaUICheckLabelButton::SetCheckImage( class USurface* NewImage )
{
	if ( CheckedImageComponent != NULL )
	{
		CheckedImageComponent->SetImage(NewImage);
	}
}

/**
* Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
*
* This version adds the CheckedImageComponent (if non-NULL) to the StyleSubscribers array.
*/
void UavaUICheckLabelButton::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	AddStyleSubscriber( CheckedImageComponent );
	AddStyleSubscriber( CheckedStringComponent);
}

/**
* Handles input events for this checkbox.
*
* @param	EventParms		the parameters for the input event
*
* @return	TRUE to consume the key event, FALSE to pass it on.
*/
UBOOL UavaUICheckLabelButton::ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;

	//Translate the Unreal key into a UI Event Key.
	FName UIKey;
	if(bResult == FALSE )
	{
		if( EventParms.InputAliasName == UIKEY_Clicked)
		{
			if ( EventParms.EventType == IE_Pressed )
			{
				// notify unrealscript
				if ( DELEGATE_IS_SET(OnPressed) )
				{
					delegateOnPressed(this, EventParms.PlayerIndex);
				}

				// activate the pressed state
				ActivateStateByClass(UUIState_Pressed::StaticClass(), EventParms.PlayerIndex);
				bResult = TRUE;
			}
			else if ( EventParms.EventType == IE_Repeat )
			{
				if ( DELEGATE_IS_SET(OnPressRepeat) )
				{
					delegateOnPressRepeat(this, EventParms.PlayerIndex);
				}

				bResult = TRUE;
			}
			else if ( EventParms.EventType == IE_Released )
			{
				// Play the ClickedCue
				PlayUISound(ClickedCue,EventParms.PlayerIndex);

				// toggle the checked state of this checkbox
				if( bAutoToggle )
					SetValue(!bIsChecked,EventParms.PlayerIndex);

				// Fire OnPressed Delegate
				UBOOL bInputConsumed = FALSE;
				if ( DELEGATE_IS_SET(OnClicked) )
				{
					bInputConsumed = delegateOnClicked(this, EventParms.PlayerIndex);
				}

				// activate the on click event
				if( ! bInputConsumed )
					ActivateEventByClass(EventParms.PlayerIndex,UUIEvent_OnClick::StaticClass(), this);

				// deactivate the pressed state
				DeactivateStateByClass(UUIState_Pressed::StaticClass(), EventParms.PlayerIndex);
				bResult = TRUE;
			}
		}
	}

	// Make sure to call the superclass's implementation after trying to consume input ourselves so that
	// we can respond to events defined in the super's class.
	if(bResult == FALSE)
	{
		bResult = Super::ProcessInputKey(EventParms);
	}

	return bResult;
}

/* === UObject interface === */
/**
* Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
*/
void UavaUICheckLabelButton::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("CheckedImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty && CheckedImageComponent != NULL )
				{
					// the user either cleared the value of the CheckedImageComponent (which should never happen since
					// we use the 'noclear' keyword on the property declaration), or is assigning a new value to the CheckedImageComponent.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(CheckedImageComponent);
				}
			}
			else if ( PropertyName == TEXT("CheckedStringComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the CheckedStringComponent itself was changed
				if ( MemberProperty == ModifiedProperty && CheckedStringComponent != NULL )
				{
					// the user either cleared the value of the CheckedStringComponent or is assigning a new value to it.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(CheckedStringComponent);
				}
			}

		}
	}
}

/**
* Called when a property value from a member struct or array has been changed in the editor.
*/
void UavaUICheckLabelButton::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{

		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("CheckedImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( CheckedImageComponent != NULL )
					{
						UUIComp_DrawImage* ComponentTemplate = GetArchetype<UUICheckbox>()->CheckedImageComponent;
						if ( ComponentTemplate != NULL )
							CheckedImageComponent->StyleResolverTag = ComponentTemplate->StyleResolverTag;
						else
							CheckedImageComponent->StyleResolverTag = TEXT("Check Style");

						// user added created a new image component background - add it to the list of style subscribers
						AddStyleSubscriber(CheckedImageComponent);

						// now initialize the component's image
						CheckedImageComponent->SetImage(CheckedImageComponent->GetImage());
					}
				}
				else if ( CheckedImageComponent != NULL )
				{
					// a property of the ImageComponent was changed
					if ( ModifiedProperty->GetFName() == TEXT("ImageRef") && CheckedImageComponent->GetImage() != NULL )
					{
						USurface* CurrentValue = CheckedImageComponent->GetImage();

						// clearing the data store binding value may have cleared the value of the image component's texture,
						// so restore the value now
						SetCheckImage(CurrentValue);
					}
				}
			}
			else if ( PropertyName == TEXT("CheckedStringComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the ImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( CheckedStringComponent != NULL )
					{
						// user added created a new string render component - add it to the list of style subscribers
						AddStyleSubscriber(CheckedStringComponent);

						// now initialize the new string component
						TScriptInterface<IUIDataStoreSubscriber> Subscriber(this);
						CheckedStringComponent->InitializeComponent(&Subscriber);

						// then initialize its style
						CheckedStringComponent->NotifyResolveStyle(GetActiveSkin(), FALSE, GetCurrentState());

						// finally initialize its text
						RefreshSubscriberValue();
					}
				}
			}
		}

	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
* Called after this object has been completely de-serialized.  This version migrates values for the deprecated CheckedImage,
* CheckCoordinates, and CheckedStyle properties over to the CheckedImageComponent.
*/
void UavaUICheckLabelButton::PostLoad()
{
	Super::PostLoad();

	//if ( !GIsUCCMake )
	//{
	//	MigrateImageSettings(CheckedImage, CheckCoordinates, CheckStyle, CheckedImageComponent, TEXT("CheckedImage"));
	//}
}

void UavaUICheckLabelButton::SetValue( INT bShouldBeChecked, INT PlayerIndex /*=INDEX_NONE*/, UBOOL bSkipNotification /*=FALSE*/)
{
	if ( bIsChecked != bShouldBeChecked )
	{
		bIsChecked = bShouldBeChecked;

		TArray<INT> OutputLinkIndexes;
		OutputLinkIndexes.AddItem(bIsChecked);

		if ( PlayerIndex == INDEX_NONE )
			PlayerIndex = GetBestPlayerIndex();

		FUIProviderFieldValue FieldValue(EC_EventParm);
		FieldValue.RangeValue.MinValue = FieldValue.RangeValue.MaxValue = bIsChecked;
		FieldValue.RangeValue.SetCurrentValue( FieldValue.RangeValue.MinValue, FALSE );
		this->SetDataStoreFieldValue( CaptionDataSource.MarkupString, FieldValue, GetScene() );

		// activate the OnCheckValue changed event to notify anyone who cares that this checkbox's value has been changed
		if( !bSkipNotification )
			ActivateEventByClass(PlayerIndex,UavaUIEvent_CheckLabelValueChanged::StaticClass(), this, FALSE, &OutputLinkIndexes);
	}
}


void UavaUICheckLabelButton::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner )
{
	if ( CheckedStringComponent != NULL )
	{
		TScriptInterface<IUIDataStoreSubscriber> Subscriber(this);
		CheckedStringComponent->InitializeComponent(&Subscriber);
	}

	Super::Initialize(inOwnerScene, inOwner);
}

/**
* Adds the specified face to the DockingStack for the specified widget
*
* @param	DockingStack	the docking stack to add this docking node to.  Generally the scene's DockingStack.
* @param	Face			the face that should be added
*
* @return	TRUE if a docking node was added to the scene's DockingStack for the specified face, or if a docking node already
*			existed in the stack for the specified face of this widget.
*/
UBOOL UavaUICheckLabelButton::AddDockingNode( TArray<struct FUIDockingNode>& DockingStack, EUIWidgetFace Face )
{
	Super::AddDockingNode( DockingStack, Face );
	return CheckedStringComponent 
		? CheckedStringComponent->AddDockingNode(DockingStack, Face)
		: Super::AddDockingNode(DockingStack, Face);
}

/**
* Evalutes the Position value for the specified face into an actual pixel value.  Should only be
* called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
*
* @param	Face	the face that should be resolved
*/
void UavaUICheckLabelButton::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition(Face);

	if ( CheckedStringComponent != NULL )
	{
		CheckedStringComponent->ResolveFacePosition(Face);
	}
}

/**
* Called when a property is modified that could potentially affect the widget's position onscreen.
*/
void UavaUICheckLabelButton::RefreshPosition()
{
	Super::RefreshPosition();

	RefreshFormatting();
}

/**
* Called to globally update the formatting of all UIStrings.
*/
void UavaUICheckLabelButton::RefreshFormatting()
{
	Super::RefreshFormatting();
	if ( CheckedStringComponent != NULL )
	{
		CheckedStringComponent->ReapplyFormatting();
	}
}

void UavaUICheckLabelButton::SetCaption( const FString& NewText)
{
	Super::SetCaption( NewText );
	if (CheckedStringComponent != NULL && CheckedStringComponent->GetValue(FALSE) != NewText)
	{
		CheckedStringComponent->SetValue(NewText);
	}
}

/**
* Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
*
* @return	TRUE if this subscriber successfully resolved and applied the updated value.
*/
UBOOL UavaUICheckLabelButton::RefreshSubscriberValue( INT BindingIndex )
{
	Super::RefreshSubscriberValue( BindingIndex );
	// resolve the value of this label into a renderable string
	if ( CheckedStringComponent != NULL && BindingIndex < UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		CheckedStringComponent->SetValue(CaptionDataSource.MarkupString);
		CheckedStringComponent->ReapplyFormatting();
	}

	return TRUE;
}

/**
* Sets the text alignment for the string that the widget is rendering.
*
* @param	Horizontal		Horizontal alignment to use for text, UIALIGN_MAX means no change.
* @param	Vertical		Vertical alignment to use for text, UIALIGN_MAX means no change.
*/
void UavaUICheckLabelButton::SetTextAlignment(BYTE Horizontal, BYTE Vertical)
{
	Super::SetTextAlignment(Horizontal, Vertical);
	if ( CheckedStringComponent != NULL )
	{
		if( Horizontal != UIALIGN_MAX )
		{
			CheckedStringComponent->SetAlignment(UIORIENT_Horizontal, Horizontal);
		}

		if(Vertical != UIALIGN_MAX)
		{
			CheckedStringComponent->SetAlignment(UIORIENT_Vertical, Vertical);
		}
	}
}


/************************************************************************/
/* UavaUIEditBoxmisc                                                    */
/************************************************************************/


UBOOL UavaUIEditBoxMisc::ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;

	FSubscribedInputEventParameters NewEventParms = EventParms;

	if( EventParms.InputAliasName == UIKEY_Clicked )
	{
		NewEventParms.InputAliasName = UIKEY_Char;
	}

	if ( NewEventParms.InputAliasName == UIKEY_Char )
	{
		if( NewEventParms.EventType != IE_Released )
		{
			FString KeyCombination;
			//KeyCombination += EventParms.bShiftPressed ? FString(KEY_LeftShift.GetName()) + TEXT(" + ") : TEXT("");
			//KeyCombination += EventParms.bCtrlPressed ? FString(KEY_LeftControl.GetName()) + TEXT(" + ") : TEXT("");
			//KeyCombination += EventParms.bAltPressed ? FString(KEY_LeftAlt.GetName()) + TEXT(" + ") : TEXT("");
			KeyCombination += EventParms.InputKeyName.GetName();
			StringRenderComponent->SetValue(KeyCombination);

			ActivateEventByClass( EventParms.PlayerIndex, UUIEvent_OnChar::StaticClass(), this );
			bResult = TRUE;
		}
		else
		{
			const static FName SystemKeyNames[] = { KEY_LeftAlt, KEY_LeftControl, KEY_LeftShift, KEY_RightControl, KEY_RightAlt, KEY_RightShift };

			UBOOL bSystemKeyFound = FALSE;
			for( INT i = 0 ; i < ARRAY_COUNT(SystemKeyNames) ; i++ )
			{
				if( SystemKeyNames[i] == EventParms.InputKeyName )
				{
					bSystemKeyFound = TRUE;
					break;
				}
			}

			if( bSystemKeyFound )
			{
				StringRenderComponent->SetValue( EventParms.InputKeyName.GetName() );
				ActivateEventByClass( EventParms.PlayerIndex, UUIEvent_OnChar::StaticClass(), this );
				bResult = TRUE;
			}
		}
	}
	// 기존에 있던 UIEditBoxMisc에 Left가 재설정이 안된다. 기존에 있던 UIKEY_MoveCursorLeft를 그대로 이용.
	else if ( NewEventParms.InputAliasName == UIKEY_MoveCursorLeft )
	{
		StringRenderComponent->SetValue( EventParms.InputKeyName.GetName() );
		ActivateEventByClass( EventParms.PlayerIndex, UUIEvent_OnChar::StaticClass(), this );
		bResult = TRUE;
	}

	return bResult;
}

UBOOL UavaUIEditBoxMisc::ProcessInputChar( INT PlayerIndex, const FInputCompositionStringData& CompStrData)
{
	return TRUE;
}


void UavaUIEditBoxMisc::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("InitialValue") )
			{
				if ( StringRenderComponent != NULL && DataSource.MarkupString.Len() == 0 )
				{
					SetValue(InitialValue);
					if (StringRenderComponent->AutoSizeParameters[UIORIENT_Horizontal].bAutoSizeEnabled
						||	StringRenderComponent->AutoSizeParameters[UIORIENT_Vertical].bAutoSizeEnabled
						||	(StringRenderComponent->WrapMode != CLIP_Normal && StringRenderComponent->WrapMode != CLIP_None) )
					{
						RefreshPosition();
					}
				}
			}
		}
	}
}

/************************************************************************/
/* avaUIOutfitter                                                       */
/************************************************************************/

/* === UUIScreenObject interface === */
/**
* Perform all initialization for this widget. Called on all widgets when a scene is opened,
* once the scene has been completely initialized.
* For widgets added at runtime, called after the widget has been inserted into its parent's
* list of children.
*
* @param	inOwnerScene	the scene to add this widget to.
* @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
*							is being added to the scene's list of children.
*/
void UavaUIOutfitter::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	Super::Initialize( inOwnerScene, inOwner );
}

/* === UUIObject interface === */

/**
* Adds the specified face to the DockingStack for the specified widget
*
* @param	DockingStack	the docking stack to add this docking node to.  Generally the scene's DockingStack.
* @param	Face			the face that should be added
*
* @return	TRUE if a docking node was added to the scene's DockingStack for the specified face, or if a docking node already
*			existed in the stack for the specified face of this widget.
*/
UBOOL UavaUIOutfitter::AddDockingNode( TArray<struct FUIDockingNode>& DockingStack, EUIWidgetFace Face )
{
	return Super::AddDockingNode( DockingStack, Face );
}

/**
* Evalutes the Position value for the specified face into an actual pixel value.  Should only be
* called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
*
* @param	Face	the face that should be resolved
*/
void UavaUIOutfitter::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition( Face );

	UBOOL bValidAll = TRUE;
	for( INT Face = 0 ; Face < UIFACE_MAX ; Face++)
		bValidAll = bValidAll && Position.IsPositionCurrent( Face );

	if( bValidAll )
		CalculateAllPosition();
}

/**
* Marks the Position for any faces dependent on the specified face, in this widget or its children,
* as out of sync with the corresponding RenderBounds.
*
* @param	Face	the face to modify; value must be one of the EUIWidgetFace values.
*/
void UavaUIOutfitter::InvalidatePositionDependencies( BYTE Face )
{
	Super::InvalidatePositionDependencies( Face );
}

/**
* Render this widget.
*
* @param	Canvas	the FCanvas to use for rendering this widget
*/
void UavaUIOutfitter::Render_Widget( FCanvas* Canvas )
{
	Super::Render_Widget( Canvas );

	if( GIsEditor && !GIsGame )
	{
		for( INT ElementIndex = 0 ; ElementIndex < OutfitElements.Num() ; ElementIndex++ )
		{
			FUIOutfitElement& Element = OutfitElements(ElementIndex);
			for( INT CellIndex = 0 ; CellIndex < Element.Cells.Num() ; CellIndex++ )
			{
				FUIOutfitCell& Cell = Element.Cells(CellIndex);
				UBOOL bContain = FALSE;

				if( Cell.bCollapse )
				{
					for( INT AttachIndex = 0 ; AttachIndex < Cell.AttachedUIObject.Num() ; AttachIndex++ )
					{
						UUIScreenObject* ScreenObject = Cell.AttachedUIObject(AttachIndex);
						if( ScreenObject == NULL )
							continue;
						if
							(	!(
							ScreenObject->IsHidden() || 
							(ScreenObject->GetBounds(UIORIENT_Horizontal) < KINDA_SMALL_NUMBER && 
							ScreenObject->GetBounds(UIORIENT_Vertical) < KINDA_SMALL_NUMBER)	
							) 
							)
						{
							bContain = TRUE;
							break;
						}
					}
				}
				
				if( !Cell.bCollapse )
					DrawBox2D(Canvas, FVector2D(Cell.Position[UIFACE_Left], Cell.Position[UIFACE_Top]), FVector2D(Cell.Position[UIFACE_Right], Cell.Position[UIFACE_Bottom]) , FLinearColor::White * 0.5f);
				else if( bContain )
					DrawBox2D(Canvas, FVector2D(Cell.Position[UIFACE_Left], Cell.Position[UIFACE_Top]), FVector2D(Cell.Position[UIFACE_Right], Cell.Position[UIFACE_Bottom]) , FLinearColor::White);
				else
					DrawBox2D(Canvas, FVector2D(Cell.Position[UIFACE_Left], Cell.Position[UIFACE_Top]), FVector2D(Cell.Position[UIFACE_Right], Cell.Position[UIFACE_Bottom]) , FLinearColor::Black);
			}
		}
	}
}

/**
* Called when a property is modified that could potentially affect the widget's position onscreen.
*/
void UavaUIOutfitter::RefreshPosition()
{
	Super::RefreshPosition();
	RequestUpdateOutfit( TRUE, TRUE );
}

/**
* Changes this widget's position to the specified value.  This version changes the default value for the bClampValues parameter to TRUE
*
* @param	LeftFace		the value (in pixels or percentage) to set the left face to
* @param	TopFace			the value (in pixels or percentage) to set the top face to
* @param	RightFace		the value (in pixels or percentage) to set the right face to
* @param	BottomFace		the value (in pixels or percentage) to set the bottom face to
* @param	InputType		indicates the format of the input value.  All values will be evaluated as this type.
*								EVALPOS_None:
*									NewValue will be considered to be in whichever format is configured as the ScaleType for the specified face
*								EVALPOS_PercentageOwner:
*								EVALPOS_PercentageScene:
*								EVALPOS_PercentageViewport:
*									Indicates that NewValue is a value between 0.0 and 1.0, which represents the percentage of the corresponding
*									base's actual size.
*								EVALPOS_PixelOwner
*								EVALPOS_PixelScene
*								EVALPOS_PixelViewport
*									Indicates that NewValue is an actual pixel value, relative to the corresponding base.
* @param	bZeroOrigin		FALSE indicates that the value specified includes the origin offset of the viewport.
* @param	bClampValues	if TRUE, clamps the values of RightFace and BottomFace so that they cannot be less than the values for LeftFace and TopFace
*/
void UavaUIOutfitter::SetPosition( const FLOAT LeftFace, const FLOAT TopFace, const FLOAT RightFace, const FLOAT BottomFace, EPositionEvalType InputType /*=EVALPOS_PixelViewport*/, UBOOL bZeroOrigin/*=FALSE*/, UBOOL bClampValues/*=TRUE*/ )
{
	Super::SetPosition( LeftFace, TopFace, RightFace, BottomFace, InputType, bZeroOrigin, bClampValues );
}

/* === UObject interface === */
/**
* Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
*/
void UavaUIOutfitter::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);
}

/**
* Called when a property value has been changed in the editor.
*/
void UavaUIOutfitter::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UBOOL bHandled = FALSE;
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if( PropertyName == TEXT("OutfitElements") )
			{
				//RequestUpdateOutfit( TRUE, TRUE );
				CalculateAllPosition();
			}
		}
	}

	Super::PostEditChange( PropertyThatChanged );
}

/**
* Called after this object has been completely de-serialized.  This version migrates the PrimaryStyle for this label over to the label's component.
*/
void UavaUIOutfitter::PostLoad()
{
	Super::PostLoad();
}


// 지금은 쓰지 않는다
// 밑에 CaculateAllPosition할때 조건에 따라서 각각처리하기 위한것인데 
void UavaUIOutfitter::RequestUpdateOutfit( UBOOL bRecalcCellPos, UBOOL bReposAttached )
{
	this->bRecalcCellPosition = this->bRecalcCellPosition || bRecalcCellPos;
	this->bReposAttached = this->bReposAttached || bReposAttached;
}

// @TODO : 조건별 업데이트를 하도록 고치려고 한다.
void UavaUIOutfitter::CalculateAllPosition()
{
	RecalcCellPosition();
	bRecalcCellPosition = FALSE;

	if( GIsEditor && !GIsGame )
		RetainAttached();
	RepositionAttached();
	bReposAttached = FALSE;
}

void UavaUIOutfitter::RecalcCellPosition()
{
	FLOAT CurrX = RenderBounds[UIFACE_Left];
	FLOAT CurrY = RenderBounds[UIFACE_Top];
	FLOAT BoxXL = 0.f;
	FLOAT BoxYL = 0.f;

	for( INT ElementIndex = 0 ; ElementIndex < OutfitElements.Num() ; ElementIndex++ )
	{
		FLOAT MaxYL = 0.f;
		FUIOutfitElement& Element = OutfitElements(ElementIndex);
		for( INT CellIndex = 0 ; CellIndex < Element.Cells.Num() ; CellIndex++ )
		{
			FUIOutfitCell& Cell = Element.Cells(CellIndex);
			UBOOL bSkipThisCell = FALSE;

			if( (GIsGame || !GIsEditor) && Cell.bCollapse )
			{
				UBOOL bContain = FALSE;
				for( INT AttachIndex = 0 ; AttachIndex < Cell.AttachedUIObject.Num() ; AttachIndex++ )
				{
					UUIScreenObject* ScreenObject = Cell.AttachedUIObject(AttachIndex);
					if( ScreenObject == NULL )
						continue;
					
					if( !(ScreenObject->IsHidden() || (ScreenObject->GetBounds(UIORIENT_Horizontal,EVALPOS_PixelViewport) < KINDA_SMALL_NUMBER && ScreenObject->GetBounds(UIORIENT_Vertical,EVALPOS_PixelViewport) < KINDA_SMALL_NUMBER )))
					{
						bContain = TRUE;
						break;
					}
				}
				bSkipThisCell = !bContain;
			}

			FLOAT CurrXL = bSkipThisCell ? 0.f : Cell.Extent[UIORIENT_Horizontal];
			FLOAT CurrYL = bSkipThisCell ? 0.f : Cell.Extent[UIORIENT_Vertical];
			MaxYL = Max(MaxYL, CurrYL);

			Cell.Position[UIFACE_Left] = CurrX;
			Cell.Position[UIFACE_Top] = CurrY;
			Cell.Position[UIFACE_Right] = CurrX + CurrXL;
			Cell.Position[UIFACE_Bottom] = CurrY + CurrYL;
			CurrX += CurrXL;
		}
		BoxXL = Max( CurrX - RenderBounds[UIFACE_Left] , BoxXL);
		CurrX = RenderBounds[UIFACE_Left];
		CurrY += MaxYL;
		BoxYL += MaxYL;
	}

	if( bFitCollapsed )
	{
		Position.SetPositionValue(this, BoxXL, UIFACE_Right, EVALPOS_PixelOwner, FALSE);
		Position.SetPositionValue(this, BoxYL, UIFACE_Bottom, EVALPOS_PixelOwner, FALSE);
		RenderBounds[UIFACE_Right] = RenderBounds[UIFACE_Left] + BoxXL;
		RenderBounds[UIFACE_Bottom] = RenderBounds[UIFACE_Top] + BoxYL;
		Position.ValidatePosition(UIFACE_Right);
		Position.ValidatePosition(UIFACE_Bottom);

		DockTargets.bResolved[UIFACE_Right] = TRUE;
		DockTargets.bResolved[UIFACE_Bottom] = TRUE;
	}
}

void UavaUIOutfitter::RetainAttached()
{
	typedef ava::pair<FVector2D, UUIScreenObject*> PointObjectPairType;
	TArray< PointObjectPairType > PointObjectPairs;

	TArray<UUIObject*> Children;
	GetChildren(Children, FALSE);

	for( INT ChildIndex = Children.Num() - 1 ; ChildIndex >= 0 ; ChildIndex-- )
	{
		UUIScreenObject* ScreenObject = Cast<UUIScreenObject>(Children(ChildIndex));
		if( ScreenObject == NULL )
			continue;

		FVector2D Point = FVector2D( (ScreenObject->GetPosition(UIFACE_Left,EVALPOS_PixelViewport) + ScreenObject->GetPosition(UIFACE_Right,EVALPOS_PixelViewport)) * 0.5f, 
										(ScreenObject->GetPosition(UIFACE_Top,EVALPOS_PixelViewport) + ScreenObject->GetPosition(UIFACE_Bottom,EVALPOS_PixelViewport)) * 0.5f );
		PointObjectPairs.AddItem( PointObjectPairType(Point, ScreenObject) );
	}		

	for( INT ElementIndex = 0 ; ElementIndex < OutfitElements.Num() ; ElementIndex++ )
	{
		FUIOutfitElement& Element = OutfitElements(ElementIndex);
		for( INT CellIndex = 0 ; CellIndex < Element.Cells.Num() ; CellIndex++ )
		{
			FUIOutfitCell& Cell = Element.Cells(CellIndex);
			for( INT AttachIndex = 0 ; AttachIndex < Cell.AttachedUIObject.Num() ; AttachIndex++ )
				eventAddChildPositionChangeNotify( Cell.AttachedUIObject( AttachIndex ) );
			
			Cell.AttachedUIObject.Reset();

			for( INT PairIndex = PointObjectPairs.Num() - 1 ; PairIndex >= 0 ; PairIndex-- )
			{
				PointObjectPairType& CurrPair = PointObjectPairs(PairIndex);
				if( Cell.Position[UIFACE_Left] <= CurrPair.first.X && CurrPair.first.X < Cell.Position[UIFACE_Right] &&
					Cell.Position[UIFACE_Top] <= CurrPair.first.Y && CurrPair.first.Y < Cell.Position[UIFACE_Bottom] )
				{
					Cell.AttachedUIObject.AddItem( CurrPair.second );
					eventRemoveChildPositionChangeNotify( CurrPair.second );
					PointObjectPairs.Remove(PairIndex);
				}
			}
		}
	}
}

void UavaUIOutfitter::RepositionAttached()
{
	for( INT ElementIndex = 0 ; ElementIndex < OutfitElements.Num() ; ElementIndex++ )
	{
		FUIOutfitElement& Element = OutfitElements(ElementIndex);
		for( INT CellIndex = 0 ; CellIndex < Element.Cells.Num() ; CellIndex++ )
		{
			FUIOutfitCell& Cell = Element.Cells(CellIndex);
			for( INT AttachedIndex = 0 ; AttachedIndex < Cell.AttachedUIObject.Num() ; AttachedIndex++ )
			{
				UUIScreenObject* ScreenObject = Cell.AttachedUIObject(AttachedIndex);
				if( ScreenObject == NULL )
					continue;

				FLOAT CellExtent[UIORIENT_MAX] = { Cell.Position[UIFACE_Right] - Cell.Position[UIFACE_Left],
													Cell.Position[UIFACE_Bottom] - Cell.Position[UIFACE_Top] };
				FLOAT NewPos[UIFACE_MAX] = { ScreenObject->GetPosition(UIFACE_Left, EVALPOS_PixelViewport), 
											ScreenObject->GetPosition(UIFACE_Top, EVALPOS_PixelViewport),
											ScreenObject->GetPosition(UIFACE_Right, EVALPOS_PixelViewport), 
											ScreenObject->GetPosition(UIFACE_Bottom, EVALPOS_PixelViewport) };
				FLOAT Extent[UIORIENT_MAX] = { NewPos[UIFACE_Right] - NewPos[UIFACE_Left], NewPos[UIFACE_Bottom] - NewPos[UIFACE_Top] };

				switch( Cell.Alignment[UIORIENT_Horizontal] )
				{
				case UIALIGN_Left:		NewPos[UIFACE_Left] = Cell.Position[UIFACE_Left];
										NewPos[UIFACE_Right] = NewPos[UIFACE_Left] + Extent[UIORIENT_Horizontal];
										break;
				case UIALIGN_Center:	NewPos[UIFACE_Left] = Cell.Position[UIFACE_Left] + ( CellExtent[UIORIENT_Horizontal] - Extent[UIORIENT_Horizontal] ) * 0.5f;
										NewPos[UIFACE_Right] = NewPos[UIFACE_Left] + Extent[UIORIENT_Horizontal];
										break;

				case UIALIGN_Right:		NewPos[UIFACE_Right] = Cell.Position[UIFACE_Right];
										NewPos[UIFACE_Left] = Cell.Position[UIFACE_Right] - Extent[UIORIENT_Horizontal];
										break;

				case UIALIGN_Default:	break;
				}

				switch( Cell.Alignment[UIORIENT_Vertical] )
				{
				case UIALIGN_Left:		NewPos[UIFACE_Top] = Cell.Position[UIFACE_Top];
										NewPos[UIFACE_Bottom] = NewPos[UIFACE_Top] + Extent[UIORIENT_Vertical];
										break;
				case UIALIGN_Center:	NewPos[UIFACE_Top] = Cell.Position[UIFACE_Top] + ( CellExtent[UIORIENT_Vertical] - Extent[UIORIENT_Vertical] ) * 0.5f; 
										NewPos[UIFACE_Bottom] = NewPos[UIFACE_Top] + Extent[UIORIENT_Vertical];
										break;

				case UIALIGN_Right:		NewPos[UIFACE_Bottom] = Cell.Position[UIFACE_Bottom];
										NewPos[UIFACE_Top] = Cell.Position[UIFACE_Bottom] - Extent[UIORIENT_Vertical];
										break;

				case UIALIGN_Default:	break;
				}

				//ScreenObject->Position.SetPositionValue( ScreenObject, NewPos[UIFACE_Left], UIFACE_Left,EVALPOS_PixelViewport,FALSE );
				//ScreenObject->Position.SetPositionValue( ScreenObject, NewPos[UIFACE_Right], UIFACE_Right,EVALPOS_PixelViewport,FALSE );
				//ScreenObject->Position.SetPositionValue( ScreenObject, NewPos[UIFACE_Top], UIFACE_Top,EVALPOS_PixelViewport,FALSE );
				//ScreenObject->Position.SetPositionValue( ScreenObject, NewPos[UIFACE_Bottom], UIFACE_Bottom,EVALPOS_PixelViewport,FALSE );

				ScreenObject->SetPosition( NewPos[UIFACE_Left], NewPos[UIFACE_Top], NewPos[UIFACE_Right], NewPos[UIFACE_Bottom] );
			}
		}
	}

}

void UavaUIOutfitter::OnAttachRepositioned( UUIScreenObject* Sender )
{
}

//////////////////////////////////////////////////////////////////////////
//	UIDataStoreSubscriber Interface
void UavaUIOutfitter::SetDataStoreBinding(const FString& MarkupText,INT BindingIndex/*=-1*/){}
FString UavaUIOutfitter::GetDataStoreBinding(INT BindingIndex/*=-1*/) const { return (TEXT("N/A")); }
UBOOL UavaUIOutfitter::RefreshSubscriberValue(INT BindingIndex/*=-1*/){ CalculateAllPosition(); return TRUE; }
void UavaUIOutfitter::GetBoundDataStores(TArray<class UUIDataStore*>& out_BoundDataStores) {  }
void UavaUIOutfitter::ClearBoundDataStores() {  }

/************************************************************************/
/* avaUIMouseRegionTracker                                              */
/************************************************************************/

void UavaUIMouseTracker::Render_Widget( FCanvas* Canvas )
{
	FLOAT CurrX = RenderBounds[UIFACE_Left];
	FLOAT CurrY = RenderBounds[UIFACE_Top];
	FLOAT BoxXL = 0.f;
	FLOAT BoxYL = 0.f;

	FUIMouseRegionCell* SelectedCell = NULL;
	INT SelectedElementIndex = INDEX_NONE;
	INT SelectedCellIndex = INDEX_NONE;

	UGameUISceneClient* SceneClient = GetSceneClient();
	FVector2D MousePosition = SceneClient ? SceneClient->MousePosition : FVector2D();
	UBOOL bContainsPoint = ContainsPoint( MousePosition );
	
	for( INT ElementIndex = 0 ; ElementIndex < MouseRegionElements.Num() ; ElementIndex++ )
	{
		FLOAT MaxYL = 0.f;
		FUIMouseRegionElement& Element = MouseRegionElements(ElementIndex);
		for( INT CellIndex = 0 ; CellIndex < Element.Cells.Num() ; CellIndex++ )
		{
			FUIMouseRegionCell& Cell = Element.Cells(CellIndex);

			// EVALPOS_Viewport로 계산하면 avaUIMouseTracker를 기준으로 Render바운드를 계산하므로
			// EVALPOS_None로 계산해야한다
			FLOAT CellBounds[UIFACE_MAX] = { Cell.Bounds[UIFACE_Left].GetValue(this, EVALPOS_PixelViewport),
											Cell.Bounds[UIFACE_Top].GetValue(this, EVALPOS_PixelViewport),
											Cell.Bounds[UIFACE_Right].GetValue(this, EVALPOS_PixelViewport),
											Cell.Bounds[UIFACE_Bottom].GetValue(this, EVALPOS_PixelViewport), };

			if( GIsEditor && !GIsGame )
			{
				DrawString(Canvas, CellBounds[UIFACE_Left], CellBounds[UIFACE_Top], *Cell.Alias, GEngine->GetSmallFont(), FLinearColor(1.f,1.f,0.f));
				DrawBox2D( Canvas, FVector2D(CellBounds[UIFACE_Left], CellBounds[UIFACE_Top]), FVector2D( CellBounds[UIFACE_Right], CellBounds[UIFACE_Bottom] ), FLinearColor(1.f, 1.f, 0.f) );
			}

			if( bContainsPoint )
			{
				if( CellBounds[UIFACE_Left] <= MousePosition.X && MousePosition.X < CellBounds[UIFACE_Right] &&
					CellBounds[UIFACE_Top] <= MousePosition.Y && MousePosition.Y < CellBounds[UIFACE_Bottom])
				{
					SelectedElementIndex = ElementIndex;
					SelectedCellIndex = CellIndex;
				}
			}
		}
	}

	// @TODO : GameSceneClient:Tick() 으로 이전 ( 드로잉과정이 아님 )
	if( SelectedElementIndex != LastElementIndex || 
		SelectedCellIndex != LastCellIndex )
	{
		LastCellIndex = SelectedCellIndex;
		LastElementIndex = SelectedElementIndex;

		FString Alias;
		if( MouseRegionElements.IsValidIndex(LastElementIndex) && 
			MouseRegionElements(LastElementIndex).Cells.IsValidIndex(LastCellIndex) )
		{
			Alias = MouseRegionElements(LastElementIndex).Cells(LastCellIndex).Alias;
		}

		TArray<UUIEvent*> EventsToActivate;
		ActivateEventByClass( GetBestPlayerIndex(), UavaUIEvent_OnMouseTrackerCellChanged::StaticClass(), this, FALSE, NULL, &EventsToActivate );
		for( INT EventIndex = 0 ; EventIndex < EventsToActivate.Num() ; EventIndex++ )
		{
			if(EventsToActivate(EventIndex) == NULL)
				continue;

			TArray<FString*> StrVars;
			EventsToActivate(EventIndex)->GetStringVars(StrVars);
			for( INT VarIndex = 0 ; VarIndex < StrVars.Num() ; VarIndex++ )
				if( StrVars(VarIndex) != NULL )
					*StrVars(VarIndex) = Alias;

			TArray<INT*> IntVars;
			EventsToActivate(EventIndex)->GetIntVars(IntVars, TEXT("Elem Index"));
			for( INT VarIndex = 0 ; VarIndex < IntVars.Num() ; VarIndex++ )
				if( IntVars(VarIndex) != NULL )
					*IntVars(VarIndex) = SelectedElementIndex;

			IntVars.Empty();
			EventsToActivate(EventIndex)->GetIntVars(IntVars, TEXT("Cell Index"));
			for( INT VarIndex = 0 ; VarIndex < IntVars.Num() ; VarIndex++ )
				if( IntVars(VarIndex) != NULL )
					*IntVars(VarIndex) = SelectedCellIndex;
		}
	}
}

void UavaUIMouseTracker::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		TDoubleLinkedList<UProperty*>::TDoubleLinkedListNode* MemberNode = PropertyThatChanged.GetActiveMemberNode();
		UProperty* MemberProperty = MemberNode->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("MouseRegionElements") )
			{
				RecalculateCellPosition();
			}
			else if( PropertyName == TEXT("ElementPadding") )
			{
				RecalculateCellPosition();
			}
		}
	}

	Super::PostEditChange( PropertyThatChanged );
}

void UavaUIMouseTracker::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition( Face );

	UBOOL bAllCurrentPosition = TRUE;
	for( INT FaceIndex = 0 ; FaceIndex < UIFACE_MAX ; FaceIndex++ )
	{
		if( !Position.IsPositionCurrent(FaceIndex) )
			bAllCurrentPosition = FALSE;
	}

	if( bAllCurrentPosition )
		RecalculateCellPosition();
}

/** Cell Position을 새로 계산. ResolveFacePosition과 같이 UIObject의 위치, 크기, 도킹 등이 변경되었을때 불린다 */
void UavaUIMouseTracker::RecalculateCellPosition()
{
	// 마우스 포지션
	UGameUISceneClient* SceneClient = GetSceneClient();
	FVector2D MousePosition = SceneClient ? SceneClient->MousePosition : FVector2D();
	UBOOL bContainsPoint = ContainsPoint( MousePosition );
	FVector CanvasMousePosition = PixelToCanvas(MousePosition);

	// OwnerWidget 포지션 설정
	FLOAT CurrX = GetPosition(UIFACE_Left, EVALPOS_PixelViewport);
	FLOAT CurrY = GetPosition(UIFACE_Top, EVALPOS_PixelViewport);
	FLOAT BoxXL = 0.f;
	FLOAT BoxYL = 0.f;
	FLOAT OwnerLeft = CurrX;


	INT SelectedElementIndex = INDEX_NONE;
	INT SelectedCellIndex = INDEX_NONE;

	for( INT ElementIndex = 0 ; ElementIndex < MouseRegionElements.Num() ; ElementIndex++ )
	{
		FLOAT MaxYL = 0.f;
		FUIMouseRegionElement& Element = MouseRegionElements(ElementIndex);
		for( INT CellIndex = 0 ; CellIndex < Element.Cells.Num() ; CellIndex++ )
		{
			FUIMouseRegionCell& Cell = Element.Cells(CellIndex);

			FLOAT CurrXL = Cell.CellExtent[UIORIENT_Horizontal].GetValue(this, EVALPOS_PixelViewport);
			FLOAT CurrYL = Cell.CellExtent[UIORIENT_Vertical].GetValue(this, EVALPOS_PixelViewport);
			MaxYL = Max(MaxYL, CurrYL);

			Cell.Bounds[UIFACE_Left].SetValue(this, CurrX, EVALPOS_PixelViewport);
			Cell.Bounds[UIFACE_Top].SetValue(this, CurrY, EVALPOS_PixelViewport);
			Cell.Bounds[UIFACE_Right].SetValue(this, CurrX + CurrXL, EVALPOS_PixelViewport);
			Cell.Bounds[UIFACE_Bottom].SetValue(this, CurrY + CurrYL, EVALPOS_PixelViewport);

			CurrX += CurrXL;
		}
		BoxXL = Max( CurrX - OwnerLeft , BoxXL);
		CurrX = OwnerLeft;
		CurrY += (MaxYL + ElementPadding.GetValue( this, EVALPOS_PixelViewport ));
		BoxYL += MaxYL;
	}
}

void UavaUIMouseTracker::PostLoad()
{
	Super::PostLoad();

	// 예전 MouseTracker를 이전하기 위한 작업 (이전 완료)
	//for( INT ElementIndex = 0 ; ElementIndex < MouseRegionElements.Num() ; ElementIndex++ )
	//{
	//	FLOAT MaxYL = 0.f;
	//	FUIMouseRegionElement& Element = MouseRegionElements(ElementIndex);
	//	for( INT CellIndex = 0 ; CellIndex < Element.Cells.Num() ; CellIndex++ )
	//	{
	//		FUIMouseRegionCell& Cell = Element.Cells(CellIndex);

	//		FLOAT CurrXL = Cell.Extent[UIORIENT_Horizontal];
	//		FLOAT CurrYL = Cell.Extent[UIORIENT_Vertical];

	//		if( CurrXL != 0.f )
	//		{
	//			Cell.CellExtent[UIORIENT_Horizontal].SetValue(this, CurrXL);
	//		}
	//		if( CurrYL != 0.f )
	//		{
	//			Cell.CellExtent[UIORIENT_Vertical].SetValue(this, CurrYL);
	//		}
	//	}
	//}
}

/************************************************************************/
/* avaUIChart                                                           */
/************************************************************************/

void UavaUIChart::Render_Widget( FCanvas* Canvas )
{
	if( CellDataComponent == NULL )
		return;

	FRenderParameters RenderParms( RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top], RenderBounds[UIFACE_Right], RenderBounds[UIFACE_Bottom]);
	FLOAT DefaultHeight = GetRowHeight();
	FLOAT DefaultWidth = GetColumnWidth();

	// 항목표시줄을 그림
	FRenderParameters ColumnHeaderRenderParms = RenderParms;
	switch( ChartShape )
	{
	case CHARTSHAPE_CrouchingStick:
		ColumnHeaderRenderParms.DrawXL = ColumnHeaderRenderParms.DrawX + DefaultWidth;	break;
	case CHARTSHAPE_StandingStick:
		ColumnHeaderRenderParms.DrawY = ColumnHeaderRenderParms.DrawYL - DefaultHeight;	break;
	case CHARTSHAPE_SimplePolygon:
		break;
	default:
		check(FALSE);	break;
	}
	
	Render_TemplateCells( Canvas, CellDataComponent->ElementSchema.Cells, ColumnHeaderRenderParms );

	// 항목표시줄을 범위에서 제외하고 나머지를 그림
	FRenderParameters CellRenderParms = RenderParms;
	switch( ChartShape )
	{
	case CHARTSHAPE_CrouchingStick:		CellRenderParms.DrawX += DefaultWidth;	break;
	case CHARTSHAPE_StandingStick:		CellRenderParms.DrawYL -= DefaultHeight;	break;
	case CHARTSHAPE_SimplePolygon:		break;
	default:	check(FALSE);	break;
	}

	for ( INT ListIndex = TopIndex; ListIndex < TopIndex + MaxVisibleItems; ListIndex++ )
	{
		if( !CellDataComponent->ListItems.IsValidIndex( ListIndex ) )
			continue;

		Render_Cells( Canvas, CellDataComponent->ListItems(ListIndex).Cells, CellRenderParms );
	}
}

void UavaUIChart::Render_TemplateCells( FCanvas* Canvas, const TArray<struct FUIListElementCellTemplate>& Cells, struct FRenderParameters& CellParameters )
{
	const FLOAT ClipX = CellParameters.DrawXL;
	const FLOAT ClipY = CellParameters.DrawYL;
	const FLOAT CellPadding = this->CellPadding.GetValue(Owner, EVALPOS_PixelViewport);

	FRenderParameters DrawParameters = CellParameters;

	// SimplePolygon타입은 배경이 먼저 (배경과 항목표시가 겹치기 때문에 먼저 배경을 그림)
	//if( ChartShape == CHARTSHAPE_SimplePolygon )
	//{
	//	EUIListElementState CellState = static_cast<EUIListElementState>(Cells(0).CellState);
	//	FRenderParameters BackgroundParameters(DrawParameters.DrawX, DrawParameters.DrawY, DrawParameters.DrawXL - DrawParameters.DrawX, DrawParameters.DrawYL - DrawParameters.DrawY);
	//	if( CellDataComponent->ListItemOverlay[CellState] != NULL )
	//		CellDataComponent->ListItemOverlay[CellState]->Render_Texture( Canvas, BackgroundParameters );
	//}

	// 우선 해당 RenderParameters에 대해 항목표시줄을 그려준다 
	for( INT CellIdx = 0 ; CellIdx < Cells.Num() ; CellIdx++ )
	{
		const FUIListElementCellTemplate& CellTemplate = Cells(CellIdx);

		if( ! CellTemplate.ValueString )
			continue;

		FLOAT CellHeight = GetRowHeight();
		FLOAT CellWidth = GetColumnWidth();

		FLOAT NextX = DrawParameters.DrawX;
		FLOAT NextY = DrawParameters.DrawY;
		if( ChartShape == CHARTSHAPE_CrouchingStick )
		{
			DrawParameters.DrawXL = CellWidth;
			DrawParameters.DrawYL = CellHeight;
			NextY = DrawParameters.DrawY + CellHeight;
		}
		else if( ChartShape == CHARTSHAPE_StandingStick )
		{
			DrawParameters.DrawXL = CellWidth;
			DrawParameters.DrawYL = CellHeight;
			NextX = DrawParameters.DrawX + CellWidth;
		}
		else if( ChartShape == CHARTSHAPE_SimplePolygon )
		{
			DrawParameters = CellParameters;
			FLOAT AngRad = ((2*PI)/Cells.Num()) * CellIdx;
			FLOAT EllipseDist[] = { (DrawParameters.DrawXL - DrawParameters.DrawX) * 0.5f, (DrawParameters.DrawYL - DrawParameters.DrawY) * 0.5f };
			FLOAT CenterPos[] = { (DrawParameters.DrawX + DrawParameters.DrawXL) * 0.5f, (DrawParameters.DrawYL+DrawParameters.DrawY) * 0.5f };

			FLOAT Radius, Multiple;
			UBOOL bNarrowEllipse = EllipseDist[0] < EllipseDist[1];
			if( bNarrowEllipse )
			{
				Radius = EllipseDist[0];
				Multiple = EllipseDist[1]/EllipseDist[0];
			}
			else
			{
				Radius = EllipseDist[1];
				Multiple = EllipseDist[0]/EllipseDist[1];
			}

			DrawParameters.DrawX = DrawParameters.DrawY = 0;
			DrawParameters.DrawX = (Radius * appSin( AngRad )) * (bNarrowEllipse ? 1.f : Multiple);
			DrawParameters.DrawY = -(Radius * appCos( AngRad )) * (bNarrowEllipse ? Multiple : 1.f);
			DrawParameters.DrawX += CenterPos[0];
			DrawParameters.DrawY += CenterPos[1];

			DrawParameters.DrawX -= (CellWidth * 0.5f);
			DrawParameters.DrawY -= (CellHeight * 0.5f);
			DrawParameters.DrawXL = CellWidth;
			DrawParameters.DrawYL = CellHeight;
		}

		if( ShouldRenderDataBindings() )
		{
			// render a box which displays the outline for this cell
			FVector2D StartLoc(DrawParameters.DrawX, DrawParameters.DrawY);
			FVector2D EndLoc(DrawParameters.DrawX + DrawParameters.DrawXL, DrawParameters.DrawY + DrawParameters.DrawYL);
			DrawBox2D(Canvas, StartLoc, EndLoc, FColor(0,255,255));	// draw an cyan box to show the bounds of this label
		}

		if( CellTemplate.Background != NULL )
			CellTemplate.Background->Render_Texture( Canvas, DrawParameters );

		FRenderParameters StringRenderParameters = DrawParameters;
		StringRenderParameters.DrawX += CellPadding * 0.5f;
		StringRenderParameters.DrawY += CellPadding * 0.5f;
		StringRenderParameters.DrawXL -= CellPadding;
		StringRenderParameters.DrawYL -= CellPadding;
		for( INT i = 0 ; i < UIORIENT_MAX ; i++ )
			StringRenderParameters.TextAlignment[i] = CellTemplate.Alignment[i];

		CellTemplate.ValueString->Render_String( Canvas, StringRenderParameters );

		if( ChartShape != CHARTSHAPE_SimplePolygon )
		{
			DrawParameters.DrawX = NextX;
			DrawParameters.DrawY = NextY;
		}
	}
}

void UavaUIChart::Render_Cells( FCanvas* Canvas, const TArray<struct FUIListElementCell>& Cells, struct FRenderParameters& CellParameters )
{

	const FLOAT ClipX = CellParameters.DrawXL;
	const FLOAT ClipY = CellParameters.DrawYL;
	const FLOAT CellPadding = this->CellPadding.GetValue(Owner, EVALPOS_PixelViewport);
	const FLOAT CellSpacing = this->CellSpacing.GetValue(Owner, EVALPOS_PixelViewport);

	FRenderParameters DrawParameters = CellParameters;

	FLOAT ListItemCount = CellDataComponent->ListItems.Num();
	FLOAT CellWidth = (CellParameters.DrawXL - CellParameters.DrawX) - CellPadding;
	FLOAT CellHeight = (CellParameters.DrawYL - CellParameters.DrawY) - CellPadding;
	FLOAT DefaultHeight = GetRowHeight();
	FLOAT DefaultWidth = GetColumnWidth();

	// Max값을 미리 계산
	FLOAT MaxValue = 0;
	typedef ava::pair<FLOAT,UBOOL> CellValueType;
	TArray<CellValueType> CellValues;
	CellValues.Add( Cells.Num() );
	for( INT CellIdx = 0 ; CellIdx < Cells.Num() ; CellIdx++ )
	{
		const FUIListElementCell& Cell = Cells(CellIdx);
		CellValueType& CellValue = CellValues(CellIdx);
		CellValue.first = Cell.ValueString ? appAtof( *(Cell.ValueString->GetValue()) ) : 0.f;
		CellValue.second = !(Cell.ValueString && (CellValue.first - appTrunc(CellValue.first)) < KINDA_SMALL_NUMBER);
		if( MaxValue < CellValue.first )
			MaxValue = CellValue.first;
	}
	MaxValue = Max( MaxValue, 1.0f );

	// Test
	MaxValue = 20.0f;
	for( INT i = 0 ; i < CellValues.Num() ; i++ )
	{
		CellValues(i).first = (i + 1)* (MaxValue/CellValues.Num());
		CellValues(i).second = FALSE;
	}

	// SimplePolygon 타입일때 각 지점을 미리 계산
	// 미리 계산해두고 각 셀을 그릴때 선(현재점-다음점)을 하나씩 그린다.
	TArray<FVector2D> PointList;
	PointList.Add( Cells.Num() );
	if( ChartShape == CHARTSHAPE_SimplePolygon )
	{
		const FLOAT EllipseDist[] = { (DrawParameters.DrawXL - DrawParameters.DrawX) * 0.5f, (DrawParameters.DrawYL - DrawParameters.DrawY) * 0.5f };
		const FLOAT CenterPos[] = { (DrawParameters.DrawX + DrawParameters.DrawXL) * 0.5f, (DrawParameters.DrawYL+DrawParameters.DrawY) * 0.5f };
		UBOOL bNarrowEllipse = EllipseDist[0] < EllipseDist[1];
		FLOAT Radius = bNarrowEllipse ? EllipseDist[0] : EllipseDist[1];
		FLOAT Multiple = bNarrowEllipse ? EllipseDist[1]/EllipseDist[0] : EllipseDist[0]/EllipseDist[1];

		for ( INT CellIdx = 0 ; CellIdx < Cells.Num() ; CellIdx++ )
		{
			FLOAT AngRad = ((2*PI)/Cells.Num()) * CellIdx;
			FLOAT CurrRadius = Radius * ( CellValues(CellIdx).first / MaxValue );

			FVector2D& DrawPoint = PointList(CellIdx);
			DrawPoint.X = DrawPoint.Y = 0;
			DrawPoint.X = (CurrRadius * appSin( AngRad )) * (bNarrowEllipse ? 1.f : Multiple);
			DrawPoint.Y = -(CurrRadius * appCos( AngRad )) * (bNarrowEllipse ? Multiple : 1.f);
			DrawPoint.X += CenterPos[0];
			DrawPoint.Y += CenterPos[1];
		}
	}

	for( INT CellIdx = 0 ; CellIdx < Cells.Num() ; CellIdx++ )
	{
		const FUIListElementCell& Cell = Cells(CellIdx);

		FLOAT NextX = DrawParameters.DrawX;
		FLOAT NextY = DrawParameters.DrawY;
		if( ChartShape == CHARTSHAPE_CrouchingStick  )
		{
			NextY = DrawParameters.DrawY + DefaultHeight;
			DrawParameters.DrawY += (CellPadding * 0.5f);
			DrawParameters.DrawXL = CellWidth * (CellValues(CellIdx).first / MaxValue);
			DrawParameters.DrawYL = DefaultHeight - CellPadding;
		}
		else if( ChartShape == CHARTSHAPE_StandingStick )
		{
			NextX = DrawParameters.DrawX + DefaultWidth;
			DrawParameters.DrawX += (CellPadding * 0.5f);
			DrawParameters.DrawXL = DefaultWidth - CellPadding;
			DrawParameters.DrawYL = CellHeight * ( CellValues(CellIdx).first / MaxValue );
			DrawParameters.DrawY = CellParameters.DrawYL - DrawParameters.DrawYL;
		}
		else if( ChartShape == CHARTSHAPE_SimplePolygon )
		{
			DrawParameters.DrawXL = CellParameters.DrawXL - CellParameters.DrawX;
			DrawParameters.DrawYL = CellParameters.DrawYL - CellParameters.DrawY;
		}

		if( ShouldRenderDataBindings() )
		{
			// render a box which displays the outline for this cell
			FVector2D StartLoc(DrawParameters.DrawX, DrawParameters.DrawY);
			FVector2D EndLoc(DrawParameters.DrawX + DrawParameters.DrawXL, DrawParameters.DrawY + DrawParameters.DrawYL);
			DrawBox2D(Canvas, StartLoc, EndLoc, FColor(0,255,255));	// draw an cyan box to show the bounds of this label
		}

		if( ChartShape == CHARTSHAPE_SimplePolygon )
		{
			check(PointList.IsValidIndex( CellIdx ));
			FVector2D CurrPt = PointList(CellIdx);
			FVector2D NextPt = (CellIdx < Cells.Num() - 1 ) ? PointList( CellIdx + 1 ) : PointList( 0 );
			DrawLine2D( Canvas, CurrPt, NextPt, FLinearColor::White );
		}
		else
		{
			EUIListElementState CellState = static_cast<EUIListElementState>(Cells(0).CellState);
			if( CellDataComponent->ListItemOverlay[CellState] != NULL )
				CellDataComponent->ListItemOverlay[CellState]->Render_Texture( Canvas, DrawParameters );

			FVector2D StartPos( DrawParameters.DrawX, DrawParameters.DrawY );
			FVector2D EndPos( DrawParameters.DrawX + DrawParameters.DrawXL, DrawParameters.DrawY + DrawParameters.DrawYL );
			DrawBox2D( Canvas, StartPos, EndPos, FLinearColor::White );
			DrawParameters.DrawX = NextX;
			DrawParameters.DrawY = NextY;
		}
	}
}

/**
* Refreshes the data for this list from the data store bound via DataSource.
*
* @param	bResolveDataSource	if TRUE, re-resolves DataSource into DataProvider prior to refilling the list's data
*
* @return	TRUE if the list data was successfully loaded; FALSE if the data source couldn't be resolved or it didn't
*			contain the data indicated by SourceData
*/
UBOOL UavaUIChart::RefreshListData( UBOOL bResolveDataSource/*=FALSE*/ )
{
	UBOOL bResult = Super::RefreshListData( bResolveDataSource );

	if( CellDataComponent != NULL )
	{
		ChartItems.Empty( CellDataComponent->ElementSchema.Cells.Num() );
		for( INT SchemaCellIndex = 0 ; SchemaCellIndex < CellDataComponent->ElementSchema.Cells.Num() ; SchemaCellIndex++ )
		{
			FUIChartItem* NewElement = new(ChartItems,SchemaCellIndex) FUIChartItem(EC_EventParm);
			NewElement->Cells.Empty( CellDataComponent->ListItems.Num() );
			UUIListString* HeaderValueString = CellDataComponent->ElementSchema.Cells(SchemaCellIndex).ValueString;
			NewElement->HeaderText = HeaderValueString ? HeaderValueString->GetValue() : TEXT("");

			NewElement->Cells.Empty(CellDataComponent->ListItems.Num());
			for( INT CellIndex = 0 ; CellIndex < CellDataComponent->ListItems.Num() ; CellIndex++ )
			{
				FUIChartElementCell* NewCell = new(NewElement->Cells, CellIndex) FUIChartElementCell(EC_EventParm);
				UUIListString* ValueString = CellDataComponent->ListItems(CellIndex).Cells(SchemaCellIndex).ValueString;
				NewCell->BaseValue = ValueString ? appAtoi(*ValueString->GetValue()) : 0;
			}
		}

		for( INT SchemaCellIndex = 0 ; SchemaCellIndex < ChartItems.Num() ; SchemaCellIndex++ )
		{
			FUIChartItem& ChartItem = ChartItems(SchemaCellIndex);
			
			for( INT CellIndex = 0 ; CellIndex < ChartItem.Cells.Num() ; CellIndex++ )
			{
				FUIChartElementCell& Cell = ChartItem.Cells(CellIndex);
				debugf(TEXT("Schema = %s, Cell = %d"), *ChartItem.HeaderText, Cell.BaseValue);
			}
		}
	}

	return bResult;
}

/************************************************************************/
/* avaUIFloatingPanel                                                   */
/************************************************************************/

/**
* Called immediately after a child has been added to this screen object.
*
* @param	WidgetOwner		the screen object that the NewChild was added as a child for
* @param	NewChild		the widget that was added
*/
void UavaUIFloatingPanel::NotifyAddedChild( UUIScreenObject* WidgetOwner, UUIObject* NewChild )
{
	check(NewChild && StateClassToDetach);

	// UIState_Active 가 활동중이라면 재운다
	UClass* ClassToDetach = StateClassToDetach;
	INT BestPlayerIndex = GetBestPlayerIndex();
	INT StateIndex = 0;
	if( NewChild->HasActiveStateOfClass( ClassToDetach, BestPlayerIndex, &StateIndex ) )
		NewChild->DeactivateState( NewChild->StateStack(StateIndex), BestPlayerIndex );

	// UIState_Active 가 InactiveStates에 있으면 마우스를 올릴시에 다시 UIState_Active를 가질 것이므로
	// InactiveStates 에서 꺼낸다.
	for( INT StateIndex = NewChild->InactiveStates.Num() - 1 ; StateIndex >= 0 ; StateIndex-- )
	{
		if( NewChild->InactiveStates(StateIndex)->IsA( ClassToDetach ) )
		{
			NewChild->InactiveStates.Remove( StateIndex );
		}
	}

	Super::NotifyAddedChild(WidgetOwner, NewChild);
}

/**
* Called immediately after a child has been removed from this screen object.
*
* @param	WidgetOwner		the screen object that the widget was removed from.
* @param	OldChild		the widget that was removed
* @param	ExclusionSet	used to indicate that multiple widgets are being removed in one batch; useful for preventing references
*							between the widgets being removed from being severed.
*/
void UavaUIFloatingPanel::NotifyRemovedChild( UUIScreenObject* WidgetOwner, UUIObject* OldChild, TArray<UUIObject*>* ExclusionSet/*=NULL*/ )
{
	check(OldChild && StateClassToDetach);

	// InactiveState에 원래 UIState_Active가 있어야한다면 추가해준다
	for( INT StateIndex = 0 ; StateIndex < OldChild->DefaultStates.Num() ; StateIndex++ )
	{
		if( OldChild->DefaultStates(StateIndex)->IsChildOf( StateClassToDetach ) )
		{
			OldChild->AddSupportedState( StateClassToDetach->GetDefaultObject<UUIState>() );
			break;
		}
	}

	Super::NotifyRemovedChild(WidgetOwner, OldChild, ExclusionSet);
}

void UavaUIFloatingPanel::CreateDefaultStates()
{
	for( INT StateIndex = DefaultStates.Num() - 1 ; StateIndex >= 0 ; StateIndex-- )
	{
		if( DefaultStates(StateIndex)->IsChildOf(StateClassToDetach) )
			DefaultStates.Remove(StateIndex);
	}
	Super::CreateDefaultStates();
}
