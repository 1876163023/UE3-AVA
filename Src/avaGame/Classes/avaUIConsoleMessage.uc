// ToDo. Background 그리기
//		 Alpha 로 사라지기

class avaUIConsoleMessage extends UIObject native;

var() Font				Font;
var() array<string>		TestMessages;
var() bool				bUseShadow;

var() float				MotionDuration;		/** 새 항목이 업데이트 되었을때 움직일 시간 */
var() float				FadeDuration;		/** 기존 항목의 생명이 끝나면 페이드 아웃 해야하는 시간*/
var() float				MessageLifeTime;	/** 메세지 라이프타임 */
var() CurveEdPresetCurve	MotionCurve;

var transient float		LeapTimeLeft;		/** 문자들이 다 움직이기 까지 남은 시간 */
var transient bool		IsInPageMode;			/** 페이지 모드에 있는지 없는지 (페이지를 넘겨 과거메세지를 보는지) 검사 */
//var transient int		LatestConsoleMessageCount;	/** 최근 업데이트된 아이템 개수 */

var transient float	PreviousBottomIndex;		/** Current가 Target에 다다르면 TargetBottomIndex가 PreviousBottomIndex가 된다 */
var transient float	CurrentBottomIndex;			/** 현재 뷰의 BottomIndex, 움직일때 시간에 따라 변함  */
var transient float	TargetBottomIndex;			/** 목표 뷰의 BottomIndex, 페이지 업,다운 또는 새 메세지가 들어올때 변함*/

var transient float		DefaultHeight;	/** 문자열의 높이를 미리 결정 */
var() editconst transient int		VisibleItemCount;	/** 현재 렌더바운드에서 보여줄 수 있는 채팅 아이템 개수 */

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