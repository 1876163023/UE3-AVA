class avaUIChart extends UIList
	native;

cpptext
{
	/**
	 * UIList로부터 헤더 정보를 받아서 ze헤더(리스트 항목)만를 그린후
	 * 차트모양에 해당하는 렌더링을 한다
	 */
	virtual void Render_Widget( FCanvas* Canvas );

protected:
	
	/**
	 * Refreshes the data for this list from the data store bound via DataSource.
	 *
	 * @param	bResolveDataSource	if TRUE, re-resolves DataSource into DataProvider prior to refilling the list's data
	 *
	 * @return	TRUE if the list data was successfully loaded; FALSE if the data source couldn't be resolved or it didn't
	 *			contain the data indicated by SourceData
	 */
	virtual UBOOL RefreshListData( UBOOL bResolveDataSource=FALSE );

	/** 항목의 이름을 그린다 (DESC) */
	virtual void Render_TemplateCells( FCanvas* Canvas, const TArray<struct FUIListElementCellTemplate>& Cells, struct FRenderParameters& CellParameters );

	/** 항목의 내용을 그린다 (SELECT FROM) */
	virtual void Render_Cells( FCanvas* Canvas, const TArray<struct FUIListElementCell>& Cells, struct FRenderParameters& CellParameters );
}

struct native UIChartElementCell
{
	var transient float BaseValue;

	structcpptext
	{
		FUIChartElementCell(){}
		FUIChartElementCell(EEventParm)
		{
			appMemzero(&BaseValue, sizeof(BaseValue));
		}
	}
};

struct native UIChartItem
{
	
	/** the cells associated with this list element */
	var()	editinline editconst editfixedsize	array<UIChartElementCell>	Cells;
	var()	transient init	string		HeaderText;

	structcpptext
	{
		FUIChartItem(){}
		FUIChartItem(EEventParm)
		{
			appMemzero(this,sizeof(FUIChartItem));
		}
	}
};

enum EChartShapeType
{
	CHARTSHAPE_StandingStick,
	CHARTSHAPE_CrouchingStick,
	CHARTSHAPE_SimplePolygon,
};

var(Data)	EChartShapeType			ChartShape;

var	editconst	editinline	transient	array<UIChartItem>	CurrChartItems;			//	CurrentChartItems
var	editconst	editinline	transient	array<UIChartItem>	TranChartItems;			//	TransientChartItemss

var()	editconst	editinline	transient	init array<UIChartItem> ChartItems;

defaultproperties
{
	bAllowDisabledItemSelection=true
	ColumnAutoSizeMode=CELLAUTOSIZE_None
	RowAutoSizeMode=CELLAUTOSIZE_None

	ChartShape = CHARTSHAPE_StandingStick;
}