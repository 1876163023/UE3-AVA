class avaColorCorrectionManager extends Actor
	native;

// Point
struct native ColorAreaPointType
{
	var int								id;
	var avaNavPoint_ColorCorrection		Point;
};

// Volume
struct native ColorAreaVolumeType
{
	var int								id;
	var avaVolume_ColorCorrection		Volume;
};

struct native ColorAreaVolumeUpdateDataType
{
	var avaVolume_ColorCorrection		Volume;
	var float							Weight;
};

// Event
enum EColorCorrectionEventType
{
	CCEvent_FadeIn,
	CCEvent_FadeOut,
	CCEvent_DisplayOnce,
};

struct native ColorCorrectionEventCmdType
{
	var EColorCorrectionEventType	EventType;
	var Name						EventName;
	var float						Weight;
	var float						FadeTime;
	var bool						bRemove;
};

struct native ColorCorrectionEventDataType
{
	var Name							Name;
	var float							Hue;
	var float							Saturation;
	var float							Lightness;
	var float							Contrast;
	var vector							Shadows;
	var vector							HighLights;
	var vector							MidTones;
	var float							Desaturation;

	var int								id;

	structdefaultproperties
	{
		Hue=0.0
		Saturation=0.0
		Lightness=0.0
		Contrast=0.0
		Shadows=(X=0.0,Y=0.0,Z=0.0)
		HighLights=(X=1.0,Y=1.0,Z=1.0)
		MidTones=(X=1.0,Y=1.0,Z=1.0)
		Desaturation=0.0
	}
};


// ColorCorrectionNavPoints
var array<ColorAreaPointType>		ColorCorrectionPoints;
var array<avaNavPoint_ColorCorrection>		CurrentPoints;

// ColorCorrectionVolumes
var array<ColorAreaVolumeType>		ColorCorrectionVolumes;
var array<ColorAreaVolumeUpdateDataType>	IncreaseVolumes;
var array<ColorAreaVolumeUpdateDataType>	DecreaseVolumes;

// Check the Actor is in Volume
var Actor									BasedViewTarget;

// ColorCorrectionEvent
var array<ColorCorrectionEventDataType>		ColorCorrectionEvents;
var array<ColorCorrectionEventCmdType>		ColorCorrectionEventCmds;

function PostBeginPlay()
{
	Local avaNavPoint_ColorCorrection Point;
	Local avaVolume_ColorCorrection Volume;
	Local ColorCorrectionEventDataType Event;
	Local Actor A;
	Local int id, length , i;

	BasedViewTarget = avaPlayerController(Owner).ViewTarget;
	SetLocation(BasedViewTarget.Location);
	SetBase(BasedViewTarget);

	/** CollideActors=true */
	SetCollision(true);

	/** 중간에 저장되어있을지 모를 ColorArea를 비운다*/
	ClearColorArea();

	/** 모든 ColorCorrection Point, Volume을 찾아 저장해둔다 */
	foreach AllActors(class'Actor', A)
	{
		Point = avaNavPoint_ColorCorrection(A);
		Volume = avaVolume_ColorCorrection(A);

		if( Point != None )
		{
			CreateTexture(id, Point.MaximumWeight, Point.PixelFormat, Point.Hue , Point.Saturation, Point.Lightness , Point.Contrast, Point.Shadows, Point.Highlights, Point.MidTones, Point.Desaturation);
			length = ColorCorrectionPoints.Length;
			ColorCorrectionPoints.Add(1);
			ColorCorrectionPoints[length].Point = Point;
			ColorCorrectionPoints[length].id = id;
		}
		else if ( Volume != None )
		{
			CreateTexture(id, Volume.MaximumWeight, Volume.PixelFormat, Volume.Hue, Volume.Saturation, Volume.Lightness, Volume.Contrast, Volume.Shadows, Volume.Highlights, Volume.MidTones, Volume.Desaturation);
			length = ColorCorrectionVolumes.Length;
			ColorCorrectionVolumes.Add(1);
			ColorCorrectionVolumes[length].Volume = Volume;
			ColorCorrectionVolumes[length].id = id;
		}
	}

	for( i = 0 ; i < ColorCorrectionEvents.Length ; i++ )
	{
		Event = ColorCorrectionEvents[i];
		CreateTexture(id, 1.0, PF_FloatRGBA,Event.Hue, Event.Saturation, Event.Lightness, Event.Contrast, Event.Shadows, Event.HighLights, Event.MidTones, Event.Desaturation, true);
		ColorCorrectionEvents[i].id = id;
	}
}

function UpdateColorCorrectionPoint(float DeltaTime)
{
	Local avaPlayerController PC;
	Local int i, FindIndex;
	Local float Distance, Weight;
	Local ColorAreaPointType PointData;
	Local avaNavPoint_ColorCorrection Point;
	Local vector PawnLocation;
	Local rotator PawnRotation;

	Assert( avaPlayerController(Owner) != None );
	PC = avaPlayerController(Owner);

	if( PC == None )
		return;

	PC.GetPlayerViewPoint(PawnLocation, PawnRotation);

	for(i = 0 ; i < ColorCorrectionPoints.Length ; i++)
	{
		PointData = ColorCorrectionPoints[i];
		Point = PointData.Point;		
		Distance = VSize(Point.Location - PawnLocation);
		if( Distance < Point.FalloffStartDistance )
		{
			FindIndex = CurrentPoints.find(Point);
			if( FindIndex < 0 )
			{
				CurrentPoints[CurrentPoints.Length] = Point;
			}
			ActivateColorArea( PointData.id, Point.MaximumWeight);
		}
		else if( Distance < Point.FalloffEndDistance )
		{
			FindIndex = CurrentPoints.find(Point);
			if( FindIndex < 0 )
			{
				CurrentPoints[CurrentPoints.Length] = Point;
			}
			Weight = Point.MaximumWeight * ( (Point.FalloffEndDistance - Distance) / (Point.FalloffEndDistance - Point.FalloffStartDistance) );
			ActivateColorArea( PointData.id, Weight );
		}
		else
		{
			FindIndex = CurrentPoints.find(Point);
			if( FindIndex >= 0)
			{
				CurrentPoints.Remove(FindIndex,1);
				DeactivateColorArea(PointData.id);
			}
		}
	}
}

function UpdateColorCorrectionVolume( float DeltaTime )
{
	Local avaVolume_ColorCorrection Volume;
	Local int FindIndex, length, i;
	Local float DeltaWeight;
	Local array<avaVolume_ColorCorrection> CurrentVolumes;

	if( avaPlayerController(Owner).ViewTarget != BasedViewTarget )
	{
		BasedViewTarget = avaPlayerController(Owner).ViewTarget;
		SetLocation(BasedViewTarget.Location);
		SetBase(BasedViewTarget);
	}
	
	foreach TouchingActors(class'avaVolume_ColorCorrection', Volume)
	{
		length = CurrentVolumes.Length;
		CurrentVolumes[length] = Volume;
	}

	// Volume 정리

	// 증가중인 Volume들중 현재 빠진 볼륨이 있으면 IncreseVolume -> DecreaseVolume
	for( i = IncreaseVolumes.Length - 1 ; i >= 0 ; i-- )
	{
		FindIndex = CurrentVolumes.find( IncreaseVolumes[i].Volume );
		if( FindIndex < 0 )
		{
			DecreaseVolumes[DecreaseVolumes.Length] = IncreaseVolumes[i];
			IncreaseVolumes.Remove(i,1);
		}
	}

	// 감소중인 Volume증 현재 찾은 볼륨이 있으면 DecreaseVolume->IncreaseVolume
	for( i = DecreaseVolumes.Length - 1 ; i >= 0 ; i-- )
	{
		FindIndex = CurrentVolumes.find( DecreaseVolumes[i].Volume );
		if( FindIndex >= 0 )
		{
			IncreaseVolumes[IncreaseVolumes.Length] = DecreaseVolumes[i];
			DecreaseVolumes.Remove(i,1);
		}
	}

	// DecreaseVolume도 아니고 IncreaseVolume도 아닌데 찾은 볼륨이 있으면 NewIncreaseVolume
	for( i = 0 ; i < CurrentVolumes.Length ; i++)
	{
		if( IncreaseVolumes.find('Volume', CurrentVolumes[i] ) < 0 &&
			DecreaseVolumes.find('Volume', CurrentVolumes[i] ) < 0 )
		{
			length = IncreaseVolumes.Length;
			IncreaseVolumes.Add(1);
			IncreaseVolumes[length].Volume = CurrentVolumes[i];
			IncreaseVolumes[length].Weight = 0.0;
		}
	}

	for( i = 0 ; i < IncreaseVolumes.Length ; i++ )
	{
		DeltaWeight = (IncreaseVolumes[i].Volume.MaximumWeight / IncreaseVolumes[i].Volume.FadeTime) * DeltaTime;
		
		IncreaseVolumes[i].Weight = fMin(IncreaseVolumes[i].Volume.MaximumWeight, IncreaseVolumes[i].Weight + DeltaWeight);
		FindIndex = ColorCorrectionVolumes.find('Volume', IncreaseVolumes[i].Volume);
		ActivateColorArea(ColorCorrectionVolumes[FindIndex].id, IncreaseVolumes[i].Weight);
	}
	
	for( i = DecreaseVolumes.Length - 1 ; i >= 0 ; i-- )
	{
		DeltaWeight = (DecreaseVolumes[i].Volume.MaximumWeight / DecreaseVolumes[i].Volume.FadeTime) * DeltaTime;
		DecreaseVolumes[i].Weight = fMax(0.0f , (DecreaseVolumes[i].Weight - DeltaWeight));

		FindIndex = ColorCorrectionVolumes.find('Volume', DecreaseVolumes[i].Volume);
		if( DecreaseVolumes[i].Weight > 0.0 )
			ActivateColorArea(ColorCorrectionVolumes[FindIndex].id, DecreaseVolumes[i].Weight);
		else
		{
			DeactivateColorArea(ColorCorrectionVolumes[FindIndex].id);
			DecreaseVolumes.Remove(i,1);
		}
	}
}

native function ActivateColorArea(int id, float weight);
native function DeactivateColorArea(int id);
native function CreateTexture(out int id, float weight, EPixelFormat PixelFormat,float hue, float sat, float light, float contrast, vector Shadows, vector Highlights, vector MidTones, float Desaturation, bool bSetStrictWeight = false);
native function SetSampleMode(bool bSampleMode);	/**< NormalMode - false , SampleMode - true */
native function ClearColorArea();

function AppendColorCorrectionEvent( Name EventName, EColorCorrectionEventType EventType,float FadeTime)
{
	Local int index;

	index = ColorCorrectionEvents.find('Name', EventName);
	if( index < 0)
	{
		`warn("ColorCorrectionEvent "$EventName$" Not Found when AppendColorCorrectionEvent called");
		return;
	}

	index = ColorCorrectionEventCmds.find('EventName',EventName);
	index = index < 0 ? ColorCorrectionEventCmds.Length : index ;

	ColorCorrectionEventCmds[ index ].EventType = EventType;
	ColorCorrectionEventCmds[ index ].EventName = EventName;
	ColorCorrectionEventCmds[ index ].FadeTime = FadeTime;

	switch(EventType)
	{
	case CCEvent_FadeIn:	ColorCorrectionEventCmds[ index ].Weight = 0.0f; break;
	case CCEvent_FadeOut:	ColorCorrectionEventCmds[ index ].Weight = 1.0f; break;
	case CCEvent_DisplayOnce: ColorCorrectionEventCmds[ index ].Weight = 1.0f; break;
	}
}

function ClearColorCorrectionEvent( optional Name EventName )
{
	Local int i, FindIndex, ClearFindIndex;
	Local array<int> ClearIndices;

	if( EventName == '' )
	{
		for( i = 0 ; i < ColorCorrectionEventCmds.Length ; i++)
		{
			FindIndex = ColorCorrectionEvents.find('Name', ColorCorrectionEventCmds[i].EventName);
			if( FindIndex >= 0)
			{
				ClearFindIndex = ClearIndices.find( FindIndex);
				if( ClearFindIndex < 0 )
					ClearIndices[ ClearIndices.Length ] = ColorCorrectionEvents[FindIndex].id;
			}
		}

		for( i = 0 ; i < ClearIndices.Length ; i++ )
			DeactivateColorArea(ClearIndices[i]);

		ColorCorrectionEventCmds.Length = 0;
	}
	else
	{
		FindIndex = ColorCorrectionEvents.find('Name', ColorCorrectionEventCmds[i].EventName);
		DeactivateColorArea(ColorCorrectionEvents[FindIndex].id);
	}
}

function UpdateColorCorrectionEvent(float DeltaTime) 
{
	Local int i, FindIndex;

	for( i = ColorCorrectionEventCmds.Length - 1 ; i >= 0 ; i-- )
	{
		FindIndex = ColorCorrectionEvents.find('Name', ColorCorrectionEventCmds[i].EventName);
		if( FindIndex < 0 )
		{
			`warn("ColorCorrectionEvent "$ColorCorrectionEventCmds[i].EventName$" not found");
			ColorCorrectionEventCmds.Remove(i,1);
			continue;
		}

		if( ColorCorrectionEventCmds[i].EventType == CCEvent_FadeIn )
		{
			if( ColorCorrectionEventCmds[i].Weight < 1.0 )
			{
				ColorCorrectionEventCmds[i].Weight += (DeltaTime/ColorCorrectionEventCmds[i].FadeTime);
				ActivateColorArea(ColorCorrectionEvents[FindIndex].id , ColorCorrectionEventCmds[i].Weight);
			}
		}
		else if( ColorCorrectionEventCmds[i].EventType == CCEvent_FadeOut )
		{
			ColorCorrectionEventCmds[i].Weight -= (DeltaTime / ColorCorrectionEventCmds[i].FadeTime);
			if( ColorCorrectionEventCmds[i].Weight > 0.0 )
				ActivateColorArea( ColorCorrectionEvents[FindIndex].id, ColorCorrectionEventCmds[i].Weight );
			else
			{
				ColorCorrectionEventCmds.Remove(i,1);
				DeactivateColorArea(ColorCorrectionEvents[FindIndex].id);
			}
		}
		else if( ColorCorrectionEventCmds[i].EventType == CCEvent_DisplayOnce )
		{
			if( ColorCorrectionEventCmds[i].bRemove == false )
			{
				ActivateColorArea( ColorCorrectionEvents[FindIndex].id, ColorCorrectionEventCmds[i].Weight );
				ColorCorrectionEventCmds[i].bRemove = true;
			}
			else
			{
				DeactivateColorArea(ColorCorrectionEvents[FindIndex].id);
				ColorCorrectionEventCmds.Remove(i,1);
			}
		}
	}
}

simulated function PostRender( float DeltaTime)
{
	UpdateColorCorrectionPoint(DeltaTime);
	UpdateColorCorrectionVolume(DeltaTime);
	UpdateColorCorrectionEvent(DeltaTime);
}


defaultproperties
{
	/*Begin Object Class=CylinderComponent Name=CollisionCylinder
	    CollisionRadius=+0034.000000
		CollisionHeight=+0078.000000
		BlockNonZeroExtent=true
		BlockZeroExtent=true
		BlockActors=true
		CollideActors=true
	End Object
	CollisionComponent=CollisionCylinder
	Components.Add(CollisionCylinder)*/

	ColorCorrectionEvents.Add((Name="GunDamage"/*,Lightness=-15,Contrast=-15*/))
	ColorCorrectionEvents.Add((Name="Shot"/*,Lightness=30.0*/))
	ColorCorrectionEvents.Add((Name="MomentOfDeath"))
}