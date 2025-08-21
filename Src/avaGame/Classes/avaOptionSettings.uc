class avaOptionSettings extends object
	config(OptionSettings)
	native;

/** 채널리스트 관련 */



/** 사용자가 마지막으로 선택한 채널그룹 */

struct native AccountChannelInfo
{
	var int AccountID;
	var int LastChannelGroup;

	structcpptext
	{
		FAccountChannelInfo( INT InAccountID, INT InLastChannelGroup ) : AccountID(InAccountID), LastChannelGroup(InLastChannelGroup) {}
	}
};

var config		array<AccountChannelInfo>			LastChannelGroupData;
var transient	bool								bLastChannelGroupChanged;


/** 비디오 설정 */
var config float	StartUpResX;
var config float	StartUpResY;
var config float	DisplayGamma;
var config bool		bDisableFog;
var config bool		bDisablePostProcess;
var config bool		bDisableDynamicShadows;
var config bool		bUseWorldShadow;
var config bool		bUseHighQualityBloom;
var config bool		bDisableDynamicLights;
var config bool		bDisableCompositeDynamicLights;
var config bool		bDisableImpactParticle;
//var deprecated config bool		bEnableSmoothFrameRate;
//var deprecated config bool		bEnableVSync;
var config bool		bUseVSync;
var config bool		bUseSmoothFrameRate;

var config int		CharacterDetail;
var config int		TextureDetail;
var config int		Anisotropy;
var config int		Antialiasing;
var config int		ShaderModel;
var config int		DecalDetail;
var config int		ShadowDetail;
var config bool		bDisableLOD;


// 임시로 저장할 현재의 화면비율
var transient vector2d	CurrentAspectRatio;
var transient vector2d	ConfirmedResolution;

/** 오디오 설정 */
var config float	MusicVolume;
var config float	GameVolume;
var config float	SystemVolume;
var config int		AudioChannel;

/** 마우스 설정 */
var config float	MouseSensitivity;
var config bool		bInvertMouse;
var config bool		bDisableMouseSmoothing;

/** 게임 설정 */

struct native FilterType
{
	var bool bEnableOthers;
	var bool bEnableFriends;
	var bool bEnableClanMembers;
};

var config bool				UseLocalSound;
var config bool				UseHUDCamera;
var config FilterType		PrivateChatFilter;
var config FilterType		InviteGameFilter;
var config FilterType		InviteClanFilter;
var config FilterType		AddFriendFilter;
var config bool				bAllowOneFrameThreadLag;
var config int				MaxRagdollCount;
var config bool				bUseLoadMapCache;

/** Net Config */
enum GameOptionFieldType
{
	GAMEOPTIONFIELD_MouseSmoothing,
};

cpptext
{
	static void StaticInit();
	static void StaticSaveConfig();

	static UavaOptionSettings* GetDefaultOptionSettings()
	{ 
		return UavaOptionSettings::StaticClass()->GetDefaultObject<UavaOptionSettings>(); 
	}

private:
	FAccountChannelInfo* FindMyAccountChannelInfo();
}

/** ChannelList */
native final static function SetLastChannelGroup( int NewChannelGroup, optional bool bSaveConfig );
native final static function int GetLastChannelGroup();
native final static function FlushChanged();

/** Common */
native static function avaOptionSettings GetDefaultObject();
native static function RecreateDevice( optional bool bRecreateWhenChanged );
native static function MiscIterativeUpdate( bool bUpdateTextureDetail, bool bUpdateCharacterDetail );

native static function SetConfirmedResolution( vector2d NewResolution );
native static function vector2d GetConfirmedResolution( bool bFlushOut = true );

/** Video */
native static function vector2d GetResolution();
native static function bool SetResolution( vector2d NewResolution, optional bool bDisableWarning );
native static function bool SetResolutionStr( string NewResolution, optional bool bDisableWarning );
native static function vector2d GetAspectRatio();
native static function bool SetAspectRatio( vector2d NewAspectRatio );
native static function float GetDisplayGamma();
native static function bool SetDisplayGamma( FLOAT NewGamma );
native static function bool GetFog();
native static function bool SetFog( bool bSet );
native static function bool GetPostProcess();
native static function bool SetPostProcess( bool bSet );
native static function bool GetImpactParticle();
native static function bool SetImpactParticle( bool bSet );
native static function int GetDynamicLightLevel();
native static function bool SetDynamicLightLevel( int DynLightLevel );

// ShadowDetail = DynamicShadow + WorldShadow
native static function GetShadowDetailList( out array<int> DetailList );
native static function int GetShadowDetail();
native static function bool SetShadowDetail( int DetailLevel );
	native static function bool GetDynamicShadows();
	native static function bool SetDynamicShadow( bool bSet );
	native static function bool GetWorldShadow();
	native static function bool SetWorldShadow( bool bSet );


// StableFrameMode = VSync + SmoothFrameRate
native static function int GetStableFrameMode();
native static function bool SetStableFrameMode( int StableFrameMode );
	native static function bool GetVSync();
	native static function bool SetVSync( bool bSet );
	native static function bool GetSmoothFrameRate();
	native static function bool SetSmoothFrameRate( bool bSet );

native static function int GetTextureDetail();
native static function bool SetTextureDetail( int DetailLevel );
native static function int GetCharacterDetail();
native static function bool SetCharacterDetail( int DetailLevel );
native static function GetAnisotropyList( out array<int> AnisotropyList );
native static function int GetAnisotropy();
native static function bool SetAnisotropy( int NewAnisoLevel );
native static function GetAntiAliasingList( out array<int> AntiAliasingList );
native static function int GetAntiAliasing();
native static function bool SetAntiAliasing( int NewAA );
native static function GetShaderModelList( out array<int> ShaderModelList );
native static function int GetShaderModel();
native static function bool SetShaderModel( int NewExtShaderNum );
native static function int GetDecalDetail();
native static function bool SetDecalDetail( int NewDelcalDetail );

native static function bool GetLODApply();
native static function bool SetLODApply( bool bSet );

/** Audio */
native static function bool SetAudioChannel( int AudioChannelCount );
native static function int GetAudioChannel();
native static function bool SetAudioVolume( name Group, FLOAT NewVolume );
native static function float GetAudioVolume( name Group );

/** Mouse */
native static function bool SetMouseSensitivity( float NewMouseSensitivity );
native static function float GetMouseSensitivity();
native static function bool SetInvertMouse( bool bNewInvertMouse );
native static function bool GetInvertMouse();
native static function bool SetMouseSmoothing( bool bSet, optional bool bSaveConfig);
native static function bool GetMouseSmoothing();

/** Game */
native static function bool SetUseLocalSound( bool bSet );
native static function bool GetUseLocalSound();
native static function bool SetUseHUDCamera( bool bSet );
native static function bool GetUseHUDCamera();
native static function bool SetOneFrameThreadLag( bool bSet );
native static function bool GetOneFrameThreadLag();
native static function bool SetMaxRagdollCount( int MaxRagdoll );
native static function int GetMaxRagdollCount();
native static function bool	GetLoadMapCache();
native static function bool SetLoadMapCache( bool bSet );

/** Net Config */
native static function string GetGameOptionString();
native static function SetGameOptionString( string NewOptionString );