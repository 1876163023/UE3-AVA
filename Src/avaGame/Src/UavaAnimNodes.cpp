//=============================================================================
// Copyright 2004-2005 Epic Games - All Rights Reserved.
// Confidential.
//=============================================================================

#include "PrecompiledHeaders.h"
#include "avaGame.h"

IMPLEMENT_CLASS(UavaAnimBlendBase);
IMPLEMENT_CLASS(UavaAnimBlendByIdle);
IMPLEMENT_CLASS(UavaAnimNodeSequence);
IMPLEMENT_CLASS(UavaAnimBlendByPhysics);
IMPLEMENT_CLASS(UavaAnimBlendByFall);
IMPLEMENT_CLASS(UavaAnimBlendByPosture);
IMPLEMENT_CLASS(UavaAnimBlendByWeapon);
IMPLEMENT_CLASS(UavaAnimBlendByDirection);
IMPLEMENT_CLASS(UavaAnimBlendBySpeed);
IMPLEMENT_CLASS(UavaAnimBlendByAimOffset);
IMPLEMENT_CLASS(UavaAnimBlendByWeaponType);
IMPLEMENT_CLASS(UavaAnimBlendByWeaponState);
IMPLEMENT_CLASS(UavaAnimBlendByEvent);
IMPLEMENT_CLASS(UavaAnimBlendByDirectionEx);
IMPLEMENT_CLASS(UavaAnimBlendBySpeedSimple);
IMPLEMENT_CLASS(UavaAnimBlendByQVC);
IMPLEMENT_CLASS(UavaAnimNotify_Sound);
IMPLEMENT_CLASS(UavaAnimNotify_EjectBullet);
IMPLEMENT_CLASS(UavaAnimBlendPerTargetBone);
IMPLEMENT_CLASS(UavaAnimBlendByWeaponHanded);
IMPLEMENT_CLASS(UavaAnimBlendByExclusiveAnim);
IMPLEMENT_CLASS(UavaAnimBlendLimitedAim);
IMPLEMENT_CLASS(UavaAnimBlendByDamage);
IMPLEMENT_CLASS(UavaAnimBlendByRun);
IMPLEMENT_CLASS(UavaAnimBlendByClassType);
IMPLEMENT_CLASS(UavaAnimBlendByLadder);
IMPLEMENT_CLASS(UavaAnimNodeRandom);


void UavaAnimNotify_Sound::Notify( UAnimNodeSequence* NodeSeq )
{
	USkeletalMeshComponent* SkelComp = NodeSeq->SkelComponent;
	check( SkelComp );

	AActor* Owner = SkelComp->GetOwner();

	//debugf( TEXT( "**avaAnimNotify_Sound::Notify %s owner:%s" ), SkelComp->GetName(), Owner ? Owner->GetName() : TEXT("None") );

	FVector Location( 0, 0, 0 );

	if( BoneName != NAME_None )
	{		
		// Global world space
		Location = SkelComp->GetBoneLocation( BoneName );
	}	

	if (!Owner)
	{
		check( !GIsGame );

		UAudioComponent* AudioComponent = UAudioDevice::CreateComponent( SoundCue, SkelComp->GetScene(), Owner, 0 );
		if( AudioComponent )
		{
			if( BoneName != NAME_None )
			{
				AudioComponent->bUseOwnerLocation	= 0;
				AudioComponent->Location			= SkelComp->GetBoneLocation( BoneName ); // Global space
			}
			
			AudioComponent->bNonRealtime			= !GIsGame;
			AudioComponent->bAllowSpatialization	&= GIsGame;
			AudioComponent->bAutoDestroy			= 1;
			AudioComponent->Play();
		}
	}
	else
	{
		eventPlaySound2( Owner, Location );
	}	
}

void UavaAnimNotify_EjectBullet::Notify( UAnimNodeSequence* NodeSeq )
{
	USkeletalMeshComponent* SkelComp = NodeSeq->SkelComponent;
	check( SkelComp );

	AActor* Owner = SkelComp->GetOwner();
	eventEjectBullet( Owner );
}

void UavaAnimBlendBase::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	if ( GWorld != NULL && GWorld->GetWorldInfo() != NULL )
		LastTickTime = GWorld->GetWorldInfo()->TimeSeconds;
	Super::TickAnim( DeltaSeconds, TotalWeight );
}


void UavaAnimBlendBase::GetChildNodes(int childIdx, TArray<UAnimNode*>& Nodes)
{
	/// bug fix -_-!!!
	if ( childIdx < Children.Num() && Children(childIdx).Anim )
		Children(childIdx).Anim->GetNodes( Nodes );
}

UAnimNode* UavaAnimBlendBase::FindChildAnimNode(INT childIdx,FName InNodeName)
{
	TArray<UAnimNode*> Nodes;
	this->GetChildNodes( childIdx, Nodes );
	for(INT i=0; i<Nodes.Num(); i++)
	{
		if( Nodes(i)->NodeName == InNodeName )
		{
			return Nodes(i);
		}
	}
	return NULL;
}

UAnimNode* UavaAnimBlendBase::FindChildAnimNodeByClass(INT childIdx,class UClass* DesiredClass)
{
	TArray<UAnimNode*> Nodes;
	this->GetChildNodes( childIdx, Nodes );
	for(INT i=0; i<Nodes.Num(); i++)
	{
		if( Nodes(i)->IsA(DesiredClass) )
		{
			return Nodes(i);
		}
	}
	return NULL;
}

void UavaAnimBlendBase::GetAnimSeqNodes(INT ChildIndex, TArray<class UAnimNodeSequence*>& AnimSeqs)
{
	if ( ChildIndex < Children.Num() && Children(ChildIndex).Anim )
		Children(ChildIndex).Anim->GetAnimSeqNodes(AnimSeqs);
}

/************************************************************************************
* UavaAnimBlendBase
***********************************************************************************/

FLOAT UavaAnimBlendBase::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	return SliderPosition;
}

void UavaAnimBlendBase::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	SliderPosition = NewSliderValue;

	const INT TargetChannel = appRound(SliderPosition*(Children.Num() - 1));
	if( ActiveChildIndex != TargetChannel )
	{
		FLOAT const BlendInTime = GetBlendTime(TargetChannel);
		SetActiveChild(TargetChannel, BlendInTime);
	}
}

FString UavaAnimBlendBase::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	INT TargetChild = appRound( SliderPosition*(Children.Num() - 1) );
	return FString::Printf( TEXT("%3.2f %s"), SliderPosition, *Children(TargetChild).Name.ToString() );
}

FLOAT UavaAnimBlendBase::GetBlendTime(INT ChildIndex, UBOOL bGetDefault/*=0*/)
{
	if ( bGetDefault || (ChildBlendTimes.Num() == 0) || (ChildIndex < 0) || (ChildIndex >= ChildBlendTimes.Num()) || (ChildBlendTimes(ChildIndex) == 0.0f) )
	{
		return BlendTime;
	}
	else
	{
		return ChildBlendTimes(ChildIndex);
	}
}



/** 
 * ClacDist - Takes 2 yaw values A & B and calculates the distance between them. 
 *
 *	@param	YawA	- First Yaw
 *  @param	YawB	- Second Yaw
 *  @param	Dist	- The distance between them is returned here
 *  
 *	@return			- Returns the sign needed to move from A to B
 */

static INT CalcDist(INT YawA, INT YawB, INT& Dist)
{
	INT Sign = 1;

	Dist = YawA - YawB;
	if ( Abs(Dist) > 32767 )
	{
		if (Dist > 0)
		{
			Sign = -1;
		}
		Dist = Abs( Abs(Dist) - 65536 );
	}
	else
	{
		if (Dist < 0)
		{
			Sign = -1;
		}
	}
	return Sign;
}


/*!
	class Our sequence player 
*/

void UavaAnimNodeSequence::OnAnimEnd(FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnAnimEnd(PlayedTime, ExcessTime);

	if ( SeqStack.Num() > 0 )
	{
		SetAnim( SeqStack(0) );
		SeqStack.Remove(0,1);


		UBOOL bSeqLoop = (SeqStack.Num() == 0) ? bLoopLastSequence : false;
		PlayAnim(bSeqLoop, Rate,0.0);
	}
}

void UavaAnimNodeSequence::PlayAnimation(FName Sequence, FLOAT SeqRate, UBOOL bSeqLoop)
{
	SetAnim(Sequence);
	PlayAnim(bSeqLoop, SeqRate);
}

void UavaAnimNodeSequence::PlayAnimationSet(const TArray<FName>& Sequences,FLOAT SeqRate,UBOOL bLoopLast)
{
	if ( Sequences.Num() > 0 )
	{
		PlayAnimation(Sequences(0), SeqRate, false);
		for ( INT i=1; i<Sequences.Num(); i++ )
		{
			SeqStack.AddItem( Sequences(i) );
		}
		bLoopLastSequence = bLoopLast;
	}
}


/**
 *  BlendByPhysics - This AnimNode type is used to determine which branch to player
 *  by looking at the current physics of the pawn.  It uses that value to choose a node
 */
void UavaAnimBlendByPhysics::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	// Get the Pawn Owner
	if (SkelComponent != NULL &&
		SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			// Get the current physics from the pawn
			INT CurrentPhysics = INT(POwner->Physics);

			// If the physics has changed, and there is a valid blend for it, blend to that value
			if ( LastPhysics != CurrentPhysics )
			{
				INT PhysicsIndex = PhysicsMap[CurrentPhysics];
				if (PhysicsIndex<0)
					PhysicsIndex = 0;
				SetActiveChild( PhysicsIndex,GetBlendTime(PhysicsIndex,false) );
			}

			LastPhysics = CurrentPhysics;						
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/**
 * BlendByFall - Will use the pawn's Z Velocity to determine what type of blend to perform.  
 * -- FIXME: Add code to trace to the ground and blend to landing
 */

void UavaAnimBlendByFall::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	if ( TotalWeight > 0 )
	{
		// If we are not being rendered, reset the state to FBT_None and exit.

		if (SkelComponent != NULL &&
			SkelComponent->GetOwner() != NULL)
		{
			APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
			if ( POwner )
			{
				if (POwner->Physics == PHYS_Falling)
				{
					FLOAT FallingVelocity = POwner->Velocity.Z;
					switch (FallState)
					{
						case FBT_Land:
						case FBT_None:		//------------- We were inactive, determine the initial state

							if ( FallingVelocity < 0 )			// Falling
								ChangeFallState(FBT_Down);
							else								// Jumping
			
								ChangeFallState(FBT_Up);
																
							for (INT i=0;i<Children.Num();i++)
								if (i != FallState)
								{
									Children(i).Weight=0.0f;									
								}
								else
								{
									Children(i).Weight=1.0f;									
								}

							break;

						case FBT_Up:		//------------- We are jumping
							if ( LastFallingVelocity < FallingVelocity )	// Double Jump
							{
								ChangeFallState(FBT_Up);
							}

							else if (FallingVelocity <= 0)					// Begun to fall
								ChangeFallState(FBT_Down);

							break;
							
						case FBT_Down:		//------------- We are falling

//							debugf(TEXT("### %f %f"),FallingVelocity,LastFallingVelocity);

							if ( FallingVelocity > 0 && FallingVelocity > LastFallingVelocity )		// Double Jump
								ChangeFallState(FBT_Up);
							else
							{
								DWORD TraceFlags = TRACE_World;
								FCheckResult Hit(1.f);
								FLOAT BTime = GetBlendTime(FBT_PreLand, false) * 1.5;
//								debugf(TEXT("### %f"),BTime);
								FVector HowFar = POwner->Velocity * BTime; //GetBlendTime(FBT_PreLand,false) * 1.5;
//								GWorld->LineBatcher->DrawLine(POwner->Location + HowFar, POwner->Location,FColor(255,255,255));
								GWorld->SingleLineCheck(Hit, POwner, POwner->Location + HowFar, POwner->Location,TraceFlags);
							
								if ( Hit.Actor ) 
								{
									ChangeFallState(FBT_PreLand);
								}
							}
							break;


					}
					LastFallingVelocity = FallingVelocity;
				}
				else if ( FallState != FBT_Land )
				{
//					debugf(TEXT("### We have Landed"));
					ChangeFallState(FBT_Land);
				}
			}
		}
	}
	else if ( FallState != FBT_None )
	{
//		debugf(TEXT("### We are inactive, setting state to none"));
		ChangeFallState(FBT_None);
	}

	Super::TickAnim(DeltaSeconds,TotalWeight);	// pass along for now until falls anims are separated			

}

void UavaAnimBlendByFall::SetActiveChild( INT ChildIndex, FLOAT BlendTime )
{
	Super::SetActiveChild(ChildIndex,BlendTime);

	if ( Cast<UAnimNodeSequence>( Children(ChildIndex).Anim ) )
	{
		UAnimNodeSequence* P = Cast<UAnimNodeSequence>( Children(ChildIndex).Anim );	
		P->PlayAnim(P->bLooping);
	}
}


void UavaAnimBlendByFall::OnChildAnimEnd(UAnimNodeSequence* Child)
{
	if ( FallState == FBT_Up && Child == Children(FBT_Up).Anim )
	{
		ChangeFallState(FBT_Down);
	}
}
		

// Changes the falling state

void UavaAnimBlendByFall::ChangeFallState(EBlendFallTypes NewState)
{
	INT NS = NewState;
//	debugf(TEXT("%s::ChangeFallState from %i to %i"),GetName(), FallState, NS);

	if (FallState != NewState)
	{
		FallState = NewState;
		if (FallState!=FBT_None)
		{
			SetActiveChild( NewState, GetBlendTime(NewState,false) );
		}
	}
}


/**
* BlendByWeaponType - Will use the pawn's WeaponType to determine what type of blend to perform.  
* -- 
*/
void UavaAnimBlendByWeaponType::InitAnimSequence(FName NewPrefix)
{
	// Child 들의 AnimSequence 에 대해서 Name 의 Prefix 를 교체해준다.
	// TurnPlayer->SetAnim(TurnAnimNames[0]);
	
	//TCHAR	prefix[128];
	//_tcscpy( prefix, *NewPrefix );

	//TEXT( "**UavaAnimBlendByEvent::OnChildAnimEnd %s"), *Child->AnimSeqName

	if( SkelComponent == NULL)	return;

	BYTE curWeaponType;
	AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
	if ( PawnOwner == NULL )	
	{
		UavaUICharacterPIP* PIP = Cast<UavaUICharacterPIP>(SkelComponent->GetOuter());

		if (PIP && PIP->WeaponTemplate)
		{				
			AavaWeaponAttachment* Attachment = Cast<AavaWeaponAttachment>( PIP->WeaponTemplate->ClassDefaultObject );

			curWeaponType = Attachment->AttachmentWeaponType;
		}	
		else
			/* 여기가 일반적인 오류의 경우임! */
			return;
	}	
	else
	{
		curWeaponType = PawnOwner->eventGetWeaponAttachmentType();
	}
	
	if( Children(curWeaponType).Anim == NULL )	return;
	if ( PrvPrefix.Num() >= curWeaponType && _tcscmp( PrvPrefix(curWeaponType).GetName(), NewPrefix.GetName() ) == 0 )	return;
	
	TArray<UAnimNodeSequence*> SeqNodeArray;

	Children(curWeaponType).Anim->GetAnimSeqNodes(SeqNodeArray);

	//debugf( TEXT( "************ UavaAnimBlendByWeaponType::InitAnimSequence Change Animation Prefix %s to %s ********************** "), *PrvPrefix(curWeaponType), *NewPrefix  );

	for ( int i = 0 ; i < SeqNodeArray.Num() ; ++ i )
	{
		const TCHAR*	sequenceName = _tcsstr( SeqNodeArray(i)->AnimSeqName.GetName(), _T("_") );
		FName	InSequenceName;
		if ( sequenceName != NULL ) 
		{
			InSequenceName = FName( *FString::Printf( TEXT("%s%s"), NewPrefix.GetName(), sequenceName ) );
			if ( SkelComponent != NULL && SkelComponent->FindAnimSequence( InSequenceName ) != NULL )
				SeqNodeArray(i)->SetAnim( InSequenceName );			
		}
	}
	if ( PrvPrefix.Num() >= curWeaponType )		PrvPrefix(curWeaponType) = NewPrefix;
}

void UavaAnimBlendByWeaponType::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	if ( TotalWeight > 0 )
	{
		BYTE curWeaponType = WBT_SMG01;	// Default 로 1번 Channel 을 Play 해준다.
		// Editor 에서 확인하기 위해서 ForcedWeaponType 을 이용한다.
		if ( !GIsGame )	curWeaponType = ForcedWeaponType;
		
		if( SkelComponent != NULL)
		{
			if (SkelComponent->GetOwner() != NULL)
			{
				AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
				if ( PawnOwner )
					curWeaponType = PawnOwner->eventGetWeaponAttachmentType();
			}			
			else
			{
				UavaUICharacterPIP* PIP = Cast<UavaUICharacterPIP>(SkelComponent->GetOuter());

				if (PIP && PIP->WeaponTemplate)
				{				
					AavaWeaponAttachment* Attachment = Cast<AavaWeaponAttachment>( PIP->WeaponTemplate->ClassDefaultObject );

					curWeaponType = Attachment->AttachmentWeaponType;
				}					
			}
		}
		
		if ( curWeaponType != ActiveChildIndex && curWeaponType < Children.Num() && Children( curWeaponType ).Anim )
			SetActiveChild( curWeaponType, GetBlendTime(curWeaponType) );
	}
	Super::TickAnim(DeltaSeconds,TotalWeight);
}

FLOAT UavaAnimBlendByWeaponType::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	return float(ForcedWeaponType)/WBT_None;
}

void UavaAnimBlendByWeaponType::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// Convert from 0.0 -> 1.0 to WBT_Knife to WBT_None
	ForcedWeaponType = BYTE( NewSliderValue * WBT_None );
}

FString UavaAnimBlendByWeaponType::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("") );
}

/**
* BlendByWeaponState - Will use the pawn's WeaponState to determine what type of blend to perform.  
* -- 
*/
void UavaAnimBlendByWeaponState::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	if ( !bBlendByEvent )
	{
		//if ( TotalWeight > 0 )
		{
			BYTE curWeaponState = 0;	// Default 로 0번 Channel 을 Play 해준다.
			//if ( !GIsGame ) curWeaponState = ForcedWeaponState;
			if( SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
			{
				AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
				if ( PawnOwner )
					curWeaponState = PawnOwner->WeaponState;
			}
			// @ToDo Blend Time 은 에디터에서 가져오도록 한다.
			if ( curWeaponState != ActiveChildIndex )
				SetActiveChild( curWeaponState, GetBlendTime(curWeaponState) );
		}
	}
	Super::TickAnim(DeltaSeconds,TotalWeight);
}

void UavaAnimBlendByWeaponState::PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */)
{
	BYTE curWeaponState = 0;	// Default 로 0번 Channel 을 Play 해준다.
	if( SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
		if ( PawnOwner )
			curWeaponState = PawnOwner->WeaponState;
	}
	// @ToDo Blend Time 은 에디터에서 가져오도록 한다.
	if ( Children( curWeaponState ).Anim )
	{
		for( int i = 0 ; i < Children.Num() ; ++ i )
		{
			if ( i == curWeaponState )	Children( i ).Weight = 1.0;
			else						Children( i ).Weight = 0.0;
		}
		Children( curWeaponState ).Anim->PlayAnim( bLoop, Rate, StartTime );
	}
}

FLOAT UavaAnimBlendByWeaponState::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// DirAngle is between -PI and PI. Return value between 0.0 and 1.0 - so 0.5 is straight ahead.
	return float(ForcedWeaponState)/( Children.Num() );
}

void UavaAnimBlendByWeaponState::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// Convert from 0.0 -> 1.0 to WBT_Knife to WBT_None
	ForcedWeaponState = BYTE( NewSliderValue * ( Children.Num() ) );
}

FString UavaAnimBlendByWeaponState::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("") );
}

/**
 *  BlendByPosture is used to determine if we should be playing the Crouch/walk animations, or the 
 *  running animations.
 */

void UavaAnimBlendByPosture::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	// Get the Pawn Owner
	int nActiveChild = -1;
	if ( TotalWeight > 0.0 )
	{
		if ( !bBlendByEvent )
		{
			if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
			{
				AavaPawn* POwner = Cast<AavaPawn>(SkelComponent->GetOwner());
				if ( POwner )
				{
					if ( POwner->bIsDash )
					{
						if ( POwner->bIsCrouched )	nActiveChild = 3;
						else						nActiveChild = 2;
					}
					else
					{
						if( POwner->bIsCrouched )	nActiveChild = 1;
						else						nActiveChild = 0;
					}

					
					if ( Children.Num() <= nActiveChild ||  Children(nActiveChild).Anim == NULL )	nActiveChild = 0;
					if ( nActiveChild != ActiveChildIndex )
						SetActiveChild( nActiveChild, GetBlendTime(nActiveChild) );
				}
			}
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UavaAnimBlendByPosture::PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */)
{
	if ( SkelComponent != NULL && SkelComponent->GetOwner() != NULL )
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			int nActiveIdx = POwner->bIsCrouched ? 1 : 0;

			Children( !nActiveIdx ).Weight = 0.0;
			Children( nActiveIdx ).Weight = 1.0;

			if ( Children( nActiveIdx ).Anim )
				Children( nActiveIdx ).Anim->PlayAnim( bLoop, Rate, StartTime );
		}
	}
}


/**
* BlendByEvent - This node is NOT automanaged.  Instead it's designed to have it's PlayEvent/StopEvent functions
* called.  If it's playing a Event animation that's not looping it will blend back out after the
* animation completes.
*/

void UavaAnimBlendByEvent::PlayAnimByName(FName Prefix,FName SequenceName,UBOOL bLoop,FLOAT AnimRate)
{
	UAnimNodeSequence*	pReservedNode;
	pReservedNode = Cast<UAnimNodeSequence>(Children(EBT_Reserved).Anim);
	if ( pReservedNode != NULL )
	{
		const TCHAR*	NewSequenceName = _tcsstr( SequenceName.GetName(), _T("_") );
		if ( NewSequenceName != NULL ) 
			pReservedNode->SetAnim( FName( *FString::Printf( TEXT("%s%s"), Prefix.GetName(), NewSequenceName ) ) );
		else
			pReservedNode->SetAnim( SequenceName );
		eventPlayAnimByEvent( EBT_Reserved, bLoop, AnimRate );

	}
}

void UavaAnimBlendByEvent::TickAnim( float DeltaSeconds, float TotalWeight )
{
	Super::TickAnim( DeltaSeconds, TotalWeight );
	if ( GIsGame )	return;
	// Editor에서 확인을 위한 Code 이기 때문에 Pawn 에 접근할 필요가 없다.
	if ( ActiveChildIndex != ForcedEventType )
	{
		SetActiveChild( ForcedEventType, GetBlendTime(ForcedEventType) );
	}
}

UAnimNode* GetParent( UAnimNodeBlendBase* pParent, UAnimNode* node )
{
	UAnimNode* ret = NULL;
	for ( int i = 0 ; i < node->ParentNodes.Num() ; ++ i )
	{
		if ( node->ParentNodes(i) == pParent )	return node;
		ret = GetParent( pParent, node->ParentNodes(i) );
		if ( ret != NULL )	return ret;
	}
	return NULL;
}
	
void UavaAnimBlendByEvent::OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnChildAnimEnd(Child,PlayedTime,ExcessTime);

	//debugf( TEXT( "**UavaAnimBlendByEvent::OnChildAnimEnd %s"), *Child->AnimSeqName );
	// 현재 Active 한 놈의 Animation 이 End 했는지를 확인해야 한다.
	UAnimNode* pActiveChild = GetParent( this, Child );
	if ( pActiveChild != NULL )
	{
		if ( Children(ActiveChildIndex).Anim == pActiveChild )
		{
			eventStopEvent( ActiveChildIndex, BlendTime);

			if ( DELEGATE_IS_SET( OnStopEvent ) )
			{
				delegateOnStopEvent( ActiveChildIndex );
			}
		}
	}
	// looping animation 이 아니라면 script event 를 호출해 준다.
}

FLOAT UavaAnimBlendByEvent::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	return float(ForcedEventType)/EBT_MAX;
}

void UavaAnimBlendByEvent::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// Convert from 0.0 -> 1.0 to WBT_Knife to WBT_None
	ForcedEventType = BYTE( NewSliderValue * EBT_MAX );
}

FString UavaAnimBlendByEvent::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("") );
}

/**
 * BlendByWeapon - This node is NOT automanaged.  Instead it's designed to have it's Fire/StopFire functions
 * called.  If it's playing a firing animation that's not looping (ie: not auto-fire) it will blend back out after the
 * animation completes.
 */

void UavaAnimBlendByWeapon::OnChildAnimEnd(UAnimNodeSequence* Child, FLOAT PlayedTime, FLOAT ExcessTime)
{
	Super::OnChildAnimEnd(Child,PlayedTime,ExcessTime);

	// Call the script event if we are not looping.
	if (!bLooping)
	{
		eventAnimStopFire(BlendTime);
	}
	else if (LoopingAnim != NAME_None)
	{
		// loop with LoopingAnim instead of the original fire animation
		UAnimNodeSequence* FireNode = Cast<UAnimNodeSequence>(Children(1).Anim);
		if (FireNode != NULL)
		{
			FireNode->SetAnim(LoopingAnim);
			FireNode->PlayAnim(true);
		}
	}
}


/** 
 * BlendByDirection nodes look at the direction their owner is moving and use it to
 * blend between the different children.  We have extended the Base (BlendDirectional) in
 * order to add the ability to adjust the animation speed of one of this node's children
 * depending on the velocity of the pawn.  
 */

void UavaAnimBlendByDirection::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{

	// We only work if we are visible
	if (TotalWeight>0)
	{
		// bAdjustRateByVelocity is used to make the animation slow down the animation
		if (bAdjustRateByVelocity)
		{
			if (SkelComponent != NULL &&
				SkelComponent->GetOwner() != NULL &&
				SkelComponent->GetOwner()->IsA(APawn::StaticClass()))
			{
				APawn* POwner = (APawn*)SkelComponent->GetOwner();

				FLOAT NewRate = POwner->Velocity.Size() / fBasicVelocity;
				for (INT i=0;i<Children.Num();i++)
				{
					if ( Cast<UAnimNodeSequence>(Children(i).Anim) )
						Cast<UAnimNodeSequence>(Children(i).Anim)->Rate = NewRate;
				}
			}
		}

		EBlendDirTypes	CurrentDirection = Get4WayDir();

		if (CurrentDirection != LastDirection)		// Direction changed
		{
			SetActiveChild( CurrentDirection, GetBlendTime(CurrentDirection,false) );
		}
		LastDirection = CurrentDirection;
	}
	else
		LastDirection = FBDir_None;

	Super::TickAnim(DeltaSeconds, TotalWeight);

}

void UavaAnimBlendByDirection::SetActiveChild( INT ChildIndex, FLOAT BlendTime )
{
	Super::SetActiveChild(ChildIndex,BlendTime);

	if ( Cast<UAnimNodeSequence>( Children(ChildIndex).Anim ) )
	{
		UAnimNodeSequence* P = Cast<UAnimNodeSequence>( Children(ChildIndex).Anim );
		P->PlayAnim(P->bLooping);
	}
}


EBlendDirTypes UavaAnimBlendByDirection::Get4WayDir()
{

    FLOAT forward, right;
    FVector V;

	if (SkelComponent != NULL &&
		SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			V = POwner->Velocity;
			V.Z = 0.0f;

			if ( V.IsNearlyZero() )
				return FBDir_Forward;

			FRotationMatrix RotMatrix(POwner->Rotation);

			V.Normalize();
			forward = RotMatrix.GetAxis(0) | V;
			if (forward > 0.82f) // cos( 35 degrees )
				return FBDir_Forward;
			else if (forward < -0.82f)
				return FBDir_Back;
			else
			{
				right = RotMatrix.GetAxis(1) | V;
				
				if (right > 0.0f)
					return FBDir_Right;
				else
					return FBDir_Left;
			}
		}
	}
	return FBDir_Forward;

}

/*-----------------------------------------------------------------------------
UavaAnimBlendByDirectionEx
-----------------------------------------------------------------------------*/
void UavaAnimBlendByDirectionEx::TickAnim(float DeltaSeconds,FLOAT TotalWeight)
{
	if (TotalWeight>0)
	{
		check(Children.Num() == 8);

		bool	bIgnore =false;

		// Calculate DirAngle based on player velocity.
		AActor* actor = SkelComponent->GetOwner(); // Get the actor to use for acceleration/look direction etc.
		if( actor )
		{
			FLOAT TargetDirAngle = 0.f;
			FVector	VelDir = actor->Velocity;
			VelDir.Z = 0.0f;

			if( VelDir.IsNearlyZero() )
			{
				TargetDirAngle	=	0.f;
				bIgnore			=	true;
			}
			else
			{
				VelDir = VelDir.SafeNormal();

				FVector LookDir = actor->Rotation.Vector();
				LookDir.Z = 0.f;
				LookDir = LookDir.SafeNormal();

				FVector LeftDir = LookDir ^ FVector(0.f,0.f,1.f);
				LeftDir = LeftDir.SafeNormal();

				FLOAT ForwardPct = (LookDir | VelDir);
				FLOAT LeftPct = (LeftDir | VelDir);

				TargetDirAngle = appAcos( Clamp<FLOAT>(ForwardPct, -1.f, 1.f) );
				if( LeftPct > 0.f )
				{
					TargetDirAngle *= -1.f;
				}
			}
			// Move DirAngle towards TargetDirAngle as fast as DirRadsPerSecond allows
			//FLOAT DeltaDir = FindDeltaAngle(DirAngle, TargetDirAngle);
			//if( DeltaDir != 0.f )
			//{
			//	FLOAT MaxDelta = DeltaSeconds * DirDegreesPerSecond * (PI/180.f);
			//	DeltaDir = Clamp<FLOAT>(DeltaDir, -MaxDelta, MaxDelta);
			//	DirAngle = UnwindHeading( DirAngle + DeltaDir );
			//}
			DirAngle = TargetDirAngle;
		}

		//		7	0	1
		//		6		2
		//		5	4	3
		if ( !bIgnore )
		{
			int nActiveSlot;

			if ( DirAngle < -0.875*PI )	nActiveSlot = 4;
			else if ( DirAngle < -0.625*PI )	nActiveSlot = 5;
			else if ( DirAngle < -0.375*PI )	nActiveSlot = 6;
			else if ( DirAngle < -0.125*PI )	nActiveSlot = 7;
			else if ( DirAngle <  0.125*PI )	nActiveSlot = 0;
			else if ( DirAngle <  0.375*PI )	nActiveSlot = 1;
			else if ( DirAngle <  0.625*PI )	nActiveSlot = 2;
			else if ( DirAngle <  0.875*PI )	nActiveSlot = 3;
			else								nActiveSlot = 4;

			// bAdjustRateByVelocity is used to make the animation slow down the animation
			if (bAdjustRateByVelocity)
			{
				if (SkelComponent != NULL &&
					SkelComponent->GetOwner() != NULL &&
					SkelComponent->GetOwner()->IsA(APawn::StaticClass()))
				{
					APawn* POwner = (APawn*)SkelComponent->GetOwner();
					FLOAT NewRate = POwner->Velocity.Size() / fBasicVelocity;
					for (INT i=0;i<Children.Num();i++)
					{
						if ( Cast<UAnimNodeSequence>(Children(i).Anim) )
							Cast<UAnimNodeSequence>(Children(i).Anim)->Rate = NewRate;
					}
				}
			}

			if ( nActiveSlot != ActiveChildIndex )
			{
				SetActiveChild( nActiveSlot, GetBlendTime(nActiveSlot,false) );
			}
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

FLOAT UavaAnimBlendByDirectionEx::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// DirAngle is between -PI and PI. Return value between 0.0 and 1.0 - so 0.5 is straight ahead.
	return 0.5f + (0.5f * (DirAngle / (FLOAT)PI));
}

void UavaAnimBlendByDirectionEx::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	// Convert from 0.0 -> 1.0 to -PI to PI.
	DirAngle = (FLOAT)PI * 2.f * (NewSliderValue - 0.5f);
}

FString UavaAnimBlendByDirectionEx::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("%3.2f%c"), DirAngle * (180.f/(FLOAT)PI), 176 );
}

/**
 * This blend looks at the velocity of the player and blends depending on if they are moving or not
 */

void UavaAnimBlendByIdle::InitAnim( USkeletalMeshComponent* meshComp, UAnimNodeBlendBase* Parent )
{
	Super::InitAnim( meshComp, Parent );

	// Get AnimNodeSync...
	TArray<UAnimNode*> Nodes;
	if ( meshComp != NULL && meshComp->Animations != NULL )
	{
		meshComp->Animations->GetNodesByClass( Nodes, UAnimNodeSynch::StaticClass() );
		for(INT i=0; i<Nodes.Num(); i++)
			SyncNodes.AddItem( Cast<UAnimNodeSynch>( Nodes(i) ) );
	}
}

void UavaAnimBlendByIdle::PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */)
{
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			int nActiveChild = ( POwner->Velocity.Size() == 0) ? 0 : 2;
			for(INT i=0; i<Children.Num(); i++)
			{
				if(i == nActiveChild)
				{
					Children(i).Weight = 1.0f;
				}
				else
				{
					Children(i).Weight = 0.0f;
				}
			}

			if ( Children(nActiveChild).Anim )
                Children(nActiveChild).Anim->PlayAnim(bLoop,Rate,StartTime);
		}
	}
}

void UavaAnimBlendByIdle::TickAnim(float DeltaSeconds, FLOAT TotalWeight)
{
	// Get the Pawn Owner

	
	if ( !bBlendByEvent )
	{
		if ( TotalWeight > 0 )
		{
			if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
			{
				APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
				if ( POwner )
 				{
					FVector	VelDir = POwner->Velocity;
					//VelDir.Z = 0.0f;
					//debugf(TEXT("Velocity %i"), POwner->Velocity.Size() );
					if( VelDir.IsNearlyZero() )
					{
						UAnimNodeSequence* TurnPlayer = Cast<UAnimNodeSequence>( Children(1).Anim );
						// If we can see the twist control, look to see if the hips are in motion.  The hips should
						// always be the last item in the TwistData array
						if ( TwistControl && TurnPlayer )
						{
							INT HipsIndex = TwistControl->TwistData.Num()-1;
							if ( TwistControl->TwistData( HipsIndex ).bInMotion )
							{
								// set the Animation Here
								if ( 1 != ActiveChildIndex )
								{
									INT Dist;
									INT Dir = CalcDist(TwistControl->TwistData(HipsIndex).BoneYawOffset,TwistControl->HeadYaw, Dist);
									if (Dir>0)
									{
										TurnPlayer->SetAnim(TurnAnimNames[0]);
									}
									else
									{
										TurnPlayer->SetAnim(TurnAnimNames[1]);
									}

									if( Children(1).Weight <= ZERO_ANIMWEIGHT_THRESH )
									{
										TurnPlayer->PlayAnim(FALSE,1.0,0.0);
										SetActiveChild(1,BlendTime);
									}
								}
								
							}
							else
							{
								SetActiveChild(0,BlendTime);
							}

						}
						else
						{
							SetActiveChild(0,BlendTime);
						}
					}
					else
					{
						if ( 2 != ActiveChildIndex )
						{
							for(INT i=0; i<SyncNodes.Num(); i++)
							{
								for(INT GroupIdx=0; GroupIdx< SyncNodes(i)->Groups.Num(); GroupIdx++)
								{
									FSynchGroup& SynchGroup = SyncNodes(i)->Groups(GroupIdx);
									if ( SynchGroup.MasterNode != NULL )
										SynchGroup.MasterNode->SetPosition( 0.0, false );
								}
							}
							SetActiveChild(2,BlendTime);
						}
					}
				}
			}
		}
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

/**
	AnimNodeAimOffset 클래스로부터 상속받았음.
	Pawn 에서 직접 Rotation 을 가져오기 위해서임.
*/

FVector2D UavaAnimBlendByAimOffset::GetAim()
{
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		APawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
		INT		LookPitch = 0;
		FLOAT	PitchMax = 1.0f, PitchMin = 1.0f;
		if ( PawnOwner )
		{
			if ( PawnOwner->DrivenVehicle != NULL )
			{
				if ( PawnOwner->DrivenVehicle->Controller != NULL )
					LookPitch = PawnOwner->DrivenVehicle->Controller->Rotation.Pitch;
				else
					LookPitch = ( PawnOwner->DrivenVehicle->RemoteViewPitch << 8 ) & 65535;

				PitchMax = PawnOwner->DrivenVehicle->ViewPitchMax;
				PitchMin = PawnOwner->DrivenVehicle->ViewPitchMin;
			}
			else
			{
				if ( PawnOwner->Controller != NULL )
					LookPitch = PawnOwner->Controller->Rotation.Pitch;
				else
					LookPitch = ( PawnOwner->RemoteViewPitch << 8 ) & 65535;

				PitchMax = PawnOwner->ViewPitchMax;
				PitchMin = PawnOwner->ViewPitchMin;
			}
		}
		else
		{
			AavaVehicle* avaVehicle = Cast<AavaVehicle>(SkelComponent->GetOwner());
			if ( avaVehicle != NULL )
			{
				if ( SeatIndex != -1 && avaVehicle->Seats.Num() >= SeatIndex )
				{
					const FVehicleSeat* SeatInfo = &avaVehicle->Seats(SeatIndex);
					if ( SeatInfo != NULL && SeatInfo->SeatPawn != NULL && SeatInfo->SeatPawn->Driver != NULL )
					{
						if ( SeatInfo->SeatPawn->Controller != NULL )
						{
							LookPitch = SeatInfo->SeatPawn->Controller->Rotation.Pitch;

							PitchMax = SeatInfo->SeatPawn->ViewPitchMax;
							PitchMin = SeatInfo->SeatPawn->ViewPitchMin;
						}
						else
						{
							LookPitch = ( SeatInfo->SeatPawn->RemoteViewPitch << 8 ) & 65535;

							PitchMax = SeatInfo->SeatPawn->ViewPitchMax;
							PitchMin = SeatInfo->SeatPawn->ViewPitchMin;
						}
					}
				}
			}
		}

		// [-32767 .. +32768]의 값으로 만들어서 Min, Max값으로 나누어 
		// [-1 .. +1]값으로 변환한다.

		if (LookPitch > 32767)	LookPitch -= 65535;
		if ( LookPitch >= 0 )	Aim.Y = (float)LookPitch/Abs( PitchMax );
		else					Aim.Y = (float)LookPitch/Abs( PitchMin );
		Aim.X = 0;

	}
	return Aim;
}

void UavaAnimBlendBySpeed::PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */)
{
	if (SkelComponent != NULL &&
		SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			float	speed = POwner->Velocity.Size();
			int		nActiveIdx = -1;
			if ( speed > 0 )
			{
				if ( !POwner->bIsWalking && Children( 2 ).Anim )	nActiveIdx = 2;
				else if ( Children( 1 ).Anim )						nActiveIdx = 1;
			}
			else
			{
				if ( Children( 0 ).Anim )	nActiveIdx = 0;
			}

			if ( nActiveIdx != -1 )
			{
				for ( int i = 0 ; i < Children.Num() ; ++ i )
				{
					if ( i == nActiveIdx )	Children( i ).Weight = 1.0;
					else					Children( i ).Weight = 0.0;
				}
				Children(nActiveIdx).Anim->PlayAnim(bLoop,Rate,StartTime);
			}
		}
	}
}

void UavaAnimBlendBySpeed::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if ( !bBlendByEvent )
	{
		if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
		{
			APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
			if ( POwner )
			{
				float	speed = POwner->Velocity.Size();
				if ( speed > 0 )
				{
					if ( !POwner->bIsWalking && Children( 2 ).Anim )	
					{
						if ( 2 != ActiveChildIndex )
							SetActiveChild( 2,GetBlendTime(2) );
					}
					else if ( Children( 1 ).Anim )
					{
						if ( 1 != ActiveChildIndex )
							SetActiveChild( 1,GetBlendTime(1) );
					}
				}
				else
				{
					if ( 0 != ActiveChildIndex && Children( 0 ).Anim )
						SetActiveChild( 0,GetBlendTime(0) );
				}
			}
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UavaAnimBlendBySpeedSimple::TickAnim(FLOAT DeltaSeconds, FLOAT TotalWeight)
{
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner		)
            Child2Weight = POwner->bIsWalking ? 0.0f : 1.0f ;
	}

	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UavaAnimBlendByQVC::PlayAnim(UBOOL bLoop/* =FALSE */,FLOAT Rate/* =1.000000 */,FLOAT StartTime/* =0.000000 */)
{
	if( SkelComponent == NULL || SkelComponent->GetOwner() == NULL)	return;
	AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
	if(PawnOwner == NULL )	return;

	BYTE qvc = PawnOwner->LastQuickVoiceMsg;
	if ( Children( qvc ).Anim )
	{
		for( int i = 0 ; i < Children.Num() ; ++ i )
		{
			if ( i == qvc )	Children( i ).Weight = 1.0;
			else			Children( i ).Weight = 0.0;
		}
		Children( qvc ).Anim->PlayAnim( bLoop, Rate, StartTime );
	}
	
}

// Target 만 Play 한다.
void UavaAnimBlendPerTargetBone::PlayAnim(UBOOL bLoop,FLOAT Rate,FLOAT StartTime)
{
	// pass on the call to our children
	for (INT i = 1; i < Children.Num(); i++)
	{
		if (Children(i).Anim != NULL)
		{
			Children(i).Anim->PlayAnim(bLoop, Rate, StartTime);
		}
	}
}

void UavaAnimBlendByWeaponHanded::PlayAnim(UBOOL bLoop,FLOAT Rate,FLOAT StartTime)
{
	BYTE	RightHanded = 1;
	if( SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
		if ( PawnOwner )	
			RightHanded = PawnOwner->bRightHandedWeapon;
		else
		{
			UavaUICharacterPIP* PIP = Cast<UavaUICharacterPIP>(SkelComponent->GetOuter());

			if (PIP && PIP->WeaponTemplate)
			{				
				AavaWeaponAttachment* Attachment = Cast<AavaWeaponAttachment>( PIP->WeaponTemplate->ClassDefaultObject );

				RightHanded = Attachment->bRightHandedWeapon;
			}
		}
	}
	// @ToDo Blend Time 은 에디터에서 가져오도록 한다.
	if ( Children( RightHanded ).Anim )
	{
		for( int i = 0 ; i < Children.Num() ; ++ i )
		{
			if ( i == RightHanded )	Children( i ).Weight = 1.0;
			else					Children( i ).Weight = 0.0;
		}
		Children( RightHanded ).Anim->PlayAnim( bLoop, Rate, StartTime );
	}
}

void UavaAnimBlendByWeaponHanded::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	Super::TickAnim(DeltaSeconds,TotalWeight);
}

void UavaAnimBlendByRun::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	float CurrentTickTime;
	AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
	if (TotalWeight>0)
	{
		CurrentTickTime = GWorld->GetWorldInfo()->TimeSeconds;
		if ( ( CurrentTickTime - LastTickTime ) - DeltaSeconds > 0.3f )
		{
			SetActiveChild( 0, GetBlendTime(0,false) );
			NormalModeElapsedTime = 0.0;
		}
		else
		{
			if ( ActiveChildIndex == 0 )
			{
				NormalModeElapsedTime += DeltaSeconds;
				if ( NormalModeElapsedTime > NormalModeTransitionTime )
				{
					SetActiveChild( 1, GetBlendTime(1,false) );
					NormalModeElapsedTime = 0.0;
				}
			}
			else
			{
				FLOAT	DeltaRotation = 0.0f;
				if ( PawnOwner != NULL )
				{
					DeltaRotation	=	Abs( PawnOwner->Rotation.Yaw - LastRotationYaw ) / DeltaSeconds;
				}
				if ( DeltaRotation > MaxRotationDelta )
				{
					SetActiveChild( 0, GetBlendTime(0,false) );
				}
			}

		}
	}
	else
	{
		SetActiveChild( 0, GetBlendTime(0,false) );
		NormalModeElapsedTime = 0.0;
	}

	if ( PawnOwner != NULL )
	{
		LastRotationYaw	  = PawnOwner->Rotation.Yaw;
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

void UavaAnimBlendByExclusiveAnim::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	if( SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
		if ( PawnOwner != NULL )
		{
			if ( PawnOwner->HeavyWeaponType != ActiveChildIndex )
				SetActiveChild( PawnOwner->HeavyWeaponType, GetBlendTime(PawnOwner->HeavyWeaponType,false) );
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

FVector2D UavaAnimBlendLimitedAim::GetAim()
{
	FVector		loc;
	float		yaw, pitch;
	if ( GIsGame )
	{
		if( SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
		{
			AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
			if ( PawnOwner != NULL )
			{
				if ( PawnOwner->Controller != NULL )
				{
					pitch = PawnOwner->Controller->Rotation.Pitch;
					yaw	  = PawnOwner->Controller->Rotation.Yaw;
				}
				else
				{
					pitch = ( PawnOwner->RemoteViewPitch << 8 ) & 65535;
					yaw	  = PawnOwner->RemoteViewYaw;
				}
				if ( pitch > 32768 )	pitch -= 65536;
				Aim.X = -0.5 + (yaw-PawnOwner->MinLimitYawAngle)/(PawnOwner->MaxLimitYawAngle-PawnOwner->MinLimitYawAngle);
				Aim.Y = -0.5 + (pitch-PawnOwner->MinLimitPitchAngle)/(PawnOwner->MaxLimitPitchAngle-PawnOwner->MinLimitPitchAngle);

				Aim.X *= 2.0;
				Aim.Y *= 2.0;
			}
		}
	}
	return Aim;
}

void UavaAnimBlendByDamage::TickAnim( FLOAT DeltaSeconds, FLOAT TotalWeight )
{
	//int	ChildIndex		=	-1;
	//float fStartTime	=	0.0;
	//if ( LastShotInfo != SI_Generic )
	//{
	//	LastShotInfo = SI_Generic;
	//	ChildIndex	 = 1;				// LastShotInfo 를 이용해서 ChildIndex 를 구해줘야 한다...

	//	SetActiveChild( ChildIndex, GetBlendTime( ChildIndex, false ) );
	//	if ( Cast<UAnimNodeSequence>( Children(ChildIndex).Anim ) )
	//	{
	//		UAnimNodeSequence* P = Cast<UAnimNodeSequence>( Children(ChildIndex).Anim );
	//		P->PlayAnim( false, 0.0, LastShotAngle * P->AnimSeq->SequenceLength / 360.0f );
	//	}
	//}

	//if ( DurationTime > 0 )
	//{
	//	DurationTime -= DeltaSeconds;
	//	if ( DurationTime < 0 )
	//	{
	//		DurationTime = 0;
	//		SetActiveChild( 0, GetBlendTime( 0, false ) );
	//	}
	//}

	Super::TickAnim(DeltaSeconds,TotalWeight);	// pass along for now until falls anims are separated	
}

void UavaAnimBlendByClassType::PlayAnim( UBOOL bLoop,FLOAT Rate,FLOAT StartTime )
{
	if( SkelComponent == NULL || SkelComponent->GetOwner() == NULL)
		return;

	AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
	if ( PawnOwner == NULL )
		return;

	// 이런 범위를 가진다면 오류이다.
	if ( PawnOwner->TypeID > 2 )
	{
		debugf( TEXT("Failed UavaAnimBlendByClassType::PlayAnim - PawnOwner->TypeID = %d"), PawnOwner->TypeID );
		return ;
	}

	// 병과에 대해서 애니메이션을 실행한다.
	Children( PawnOwner->TypeID ).Anim->PlayAnim( bLoop, Rate, StartTime );
}

void UavaAnimBlendByClassType::TickAnim( float DeltaSeconds, float TotalWeight )
{
	int ActiveIndex;
	if ( TotalWeight > 0.0 )
	{
		if( SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
		{
			AavaPawn* PawnOwner = Cast<AavaPawn>(SkelComponent->GetOwner());
			if ( PawnOwner != NULL )
			{
				ActiveIndex = PawnOwner->TypeID;
				if ( ActiveIndex != ActiveChildIndex )
				{
					SetActiveChild( ActiveIndex, GetBlendTime(ActiveIndex) );
				}
			}
		}	
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}


void UavaAnimBlendByLadder::TickAnim( float DeltaSeconds, float TotalWeight )
{
	if (SkelComponent != NULL && SkelComponent->GetOwner() != NULL)
	{
		APawn* POwner = SkelComponent->GetOwner()->GetAPawn();
		if ( POwner )
		{
			bool	bFaceLadder = TRUE;
			FLOAT	speed = POwner->Velocity.Z;
			ALadderVolume *LadderVolume = POwner->OnLadder;
			if ( LadderVolume != NULL )
			{
				bFaceLadder = ( POwner->Rotation.Vector() | LadderVolume->LookDir ) > 0.0;
			}
			
			int ActiveIndex = -1;
			if ( speed < KINDA_SMALL_NUMBER && speed > -KINDA_SMALL_NUMBER )	ActiveIndex = 0;
			else 
			{
				if ( bFaceLadder )
				{
					if ( speed > 0 )		ActiveIndex = 1;
					else if ( speed < 0 )	ActiveIndex = 2;
				}
				else
				{
					if ( speed > 0 )		ActiveIndex = 3;
					else if ( speed < 0 )	ActiveIndex = 4;
				}
			}
			if ( ActiveIndex != ActiveChildIndex )
				SetActiveChild( ActiveIndex, GetBlendTime(ActiveIndex) );
		}
	}
	Super::TickAnim(DeltaSeconds, TotalWeight);
}

//! 애니메이션 초기화는 할 수 있어야 하지 않겠는가??
void UavaAnimNodeRandom::ResetActiveChild( INT ChildIndex, FLOAT BlendTime )
{
	// 입력된 Pending으로 설정하고.
	PendingChildIndex = ChildIndex;

	// Set new active child w/ blend
	Super::SetActiveChild(PendingChildIndex, BlendTime);

	// Play the animation if this child is a sequence
	PlayingSeqNode = Cast<UAnimNodeSequence>(Children(ActiveChildIndex).Anim);
	if( PlayingSeqNode )
	{
		FRandomAnimInfo& Info = RandomInfo(ActiveChildIndex);

		FLOAT PlayRate = Lerp(Info.PlayRateRange.X, Info.PlayRateRange.Y, appFrand());
		if( PlayRate < KINDA_SMALL_NUMBER )
		{
			PlayRate = 1.f;
		}
		PlayingSeqNode->PlayAnim(FALSE, PlayRate, 0.f);
	}

	// Pick PendingChildIndex for next time...
	PendingChildIndex = PickNextAnimIndex();
}

/*
FLOAT UavaAnimBlendByClassType::GetSliderPosition(INT SliderIndex, INT ValueIndex)
{
	check(0 == SliderIndex && 0 == ValueIndex);
	return SliderPosition;
}

void UavaAnimBlendByClassType::HandleSliderMove(INT SliderIndex, INT ValueIndex, FLOAT NewSliderValue)
{
	SliderPosition = NewSliderValue;
}

FString UavaAnimBlendByClassType::GetSliderDrawValue(INT SliderIndex)
{
	check(0 == SliderIndex);
	return FString::Printf( TEXT("%0.1f"), SliderPosition );
}
*/