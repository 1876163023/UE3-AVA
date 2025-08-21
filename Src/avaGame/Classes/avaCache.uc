/*
	ĳ���� ���� ���� ����ϴ� ������Ʈ(Mesh & Texture)�� ����� �δ� Ŭ����.

	2007/04/18	����
		Actor�� ��ӹ����� World�� ���ӵǱ� ������ �θ� Object�� ��ü.
*/
class avaCache extends Object
	native;

//! ��ϵ� ��ü�� �̸��� �ѽ����� ������ �ִ� ����ü.
struct native ObjectPair
{
	var Object	ObjPtr;
	var string	ObjName;
};

//! ��ϵ� ��ü��.
var array<ObjectPair>	ObjList;

//! �ؽ��ĵ� �������� ����.
var bool				bCacheTexture;

/*! @brief ����Ʈ�� ������Ʈ�� �����ؼ� �߰��Ѵ�.
	@param ObjName
		������Ʈ �̸�.
	@param
		������Ʈ�� ������ Ŭ���� Ÿ��.
*/
function AddObject(string ObjName, class ObjClass)
{
	local Object		Obj;
	local StaticMesh	TmpStaticMesh;
	local SkeletalMesh	TmpSkelMesh;
	local int			i, j;
	local ObjectPair	ObjPair;

	if ( ObjName == "" )
		return ;

	// �̷��� �ϸ� ��� �ǳ�?
//	ObjClass = class'Object';

	// ���� �̸����� �̹� ��ϵ� ������Ʈ�� �ִٸ� ����.
	for ( i = 0; i < ObjList.Length; ++i )
	{
		if ( ObjList[i].ObjName == ObjName )
		{
			`log("# already loaded object - " @ObjName);
			return ;
		}
	}

	// �ε����ش�.
	Obj = DynamicLoadObject( ObjName, ObjClass );
	if ( Obj == None )
		return ;

	// �߰��Ѵ�.
	ObjPair.ObjPtr  = Obj;
	ObjPair.ObjName = ObjName;
	ObjList.AddItem( ObjPair );

	// Texture ���� ����.
	if( bCacheTexture )
	{
		// Texture ������ �̸� �о��ش�.
		if ( ObjClass == class'SkeletalMesh' )
		{
			TmpSkelMesh = SkeletalMesh(Obj);
			if ( TmpSkelMesh == None )
				return ;

			for ( i = 0; i < TmpSkelMesh.Materials.Length; ++i )
				UpdateMaterial( TmpSkelMesh.Materials[i] );
		}
		else if ( ObjClass == class'StaticMesh' )
		{
			TmpStaticMesh = StaticMesh(Obj);
			if ( TmpStaticMesh == None )
				return ;

			for ( i = 0; i < TmpStaticMesh.LODInfo.Length; ++i )
			{
				for ( j = 0; j < TmpStaticMesh.LODInfo[i].Elements.Length; ++j )
					UpdateMaterial( TmpStaticMesh.LODInfo[i].Elements[j].Material );
			}
		}
	}

	`log("### CacheObject :" @ObjName);
}

//! �������� �ؽ��ĸ� ����(����)�Ѵ�.
function UpdateMaterial(MaterialInstance MI)
{
	local Material					tmpMtl;
	local MaterialInstanceConstant	tmpMIC;
	local array<Texture>			Textures;
	local int						i;

	tmpMtl = Material(MI);
	tmpMIC = MaterialInstanceConstant(MI);

	// ������ ���缭 �߰�.
	if ( tmpMtl != None )
	{
		Textures = tmpMtl.GetTextures();

		class'avaUtil'.static.UpdateTextures(Textures);
	}
	else if ( tmpMIC != None )
	{
		Textures.Length = 0;

		for ( i = 0; i < tmpMIC.TextureParameterValues.Length; ++i )
			Textures.AddItem( tmpMIC.TextureParameterValues[i].ParameterValue );

		class'avaUtil'.static.UpdateTextures(Textures);
	}
}

//! �ܺο��� ����� ��� �Լ�.
static function RegisterObject(string ObjName, class ObjClass)
{
	class'avaCache'.static.GetInstance().AddObject(ObjName, ObjClass);
}

//! ��ϵ� ������Ʈ�� ������ ��� �Լ�.
static function int GetCount()
{
	return class'avaCache'.static.GetInstance().ObjList.Length;
}

//! singleton object.
native static function avaCache GetInstance();

DefaultProperties
{
	// �����൵ �޸𸮸� �Ȱ��� ����Ѵٸ�... �����൵ �Ǵ°ǰ�?? @_@a
	bCacheTexture = false;
}