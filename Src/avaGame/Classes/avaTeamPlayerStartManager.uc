class avaTeamPlayerStartManager extends NavigationPoint
	native 
	placeable;

cpptext
{
	virtual void PostEditChange( UProperty* PropertyThatChanged );
	virtual void Spawned();
}


var() ETeamType InitialTeamNumber;
var() bool bInitialEnabled;
var() byte ManagerGroup;     // 0 는 쓰면 안 됨.

// 아래의 두 값은 replication 되지만, 그에 따라서 avaTeamPlayerStart 를
// 적절하게 수정하지는 않는다.
var ETeamType TeamNumber;
var bool bEnabled;

var array<avaTeamPlayerStart> PlayerStarts; // 이거랑 관련이 있는 playerstart 목록.

// sprites used for this actor in the editor, depending on which team it's on
var array<Texture2D> TeamSprites;

replication
{
	if ( Role == ROLE_Authority && bNetDirty )
		TeamNumber, bEnabled;
}

simulated function PostBeginPlay()
{
	`log( "avaTeamPlayerStartManager.PostBeginPlay" @self );
	super.PostBeginPlay();

	BuildPlayerStarts();

	SetTeamNum( InitialTeamNumber );
	EnablePSM( bInitialEnabled );
}


// reset actor to initial state.
function Reset()
{
	super.Reset();

	SetTeamNum( InitialTeamNumber );
	EnablePSM( bInitialEnabled );
}

simulated function ClientReset();

function BuildPlayerStarts()
{
	local avaTeamPlayerStart P;
	foreach WorldInfo.AllNavigationPoints( class'avaTeamPlayerStart', P )
	{
		if ( P.ManagerGroup != 0 && P.ManagerGroup == ManagerGroup )
		{
			PlayerStarts[ PlayerStarts.Length ] = P;
		}
	}

	if ( PlayerStarts.Length == 0 )
	{
		`warn( self$" has no managing playerstarts." );
	}
	else
	{
		`log( self$" has "$PlayerStarts.Length$" player starts for "$InitialTeamNumber );
	}
}

/* ====================================================	
* ::OnToggle
*
* Scripted support for toggling a PSM, checks which
* operation to perform by looking at the action input.
*
* Input 1: turn on
* Input 2: turn off
* Input 3: toggle
*
* =====================================================
*/
simulated function OnToggle( SeqAct_Toggle action )
{
	if (action.InputLinks[0].bHasImpulse)
	{
		// turn on
		EnablePSM( true );
	}
	else
	if (action.InputLinks[1].bHasImpulse)
	{
		// turn off
		EnablePSM( false );
	}
	else
	if (action.InputLinks[2].bHasImpulse)
	{
		// toggle
		EnablePSM( !bEnabled );
	}
}

function EnablePSM( bool enable )
{
	local int i;
	local avaTeamPlayerStart P;

	bEnabled = enable;

	for( i=0; i<PlayerStarts.Length; ++ i )
	{
		P = PlayerStarts[i];
		P.bEnabled   = enable;
	}
}

function bool IsEnabled()
{
	return bEnabled;
}

function SetTeamNum( ETeamType num )
{
	local int i;
	local avaTeamPlayerStart P;

	TeamNumber = num;

	for( i=0; i<PlayerStarts.Length; ++ i )
	{
		P = PlayerStarts[i];
		P.TeamNumber = num;
	}
}

simulated function byte GetTeamNum()
{
	return TeamNumber;
}



defaultproperties
{
	TeamSprites[0]=Texture2D'EnvyEditorResources.S_Players_Red'
	TeamSprites[1]=Texture2D'EnvyEditorResources.S_Players_Blue'
	TeamSprites[2]=Texture2D'EnvyEditorResources.S_Players_Gray'

	bInitialEnabled=true
	InitialTeamNumber=TEAM_EU
	ManagerGroup = 1
}
