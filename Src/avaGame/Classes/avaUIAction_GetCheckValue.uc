/**
 * This action allows designers to change the value of a widget that contains Check data.
 * CheckBox, CheckLabelButton의 값을 읽어들이되 여러개의 타겟이 들어올경우
 * Exclusive or Inclusive에 따라 각각 적어도 하나의 참값과 모든 참값에 대해 결과가 참임
 *
 * Copyright 2005 Epic Games, Inc. All Rights Reserved.
 */
class avaUIAction_GetCheckValue extends UIAction_GetValue;

var()				bool					bNewValue;
var()				bool					bExclusive;

event Activated()
{
	Local int i;
	Local avaUICheckLabelButton CheckLabelButton;
	Local UICheckBox CheckBox;
	Local bool BoolValue;

	bNewValue = false;

	for( i = 0 ; i < Targets.Length ; i++ )
	{
		CheckBox = UICheckBox(Targets[i]);
		CheckLabelButton = avaUICheckLabelButton(Targets[i]);

		if( CheckBox == none && CheckLabelButton == none )
			continue;

		BoolValue = CheckBox != none ? CheckBox.IsChecked() : CheckLabelButton != none ? CheckLabelButton.IsChecked() : false;
		if( bExclusive )
		{
			if( !BoolValue )
			{
				bNewValue = false;
				break;
			}

			if( Targets.Length - 1 == i )
			{
				bNewValue = true;
			}
		}
		else
		{
			if( BoolValue )
			{
				bNewValue = true;
				break;
			}
		}
	}
}

DefaultProperties
{
	ObjName="Get Check Value"
	VariableLinks.Add((ExpectedType=class'SeqVar_Bool',LinkDesc="Value",PropertyName=bNewValue))
}