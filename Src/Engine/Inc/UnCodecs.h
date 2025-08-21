/*=============================================================================
	UnCodecs.h: Movie codec definitions.
	Copyright 2006 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef CODECS_H
#define CODECS_H

#ifndef USE_BINK_CODEC
#define USE_BINK_CODEC 0
#endif

#if SUPPORTS_PRAGMA_PACK
#pragma pack (push,8)
#endif
#include <theora/theora.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#if SUPPORTS_PRAGMA_PACK
#pragma pack (pop)
#endif

/**
 * Implementation of Theora <http://www.theora.org> decoder.
 */
class UCodecMovieTheora : public UCodecMovie
{
	DECLARE_CLASS(UCodecMovieTheora,UCodecMovie,CLASS_Intrinsic,Engine)

	/**
	* Not all codec implementations are available
	* @return TRUE if the current codec is supported
	*/
	virtual UBOOL IsSupported();
	/**
	* Returns the movie width.
	*
	* @return width of movie.
	*/
	virtual UINT GetSizeX();
	/**
	* Returns the movie height.
	*
	* @return height of movie.
	*/
	virtual UINT GetSizeY();
	/** 
	* Returns the movie format.
	*
	* @return format of movie.
	*/
	virtual EPixelFormat GetFormat();
	/**
	* Returns the framerate the movie was encoded at.
	*
	* @return framerate the movie was encoded at.
	*/
	virtual FLOAT GetFrameRate();	
	/**
	* Initializes the decoder to stream from disk.
	*
	* @param	Filename	Filename of compressed media.
	* @param	Offset		Offset into file to look for the beginning of the compressed data.
	* @param	Size		Size of compressed data.
	*
	* @return	TRUE if initialization was successful, FALSE otherwise.
	*/
	virtual UBOOL Open( const FString& Filename, DWORD Offset, DWORD Size );
	/**
	* Initializes the decoder to stream from memory.
	*
	* @param	Source		Beginning of memory block holding compressed data.
	* @param	Size		Size of memory block.
	*
	* @return	TRUE if initialization was successful, FALSE otherwise.
	*/
	virtual UBOOL Open( void* Source, DWORD Size );

	virtual void Close();
	/**
	* Resets the stream to its initial state so it can be played again from the beginning.
	*/
	virtual void ResetStream();
	/**
	* Queues the request to retrieve the next frame.
	*
	* @param InTextureMovieResource - output from movie decoding is written to this resource
	*/
	virtual void GetFrame( class FTextureMovieResource* InTextureMovieResource );

	virtual void Play(UBOOL bLooping, UBOOL bOneFrameOnly);
	virtual void Pause(UBOOL bPause);
	virtual void Stop();


	// UObject interface.

	/**
	 * Tears down stream by calling Close.	
	 */
	virtual void FinishDestroy();

	virtual void FeedAudio();

	INT GrabAudio( INT NumBytesToFill, BYTE* Buffer );
	void FeedAudio( INT Bytes, BYTE* Buffer );

	virtual FLOAT EnableAudioFeed( UBOOL bEnable );

protected :
	void UploadVideo( FTexture2DRHIRef Texture2DRHI );

private:
	// Theora/ Ogg status objects.
	ogg_packet			OggPacket;
	ogg_sync_state		OggSyncState;
	ogg_page			OggPage;
	ogg_stream_state	TheoraStreamState;
	ogg_stream_state	VorbisStreamState;
	theora_info			TheoraInfo;
	theora_comment		TheoraComment;
	theora_state		TheoraDecoder;
	vorbis_info			VorbisInfo;
	vorbis_dsp_state	VorbisDSPState;
	vorbis_block		VorbisBlock;
	vorbis_comment		VorbisComment;		
	ogg_int64_t			Audio_Granulepos;
	UBOOL				bAudioReady;
	UBOOL				bVideoReady;

	/** Current codec's time */
	DOUBLE				MovieTime;

	/** Current video frame's time */	
	DOUBLE				VideoTime;
	DOUBLE				PlayStartTime;

	/** Handle to file we're streaming from, NULL if streaming directly from memory.		*/
	FILE*				FileHandle;
	
	/** Pointer to memory we're streaming from, NULL if streaming from disk.				*/
	BYTE*				MemorySource;
	/** Number of compressed bytes remaining in input stream								*/
	DWORD				BytesRemaining;

	/** Original offset into file as passed to Open.										*/
	DWORD				OrigOffset;
	/** Original size of input stream.														*/
	DWORD				OrigCompressedSize;
	/** Original memory location as passed to Open.											*/
	BYTE*				OrigSource;

	/** Whether the decoder has bee successfully initialized.								*/
	UBOOL				Initialized;
	UBOOL				bVorbisActivated;	

	UBOOL				bLooping;
	UBOOL				bPlaying;
	UBOOL				bOneFrameOnly;

	/**
	 * Reads an Ogg packet from either memory or disk and consumes it via the ogg stream
	 * handling.
	 *
	 * @return the amount of bytes read.
	 */
	INT ReadData();

	/**
	 * Queries whether we're out of data/ for the end of the source stream.
	 *	
	 * @return TRUE if the end of the source stream has been reached, FALSE otherwise.
	 */
	UBOOL IsEndOfBuffer();

	/**
	 * Inits the decoder by parsing the first couple of packets in the stream. This function
	 * contains the shared initialization code and is used by both Open functions.
	 *
	 * @return TRUE if decoder was successfully initialized, FALSE otherwise
	 */
	UBOOL InitDecoder();

	void GrabVideoFrame();	
	void FeedData();
};

/**
 * Dummy implementation for PC
 */
class UCodecMovieWMV : public UCodecMovie
{
	DECLARE_CLASS(UCodecMovieWMV,UCodecMovie,CLASS_Intrinsic,Engine)	
};

#endif //CODECS_H




