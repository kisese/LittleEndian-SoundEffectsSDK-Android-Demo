////////////////////////////////////////////////////////////////////////////////
///
/// exampleBasic.hpp
/// ----------------
///
/// LE example app contents (not to be confused with the official SDK API).
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef exampleBasic_hpp__CF35DB4E_632E_4F88_918D_525F3E43E5D2
#define exampleBasic_hpp__CF35DB4E_632E_4F88_918D_525F3E43E5D2
#pragma once
//------------------------------------------------------------------------------
#include <le/audioio/device.hpp>
#include <le/audioio/file.hpp>
#include <le/audioio/inputWaveFile.hpp>

#include <le/spectrumworx/engine/moduleProcessor.hpp>
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Demonstration of the basic, high level XML preset interface.
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Important note: This LE SoundEffects SDK example code uses the LE AudioIO SDK
// in order to provide an audible demonstration of the LE SoundEffects SDK as
// well as to demonstrate the possible ways of integration of the two LE SDKs
// (AudioIO and SoundEffects). The LE SW SoundEffects SDK license does not
// however cover the LE AudioIO SDK which must be purchased separately.
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Important note: for improved readability, most error handling has been
// omitted from this example code (such lines will be marked with "errchk"
// comments).
////////////////////////////////////////////////////////////////////////////////


using namespace LE;


////////////////////////////////////////////////////////////////////////////////
// The simplest example: render an input file with a preset into an output file.
////////////////////////////////////////////////////////////////////////////////

void processingExample_FileOffline( char const * inputAudioFile, char const * outputAudioFile, char const * presetFile );


////////////////////////////////////////////////////////////////////////////////
// ExampleBlockingFilePlayer
////////////////////////////////////////////////////////////////////////////////

class ExampleBlockingFilePlayer
{
public:
    void play( char const * inputAudioFile );

private:
    static void callback( ExampleBlockingFilePlayer * pPlayer, AudioIO::Device::InterleavedOutput data );

private:
    AudioIO::Device         device_        ;
    AudioIO::InputWaveFile  inputFile_     ;
    AudioIO::BlockingDevice blockingDevice_;
}; // class ExampleBlockingFilePlayer


////////////////////////////////////////////////////////////////////////////////
// ExampleFileRenderer
////////////////////////////////////////////////////////////////////////////////

class ExampleFileRenderer
{
public:
    void setup( char const * inputAudioFile, char const * presetFile );

    void play();
    void stop();

private:
    static void callback( ExampleFileRenderer * pPlayer, AudioIO::Device::InterleavedOutput data );

private:
    AudioIO::File               file_;
    AudioIO::Device             device_;
    SW::Engine::ModuleProcessor processor_;
}; // class ExampleFileRenderer


////////////////////////////////////////////////////////////////////////////////
// ExampleLiveInputRenderer
////////////////////////////////////////////////////////////////////////////////

class ExampleLiveInputRenderer
{
public:
    void setup( char const * presetFile );

    void play();
    void stop();

private:
    static void callback( ExampleLiveInputRenderer *, AudioIO::Device::InputOutput data );

private:
    AudioIO::Device             device_   ;
    SW::Engine::ModuleProcessor processor_;
}; // class ExampleLiveInputRenderer


////////////////////////////////////////////////////////////////////////////////
//
// Callbacks used by the shared/cross-platform C++ part of the example to
// communicate with platform-specific UI code: implemented in the platform
// specific part/sources of the example app.
//
////////////////////////////////////////////////////////////////////////////////

void exampleUICallback_processingStopped();

//------------------------------------------------------------------------------
#endif // exampleBasic_hpp
