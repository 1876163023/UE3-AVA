/*=============================================================================
  avaModifier
 
  Copyright 2006 RedDuck Studio, Inc. All Rights Reserved.
 
	2006/07/20 by OZ
		
		Item, Weapon Customize, Character Part 등등을 위한 Modifier 이다...

=============================================================================*/
class avaModifier extends object
	native; // [+] 20070212 dEAthcURe|HM native 추가
	//abstract; //[-] 20070212 dEAthcURe|HM	

struct native AttachedItem // [!] 20070212 dEAthcURe|HM native 추가
{
	var string		MeshName;
	var name		PrimarySocket;
	var name		SecondarySocket;
	var float		MaxVisibleDistance;			// 이 이상 넘어가면 안 찍음, 0 일 경우는 항상 찍는다...	

	structdefaultproperties
	{
		MaxVisibleDistance = 300
	}
};

struct native ExtraMesh // [!] 20070212 dEAthcURe|HM native 추가
{
	var string		MeshName;
	var	float		MaxVisibleDistance;			// 이 이상 넘어가면 안 찍음, 0 일 경우는 항상 찍는다...
};

var int		ID;									// Server 와 교신하기 위한 id, 1부터 65535 까지 사용할 수 있음
var int		Priority;

var array<AttachedItem>	CommonAttachedItems;	// 팀및 병과 구분없이 Socket 에 부착되는 Static Mesh 들
var array<ExtraMesh>	CommonExtraMeshes;		// 팀및 병과 구분없이 Root Bone 에 Add 되는 Skeletal Mesh 들

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