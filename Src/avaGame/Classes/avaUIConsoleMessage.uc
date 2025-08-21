// ToDo. Background �׸���
//		 Alpha �� �������

class avaUIConsoleMessage extends UIObject native;

var() Font				Font;
var() array<string>		TestMessages;
var() bool				bUseShadow;

var() float				MotionDuration;		/** �� �׸��� ������Ʈ �Ǿ����� ������ �ð� */
var() float				FadeDuration;		/** ���� �׸��� ������ ������ ���̵� �ƿ� �ؾ��ϴ� �ð�*/
var() float				MessageLifeTime;	/** �޼��� ������Ÿ�� */
var() CurveEdPresetCurve	MotionCurve;

var transient float		LeapTimeLeft;		/** ���ڵ��� �� �����̱� ���� ���� �ð� */
var transient bool		IsInPageMode;			/** ������ ��忡 �ִ��� ������ (�������� �Ѱ� ���Ÿ޼����� ������) �˻� */
//var transient int		LatestConsoleMessageCount;	/** �ֱ� ������Ʈ�� ������ ���� */

var transient float	PreviousBottomIndex;		/** Current�� Target�� �ٴٸ��� TargetBottomIndex�� PreviousBottomIndex�� �ȴ� */
var transient float	CurrentBottomIndex;			/** ���� ���� BottomIndex, �����϶� �ð��� ���� ����  */
var transient float	TargetBottomIndex;			/** ��ǥ ���� BottomIndex, ������ ��,�ٿ� �Ǵ� �� �޼����� ���ö� ����*/

var transient float		DefaultHeight;	/** ���ڿ��� ���̸� �̸� ���� */
var() editconst transient int		VisibleItemCount;	/** ���� �����ٿ�忡�� ������ �� �ִ� ä�� ������ ���� */

var(Icon) editinline UITileMotionTexture	ScrollUpIcon;
var(Icon) editinline UITileMotionTexture	ScrollDownIcon;
var(Icon) editinline RenderParameters	ScrollUpIconParms;
var(Icon) editinline RenderParameters	ScrollDownIconParms;

var const editconst name	MSGType_QC;
var const editconst name	MSGType_Say;
var const editconst name	MSGType_TeamSay;

cpptext
{
	virtual void Render_Widget( FCanvas* Canvas );
	virtual void PostEditChange( UProperty* PropertyThatChanged );
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );

	virtual void Render_Messages( FCanvas* Canvas );
	inline void Render_MessageUnit( FCanvas* Canvas, FLOAT X, FLOAT Y, FString& Msg, UFont* DrawFont, FLinearColor& DrawColor, FLinearColor& ShadowColor );
	
	void ResolveFacePosition( EUIWidgetFace Face );
protected:
	UUIStyle_Text* GetTextStyleByTypeName( FName TypedTextName );
	void UpdateVisibleItemCount();
	void SetTargetBottomIndex( FLOAT TargetBottomIndex );
}

native function int GetConsoleMessageSize();

native function SetTargetIndex( int NewTargetIndex, optional bool bImmediately = false);

function OnGetListItemCount( UIAction_GetListItemCount Action )
{
	`Log("GetItemCount("$Self$") = "$GetConsoleMessageSize());
	Action.ItemCount = GetConsoleMessageSize();
}

protected final function OnSetListIndex( UIAction_SetListIndex Action )
{
	local int OutputLinkIndex;
	Local int NewTargetIndex;
	Local int ConsoleMessageSize;

	if ( Action != None )
	{
		ConsoleMessageSize = GetConsoleMessageSize();
		NewTargetIndex = Action.NewIndex;
		if( Action.bClampInvalidValues )
			NewTargetIndex = Clamp(Action.NewIndex, 0 , Max( ConsoleMessageSize - 1, 0 ) );
		
		if( (0 <= NewTargetIndex && NewTargetIndex < ConsoleMessageSize) )
			SetTargetIndex( NewTargetIndex );
		else
			OutputLinkIndex = 1; // "Failed"

		// activate the appropriate output link on the action
		if ( !Action.OutputLinks[OutputLinkIndex].bDisabled )
		{
			Action.OutputLinks[OutputLinkIndex].bHasImpulse = true;
		}
	}
}


defaultproperties
{
	bUseShadow = true

	MotionDuration = 0.5
	FadeDuration = 0.5
	MessageLifeTime =  2.0

	PreviousBottomIndex = INDEX_NONE
	CurrentBottomIndex = INDEX_NONE
	TargetBottomIndex = INDEX_NONE

	MSGType_QC="QuickChat"
	MSGType_Say="Say"
	MSGType_TeamSay="TeamSay"

	//Begin Object Class=UITexture Name=UITextureForScrollUp
	//End Object
	//ScrollUpIcon = UITextureForScrollUp

	//Begin Object Class=UITexture Name=UITextureForScrollDown
	//End Object
	//ScrollDownIcon = UITextureForScrollDown

}