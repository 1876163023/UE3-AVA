class avaUIChart extends UIList
	native;

cpptext
{
	/**
	 * UIList�κ��� ��� ������ �޾Ƽ� ze���(����Ʈ �׸�)���� �׸���
	 * ��Ʈ��翡 �ش��ϴ� �������� �Ѵ�
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

	/** �׸��� �̸��� �׸��� (DESC) */
	virtual void Render_TemplateCells( FCanvas* Canvas, const TArray<struct FUIListElementCellTemplate>& Cells, struct FRenderParameters& CellParameters );

	/** �׸��� ������ �׸��� (SELECT FROM) */
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