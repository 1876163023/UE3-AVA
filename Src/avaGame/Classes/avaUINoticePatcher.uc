/*
	UI 공지 이미지를 HTTP로 다운받기 위한 클래스.

	2007/04/02	고광록
*/
class avaUINoticePatcher extends Actor
	native;

//! 다운로드의 완료 유무.
var bool	bDownloaded;

//! 다운로드의 완료 유무.
var bool	bPreloaded;

//! ini파일 이름.
var string	IniName;

//! 추가 경로.
var string	SubPath;

//! 리소스 접두어.
var string	PrefixName;

//! 패치할 서버 주소.
var string	URL;

//! 메모리에 가지고 있는다.
var array<Texture2D>	ImageCaches;


cpptext
{


}


/*! @brief 서버에서 버전 검사후 다운로드를 시작한다.
	@note
		스레드로 처리된다.(nonblocking)
*/
native function Download();

//! 디버그를 위한 시간(appSeconds()) 얻는 함수. 
final native function float GetSeconds();

//! 2D용 텍스쳐 파일을 적재한다.
native final static function Texture2D LoadImage(string FullFilename);

//! ini 파일에서 해당 값을 얻어온다.
native final static function bool LoadIni(string FullFilename, string Section, string KeyName, out array<string> Values);

//! 최상위 경로명을 얻어온다.
native final static function string GetBasePath();

//! 문자열의 공백을 제거한다.
native final static function string Trim(string text, string whitespaces, bool bRight=true);


//! 전체 경로를 얻어온다.
function string GetFullPath()
{
	local string FullPath;

	// 레지스트리에서 설치된 경로를 얻어온다.
	FullPath = GetBasePath();

	// 상대 경로를 추가한다.
	if ( Len(SubPath) > 0 )
	{
		FullPath $= "\\";
		FullPath $= SubPath;
	}

	return FullPath;
}


/*! @brief native에서 download유무를 설정하기 위해서 사용.
	@note
		다른 스레드에서 이 함수가 호출되는 것이 문제가 없는지 검증안됨.
*/
event function OnDownloaded()
{
	`log("### OnDownloaded() - " @ GetSeconds() );

	bDownloaded = true;

	// 이 함수는 download thread에서 호출되기에 일단 타이머 함수를 작동시켜서
	// 원래의 thread에서 호출되도록 해준다.
	SetTimer(0.1, false, 'OnPreload');

	// 다른 스레드에서 호출된다면??
	// 깔끔하게~뻑난다~!!
//	OnPreload();
//	SetTimer(0.1, false, 'NotifyOwner');
}


//! 이미지를 다시 로딩한다.
function OnPreload()
{
	local array<string>		values;
	local int				i;
	local string			tmp;

	`log("### OnPreload() - " @ GetSeconds() );

	// 외부 파일을 읽어들여서 적용한다.
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

	// 부모에게 알려준다.
	NotifyOwner();
}


//! 부모객체에 다운로드와 선로딩이 완료되었다고 통보해 준다.
function NotifyOwner()
{
	local avaNetEntryGameEx	entry;

	// 부모가 있다면 직접 발생.
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