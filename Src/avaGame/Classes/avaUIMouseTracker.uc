class avaUIMouseTracker extends UIObject
	native;


struct native UIMouseRegionCell
{
	//var deprecated float			Extent[EUIOrientation.UIORIENT_MAX];
	var() UIScreenValue						CellExtent[EUIOrientation.UIORIENT_MAX];
	var() transient UIScreenValue			Bounds[EUIWidgetFace.UIFACE_MAX];
	var() string							Alias;

	structdefaultproperties
	{
		CellExtent[UIORIENT_Horizontal]=(ScaleType=EVALPOS_PixelViewport)
		CellExtent[UIORIENT_Vertical]=(ScaleType=EVALPOS_PixelViewport)
		Bounds[UIFACE_Left]=(ScaleType=EVALPOS_PixelViewport)
		Bounds[UIFACE_Right]=(ScaleType=EVALPOS_PixelViewport)
		Bounds[UIFACE_Top]=(ScaleType=EVALPOS_PixelViewport)
		Bounds[UIFACE_Bottom]=(ScaleType=EVALPOS_PixelViewport)
	}
};

struct native UIMouseRegionElement
{
	var() array<UIMouseRegionCell>		Cells;
};

var() array<UIMouseRegionElement>		MouseRegionElements;
var	private transient int				LastElementIndex;
var	private transient int				LastCellIndex;

var() UIScreenValue						ElementPadding;

cpptext
{
	/* === UUIScreenObject interface === */

	/* === UUIObject interface === */

public:
	/**
	 * Render this widget.
	 *
	 * @param	Canvas	the FCanvas to use for rendering this widget
	 */
	virtual void Render_Widget( FCanvas* Canvas );

	/* === UObject interface === */

	/**
	 * Called when a property value has been changed in the editor.
	 */
	virtual void PostEditChange( FEditPropertyChain& PropertyThatChanged );

	/**
	 * deprecated UIMouseRegionCell.Extent를 UIMouseRegionCell.CellExtent로 변환
	 */
	virtual void PostLoad();

	/**
	 * Evalutes the Position value for the specified face into an actual pixel value.  Should only be
	 * called from UIScene::ResolvePositions.  Any special-case positioning should be done in this function.
	 *
	 * @param	Face	the face that should be resolved
	 */
	virtual void ResolveFacePosition( EUIWidgetFace Face );

protected:
	/** Cell Position을 새로 계산. ResolveFacePosition과 같이 UIObject의 위치, 크기, 도킹 등이 변경되었을때 불린다 */
	void RecalculateCellPosition();
}

defaultproperties
{
	// States
	DefaultStates.Add(class'Engine.UIState_Focused')
	
	LastElementIndex=INDEX_NONE
	LastCellIndex=INDEX_NONE
}