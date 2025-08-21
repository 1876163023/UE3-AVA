//------------------------------------------------------------------------------
// An OpenAL based audio player.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2006 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#ifndef __APPLE__

#include "FxAudioPlayerOpenAL.h"
#include "FxConsole.h"

#ifdef __UNREAL__
/*	#include "Engine.h"
	#include "ALAudioDevice.h"*/
#endif

// Link in OpenAL but only if __UNREAL__ is not defined.
#ifndef __UNREAL__
	#pragma comment(lib, "OpenAL32.lib")
	#pragma message("Linking OpenAL...")
#endif

namespace OC3Ent
{

namespace Face
{

#define kCurrentFxAudioPlayerOpenALVersion 0

FX_IMPLEMENT_CLASS(FxAudioPlayerOpenAL,kCurrentFxAudioPlayerOpenALVersion,FxAudioPlayer);

FxAudioPlayerOpenAL::FxAudioPlayerOpenAL(){}
FxAudioPlayerOpenAL::~FxAudioPlayerOpenAL(){}
void FxAudioPlayerOpenAL::Initialize( wxWindow* ){}
const FxDigitalAudio* FxAudioPlayerOpenAL::GetSource( void ) const{return NULL;}
void FxAudioPlayerOpenAL::SetSource( FxDigitalAudio* ){}
void FxAudioPlayerOpenAL::GetPlayRange( FxReal& , FxReal& ) const{}
void FxAudioPlayerOpenAL::SetPlayRange( FxReal , FxReal ){}
FxReal FxAudioPlayerOpenAL::GetVolume( void ) const
{
	return _volume;
}

void FxAudioPlayerOpenAL::SetVolume( FxReal ){}
FxReal FxAudioPlayerOpenAL::GetPitch( void ) const
{
	return _pitch;
}

void FxAudioPlayerOpenAL::SetPitch( FxReal ){}
void FxAudioPlayerOpenAL::Play( FxBool ){}
void FxAudioPlayerOpenAL::Pause( void ){}
void FxAudioPlayerOpenAL::Stop( void ){}
FxBool FxAudioPlayerOpenAL::IsPlaying( void ) const{return false;}
FxBool FxAudioPlayerOpenAL::IsLooping( void ) const{return false;}
FxReal FxAudioPlayerOpenAL::GetPlayCursorPosition( void ) const{return 0;}
FxBool FxAudioPlayerOpenAL::SetPlayCursorPosition( FxReal ){return false;}
void FxAudioPlayerOpenAL::_clearOpenALBuffer( void ){}
void FxAudioPlayerOpenAL::_createOpenALBuffer( void ){}
} // namespace Face

} // namespace OC3Ent

#endif
