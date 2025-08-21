/*
	인터넷에 있는 이미지를 다운로드 하여 그려주는 클래스.

	2007/06/23 고광록
		LTCG버전에서는 PNG를 따로 읽어줘야 해서 추가되었고,
		Download처리를 합쳐 놓음.

	2007/03/26 고광록
*/
class avaUIComp_DrawCustomImage extends UIComp_DrawImage
	native;

//! ini파일에서 얻은 사용자 이미지 정보를 저장할 구조체.
struct native CustomImageInfo
{
	var() transient editconst string	ImageName;	//!< 그림 이름.(from ini-file).
	var() transient editconst float		Delay;		//!< 지연 시간.(from ini-file).

	var() transient editconst Texture2D	Tex;		//!< 내부에서 생성.
};

cpptext
{
	/* === UIComp_DrawImage interface === */
	/**
	 * Renders the image.  The owning widget is responsible for applying any transformations to the canvas
	 * prior to rendering this component.
	 *
	 * @param	Canvas		the canvas to render the image to
	 * @param	Parameters	the bounds for the region that this texture can render to.
	 */
	virtual void RenderComponent( class FCanvas* Canvas, FRenderParameters Parameters );

	/* === UObject interface === */
	/**
	 * Called when a property value has been changed in the editor.
	 */
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Called after this object has been completely de-serialized.
	 *
	 * This version migrates the ImageCoordinates value over to the StyleCustomization member.
	 */
	virtual void PostLoad();

	/**
	 * Applies the current style data (including any style data customization which might be enabled) to the component's image.
	 */
	void RefreshAppliedStyleData();

	/*! @brief 서버에서 버전 검사후 다운로드를 시작한다.
		@note
			스레드로 처리된다.(nonblocking)
	*/
	void Download();
}

//! ini 파일
var(CustomImage) string						IniName<Tooltip=ini file name>;

//! 애니메이션  정보.
var(CustomImage) transient editconst array<CustomImageInfo>	ImageInfos;

//! 상대 경로.(Registry의 경로에 추가됨)
var(CustomImage) string						SubPath<Tooltip=file path>;

//! 자원 접두어.
var(CustomImage) string						PrefixName;

//! URL Prefix
var(CustomImage) string						URLPrefix;

//! Registry에서 얻어온 경로 정보.
var(CustomImage) transient editconst string	FullPath;

//! 실시간으로 ImageName의 파일에 대해서 갱신한다.
var	transient UITexture						CustomImageRef;

//! 내부에서 생성.
var transient Texture2D						CustomImage;

//! 다운로드의 완료 유무.
//	var transient bool							bDownloaded;
//! 버전이 달라서 새로 다운로드를 했다면...
var transient bool							bNeedUpdate;
//! 이미 다운로딩 중인가?
var transient bool							bDownloading;

var transient float							LocalTime;
var transient int							CurrentInfo;
var transient bool							bInitStyleData;

/*! @brief Registry에서 경로명을 얻어온다.
	@return
		만약 정보를 얻어오지 못한다면 false를 리턴한다.
*/
native final function string GetBasePath();

/**
 * Changes the image for this component, creating the wrapper UITexture if necessary.
 *
 * @param	NewImage		the new texture or material to use in this component
 */
native final function SetCustomImage( Surface NewImage );

//! 2D용 텍스쳐 파일을 적재한다.
native final function Texture2D LoadImage(string FullFilename);

//! ini 파일에서 해당 값을 얻어온다.
native final function bool LoadIni(string FullFilename, string Section, string KeyName, out array<string> Values);

//-----------------------------------------------------------------------------
// 문자열 관련 유틸리티.(생각같아선 FString에 넣어 주고도 싶지만...)
//-----------------------------------------------------------------------------

//! 문자열을 구분자로 나눠 준다.(단, 구분자는 제외된다)
native final function int Tokenize(string text, string delims, out array<string> Values);
//! 문자열의 공백을 제거한다.
native final function string Trim(string text, string whitespaces, bool bRight=true);


//! 전체 경로를 얻어온다.
function string GetFullPath()
{
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

//! IniName이 바뀌면 호출되는 함수.
event function OnChangeIniName()
{
	local array<string>		values;
	local array<string>		parts;
	local array<string>		nameValues;
	local int				i;
	local CustomImageInfo	Info;
	local string			tmp;

	if ( Len(IniName) == 0 )
		return;

	// 외부 파일을 읽어들여서 적용한다.
	if ( !LoadIni(GetFullPath() $ "\\" $ IniName, "Notice", "+Image", values) )
	{
		`log("Cannot Open " $ GetFullPath() $ "\\" $ IniName);
		return ;
	}

	// 초기화.
	ImageInfos.Length = 0;

	// 각각의 그림정보와 지연시간을 읽어서 추가한다.
	// TMultiMap에서 같은 키값("+Image")에 대해서 더 앞으로 추가되는 듯 싶어서
	// 일단 역순으로 처리해서 해결해 둠.
	for ( i = values.Length - 1; i >= 0; --i )
	{
		tmp = Trim(values[i], " ()", TRUE);

		Tokenize(tmp, ",", parts);
		// 파라미터는 2개이다.
		if ( parts.Length != 2 )
			continue;

		Tokenize(parts[0], "=", nameValues);
		if ( nameValues.Length != 2 )
			continue;

		Info.ImageName = Trim(nameValues[1], " \"", TRUE);

		Tokenize(parts[1], "=", nameValues);
		if ( nameValues.Length != 2 )
			continue;

		Info.Delay = float(Trim(nameValues[1], " ", TRUE));

		ImageInfos[ImageInfos.Length] = Info;

		`log("value[" @i @"]" @values[i] @"Info.ImageName=" @Info.ImageName @"Info.Delay=" @Info.Delay);
	}

	// 각각의 모든 텍스쳐를 읽어들인다.
	for ( i = 0; i < ImageInfos.Length; ++i )
		ImageInfos[i].Tex = LoadImage(GetFullPath() $ "\\" $ ImageInfos[i].ImageName);

	// 기본 텍스쳐 설정.
	if ( ImageInfos.Length > 0 )
		SetCustomImage(ImageInfos[0].Tex);
}

DefaultProperties
{
	// default로 생성되는 객체에도 PostEditChange함수가 호출되면서
	// 자동으로 OnChangeIniName() 함수에 의해 오류 처리나는 것을 막기 위해서
	// 기본값은 없는 걸로 한다.
//	IniName = "notice.ini"

	SubPath = "avaGame\\Web\\Notice"
	PrefixName = "CustomImage_"
	// Neowiz에서 전달받은 URL.(2007/06/23)
	URLPrefix = "http://mm.sayclub.com/pmang/ava/client_img"
//	URLPrefix = "http://loozend.cafe24.com/avanotice1"
}
