/*=============================================================================
  avaModifier
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/20 by OZ
		
		Item, Weapon Customize, Character Part ����� ���� Modifier �̴�...

=============================================================================*/
class avaModifier extends object
	native; // [+] 20070212 dEAthcURe|HM native �߰�
	//abstract; //[-] 20070212 dEAthcURe|HM	

struct native AttachedItem // [!] 20070212 dEAthcURe|HM native �߰�
{
	var string		MeshName;
	var name		PrimarySocket;
	var name		SecondarySocket;
	var float		MaxVisibleDistance;			// �� �̻� �Ѿ�� �� ����, 0 �� ���� �׻� ��´�...	

	structdefaultproperties
	{
		MaxVisibleDistance = 300
	}
};

struct native ExtraMesh // [!] 20070212 dEAthcURe|HM native �߰�
{
	var string		MeshName;
	var	float		MaxVisibleDistance;			// �� �̻� �Ѿ�� �� ����, 0 �� ���� �׻� ��´�...
};

var int		ID;									// Server �� �����ϱ� ���� id, 1���� 65535 ���� ����� �� ����
var int		Priority;

var array<AttachedItem>	CommonAttachedItems;	// ���� ���� ���о��� Socket �� �����Ǵ� Static Mesh ��
var array<ExtraMesh>	CommonExtraMeshes;		// ���� ���� ���о��� Root Bone �� Add �Ǵ� Skeletal Mesh ��

static function ApplyToCharacter_Client( avaPawn Pawn );
static function ApplyToCharacter_Server( avaPawn Pawn );

static function ApplyToWeapon_Client( avaWeapon Weapon );
static function ApplyToWeapon_Server( avaWeapon Weapon );

static function DLO( string resource, out array<object> outList )
{
	local Object	obj;
	if (resource != "")
	{		
		`log( "DLO - "@resource );
		obj = DynamicLoadObject( resource, class'Object' );
		if ( obj != None )
		{
			outList.length = outList.length + 1;
			outList[outList.length-1] = obj;
		}
	}
}

static simulated function PreCache( out array<object> outlist )
{
	local int i;
	Super.PreCache( outList );
	for ( i = 0 ; i < default.CommonAttachedItems.length ; ++ i )
	{
		DLO( default.CommonAttachedItems[i].MeshName, outList );
		DLO( default.CommonAttachedItems[i].MeshName$"_3p", outList );
	}

	for ( i = 0 ; i < default.CommonExtraMeshes.length ; ++ i )
	{
		DLO( default.CommonExtraMeshes[i].MeshName, outList );
	}
}

static event LoadDLOs()
{
	local array<object>	outlist;
	PreCache( outList );
}

// {{ [+] 20070212 dEAthcURe|HM
cpptext
{
	#ifdef EnableHostMigration
	virtual void hmSerialize(FArchive& Ar);
	#endif
}
// }} [+] 20070212 dEAthcURe|HM