/**
 * Copyright 2004-2006 Epic Games, Inc. All Rights Reserved.
 */
class SoundNodeWave extends SoundNode
	PerObjectConfig
	native
	noexport
	collapsecategories
	hidecategories(Object)
	editinlinenew;

/** Platform agnostic compression quality. 1..100 with 1 being best compression and 100 being best quality */
var(Compression)		int								CompressionQuality<Tooltip=1 smallest size, 100 is best quality>;
var(Compression)		bool							bCompressedSample;

/** Playback volume of sound 0 to 1 */
var(Info)	editconst const	float						Volume;
/** Playback pitch for sound 0.5 to 2.0 */
var(Info)	editconst const	float						Pitch;
/** Duration of sound in seconds. */
var(Info)	editconst const	float						Duration;
/** Number of channels of multichannel data; 1 or 2 for regular mono and stereo files */
var(Info)	editconst const	int							NumChannels;
/** Cached sample rate for displaying in the tools */
var(Info)	editconst const int							SampleRate;

/** Offsets into the bulk data for the source wav data */
var		native const	array<int>						ChannelOffsets;
/** Sizes of the bulk data for the source wav data */
var		native const	array<int>						ChannelSizes;
/** Uncompressed wav data 16 bit in mono or stereo - stereo not allowed for multichannel data */
var		native const	UntypedBulkData_Mirror			RawData;

/** Pointer to 16 bit PCM data - used to preview sounds */
var		native const	pointer							RawPCMData;

/** Whether to free the resource data after it has been uploaded to the hardware */
var		transient const bool							DynamicResource;

/** Cached ogg vorbis data. */
var		native const	UntypedBulkData_Mirror			CompressedPCData;
/** Cached cooked Xbox 360 data to speed up iteration times. */
var		native const	UntypedBulkData_Mirror			CompressedXbox360Data;
/** Cached cooked PS3 data to speed up iteration times. */
var		native const	UntypedBulkData_Mirror			CompressedPS3Data;

var		transient const int					ResourceID;

/** Size of resource copied from the bulk data */
var		transient const int								ResourceSize;
/** Memory containing the data copied from the compressed bulk data */
var		native const pointer							ResourceData{const BYTE};

/** A localized version of the text that is actually spoken in the audio. */
var() localized string			SpokenText;

/**
 * A line of subtitle text and the time at which it should be displayed.
 */
struct native SubtitleCue
{
	/** The text too appear in the subtitle. */
	var() localized string	Text;

	/** The time at which the subtitle is to be displayed, in seconds relative to the beginning of the line. */
	var() localized float	Time;
};

/**
 * Subtitle cues.  If empty, use SpokenText as the subtitle.  Will often be empty,
 * as the contents of the subtitle is commonly identical to what is spoken.
 */
var() localized array<SubtitleCue>	Subtitles;

/** Provides contextual information for the sound to the translator. */
var() localized string				Comment;

/** TRUE if this sound is considered mature. */
var() localized bool				bMature;

/** TRUE if the subtitles have been split manually. */
var() localized bool				bManualWordWrap;

defaultproperties
{
	Volume=0.75
	Pitch=1.0
	CompressionQuality=40
}
