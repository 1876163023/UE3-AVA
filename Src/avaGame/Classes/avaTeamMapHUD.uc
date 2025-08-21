/**
 * Copyright ?2004-2005 Epic Games, Inc. All Rights Reserved.
 */
class avaTeamMapHUD extends avaTeamHUD;

`include(avaGame/avaGame.uci)

var float CenterPosX, CenterPosY, RadarWidth, RadarRange;
var vector MapCenter;
var float ColorPercent;

var bool bFullScreenMap;

var Material HUDIcons;
var Texture2D HUDIconsT;
var LinearColor TeamLinearColor[2];
var MaterialInstanceConstant GreenIconMaterialInstance;

var float MapScale;

var bool bCtrlPressed;
var bool bAltPressed;


/** JOE's QUICK AND DIRTY UI - PLEASE DON'T USE ELSEWHERE! */

var 		int			MouseX,MouseY;
var			bool		bMouseLPressed, bMouseRPressed;
var const	Texture2D	MouseTexture;

var const Texture2D		ButtonImages[3];

enum	EButtonState
{
	EBS_Inactive,
	EBS_Watched,
	EBS_Pressed,
};

struct avaUIButton
{
	var float			ButtonLeft, ButtonTop, ButtonWidth, ButtonHeight;
	var string			ButtonCaption;
	var bool			bHidden;
};

var array<avaUIButton>	ButtonStack;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();	
	
	GreenIconMaterialInstance = new(Outer) class'MaterialInstanceConstant';
	GreenIconMaterialInstance.SetParent(HUDIcons);
	GreenIconMaterialInstance.SetVectorParameterValue('HUDColor', MakeLinearColor(0,1,0,1));
}

simulated function Tick(float DeltaTime)
{
	Super.Tick(DeltaTime);
	ColorPercent = 0.5f + Cos((WorldInfo.TimeSeconds * 4.0) * 3.14159 * 0.5f) * 0.5f;
}

/** ShowMap()
toggles full screen map mode
*/
exec function ShowMap()
{
	local avaPlayerInput PInput;


	bFullScreenMap = !bFullScreenMap;
	bShowHUD = !bFullScreenMap;

	PInput = avaPlayerInput(PlayerOwner.PlayerInput);

	if (PInput != none)
	{
		if (bFullScreenMap)
		{
			PInput.OnInputAxis=MenuInputAxis;
			PInput.OnInputKey=MenuInputKey;
		}
		else
		{
			PInput.OnInputAxis=none;
			PInput.OnInputKey=none;
		}
	}

}

function DrawDemoHUD()
{
	if ( bFullScreenMap )
		DrawMap();
	else
		Super.DrawDemoHUD();
}

function DrawScore()
{
	DrawMap();
	Super.DrawScore();
}

function DrawMap()
{
	local int PlayerYaw;
	local vector PlayerLocation, ScreenLocation;
	local float RadarScaling, /*HudScaling, */x, y;		
	local array<string> Info;	

	x = 0;
	y = 0;
	Info.Length=0;

/*
	Canvas.SetPos(0.5*Canvas.ClipX - 200, 0.5*Canvas.ClipY - 200);
	Canvas.SetDrawColor(255,255,255,255);
	Canvas.DrawTile(HudIconsT,400,400,0,0,512,512);
*/
	if ( bFullScreenMap )
	{
		CenterPosY = 0.5*Canvas.ClipY;
		CenterPosX = 0.5*Canvas.ClipX;
		RadarWidth = 1.8*CenterPosY;
		MapScale = Canvas.ClipX/512;
//		HudScaling = Canvas.ClipX/640;
	}
	else
	{
		CenterPosY = 0.125*Canvas.ClipY;
		CenterPosX = Canvas.ClipX - CenterPosY;
		RadarWidth = 2*CenterPosY;
		MapScale = Canvas.ClipX/640;
	}
	
	
	RadarScaling = RadarWidth/RadarRange;
	
	Canvas.SetPos(CenterPosX - 0.5*RadarWidth, CenterPosY - 0.5*RadarWidth);
	if (bFullScreenMap)
	{
		Canvas.SetDrawColor(0,0,0,128);
	}
	else
	{
		Canvas.SetDrawColor(0,0,0,64);
	}

	Canvas.DrawTile(Canvas.DefaultTexture,RadarWidth,RadarWidth,1,1,32,32);
	Canvas.SetPos(CenterPosX - 0.5*RadarWidth, CenterPosY - 0.5*RadarWidth);
	Canvas.SetDrawColor(255,255,0,0);

	if (bFullScreenMap)
	{
		Canvas.DrawBox(RadarWidth, RadarWidth);
	}

//Players: 160, 0  32,32
//Player Leader: 160, 32  32,32	
	if ( PawnOwner != None )
	{
		ScreenLocation = PawnOwner.Location - MapCenter;
		ScreenLocation.Z = 0;
		PlayerLocation.X = CenterPosX + ScreenLocation.X * RadarScaling;
		PlayerLocation.Y = CenterPosY + ScreenLocation.Y * RadarScaling;
		Canvas.DrawColor = class'avaTeamInfo'.Default.BaseTeamColor[2];
		PlayerYaw = PawnOwner.Rotation.Yaw;
		DrawRotatedTile(GreenIconMaterialInstance, PlayerLocation, PlayerYaw, 12, 12, 0.625, 0, 0.125, 0.125);
	}


	if ( bFullScreenMap )
	{
		DrawMapMenu();
	}

	if (Info.Length>0)
	{
		DrawInfo(x,y,Info);
	}

}

function DrawInfo(float OrgX, float OrgY, array<String> Info)
{
	local int i;
	local float w,h,ys,xl,yl;

	Canvas.Font = class'Engine'.Static.GetSmallFont();
	Canvas.StrLen(Info[0],xl,yl);
	w = xl;
	h = yl;
	ys = h;
	Canvas.Font = class'Engine'.Static.GetTinyFont();
	for (i=1;i<Info.Length;i++)
	{
		Canvas.StrLen(Info[i],xl,yl);
		if (xl>w)
		{
			w = xl;
		}
	}
	h += yl;

	OrgX+=10;
	OrgY+=20;

	Canvas.Font = class'Engine'.Static.GetSmallFont();
	Canvas.SetPos(OrgX, OrgY);
	Canvas.SetDrawColor(0,0,0,255);
	Canvas.DrawRect(w,h);
	Canvas.SetPos(OrgX, OrgY);
	Canvas.SetDrawColor(255,255,255,255);
	Canvas.DrawText(Info[0]);
	Canvas.Font = class'Engine'.Static.GetTinyFont();
	OrgY += ys;
	for (i=1;i<Info.Length;i++)
	{
		Canvas.SetPos(OrgX, OrgY);
		Canvas.DrawText(Info[i]);
		OrgY += YL;
	}
}


function UpdateHUDLocation( Actor A )
{
	local vector ScreenLocation, NewHUDLocation;

    ScreenLocation = A.Location - MapCenter;
	NewHUDLocation.X = CenterPosX + ScreenLocation.X * (RadarWidth/RadarRange);
	NewHUDLocation.Y = CenterPosY + ScreenLocation.Y * (RadarWidth/RadarRange);
	NewHUDLocation.Z = 0;
	A.SetHUDLocation(NewHUDLocation);
}

function DrawRotatedTile(MaterialInstanceConstant M, vector MapLocation, int InYaw, float XWidth, float YWidth, float XStart, float YStart, float XLength, float YLength)
{
	local vector Corners[4];
	local int i;
	local float MinX, MaxX, MinY, MaxY, RotAngle;

	RotAngle = (16384-InYaw)* PI/32768;
	XStart += 0.00781; // 1/256
	YStart += 0.00781;
	XLength -= 0.01562;
	YLength -= 0.01562;

	Canvas.SetPos(MapLocation.X - 0.5*XWidth*MapScale, MapLocation.Y - 0.5*YWidth*MapScale);
	M.SetScalarParameterValue('TexRotation', 4*RotAngle);

	Corners[0].X = XStart;
	Corners[0].Y = YStart;
	for ( i=1; i<4; i++ )
	{
		Corners[i] = Corners[0];
	}
	Corners[1].X += XLength;
	Corners[2].Y += YLength;
	Corners[3].X += XLength;
	Corners[3].Y += YLength;

	for ( i=0; i<4; i++ )
	{
		RotateCorner(Corners[i].X, Corners[i].Y, RotAngle);
	}

	MinX = Corners[0].X;
	MaxX = MinX;
	MinY = Corners[0].Y;
	MaxY = MinY;
	for ( i=1; i<4; i++ )
	{
		if ( Corners[i].X < MinX )
			MinX = Corners[i].X;
		else if ( Corners[i].X > MaxX )
			MaxX = Corners[i].X;

		if ( Corners[i].Y < MinY )
			MinY = Corners[i].Y;
		else if ( Corners[i].Y > MaxY )
			MaxY = Corners[i].Y;
	}

	XStart = MinX + 0.5*(MaxX-MinX-XLength);
	YStart = MinY + 0.5*(MaxY-MinY-YLength);

	Canvas.DrawMaterialTile(M, XWidth*MapScale, YWidth*MapScale, XStart, YStart, XLength, YLength);
}

function RotateCorner(out float X, out float Y, float RotAngle)
{
	local float Angle, Mag;

	// adjust coordinates since rotation center is at 0.5,0.5 and Y is inverted
	X -= 0.5;
	Y = 0.5 - Y;

	// convert to polar coordinates and add angle adjustment
	Angle = Atan(Y,X) + RotAngle;
	Mag = Sqrt(X*X+Y*Y);

	// convert back to cartesian coordinates
	X = cos(Angle) * Mag;
	Y = sin(Angle) * Mag;

	// adjust coordinates back to texture coordinate system
	X += 0.5;
	Y = 0.5 - Y;
}

function vector ScreenToWorld(int MapX, int MapY)
{
	local vector WorldOffset;

	WorldOffset.X = MapX - CenterPosX;
	WorldOffset.Y = MapY - CenterPosY;
	WorldOffset = MapCenter + ( WorldOffset / (RadarWidth/RadarRange) );

	if ( WorldOffset == vect(0,0,0) )
	{
		return vect(0,0,1);
	}
	else
	{
		return WorldOffset;
	}
}

function WorldToScreen(vector Loc, out float X, out float Y)
{
	local vector ScreenLocation;

    ScreenLocation = Loc - MapCenter;
	X = CenterPosX + ScreenLocation.X * (RadarWidth/RadarRange);
	Y = CenterPosY + ScreenLocation.Y * (RadarWidth/RadarRange);
}


// ===========================================================================================================
// This is a very quick and simple test ui for Steve!
// ===========================================================================================================

function DrawMapMenu()
{
	// Clamp out the mouse

	MouseX = clamp(MouseX,0,Canvas.ClipX);
	MouseY = clamp(MouseY,0,Canvas.ClipY);
	
	DrawButtons();
	DrawMenuCursor();
}


function DrawButtons()
{
	local int i;
	local float XL,YL,X,Y,W,H,S;

	S = Canvas.ClipX / 640;

	Canvas.Font = class'Engine'.Static.GetSmallFont();

	X = (5 * S);
	Y = Canvas.ClipY - (10 * s);

	for (i=0;i<ButtonStack.Length;i++)
	{
		Canvas.StrLen(ButtonStack[i].ButtonCaption,XL,YL);
		if (XL > W)
			W = XL;
		if (YL > H)
			H = YL;
	}

	for (i=0;i<ButtonStack.Length;i++)
	{

		ButtonStack[i].ButtonWidth  = W * 1.3;
		ButtonStack[i].ButtonHeight = H * 1.5;

		ButtonStack[i].ButtonLeft 	= X;

		Y = Y - (ButtonStack[i].ButtonHeight + (10 * s) );

		ButtonStack[i].ButtonTop  = Y + (10*s);

		if ( ButtonHitTest(i) )
		{
			if (bMouseLPressed)
			{
				DrawButton(i,EBS_Pressed);
			}
			else
			{
				DrawButton(i,EBS_Watched);
			}
		}
		else
		{
			DrawButton(i,EBS_Inactive);
		}
	}
}

function DrawButton(int Index, EButtonState ButtonState)
{
	local float xl,yl,x,y;
	local int xmod,ymod;

	if ( ButtonStack[Index].bHidden )
		return;

	Canvas.Font = class'Engine'.Static.GetSmallFont();

	Canvas.SetPos(ButtonStack[Index].ButtonLeft, ButtonStack[Index].ButtonTop);
	Canvas.SetDrawColor(255,255,255,255);
	Canvas.DrawTile(ButtonImages[int(ButtonState)],ButtonStack[Index].ButtonWidth,ButtonStack[Index].ButtonHeight,0,0,64,64);

	Canvas.StrLen(ButtonStack[Index].ButtonCaption,xl,yl);
	x = ButtonStack[Index].ButtonLeft + (ButtonStack[Index].ButtonWidth/2)  - (xl/2);
	y = ButtonStack[Index].ButtonTop  + (ButtonStack[Index].ButtonHeight/2) - (yl/2);

	Canvas.SetDrawColor(255,255,255,255);
	for (ymod=-1; ymod<2; ymod++)
	{
		for(xmod=-1; xmod<2; xmod++)
		{
			Canvas.SetPos(x+xmod,y+ymod);
			Canvas.DrawText(ButtonStack[Index].ButtonCaption);
		}
	}

	Canvas.SetDrawColor(0,0,0,255);
	Canvas.SetPos(x,y);
	Canvas.DrawText(ButtonStack[Index].ButtonCaption);

}

function bool ButtonHitTest(int Index)
{
	return IsUnder( ButtonStack[Index].ButtonLeft,
					ButtonStack[Index].ButtonTop,
					ButtonStack[Index].ButtonLeft + ButtonStack[Index].ButtonWidth,
					ButtonStack[Index].Buttontop + ButtonStack[Index].ButtonHeight);
}

function bool IsUnder(float x1, float y1, float x2, float y2)
{
	return (MouseX >= X1 && MouseX <= X2) && (MouseY>=Y1 && MouseY <= Y2);
}

function DrawMenuCursor()
{
	Canvas.SetDrawColor(255,255,255,255);
	Canvas.SetPos(MouseX,MouseY);
	Canvas.DrawTile(MouseTexture,32,32,0,0,32,32);
}

function bool MenuInputKey(int ControllerID, name Key, EInputEvent Event, float AmountDepressed)
{
	local int i;

	if ( Key == 'LeftControl' || Key == 'RightControl' )
	{
		bCtrlPressed = Event != IE_Released;
		return true;
	}

	if ( Key == 'LeftAlt' || Key == 'RightAlt' )
	{
		bAltPressed = Event != IE_Released;
		return true;
	}

	if (Key == 'LeftMouseButton')
	{
		bMouseLPressed = Event == IE_Pressed;

		for (i=0;i<ButtonStack.Length;i++)
		{
			if ( ButtonHitTest(i) )
			{
				if (Event == IE_Released)
				{
					ButtonPressed(ButtonStack[i].ButtonCaption);
				}
				return true;
			}
		}

		if (Event == IE_Released)
		{			

			MouseReleased(Key);
		}
		else if (Event == IE_Pressed)
		{
			MousePressed(Key);
		}


		return true;
	}

	if (Key == 'RightMouseButton')
	{
		bMouseRPressed = Event == IE_Pressed;

		if (Event == IE_Pressed)
		{
			MousePressed(Key);
		}
		else
		{
			MouseReleased(Key);
		}

		return true;
	}

	if ( Key == 'Escape' )
	{
		if ( Event == IE_Released )
		{
			CloseMenu();
		}
		return true;
	}

	return false;
}

function MousePressed(name MouseButton);

function MouseReleased(name MouseButton);

function bool MenuInputAxis(int ControllerID, name Key, float Delta, float DeltaTime)
{
	if (Key == 'mousex')
	{
		MouseX += int(Delta);
	}
	else if (Key == 'mousey')
	{
		MouseY -= int(Delta);	// Negate the Y
	}
	return true;
}


function ButtonPressed(string Caption)
{
	if ( Caption ~= "close" )
	{
		CloseMenu();
	}

}

function CloseMenu()
{
	ShowMap();
}

defaultproperties
{
	/*HUDIcons=Material'UI_HUD.Icons.M_UI_HUD_Icons01'
	HUDIconsT=Texture2D'UI_HUD.Icons.T_UI_HUD_Icons01'*/
	TeamLinearColor(0)=(R=1.0)
	TeamLinearColor(1)=(B=1.0)

	/*MouseTexture=Texture2D'T_UTHudGraphics.Textures.T_MouseCursor'

	ButtonImages(0)=Texture2D'T_UTHudGraphics.Textures.T_ButtonInactive'
	ButtonImages(1)=Texture2D'T_UTHudGraphics.Textures.T_ButtonHighlighted'
	ButtonImages(2)=Texture2D'T_UTHudGraphics.Textures.T_ButtonPressed'*/

	ButtonStack(0)=(ButtonCaption="Close")
}
