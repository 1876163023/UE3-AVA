/*==========================================================================

	Instigator 가 해당 Inventory 를 가지고 있는지를 Check 한다...


==========================================================================*/
class avaSeqAct_CheckInventory extends SequenceAction;

var() class<avaWeapon>	CheckInventory;

event Activated()
{
	local SeqVar_Object ObjVar;
	local Pawn			P;
	local Controller	C;
	local bool			bHas;

	bHas = false;

	if ( CheckInventory != None )
	{
		foreach LinkedVariables( class'SeqVar_Object', ObjVar, "Target" )
		{
			P = Pawn( ObjVar.GetObjectValue() );
			if ( P == None )
			{
				C = Controller( ObjVar.GetObjectValue() );
				if ( C != None )
				{
					P = C.Pawn;
				}
			}

			if ( P == None )	continue;

			if ( P.FindInventoryType( CheckInventory ) != None )
			{
				bHas = true;
				break;
			}
		}
	}

	GenerateImpulse( bHas );
}

function GenerateImpulse( bool bHas )
{
	if ( bHas )	OutputLinks[0].bHasImpulse = true;
	else		OutputLinks[1].bHasImpulse = true;
}

defaultproperties
{
	bCallHandler				=	false
	ObjCategory					=	"Pawn"
	ObjName						=	"Check Inventory"
	bAutoActivateOutputLinks	=	false
	
	OutputLinks(0)				=	(LinkDesc="Yes")
	OutputLinks(1)				=	(LinkDesc="No")
}
