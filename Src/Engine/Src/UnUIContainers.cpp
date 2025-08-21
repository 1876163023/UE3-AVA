/*=============================================================================
	UnUIContainers.cpp: Implementations for complex UI widget classes
	Copyright 1998-2007 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


/* ==========================================================================================================
	Includes
========================================================================================================== */
#include "EnginePrivate.h"
#include "CanvasScene.h"

#include "EngineUserInterfaceClasses.h"
#include "EngineUIPrivateClasses.h"

#include "EngineSequenceClasses.h"
#include "EngineUISequenceClasses.h"

#include "ScopedObjectStateChange.h"

#include "UnUIKeys.h"


/* ==========================================================================================================
	Definitions
========================================================================================================== */
IMPLEMENT_CLASS(UUIContainer);
	IMPLEMENT_CLASS(UUIPanel);
	IMPLEMENT_CLASS(UUIFrameBox);
	IMPLEMENT_CLASS(UUISafeRegionPanel);
	IMPLEMENT_CLASS(UUIScrollFrame);
	IMPLEMENT_CLASS(UUITabPage);

IMPLEMENT_CLASS(UUITabControl);

#define VALIDATE_COMPONENT(comp) checkfSlow(comp==NULL||comp->GetOuter()==this,TEXT("Invalid #comp for %s: %s"), *GetPathName(), *comp->GetPathName())

/* ==========================================================================================================
	Implementations
========================================================================================================== */

extern void MigrateImageSettings( UUITexture*& Texture, const FTextureCoordinates& Coordinates, const FUIStyleReference& Style, UUIComp_DrawImage* DestinationComponent, const TCHAR* PropertyName );

/* ==========================================================================================================
	UUIContainer
========================================================================================================== */
/**
 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
 *
 * @param	Face	the face that should be resolved
 */
void UUIContainer::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition(Face);

	if ( AutoAlignment != NULL )
	{
		AutoAlignment->ResolveFacePosition(Face);
	}
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIContainer::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIContainer::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);
}


/* ==========================================================================================================
	UIPanel
========================================================================================================== */
/* === UIPanel interface === */
/**
 * Changes the background image for this button, creating the wrapper UITexture if necessary.
 *
 * @param	NewImage		the new surface to use for this UIImage
 * @param	NewCoordinates	the optional coordinates for use with texture atlasing
 */
void UUIPanel::SetBackgroundImage( USurface* NewImage )
{
	if ( BackgroundImageComponent != NULL )
	{
		BackgroundImageComponent->SetImage(NewImage);
	}
}

/* === UIObject interface === */
/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UUIPanel::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	VALIDATE_COMPONENT(BackgroundImageComponent);
	AddStyleSubscriber(BackgroundImageComponent);
}

/* === UUIScreenObject interface === */
/**
 * Render this widget.
 *
 * @param	Canvas	the canvas to use for rendering this widget
 */
void UUIPanel::Render_Widget( FCanvas* Canvas )
{
	if ( BackgroundImageComponent != NULL )
	{
		FRenderParameters Parameters(
			RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top],
			RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left],
			RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top]
		);

		BackgroundImageComponent->RenderComponent(Canvas, Parameters);
	}
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUIPanel::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty && BackgroundImageComponent != NULL )
				{
					// the user either cleared the value of the BackgroundImageComponent (which should never happen since
					// we use the 'noclear' keyword on the property declaration), or is assigning a new value to the BackgroundImageComponent.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(BackgroundImageComponent);
				}
			}
		}
	}
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIPanel::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( BackgroundImageComponent != NULL )
					{
						UUIComp_DrawImage* ComponentTemplate = GetArchetype<UUIPanel>()->BackgroundImageComponent;
						if ( ComponentTemplate != NULL )
						{
							BackgroundImageComponent->StyleResolverTag = ComponentTemplate->StyleResolverTag;
						}
						else
						{
							BackgroundImageComponent->StyleResolverTag = TEXT("Panel Background Style");
						}

						// user created a new background image component - add it to the list of style subscribers
						AddStyleSubscriber(BackgroundImageComponent);

						// now initialize the component's image
						BackgroundImageComponent->SetImage(BackgroundImageComponent->GetImage());
					}
				}
				else if ( BackgroundImageComponent != NULL )
				{
					// a property of the ImageComponent was changed
					if ( ModifiedProperty->GetFName() == TEXT("ImageRef") && BackgroundImageComponent->GetImage() != NULL )
					{
#if 0
						USurface* CurrentValue = BackgroundImageComponent->GetImage();

						// changed the value of the image texture/material
						// clear the data store binding
						//@fixme ronp - do we always need to clear the data store binding?
						SetDataStoreBinding(TEXT(""));

						// clearing the data store binding value may have cleared the value of the image component's texture,
						// so restore the value now
						SetImage(CurrentValue);
#endif
					}
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Called after this object has been completely de-serialized.  This version migrates values for the deprecated PanelBackground,
 * Coordinates, and PrimaryStyle properties over to the BackgroundImageComponent.
 */
void UUIPanel::PostLoad()
{
	Super::PostLoad();

	if ( !GIsUCCMake && GetLinkerVersion() < VER_FIXED_UISCROLLBARS  )
	{
		MigrateImageSettings(PanelBackground, Coordinates, PrimaryStyle, BackgroundImageComponent, TEXT("PanelBackground"));
	}
}


/* ==========================================================================================================
	UIFrameBox
========================================================================================================== */
/* === UIFrameBox interface === */
/**
 * Changes the background image for one of the image components.
 *
 * @param	ImageToSet		The image component we are going to set the image for.
 * @param	NewImage		the new surface to use for this UIImage
 */
void UUIFrameBox::SetBackgroundImage( EFrameBoxImage ImageToSet, USurface* NewImage )
{
	if ( BackgroundImageComponent[ImageToSet] != NULL )
	{
		BackgroundImageComponent[ImageToSet]->SetImage(NewImage);
	}
}

/* === UIObject interface === */
/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UUIFrameBox::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	for(INT ImageIdx=0; ImageIdx<FBI_MAX; ImageIdx++)
	{
		VALIDATE_COMPONENT(BackgroundImageComponent[ImageIdx]);
		AddStyleSubscriber(BackgroundImageComponent[ImageIdx]);
	}
}

/* === UUIScreenObject interface === */
/**
 * Render this widget.
 *
 * @param	Canvas	the canvas to use for rendering this widget
 */
void UUIFrameBox::Render_Widget( FCanvas* Canvas )
{
	if ( BackgroundImageComponent != NULL )
	{
		FRenderParameters ImageParams;
		FRenderParameters Parameters(
			RenderBounds[UIFACE_Left], RenderBounds[UIFACE_Top],
			RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left],
			RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top]
		);

		// Top Left
		ImageParams = Parameters;
		ImageParams.DrawXL = BackgroundCornerSizes.TopLeft[0];
		ImageParams.DrawYL = BackgroundCornerSizes.TopLeft[1];
		BackgroundImageComponent[FBI_TopLeft]->RenderComponent(Canvas, ImageParams);

		// Top
		ImageParams = Parameters;
		ImageParams.DrawX += BackgroundCornerSizes.TopLeft[0];
		ImageParams.DrawXL -= (BackgroundCornerSizes.TopLeft[0] + BackgroundCornerSizes.TopRight[0]);
		ImageParams.DrawYL = BackgroundCornerSizes.TopHeight;
		BackgroundImageComponent[FBI_Top]->RenderComponent(Canvas, ImageParams);

		// Top Right
		ImageParams = Parameters;
		ImageParams.DrawX += Parameters.DrawXL - BackgroundCornerSizes.TopRight[0];
		ImageParams.DrawXL = BackgroundCornerSizes.TopRight[0];
		ImageParams.DrawYL = BackgroundCornerSizes.TopRight[1];
		BackgroundImageComponent[FBI_TopRight]->RenderComponent(Canvas, ImageParams);

		// Center Left
		ImageParams = Parameters;
		ImageParams.DrawY += BackgroundCornerSizes.TopLeft[1];
		ImageParams.DrawXL = BackgroundCornerSizes.CenterLeftWidth;
		ImageParams.DrawYL -= (BackgroundCornerSizes.TopLeft[1] + BackgroundCornerSizes.BottomLeft[1]);
		BackgroundImageComponent[FBI_CenterLeft]->RenderComponent(Canvas, ImageParams);

		// Center
		ImageParams = Parameters;
		ImageParams.DrawX += BackgroundCornerSizes.CenterLeftWidth;
		ImageParams.DrawY += BackgroundCornerSizes.TopHeight;
		ImageParams.DrawXL -= (BackgroundCornerSizes.CenterLeftWidth + BackgroundCornerSizes.CenterRightWidth);
		ImageParams.DrawYL -= (BackgroundCornerSizes.TopHeight + BackgroundCornerSizes.BottomHeight);
		BackgroundImageComponent[FBI_Center]->RenderComponent(Canvas, ImageParams);

		// Center Right
		ImageParams = Parameters;
		ImageParams.DrawX += Parameters.DrawXL - BackgroundCornerSizes.CenterRightWidth;
		ImageParams.DrawY += BackgroundCornerSizes.TopRight[1];
		ImageParams.DrawXL = BackgroundCornerSizes.CenterRightWidth;
		ImageParams.DrawYL -= (BackgroundCornerSizes.TopRight[1] + BackgroundCornerSizes.BottomRight[1]);
		BackgroundImageComponent[FBI_CenterRight]->RenderComponent(Canvas, ImageParams);

		// Bottom Left
		ImageParams = Parameters;
		ImageParams.DrawY += Parameters.DrawYL - BackgroundCornerSizes.BottomLeft[1];
		ImageParams.DrawXL = BackgroundCornerSizes.BottomLeft[0];
		ImageParams.DrawYL = BackgroundCornerSizes.BottomLeft[1];
		BackgroundImageComponent[FBI_BottomLeft]->RenderComponent(Canvas, ImageParams);

		// Bottom
		ImageParams = Parameters;
		ImageParams.DrawX += BackgroundCornerSizes.BottomLeft[0];
		ImageParams.DrawY += Parameters.DrawYL - BackgroundCornerSizes.BottomHeight;
		ImageParams.DrawXL -= (BackgroundCornerSizes.BottomLeft[0] + BackgroundCornerSizes.BottomRight[0]);
		ImageParams.DrawYL = BackgroundCornerSizes.BottomHeight;
		BackgroundImageComponent[FBI_Bottom]->RenderComponent(Canvas, ImageParams);

		// Bottom Right
		ImageParams = Parameters;
		ImageParams.DrawX += Parameters.DrawXL - BackgroundCornerSizes.BottomRight[0];
		ImageParams.DrawY += Parameters.DrawYL - BackgroundCornerSizes.BottomRight[1];
		ImageParams.DrawXL = BackgroundCornerSizes.BottomRight[0];
		ImageParams.DrawYL = BackgroundCornerSizes.BottomRight[1];
		BackgroundImageComponent[FBI_BottomRight]->RenderComponent(Canvas, ImageParams);
	}
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUIFrameBox::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	/*
	if ( PropertyThatChanged.Num() > 0 )
	{
	UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
	if ( MemberProperty != NULL )
	{
	FName PropertyName = MemberProperty->GetFName();
	if ( PropertyName == TEXT("BackgroundImageComponent") )
	{
	// this represents the inner-most property that the user modified
	UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

	// if the value of the BackgroundImageComponent itself was changed
	if ( MemberProperty == ModifiedProperty && BackgroundImageComponent != NULL )
	{
	// the user either cleared the value of the BackgroundImageComponent (which should never happen since
	// we use the 'noclear' keyword on the property declaration), or is assigning a new value to the BackgroundImageComponent.
	// Unsubscribe the current component from our list of style resolvers.
	RemoveStyleSubscriber(BackgroundImageComponent);
	}
	}
	}
	}
	*/
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIFrameBox::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	/*
	if ( PropertyThatChanged.Num() > 0 )
	{
	UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
	if ( MemberProperty != NULL )
	{
	FName PropertyName = MemberProperty->GetFName();
	if ( PropertyName == TEXT("BackgroundImageComponent") )
	{
	// this represents the inner-most property that the user modified
	UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

	// if the value of the BackgroundImageComponent itself was changed
	if ( MemberProperty == ModifiedProperty )
	{
	if ( BackgroundImageComponent != NULL )
	{
	UUIComp_DrawImage* ComponentTemplate = GetArchetype<UUIPanel>()->BackgroundImageComponent;
	if ( ComponentTemplate != NULL )
	{
	BackgroundImageComponent->StyleResolverTag = ComponentTemplate->StyleResolverTag;
	}
	else
	{
	BackgroundImageComponent->StyleResolverTag = TEXT("Panel Background Style");
	}

	// user created a new background image component - add it to the list of style subscribers
	AddStyleSubscriber(BackgroundImageComponent);

	// now initialize the component's image
	BackgroundImageComponent->SetImage(BackgroundImageComponent->GetImage());
	}
	}
	else if ( BackgroundImageComponent != NULL )
	{
	// a property of the ImageComponent was changed
	if ( ModifiedProperty->GetFName() == TEXT("ImageRef") && BackgroundImageComponent->GetImage() != NULL )
	{
	#if 0
	USurface* CurrentValue = BackgroundImageComponent->GetImage();

	// changed the value of the image texture/material
	// clear the data store binding
	//@fixme ronp - do we always need to clear the data store binding?
	SetDataStoreBinding(TEXT(""));

	// clearing the data store binding value may have cleared the value of the image component's texture,
	// so restore the value now
	SetImage(CurrentValue);

	#endif
	}
	}
	}
	}
	}
	}
	*/

	Super::PostEditChange(PropertyThatChanged);
}

/* ==========================================================================================================
	UUISafeRegionPanel
========================================================================================================== */

/**
 * Initializes the panel and sets its position to match the safe region.
 * This is ugly.
 */
void UUISafeRegionPanel::ResolveFacePosition(EUIWidgetFace Face)
{
	// if this is the first face of this panel that is being resolved, set the position of the panel according to the current viewport size
	if ( GetNumResolvedFaces() == 0 )
	{
		AlignPanel();
	}

	Super::ResolveFacePosition(Face);
}


void UUISafeRegionPanel::Render_Widget( FCanvas* Canvas )
{
	if ( !GIsGame && !bHideBounds ) 
	{
		// Render the bounds 
		FLOAT X  = RenderBounds[UIFACE_Left];
		FLOAT Y  = RenderBounds[UIFACE_Top];
		FLOAT tW = RenderBounds[UIFACE_Right] - RenderBounds[UIFACE_Left];
		FLOAT tH = RenderBounds[UIFACE_Bottom] - RenderBounds[UIFACE_Top];

		DrawTile(Canvas, X,Y, tW, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,FLinearColor(0.6f,0.6f,0.2f,1.0f));
		DrawTile(Canvas, X,Y, 1.0f, tH, 0.0f, 0.0f, 1.0f, 1.0f,FLinearColor(0.6f,0.6f,0.2f,1.0f));

		DrawTile(Canvas, X,Y+tH, tW, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,FLinearColor(0.6f,0.6f,0.2f,1.0f));
		DrawTile(Canvas, X+tW,Y, 1.0f, tH, 0.0f, 0.0f, 1.0f, 1.0f,FLinearColor(0.6f,0.6f,0.2f,1.0f));
	}

	Super::Render_Widget(Canvas);
}

void UUISafeRegionPanel::AlignPanel()
{
	FVector2D ViewportSize;

	// Calculate the Preview Platform.

	if( OwnerScene && OwnerScene->GetViewportSize(ViewportSize) )
	{
		if ( bForce4x3AspectRatio )
		{
			FLOAT PercentageAmount;

			if ( bUseFullRegionIn4x3 )
			{
				PercentageAmount = RegionPercentages( ESRT_FullRegion );
			}
			else
			{
				PercentageAmount = RegionPercentages( RegionType );
			}

			// Caculate the Height/Width maintining a 4:3 ratio using the height

			INT Height = appTrunc( ViewportSize.Y * PercentageAmount );
			INT Width = appTrunc( FLOAT(Height) * 1.3333334f );

			FLOAT Left = (ViewportSize.X - Width) * 0.5;
			FLOAT Top = (ViewportSize.Y - Height) * 0.5;

			SetPosition(Left,Top,Left+Width, Top+Height, EVALPOS_PixelViewport, FALSE);
		}
		else
		{
			FLOAT PercentageAmount;

			if ( bUseFullRegionIn4x3 && ( (ViewportSize.X / ViewportSize.Y) - 1.333333 <= KINDA_SMALL_NUMBER ) )
			{
				PercentageAmount = RegionPercentages( ESRT_FullRegion );
			}
			else
			{
				PercentageAmount = RegionPercentages( RegionType );
			}

			FLOAT BorderPerc = (1.0 - PercentageAmount) * 0.5;

			SetPosition(BorderPerc, BorderPerc, PercentageAmount, PercentageAmount, EVALPOS_PercentageOwner, FALSE);
		}
	}

}

void UUISafeRegionPanel::PostEditChange(UProperty* PropertyThatChanged )
{
	AlignPanel();
	Super::PostEditChange(PropertyThatChanged);
}

/* ==========================================================================================================
	UITabControl
========================================================================================================== */
/* === UITabControl interface === */
/**
 * Enables the bUpdateLayout flag and triggers a scene update to occur during the next frame.
 */
void UUITabControl::RequestLayoutUpdate()
{
	bUpdateLayout = TRUE;
	RequestSceneUpdate(TRUE,TRUE);
}

/**
 * Positions and resizes the tab buttons according the tab control's configuration.
 */
void UUITabControl::ReapplyLayout()
{
	// first, setup all the docking links between the tab control, tab buttons, and tab pages
	SetupDockingRelationships();

	// now resize the tabs according to the value of TabSizeMode
	switch ( TabSizeMode )
	{
	case TAST_Manual:
		// nothing - the size for each button is set manually
		break;

	case TAST_Fill:
		//@todo
		//break;

	case TAST_Auto:
		{
			// the width (or height if the tabs are vertical) of each tab button is determined
			// by the length of its caption; so just enable auto-sizing
			for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
			{
				UUITabButton* TabButton = Pages(PageIndex)->TabButton;
				if ( TabButton != NULL && TabButton->StringRenderComponent != NULL )
				{
					const FLOAT SplitPadding = TabButtonPadding.Value * 0.5f;
					TabButton->StringRenderComponent->eventSetAutoSizePadding(UIORIENT_Horizontal, 
						SplitPadding, SplitPadding, TabButtonPadding.ScaleType, TabButtonPadding.ScaleType);
					TabButton->StringRenderComponent->eventEnableAutoSizing(UIORIENT_Horizontal, TRUE);
				}
			}
		}
		break;
	}
}

/**
 * Set up the docking links between the tab control, buttons, and pages, based on the TabDockFace.
 */
void UUITabControl::SetupDockingRelationships()
{
	if ( TabDockFace < UIFACE_MAX )
	{
		FLOAT ActualButtonHeight = TabButtonSize.GetValue(this, EVALPOS_PixelViewport);

		EUIWidgetFace TargetFace=(EUIWidgetFace)TabDockFace;
		EUIWidgetFace SourceFace = GetOppositeFace(TabDockFace);

		EUIWidgetFace AlignmentSourceFace, AlignmentTargetFace;
		if ( TargetFace == UIFACE_Top || TargetFace == UIFACE_Bottom )
		{
			AlignmentSourceFace = AlignmentTargetFace = UIFACE_Left;
			if ( TargetFace == UIFACE_Bottom )
			{
				ActualButtonHeight *= -1;
			}
		}
		else
		{
			AlignmentSourceFace = AlignmentTargetFace = UIFACE_Top;
		}

		UUIObject* CurrentAlignmentDockTarget = this;
		for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
		{
			UUITabPage* TabPage = Pages(PageIndex);
			UUITabButton* TabButton = TabPage->TabButton;

			checkSlow(TabPage!=NULL);
			checkSlow(TabButton!=NULL);

			// dock the buttons to the tab control
			TabButton->SetDockTarget(TargetFace, this, TargetFace);
			TabButton->SetDockTarget(SourceFace, this, TargetFace, ActualButtonHeight);
			TabButton->SetDockTarget(AlignmentSourceFace, CurrentAlignmentDockTarget, AlignmentTargetFace);


			// dock the corresponding face of the page page to the button
			TabPage->SetDockTarget(TargetFace, TabButton, SourceFace);

			// then dock the remaining faces of the page to the tab control
			for ( INT FaceIndex = 0; FaceIndex < UIFACE_MAX; FaceIndex++ )
			{
				if ( FaceIndex != TargetFace )
				{
					TabPage->SetDockTarget(FaceIndex, this, FaceIndex);
				}
			}

			CurrentAlignmentDockTarget = TabButton;
			AlignmentTargetFace = GetOppositeFace(AlignmentSourceFace);
		}
	}
}

/**
 * Returns the number of pages in this tab control.
 */
INT UUITabControl::GetPageCount() const
{
	return Pages.Num();
}

/**
 * Returns a reference to the page at the specified index.
 */
UUITabPage* UUITabControl::GetPageAtIndex( INT PageIndex ) const
{
	UUITabPage* Result = NULL;
	if ( Pages.IsValidIndex(PageIndex) )
	{
		Result = Pages(PageIndex);
	}
	return Result;
}

/**
 * Creates a new UITabPage of the specified class as well as its associated tab button.
 *
 * @param	TabPageClass	the class to use for creating the tab page.
 *
 * @return	a pointer to a new instance of the specified UITabPage class
 */
UUITabPage* UUITabControl::CreateTabPage( UClass* TabPageClass )
{
	UUITabPage* Result = NULL;

	if ( TabPageClass != NULL )
	{
		checkSlow(TabPageClass->IsChildOf(UUITabPage::StaticClass()));

		// first, we need to create the tab button; to do this, we call a static method in the tab page class,
		// which we'll need the TabPageClass's CDO for
		UUITabPage* TabPageCDO = TabPageClass->GetDefaultObject<UUITabPage>();
		UUITabButton* TabButton = TabPageCDO->eventCreateTabButton(this);

#if !FINAL_RELEASE
		if ( TabButton != NULL )
		{
			FScopedObjectStateChange TabButtonNotification(TabButton);

			// now that we have the TabButton, have it create the tab page using the specified class
			UUITabPage* NewTabPage = Cast<UUITabPage>(TabButton->CreateWidget(TabButton, TabPageClass));
			FScopedObjectStateChange TabPageNotification(NewTabPage);

			// need to link the tab page and tab button together.
			if ( NewTabPage->eventLinkToTabButton(TabButton, this) )
			{
				Result = NewTabPage;
			}
			else
			{
				TabPageNotification.CancelEdit();
				TabButtonNotification.CancelEdit();
			}
		}
#endif
	}
	return Result;
}

/* === UIObject interface === */
/**
 * Render this widget.
 *
 * @param	Canvas	the FCanvas to use for rendering this widget
 */
void UUITabControl::Render_Widget( FCanvas* Canvas )
{
	// don't call super - it's pure virtual
}

/**
 * Adds docking nodes for all faces of this widget to the specified scene
 *
 * @param	DockingStack	the docking stack to add this widget's docking.  Generally the scene's DockingStack.
 *
 * @return	TRUE if docking nodes were successfully added for all faces of this widget.
 */
UBOOL UUITabControl::AddDockingLink( TArray<FUIDockingNode>& DockingStack )
{
	if ( bUpdateLayout )
	{
		bUpdateLayout = FALSE;
		ReapplyLayout();
	}

	return Super::AddDockingLink(DockingStack);
}

/**
 * Called when a style reference is resolved successfully.  Applies the TabButtonCaptionStyle and TabButtonBackgroundStyle
 * to the tab buttons.
 *
 * @param	ResolvedStyle			the style resolved by the style reference
 * @param	StyleProperty			the name of the style reference property that was resolved.
 * @param	ArrayIndex				the array index of the style reference that was resolved.  should only be >0 for style reference arrays.
 * @param	bInvalidateStyleData	if TRUE, the resolved style is different than the style that was previously resolved by this style reference.
 */
void UUITabControl::OnStyleResolved( UUIStyle* ResolvedStyle, const FStyleReferenceId& StylePropertyId, INT ArrayIndex, UBOOL bInvalidateStyleData )
{
	Super::OnStyleResolved(ResolvedStyle,StylePropertyId,ArrayIndex,bInvalidateStyleData);

	FString StylePropertyName = StylePropertyId.GetStyleReferenceName();
	if ( StylePropertyName == TEXT("TabButtonBackgroundStyle") || StylePropertyName == TEXT("TabButtonCaptionStyle") )
	{
		for ( INT PageIndex = 0; PageIndex < Pages.Num(); PageIndex++ )
		{
			UUITabPage* Page = Pages(PageIndex);
			if ( Page != NULL && Page->TabButton != NULL && Page->TabButton )
			{
				Page->TabButton->SetWidgetStyle(ResolvedStyle, StylePropertyId, ArrayIndex);
			}
		}
	}
}

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
void UUITabControl::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	Super::Initialize(inOwnerScene, inOwner);
}

/**
 * Generates a array of UI Action keys that this widget supports.
 *
 * @param	out_KeyNames	Storage for the list of supported keynames.
 */
void UUITabControl::GetSupportedUIActionKeyNames( TArray<FName>& out_KeyNames )
{
	Super::GetSupportedUIActionKeyNames(out_KeyNames);

	out_KeyNames.AddUniqueItem(UIKEY_NextPage);
	out_KeyNames.AddUniqueItem(UIKEY_PreviousPage);
}

/**
 * Assigns values to the links which are used for navigating through this widget using the keyboard.  Sets the first and
 * last focus targets for this widget as well as the next/prev focus targets for all children of this widget.
 *
 * This version clears the navigation links between the tab buttons so that
 */
void UUITabControl::RebuildKeyboardNavigationLinks()
{
	Super::RebuildKeyboardNavigationLinks();
}

/**
 * Sets focus to the next control in the tab order (relative to Sender) for widget.  If Sender is the last control in
 * the tab order, propagates the call upwards to this widget's parent widget.
 *
 * @param	Sender			the widget to use as the base for determining which control to focus next
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 *
 * @return	TRUE if we successfully set focus to the next control in tab order.  FALSE if Sender was the last eligible
 *			child of this widget or we couldn't otherwise set focus to another control.
 */
UBOOL UUITabControl::NextControl( UUIScreenObject* Sender, INT PlayerIndex/*=0*/ )
{
	debugf(TEXT("UUITabControl::NextControl  Sender:%s  Focused:%s"), *Sender->GetName(), *GetFocusedControl(PlayerIndex)->GetName());
	UBOOL bResult = FALSE;

	UUITabButton* TabButtonSender = Cast<UUITabButton>(Sender);

	// if the sender is one of this tab control's tab buttons, it means that the currently focused control
	// is the last control in the currently active page and the focus chain is attempting to set focus to the
	// next page's first control.  We don't allow this because in the tab control, navigation between pages can
	// only happen when ActivatePage is called.  Instead, what we do is make the tab control itself the overall
	// focused control so that the user can use the arrow keys to move between tab buttons.
	if ( TabButtonSender == NULL || TabButtonSender->GetOwner() != this )
	{
		bResult = Super::NextControl(Sender, PlayerIndex);
	}
	else
	{
		GainFocus(NULL, PlayerIndex);

		// make the tab button the targeted tab button
		TabButtonSender->ActivateStateByClass(UUIState_TargetedTab::StaticClass(), PlayerIndex);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Sets focus to the previous control in the tab order (relative to Sender) for widget.  If Sender is the first control in
 * the tab order, propagates the call upwards to this widget's parent widget.
 *
 * @param	Sender			the widget to use as the base for determining which control to focus next
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player that generated the focus change.
 *
 * @return	TRUE if we successfully set focus to the previous control in tab order.  FALSE if Sender was the first eligible
 *			child of this widget or we couldn't otherwise set focus to another control.
 */
UBOOL UUITabControl::PrevControl( UUIScreenObject* Sender, INT PlayerIndex/*=0*/ )
{
	debugf(TEXT("UUITabControl::PrevControl  Sender:%s  Focused:%s"), *Sender->GetName(), *GetFocusedControl(PlayerIndex)->GetName());
	UBOOL bResult = FALSE;

	UUITabButton* TabButtonSender = Cast<UUITabButton>(Sender);

	// if the sender is one of this tab control's tab buttons, it means that the currently focused control
	// is the first control in the currently active page and the focus chain is attempting to set focus to the
	// previous page's last control.  We don't allow this because in the tab control, navigation between pages can
	// only happen when ActivatePage is called.  Instead, what we do is make the tab control itself the overall
	// focused control so that the user can use the arrow keys to move between tab buttons.
	if ( TabButtonSender == NULL || TabButtonSender->GetOwner() != this )
	{
		bResult = Super::PrevControl(Sender, PlayerIndex);
	}
	else
	{
		GainFocus(NULL, PlayerIndex);

		// make the tab button the targeted tab button
		TabButtonSender->ActivateStateByClass(UUIState_TargetedTab::StaticClass(), PlayerIndex);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Sets focus to the child widget that is next in the specified direction in the navigation network within this widget.
 *
 * @param	Sender		Control that called NavigateFocus.  Possible values are:
 *						-	if NULL is specified, it indicates that this is the first step in a focus change.  The widget will
 *							attempt to set focus to its most eligible child widget.  If there are no eligible child widgets, this
 *							widget will enter the focused state and start propagating the focus chain back up through the Owner chain
 *							by calling SetFocus on its Owner widget.
 *						-	if Sender is the widget's owner, it indicates that we are in the middle of a focus change.  Everything else
 *							proceeds the same as if the value for Sender was NULL.
 *						-	if Sender is a child of this widget, it indicates that focus has been successfully changed, and the focus is now being
 *							propagated upwards.  This widget will now enter the focused state and continue propagating the focus chain upwards through
 *							the owner chain.
 * @param	Direction 		the direction to navigate focus.
 * @param	PlayerIndex		the index [into the Engine.GamePlayers array] for the player to set focus for.
 *
 * @return	TRUE if the next child widget in the navigation network for the specified direction successfully received focus.
 */
UBOOL UUITabControl::NavigateFocus( UUIScreenObject* Sender, BYTE Direction, INT PlayerIndex/*=0*/ )
{
	debugf(TEXT("UUITabControl::NavigateFocus  Sender:%s  Direction:%s  Focused:%s"), *Sender->GetName(), *GetDockFaceText(Direction), *GetFocusedControl(PlayerIndex)->GetName());
	UBOOL bResult = FALSE;

	UUITabButton* TabButtonSender = Cast<UUITabButton>(Sender);

	// if the sender is one of this tab control's tab buttons, it means that the currently focused control
	// is the first or last control in the currently active page and the focus chain is attempting to set focus to the
	// the nearest sibling of that tab button.  We don't allow this because in the tab control, navigation between pages can
	// only happen when ActivatePage is called.  Instead, what we do is make the tab control itself the overall
	// focused control so that the user can use the arrow keys to move between tab buttons.
	if ( TabButtonSender == NULL || TabButtonSender->GetOwner() != this )
	{
		bResult = Super::NavigateFocus(Sender, Direction, PlayerIndex);
	}
	else
	{
		GainFocus(NULL, PlayerIndex);

		// make the tab button the targeted tab button
		TabButtonSender->ActivateStateByClass(UUIState_TargetedTab::StaticClass(), PlayerIndex);
		bResult = TRUE;
	}

	return bResult;
}

/**
 * Handles input events for this widget.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUITabControl::ProcessInputKey( const FSubscribedInputEventParameters& EventParms )
{
	UBOOL bResult = FALSE;
	if ( EventParms.InputAliasName == UIKEY_NextPage || EventParms.InputAliasName == UIKEY_PreviousPage )
	{
		if ( EventParms.EventType == IE_Pressed || EventParms.EventType == IE_Repeat )
		{
			if ( EventParms.InputAliasName == UIKEY_NextPage )
			{
				eventActivateNextPage(EventParms.PlayerIndex);
			}
			else
			{
				eventActivatePreviousPage(EventParms.PlayerIndex);
			}
		}

		bResult = TRUE;
	}

	// Make sure to call the superclass's implementation after trying to consume input ourselves so that
	// we can respond to events defined in the super's class.
	bResult = bResult || Super::ProcessInputKey(EventParms);
	return bResult;
}

/* === UObject interface === */
/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUITabControl::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);
}

/**
 * Called when a property value from a member struct or array has been changed in the editor.
 */
void UUITabControl::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PostEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* ModifiedMemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( ModifiedMemberProperty != NULL )
		{
			FName PropertyName = ModifiedMemberProperty->GetFName();
			if ( PropertyName == TEXT("TabDockFace") || PropertyName == TEXT("TabButtonSize") || PropertyName == TEXT("Pages") )
			{
				RequestLayoutUpdate();
			}
			else if ( PropertyName == TEXT("TabSizeMode") )
			{
				RequestLayoutUpdate();

				// if the user changed the way tabs are autosized, we'll need to reapply formatting to the tab's captions
				RequestFormattingUpdate();
			}
		}
	}
}

/* ==========================================================================================================
	UITabButton
========================================================================================================== */
/**
 * Determines whether this page can be activated.  Calls the IsActivationAllowed delegate to provide other objects
 * a chance to veto the activation of this button.
 *
 * Child classes which override this method should call Super::CanActivateButton() FIRST and only check additional
 * conditions if the return value is true.
 *
 * @param	PlayerIndex	the index [into the Engine.GamePlayers array] for the player that wishes to activate this page.
 *
 * @return	TRUE if this button is allowed to become the active tab button.
 */
UBOOL UUITabButton::CanActivateButton( INT PlayerIndex )
{
	UBOOL bResult = FALSE;

	if ( GIsGame && IsEnabled(PlayerIndex) && TabPage != NULL )
	{
		if ( DELEGATE_IS_SET(IsActivationAllowed) )
		{
			bResult = delegateIsActivationAllowed(this, PlayerIndex);
		}
		else
		{
			bResult = TRUE;
		}
	}

	return bResult;
}

/**
 * Returns TRUE if this widget has a UIState_TargetedTab object in its StateStack
 */
UBOOL UUITabButton::IsTargeted( INT PlayerIndex/*=0*/ ) const
{
	return HasActiveStateOfClass(UUIState_TargetedTab::StaticClass(),PlayerIndex);
}

/* ==========================================================================================================
	UITabPage
========================================================================================================== */
/**
 * Returns the tab control that contains this tab page, or NULL if it's not part of a tab control.
 */
UUITabControl* UUITabPage::GetOwnerTabControl() const
{
	UUITabControl* Result=NULL;
	for ( UUIObject* NextOwner = GetOwner(); NextOwner && Result == NULL; NextOwner = NextOwner->GetOwner() )
	{
		Result = Cast<UUITabControl>(NextOwner);
	}
	if ( Result == NULL )
	{
		if ( TabButton != NULL && TabButton != GetOwner() )
		{
			for ( UUIObject* NextOwner = TabButton->GetOwner(); NextOwner && Result == NULL; NextOwner = NextOwner->GetOwner() )
			{
				Result = Cast<UUITabControl>(NextOwner);
			}
		}

		if ( Result == NULL && GetOuter() != GetOwner() )
		{
			for ( UUIObject* NextOwner = Cast<UUIObject>(GetOuter()); NextOwner && Result == NULL; NextOwner = Cast<UUIObject>(NextOwner->GetOuter()) )
			{
				Result = Cast<UUITabControl>(NextOwner);
			}
		}
	}
	return Result;
}

/* === UIScreenObject interface === */
/**
 * Called when this widget is created.
 */
void UUITabPage::Created( UUIScreenObject* Creator )
{
	UUITabButton* TabButtonCreator = Cast<UUITabButton>(Creator);
	if ( TabButtonCreator != NULL )
	{
		//@fixme ronp - verify that we don't have a TabButton at this point!  we shouldn't, since it's transient now
		TabButton = TabButtonCreator;
	}

	Super::Created(Creator);
}

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
void UUITabPage::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	UUITabButton* TabButtonOwner = Cast<UUITabButton>(inOwner);
	if ( TabButtonOwner != NULL )
	{
		//@fixme ronp - verify that we don't have a TabButton at this point!  we shouldn't, since it's transient now
		TabButton = TabButtonOwner;
	}

	Super::Initialize(inOwnerScene, inOwner);
}

/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
void UUITabPage::SetDataStoreBinding( const FString& MarkupText, INT BindingIndex/*=INDEX_NONE*/ )
{
	switch( BindingIndex )
	{
	case UCONST_TOOLTIP_BINDING_INDEX:
		if ( TabButton != NULL )
		{
			TabButton->SetDataStoreBinding(MarkupText, BindingIndex);
		}
		break;

	case UCONST_DESCRIPTION_DATABINDING_INDEX:
		if ( PageDescription.MarkupString != MarkupText )
		{
			Modify();
			PageDescription.MarkupString = MarkupText;
		}
		break;

	case UCONST_CAPTION_DATABINDING_INDEX:
	case INDEX_NONE:
		if ( TabButton != NULL )
		{
			TabButton->SetDataStoreBinding(MarkupText, INDEX_NONE);
		}

		if ( ButtonCaption.MarkupString != MarkupText )
		{
			Modify();
			ButtonCaption.MarkupString = MarkupText;
		}
		break;

	default:
		if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
		{
			SetDefaultDataBinding(MarkupText, BindingIndex);
		}
	}

	RefreshSubscriberValue(BindingIndex);
}

/**
 * Retrieves the markup string corresponding to the data store that this object is bound to.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	a datastore markup string which resolves to the datastore field that this object is bound to, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
FString UUITabPage::GetDataStoreBinding(INT BindingIndex/*=INDEX_NONE*/) const
{
	FString Result;

	switch( BindingIndex )
	{
	case UCONST_DESCRIPTION_DATABINDING_INDEX:
		Result = PageDescription.MarkupString;
		break;

	case UCONST_CAPTION_DATABINDING_INDEX:
	case INDEX_NONE:
		Result = ButtonCaption.MarkupString;
		break;

	default:
		if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
		{
			Result = GetDefaultDataBinding(BindingIndex);
		}
		break;
	}

	return Result;
}

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
UBOOL UUITabPage::RefreshSubscriberValue(INT BindingIndex/*=INDEX_NONE*/)
{
	UBOOL bResult = FALSE;

	if ( BindingIndex >= UCONST_FIRST_DEFAULT_DATABINDING_INDEX )
	{
		bResult = ResolveDefaultDataBinding(BindingIndex);
	}
	else if ( TabButton != NULL )
	{
		TabButton->SetDataStoreBinding(ButtonCaption.MarkupString, INDEX_NONE);
	}

	return TRUE;//bResult;
}

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
void UUITabPage::GetBoundDataStores(TArray<UUIDataStore*>& out_BoundDataStores)
{
	GetDefaultDataStores(out_BoundDataStores);
	if ( ButtonCaption )
	{
		out_BoundDataStores.AddUniqueItem(*ButtonCaption);
	}
	if ( ButtonToolTip )
	{
		out_BoundDataStores.AddUniqueItem(*ButtonToolTip);
	}
	if ( PageDescription )
	{
		out_BoundDataStores.AddUniqueItem(*PageDescription);
	}
}

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
void UUITabPage::ClearBoundDataStores()
{
	TMultiMap<FName,FUIDataStoreBinding*> DataBindingMap;
	GetDataBindings(DataBindingMap);

	TArray<FUIDataStoreBinding*> DataBindings;
	DataBindingMap.GenerateValueArray(DataBindings);
	for ( INT BindingIndex = 0; BindingIndex < DataBindings.Num(); BindingIndex++ )
	{
		FUIDataStoreBinding* Binding = DataBindings(BindingIndex);
		Binding->ClearDataBinding();
	}

	TArray<UUIDataStore*> DataStores;
	GetBoundDataStores(DataStores);

	for ( INT DataStoreIndex = 0; DataStoreIndex < DataStores.Num(); DataStoreIndex++ )
	{
		UUIDataStore* DataStore = DataStores(DataStoreIndex);
		DataStore->eventSubscriberDetached(this);
	}
}

/* ==========================================================================================================
	UUIScrollFrame
========================================================================================================== */
/* === UUIScrollFrame interface === */
/**
 * Changes the background image for this slider, creating the wrapper UITexture if necessary.
 *
 * @param	NewBarImage		the new surface to use for the slider's background image
 */
void UUIScrollFrame::SetBackgroundImage( USurface* NewBackgroundImage )
{
	if ( BackgroundImageComponent != NULL )
	{
		BackgroundImageComponent->SetImage(NewBackgroundImage);
	}
}

/* === UIObject interface === */
/**
 * Provides a way for widgets to fill their style subscribers array prior to performing any other initialization tasks.
 *
 * This version adds the BackgroundImageComponent (if non-NULL) to the StyleSubscribers array.
 */
void UUIScrollFrame::InitializeStyleSubscribers()
{
	Super::InitializeStyleSubscribers();

	VALIDATE_COMPONENT(BackgroundImageComponent);
	AddStyleSubscriber(BackgroundImageComponent);
}

/**
 * Initializes the buttons and creates the background image.
 *
 * @param	inOwnerScene	the scene to add this widget to.
 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
 *							is being added to the scene's list of children.
 */
void UUIScrollFrame::Initialize( UUIScene* inOwnerScene, UUIObject* inOwner/*=NULL*/ )
{
	if( ScrollbarVertical == NULL )
	{
		ScrollbarVertical = Cast<UUIScrollbar>(CreateWidget(this, UUIScrollbar::StaticClass()));
		InsertChild( ScrollbarVertical );
	}
	if( ScrollbarHorizontal == NULL )
	{
		ScrollbarHorizontal = Cast<UUIScrollbar>(CreateWidget(this, UUIScrollbar::StaticClass()));
		InsertChild( ScrollbarHorizontal );
	}

	ScrollbarVertical->ScrollbarOrientation   = UIORIENT_Vertical;
	ScrollbarHorizontal->ScrollbarOrientation = UIORIENT_Horizontal;
	ScrollbarVertical->EnableCornerPadding(TRUE);
	ScrollbarHorizontal->EnableCornerPadding(TRUE);

	Super::Initialize( inOwnerScene, inOwner );

	// Shouldn't see scroll bars unless they are need.
	ScrollbarVertical->eventPrivateSetVisibility(FALSE);
	ScrollbarHorizontal->eventPrivateSetVisibility(FALSE);
}

/**
 * Generates a array of UI Action keys that this widget supports.
 *
 * @param	out_KeyNames	Storage for the list of supported keynames.
 */
void UUIScrollFrame::GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames )
{

}

/**
 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
 *
 * @param	Face	the face that should be resolved
 */
void UUIScrollFrame::ResolveFacePosition( EUIWidgetFace Face )
{
	Super::ResolveFacePosition( Face );

	// The region could have changed if we are resolving faces.
	if( bResolveRegion )
	{
		// CalculateRegionExtent resets the value of bResolveRegion to FALSE, so this should only happen once
		CalculateRegionExtent( );
	}
}

/**
 * Render this scroll frame.
 *
 * @param	Canvas	the FCanvas to use for rendering this widget
 */
void UUIScrollFrame::Render_Widget( FCanvas* Canvas )
{
	if ( BackgroundImageComponent != NULL && bDisplayBackground )
	{
		FVector2D ViewportOrigin;
		if ( !GetViewportOrigin(ViewportOrigin) )
		{
			ViewportOrigin.X = ViewportOrigin.Y = 0;
		}

		FRenderParameters Parameters(
			RegionExtent.GetPositionValue(this,UIFACE_Left) + ViewportOrigin.X,
			RegionExtent.GetPositionValue(this,UIFACE_Top) + ViewportOrigin.Y,
			RegionExtent.GetPositionValue(this,UIFACE_Right) - RegionExtent.GetPositionValue(this,UIFACE_Left),
			RegionExtent.GetPositionValue(this,UIFACE_Bottom) - RegionExtent.GetPositionValue(this,UIFACE_Top)
			);

		BackgroundImageComponent->RenderComponent(Canvas, Parameters);
	}
}

/**
 * Allow the widget to do any special rendering after its children have been rendered.
 *
 * @param	Canvas	the canvas to use for rendering this widget
 */
void UUIScrollFrame::PostRender_Widget( FCanvas* Canvas )
{
	// Draw border around the visible region, this is an editor only feature
	if( !GIsGame )
	{
		FLinearColor BorderColor(0.0f,1.0f,0.0f);
		FLOAT HorizontalBarWidth = ScrollbarHorizontal->IsVisible() ? ScrollbarHorizontal->GetScrollZoneWidth() : 0.f;
		FLOAT VerticalBarWidth = ScrollbarVertical->IsVisible() ? ScrollbarVertical->GetScrollZoneWidth() : 0.f; 

		FVector2D StartLoc(RenderBounds[UIFACE_Left] , RenderBounds[UIFACE_Top] );
		FVector2D EndLoc(RenderBounds[UIFACE_Right] - VerticalBarWidth , RenderBounds[UIFACE_Bottom] - HorizontalBarWidth );

		// Draw basic 2d box
		DrawBox2D(Canvas, StartLoc, EndLoc,	BorderColor);
	}
}

/**
 * Insert a widget at the specified location, this function makes sure that scrollframe's scrollbars are placed last in the Children array,
 * so they render last 
 *
 * @param	NewChild		the widget to insert
 * @param	InsertIndex		the position to insert the widget.  If not specified, the widget is insert at the end of
 *							the list
 * @param	bRenameExisting	controls what happens if there is another widget in this widget's Children list with the same tag as NewChild.
 *							if TRUE, renames the existing widget giving a unique transient name.
 *							if FALSE, does not add NewChild to the list and returns FALSE.
 *
 * @return	the position that that the child was inserted in, or INDEX_NONE if the widget was not inserted
 */
INT UUIScrollFrame::InsertChild( UUIObject* NewChild, INT InsertIndex/*=INDEX_NONE*/, UBOOL bRenameExisting/*=TRUE*/ )
{
	if(NewChild == ScrollbarHorizontal || NewChild == ScrollbarVertical)
	{
		InsertIndex = INDEX_NONE;	// Scrollbars must be last in the children array to be rendered last
	}
	else
	{
		InsertIndex = 0;   // Insert other children at the beginning of the array, before the scrollbars
	}

	return Super::InsertChild(NewChild, InsertIndex); 
}

/**
 * Handler for responding to child widget changing its position, it recalculates the scrollframe's scroll region bounds
 *
 * @param	Sender	Child widget which has been repositioned
 */
void UUIScrollFrame::OnChildRepositioned(class UUIScreenObject* Sender)
{
	RefreshFormatting();
}

/**
 * Called immediately after a child has been added to this screen object.
 *
 * @param	WidgetOwner		the screen object that the NewChild was added as a child for
 * @param	NewChild		the widget that was added
 */
void UUIScrollFrame::NotifyAddedChild( UUIScreenObject* WidgetOwner, UUIObject* NewChild )
{
	Super::NotifyAddedChild( WidgetOwner, NewChild );

	// Our scrollbars should be ignored.
	if( NewChild != ScrollbarHorizontal && NewChild != ScrollbarVertical )
	{
		// Recalculate the region extent since adding a widget could have affected it.
		RefreshFormatting();
	}
}

/**
 * Called immediately after a child has been removed from this screen object.
 *
 * @param	WidgetOwner		the screen object that the widget was removed from.
 * @param	OldChild		the widget that was removed
 * @param	ExclusionSet	used to indicate that multiple widgets are being removed in one batch; useful for preventing references
 *							between the widgets being removed from being severed.
 */
void UUIScrollFrame::NotifyRemovedChild( UUIScreenObject* WidgetOwner, UUIObject* OldChild, TArray<UUIObject*>* ExclusionSet/*=NULL*/ )
{
	Super::NotifyRemovedChild( WidgetOwner, OldChild, ExclusionSet );

	// Recalculate the region extent since removing a widget could have affected it.
	RefreshFormatting();
}

/**
 * Called when a property is modified that could potentially affect the widget's position onscreen.
 */
void UUIScrollFrame::RefreshPosition()
{	
	Super::RefreshPosition();

	RefreshFormatting();
}

/**
 * Called to globally update the formatting of all UIStrings.
 */
void UUIScrollFrame::RefreshFormatting()
{
	// Recalculate the region extent since the movement of the frame could have affected it.
	bResolveRegion = TRUE;
}

/**
 * Handles input events for this editbox.
 *
 * @param	EventParms		the parameters for the input event
 *
 * @return	TRUE to consume the key event, FALSE to pass it on.
 */
UBOOL UUIScrollFrame::ProcessInputKey( const struct FSubscribedInputEventParameters& EventParms )
{
	return Super::ProcessInputKey( EventParms );
}

/**
 * Iterates through this widgets child and determines the outermost extend that will encapsulate all of them.
 * It will set the RegionExtent member appropriately.
 */
void UUIScrollFrame::CalculateRegionExtent( )
{
	FLOAT PositionX  = Position.GetPositionValue( this, UIFACE_Left, EVALPOS_PixelViewport );
	FLOAT PositionY  = Position.GetPositionValue( this, UIFACE_Top, EVALPOS_PixelViewport );
	FLOAT PositionXL = Position.GetPositionValue( this, UIFACE_Right, EVALPOS_PixelViewport );
	FLOAT PositionYL = Position.GetPositionValue( this, UIFACE_Bottom, EVALPOS_PixelViewport );

	FLOAT RegionX  = PositionX;
	FLOAT RegionY  = PositionY;
	FLOAT RegionXL = PositionXL;
	FLOAT RegionYL = PositionYL;

	// Iterate over our children to see if any of them extend outside of our bounds.
	for( INT i = 0; i < Children.Num( ); ++i )
	{
		UUIObject * Child = Children(i);

		// Our scrollbars should be ignored.
		if( Child == ScrollbarHorizontal || Child == ScrollbarVertical )
			continue;

		FLOAT ChildPositionX  = Child->Position.GetPositionValue( Child, UIFACE_Left, EVALPOS_PixelViewport );
		FLOAT ChildPositionY  = Child->Position.GetPositionValue( Child, UIFACE_Top, EVALPOS_PixelViewport );
		FLOAT ChildPositionXL = Child->Position.GetPositionValue( Child, UIFACE_Right, EVALPOS_PixelViewport );
		FLOAT ChildPositionYL = Child->Position.GetPositionValue( Child, UIFACE_Bottom, EVALPOS_PixelViewport );
		/*
		// We don't want the obscured widgets to be invisible in the editor.
		if( GIsGame )
		{
		// See if this child is completely outside the visible area.
		if( ( ( ChildPositionX < PositionX && ChildPositionXL < PositionX ) || ( ChildPositionX > PositionXL && ChildPositionXL > PositionXL ) ) ||
		( ( ChildPositionY < PositionY && ChildPositionYL < PositionY ) || ( ChildPositionY > PositionYL && ChildPositionYL > PositionYL ) ) )
		{
		Child->bHidden = TRUE;
		}
		}
		*/
		if( ChildPositionX < RegionX )
			RegionX = ChildPositionX;

		if( ChildPositionY < RegionY )
			RegionY = ChildPositionY;

		if( ChildPositionXL > RegionXL )
			RegionXL = ChildPositionXL;

		if( ChildPositionYL > RegionYL )
			RegionYL = ChildPositionYL;
	}

	RegionExtent.SetRawPositionValue(UIFACE_Left,RegionX);
	RegionExtent.SetRawPositionValue(UIFACE_Top,RegionY);
	RegionExtent.SetRawPositionValue(UIFACE_Right,RegionXL);
	RegionExtent.SetRawPositionValue(UIFACE_Bottom,RegionYL);

	// Reset the scrollbars.
	RefreshScrollbars();

	// Resolved...
	bResolveRegion = FALSE;
}

/**
 * Enables and initializes the scrollbars that need to be visible
 */
void UUIScrollFrame::RefreshScrollbars()
{
	const FLOAT HorizontalRegionBounds   = RegionExtent.GetBoundsExtent( this, UIORIENT_Horizontal, EVALPOS_PixelViewport );
	const FLOAT HorizontalPositionBounds = Position.GetBoundsExtent( this, UIORIENT_Horizontal, EVALPOS_PixelViewport );
	const FLOAT VerticalRegionBounds   = RegionExtent.GetBoundsExtent( this, UIORIENT_Vertical, EVALPOS_PixelViewport );
	const FLOAT VerticalPositionBounds = Position.GetBoundsExtent( this, UIORIENT_Vertical, EVALPOS_PixelViewport );

	const FLOAT PositionLeft = Position.GetPositionValue(this, UIFACE_Left, EVALPOS_PixelViewport);
	const FLOAT RegionLeft = RegionExtent.GetPositionValue(this, UIFACE_Left, EVALPOS_PixelViewport);
	const FLOAT PositionTop = Position.GetPositionValue(this, UIFACE_Top, EVALPOS_PixelViewport);
	const FLOAT RegionTop = RegionExtent.GetPositionValue(this, UIFACE_Top, EVALPOS_PixelViewport);

	// See if scroll bars need to be setup and made visible.
	if( HorizontalRegionBounds > HorizontalPositionBounds )
	{
		ScrollbarHorizontal->eventPrivateSetVisibility(TRUE);
	}
	else
	{
		ScrollbarHorizontal->eventPrivateSetVisibility(FALSE);
	}

	// See if scroll bars need to be setup and made visible.
	if( VerticalRegionBounds > VerticalPositionBounds )
	{
		ScrollbarVertical->eventPrivateSetVisibility(TRUE);
	}
	else
	{
		ScrollbarVertical->eventPrivateSetVisibility(FALSE);
	}

	const UBOOL bHorzVisible = ScrollbarHorizontal->IsVisible();
	const UBOOL bVertVisible = ScrollbarVertical->IsVisible();

	FLOAT HorizontalBarWidth = bHorzVisible ? ScrollbarHorizontal->GetScrollZoneWidth() : 0.f;
	FLOAT VerticalBarWidth = bVertVisible ? ScrollbarVertical->GetScrollZoneWidth() : 0.f;
	UBOOL HorizontalPadding = bVertVisible;
	UBOOL VerticalPadding = bHorzVisible;

	ScrollbarHorizontal->EnableCornerPadding(HorizontalPadding);
	ScrollbarVertical->EnableCornerPadding(VerticalPadding);

	ScrollbarHorizontal->SetNudgeSizePixels( 1 );
	ScrollbarHorizontal->SetMarkerSize( (HorizontalPositionBounds - VerticalBarWidth) / HorizontalRegionBounds );
	ScrollbarHorizontal->SetMarkerPosition( (PositionLeft - RegionLeft)/HorizontalRegionBounds );

	ScrollbarVertical->SetNudgeSizePixels( 1 );
	ScrollbarVertical->SetMarkerSize( (VerticalPositionBounds - HorizontalBarWidth) / VerticalRegionBounds );
	ScrollbarVertical->SetMarkerPosition( (PositionTop - RegionTop)/VerticalRegionBounds );
}

/**
 * Attempts to scroll the scroll frame by the PositionChange value passed by the scrollbar.
 *
 * @return	TRUE if the scroll frame was able to scroll. FALSE if the could not scroll such as if the RegionExtent is not greater than the scroll frame's bounds.
 */
UBOOL UUIScrollFrame::ScrollHorizontal( FLOAT PositionChange, UBOOL bPositionMaxed/*=FALSE*/ )
{
	UBOOL Result = FALSE;

	ScrollRegion( UIORIENT_Horizontal, PositionChange, bPositionMaxed );

	return Result;
}

/**
 * Attempts to scroll the scroll frame by the PositionChange value passed by the scrollbar.
 *
 * @return	TRUE if the scroll frame was able to scroll. FALSE if the could not scroll such as if the RegionExtent is not greater than the scroll frame's bounds.
 */
UBOOL UUIScrollFrame::ScrollVertical( FLOAT PositionChange, UBOOL bPositionMaxed/*=FALSE*/ )
{
	UBOOL Result = FALSE;

	ScrollRegion( UIORIENT_Vertical, PositionChange, bPositionMaxed );

	return Result;
}

/**
 * Scrolls all of the child widgets by the specified amount in the specified direction.
 *
 * @param	Dimension		indicates whether the horizontal or vertical scrollbar should be evaluated.
 * @param	PositionChange	indicates the amount that the scrollbar has travelled.
 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
 *                          used to achieve pixel exact scrolling
 */
void UUIScrollFrame::ScrollRegion(  EUIOrientation Dimension, FLOAT PositionChange, UBOOL bPositionMaxed/*=FALSE*/ )
{
	FLOAT Change = CalculatedPositionChange( Dimension, PositionChange, bPositionMaxed);

	EUIWidgetFace Face = Dimension == UIORIENT_Horizontal ? UIFACE_Left : UIFACE_Top;
	EUIWidgetFace OpposingFace = Dimension == UIORIENT_Horizontal ? UIFACE_Right : UIFACE_Bottom;
	for( INT i = 0; i < Children.Num( ); ++i )
	{
		UUIObject * Child = Children(i);


		// Our scrollbars should be ignored.
		if( Child == ScrollbarHorizontal || Child == ScrollbarVertical )
			continue;

		FLOAT FaceValue = Child->Position.GetPositionValue( Child, Face, EVALPOS_PixelViewport );
		FaceValue -= Change;

		Child->Position.SetPositionValue( Child, FaceValue, Face, EVALPOS_PixelViewport );
	}

	// Update the RegionExtent Position
	FLOAT RegionFaceValue = RegionExtent.GetPositionValue( this, Face, EVALPOS_PixelViewport );
	RegionFaceValue -= Change;
	RegionExtent.SetPositionValue(this, RegionFaceValue, Face, EVALPOS_PixelViewport );

	FLOAT RegionOpposingFaceValue = RegionExtent.GetPositionValue( this, OpposingFace, EVALPOS_PixelViewport );
	RegionOpposingFaceValue -= Change;
	RegionExtent.SetPositionValue(this, RegionOpposingFaceValue, OpposingFace, EVALPOS_PixelViewport );
}

/**
 * Takes position change given in previously defined scrollbar units and translates it into actual pixel values by which the UIScrollframe's 
 * content should be moved. To achieve pixel exact scrolling function also does special bounds evaluation when scrollbar's marker has reached 
 * its rightmost/leftmost position which is indicated by the bPositionMaxed flag.
 *
 * @param	Dimension		indicates whether the horizontal or vertical scrollbar should be evaluated.
 * @param	PositionChange	indicates the amount that the scrollbar has travelled.
 * @param	bPositionMaxed	indicates that the scrollbar's marker has reached its farthest available position,
 *                          used to achieve pixel exact scrolling
 *
 * @return	number of pixels by which the scrollframe contents should be scrolled
 */
FLOAT UUIScrollFrame::CalculatedPositionChange( EUIOrientation Dimension, FLOAT PositionChange, UBOOL bPositionMaxed)
{
	FLOAT Change; 

	FLOAT RegionBounds   = RegionExtent.GetBoundsExtent( this, Dimension, EVALPOS_PixelViewport );
	FLOAT PositionBounds = Position.GetBoundsExtent( this, Dimension, EVALPOS_PixelViewport );

	UUIScrollbar * Scrollbar = Dimension == UIORIENT_Horizontal ? ScrollbarHorizontal : ScrollbarVertical;
	UUIScrollbar * OppositeScrollbar = Dimension == UIORIENT_Horizontal ? ScrollbarVertical : ScrollbarHorizontal;

	EUIWidgetFace Face = Dimension == UIORIENT_Horizontal ? UIFACE_Left : UIFACE_Top;
	EUIWidgetFace OpposingFace = Dimension == UIORIENT_Horizontal ? UIFACE_Right : UIFACE_Bottom;

	// To obtain "pixel exact" scrolling we have to manually calculate appropriate position change when bPositionMaxed flag is set
	if(bPositionMaxed)
	{
		if(PositionChange > 0)  // scrollbar is scrolled completely to the right or down
		{
			FLOAT OppositeBarWidth = OppositeScrollbar->IsVisible() ? OppositeScrollbar->GetScrollZoneWidth() : 0.f;

			Change = RegionExtent.GetPositionValue(this, OpposingFace, EVALPOS_PixelViewport) - Position.GetPositionValue(this, OpposingFace, EVALPOS_PixelViewport)
				+ OppositeBarWidth;
		}
		else					// scrollbar is scrolled completely to the left or top
		{
			Change = RegionExtent.GetPositionValue(this, Face, EVALPOS_PixelViewport) - Position.GetPositionValue(this, Face, EVALPOS_PixelViewport);
		}
	}
	else
	{
		// Translate pixels moved on a scrollbar into proportional amount of scrollframe pixels  
		const FLOAT BarExtent = Scrollbar->GetScrollZoneExtent() * RegionBounds;
		check(BarExtent>0);

		Change = PositionChange / BarExtent;
	}

	return Change;
}

/**
 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
 */
void UUIScrollFrame::PreEditChange( FEditPropertyChain& PropertyThatChanged )
{
	Super::PreEditChange(PropertyThatChanged);

	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty && BackgroundImageComponent != NULL )
				{
					// the user either cleared the value of the BackgroundImageComponent (which should never happen since
					// we use the 'noclear' keyword on the property declaration), or is assigning a new value to the BackgroundImageComponent.
					// Unsubscribe the current component from our list of style resolvers.
					RemoveStyleSubscriber(BackgroundImageComponent);
				}
			}
		}
	}
}

/**
 * Called when a property value has been changed in the editor.
 */
void UUIScrollFrame::PostEditChange( FEditPropertyChain& PropertyThatChanged )
{
	if ( PropertyThatChanged.Num() > 0 )
	{
		UProperty* MemberProperty = PropertyThatChanged.GetActiveMemberNode()->GetValue();
		if ( MemberProperty != NULL )
		{
			FName PropertyName = MemberProperty->GetFName();
			if ( PropertyName == TEXT("BackgroundImageComponent") )
			{
				// this represents the inner-most property that the user modified
				UProperty* ModifiedProperty = PropertyThatChanged.GetTail()->GetValue();

				// if the value of the BackgroundImageComponent itself was changed
				if ( MemberProperty == ModifiedProperty )
				{
					if ( BackgroundImageComponent != NULL )
					{
						UUIComp_DrawImage* ComponentTemplate = GetArchetype<UUIScrollFrame>()->BackgroundImageComponent;
						if ( ComponentTemplate != NULL )
						{
							BackgroundImageComponent->StyleResolverTag = ComponentTemplate->StyleResolverTag;
						}
						else
						{
							BackgroundImageComponent->StyleResolverTag = TEXT("Background Image Style");
						}

						// user created a new background image component - add it to the list of style subscribers
						AddStyleSubscriber(BackgroundImageComponent);

						// now initialize the component's image
						BackgroundImageComponent->SetImage(BackgroundImageComponent->GetImage());
					}
				}
				else if ( BackgroundImageComponent != NULL )
				{
					// a property of the ImageComponent was changed
					if ( ModifiedProperty->GetFName() == TEXT("ImageRef") && BackgroundImageComponent->GetImage() != NULL )
					{
#if 0
						USurface* CurrentValue = BackgroundImageComponent->GetImage();

						// changed the value of the image texture/material
						// clear the data store binding
						//@fixme ronp - do we always need to clear the data store binding?
						SetDataStoreBinding(TEXT(""));

						// clearing the data store binding value may have cleared the value of the image component's texture,
						// so restore the value now
						SetImage(CurrentValue);
#endif
					}
				}
			}
		}
	}

	Super::PostEditChange(PropertyThatChanged);
}

/**
 * Called after this object has been completely de-serialized.  This version migrates values for the deprecated Background,
 * BackgroundCoordinates, and PrimaryStyle properties over to the BackgroundImageComponent.
 */
void UUIScrollFrame::PostLoad()
{
	Super::PostLoad();

	if ( GetLinkerVersion() < VER_FIXED_UISCROLLBARS )
	{
		if ( GIsEditor )
		{
			if ( ScrollbarVertical != NULL )
			{
				RemoveChild(ScrollbarVertical);
				ScrollbarVertical = NULL;
			}

			if ( ScrollbarHorizontal != NULL )
			{
				RemoveChild(ScrollbarHorizontal);
				ScrollbarHorizontal = NULL;
			}
		}

		if ( !GIsUCCMake )
		{
			MigrateImageSettings(Background, BackgroundCoordinates, PrimaryStyle, BackgroundImageComponent, TEXT("Background"));
		}
	}
}
