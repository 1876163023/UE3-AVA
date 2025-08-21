#include "PrecompiledHeaders.h"
#include "avaGame.h"

//IMPLEMENT_CLASS(UavaSeqEvent_VehicleFactory); // 20070801

IMPLEMENT_CLASS(UavaSeqAct_AnimateMatInstScalarParam);
IMPLEMENT_CLASS(UavaUIAction_ActivateLevelEventString);
IMPLEMENT_CLASS(UavaUIAction_ActivateLevelEventInt);
IMPLEMENT_CLASS(UavaUIAction_ActivateLevelEventObject);
IMPLEMENT_CLASS(UavaSeqEvent_RemoteEventString);
IMPLEMENT_CLASS(UavaSeqEvent_RemoteEventInt);
IMPLEMENT_CLASS(UavaSeqEvent_RemoteEventObject);
IMPLEMENT_CLASS(UavaUIEvent_UIRemoteEvent);
IMPLEMENT_CLASS(UavaUIAction_ActivateUIEvent);
IMPLEMENT_CLASS(UavaUIAction_OpenSceneManaged);



IMPLEMENT_CLASS(UavaUIAction_RemoveListItem);

IMPLEMENT_CLASS(UavaUIAction_SetListIndexFromCellValue);
IMPLEMENT_CLASS(UavaUIAction_SetEnabled);

IMPLEMENT_CLASS(UavaUIEvent_BeforeHidden);
IMPLEMENT_CLASS(UavaSeqAct_Sprintf);
IMPLEMENT_CLASS(UavaSeqAct_ParseIntoArray);
IMPLEMENT_CLASS(UavaSeqAct_TrimStr);
IMPLEMENT_CLASS(UavaSeqCond_HasAudioDevice);

IMPLEMENT_CLASS(UavaUIAction_FakeFullScreen);
IMPLEMENT_CLASS(UavaUIAction_SetFocusMode);
IMPLEMENT_CLASS(UavaUIAction_SetLocation);
IMPLEMENT_CLASS(UavaSeqAct_SetTimer);

IMPLEMENT_CLASS(UavaUIAction_TransitionBase);
	IMPLEMENT_CLASS(UavaUIAction_TransOpacity);
	IMPLEMENT_CLASS(UavaUIAction_TransPosition);
	IMPLEMENT_CLASS(UavaUIAction_TransScale);

IMPLEMENT_CLASS(UavaUIAction_RefreshBindingValue);
IMPLEMENT_CLASS(UavaUIAction_SetDataProviderParm);

IMPLEMENT_CLASS(UavaSeqAct_SwitchInt);
IMPLEMENT_CLASS(UavaSeqCond_SwitchObject);
IMPLEMENT_CLASS(UavaSeqCond_SwitchString);
IMPLEMENT_CLASS(UavaSeqCond_BoolTable);

IMPLEMENT_CLASS(UavaUIEvent_CheckLabelValueChanged);
IMPLEMENT_CLASS(UavaUIEvent_OnMouseTrackerCellChanged);
IMPLEMENT_CLASS(UavaUIEvent_SimpleTextChanged);

IMPLEMENT_CLASS(UavaUIAction_EmulatePlayerInput);

IMPLEMENT_CLASS(UavaUIAction_SetAvaNetTransaction);
IMPLEMENT_CLASS(UavaUIAction_PendingRecreateDevice);
IMPLEMENT_CLASS(UavaSeqCond_CompareInt);


#include "FConfigCacheIni.h"

#include "avaNetClient.h"
#include "avaTransactions.h"
/*!
	@brief
		UIAction_ActivateLevelEvent::Activated()에 String변수 얻어서 설정하는 부분만 추가.
		(2007/01/13 고광록)
*/
void UavaUIAction_ActivateLevelEventString::Activated()
{
	APlayerController* PlayerOwner = NULL;

	// assign the player that activated this event as the Instigator for the remote event
	UUIScreenObject* Owner = GetOwner();
	if ( Owner != NULL )
	{
		if ( !GEngine->GamePlayers.IsValidIndex(PlayerIndex) )
		{
			PlayerIndex = 0;
		}

		ULocalPlayer* LocalPlayer = Owner->GetPlayerOwner(PlayerIndex);
		if ( LocalPlayer != NULL )
		{
			PlayerOwner = LocalPlayer->Actor;
		}
	}

	UBOOL bFoundRemoteEvents = FALSE;

	// "StringValue"라는 이름에 링크된 문자열들을 얻어온다.
	TArray<FString*> sendStrVars;
	TArray<FString*> recvStrVars;
	// 실제 1개의 값만 얻어오게 된다.
	GetStringVars(sendStrVars, TEXT("StringValue"));
	FString sendString = ( sendStrVars.Num() > 0 ) ? *(sendStrVars(0)) : "";

	// now find the level's sequence
	AWorldInfo* WorldInfo = GetWorldInfo();
	if ( WorldInfo != NULL )
	{
		USequence* GameSequence = WorldInfo->GetGameSequence();
		if ( GameSequence != NULL )
		{
			// now find the root sequence
			USequence* RootSequence = GameSequence->GetRootSequence();
			if ( RootSequence != NULL )
			{
				TArray<UavaSeqEvent_RemoteEventString*> RemoteEvents;
				RootSequence->FindSeqObjectsByClass(UavaSeqEvent_RemoteEventString::StaticClass(), (TArray<USequenceObject*>&)RemoteEvents);

				// now iterate through the list of events, activating those events that have a matching event tag
				for ( INT EventIndex = 0; EventIndex < RemoteEvents.Num(); EventIndex++ )
				{
					USeqEvent_RemoteEvent* RemoteEvt = RemoteEvents(EventIndex);
					if ( RemoteEvt != NULL && RemoteEvt->EventName == EventName && RemoteEvt->bEnabled == TRUE )
					{
						recvStrVars.Reset();
						RemoteEvt->GetStringVars(recvStrVars, TEXT("StringValue"));

						// 액션의 보낼 문자열을 받는 이벤트의 문자열로 복사.
						for( int j = 0; j < recvStrVars.Num(); j++ )
							*(recvStrVars(j)) = sendString;

//						debugf(TEXT("Copy StringValue : [%s], numRecvs(%d)"), *sendString, recvStrVars.Num());

						// attempt to activate the remote event
						RemoteEvt->CheckActivate(WorldInfo, PlayerOwner);
						bFoundRemoteEvents = TRUE;
					}
				}
			}
		}
	}

	OutputLinks(bFoundRemoteEvents==TRUE ? 1 : 0).ActivateOutputLink();
}

/*!	@brief
		UIAction_ActivateLevelEvent::Activated()에 Int변수 얻어서 설정하는 부분만 추가.
		(2007/01/29 고광록)
*/
void UavaUIAction_ActivateLevelEventInt::Activated()
{
	APlayerController* PlayerOwner = NULL;

	// assign the player that activated this event as the Instigator for the remote event
	UUIScreenObject* Owner = GetOwner();
	if ( Owner != NULL )
	{
		if ( !GEngine->GamePlayers.IsValidIndex(PlayerIndex) )
		{
			PlayerIndex = 0;
		}

		ULocalPlayer* LocalPlayer = Owner->GetPlayerOwner(PlayerIndex);
		if ( LocalPlayer != NULL )
		{
			PlayerOwner = LocalPlayer->Actor;
		}
	}

	UBOOL bFoundRemoteEvents = FALSE;

	// "StringValue"라는 이름에 링크된 문자열들을 얻어온다.
	TArray<INT*> sendIntVars;
	TArray<INT*> recvIntVars;
	// 실제 1개의 값만 얻어오게 된다.
	GetIntVars(sendIntVars, TEXT("IntValue"));
	INT sendInt = ( sendIntVars.Num() > 0 ) ? *(sendIntVars(0)) : 0;

	// now find the level's sequence
	AWorldInfo* WorldInfo = GetWorldInfo();
	if ( WorldInfo != NULL )
	{
		USequence* GameSequence = WorldInfo->GetGameSequence();
		if ( GameSequence != NULL )
		{
			// now find the root sequence
			USequence* RootSequence = GameSequence->GetRootSequence();
			if ( RootSequence != NULL )
			{
				TArray<UavaSeqEvent_RemoteEventInt*> RemoteEvents;
				RootSequence->FindSeqObjectsByClass(UavaSeqEvent_RemoteEventInt::StaticClass(), (TArray<USequenceObject*>&)RemoteEvents);

				// now iterate through the list of events, activating those events that have a matching event tag
				for ( INT EventIndex = 0; EventIndex < RemoteEvents.Num(); EventIndex++ )
				{
					USeqEvent_RemoteEvent* RemoteEvt = RemoteEvents(EventIndex);
					if ( RemoteEvt != NULL && RemoteEvt->EventName == EventName && RemoteEvt->bEnabled == TRUE )
					{
						recvIntVars.Reset();
						RemoteEvt->GetIntVars(recvIntVars, TEXT("IntValue"));

						// 액션의 보낼 문자열을 받는 이벤트의 문자열로 복사.
						for( int j = 0; j < recvIntVars.Num(); j++ )
							*(recvIntVars(j)) = sendInt;

//						debugf(TEXT("Copy IntValue : [%d], numRecvs(%d)"), sendInt, recvIntVars.Num());

						// attempt to activate the remote event
						RemoteEvt->CheckActivate(WorldInfo, PlayerOwner);
						bFoundRemoteEvents = TRUE;
					}
				}
			}
		}
	}

	OutputLinks(bFoundRemoteEvents==TRUE ? 1 : 0).ActivateOutputLink();
}

/*! @brief
		UIAction_ActivateLevelEvent::Activated()에 Object변수 얻어서 설정하는 부분만 추가.
		(2007/01/30 고광록)
*/
void UavaUIAction_ActivateLevelEventObject::Activated()
{
	APlayerController* PlayerOwner = NULL;

	// assign the player that activated this event as the Instigator for the remote event
	UUIScreenObject* Owner = GetOwner();
	if ( Owner != NULL )
	{
		if ( !GEngine->GamePlayers.IsValidIndex(PlayerIndex) )
		{
			PlayerIndex = 0;
		}

		ULocalPlayer* LocalPlayer = Owner->GetPlayerOwner(PlayerIndex);
		if ( LocalPlayer != NULL )
		{
			PlayerOwner = LocalPlayer->Actor;
		}
	}

	UBOOL bFoundRemoteEvents = FALSE;

	// "StringValue"라는 이름에 링크된 문자열들을 얻어온다.
	TArray<UObject**> sendObjVars;
	TArray<UObject**> recvObjVars;
	// 실제 1개의 값만 얻어오게 된다.
	GetObjectVars(sendObjVars, TEXT("Object"));
	UObject* sendObject = ( sendObjVars.Num() > 0 ) ? *(sendObjVars(0)) : NULL;

	// now find the level's sequence
	AWorldInfo* WorldInfo = GetWorldInfo();
	if ( WorldInfo != NULL )
	{
		USequence* GameSequence = WorldInfo->GetGameSequence();
		if ( GameSequence != NULL )
		{
			// now find the root sequence
			USequence* RootSequence = GameSequence->GetRootSequence();
			if ( RootSequence != NULL )
			{
				TArray<UavaSeqEvent_RemoteEventObject*> RemoteEvents;
				RootSequence->FindSeqObjectsByClass(UavaSeqEvent_RemoteEventObject::StaticClass(), (TArray<USequenceObject*>&)RemoteEvents);

				// now iterate through the list of events, activating those events that have a matching event tag
				for ( INT EventIndex = 0; EventIndex < RemoteEvents.Num(); EventIndex++ )
				{
					USeqEvent_RemoteEvent* RemoteEvt = RemoteEvents(EventIndex);
					if ( RemoteEvt != NULL && RemoteEvt->EventName == EventName && RemoteEvt->bEnabled == TRUE )
					{
						recvObjVars.Reset();
						RemoteEvt->GetObjectVars(recvObjVars, TEXT("Object"));

						if( sendObject != NULL )
						{
							// 액션의 보낼 문자열을 받는 이벤트의 문자열로 복사.
							for( int j = 0; j < recvObjVars.Num(); j++ )
								*(recvObjVars(j)) = sendObject;

							//debugf(TEXT("Copy Object : [%s], numRecvs(%d)"), 
							//	   *(sendObject->GetName()), recvObjVars.Num());
						}

						// attempt to activate the remote event
						RemoteEvt->CheckActivate(WorldInfo, PlayerOwner);
						bFoundRemoteEvents = TRUE;
					}
				}
			}
		}
	}

	OutputLinks(bFoundRemoteEvents==TRUE ? 1 : 0).ActivateOutputLink();
}

void UavaUIAction_ActivateUIEvent::Activated()
{
	UClass* UIRemoteEventClass = UavaUIEvent_UIRemoteEvent::StaticClass();
	TArray<UUIEvent*> ActivatedEvents;
	TArray<UUIScreenObject*> ScreenObjectsToFind;

	UUIScreenObject* OwnerWidget = GetOwner();
	UUIScene* OwnerScene =  GetOwnerScene();
	if( OwnerWidget == NULL || OwnerScene == NULL)
	{
		OutputLinks(0).ActivateOutputLink();
		return;
	}

	// Find All ActiveScenes.
	if( bFindAllActiveScenes )
	{
		UGameUISceneClient* SceneClient = OwnerScene->GetSceneClient();
		if( SceneClient )
		{
			for(INT i = 0 ; i < SceneClient->ActiveScenes.Num() ; i++)
			{
				if( SceneClient->ActiveScenes(i) != NULL )
					GetEventsOfClassRecursive(UIRemoteEventClass, SceneClient->ActiveScenes(i), ActivatedEvents);
			}
		}
	}
	// Find itself and its ancestors
	else
	{
		if( OwnerScene != NULL )
			OwnerScene->FindEventsOfClass(UIRemoteEventClass, ActivatedEvents);
		for( UUIScreenObject* CurrentObject = OwnerWidget ; CurrentObject != NULL ; CurrentObject = CurrentObject->GetOwner() )
			CurrentObject->FindEventsOfClass(UIRemoteEventClass, ActivatedEvents);
	}

	UBOOL bSuccessTriggerEvent = FALSE;
	for ( INT EventIndex = 0; EventIndex < ActivatedEvents.Num(); EventIndex++ )
	{
		UavaUIEvent_UIRemoteEvent* Event = Cast<UavaUIEvent_UIRemoteEvent>(ActivatedEvents(EventIndex));
		if( Event == NULL || (Event->EventName != EventName && Event->EventName != *EventNameString) )
			continue;

		if( Event->ConditionalActivateUIEvent(INDEX_NONE , OwnerWidget, NULL /*InEventActivator*/, FALSE/*bActivateImmediately*/, NULL) )
			bSuccessTriggerEvent = TRUE;
	}

	OutputLinks( bSuccessTriggerEvent == TRUE ? 1 : 0 ).ActivateOutputLink();
}

void UavaUIAction_ActivateUIEvent::GetEventsOfClassRecursive( class UClass* EventClassToFind, UUIScreenObject* TargetObject, TArray<UUIEvent*>& outEventFound )
{
	if( TargetObject == NULL )
		return;

	TargetObject->FindEventsOfClass( EventClassToFind, outEventFound);
	for(INT i = 0 ; i < TargetObject->Children.Num() ; i++)
		GetEventsOfClassRecursive(EventClassToFind, TargetObject->Children(i), outEventFound );
}

/** 패키지 마이그레이션이 필요할때만 임시로 사용 */
void UavaUIAction_OpenSceneManaged::PostLoad()
{
	Super::PostLoad();
	//debugf(TEXT("PostLoad@avaUIAction_OpenSceneManaged : FullPath = %s, Scene = %s, SceneName = %s"), *GetPathName(), Scene ? *Scene->GetPathName() : GNone, *SceneName);

	//const FString PrevScenePrefixLower = TEXT("avauisceneex_");
	//const FString CurrScenePrefixLower = TEXT("avauiscenere_");

	//FString PathName = GetPathName();
	//
	//if( PathName.ToLower().InStr(CurrScenePrefixLower) != INDEX_NONE )
	//{
	//	TArray<UObject**> ObjVars;
	//	GetObjectVars(ObjVars,TEXT("Scene"));

	//	TArray<FString*> StrVars;
	//	GetStringVars(StrVars, TEXT("Scene Name"));

	//	UBOOL bModify = FALSE;
	//	if( SceneName.Trim().Len() > 0 && SceneName.ToLower().InStr(PrevScenePrefixLower) != INDEX_NONE )
	//	{
	//		SceneName = SceneName.Replace(*PrevScenePrefixLower, TEXT(""),TRUE);
	//		bModify = TRUE;
	//	}
	//	if( StrVars.Num() > 0 )
	//	{
	//		for( INT VarIndex = StrVars.Num() - 1 ; VarIndex >= 0 ; VarIndex-- )
	//		{
	//			if( StrVars(VarIndex) != NULL && StrVars(VarIndex)->ToLower().InStr(PrevScenePrefixLower) != INDEX_NONE )
	//			{
	//				*StrVars(VarIndex) = StrVars(VarIndex)->Replace(*PrevScenePrefixLower,TEXT(""),TRUE);
	//				bModify = TRUE;
	//			}
	//		}
	//	}

	//	UBOOL bSceneNameExist = (SceneName.Trim().Len() > 0 || StrVars.Num() > 0 );
	//	if( Scene != NULL && !bSceneNameExist )
	//	{
	//		SceneName = Scene->GetPathName().Replace(*PrevScenePrefixLower,TEXT(""),TRUE);
	//		if( ObjComment.InStr(TEXT("#PostLoad : ")) == INDEX_NONE )
	//			ObjComment += FString::Printf(TEXT(" #PostLoad : SceneName = %s"), *SceneName);
	//		bModify = TRUE;
	//	}

	//	bSceneNameExist = (SceneName.Trim().Len() > 0 || StrVars.Num() > 0 );
	//	for( INT VarIndex = ObjVars.Num() - 1 ; VarIndex >= 0 ; VarIndex-- )
	//	{
	//		UUIScene* UIScene = Cast<UUIScene>(*ObjVars(VarIndex));
	//		if( UIScene != NULL )
	//		{
	//			if( !bModify && !bSceneNameExist )
	//			{
	//				FString ObjPathName = UIScene->GetPathName();
	//				ObjPathName = ObjPathName.Replace(*PrevScenePrefixLower,TEXT(""),TRUE);
	//				SceneName = ObjPathName;
	//				if( ObjComment.InStr(TEXT("#PostLoad : ")) == INDEX_NONE )
	//					ObjComment += FString::Printf(TEXT(" #PostLoad : SceneName = %s"), *SceneName);

	//				bModify = TRUE;
	//			}
	//		}
	//	}

	//	bSceneNameExist = (SceneName.Trim().Len() > 0 || StrVars.Num() > 0 );
	//	if( ObjVars.Num() > 0 && bSceneNameExist )
	//	{
	//		for( INT LinkIndex = 0 ; LinkIndex < VariableLinks.Num() ; LinkIndex++ )
	//		{
	//			if( VariableLinks(LinkIndex).LinkDesc == TEXT("Scene") )
	//			{
	//				VariableLinks(LinkIndex).LinkedVariables.Empty();
	//				bModify = TRUE;
	//			}
	//		}
	//	}

	//	if( bModify )
	//		Modify();
	//}
}

void UavaUIAction_SetLocation::Activated()
{
	if( Targets.Num() == 0 )
	{
		warnf(TEXT("There's no target in UIAction %s"),*this->GetName());
		return;
	}

	FLOAT Left = 0.0f, Top = 0.0f, Right = 0.0f, Bottom = 0.0f;
	FLOAT *FloatRef;
	UBOOL bLeft = FALSE, bTop = FALSE, bRight = FALSE, bBottom = FALSE;

	for( INT VarLinksIndex = 0 ; VarLinksIndex < VariableLinks.Num() ; VarLinksIndex++)
	{
		FSeqVarLink& VarLink = VariableLinks(VarLinksIndex);
		for( INT LinkedVarIndex = 0 ; LinkedVarIndex < VarLink.LinkedVariables.Num() ; LinkedVarIndex++ )
		{
			if( VarLink.LinkedVariables(LinkedVarIndex) == NULL)
				continue;

			FloatRef = VarLink.LinkedVariables(LinkedVarIndex)->GetFloatRef();
			if( FloatRef == NULL )
				continue;

			if( VarLink.LinkDesc == TEXT("Left"))
			{
				bLeft = TRUE;
				Left = *FloatRef;
			}
			else if( VarLink.LinkDesc == TEXT("Top") )
			{
				bTop = TRUE;
				Top = *FloatRef;
			}
			else if ( VarLink.LinkDesc == TEXT("Right") )
			{
				bRight = TRUE;
				Right = *FloatRef;
			}
			else if ( VarLink.LinkDesc == TEXT("Bottom") )
			{
				bBottom = TRUE;
				Bottom = *FloatRef;
			}
		}
	}

	if( !bLeft && !bRight && !bTop && !bBottom )
		return;

	FVector2D ViewportSize;
	GetOwnerScene()->GetViewportSize( ViewportSize );

	for( INT i = 0 ; i < Targets.Num() ; i++)
	{
		UUIScreenObject* ScreenObject = Cast<UUIScreenObject>(Targets(i));
		if( ScreenObject == NULL )
			continue;

		FLOAT ViewportLeft = ScreenObject->Position.GetPositionValue( ScreenObject, UIFACE_Left, EVALPOS_PixelViewport);
		FLOAT ViewportRight = ScreenObject->Position.GetPositionValue( ScreenObject, UIFACE_Right, EVALPOS_PixelViewport);
		FLOAT ViewportTop = ScreenObject->Position.GetPositionValue( ScreenObject, UIFACE_Top, EVALPOS_PixelViewport );
		FLOAT ViewportBottom = ScreenObject->Position.GetPositionValue( ScreenObject, UIFACE_Bottom, EVALPOS_PixelViewport);

		if( bLeft && bRight && bTop	 && bBottom )
		{
			ScreenObject->SetPosition(Left, Top, Right, Bottom, EVALPOS_PixelViewport);
		}
		else
		{
			FLOAT ViewportWidth = ViewportRight - ViewportLeft;
			FLOAT ViewportHeight = ViewportBottom - ViewportTop;
			FLOAT BoundCheckValue;

			if( bLeft || bRight )
			{
				if( bLeft && !bRight )
					Right = !bFitToScreen ||  (0 <= ( BoundCheckValue = Left + ViewportWidth) && BoundCheckValue < ViewportSize.X) ? Left + ViewportWidth  : Left - ViewportWidth;
				else if ( !bLeft && bRight )
				Left = !bFitToScreen || (0 <= ( BoundCheckValue = Right - ViewportWidth) && BoundCheckValue < ViewportSize.X) ? Right - ViewportWidth : Right + ViewportWidth;

				if( Left > Right )
					appMemswap( &Left, &Right, sizeof(Left) );

				ScreenObject->SetPosition(Left + LocationOffset.X, UIFACE_Left, EVALPOS_PixelViewport);
				ScreenObject->SetPosition(Right + LocationOffset.X, UIFACE_Right, EVALPOS_PixelViewport);
			}
			if( bTop || bBottom )
			{
				if( bTop && !bBottom)
					Bottom = !bFitToScreen || ( 0 <= (BoundCheckValue = Top + ViewportHeight) && BoundCheckValue < ViewportSize.Y ) ? Top + ViewportHeight : Top - ViewportHeight;
				else if ( !bTop && bBottom )
					Top = !bFitToScreen || ( 0 <= (BoundCheckValue = Bottom - ViewportHeight) && BoundCheckValue < ViewportSize.Y ) ? Bottom - ViewportHeight : Bottom + ViewportHeight;

				if( Top > Bottom )
					appMemswap( &Top, &Bottom, sizeof(FLOAT) );

				ScreenObject->SetPosition(Top + LocationOffset.Y, UIFACE_Top, EVALPOS_PixelViewport);
				ScreenObject->SetPosition(Bottom + LocationOffset.Y, UIFACE_Bottom, EVALPOS_PixelViewport);
			}
		}
	}
}

void UavaUIAction_RemoveListItem::Activated()
{
	Super::Activated();

	UBOOL bSuccess = FALSE;

	// find the list that owns this action
	UUIList* TargetList = NULL;
	for ( INT TargetIndex = 0; TargetIndex < Targets.Num(); TargetIndex++ )
	{
		TargetList = Cast<UUIList>(Targets(TargetIndex));
		if ( TargetList != NULL )
		{
			break;
		}
	}

	if ( TargetList != NULL )
	{
		INT IndexToMove = ElementIndex;

		// if no index was specified, use the list's currently selected item
		if ( IndexToMove == INDEX_NONE )
		{
			IndexToMove = TargetList->Index;
		}
		bSuccess = TargetList->RemoveElementAtIndex(IndexToMove);
	}

	if ( bSuccess == TRUE )
	{
		OutputLinks(1).ActivateOutputLink();
	}
	else
	{
		OutputLinks(0).ActivateOutputLink();
	}
}

//! List에서 해당 CellFieldName과 CellStringValue와 같은 Cell을 선택되어지도록 한다.
void UavaUIAction_SetListIndexFromCellValue::Activated()
{
	Super::Activated();

	UBOOL bSuccess = FALSE;

	if ( CellFieldName != NAME_None )
	{
		TArray<UUIButton*> ButtonsCanBeChecked;
		UUIList* TargetList = NULL;
		for ( INT TargetIndex = 0; TargetIndex < Targets.Num(); TargetIndex++ )
		{
			TargetList = Cast<UUIList>(Targets(TargetIndex));
			if ( TargetList != NULL )
				break;

			UUIButton* Button = Cast<UavaUICheckLabelButton>(Targets(TargetIndex));
			if( Button != NULL )
				ButtonsCanBeChecked.AddUniqueItem(Button);
		}

		if ( TargetList != NULL && TargetList->DataProvider )
		{
			UBOOL bMultiSelection = TargetList->bEnableMultiSelect;
			for ( INT i = 0; i < TargetList->Items.Num(); i++)
			{
				const INT Index = TargetList->Items(i);

				TScriptInterface<IUIListElementCellProvider> CellProvider = TargetList->DataProvider->GetElementCellValueProvider(TargetList->DataSource.DataStoreField, Index);
				if ( CellProvider )
				{
					FUIProviderFieldValue CellValue(EC_EventParm);
					if ( CellProvider->GetCellFieldValue(CellFieldName, Index, CellValue) == TRUE )
					{
						// 같은 Cell String Value를 가지고 있다면.
						if ( CellStringValue == CellValue.StringValue )
						{
							if( bMultiSelection )
							{
								TargetList->SelectElement(i, TRUE );
								ItemIndex = i;
								ListIndex = Index;
							}
							else
							{
								// 내부적으로 (MaxVisibleItems == 0)라서 TopIndex값이 i값으로 설정되면서
								// 생기는 버그였다.
								TargetList->SetIndex(i);
								if ( TopIndex != INDEX_NONE )
									TargetList->SetTopIndex( TopIndex );

								ItemIndex = i;
								ListIndex = Index;
								bSuccess = TRUE;
								break;
							}

							// 왠지 아래처럼 직접 상태 바꿔주고 값을 넣어주면 잘 된다.(불안할까??)
							//if ( TargetList->Index != INDEX_NONE )
							//	TargetList->SelectElement(TargetList->Index, FALSE);
							//TargetList->SelectElement(i, TRUE);
							//TargetList->Index = i;
						}
					}
				}
			}
		}
		else if ( ButtonsCanBeChecked.Num() > 0 )
		{
			for( INT ButtonIndex = 0 ; ButtonIndex < ButtonsCanBeChecked.Num() ; ButtonIndex++ )
			{
				UavaUICheckLabelButton* CLButton = Cast<UavaUICheckLabelButton>(ButtonsCanBeChecked(ButtonIndex));
				if( CLButton )
				{
					debugf(TEXT("Compare %s == %s"), *CLButton->eventGetCaption(), *CellStringValue );
					CLButton->SetValue( CLButton->eventGetCaption() == CellStringValue );
				}
			}
		}
	}

	if ( bSuccess == TRUE )
	{
		OutputLinks(0).ActivateOutputLink();
	}
	else
	{
		OutputLinks(1).ActivateOutputLink();
	}
}

void UavaUIAction_SetEnabled::Activated()
{
	BOOL	bResult = FALSE;
	Super::Activated();

	if ( !bCallHandler )
	{
		for ( INT TargetIndex = 0; TargetIndex < Targets.Num(); TargetIndex++ )
		{
			UUIScreenObject* Widget = Cast<UUIScreenObject>(Targets(TargetIndex));
			if ( Widget != NULL )
			{
				bResult = Widget->SetEnabled( bEnabled );
			}
		}
	}

	// now activate the appropriate output link
	if ( bResult == TRUE )
	{
		OutputLinks(1).ActivateOutputLink();
	}
	else
	{
		OutputLinks(0).ActivateOutputLink();
	}
}

static FString Replace(const FString & Replace,const FString& Match, const FString& Target)
{
	FString Str,Temp;
	INT Offset;
	Temp = Target;

	Offset = Temp.InStr(Match);
	if (Offset != INDEX_NONE)
	{
		Str = Temp.Left(Offset);
		Str += Replace;
		Str += Temp.Right(Temp.Len() - Offset - Match.Len());
	}
	else
		Str = Target;

	return Str;
}

void UavaSeqAct_Sprintf::Activated()
{
	FString SourceStr;
	USeqVar_String* Str = NULL;
	USeqVar_String* TargetStr = NULL;
	UBOOL bHasSource = FALSE;
	UBOOL bHasTarget = FALSE;

	for( INT VarLinksIndex = 0 ; VarLinksIndex < VariableLinks.Num() ; VarLinksIndex++ )
	{
		FSeqVarLink& VarLink = VariableLinks(VarLinksIndex);
		for( INT LinkedVarIndex = 0 ; LinkedVarIndex < VarLink.LinkedVariables.Num() ; LinkedVarIndex++ )
		{
			if( VarLink.LinkDesc == TEXT("SourceStr"))
			{
				Str = Cast<USeqVar_String>(VarLink.LinkedVariables(LinkedVarIndex));
				if( Str )			
				{
					SourceStr = Str->StrValue;
					bHasSource = TRUE;
				}
			}
			if( VarLink.LinkDesc == TEXT("TargetStr"))
			{
				TargetStr = Cast<USeqVar_String>(VarLink.LinkedVariables(LinkedVarIndex));
				if( TargetStr )			
				{
					bHasTarget = TRUE;
				}
			}
		}
	}

	if( bHasSource == FALSE || bHasTarget == FALSE)
		return;

	for( INT VarLinksIndex = 0 ; VarLinksIndex < VariableLinks.Num() ; VarLinksIndex++ )
	{
		FSeqVarLink& VarLink = VariableLinks(VarLinksIndex);
		for( INT LinkedVarIndex = 0 ; LinkedVarIndex < VarLink.LinkedVariables.Num() ; LinkedVarIndex++ )
		{
			if( VarLink.LinkDesc == TEXT("Str"))
			{
				USeqVar_String* SeqVarStr = Cast<USeqVar_String>(VarLink.LinkedVariables(LinkedVarIndex));
				if( SeqVarStr )
					SourceStr = Replace( SeqVarStr->StrValue, FString(TEXT("%s")), SourceStr);
//					SourceStr = SourceStr.Replace(TEXT("%s"), *SeqVarStr->StrValue);
			}
			else if ( VarLink.LinkDesc == TEXT("Float") )
			{
				USeqVar_Float* SeqVarFloat = Cast<USeqVar_Float>(VarLink.LinkedVariables(LinkedVarIndex));
				if( SeqVarFloat )
					SourceStr = FString::Printf(*SourceStr, SeqVarFloat->FloatValue);
					//SourceStr = Replace( FString::Printf(TEXT("%.4f"),SeqVarFloat->FloatValue) , FString(TEXT("%f")), SourceStr);
//					SourceStr = SourceStr.Replace(TEXT("%f"),  *FString::Printf(TEXT("%.4f"),SeqVarFloat->FloatValue) );
			}
			else if ( VarLink.LinkDesc == TEXT("Int") )
			{
				USeqVar_Int* SeqVarInt = Cast<USeqVar_Int>(VarLink.LinkedVariables(LinkedVarIndex));
				if( SeqVarInt )
					SourceStr = Replace( appItoa(SeqVarInt->IntValue), FString(TEXT("%d")), SourceStr);
//					SourceStr = SourceStr.Replace(TEXT("%d"), *appItoa(SeqVarInt->IntValue) );
			}
		}
	}

	TargetStr->StrValue = SourceStr;
}

void UavaSeqAct_ParseIntoArray::Activated()
{
	TArray<FString> ParsedStrings;
	SrcStr.ParseIntoArray( &ParsedStrings, *Delimeter, FALSE);
	for( INT i = 0 ; i < Min(StrParmCount,ParsedStrings.Num()) ; i++ )
	{
		StrParm[i] = ParsedStrings(i);
		TArray<FString*> StrVars;
		GetStringVars( StrVars, *(FString(TEXT("Parm")) + appItoa(i+1)) );
		for( INT VarIndex = 0 ; VarIndex < StrVars.Num() ; VarIndex++ )
			*StrVars(VarIndex) = StrParm[i];
	}

}

void UavaSeqAct_TrimStr::Activated()
{
	TargetStr = SourceStr;
	if( bTrim )
		TargetStr = TargetStr.Trim();

	if( bTrimTrailing )
		TargetStr = TargetStr.TrimTrailing();
}

void UavaSeqCond_HasAudioDevice::Activated()
{
	UBOOL bHasAudioDevice = GEngine && GEngine->Client && GEngine->Client->GetAudioDevice();
	OutputLinks( bHasAudioDevice ? 0 : 1 ).ActivateOutputLink();
}


/************************************************************************/
/* UavaSeqCond_SwitchObject
/************************************************************************/
void UavaSeqCond_SwitchObject::Activated()
{
	Super::Activated();

	INT MatchIndex = -1;

	if ( Compare != NULL )
	{
		for( INT i = 0 ; i < CompareList.Num() - 1 ; i++)
		{
			if( CompareList(i) == Compare )
			{
				MatchIndex = i;
				break;
			}
		}

		debugf(TEXT("avaSeqCond_SwitchObject::Activated() - %s MatchIndex[%d]"), *Compare->GetName(), MatchIndex);
	}

	OutputLinks( MatchIndex >= 0 ? MatchIndex : OutputLinks.Num() - 1).ActivateOutputLink();

	// 선택된 Index를 넣어준다.
	TArray<INT*> IntVars;
	GetIntVars(IntVars,TEXT("SelectedIndex"));

	for (INT Idx = 0; Idx < IntVars.Num(); Idx++)
		*(IntVars(Idx)) = MatchIndex;
}

void UavaSeqCond_SwitchObject::UpdateDynamicLinks()
{
	Super::UpdateDynamicLinks();

	// If there are too many output Links
	if( OutputLinks.Num() > CompareList.Num() )
	{
		// Search through each output description and see if the name matches the
		// remaining class names.
		for( INT i = OutputLinks.Num() - 1; i >= 0; i-- )
		{
			INT j = INDEX_NONE;
			for( j = CompareList.Num() - 1; j >= 0; j-- )
				if( OutputLinks(i).LinkDesc == CompareList(j)->GetName())
					break;

			// If no match was found, remove the obsolete output Link
			if( j < 0 )
			{
				OutputLinks(i).Links.Empty();
				OutputLinks.Remove( i );
			}
		}
	}

	// If there aren't enough output Links, add some
	while( OutputLinks.Num() < CompareList.Num() )
	{
		INT Num	= Max<INT>( OutputLinks.Num() - 1, 0 );
		OutputLinks.InsertZeroed( Num );
	}

	// 만약 OutputLinks개수가 CompareList보다 크면... 줄여준다.
	while( OutputLinks.Num() > CompareList.Num() )
		OutputLinks.Remove(OutputLinks.Num() - 1);

	if ( OutputLinks.Num() > 0 )
	{
		INT Num = OutputLinks.Num() - 1;
		if( OutputLinks(Num).LinkDesc != TEXT("None") )
		{
			OutputLinks.AddZeroed();
			CompareList.AddZeroed();
		}

		// Ensure last entry is always default
		Num = OutputLinks.Num() - 1;
		OutputLinks(Num).LinkDesc = TEXT("None");
		CompareList(Num) = NULL;
	}
	else
	{
		OutputLinks.AddZeroed();
		CompareList.AddZeroed();

		OutputLinks(OutputLinks.Num() - 1).LinkDesc = TEXT("None");
	}

	// Match up descriptions with each class entry
	for( INT i = 0; i < OutputLinks.Num(); i++ )
	{
		FString PathName = CompareList(i)->GetPathName();

		OutputLinks(i).LinkDesc = CompareList(i)->GetName();
		debugf(TEXT("SwitchObject - Name[%s]-PathName[%s]"), *CompareList(i)->GetName(), *PathName);
	}
}

FColor UavaSeqCond_SwitchObject::GetConnectionColor( INT ConnType, INT ConnIndex, INT MouseOverConnType, INT MouseOverConnIndex )
{
	return Super::GetConnectionColor( ConnType, ConnIndex, MouseOverConnType, MouseOverConnIndex );
}

/************************************************************************/
/* UavaSeqCond_SwitchString		                                        */
/************************************************************************/

void UavaSeqCond_SwitchString::Activated()
{
	Super::Activated();

	INT MatchIndex = -1;
	for( INT i = 0 ; i < CompareList.Num() - 1 ; i++)
	{
		if( bIgnoreCase ? appStricmp(*CompareList(i), *StrToCmp) == 0 : appStrcmp(*CompareList(i), *StrToCmp) == 0 )
			MatchIndex = i;
	}

	OutputLinks( MatchIndex >= 0 ? MatchIndex : OutputLinks.Num() - 1).ActivateOutputLink();
}

void UavaSeqCond_SwitchString::UpdateDynamicLinks()
{
	Super::UpdateDynamicLinks();

	// If there are too many output Links
	if( OutputLinks.Num() > CompareList.Num() )
	{
		// Search through each output description and see if the name matches the
		// remaining class names.
		for( INT i = OutputLinks.Num() - 1; i >= 0; i-- )
		{
			INT j = INDEX_NONE;
			for( j = CompareList.Num() - 1; j >= 0; j-- )
				if( OutputLinks(i).LinkDesc == CompareList(j))
					break;

			// If no match was found, remove the obsolete output Link
			if( j < 0 )
			{
				OutputLinks(i).Links.Empty();
				OutputLinks.Remove( i );
			}
		}
	}

	// If there aren't enough output Links, add some
	while( OutputLinks.Num() < CompareList.Num() )
	{
		INT Num	= Max<INT>( OutputLinks.Num() - 1, 0 );
		OutputLinks.InsertZeroed( Num );
	}

	INT Num = OutputLinks.Num() - 1;
	if( OutputLinks(Num).LinkDesc != TEXT("Default") )
	{
		OutputLinks.AddZeroed();
		CompareList.AddZeroed();
	}

	// Ensure last entry is always default
	Num = OutputLinks.Num() - 1;
	OutputLinks(Num).LinkDesc = TEXT("Default");
	CompareList(Num) = TEXT("Default");

	// Match up descriptions with each class entry
	for( INT i = 0; i < OutputLinks.Num(); i++ )
		OutputLinks(i).LinkDesc = CompareList(i);
}

FColor UavaSeqCond_SwitchString::GetConnectionColor( INT ConnType, INT ConnIndex, INT MouseOverConnType, INT MouseOverConnIndex )
{
	return Super::GetConnectionColor( ConnType, ConnIndex, MouseOverConnType, MouseOverConnIndex );
}

void UavaUIAction_FakeFullScreen::SetFakeFullScreen(UBOOL bEnable)
{
	extern UBOOL GFakeFullScreen;
	extern UBOOL GPendingRecreateDevice;

	if ( bEnable )
	{
		// 이미 FakeFullScreen이면 무시.
		if ( GFakeFullScreen )
			return;

		GFakeFullScreen = TRUE;
		GPendingRecreateDevice = TRUE;
	}
	else
	{
		// 이미 FullScreen이면 무시.
		if ( !GFakeFullScreen )
			return;

		GFakeFullScreen = FALSE;
		GPendingRecreateDevice = TRUE;
	}
}

void UavaUIAction_SetFocusMode::SetFocusMode(UBOOL bLock)
{
	extern HWND GD3DDeviceWindow;
	HWND hWnd = GD3DDeviceWindow;

	if ( hWnd == NULL )
		return ;

	//! FullScreen <-> FakeFullScreen 사이로 전환 되기 때문에 포커스를 잠궈둘 필요가 없다.
	if ( bLock )
	{
		::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
	else
	{
		::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		::SetForegroundWindow(hWnd);
	}
}

void UavaUIEvent_CheckLabelValueChanged::InitializeLinkedVariableValues()
{
	Super::InitializeLinkedVariableValues();

	// for now, it's assumed that only checkboxes activate this event...if this ever changes, this function needs to be updated
	// so assert if this is not the case
	UavaUICheckLabelButton* CheckLabelButtonTarget = CastChecked<UavaUICheckLabelButton>(EventActivator);

	// copy the current value of the checkbox into the "New Value" variable link
	TArray<UBOOL*> BoolVars;
	GetBoolVars(BoolVars,TEXT("Value"));

	for (INT Idx = 0; Idx < BoolVars.Num(); Idx++)
	{
		*(BoolVars(Idx)) = CheckLabelButtonTarget->bIsChecked;
	}
}

/************************************************************************/
/* avaUIAction_TransitionBase		                                    */
/************************************************************************/
void UavaUIAction_TransitionBase::GetUIDrawComponents(class UUIObject* TargetObj,TArray<class UUIComp_DrawComponents*>& OutComps,UBOOL bRecursive/*=FALSE*/)
{
	check(TargetObj != NULL);
	TArray<UUIComponent*> UIComps;
	TargetObj->GetUIComponents(UIComps);
	for(INT i = 0 ; i < UIComps.Num() ; i++)
		if( Cast<UUIComp_DrawComponents>(UIComps(i)) != NULL )
			OutComps.AddItem(Cast<UUIComp_DrawComponents>(UIComps(i)) );
}

FLOAT UavaUIAction_TransitionBase::GetStableDeltaTime( FLOAT DeltaTime )
{
	check(WorkingDelta.Num() > 0);
	check(WorkingDelta.IsValidIndex(QueueIndex));
	static FLOAT UpperBound = 0.05f;

	INT Sum = 0;
	FLOAT StableDelta = Min(DeltaTime,UpperBound);

	if( DeltaTime < UpperBound )
	{
		WorkingDelta(QueueIndex) = appTrunc(DeltaTime * 1000.f);
		QueueIndex = (QueueIndex + 1) % WorkingDelta.Num();

		for( INT i = 0 ; i < WorkingDelta.Num() ; i++ )
		{
			INT EachDelta = WorkingDelta(i);
			if( EachDelta > 0 )
				Sum += EachDelta;
		}
		StableDelta = (Sum / WorkingDelta.Num()) * 0.001f;
	}

	return StableDelta;
}

/************************************************************************/
/* avaUIAction_TransOpacity			                                    */
/************************************************************************/

void UavaUIAction_TransOpacity::Activated()
{
	TimeElapsed = 0.f;

	// bDrawComp일때는 여기서 처리
	if( bDrawComp && OpacityCurve && OpacityDuration > 0.f)
	{
		for( INT i = 0 ; i < Targets.Num() ; i++ )
		{
			UUIObject* Obj = Cast<UUIObject>(Targets(i));
			if( Obj == NULL )
				continue;

			TArray<UUIComp_DrawComponents*> OutComps;
			GetUIDrawComponents(Obj,OutComps);

			for( INT CompIndex = 0 ; CompIndex < OutComps.Num() ; CompIndex++ )
				OutComps(CompIndex)->Fade( EFT_Fading, OpacityCurve, OpacityDuration);
		}
	}
}

UBOOL UavaUIAction_TransOpacity::UpdateOp(FLOAT deltaTime)
{
	if( OpacityCurve == NULL )
	{
		OutputLinks(1).ActivateOutputLink();
		return TRUE;
	}

	if( TimeElapsed >= OpacityDuration )
	{
		// 혹시 모르니 Fade In/Out  후에 100%상태로 Opacity를 적용해 준다.
		for( INT i = 0 ; i < Targets.Num() ; i++ )
		{
			UUIScreenObject* ScreenObj = Cast<UUIScreenObject>(Targets(i));
			if( ScreenObj == NULL )
				continue;

			ScreenObj->Opacity = OpacityCurve->EvalRatio(1.0f);
		}

		OutputLinks(0).ActivateOutputLink();
		return TRUE;
	}

	FLOAT StableDelta = GetStableDeltaTime(deltaTime);
	TimeElapsed += StableDelta;

	if( bDrawComp )
	{
		return FALSE;
	}

	for( INT i = 0 ; i < Targets.Num() ; i++ )
	{
		UUIScreenObject* ScreenObj = Cast<UUIScreenObject>(Targets(i));
		if( ScreenObj == NULL )
			continue;

		FLOAT Ratio = Clamp(TimeElapsed / OpacityDuration,0.f,1.f);
		ScreenObj->Opacity = OpacityCurve->EvalRatio(Ratio);
	}

	return FALSE;
}

/************************************************************************/
/* avaUIAction_TransPosition		                                    */
/************************************************************************/

void UavaUIAction_TransPosition::Activated()
{
	TimeElapsed = 0.f;

	for( INT i = 0 ; i < Targets.Num() ; i++ )
	{
		FTransPosInfo PosInfo;
		UUIScreenObject* ScreenObj = Cast<UUIScreenObject>(Targets(i));
		if( ScreenObj == NULL )
			continue;

		PosInfo.ScreenObj = ScreenObj;
		for( INT FaceIndex = 0 ; FaceIndex < UIFACE_MAX ; FaceIndex++ )
			PosInfo.InitRawPos[FaceIndex] = ScreenObj->GetPosition(FaceIndex, EVALPOS_PixelViewport );

		TransPosData.AddItem(PosInfo);
	}
}

UBOOL UavaUIAction_TransPosition::UpdateOp(FLOAT deltaTime)
{
	if( TimeElapsed >= PositionDuration )
	{
		OutputLinks(0).ActivateOutputLink();
		return TRUE;
	}

	if( PositionCurve == NULL || PositionTargetObject == NULL)
	{
		OutputLinks(1).ActivateOutputLink();
		return TRUE;
	}
	TimeElapsed += deltaTime;

	FLOAT Ratio = PositionCurve->EvalRatio(TimeElapsed / PositionDuration);

	for ( INT i = 0 ; i < TransPosData.Num() ; i++ )
	{
		FTransPosInfo& PosInfo = TransPosData(i);
		check(PosInfo.ScreenObj != NULL );

		FLOAT NewRawPos[UIFACE_MAX];
		for ( INT i = 0 ; i < UIFACE_MAX ; i++ )
			NewRawPos[i] = Lerp( PosInfo.InitRawPos[i], PositionTargetObject->GetPosition(i, EVALPOS_PixelViewport) , Ratio );

		PosInfo.ScreenObj->SetPosition(NewRawPos[UIFACE_Left], NewRawPos[UIFACE_Top], NewRawPos[UIFACE_Right], NewRawPos[UIFACE_Bottom], EVALPOS_PixelViewport);
	}

	return FALSE;
}

/************************************************************************/
/* avaUIAction_SetTransScale		                                    */
/************************************************************************/

void UavaUIAction_TransScale::Activated()
{
	TimeElapsed = 0.f;
	for( INT i = 0 ; i < Targets.Num() ; i++ )
	{
		FTransScaleInfo ScaleInfo;
		UUIScreenObject* ScreenObj = Cast<UUIScreenObject>(Targets(i));
		if( ScreenObj == NULL )
			continue;
		
		ScaleInfo.ScreenObj = ScreenObj;
		for( INT OrientIndex = 0 ; OrientIndex < UIORIENT_MAX ; OrientIndex++ )
			ScaleInfo.InitBound[OrientIndex] = ScreenObj->GetBounds(OrientIndex, EVALPOS_PixelViewport );

		TransScaleData.AddItem(ScaleInfo);
	}

	if( bDrawComp && ScaleCurveHorz && ScaleCurveVert && ScaleDuration > 0.f )
	{
		for( INT i = 0 ; i < Targets.Num() ; i++ )
		{
			UUIObject* Obj = Cast<UUIObject>(Targets(i));
			if( Obj == NULL )
				continue;

			TArray<UUIComp_DrawComponents*> OutComps;
			GetUIDrawComponents(Obj,OutComps);

			for( INT CompIndex = 0 ; CompIndex < OutComps.Num() ; CompIndex++ )
				OutComps(CompIndex)->ScaleRender(ScaleCurveHorz, ScaleCurveVert, ScaleAxisHorz, ScaleAxisVert ,ScaleDuration);
		}
	}
}

UBOOL UavaUIAction_TransScale::UpdateOp(FLOAT deltaTime)
{
	if( TimeElapsed >= ScaleDuration )
	{
		OutputLinks(0).ActivateOutputLink();
		return TRUE;
	}

	if( ScaleCurveVert == NULL && ScaleCurveHorz == NULL )
	{
		OutputLinks(1).ActivateOutputLink();
		return TRUE;
	}

	if( bDrawComp )
		return FALSE;

	TimeElapsed += deltaTime;

	FLOAT Ratio = TimeElapsed / ScaleDuration;
	FLOAT ScaleX = ScaleCurveHorz ? ScaleCurveHorz->EvalRatio(Ratio) : 1.f;
	FLOAT ScaleY = ScaleCurveVert ? ScaleCurveVert->EvalRatio(Ratio) : 1.f;

	for( INT i = 0 ; i < TransScaleData.Num() ; i++ )
	{
		FTransScaleInfo& ScaleInfo = TransScaleData(i);

		FLOAT NewRawPosition[UIFACE_MAX];
		for( INT FaceIndex = 0 ; FaceIndex < UIFACE_MAX ; FaceIndex++ )
			NewRawPosition[FaceIndex] = ScaleInfo.ScreenObj->GetPosition(FaceIndex, EVALPOS_PixelViewport);

		FLOAT Width = (NewRawPosition[UIFACE_Right] - NewRawPosition[UIFACE_Left]);
		FLOAT Height = (NewRawPosition[UIFACE_Bottom] - NewRawPosition[UIFACE_Top]);
		FLOAT CenterPosX = NewRawPosition[UIFACE_Left] + Width * ScaleAxisHorz;
		FLOAT CenterPosY = NewRawPosition[UIFACE_Top] + Height * ScaleAxisVert;
		FLOAT NewPosAtZero[UIFACE_MAX];
		NewPosAtZero[UIFACE_Left] = - (Clamp(ScaleAxisHorz,0.f,1.f) * ScaleInfo.InitBound[UIORIENT_Horizontal] ) * ScaleX;
		NewPosAtZero[UIFACE_Top] = - (Clamp(ScaleAxisVert,0.f,1.f) * ScaleInfo.InitBound[UIORIENT_Vertical] ) * ScaleY;
		NewPosAtZero[UIFACE_Right] = (1.f - Clamp(ScaleAxisHorz,0.f,1.f) ) * ScaleInfo.InitBound[UIORIENT_Horizontal] * ScaleX;
		NewPosAtZero[UIFACE_Bottom] = (1.f - Clamp(ScaleAxisVert,0.f,1.f) ) * ScaleInfo.InitBound[UIORIENT_Vertical] * ScaleY;

		NewRawPosition[UIFACE_Left] = NewPosAtZero[UIFACE_Left] + CenterPosX;
		NewRawPosition[UIFACE_Top] = NewPosAtZero[UIFACE_Top] + CenterPosY;
		NewRawPosition[UIFACE_Right] = NewPosAtZero[UIFACE_Right] + CenterPosX;
		NewRawPosition[UIFACE_Bottom] = NewPosAtZero[UIFACE_Bottom] + CenterPosY;

		ScaleInfo.ScreenObj->SetPosition(NewRawPosition[UIFACE_Left], NewRawPosition[UIFACE_Top], NewRawPosition[UIFACE_Right], NewRawPosition[UIFACE_Bottom]);
	}
	return FALSE;
}

/************************************************************************/
/* UavaUIAction_RefreshBindingValue	                                    */
/************************************************************************/

void UavaUIAction_RefreshBindingValue::Activated()
{
	for ( INT TargetIndex = 0; TargetIndex < Targets.Num(); TargetIndex++ )
	{
		UUIScreenObject* Widget = Cast<UUIScreenObject>(Targets(TargetIndex));
		UUIList* ListWidget = Cast<UUIList>(Widget);

		if ( ListWidget != NULL )
		{
			UBOOL bPreserveList = Option == ERBV_List_PreserveIndex;
			UBOOL bSelectNewItem = Option == ERBV_List_SelectNewItem;

			// (Pre) Preserve ListIndex
			INT DiffItemCount = 0;						// 선택된 아이템과 TopIndex 사이의 아이템 갯수
			INT CurrentSelectedItem = INDEX_NONE;		// 현재 선택된 아이템 인덱스 (CollectionIndex)
			INT CurrentSelectedIndex = INDEX_NONE;		// 현재 선택된 리스트 인덱스 (위에서부터의 순서)
			FString CurrentCellValue;					// 현재 선택된 ID셀의 내용
			INT LastTopIndex = ListWidget->TopIndex;	// 현재 리스트의 맨 위 항목이 가리키는 리스트 인덱스

			// UIEvent가 발생유무를 설정한다.(리스트 선택 후 갱신이 필요해서...)
			ListWidget->bAlwaysSkipNotification = bSkipNotification;

			if( bPreserveList || bSelectNewItem )
			{
				CurrentSelectedItem = ListWidget->GetCurrentItem();
				if( CurrentSelectedItem != INDEX_NONE )
				{
					CurrentSelectedIndex = ListWidget->Items.FindItemIndex( CurrentSelectedItem );
					if( CurrentSelectedIndex != INDEX_NONE )
					{
						// 어쨌든 선택된 항목이 있다.
						DiffItemCount = CurrentSelectedIndex - ListWidget->TopIndex;

						if( ListWidget->CellDataComponent && IDCellTag != NAME_None )
						{
							// 그중에서 ID Cell의 내용을 찾는다.
							FUIElementCellSchema& Schema = ListWidget->CellDataComponent->ElementSchema;
							INT FindCellIndex = INDEX_NONE;
							// ID CellTag가 존재한다.
							for( INT CellIndex = 0 ; CellIndex < Schema.Cells.Num() ; CellIndex++ )
							{
								if( IDCellTag == Schema.Cells(CellIndex).CellDataField )
								{
									FindCellIndex = CellIndex;
									break;
								}
							}
							if( FindCellIndex != INDEX_NONE )
							{
								FUIListElementCell& CurrentCell = ListWidget->CellDataComponent->ListItems( CurrentSelectedIndex ).Cells(FindCellIndex);
								CurrentCellValue = CurrentCell.ValueString ? CurrentCell.ValueString->GetValue() : TEXT("");
							}
						}
						// ID Cell을 찾았음
					}
				}
			}

			// (Pre) Select New Item
			TArray<INT> CurrentItems;
			if( bSelectNewItem )
				CurrentItems = ListWidget->Items;

			// if this child implements the UIDataStoreSubscriber interface, tell the child to load the value from the data store
			IUIDataStoreSubscriber* SubscriberChild = (IUIDataStoreSubscriber*)Widget->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass());
			if ( SubscriberChild != NULL )
			{
				SubscriberChild->RefreshSubscriberValue(BindingIndex);
			}

			// (Post) Select NewItem
			UBOOL bSelectNewItem_ItemFound = FALSE;
			if( bSelectNewItem )
			{
				for( INT i = ListWidget->Items.Num() - 1 ; i >= 0 ; i-- )
				{
					if( CurrentItems.FindItemIndex( ListWidget->Items(i) ) == INDEX_NONE )
					{
						INT NewIndex = i;
						ListWidget->SetIndex( NewIndex, TRUE );
						ListWidget->SetTopIndex( NewIndex - ListWidget->MaxVisibleItems/2, TRUE );
						bSelectNewItem_ItemFound = TRUE;
						break;
					}
				}
			}

			// (Post) Preserve ListIndex
			if( bPreserveList || !bSelectNewItem_ItemFound )
			{
				//	선택된 항목이 있을때 : 선택된 항목을 유지한다 ( = 해당 리스트 아이템을 따라간다. )
				//	선택된 항목이 없을때 : 뷰를 유지한다.( = 리스트 탑인덱스를 유지한다 )
				UBOOL bHasSelection = CurrentSelectedItem != INDEX_NONE && CurrentSelectedIndex != INDEX_NONE;
				if( bHasSelection )
				{
					INT NewIndex = ListWidget->Items.FindItemIndex(CurrentSelectedItem);

					if( CurrentCellValue.Len() > 0 && ListWidget->CellDataComponent)
					{
						FUIElementCellSchema& Schema = ListWidget->CellDataComponent->ElementSchema;
						INT FindCellIndex = INDEX_NONE;

						// ID CellTag가 존재한다.
						for( INT CellIndex = 0 ; CellIndex < Schema.Cells.Num() ; CellIndex++ )
						{
							if( IDCellTag == Schema.Cells(CellIndex).CellDataField )
							{
								FindCellIndex = CellIndex;
								break;
							}
						}

						UBOOL bFoundMatchCell = FALSE;

						if( FindCellIndex != INDEX_NONE )
						{
							for ( INT ElementIndex = 0 ; ElementIndex < ListWidget->CellDataComponent->ListItems.Num() ; ElementIndex++)
							{
								FUIListItem& ListItem = ListWidget->CellDataComponent->ListItems( ElementIndex );
								FUIListElementCell& CurrentCell =  ListItem.Cells(FindCellIndex);
								if( CurrentCell.ValueString != NULL && CurrentCell.ValueString->GetValue() == CurrentCellValue )
								{
									ListWidget->SetIndex( ElementIndex, TRUE);
									ListWidget->SetTopIndex( ElementIndex - DiffItemCount, TRUE );
									bFoundMatchCell = TRUE;
									break;
								}
							}
						}

						if( !bFoundMatchCell)
						{
							ListWidget->SetIndex( NewIndex != INDEX_NONE ? NewIndex : CurrentSelectedIndex + 1, TRUE );
							ListWidget->SetTopIndex( NewIndex != INDEX_NONE ? NewIndex - DiffItemCount : CurrentSelectedIndex - DiffItemCount, TRUE );
						}

					}
					else
					{
						ListWidget->SetIndex( NewIndex != INDEX_NONE ? NewIndex : CurrentSelectedIndex + 1, TRUE );
						ListWidget->SetTopIndex( NewIndex != INDEX_NONE ? NewIndex - DiffItemCount : CurrentSelectedIndex - DiffItemCount, TRUE );
					}
				}
				else
				{
					ListWidget->SetTopIndex( LastTopIndex );
				}
			}

			ListWidget->bAlwaysSkipNotification = FALSE;
		}
		else
		{
			// if this child implements the UIDataStoreSubscriber interface, tell the child to load the value from the data store
			IUIDataStoreSubscriber* SubscriberChild = (IUIDataStoreSubscriber*)Widget->GetInterfaceAddress(IUIDataStoreSubscriber::UClassType::StaticClass());
			if ( SubscriberChild != NULL )
			{
				SubscriberChild->RefreshSubscriberValue(BindingIndex);
			}
		}
	}
}

/************************************************************************/
/* UavaUIAction_SetDataProviderParm	                                    */
/************************************************************************/
void UavaUIAction_SetDataProviderParm::Activated()
{
	TargetScene = GetOwnerScene();

	TArray<UProperty*> SupportedProps;
	UUIDataProvider* DataProvider = NULL;
	UavaUIParamDataProvider* ParmDataProvider = NULL;
	FString DataFieldName;
	if( ResolveMarkup( DataProvider, DataFieldName ) &&
		(ParmDataProvider = Cast<UavaUIParamDataProvider>(DataProvider)) != NULL)
	{
		ParmDataProvider->GetSupportedParameterProps( SupportedProps );
	}

	if( ! ParmDataProvider )
		return;

	FString FieldName = DataFieldMarkupString;
	FString ParmStr;

	if( bClearParameters )
		ParmDataProvider->ClearParameters();

	for( INT i = 0 ; i < SupportedProps.Num() ; i++ )
	{
		FString UnitParmStr;
		UProperty* Property = SupportedProps(i);
		FString PropNameStr = Property->GetFName().GetName();
		BYTE* PropAddr = (BYTE*)ParmDataProvider + Property->Offset;

		if( Property->IsA(UBoolProperty::StaticClass()) )
		{
			TArray<UBOOL*> BoolVars;
			GetBoolVars( BoolVars, *PropNameStr );
			if( BoolVars.Num() > 0 )
				UnitParmStr = (PropNameStr + TEXT("=") + (*BoolVars(0) ? GTrue : GFalse) );
		}
		else if( Property->IsA(UByteProperty::StaticClass()) )
		{
			TArray<INT*> IntVars;
			GetIntVars( IntVars, *PropNameStr );
			if( IntVars.Num() > 0 )
				UnitParmStr = (PropNameStr + TEXT("=") + appItoa( BYTE(*IntVars(0)) ) );
		}
		else if( Property->IsA(UIntProperty::StaticClass()) )
		{
			TArray<INT*> IntVars;
			GetIntVars( IntVars, *PropNameStr );
			if( IntVars.Num() > 0 )
				UnitParmStr = (PropNameStr + TEXT("=") + appItoa(*IntVars(0)));
		}
		else if( Property->IsA(UFloatProperty::StaticClass()) )
		{
			TArray<FLOAT*> FloatVars;
			GetFloatVars( FloatVars, *PropNameStr );
			if( FloatVars.Num() > 0 )
				UnitParmStr = (PropNameStr + TEXT("=") + FString::Printf(TEXT("%f"),*FloatVars(0)) );
		}
		else if( Property->IsA(UStrProperty::StaticClass()) )
		{
			TArray<FString*> StrVars;
			GetStringVars( StrVars, *PropNameStr );
			if( StrVars.Num() > 0 )
				UnitParmStr = (PropNameStr + TEXT("=") + *StrVars(0));
		}
		else if( Property->IsA(UNameProperty::StaticClass()) )
		{
			TArray<FName*> NameVars;
			GetNameVars( NameVars, *PropNameStr );
			if( NameVars.Num() > 0 )
				UnitParmStr = (PropNameStr + TEXT("=") + NameVars(0)->GetName());
		}
		else
		{
			check(FALSE);
		}
		if( UnitParmStr.Len() > 0 )
			ParmStr += (UnitParmStr + TEXT(","));
	}

#include "UnUIMarkupResolver.h"
	static const TCHAR MarkupEndStr[] = { MARKUP_END_CHAR, TEXT('\0') };

	INT FindIndex = INDEX_NONE;
	if( ParmStr.Len() > 0 &&
		(FindIndex = FieldName.InStr(MarkupEndStr,TRUE)) != INDEX_NONE )
	{
		FieldName = FieldName.Left(FindIndex) + TEXT("?") + ParmStr + FieldName.Right( FieldName.Len() - FindIndex );
	}

	ParmDataProvider->UpdateParameters( FieldName, FALSE );
}

void UavaUIAction_SetDataProviderParm::PostEditChange( UProperty* PropertyThatChanged )
{
	Super::PostEditChange( PropertyThatChanged );

	TargetScene = GetOwnerScene();

	TArray<UProperty*> SupportedProps;
	FName PropertyName;
	UBOOL bMarkupStringChanged = FALSE;

	if ( PropertyThatChanged )
	{
		PropertyName = PropertyThatChanged->GetFName();

		if ( PropertyName == TEXT("DataFieldMarkupString") )
		{
			UUIDataProvider* DataProvider = NULL;
			UavaUIParamDataProvider* ParmDataProvider = NULL;
			FString DataFieldName;
			if( ResolveMarkup( DataProvider, DataFieldName ) &&
				(ParmDataProvider = Cast<UavaUIParamDataProvider>(DataProvider)) != NULL)
			{
				ParmDataProvider->GetSupportedParameterProps( SupportedProps);
			}
			bMarkupStringChanged = TRUE;
		}

		if( ! bMarkupStringChanged )
		{
			return;
		}
	}

	VariableLinks.Empty();
	VariableLinks.AddZeroed( SupportedProps.Num() );

	check( VariableLinks.Num() == SupportedProps.Num() );

	for( INT i = 0 ; i < SupportedProps.Num() ; i++ )
	{
		FSeqVarLink& SeqVarLink = VariableLinks(i);
		UProperty* Property = SupportedProps(i);

		SeqVarLink.LinkDesc = Property->GetFName().GetName();
		SeqVarLink.MaxVars = 255;
		SeqVarLink.bWriteable = TRUE;

		if( Property->IsA(UBoolProperty::StaticClass()) )
			SeqVarLink.ExpectedType = USeqVar_Bool::StaticClass();
		else if( Property->IsA(UByteProperty::StaticClass()) )
			SeqVarLink.ExpectedType = USeqVar_Int::StaticClass();
		else if( Property->IsA(UIntProperty::StaticClass()) )
			SeqVarLink.ExpectedType = USeqVar_Int::StaticClass();
		else if( Property->IsA(UFloatProperty::StaticClass()) )
			SeqVarLink.ExpectedType = USeqVar_Float::StaticClass();
		else if( Property->IsA(UStrProperty::StaticClass()) )
			SeqVarLink.ExpectedType = USeqVar_String::StaticClass();
		else if( Property->IsA(UNameProperty::StaticClass()) )
			SeqVarLink.ExpectedType = USeqVar_Name::StaticClass();
		else
			check(FALSE);
	}
}

/************************************************************************/
/* UavaSeqAct_SwitchInt				                                    */
/************************************************************************/

void UavaSeqAct_SwitchInt::PostEditChange(UProperty* PropertyThatChanged)
{
	if (OutputLinks.Num() < Values.Num())
	{
		// keep adding, updating the description
		while (OutputLinks.Num() < Values.Num())
		{
			OutputLinks.AddZeroed();
		}
	}
	else
		if (OutputLinks.Num() > Values.Num())
		{
			while (OutputLinks.Num() > Values.Num())
			{
				//FIXME: any cleanup needed for each link, or can we just mass delete?
				OutputLinks.Remove(OutputLinks.Num()-1);
			}
		}
		// match all the link descriptions to the range values
		for (INT idx = 0; idx < Values.Num(); idx++)
		{
			OutputLinks(idx).LinkDesc = FString::Printf(TEXT("A == %d"),Values(idx));
		}
		Super::PostEditChange(PropertyThatChanged);
}

void UavaSeqAct_SwitchInt::Activated()
{
	// compare the values and set appropriate output impulse
	for( int i = 0; i < Values.Num(); i++)
	{
		if( ValueA == Values(i) )
			OutputLinks(i).bHasImpulse = TRUE;
		else
			OutputLinks(i).bHasImpulse = FALSE;
	}
}


/************************************************************************/
/* UavaSeqCond_BoolTable			                                    */
/************************************************************************/

void UavaSeqCond_BoolTable::PostEditChange(UProperty* PropertyThatChanged)
{
	// force at least one output link
	BoolTableVarCount = Clamp( BoolTableVarCount, 1, 5 );
	UpdateAllDynamicLinks();
	Super::PostEditChange(PropertyThatChanged);
}

void UavaSeqCond_BoolTable::Activated()
{
	INT OutLinkIndex = 0;
	const INT LinkCount = 0x01 << BoolTableVarCount;
	
	if( OutputLinks.Num() != LinkCount || VariableLinks.Num() != BoolTableVarCount )
	{
		check(FALSE);
		return;
	}

	for( BYTE VarMask = 0 ; VarMask < BoolTableVarCount ; VarMask++ )
	{
		TArray<UBOOL*> BoolVars;
		TCHAR szVarLinkDesc[] = { TEXT('A') + VarMask, TEXT('\0') };
		GetBoolVars(BoolVars, szVarLinkDesc );
		if( BoolVars.Num() > 0 && BoolVars(0) != NULL && *BoolVars(0) )
			OutLinkIndex = OutLinkIndex | (0x01 << (BoolTableVarCount - VarMask - 1) );
	}

	if( ! OutputLinks.IsValidIndex( OutLinkIndex ) )
	{
		check(FALSE);
		return;
	}

	OutputLinks(OutLinkIndex).ActivateOutputLink();
}

void UavaSeqCond_BoolTable::UpdateObject()
{
	// save the output links
	TArray<FSeqOpOutputLink> SavedOutputLinks = OutputLinks;
	Super::UpdateObject();
	OutputLinks.Empty();
	OutputLinks = SavedOutputLinks;
}

void UavaSeqCond_BoolTable::DeActivated()
{
	// do nothing, already activated output links
}

void UavaSeqCond_BoolTable::UpdateAllDynamicLinks()
{
	INT LinkCount = 0x01 << BoolTableVarCount;

	if( OutputLinks.Num() != LinkCount || VariableLinks.Num() != BoolTableVarCount )
	{
		OutputLinks.Empty();
		OutputLinks.AddZeroed(LinkCount);
		VariableLinks.Empty();
		VariableLinks.AddZeroed(BoolTableVarCount);

		for( INT LinkIndex = 0 ; LinkIndex < LinkCount ; LinkIndex++ )
		{
			FString OutLinkDesc;
			for( BYTE VarMask = 0 ; VarMask < BoolTableVarCount ; VarMask++ )
			{
				TCHAR Negation = (0x01 << VarMask) & LinkIndex ? TEXT(' ') : TEXT('/');
				TCHAR VarAlpha = TEXT('A') + ( BoolTableVarCount - VarMask - 1 );
				TCHAR szOutLinkDesc[] = { Negation, VarAlpha, TEXT('\0') };
				OutLinkDesc = FString(szOutLinkDesc) + OutLinkDesc;
			}
			OutputLinks(LinkIndex).LinkDesc = OutLinkDesc;
		}

		for( INT VarIndex = 0 ; VarIndex < BoolTableVarCount ; VarIndex++ )
		{
			TCHAR szVarName[] = { TEXT('A') + VarIndex , TEXT('\0') };
			FSeqVarLink& VarLink = VariableLinks(VarIndex);
			VarLink.LinkDesc = szVarName;
			VarLink.ExpectedType = USeqVar_Bool::StaticClass();
			VarLink.MaxVars = 1;
		}
	}
}

/************************************************************************/
/* UavaSeqCond_SetAvaNetTransaction	                                    */
/************************************************************************/

void UavaUIAction_SetAvaNetTransaction::Activated()
{

	// Begin
	if( InputLinks(0).bHasImpulse )
	{
		GetAvaNetHandler()->BeginTransaction(*SessionName);
	}
	// Undo
	else if( InputLinks(1).bHasImpulse )
	{
		// 해상도와 상관없는 Object를 Transaction에 저장하더라도
		// OptionSettings가 변경되지 않기때문에 밑의 코드는 옵션설정이 아닌이상 동작하지 않는다

		// Undo 와 상관없이 설정되는 '해상도', '화면비율'은 Undo 가 끝난후 복원해줘야한다
		// @ TODO : 다만 여기 이 코드가 위치하면 'Transaction만' 처리하는 Undo와 문맥상으로 맞지 않다.

		// { OptionSettings
		UavaOptionSettings* OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
		INT PrevTexDetail = OptionSettings->TextureDetail;
		INT PrevCharDetail = OptionSettings->CharacterDetail;
		FVector2D ConfirmedResolution = OptionSettings->GetConfirmedResolution();
		FVector2D ConfirmedAspectRatio = OptionSettings->GetAspectRatio();
		// OptionSettings }

		GetAvaNetHandler()->UndoTransaction();

		// { OptionSettings
		OptionSettings = UavaOptionSettings::GetDefaultOptionSettings();
		OptionSettings->MiscIterativeUpdate( PrevTexDetail != OptionSettings->TextureDetail, PrevCharDetail != OptionSettings->CharacterDetail );
		if( !ConfirmedResolution.IsZero() )
		{
			OptionSettings->SetResolution( ConfirmedResolution, TRUE );
			OptionSettings->SetAspectRatio( ConfirmedAspectRatio );
		}
		// OptionSettings }
	}
	// End
	else if( InputLinks(2).bHasImpulse )
	{
		TArray<UObject*> TransactionObjects;
		GetAvaNetHandler()->GetTransactionObjects(TransactionObjects);
		for( INT TransObjIdx = 0 ; TransObjIdx < TransactionObjects.Num() ; TransObjIdx++ )
			TransactionObjects(TransObjIdx)->SaveConfig();
		
		GetAvaNetHandler()->EndTransaction();
	}	
}

//! RenderDevice가 다시 생성되기 위해 대기중인지 유무.
UBOOL UavaUIAction_PendingRecreateDevice::PendingRecreateDevice()
{
	extern UBOOL GPendingRecreateDevice;
	extern UBOOL GForceRecreateDevice;

	return (GPendingRecreateDevice == TRUE || GForceRecreateDevice == TRUE);
}
