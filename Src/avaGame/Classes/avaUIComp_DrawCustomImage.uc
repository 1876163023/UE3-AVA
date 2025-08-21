/*
	���ͳݿ� �ִ� �̹����� �ٿ�ε� �Ͽ� �׷��ִ� Ŭ����.

	2007/06/23 ����
		LTCG���������� PNG�� ���� �о���� �ؼ� �߰��Ǿ���,
		Downloadó���� ���� ����.

	2007/03/26 ����
*/
class avaUIComp_DrawCustomImage extends UIComp_DrawImage
	native;

//! ini���Ͽ��� ���� ����� �̹��� ������ ������ ����ü.
struct native CustomImageInfo
{
	var() transient editconst string	ImageName;	//!< �׸� �̸�.(from ini-file).
	var() transient editconst float		Delay;		//!< ���� �ð�.(from ini-file).

	var() transient editconst Texture2D	Tex;		//!< ���ο��� ����.
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

	/*! @brief �������� ���� �˻��� �ٿ�ε带 �����Ѵ�.
		@note
			������� ó���ȴ�.(nonblocking)
	*/
	void Download();
}

//! ini ����
var(CustomImage) string						IniName<Tooltip=ini file name>;

//! �ִϸ��̼�  ����.
var(CustomImage) transient editconst array<CustomImageInfo>	ImageInfos;

//! ��� ���.(Registry�� ��ο� �߰���)
var(CustomImage) string						SubPath<Tooltip=file path>;

//! �ڿ� ���ξ�.
var(CustomImage) string						PrefixName;

//! URL Prefix
var(CustomImage) string						URLPrefix;

//! Registry���� ���� ��� ����.
var(CustomImage) transient editconst string	FullPath;

//! �ǽð����� ImageName�� ���Ͽ� ���ؼ� �����Ѵ�.
var	transient UITexture						CustomImageRef;

//! ���ο��� ����.
var transient Texture2D						CustomImage;

//! �ٿ�ε��� �Ϸ� ����.
//	var transient bool							bDownloaded;
//! ������ �޶� ���� �ٿ�ε带 �ߴٸ�...
var transient bool							bNeedUpdate;
//! �̹� �ٿ�ε� ���ΰ�?
var transient bool							bDownloading;

var transient float							LocalTime;
var transient int							CurrentInfo;
var transient bool							bInitStyleData;

/*! @brief Registry���� ��θ��� ���´�.
	@return
		���� ������ ������ ���Ѵٸ� false�� �����Ѵ�.
*/
native final function string GetBasePath();

/**
 * Changes the image for this component, creating the wrapper UITexture if necessary.
 *
 * @param	NewImage		the new texture or material to use in this component
 */
native final function SetCustomImage( Surface NewImage );

//! 2D�� �ؽ��� ������ �����Ѵ�.
native final function Texture2D LoadImage(string FullFilename);

//! ini ���Ͽ��� �ش� ���� ���´�.
native final function bool LoadIni(string FullFilename, string Section, string KeyName, out array<string> Values);

//-----------------------------------------------------------------------------
// ���ڿ� ���� ��ƿ��Ƽ.(�������Ƽ� FString�� �־� �ְ� ������...)
//-----------------------------------------------------------------------------

//! ���ڿ��� �����ڷ� ���� �ش�.(��, �����ڴ� ���ܵȴ�)
native final function int Tokenize(string text, string delims, out array<string> Values);
//! ���ڿ��� ������ �����Ѵ�.
native final function string Trim(string text, string whitespaces, bool bRight=true);


//! ��ü ��θ� ���´�.
function string GetFullPath()
{
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

//! IniName�� �ٲ�� ȣ��Ǵ� �Լ�.
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

	// �ܺ� ������ �о�鿩�� �����Ѵ�.
	if ( !LoadIni(GetFullPath() $ "\\" $ IniName, "Notice", "+Image", values) )
	{
		`log("Cannot Open " $ GetFullPath() $ "\\" $ IniName);
		return ;
	}

	// �ʱ�ȭ.
	ImageInfos.Length = 0;

	// ������ �׸������� �����ð��� �о �߰��Ѵ�.
	// TMultiMap���� ���� Ű��("+Image")�� ���ؼ� �� ������ �߰��Ǵ� �� �;
	// �ϴ� �������� ó���ؼ� �ذ��� ��.
	for ( i = values.Length - 1; i >= 0; --i )
	{
		tmp = Trim(values[i], " ()", TRUE);

		Tokenize(tmp, ",", parts);
		// �Ķ���ʹ� 2���̴�.
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

	// ������ ��� �ؽ��ĸ� �о���δ�.
	for ( i = 0; i < ImageInfos.Length; ++i )
		ImageInfos[i].Tex = LoadImage(GetFullPath() $ "\\" $ ImageInfos[i].ImageName);

	// �⺻ �ؽ��� ����.
	if ( ImageInfos.Length > 0 )
		SetCustomImage(ImageInfos[0].Tex);
}

DefaultProperties
{
	// default�� �����Ǵ� ��ü���� PostEditChange�Լ��� ȣ��Ǹ鼭
	// �ڵ����� OnChangeIniName() �Լ��� ���� ���� ó������ ���� ���� ���ؼ�
	// �⺻���� ���� �ɷ� �Ѵ�.
//	IniName = "notice.ini"

	SubPath = "avaGame\\Web\\Notice"
	PrefixName = "CustomImage_"
	// Neowiz���� ���޹��� URL.(2007/06/23)
	URLPrefix = "http://mm.sayclub.com/pmang/ava/client_img"
//	URLPrefix = "http://loozend.cafe24.com/avanotice1"
}
