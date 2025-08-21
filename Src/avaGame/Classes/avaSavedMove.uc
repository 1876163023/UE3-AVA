class avaSavedMove extends SavedMove;

var bool	bDash;

function Clear()
{
	Super.Clear();
	bDash = false;
}

function bool CanCombineWith(SavedMove NewMove, Pawn InPawn, float MaxDelta)
{
	return Super.CanCombineWith( NewMove, InPawn, MaxDelta ) && (bDash == avaSavedMove( NewMove ).bDash );
}

function SetMoveFor(PlayerController P, float DeltaTime, vector NewAccel, EDoubleClickDir InDoubleClick)
{
	Super.SetMoveFor( P, DeltaTime, NewAccel, InDoubleClick );
	bDash = ( avaPlayerController( P ).bDash >  0 );
}

function byte CompressedFlags()
{
	return (Super.CompressedFlags() & ~64) + bDash ? 64 : 0;
}

static function EDoubleClickDir SetFlags(byte Flags, PlayerController PC)
{
	if ( ( Flags & 64 ) != 0 )
		avaPlayerController(PC).bDash = 1;
	else
		avaPlayerController(PC).bDash = 0;

	return Super.SetFlags( Flags & ~64, PC );
}
