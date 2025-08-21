class avaUIPDAPanel extends UIObject
	native;

var(Image)	instanced	editinlineuse		UITexture				Background;
var(Image)									TextureCoordinates		BackgroundCoordinates;

/* Enemy Icon*/
var(Icon)								Surface					EnemyIcon;
var(Icon)								Color					EnemyColor;
var(Icon)								bool					bShowEnemyName;

/* Team Icon */
var(Icon)								Surface					TeamIcon;
var(Icon)								array<Color>			TeamColor;
var(Icon)								bool					bShowTeamName;

/* Flag Icon */
var(Icon)								Surface					FlagIcon;
var(Icon)								array<Color>			FlagColor;
var(Icon)								bool					bShowFlagDistance;

/* General setting for Icons */
var(Icon)								float					IconScale;
var(Icon)								int						MinIconSize;

var(Icon)								bool					bFixUpperRightIndicator;

var(Icon)								Surface					LocationIcon;
var(Icon)								Color					LocationColor;
var(Icon)								bool					bShowLocationIcon;


struct native VolumeButtonInfo
{
	var UILabelButton	Button;
	var Volume			Volume;
};

struct native StrategyButtonInfo
{
	var UILabelButton			Button;
	var EWaypointActionType		nWPAction;
	var EWaypointTeamType		nWPTeam;
};

/* StyleReferences */
var										UIStyleReference		ButtonStyle;
var										UIStyleReference		IndicatorStyle;

/* Button Data for each functionalities.*/
var		private							array<VolumeButtonInfo>		VolumeButtons;
var		private							array<StrategyButtonInfo>	StrategyButtons;
var		private							array<UILabelButton>		IndicatorButtons;	// There's no functionalities for IndicatorButtons.
																						// only for displaying.
var		private							array<Volume>				CurrentVolumes;

/* Reseved Numbers of MenuItems(MenuButtons) */
var		const							int						MaxMenuItems;

/* Button Width & Height (individual) */
var(Menu)								IntPoint				ButtonExtent; // using ScreenValue_Position as a dimension value(width, height)

/* Selected Volume */
var		private							Volume					VolumeSelected;


cpptext
{
	virtual void Render_Widget( FCanvas *Canvas );
	virtual UBOOL ProcessInputKey( const FInputEventParameters& EventParms );
	virtual UBOOL ProcessInputAxis( const FInputEventParameters& EventParms );
	virtual void GetSupportedUIActionKeyNames(TArray<FName> &out_KeyNames );

	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );
	/**
	 * Called when a style reference is resolved successfully.
	 *
	 * @param	ResolvedStyle			the style resolved by the style reference
	 * @param	StyleProperty			the name of the style reference property that was resolved.
	 * @param	ArrayIndex				the array index of the style reference that was resolved.  should only be >0 for style reference arrays.
	 * @param	bInvalidateStyleData	if TRUE, the resolved style is different than the style that was previously resolved by this style reference.
	 */
	void OnStyleResolved( UUIStyle* ResolvedStyle, const FStyleReferenceId& StylePropertyId, INT ArrayIndex, UBOOL bInvalidateStyleData );


protected:
	// [2006/10/19, YTS] it has similar functionality with 'UIScrollbar.SetupDockLinks()'
	// Reposition volume buttons relative to a clicked position.
	void SetupDocLinksVolumeButton( UBOOL bResetLinks = FALSE );
	void SetupDocLinksStrategyButton( UBOOL bResetLinks = FALSE );
	void SetupDocLinksIndicatorButton( UBOOL bResetLinks = FALSE );
	void HideButtons( UBOOL bHideVolumeButton = TRUE, UBOOL bHideStrategyButton = TRUE , UBOOL bHideIndicatorButton = TRUE);

	void UpdateVolumeButton();
	void UpdateStrategyButton();
	void UpdateIndicatorButton();
}

event Initialized()
{
	local byte ButtonMask;
	Local int i;

	Super.Initialized();

	ButtonMask = PRIVATE_NotFocusable | PRIVATE_NotDockable | PRIVATE_TreeHidden | PRIVATE_NotEditorSelectable | PRIVATE_ManagedStyle;
	// Private Behavior
	SetPrivateBehavior( PRIVATE_NotFocusable | PRIVATE_NotRotatable, true );

	for( i = 0 ; i < VolumeButtons.Length ; i++)
	{
		if( VolumeButtons[i].Button != None )
		{
			VolumeButtons[i].Button.OnMouseClicked = SetLocation;
			VolumeButtons[i].Button.SetPrivateBehavior(ButtonMask, true);
		}
	}

	for( i = 0 ; i < StrategyButtons.Length ; i++ )
	{
		if( StrategyButtons[i].Button != None )
		{
			StrategyButtons[i].Button.OnMouseClicked = DoStrategy;
			StrategyButtons[i].Button.SetPrivateBehavior(ButtonMask, true);
		}
	}
}

native final function SetLocation( UIButton Sender, int PlayerIndex );
native final function DoStrategy( UIButton Sender, int PlayerIndex);

defaultproperties
{
	EnemyIcon=Texture2D'EngineResources.WhiteSquareTexture'
	EnemyColor=(R=255,G=0,B=0,A=255)
	
	TeamIcon=Texture2D'EngineResources.WhiteSquareTexture'
	TeamColor(WPTeam_Blue)=(R=0,G=0,B=255,A=255)
	TeamColor(WPTeam_Yellow)=(R=255,G=255,B=0,A=255)
	TeamColor(WPTeam_MAX)=(R=0,G=255,B=0,A=255)

	FlagIcon=Texture2D'EngineResources.WhiteSquareTexture'
	FlagColor(WPTeam_Blue)=(R=0,G=0,B=255,A=255)
	FlagColor(WPTeam_Yellow)=(R=255,G=255,B=0,A=255)
	FlagColor(WPTeam_MAX)=(R=0,G=255,B=0,A=255)


	LocationIcon=Texture2D'EngineResources.WhiteSquareTexture'
	LocationColor=(R=0,G=255,B=0,A=255)
	

	ButtonExtent=(X=100,Y=20)
	MaxMenuItems=10
	bFixUpperRightIndicator=true

	// Styles
	PrimaryStyle=(DefaultStyleTag="DefaultPrimaryStyle",RequiredStyleClass=class'Engine.UIStyle_Combo')
	ButtonStyle=(DefaultStyleTag="DefaultButtonStyle",RequiredStyleClass=class'Engine.UIStyle_Combo')
	IndicatorStyle=(DefaultStyleTag="DefaultButtonStyle",RequiredStyleClass=class'Engine.UIStyle_Combo')

	// IconSize = IconScale / ViewTarget.Location.Z
	IconScale=64000
	MinIconSize=8
}