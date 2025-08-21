/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaUIInteraction extends Interaction
	within avaGameViewportClient
	config(Game);

struct tMenuData
{
	var	string	Title;
	var string  EnterCmd;
	var string  OverCmd;
	var string 	LeaveCmd;
	var string	Data;
	var string  Hint;
};

var array<tMenuData> CurrentMenu;
var string MenuURL;
var string MenuTitle;
var string MenuHint;

var array<string> MenuStack;

// STEVE: Add additional Menus here

var array<tMenuData> MainMenu;
var array<tMenuData> MapList;
var array<tMenuData> DemoMapList;

var int	Menu_Top, Menu_At, Menu_IPP;

/** True if the menus are open. */
var bool bOpened;

function Initialized()
{
	Super.Initialized();

	LoadMenuData("MainMenu",":: Main Menu ::",MainMenu);
}

function AddToURL(string NewCommand)
{
	MenuURL = MenuURL$NewCommand;
}

function RemoveFromURL(string Command)
{
	class'Actor'.static.ReplaceText(MenuURL,Command,"");
}

function DrawShadowText(Canvas C, string Text, float x, float y)
{
	local color TempColor;

	TempColor = C.DrawColor;
	C.SetDrawColor(0,0,0,255);

	C.SetPos(X+1,Y+1);
	C.DrawText(Text);
	C.DrawColor = TempColor;
	C.SetPos(X,Y);
	C.DrawText(Text);
}

event bool InputKey( int ControllerId, name Key, EInputEvent Event, float AmountDepressed = 1.f , bool bGamepad = FALSE)
{
	if(bOpened && ViewportConsole.IsInState('Console') )
	{
		if(Event == IE_Released)
		{
			if(Key == 'Escape')
			{
				MenuStack.Remove(MenuStack.Length-1,1);
				if (MenuStack.Length <= 0)
				{
					bOpened = false;
				}
				else
					ProcessCmd(MenuStack[MenuStack.Length-1]);
				return true;
			}
			else if(Key=='Enter')
			{
				ProcessCmd(CurrentMenu[Menu_At].EnterCmd);
				return true;
			}
			else if(Key == 'q')
			{
				bOpened = false;
				ViewportConsole.ConsoleCommand( "quit" );
				return true;
			}
		}
		else if( Event == IE_Pressed || Event == IE_Repeat )
		{
			if( Key=='up' )
			{
				UpdateLeave();
        		if (Menu_At>0)
        		{
        			Menu_At--;
        			while (Menu_At < Menu_Top)
        				Menu_Top--;
        		}
        		UpdateOver();
        		return true;
			}
			else if ( Key=='down' )
			{
				UpdateLeave();
        		if ( Menu_At<CurrentMenu.Length-1 )
        		{
        			Menu_At++;
        			while ( Menu_At >= Menu_Top+Menu_IPP)
					{
						Menu_Top++;
					}
        		}
        		UpdateOver();
        		return true;
			}
			else if ( key=='pagedown' )
			{
				UpdateLeave();
        		if ( Menu_At<CurrentMenu.Length-1 )
        		{
        			Menu_At += Menu_IPP;
        			Menu_At = min(Menu_At,CurrentMenu.Length-1);
        			while ( Menu_At >= Menu_Top+Menu_IPP)
					{
						Menu_Top++;
					}
				}
				UpdateOver();
				return true;
			}
			else if ( key=='pageup' )
			{
				UpdateLeave();
        		if ( Menu_At>0 )
        		{
        			Menu_At -= Menu_IPP;
        			Menu_At = max(0,Menu_At);
        			while ( Menu_At < Menu_Top)
					{
						Menu_Top--;
					}
				}
				UpdateOver();
				return true;
			}

			else if (key=='home')
			{
				UpdateLeave();
				Menu_Top = 0;
				Menu_At = 0;
				UpdateOver();
				return true;
			}
			else if (key=='end')
			{
				UpdateLeave();
				Menu_At = CurrentMenu.Length-1;
				Menu_Top = 0;
    			while ( Menu_At >= Menu_Top+Menu_IPP)

				{
					Menu_Top++;
				}
				UpdateOver();
				return true;
			}
		}
	}

	return false;
}

function RenderMenu(Canvas Canvas)
{
	local float xl,yl,x,y,barw;
	local string S;
	local int i;	

	barw=0;
	Canvas.Font = class'Engine'.Static.GetSmallFont();
	for (i=0;i<CurrentMenu.Length;i++)
	{
		Canvas.Strlen(CurrentMenu[i].Title,xl,yl);
		if (xl>barw)
			barw = xl;
	}

	Canvas.Font = class'Engine'.Static.GetSmallFont();
	Canvas.Strlen("Q",xl,yl);
    Canvas.SetDrawColor(255,255,0,255);
    Canvas.SetPos(5,5+YL);
	//   if (MenuStack.Length==1)
	//    Canvas.DrawText("Press 'ESC' to close menu!");
	//else
	//    Canvas.DrawText("Press 'ESC' to return to the last menu!");
    Canvas.SetDrawColor(255,255,255,255);

	x = 5;
	y = 50;
	Canvas.Font	 = class'Engine'.Static.GetMediumFont();
	Canvas.Strlen(MenuTitle,xl,yl);
	Canvas.SetPos(x,y);
	Canvas.DrawText(MenuTitle);
	y+=yl;
	Canvas.SetPos(x,y);
	Canvas.DrawTile(Texture2D 'EngineResources.White',xl,4,0,0,32,32);
	y+=8;

	Canvas.Font = class'Engine'.Static.GetSmallFont();
	Canvas.Strlen("Q",xl,yl);

	Menu_IPP = int( ( Canvas.ClipY - yl*2 - y)/yl);
	for (i=0;i<Menu_IPP;i++)
	{
		if (Menu_Top + i >= CurrentMenu.Length)
			break;

		s = CurrentMenu[Menu_Top + i].Title;
		if (S=="")
			Canvas.StrLen("Q",xl,yl);
		else
			Canvas.Strlen(s,xl,yl);
		Canvas.SetPos(x,y);

		if (Menu_Top + i == Menu_At)
		{
			Canvas.SetDrawColor(255,0,0,128);
			Canvas.DrawTile(Texture2D 'EngineResources.White',barw,yl,0,0,32,32);
			Canvas.SetDrawColor(255,255,255,255);
			Canvas.SetPos(x,y);
		}

		Canvas.DrawText(s);
		y+=yl;
	}

	if (Menu_At<CurrentMenu.Length)
	{
		Canvas.Font = class'Engine'.Static.GetMediumFont();
		Canvas.Strlen("Q",xl,yl);
		Canvas.SetDrawColor(220,220,220,255);

        DrawShadowText(Canvas, MenuHint, x, Canvas.ClipY-(YL*2)-5);
		Canvas.Font = class'Engine'.Static.GetSmallFont();
		DrawShadowText(Canvas, MenuURL, x, Canvas.ClipY-yl-5);
	}
}

function LoadMenuData(string NewMenuCmd, string NewMenuTitle, array<TMenuData> NewMenu)
{
	local int i,Cnt;

	Cnt = MenuStack.Length;
	if ( MenuStack.Length == 0 || NewMenuCmd != MenuStack[Cnt-1] )	// Add to stack
	{
		MenuStack.Length = Cnt + 1;
		MenuStack[Cnt] = NewMenuCmd;
	}

	MenuTitle = NewMenuTitle;

	if (CurrentMenu.Length>0)
	{
		CurrentMenu.Remove(0,CurrentMenu.Length);
	}

	Cnt = 0;

	for (i=0; i<NewMenu.Length; i++)
	{
		CurrentMenu.Length = Cnt + 1;
		CurrentMenu[Cnt] = NewMenu[i];
		Cnt++;
	}

	Menu_Top = 0;
	Menu_At = 0;

	UpdateOver();

}




function UpdateOver()
{
	ProcessCmd(CurrentMenu[Menu_At].OverCmd);
}

function UpdateLeave()
{
	ProcessCmd(CurrentMenu[Menu_At].LeaveCmd);
}



function ProcessCmd(string Cmd)
{
	if ( Cmd ~= "SetHint")
	{
		MenuHint = CurrentMenu[Menu_At].Hint;
	}
	else if ( Cmd ~= "SetURL")
	{
		MenuURL = CurrentMenu[Menu_At].Data;
	}
	else if ( Cmd ~= "AddURL")
	{
		AddToURL(CurrentMenu[Menu_At].Data);
	}
	else if ( Cmd ~= "DelURL")
	{
		RemoveFromURL(CurrentMenu[Menu_At].Data);
	}
	else if ( Cmd ~= "ClearURL")
	{
		MenuURL = "";
	}
	else if ( Cmd ~= "Connect" )
	{
		bOpened = false;
		ViewportConsole.ConsoleCommand( "open "$MenuURL );
	}
	else if ( Cmd ~= "Quit" )
	{
		bOpened = false;
		ViewportConsole.ConsoleCommand( "quit" );
	}


	// New Menus

	else if ( Cmd ~= "MainMenu" )
	{
		LoadMenuData("MainMenu",":: Main Menu ::",MainMenu);
		MenuURL = "";
	}
	else if ( Cmd ~= "MapList" )
	{
		LoadMenuData("MapList",":: Map List ::",MapList);
	}
	else if ( Cmd ~= "DemoMapList" )
	{
		LoadMenuData("DemoMapList",":: Demo Map List ::",DemoMapList);
	}
}

defaultproperties
{
	MainMenu(0)=(Title=" Play A Map ",OverCmd="SetHint",EnterCmd="MapList",Data="",Hint="Play a map");    
	MainMenu(1)=(Title=" Host a Game ",OverCmd="SetHint",EnterCmd="HostGame",Data="",Hint="Start a server for others");
	MainMenu(2)=(Title=" Join a Game ",OverCmd="SetHint",EnterCmd="JoinGame",Data="",Hint="Play over the LAN");
	MainMenu(3)=(Title=" Quit ",OverCmd="SetHint",EnterCmd="Quit",Data="",Hint="Exit the game");

	DemoMapList(0)=(Title=" Effects Demo ",OverCmd="SetURL",LeaveCmd="ClearURL",EnterCmd="Connect",Data="EffectsDemo");	
	
	MapList(0)=(Title=" DeathMatch in Test",OverCmd="SetURL",LeaveCmd="ClearURL",EnterCmd="connect",Data="dm-test?numplay=4");	
}
