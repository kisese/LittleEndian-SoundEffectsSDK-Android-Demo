////////////////////////////////////////////////////////////////////////////////
///
/// exampleBasic.cpp
/// ----------------
///
/// LE example app contents (not to be confused with the official SDK API).
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "exampleBasic.hpp"

#include <le/audioio/device.hpp>
#include <le/audioio/file.hpp>
#include <le/audioio/inputWaveFile.hpp>
#include <le/audioio/outputWaveFile.hpp>

#include <le/spectrumworx/engine/moduleProcessor.hpp>

#include <le/utility/filesystem.hpp> // Utility::SpecialLocations
#include <le/utility/trace.hpp>
#include <le/utility/profiler.hpp>

#include <cerrno>
#include <vector>
//------------------------------------------------------------------------------

typedef AudioIO::error_msg_t err_t;

////////////////////////////////////////////////////////////////////////////////
// processingExample_FileOffline()
////////////////////////////////////////////////////////////////////////////////

void processingExample_FileOffline( char const * inputAudioFile, char const * outputAudioFile, char const * presetFile )
{
    // Open the input file
    // (please consult the documentation for the LE::Utility::SpecialLocations
    // enumeration for an explanation of that cross-platform filesystem
    // example/hierarchy abstraction).
    AudioIO::File inputFile;
    if ( err_t err = inputFile.open<Utility::ToolResources>( inputAudioFile ) )
    {
        Utility::Tracer::error( "Failed to open main input file: %s (%s).", inputAudioFile, err );
        return;
    }

    // Create the output file.
    // Construct a full path to the output file so that it can be logged.
    AudioIO::OutputWaveFile outputFile;
    if ( err_t err = outputFile.create<Utility::ToolOutput>( outputAudioFile, inputFile.numberOfChannels(), inputFile.sampleRate() ) )
    {
        Utility::Tracer::error( "Failed to create output file: %s (%s).", Utility::fullPath<Utility::ToolOutput>( outputAudioFile ), err );
        return;
    }

    // Setup the processor for the current audio input and effects preset:
    SW::Engine::ModuleProcessor processor;
    processor.setAudioFormat( inputFile.numberOfChannels(), inputFile.sampleRate() ); // errchk
    processor.loadPreset<Utility::ToolResources>( presetFile ); // errchk

    ////////////////////////////////////////////////////////////////////////////
    // Process the data in blocks:
    ////////////////////////////////////////////////////////////////////////////

    Utility::DSPProfiler::singleton().setSignalSampleRate( inputFile.sampleRate() );

    unsigned int processingBlockSize = 4096;
    std::vector<float> buffer( processingBlockSize * inputFile.numberOfChannels() ); // errchk
    for ( ; ; )
    {
        Utility::DSPProfiler::singleton().beginInterval();

        // Inplace processing without sidechaining
        unsigned int readSampleFrames = inputFile.read( &buffer[ 0 ], processingBlockSize ); // errchk
        processor .process( &buffer[ 0 ], readSampleFrames );
        outputFile.write  ( &buffer[ 0 ], readSampleFrames ); // errchk

        Utility::DSPProfiler::singleton().endInterval( readSampleFrames );
        if ( readSampleFrames < processingBlockSize )
            break;
    }
}


////////////////////////////////////////////////////////////////////////////////
// ExampleBlockingFilePlayer
////////////////////////////////////////////////////////////////////////////////

void ExampleBlockingFilePlayer::play( char const * inputAudioFile )
{
    inputFile_.open<Utility::AbsolutePath>( inputAudioFile                                         ); // errchk
    device_   .setup                      ( inputFile_.numberOfChannels(), inputFile_.sampleRate() ); // errchk
    device_   .setCallback                ( this, &ExampleBlockingFilePlayer::callback             ); // errchk

    // As an example on how to easily play audio in simple tests and/or
    // console/single-threaded apps, here we use the AudioIO::BlockingDevice
    // wrapper class that performs all the required inter-thread synchronization
    // and provides a synchronous/blocking startAndWait() method.
    blockingDevice_.startAndWait( device_ );
}

void ExampleBlockingFilePlayer::callback( ExampleBlockingFilePlayer * pPlayer, AudioIO::Device::InterleavedOutput data )
{
    unsigned int readSampleFrames = pPlayer->inputFile_.read( data.pOutput, data.numberOfSampleFrames );
    if ( readSampleFrames < data.numberOfSampleFrames )
        pPlayer->blockingDevice_.stop();
}


////////////////////////////////////////////////////////////////////////////////
// ExampleFileRenderer
////////////////////////////////////////////////////////////////////////////////

void ExampleFileRenderer::setup( char const * inputAudioFile, char const * presetFile )
{
    file_     .open      <Utility::ToolResources>( inputAudioFile                               ); // errchk
    processor_.setAudioFormat                    ( file_.numberOfChannels(), file_.sampleRate() ); // errchk
    processor_.loadPreset<Utility::ToolResources>( presetFile                                   ); // errchk
    device_   .setup                             ( file_.numberOfChannels(), file_.sampleRate() ); // errchk
    device_   .setCallback                       ( this, &ExampleFileRenderer::callback         ); // errchk

    Utility::DSPProfiler::singleton().setSignalSampleRate( processor_.sampleRate() );
}

void ExampleFileRenderer::play() { device_.start(); }
void ExampleFileRenderer::stop() { device_.stop (); }

void ExampleFileRenderer::callback( ExampleFileRenderer * pPlayer, AudioIO::Device::InterleavedOutput data )
{
    Utility::DSPProfiler::singleton().beginInterval();

    unsigned int readSampleFrames = pPlayer->file_.read( data.pOutput, data.numberOfSampleFrames );
    pPlayer->processor_.process( data.pOutput, readSampleFrames );
    if ( readSampleFrames < data.numberOfSampleFrames )
    {
        pPlayer->device_.stop();
        exampleUICallback_processingStopped();
    }

    Utility::DSPProfiler::singleton().endInterval( data.numberOfSampleFrames );
}


////////////////////////////////////////////////////////////////////////////////
// ExampleLiveInputRenderer
////////////////////////////////////////////////////////////////////////////////

void ExampleLiveInputRenderer::setup( char const * presetFile )
{
    processor_.setAudioFormat                    ( 1, 44100   ); // errchk
    processor_.loadPreset<Utility::ToolResources>( presetFile ); // errchk

    device_.setup      ( processor_.numberOfChannels(), processor_.sampleRate() ); // errchk
    device_.setCallback( this, &ExampleLiveInputRenderer::callback              ); // errchk

    Utility::DSPProfiler::singleton().setSignalSampleRate( processor_.sampleRate() );
}

void ExampleLiveInputRenderer::play() { device_.start(); }
void ExampleLiveInputRenderer::stop() { device_.stop (); }

void ExampleLiveInputRenderer::callback( ExampleLiveInputRenderer * pRenderer, AudioIO::Device::InputOutput data )
{
    Utility::DSPProfiler::singleton().beginInterval();

    pRenderer->processor_.process( data.pInputOutput, data.numberOfSampleFrames );

    Utility::DSPProfiler::singleton().endInterval( data.numberOfSampleFrames );
}

//------------------------------------------------------------------------------
