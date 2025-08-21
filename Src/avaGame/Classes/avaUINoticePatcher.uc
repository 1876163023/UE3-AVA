/*
	UI ���� �̹����� HTTP�� �ٿ�ޱ� ���� Ŭ����.

	2007/04/02	����
*/
class avaUINoticePatcher extends Actor
	native;

//! �ٿ�ε��� �Ϸ� ����.
var bool	bDownloaded;

//! �ٿ�ε��� �Ϸ� ����.
var bool	bPreloaded;

//! ini���� �̸�.
var string	IniName;

//! �߰� ���.
var string	SubPath;

//! ���ҽ� ���ξ�.
var string	PrefixName;

//! ��ġ�� ���� �ּ�.
var string	URL;

//! �޸𸮿� ������ �ִ´�.
var array<Texture2D>	ImageCaches;


cpptext
{


}


/*! @brief �������� ���� �˻��� �ٿ�ε带 �����Ѵ�.
	@note
		������� ó���ȴ�.(nonblocking)
*/
native function Download();

//! ����׸� ���� �ð�(appSeconds()) ��� �Լ�. 
final native function float GetSeconds();

//! 2D�� �ؽ��� ������ �����Ѵ�.
native final static function Texture2D LoadImage(string FullFilename);

//! ini ���Ͽ��� �ش� ���� ���´�.
native final static function bool LoadIni(string FullFilename, string Section, string KeyName, out array<string> Values);

//! �ֻ��� ��θ��� ���´�.
native final static function string GetBasePath();

//! ���ڿ��� ������ �����Ѵ�.
native final static function string Trim(string text, string whitespaces, bool bRight=true);


//! ��ü ��θ� ���´�.
function string GetFullPath()
{
	local string FullPath;

	// ������Ʈ������ ��ġ�� ��θ� ���´�.
	FullPath = GetBasePath();

	// ��� ��θ� �߰��Ѵ�.
	if ( Len(SubPath) > 0 )
	{
		FullPath $= "\\";
		FullPath $= SubPath;
	}

	return FullPath;
}


/*! @brief native���� download������ �����ϱ� ���ؼ� ���.
	@note
		�ٸ� �����忡�� �� �Լ��� ȣ��Ǵ� ���� ������ ������ �����ȵ�.
*/
event function OnDownloaded()
{
	`log("### OnDownloaded() - " @ GetSeconds() );

	bDownloaded = true;

	// �� �Լ��� download thread���� ȣ��Ǳ⿡ �ϴ� Ÿ�̸� �Լ��� �۵����Ѽ�
	// ������ thread���� ȣ��ǵ��� ���ش�.
	SetTimer(0.1, false, 'OnPreload');

	// �ٸ� �����忡�� ȣ��ȴٸ�??
	// ����ϰ�~������~!!
//	OnPreload();
//	SetTimer(0.1, false, 'NotifyOwner');
}


//! �̹����� �ٽ� �ε��Ѵ�.
function OnPreload()
{
	local array<string>		values;
	local int				i;
	local string			tmp;

	`log("### OnPreload() - " @ GetSeconds() );

	// �ܺ� ������ �о�鿩�� �����Ѵ�.
	if ( !LoadIni(GetFullPath() $ "\\" $ IniName, "Files", "name", values) )
	{
		`log("Cannot Open " $ GetFullPath() $ "\\" $ IniName);
		return ;
	}

	ImageCaches.Length = 0;

	for ( i = 0; i < values.Length; ++i )
	{
		tmp = Trim(values[i], " \"", TRUE);

		ImageCaches[ImageCaches.Length] = LoadImage(GetFullPath() $ "\\" $ tmp);
	}

	bPreloaded = true;

	// �θ𿡰� �˷��ش�.
	NotifyOwner();
}


//! �θ�ü�� �ٿ�ε�� ���ε��� �Ϸ�Ǿ��ٰ� �뺸�� �ش�.
function NotifyOwner()
{
	local avaNetEntryGameEx	entry;

	// �θ� �ִٸ� ���� �߻�.
	if ( Owner != None )
	{
		entry = avaNetEntryGameEx(Owner);
		if ( entry != None )
			entry.OnDownloaded();
	}
}


DefaultProperties
{
	bDownloaded = false

	IniName = "notice.ini"
	SubPath = "avaGame\\Notice"
	PrefixName = "CustomImage_"
	URL = "http://file.sayclub.co.kr/images/pmang/ava/ob/img/client_img"
}