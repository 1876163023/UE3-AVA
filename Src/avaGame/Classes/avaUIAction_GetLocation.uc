class avaUIAction_GetLocation extends UIAction;

var() float fLeft;
var() float fTop;
var() float fRight;
var() float fBottom;

event Activated()
{
	Local UIScreenObject ScreenObj; 
	Local int i;

	for( i = 0 ; i < Targets.Length ; i++ )
	{
		ScreenObj = UIScreenObject(Targets[i]);
		if( ScreenObj != none )
			break;	
	}

	`Log("Get Location of "$ScreenObj);
	if( ScreenObj == none )
	{
		fLeft = 0;
		fTop = 0;
		fRight = 0;
		fBottom = 0;
	}
	else
	{
		for( i = 0 ; i < UIFACE_MAX ; i++ )
		{
			switch( i )
			{
			case UIFACE_Left:	fLeft = ScreenObj.GetPosition(EUIWidgetFace(i),EVALPOS_PixelViewport);	break;
			case UIFACE_Top:	fTop = ScreenObj.GetPosition(EUIWidgetFace(i),EVALPOS_PixelViewport);	break;
			case UIFACE_Right:	fRight = ScreenObj.GetPosition(EUIWidgetFace(i),EVALPOS_PixelViewport);	break;
			case UIFACE_Bottom:	fBottom = ScreenObj.GetPosition(EUIWidgetFace(i),EVALPOS_PixelViewport);	break;
			}
		}
	}
}

defaultproperties
{
	ObjName="Get Location( UI Widget )"
	ObjCategory="UI"

	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Left",PropertyName=fLeft,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Top",PropertyName=fTop,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Right",PropertyName=fRight,bWriteable=true))
	VariableLinks.Add((ExpectedType=class'SeqVar_Float',LinkDesc="Bottom",PropertyName=fBottom,bWriteable=true))
}