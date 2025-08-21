/**
 * @ingroup _AVAGame
 * 
 * UIObject들을 지정된 장소에 재배열하는 Widget입니다.
 *
 * 세개의 버튼이 가로로 순서대로 배열되어있을때 가운데 버튼이 없어지면 보기 흉하기 때문에
 * 왼쪽의 버튼을 가운데 버튼이 있던 자리로 옮길 필요가 있습니다. ( 혹은 오른쪽 버튼을 가운데로 )
 *
 * 그럴때 avaUIOutfitter를 배치해서 해당 리젼을 만든후 적용할 UIObject들을 avaUIOutfitter의 자식으로 만들면
 * 해당 UIObject가 없어지거나 안보일때 우측의 UIObject가 빈자리를 매꿔 이동합니다.
 *
 * @date 2007-11-13
 *
 */
class avaUIOutfitter extends UIPanel
	native(UIPrivate)
	implements(UIDataStoreSubscriber);


struct native UIOutfitCell
{
	var() float									Extent[EUIOrientation.UIORIENT_MAX];
	var() EUIAlignment							Alignment[EUIOrientation.UIORIENT_MAX];
	var() bool									bCollapse;
	var() array<UIScreenObject>					AttachedUIObject;

	var transient editconst float				Position[EUIWidgetFace.UIFACE_MAX];

	structdefaultproperties
	{
		Extent(0)=100
		Extent(1)=50
		bCollapse=true
	}
};

struct native UIOutfitElement
{
	var() array<UIOutfitCell>		Cells;
};

var() array<UIOutfitElement>		OutfitElements;
var() bool							bFitCollapsed<ToolTip:adjust renderbounds to the collapsed boxes>;

var private	transient	bool		bRecalcCellPosition;
var private transient	bool		bReposAttached;

cpptext
{
	/* === UUIScreenObject interface === */
	/**
	 * Perform all initialization for this widget. Called on all widgets when a scene is opened,
	 * once the scene has been completely initialized.
	 * For widgets added at runtime, called after the widget has been inserted into its parent's
	 * list of children.
	 *
	 * @param	inOwnerScene	the scene to add this widget to.
	 * @param	inOwner			the container widget that will contain this widget.  Will be NULL if the widget
	 *							is being added to the scene's list of children.
	 */
	virtual void Initialize( UUIScene* inOwnerScene, UUIObject* inOwner=NULL );

	/* === UUIObject interface === */

	/**
	 * Adds the specified face to the DockingStack for the specified widget
	 *
	 * @param	DockingStack	the docking stack to add this docking node to.  Generally the scene's DockingStack.
	 * @param	Face			the face that should be added
	 *
	 * @return	TRUE if a docking node was added to the scene's DockingStack for the specified face, or if a docking node already
	 *			existed in the stack for the specified face of this widget.
	 */
	virtual UBOOL AddDockingNode( TArray<struct FUIDockingNode>& DockingStack, EUIWidgetFace Face );

	/**
	 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
	 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
	 *
	 * @param	Face	the face that should be resolved
	 */
	virtual void ResolveFacePosition( EUIWidgetFace Face );

protected:
	/**
	 * Marks the Position for any faces dependent on the specified face, in this widget or its children,
	 * as out of sync with the corresponding RenderBounds.
	 *
	 * @param	Face	the face to modify; value must be one of the EUIWidgetFace values.
	 */
	virtual void InvalidatePositionDependencies( BYTE Face );

public:
	/**
	 * Render this widget.
	 *
	 * @param	Canvas	the FCanvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas );

	/**
	 * Called when a property is modified that could potentially affect the widget's position onscreen.
	 */
	virtual void RefreshPosition();

	/**
	 * Changes this widget's position to the specified value.  This version changes the default value for the bClampValues parameter to TRUE
	 *
	 * @param	LeftFace		the value (in pixels or percentage) to set the left face to
	 * @param	TopFace			the value (in pixels or percentage) to set the top face to
	 * @param	RightFace		the value (in pixels or percentage) to set the right face to
	 * @param	BottomFace		the value (in pixels or percentage) to set the bottom face to
	 * @param	InputType		indicates the format of the input value.  All values will be evaluated as this type.
	 *								EVALPOS_None:
	 *									NewValue will be considered to be in whichever format is configured as the ScaleType for the specified face
	 *								EVALPOS_PercentageOwner:
	 *								EVALPOS_PercentageScene:
	 *								EVALPOS_PercentageViewport:
	 *									Indicates that NewValue is a value between 0.0 and 1.0, which represents the percentage of the corresponding
	 *									base's actual size.
	 *								EVALPOS_PixelOwner
	 *								EVALPOS_PixelScene
	 *								EVALPOS_PixelViewport
	 *									Indicates that NewValue is an actual pixel value, relative to the corresponding base.
	 * @param	bZeroOrigin		FALSE indicates that the value specified includes the origin offset of the viewport.
	 * @param	bClampValues	if TRUE, clamps the values of RightFace and BottomFace so that they cannot be less than the values for LeftFace and TopFace
	 */
	virtual void SetPosition( const FLOAT LeftFace, const FLOAT TopFace, const FLOAT RightFace, const FLOAT BottomFace, EPositionEvalType InputType=EVALPOS_PixelViewport, UBOOL bZeroOrigin=FALSE, UBOOL bClampValues=TRUE );

	/* === UObject interface === */
	/**
	 * Called when a property value from a member struct or array has been changed in the editor, but before the value has actually been modified.
	 */
	virtual void PreEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Called when a property value has been changed in the editor.
	 */
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * Called after this object has been completely de-serialized.  This version migrates the PrimaryStyle for this label over to the label's component.
	 */
	virtual void PostLoad();

protected:
	void CalculateAllPosition();
	void RecalcCellPosition();
	void RetainAttached();
	void RepositionAttached();

	void RequestUpdateOutfit( UBOOL bRecalcCellPos, UBOOL bReposAttached );
}

/**
 * Sets the data store binding for this object to the text specified.
 *
 * @param	MarkupText			a markup string which resolves to data exposed by a data store.  The expected format is:
 *								<DataStoreTag:DataFieldTag>
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 */
native function SetDataStoreBinding( string MarkupText, optional int BindingIndex=INDEX_NONE );

/**
 * Retrieves the markup string corresponding to the data store that this object is bound to.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	a datastore markup string which resolves to the datastore field that this object is bound to, in the format:
 *			<DataStoreTag:DataFieldTag>
 */
native function string GetDataStoreBinding( optional int BindingIndex=INDEX_NONE ) const;

/**
 * Resolves this subscriber's data store binding and updates the subscriber with the current value from the data store.
 *
 * @param	BindingIndex		optional parameter for indicating which data store binding is being requested for those
 *								objects which have multiple data store bindings.  How this parameter is used is up to the
 *								class which implements this interface, but typically the "primary" data store will be index 0.
 *
 * @return	TRUE if this subscriber successfully resolved and applied the updated value.
 */
native function bool RefreshSubscriberValue( optional int BindingIndex=INDEX_NONE );

/**
 * Retrieves the list of data stores bound by this subscriber.
 *
 * @param	out_BoundDataStores		receives the array of data stores that subscriber is bound to.
 */
native function GetBoundDataStores( out array<UIDataStore> out_BoundDataStores );

/**
 * Notifies this subscriber to unbind itself from all bound data stores
 */
native function ClearBoundDataStores();

/**
 * Handler for responding to child widget changing its position, it recalculates the scrollframe's scroll region bounds
 *
 * @param	Sender	Child widget which has been repositioned
 */
native final function OnAttachRepositioned( UIScreenObject Sender );

/**
 * @param NewAttached		새로 붙은 UIScreenObject. 위치가 바뀔때마다 UIOutfitter를 갱신하는 역할을 한다.
 */
event AddChildPositionChangeNotify( UIScreenObject NewAttached )
{
	if( NewAttached != none )
	{
		NewAttached.NotifyPositionChanged = OnAttachRepositioned;
	}
}

/**
 * @param NewAttached		제거된 UIScreenObject. 위치가 바뀔때마다 UIOutfitter를 갱신하는 역할을 한다.
 */
event RemoveChildPositionChangeNotify( UIScreenObject NewAttached )
{
	if( NewAttached != none )
	{
		NewAttached.NotifyPositionChanged = none;
	}
}


defaultproperties
{
	// States
	DefaultStates.Add(class'Engine.UIState_Focused')
}