/*=============================================================================
UnCodecs.cpp: Movie codec implementations, e.g. Theora and Fallback codec.
Copyright 2004-2005 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnCodecs.h"

IMPLEMENT_CLASS(UCodecMovieTheora);

//
//	UCodecMovieTheora implementation.
//

// @todo xenon: port theora codec pending needs and availability of xmv
// @todo movie textures: replace blocking implementation using fread/fclose/fopen with non- blocking
// @todo movie textures: implementation that uses a yet to be designed streaming IO manager.

UBOOL UCodecMovieTheora::IsSupported()
{
	return TRUE;
}

/**
* Returns the movie width.
*
* @return width of movie.
*/
UINT UCodecMovieTheora::GetSizeX() 
{ 
	check(Initialized);
	return TheoraInfo.width; //@todo movie textures: do we want to use the frame_width instead?
}

/**
* Returns the movie height.
*
* @return height of movie.
*/
UINT UCodecMovieTheora::GetSizeY() 
{ 
	check(Initialized);
	return TheoraInfo.height; //@todo movie textures: do we want to use the frame_height instead?
}

/** 
* Returns the movie format, PF_UYVY if supported, PF_A8R8G8B8 otherwise.
*
* @return texture format used for movie.
*/
EPixelFormat UCodecMovieTheora::GetFormat() 
{ 
	check(Initialized);
	//return GPixelFormats[PF_UYVY].Supported ? PF_UYVY : PF_A8R8G8B8;
	//<@ ava specific : 2007. 5. 12 changmin... 이렇게 check 해야지... 이미지가 안깨지지... 
	return (GPixelFormats[PF_UYVY].Supported && !GPixelFormats[PF_UYVY].Flags & PF_REQUIRES_GAMMA_CORRECTION ) ? PF_UYVY : PF_A8R8G8B8;
	//>@ ava
}

/**
* Returns the framerate the movie was encoded at.
*
* @return framerate the movie was encoded at.
*/
FLOAT UCodecMovieTheora::GetFrameRate()
{
	check(Initialized);
	return (FLOAT) TheoraInfo.fps_numerator / TheoraInfo.fps_denominator;
}

/**
* Reads an Ogg packet from either memory or disk and consumes it via the ogg stream
* handling.
*
* @return the amount of bytes read.
*/
INT UCodecMovieTheora::ReadData()
{
	DWORD ReadSize	= 4096;
	char* Buffer	= ogg_sync_buffer( &OggSyncState, ReadSize );

	ReadSize = Min( ReadSize, BytesRemaining );
	if( MemorySource )
	{
		appMemcpy( Buffer, MemorySource, ReadSize );
		MemorySource += ReadSize;
	}
	else
	{
		ReadSize = fread( Buffer, 1, ReadSize, FileHandle );
	}
	BytesRemaining -= ReadSize;

	//debugf( TEXT( "ReadData %d %d"), BytesRemaining, ReadSize );

	ogg_sync_wrote( &OggSyncState, ReadSize );
	return ReadSize;
}

/**
* Queries whether we're out of data/ for the end of the source stream.
*	
* @return TRUE if the end of the source stream has been reached, FALSE otherwise.
*/
UBOOL UCodecMovieTheora::IsEndOfBuffer()
{
	return BytesRemaining <= 0;
}

/**
* Initializes the decoder to stream from disk.
*
* @param	Filename	Filename of compressed media.
* @param	Offset		Offset into file to look for the beginning of the compressed data.
* @param	Size		Size of compressed data.
*
* @return	TRUE if initialization was successful, FALSE otherwise.
*/
UBOOL UCodecMovieTheora::Open( const FString& Filename, DWORD Offset, DWORD Size )
{
	FileHandle = fopen( TCHAR_TO_ANSI(*Filename), "rb" );
	if( FileHandle == NULL )
	{
		// Error.
		return FALSE;
	}
	fseek( FileHandle, Offset, SEEK_SET );

	OrigOffset				= Offset;
	BytesRemaining			= Size;
	OrigCompressedSize		= Size;

	MemorySource			= NULL;
	OrigSource				= NULL;

	return InitDecoder();
}

/**
* Initializes the decoder to stream from memory.
*
* @param	Source		Beginning of memory block holding compressed data.
* @param	Size		Size of memory block.
*
* @return	TRUE if initialization was successful, FALSE otherwise.
*/
UBOOL UCodecMovieTheora::Open( void* Source, DWORD Size )
{
	MemorySource			= (BYTE*) Source;
	OrigSource				= (BYTE*) Source;
	BytesRemaining			= Size;
	OrigCompressedSize		= Size;

	FileHandle				= NULL;
	OrigOffset				= 0;

	return InitDecoder();
}

/**
* Resets the stream to its initial state so it can be played again from the beginning.
*/
void UCodecMovieTheora::ResetStream()
{
	check(Initialized);

	ogg_stream_clear( &TheoraStreamState );
	theora_clear( &TheoraDecoder );
	theora_comment_clear( &TheoraComment );
	theora_info_clear( &TheoraInfo );

	if (bVorbisActivated)
	{
		ogg_stream_clear( &VorbisStreamState );
		vorbis_block_clear( &VorbisBlock );
		vorbis_dsp_clear( &VorbisDSPState );
		vorbis_comment_clear( &VorbisComment );
		vorbis_info_clear( &VorbisInfo );

		bVorbisActivated = FALSE;
	}

	ogg_sync_clear( &OggSyncState );	

	if( MemorySource )
	{
		MemorySource = OrigSource;
	}
	else
	{
		fseek( FileHandle, OrigOffset, SEEK_SET );
	}

	BytesRemaining = OrigCompressedSize;

	InitDecoder();
}

/**
* Inits the decoder by parsing the first couple of packets in the stream. This function
* contains the shared initialization code and is used by both Open functions.
*
* @return TRUE if decoder was successfully initialized, FALSE otherwise
*/
UBOOL UCodecMovieTheora::InitDecoder()
{
	bVorbisActivated = FALSE;

	check(!HasAnyFlags(RF_ClassDefaultObject));

	appMemzero( &OggPacket		, sizeof(OggPacket)			);
	appMemzero( &OggSyncState	, sizeof(OggSyncState)		);
	appMemzero( &OggPage		, sizeof(OggPage)			);
	appMemzero( &TheoraStreamState	, sizeof(TheoraStreamState)	);
	appMemzero( &TheoraInfo		, sizeof(TheoraInfo)		);
	appMemzero( &TheoraComment	, sizeof(TheoraComment)		);
	appMemzero( &TheoraDecoder	, sizeof(TheoraDecoder)		);
	appMemzero( &VorbisInfo		, sizeof(VorbisInfo)		);	
	appMemzero( &VorbisComment	, sizeof(VorbisComment)		);	

	ogg_sync_init( &OggSyncState );

	theora_comment_init( &TheoraComment );
	theora_info_init( &TheoraInfo );

	vorbis_info_init( &VorbisInfo );
	vorbis_comment_init( &VorbisComment );

	UBOOL DoneParsing	= 0;
	DWORD TheoraPacketCount	= 0;
	DWORD VorbisPacketCount = 0;
	Initialized			= 0;	
	bAudioReady = FALSE;
	bVideoReady = FALSE;
	VideoTime = 0;

	while( !DoneParsing )
	{
		if( ReadData() == 0 )
			break;

		while( ogg_sync_pageout( &OggSyncState, &OggPage ) > 0 )
		{
			ogg_stream_state TestStream;

			if( !ogg_page_bos( &OggPage ) )
			{				
				if(TheoraPacketCount)
				{				
					ogg_stream_pagein( &TheoraStreamState, &OggPage );
				}
				if(VorbisPacketCount)
				{
					ogg_stream_pagein( &VorbisStreamState, &OggPage );
				}
				DoneParsing = 1;
				break;
			}

			ogg_stream_init( &TestStream, ogg_page_serialno( &OggPage) );
			ogg_stream_pagein( &TestStream, &OggPage );
			ogg_stream_packetout( &TestStream, &OggPacket ) ;

			if( TheoraPacketCount == 0 && theora_decode_header( &TheoraInfo, &TheoraComment, &OggPacket ) >=0 )
			{
				memcpy( &TheoraStreamState, &TestStream, sizeof(TestStream) ); //@todo movie textures: we're using memcpy here instead of appMemcpy as I know memcpy is thread safe
				TheoraPacketCount = 1;
			}
			else if( VorbisPacketCount == 0 && vorbis_synthesis_headerin( &VorbisInfo, &VorbisComment, &OggPacket ) >=0 )
			{
				memcpy( &VorbisStreamState, &TestStream, sizeof(TestStream) ); //@todo movie textures: we're using memcpy here instead of appMemcpy as I know memcpy is thread safe
				VorbisPacketCount = 1;
			}
			else
			{
				ogg_stream_clear( &TestStream );
			}
		}
	}

	while( TheoraPacketCount && TheoraPacketCount < 3 || VorbisPacketCount && VorbisPacketCount < 3)
	{
		INT RetVal;
		while( TheoraPacketCount && TheoraPacketCount < 3 && (RetVal=ogg_stream_packetout( &TheoraStreamState, &OggPacket )) != 0 )
		{
			if( RetVal < 0 )
				return FALSE;

			if( theora_decode_header( &TheoraInfo, &TheoraComment, &OggPacket ) )
				return FALSE;

			TheoraPacketCount++;
		}

		while( VorbisPacketCount && VorbisPacketCount < 3 && (RetVal=ogg_stream_packetout( &VorbisStreamState, &OggPacket )) != 0 )
		{
			if( RetVal < 0 )
				return FALSE;

			if( vorbis_synthesis_headerin( &VorbisInfo, &VorbisComment, &OggPacket ) )
				return FALSE;

			VorbisPacketCount++;
		}


		if( ogg_sync_pageout( &OggSyncState, &OggPage ) > 0 )
		{
			if (TheoraPacketCount)
				ogg_stream_pagein( &TheoraStreamState, &OggPage );
			if (VorbisPacketCount)
				ogg_stream_pagein( &VorbisStreamState, &OggPage );
		}
		else if( ReadData() == 0 )
		{
			return FALSE;		
		}
	}


	if( TheoraPacketCount )
	{
		theora_decode_init( &TheoraDecoder, &TheoraInfo );

		if (VorbisPacketCount)
		{
			bVorbisActivated = TRUE;
			Audio_Granulepos = 0;

			vorbis_synthesis_init( &VorbisDSPState, &VorbisInfo );
			vorbis_block_init( &VorbisDSPState, &VorbisBlock );
		}

		while( ogg_sync_pageout( &OggSyncState, &OggPage ) > 0 )
		{
			ogg_stream_pagein( &TheoraStreamState, &OggPage );

			if (VorbisPacketCount)
				ogg_stream_pagein( &VorbisStreamState, &OggPage );
		}

		Initialized = 1;
		
		bPlaying = FALSE;				
		
		MovieTime = 0;

		return TRUE;
	}
	else
	{
		theora_info_clear( &TheoraInfo );
		theora_comment_clear( &TheoraComment );

		return FALSE;
	}
}

/**
* Tears down stream by calling Close.	
*/
void UCodecMovieTheora::FinishDestroy()
{
	Close();
	Super::FinishDestroy();
}

/**
* Tears down stream, closing file handle if there was an open one.	
*/
void UCodecMovieTheora::Close()
{
	if( Initialized )
	{
		ogg_stream_clear( &TheoraStreamState );
		theora_clear( &TheoraDecoder );
		theora_comment_clear( &TheoraComment );
		theora_info_clear( &TheoraInfo );

		ogg_sync_clear( &OggSyncState );

		if( FileHandle )
		{
			fclose( FileHandle );
		}

		Initialized		= 0;

		appFree(OrigSource);
		OrigSource		= NULL;
		MemorySource	= NULL;

		BytesRemaining	= 0;
		FileHandle		= NULL;
	}
}

extern void AudioDevice_LockStream( INT* Bytes1, void** Buffer1, INT* Bytes2, void** Buffer2 );
extern void AudioDevice_UnlockStream( INT Bytes1, void* Buffer1, INT Bytes2, void* Buffer2 );

void UCodecMovieTheora::FeedAudio() 
{
	UBOOL	EnoughData	= 0;
	INT StreamId = 0;	

	UAudioDevice* AudioDevice = GEngine && GEngine->Client ? GEngine->Client->GetAudioDevice() : NULL;
	if( AudioDevice && bVorbisActivated )
	{
		INT Bytes1, Bytes2;
		BYTE *Buffer1, *Buffer2;

		AudioDevice->LockStream(TRUE);

		AudioDevice_LockStream( &Bytes1, (void**)&Buffer1, &Bytes2, (void**)&Buffer2 );

		if (Bytes1 > 0)
		{
			FeedAudio( Bytes1, Buffer1 );

			if (Bytes2 > 0)
			{
				FeedAudio( Bytes2, Buffer2 );
			}					

			/* 첫번째 buffer가 꽉 찼네! */
			if (!bAudioReady)
			{
				bAudioReady = TRUE;
				AudioDevice->PlayStream( StreamId );
			}								
		}

		AudioDevice_UnlockStream( Bytes1, Buffer1, Bytes2, Buffer2 );

		AudioDevice->LockStream(FALSE);
	}
}

INT UCodecMovieTheora::GrabAudio( INT NumBytesToFill, BYTE* Buffer )
{
	FLOAT** pcm;
	
	// Vorbis에서 현재 얼마나 줄 수 있는가?
	INT NumSamplesVorbisOffers = vorbis_synthesis_pcmout( &VorbisDSPState, &pcm );

	// 없어!!
	while (NumSamplesVorbisOffers == 0)
	{
		// 모자르니까 더 가져 와야지
		ogg_packet opVorbis;

		if( ogg_stream_packetout( &VorbisStreamState, &opVorbis) >0 )
		{
			//test for success!
			if(vorbis_synthesis( &VorbisBlock, &opVorbis) == 0 )
				vorbis_synthesis_blockin( &VorbisDSPState, &VorbisBlock );
		}					
		else
		{
			// 아이고 이게 끝이었어?	
			if( IsEndOfBuffer() )
			{				
				return 0;
			}
			else
			{
				FeedData();
			}
		}

		NumSamplesVorbisOffers = vorbis_synthesis_pcmout( &VorbisDSPState, &pcm );
	}

	INT NumSamplesGrabbed = NumBytesToFill / VorbisInfo.channels / sizeof(SHORT);
	if (NumSamplesGrabbed > NumSamplesVorbisOffers)
		NumSamplesGrabbed = NumSamplesVorbisOffers;

	SHORT* Dest = (SHORT*)Buffer;
	for(INT VorbisSampleIndex=0; VorbisSampleIndex < NumSamplesGrabbed; VorbisSampleIndex++ )
	{
		for(INT ChannelIndex=0; ChannelIndex<VorbisInfo.channels; ChannelIndex++)
		{
			INT val = (INT)(pcm[ChannelIndex][VorbisSampleIndex]*32767.f);

			if(val>32767)	val=32767;
			if(val<-32768)	val=-32768;

			*Dest++ = val;			
		}
	}

	// Vorbis에게 우리가 쓴 sample 수를 알려줘야 한다.
	vorbis_synthesis_read( &VorbisDSPState, NumSamplesGrabbed );

	if( VorbisDSPState.granulepos >= 0 )	
		Audio_Granulepos = VorbisDSPState.granulepos - NumSamplesVorbisOffers + NumSamplesGrabbed;
	else					
		Audio_Granulepos += NumSamplesGrabbed;			
	
	return NumSamplesGrabbed * sizeof(SHORT) * VorbisInfo.channels;
}

void UCodecMovieTheora::FeedAudio( INT Bytes, BYTE* Buffer )
{	
	while (Bytes > 0)
	{
		INT FilledBytes = GrabAudio( Bytes, Buffer );
		if (FilledBytes == 0)
		{
			appMemzero( Buffer, Bytes );
			break;
		}

		Buffer += FilledBytes;
		Bytes -= FilledBytes;
	}	
}

static FLOAT GAudioVideoSyncOffset = 0.f;

FLOAT UCodecMovieTheora::EnableAudioFeed( UBOOL bEnable ) 
{
	INT StreamId = 0;

	UAudioDevice* AudioDevice = GEngine && GEngine->Client ? GEngine->Client->GetAudioDevice() : NULL;
	if( AudioDevice )
	{
		if (bVorbisActivated)
		{	
			if (bEnable) 
			{
				bAudioReady = FALSE;

				return GAudioVideoSyncOffset;
			}				
			else
				AudioDevice->StopStream( StreamId );
		}
	}

	return 0.0f;
}

void UCodecMovieTheora::Play(UBOOL bLooping, UBOOL bOneFrameOnly)
{
	ResetStream();

	this->bLooping = bLooping;
	this->bPlaying = !bOneFrameOnly;
	this->bOneFrameOnly = bOneFrameOnly;	

	if (!bOneFrameOnly)
	{
		if (bVorbisActivated)
		{
			bAudioReady = FALSE;

			FeedAudio();

			PlayStartTime = appSeconds() + 0.5f;
		}
		else
		{
			PlayStartTime = appSeconds();
		}
	}	
}

void UCodecMovieTheora::Pause(UBOOL bPause)
{
	bPlaying = !bPause;

	if (bPlaying)
	{
		PlayStartTime = appSeconds() - MovieTime;
	}
}

void UCodecMovieTheora::Stop()
{
	if (bVorbisActivated)
	{
		INT StreamId = 0;

		UAudioDevice* AudioDevice = GEngine && GEngine->Client ? GEngine->Client->GetAudioDevice() : NULL;
		if( AudioDevice )
		{
			AudioDevice->StopStream( 0 );
		}		
	}

	bPlaying = FALSE;
}

/**
* Queues the request to retrieve the next frame.
*
* @param	Destination		Memory block to uncompress data into.
* @param	Size			Size of the destination memory block
*/
void UCodecMovieTheora::GetFrame( class FTextureMovieResource* InTextureMovieResource )
{	
	if( InTextureMovieResource &&
		InTextureMovieResource->IsInitialized() )
	{	
		if (bPlaying)
		{
			MovieTime = appSeconds() - PlayStartTime;		

			if (bVorbisActivated)
			{
				FeedAudio();
			}
		}		

		/* Video frame이 없다면 하나 가져 온다 */
		if (!bVideoReady)
			GrabVideoFrame();

		/// 비디오의 시간이 현재 시간보다 같거나 지나간 경우에만 올린다.
		if (bVideoReady && VideoTime <= MovieTime)
		{
			UploadVideo( InTextureMovieResource->Texture2DRHI );			
			bVideoReady = FALSE;
		}		
	}
}

void UCodecMovieTheora::GrabVideoFrame()
{
	/* Video frame을 받아 올 때까지 반복 */
	while( !bVideoReady )
	{
		while( !bVideoReady )
		{
			/* ogg packet을 구성 */
			if( ogg_stream_packetout( &TheoraStreamState, &OggPacket) > 0 )
			{
				/* theora decode */
				theora_decode_packetin( &TheoraDecoder, &OggPacket );

				/* video 시간 계산 */
				ogg_int64_t GranulePos	= TheoraDecoder.granulepos;
				VideoTime = theora_granule_time( &TheoraDecoder, GranulePos );

				/* 이미 지난 frame은 필요 없다! */
				if (VideoTime >= MovieTime - 1.0f / GetFrameRate())
					bVideoReady = TRUE;
			}
			/* packet을 구성할 수 없다 */
			else
				break;
		}

		/* 데이터가 모자르다 */
		if( !bVideoReady )	
		{
			/* 끝까지 play해서 그런가? */
			if( IsEndOfBuffer() )
			{	
				if (!bLooping)
					return;

				/* rewind */
				ResetStream();

				PlayStartTime = appSeconds();

				continue;
			}
			/* 데이터를 더 긁어온다 */
			else
			{
				FeedData();				
			}
		}
	}	
}

void UCodecMovieTheora::FeedData()
{
	ReadData();

	while( ogg_sync_pageout( &OggSyncState, &OggPage ) > 0 )
	{
		ogg_stream_pagein( &TheoraStreamState, &OggPage );

		if (bVorbisActivated)
			ogg_stream_pagein( &VorbisStreamState, &OggPage );
	}
}

void UCodecMovieTheora::UploadVideo( FTexture2DRHIRef Texture2DRHI )
{	
	UINT SrcStride;
	void* Dest = RHILockTexture2D( Texture2DRHI, 0, TRUE, SrcStride );						

	yuv_buffer YUVBuffer;
	theora_decode_YUVout( &TheoraDecoder, &YUVBuffer );

	DWORD	SizeX	= TheoraInfo.width,
		SizeY	= TheoraInfo.height;

	char*	SrcY	= YUVBuffer.y;
	char*	SrcU	= YUVBuffer.u;
	char*	SrcV	= YUVBuffer.v;

	check( YUVBuffer.y_width  == YUVBuffer.uv_width  * 2 );
	check( YUVBuffer.y_height == YUVBuffer.uv_height * 2 );
	check( YUVBuffer.y_width  == GetSizeX() );
	check( YUVBuffer.y_height == GetSizeY() );

	if( GPixelFormats[PF_UYVY].Supported 
	&& !GPixelFormats[PF_UYVY].Flags & PF_REQUIRES_GAMMA_CORRECTION) // 2007. 5. 12 changmin.... 이미지가 깨짐..
	{
		//@todo optimization: Converts from planar YUV to interleaved PF_UYVY format.
		for( UINT y=0; y<SizeY; y++ )
		{
			for( UINT x=0; x<SizeX/2; x++ )
			{
				DWORD			OffsetY0	= YUVBuffer.y_stride  *  y    + x*2;
				DWORD			OffsetY1	= YUVBuffer.y_stride  *  y    + x*2 + 1;
				DWORD			OffsetUV	= YUVBuffer.uv_stride * (y/2) + x;

				unsigned char	Y0			= SrcY[OffsetY0];
				unsigned char   Y1			= SrcY[OffsetY1];
				unsigned char	U			= SrcU[OffsetUV];
				unsigned char	V			= SrcV[OffsetUV];

				((DWORD*)Dest)[y*(SizeX/2) + x] = (Y1 << 24) | (V << 16) | (Y0 << 8) | U; //@todo xenon: I bet this needs to be shuffled around.
			}
		}
	}
	else
	{
		FColor*	DestColor = (FColor*) Dest;

		//@todo optimization: Converts from planar YUV to interleaved ARGB. This codepath is currently hit with NVIDIA cards!
		for( UINT y=0; y<SizeY; y++ )
		{
			for( UINT x=0; x<SizeX/2; x++ )
			{
				DWORD			OffsetY0	= YUVBuffer.y_stride  *  y    + x*2;
				DWORD			OffsetY1	= YUVBuffer.y_stride  *  y    + x*2 + 1;
				DWORD			OffsetUV	= YUVBuffer.uv_stride * (y/2) + x;

				unsigned char	Y0			= SrcY[OffsetY0];
				unsigned char   Y1			= SrcY[OffsetY1];
				unsigned char	U			= SrcU[OffsetUV];
				unsigned char	V			= SrcV[OffsetUV];

				//@todo optimization: this is a prime candidate for SSE/ Altivec or fixed point integer or moving it to the GPU though we really ought to just avoid this codepath altogether.
				DestColor->R				= Clamp( appTrunc( Y0 + 1.402f * (V-128) ), 0, 255 );
				DestColor->G				= Clamp( appTrunc( Y0 - 0.34414f * (U-128) - 0.71414f * (V-128) ), 0, 255 );
				DestColor->B				= Clamp( appTrunc( Y0 + 1.772 * (U-128) ), 0, 255 );
				DestColor->A				= 255;
				DestColor++;

				//@todo optimization: this is a prime candidate for SSE/ Altivec or fixed point integer or moving it to the GPU though we really ought to just avoid this codepath altogether.
				DestColor->R				= Clamp( appTrunc( Y1 + 1.402f * (V-128) ), 0, 255 );
				DestColor->G				= Clamp( appTrunc( Y1 - 0.34414f * (U-128) - 0.71414f * (V-128) ), 0, 255 );
				DestColor->B				= Clamp( appTrunc( Y1 + 1.772 * (U-128) ), 0, 255 );
				DestColor->A				= 255;
				DestColor++;
			}
		}

	}

	RHIUnlockTexture2D( Texture2DRHI, 0 );							
}

