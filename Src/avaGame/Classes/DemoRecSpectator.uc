//=============================================================================
// DemoRecSpectator - spectator for demo recordings to replicate ClientMessages
//=============================================================================

class DemoRecSpectator extends avaPlayerController;

var bool bTempBehindView;
var bool bFoundPlayer;
var string RemoteViewTarget;	// Used to track targets without a controller

simulated event PostBeginPlay()
{
	// We're currently doing demo recording
	if( Role == ROLE_Authority && WorldInfo.Game != None )
	{
		ClientSetHUD(WorldInfo.Game.HUDType, WorldInfo.Game.ScoreBoardType);
	}
	
	`Log(self);

	Super.PostBeginPlay();

	if ( PlayerReplicationInfo != None )
		PlayerReplicationInfo.bOutOfLives = true;
}

function InitPlayerReplicationInfo()
{
	Super.InitPlayerReplicationInfo();
	PlayerReplicationInfo.PlayerName="DemoRecSpectator";
	PlayerReplicationInfo.bIsSpectator = true;
	PlayerReplicationInfo.bOnlySpectator = true;
	PlayerReplicationInfo.bOutOfLives = true;
	PlayerReplicationInfo.bWaitingPlayer = false;
}

exec function ViewClass( class<actor> aClass, optional bool bQuiet, optional bool bCheat )
{
	local actor other, first;
	local bool bFound;

	first = None;

	ForEach AllActors( aClass, other )
	{
		if ( bFound || (first == None) )
		{
			first = other;
			if ( bFound )
				break;
		}
		if ( other == ViewTarget )
			bFound = true;
	}

	if ( first != None )
	{
		SetViewTarget(first);
		bBehindView = ( ViewTarget != self );

		if ( bBehindView )
			ViewTarget.BecomeViewTarget(self);
	}
	else
		SetViewTarget(self);
}

//==== Called during demo playback ============================================

exec function DemoViewNextPlayer()
{
    local Pawn P, Pick;
    local bool bFound;

    // view next player
/*
    if ( PlayerController(RealViewTarget) != None )
		PlayerController(RealViewTarget).DemoViewer = None;
*/
	`Log(GetFuncName());

	foreach DynamicActors(class'Pawn', P)
	{
		`Log("1" @ P);
		if (Pick == None)
		{
			Pick = P;
		}
		if (bFound)
		{
			Pick = P;
			break;
		}
		else
		{
			bFound = (RealViewTarget == P.PlayerReplicationInfo || ViewTarget == P);
		}
	}

	`Log("2" @ Pick);

    SetViewTarget(Pick);
    
    `Log(ViewTarget);
    
/*
    if ( PlayerController(RealViewTarget) != None )
		PlayerController(RealViewTarget).DemoViewer = self;
*/
}

auto state Spectating
{
    exec function Fire( optional float F )
    {
        bBehindView = false;
        demoViewNextPlayer();
    }

    exec function AltFire( optional float F )
    {
        bBehindView = !bBehindView;
    }

	event PlayerTick( float DeltaTime )
	{
		Super.PlayerTick( DeltaTime );

		// attempt to find a player to view.
		if( Role == ROLE_AutonomousProxy && (RealViewTarget==None || RealViewTarget==PlayerReplicationInfo) && !bFoundPlayer )
		{
			DemoViewNextPlayer();
			if( RealViewTarget!=None && RealViewTarget!=PlayerReplicationInfo)
				bFoundPlayer = true;
		}

/*
		// hack to go to 3rd person during deaths
		if( RealViewTarget!=None && RealViewTarget.Pawn==None )
		{
			if (!bBehindview)
			{
				if( !bTempBehindView )
				{
					bTempBehindView = true;
					bBehindView = true;
				}
			}
		}
		else
		if( bTempBehindView )
		{
			bBehindView = false;
			bTempBehindView = false;
		}
*/
	}
}

/*
event GetPlayerViewPoint(out vector CameraLocation, out rotator CameraRotation)
{
	local Rotator R;

	if( RealViewTarget != None )
	{
		R = RealViewTarget.Rotation;
	}

	Super.GetPlayerViewPoint(CameraLocation, CameraRotation );

	if( RealViewTarget != None )
	{
		if ( !bBehindView )
		{
			CameraRotation = R;
			if ( Pawn(ViewTarget) != None )
				CameraLocation.Z += Pawn(ViewTarget).BaseEyeHeight; // FIXME TEMP
		}
		RealViewTarget.SetRotation(R);
	}
}
*/

defaultproperties
{
	RemoteRole=ROLE_AutonomousProxy
	bDemoOwner=1
}

