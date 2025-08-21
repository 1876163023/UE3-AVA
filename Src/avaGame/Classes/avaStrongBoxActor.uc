class avaStrongBoxActor extends avaInterpActor;

var BYTE						PrvProgress;
var BYTE						Progress;		// Animation 진행 정도...
												// 0 이면 Default 상태
												// 1 이면 Animation 시작
												// 2~7 이면 각 Progress 의 완료정도 
												// 8 이면 Progress 가 완료되었음
var MaterialInstanceConstant	NumericMIC[7];
var int							Number[7];
var int							Complete[7];
var float						UpdateTime;


replication
{
	if ( bNetDirty )
		Progress;
}

function Reset()
{
	Super.Reset();
	Progress = 0;
}

simulated event PostBeginPlay()
{
	local int i;
	Super.PostBeginPlay();

	for ( i = 0 ; i < 7 ; ++ i )
	{
		NumericMIC[i] = StaticMeshComponent.CreateMaterialInstance( 8 - i );
	}
	//	MIC 를 설정한다...
}

function SetProgress( int NextProgress )
{
	Progress = NextProgress;
	ForceNetRelevant();
}

simulated event Tick( float DeltaTime )
{	
	local int	i;
	local bool	bUpdate;
	Super.Tick( DeltaTime );

	UpdateTime -= DeltaTime;
	bUpdate		= false;
	if ( UpdateTime <= 0 )
	{
		UpdateTime = default.UpdateTime;
		bUpdate = true;
	}

	if ( Progress == 0 )
	{
		if ( PrvProgress != 0 )
		{
			for ( i = 0 ; i < 7 ; ++ i )
				SetNumber( i, 8 );
		}
	}
	else if ( Progress > 0 && Progress <= 8 )
	{
		for ( i = 0 ; i < 7 ; ++ i )
		{
			if ( Progress - 2 >= i )
			{
				// Set Number
				SetNumber( i, Number[i], true );
			}
			else
			{
				if ( bUpdate )
					SetNumber( i, Rand( 10 ) );
			}
		}
	}

	PrvProgress = Progress;
}

simulated function SetNumber( int nGroup, int nNumber, optional bool bComplete )
{
	local float U,V;
	if ( Complete[nGroup] == 1 && bComplete )	return;
	Complete[nGroup] = int( bComplete );
	U = 0.0625 * float( nNumber );
	V = 0.0;
	NumericMIC[nGroup].SetVectorParameterValue( 'UV', MakeLinearColor( U,V,0.0,1.0 ) );
}

defaultproperties
{
	PrvProgress =	255
	Progress	=	0

	Number(0)	=	5
	Number(1)	=	1	
	Number(2)	=	4
	Number(3)	=	7
	Number(4)	=	0
	Number(5)	=	0
	Number(6)	=	2

	UpdateTime	=	0.1
}