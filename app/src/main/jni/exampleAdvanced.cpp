////////////////////////////////////////////////////////////////////////////////
///
/// exampleAdvanced.cpp
/// -------------------
///
/// LE example app contents (not to be confused with the official SDK API).
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "exampleAdvanced.hpp"

#include <le/audioio/device.hpp>
#include <le/audioio/file.hpp>
#include <le/audioio/inputWaveFile.hpp>
#include <le/audioio/outputWaveFile.hpp>

#include <le/parameters/lfo.hpp>

#include <le/spectrumworx/engine/moduleChain.hpp>
#include <le/spectrumworx/engine/moduleFactory.hpp>
#include <le/spectrumworx/engine/moduleProcessor.hpp>

#include <le/spectrumworx/effects/blender.hpp>
#include <le/spectrumworx/effects/freqverb.hpp>
#include <le/spectrumworx/effects/pitchShifter.hpp>

#include <le/utility/filesystem.hpp>
#include <le/utility/profiler.hpp>
#include <le/utility/trace.hpp>

#include <cassert>
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Demonstration of advanced features:
// - lower level interface
// - side chaining
// - LFOs
// - asynchronous rendering to disk
// - effect parameter meta data
//
////////////////////////////////////////////////////////////////////////////////

using namespace LE;


////////////////////////////////////////////////////////////////////////////////
// setupRenderingObjects() (helper for processingExampleAdvanced_* functions)
////////////////////////////////////////////////////////////////////////////////

void setupRenderingObjects
(
    AudioIO::InputWaveFile       & sideChainAudioInputFile,
    AudioIO::OutputWaveFileAsync & asyncOutputFile
)
{
    auto const sideChainAudio ( "samples/background.wav"         );
    auto const outputAudioFile( "LE_example_output_advanced.wav" );

    Utility::Tracer::message
    (
        "Advanced realtime processing with a side chain signal (%s) + streaming result to \"%s\".",
        sideChainAudio,
        outputAudioFile
    );

    // If we were using the 'higher level' interface, here we would just simply
    // load the XML preset:
    {
        using namespace SW::Engine;

        ModuleProcessor processor;
        std::string sideChainPath;
        processor              .loadPreset<Utility::ToolResources>( "presets/more presets/SDK advanced.swp", sideChainPath                           ); // errchk
        sideChainAudioInputFile.open      <Utility::ToolResources>( ( "samples/" + sideChainPath ).c_str()                                           ); // errchk
        processor              .setAudioFormat                    ( sideChainAudioInputFile.numberOfChannels(), sideChainAudioInputFile.sampleRate() ); // errchk

        // ...we could now still 'go low level' and access the individual
        // effects and their parameters by using internal knowledge/assuming
        // certain things about the loaded preset (e.g. the effects used and
        // their ordering):
    #if 0
        ModuleChain & moduleChain( processor.moduleChain() );

        ModuleBase & pitchShifter( moduleChain[ 0 ] );
        ModuleBase & freqverb    ( moduleChain[ 1 ] );
        ModuleBase & blender     ( moduleChain[ 2 ] );

        using namespace SW::Effects;
        exampleUICallback_addParameterControl( pitchShifter, Module<PitchShifter>::parameterIndex<PitchShifter::SemiTones>(), "Pitch"  );
        exampleUICallback_addParameterControl( freqverb    , Module<Freqverb    >::parameterIndex<Freqverb    ::Time60dB >(), "Reverb" );
        exampleUICallback_addParameterControl( blender     , Module<Blender     >::parameterIndex<Blender     ::Amount   >(), "Mix"    );
    #endif
    }
    // ...however we are here to demonstrate the lower level interface and all
    // the power of the LE SW SoundEffects SDK so let's leave aside XML presets
    // and create the exact same effect purely through code:

    sideChainAudioInputFile.open<Utility::ToolResources>( sideChainAudio ); // errchk

    auto const numberOfChannels( sideChainAudioInputFile.numberOfChannels() );
    auto const sampleRate      ( sideChainAudioInputFile.sampleRate      () );

    asyncOutputFile.create<Utility::ToolOutput>( outputAudioFile, numberOfChannels, sampleRate ); // errchk

    ////////////////////////////////////////////////////////////////////////////
    // Create and setup SW SDK objects.
    ////////////////////////////////////////////////////////////////////////////

    // Setup the processor for the loaded audio file:
    using namespace SW::Engine ;
    using namespace SW::Effects;

    // Create and setup some modules just as they are in the preset file:
    auto const pPitchShifter( Module<PitchShifter>::create() ); // errchk
    auto const pFreqverb    ( Module<Freqverb    >::create() ); // errchk
    auto const pBlender     ( Module<Blender     >::create() ); // errchk

    // Setup module parameters:

    // Pitch Shifter (w/ 'classic' parameter accessors):
    pPitchShifter->parameters().setSemiTones( 3 );

    // Freqverb (w/ template parameter accessors):
    pFreqverb->set<Freqverb::Time60dB>( 2 );

    // Instead of setting "room size" to a fixed value:
    //pFreqverb->set<Freqverb::RoomSize>( -1.5 );
    // lets turn on its LFO (low frequency oscillator) as specified in the
    // preset:
    auto const roomSizeParameterIndex( pFreqverb->parameterIndex<Freqverb::RoomSize>() ); // get the parameter index
    Parameters::LFO & lfo( pFreqverb->lfo( roomSizeParameterIndex ) ); // get the LFO for the parameter
    lfo.setPeriodInMilliseconds( 4000 ); // set LFO oscillation period
    lfo.setLowerBound          ( 0.6f ); // start at 60% of the full range
	lfo.setUpperBound          ( 0.7f ); // end at 70% of the full range
    lfo.setEnabled             ( true ); // enable the LFO

    // Blender:
	pBlender->set<Blender::Amount>( 5 ); // mix with 5% of the side channel
#if 0
    // and optionally limit its effect to a frequency subrange:
    pBlender->baseParameters().setStartFrequency( 0.0f ); // the default and as such not necessary
    pBlender->baseParameters().setStopFrequency ( 0.5f );
    // or the equivalent (for a 44.1 kHz sampling rate):
    pBlender->setStartFrequencyInHz(     0, sampleRate );
    pBlender->setStopFrequencyInHz ( 11025, sampleRate );
#endif
    // ...end effect setting.

    // Chain the effects in the same order:
    auto & processor  ( ModuleProcessor::singleton() );
    auto & moduleChain( processor.moduleChain()      );

    // Since we are (re)using the global/singleton ModuleProcessor instance, we
    // have to perform the following two steps:
    moduleChain.clear(); // remove any already existing modules
    processor  .reset(); // flush any residual signal from a previous streaming

    processor.setEngineParameters( numberOfChannels, sampleRate, 2048, 4, Constants::Hann ); // errchk

    moduleChain.append( pPitchShifter ); // errchk
    moduleChain.append( pFreqverb     ); // errchk
    moduleChain.append( pBlender      ); // errchk

    AudioIO::Device::singleton().setup( numberOfChannels, sampleRate ); // errchk

    Utility::DSPProfiler::singleton().setSignalSampleRate( sampleRate );

    exampleUICallback_addParameterControl( *pPitchShifter, pPitchShifter->parameterIndex<PitchShifter::SemiTones>(), "Pitch"  );
    exampleUICallback_addParameterControl( *pFreqverb    , pFreqverb    ->parameterIndex<Freqverb    ::Time60dB >(), "Reverb" );
    exampleUICallback_addParameterControl( *pBlender     , pBlender     ->parameterIndex<Blender     ::Amount   >(), "Mix"    );
} // bool setupRenderingObjects( AudioIO::InputWaveFile &, AudioIO::OutputWaveFileAsync & )


////////////////////////////////////////////////////////////////////////////////
// processingExampleAdvanced_microphone()
////////////////////////////////////////////////////////////////////////////////

struct MicRenderer
{
    void operator()( AudioIO::Device::InterleavedInputOutput const data )
    {
        Utility::DSPProfiler::singleton().beginInterval();

        using SW::Engine::ModuleProcessor;

    #ifndef _MSC_VER
        float sideChainData[ data.numberOfSampleFrames ];
    #else // MSVC does not support VLAs so we have to use alloca
        float * sideChainData( (float *)_alloca( data.numberOfSampleFrames * sizeof( float ) ) );
    #endif // compiler
        sideChainAudioInputFile     .readLooped(                    sideChainData,                    data.numberOfSampleFrames ); // errchk
        ModuleProcessor::singleton().process   ( data.pInputOutput, sideChainData, data.pInputOutput, data.numberOfSampleFrames );
        asyncOutputFile             .write     (                                   data.pInputOutput, data.numberOfSampleFrames ); // errchk

        Utility::DSPProfiler::singleton().endInterval( data.numberOfSampleFrames );
    }

    AudioIO::InputWaveFile       sideChainAudioInputFile;
    AudioIO::OutputWaveFileAsync asyncOutputFile        ;

#ifdef _MSC_VER // Workarounds for Visual Studio 2013 and earlier which don't properly implement the C++11 standard
    MicRenderer() {}
    MicRenderer( MicRenderer && other ) : sideChainAudioInputFile( std::move( other.sideChainAudioInputFile ) ), asyncOutputFile( std::move( other.asyncOutputFile ) ) {}
#endif // workarounds for Visual Studio prior to 2015
}; // struct MicRenderer

void processingExampleAdvanced_microphone()
{
    MicRenderer renderer;

    setupRenderingObjects( renderer.sideChainAudioInputFile, renderer.asyncOutputFile );

    // Thanks to the moveability of AudioIO *File* objects, here we can simply
    // move the renderer object 'into' the Device instance and 'forget about it'.
    AudioIO::Device::singleton().setCallback( std::move( renderer ) ); // errchk
    AudioIO::Device::singleton().start();
} // processingExampleAdvanced_microphone


////////////////////////////////////////////////////////////////////////////////
// processingExampleAdvanced_file()
////////////////////////////////////////////////////////////////////////////////

struct FileRenderer : MicRenderer
{
    void operator()( AudioIO::Device::InterleavedOutput const data )
    {
        auto const readSampleFrames( inputAudioFile.read( data.pOutput, data.numberOfSampleFrames ) );
        // Let's reuse MicRenderer and simply feed it samples from the input
        // file (as 'microphone' data):
        AudioIO::Device::InterleavedInputOutput const inputOutputData =
        {
            data.pOutput,
            static_cast<std::uint16_t>( readSampleFrames )
        };
        MicRenderer::operator()( inputOutputData );
        if ( readSampleFrames < data.numberOfSampleFrames )
        {
            AudioIO::Device::singleton().stop();
            exampleUICallback_processingStopped();
        }
    }

    AudioIO::File inputAudioFile;

#ifdef _MSC_VER // workarounds for old Visual Studio bugs
    FileRenderer() {}
    FileRenderer( FileRenderer && other ) : MicRenderer( std::move( other ) ), inputAudioFile( std::move( other.inputAudioFile ) ) {}
#endif // workarounds for old Visual Studio bugs
}; // struct FileRenderer

void processingExampleAdvanced_file( char const * const inputAudioPath )
{
    FileRenderer renderer;

    setupRenderingObjects( renderer.sideChainAudioInputFile, renderer.asyncOutputFile );

    renderer.inputAudioFile.open<Utility::ToolResources>( inputAudioPath ); // errchk
    assert( AudioIO::equalFormats( renderer.inputAudioFile, renderer.sideChainAudioInputFile ) );

    AudioIO::Device::singleton().setCallback( std::move( renderer ) ); // errchk
    AudioIO::Device::singleton().start();
} // processingExampleAdvanced_file

//------------------------------------------------------------------------------
