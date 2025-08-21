class avaSavedCam extends Object
	config(SavedCam);

struct	CamData
{
	var string	MapFileName;
	var int		SlotNum;
	var vector	loc;
	var rotator rot;
};

var config array < CamData >	SavedCamData;

static function SaveCamData( string MapFileName, int SlotNum, vector loc, rotator rot )
{
	local int	i;
	local int	index;
	local bool	bFind;
	bFind = false;
	for ( i = 0 ; i < default.SavedCamData.length ; ++ i )
	{
		if ( default.SavedCamData[i].MapFileName == MapFileName && 
			 default.SavedCamData[i].SlotNum == SlotNum )
		{
			default.SavedCamData[i].loc = loc;
			default.SavedCamData[i].rot = rot;
			bFind = true;
			break;
		}
	}

	if ( bFind == false )
	{
		index = default.SavedCamData.length;
		default.SavedCamData.length = index + 1;
		default.SavedCamData[index].MapFileName = MapFileName;
		default.SavedCamData[index].SlotNum	 = SlotNum;
		default.SavedCamData[index].loc		 = loc;
		default.SavedCamData[index].rot		 = rot;
	}

	StaticSaveConfig();
}

static function bool GetCamData( string MapFileName, int SlotNum, out vector loc, out rotator rot )
{
	local int	i;
	for ( i = 0 ; i < default.SavedCamData.length ; ++ i )
	{
		if ( default.SavedCamData[i].MapFileName == MapFileName && 
			 default.SavedCamData[i].SlotNum == SlotNum )
		{
			loc = default.SavedCamData[i].loc;
			rot = default.SavedCamData[i].rot;
			return true;
		}
	}	
	return false;
}
