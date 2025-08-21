/*
	캐릭터 등의 자주 사용하는 오브젝트(Mesh & Texture)를 등록해 두는 클래스.

	2007/04/18	고광록
		Actor를 상속받으면 World에 종속되기 때문에 부모를 Object로 교체.
*/
class avaCache extends Object
	native;

//! 등록된 객체와 이름을 한쌍으로 가지고 있는 구조체.
struct native ObjectPair
{
	var Object	ObjPtr;
	var string	ObjName;
};

//! 등록된 객체들.
var array<ObjectPair>	ObjList;

//! 텍스쳐도 적재할지 유무.
var bool				bCacheTexture;

/*! @brief 리스트에 오브젝트를 적재해서 추가한다.
	@param ObjName
		오브젝트 이름.
	@param
		오브젝트를 생성할 클래스 타입.
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

	// 이렇게 하면 어떻게 되나?
//	ObjClass = class'Object';

	// 같은 이름으로 이미 등록된 오브젝트가 있다면 제외.
	for ( i = 0; i < ObjList.Length; ++i )
	{
		if ( ObjList[i].ObjName == ObjName )
		{
			`log("# already loaded object - " @ObjName);
			return ;
		}
	}

	// 로딩해준다.
	Obj = DynamicLoadObject( ObjName, ObjClass );
	if ( Obj == None )
		return ;

	// 추가한다.
	ObjPair.ObjPtr  = Obj;
	ObjPair.ObjName = ObjName;
	ObjList.AddItem( ObjPair );

	// Texture 적재 유무.
	if( bCacheTexture )
	{
		// Texture 정보도 미리 읽어준다.
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

//! 재질에서 텍스쳐를 갱신(생성)한다.
function UpdateMaterial(MaterialInstance MI)
{
	local Material					tmpMtl;
	local MaterialInstanceConstant	tmpMIC;
	local array<Texture>			Textures;
	local int						i;

	tmpMtl = Material(MI);
	tmpMIC = MaterialInstanceConstant(MI);

	// 종류에 맞춰서 추가.
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

//! 외부에서 사용할 등록 함수.
static function RegisterObject(string ObjName, class ObjClass)
{
	class'avaCache'.static.GetInstance().AddObject(ObjName, ObjClass);
}

//! 등록된 오브젝트의 개수를 얻는 함수.
static function int GetCount()
{
	return class'avaCache'.static.GetInstance().ObjList.Length;
}

//! singleton object.
native static function avaCache GetInstance();

DefaultProperties
{
	// 안해줘도 메모리를 똑같이 사용한다면... 안해줘도 되는건가?? @_@a
	bCacheTexture = false;
}