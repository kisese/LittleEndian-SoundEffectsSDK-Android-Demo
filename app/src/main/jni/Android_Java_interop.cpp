////////////////////////////////////////////////////////////////////////////////
///
/// Android_Java_interop.cpp
/// ------------------------
///
/// LE example app contents (not to be confused with the official SDK API).
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "exampleBasic.hpp"
#include "exampleAdvanced.hpp"

#include <le/parameters/runtimeInformation.hpp>

#include <le/utility/jni.hpp>
#include <le/utility/profiler.hpp>
#include <le/utility/trace.hpp>

#include <cassert>
#include <cstdint>
//------------------------------------------------------------------------------

using namespace LE;
using Module = SW::Engine::ModuleBase;

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

Utility::JNI::GlobalRef<jobject> activity          ;
jmethodID                        processingStopped ;
jmethodID                        addParameterWidget;


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

ExampleFileRenderer      fileRenderer      ;
ExampleLiveInputRenderer microphoneRenderer;


////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

extern "C" JNIEXPORT
void JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_initialise( JNIEnv * const pEnv, jobject const javaActivity, jobject const javaAssetManager )
{
    using namespace LE;

    // Provide the bundle/APK/environment/context information to the SDK...
    Utility::setAppContext( *pEnv, javaActivity, javaAssetManager );

    // Setup the logging utility class...
    using Utility::Tracer;
    Tracer::pTagString = "LE SW SoundEffects SDK example";
    Tracer::setJavaCallback( *pEnv, javaActivity, "userMessage" ); // errchk

    jclass const activityClass( pEnv->GetObjectClass( javaActivity ) ); assert( activityClass );
    activity = Utility::JNI::globalReference( javaActivity ); // errchk
    processingStopped  = pEnv->GetMethodID( activityClass, "processingStopped", "()V" ); assert( processingStopped );
    addParameterWidget = pEnv->GetMethodID( activityClass, "addParameterWidget",
        "("
            "Ljava/lang/String;" // effect name
            "Ljava/lang/String;" // app-customised parameter name
            "Ljava/lang/String;" // SDK parameter name
            "Ljava/lang/String;" // parameter unit
            "J"                  // module 'handle'
            "I"                  // parameter index
            "I"                  // current value
        ")V"
    ); assert( addParameterWidget );
}


extern "C" JNIEXPORT
void JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_renderPresetWithFileRealtime( JNIEnv * const pEnv, jobject /*activity*/, jstring const javaPresetName )
{
    auto inputAudioFile = "samples/speech.m4a";
    auto presetName     = Utility::JNI::c_str( *pEnv, javaPresetName );

    fileRenderer.setup( inputAudioFile, presetName.get() );
    fileRenderer.play();
}


extern "C" JNIEXPORT
void JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_renderPresetWithLiveInput( JNIEnv * const pEnv, jobject /*activity*/, jstring const javaPresetName )
{
    auto presetName = Utility::JNI::c_str( *pEnv, javaPresetName );

    // We might use the ExampleLiveInputRenderer class from
    // exampleBasic.hpp, similarly to how ExampleFileRenderer was used
    // in renderPresetWithFileRealtime(), to process and playback microphone
    // input here. Luckily, writing a separate class for such a simple task is
    // unnecessary with LE's AudioIO and SoundEffects SDKs: we'll rather
    // demonstrate how to use AudioIO::Device's support for closures and
    // singleton interfaces of the central SW::Engine::ModuleProcessor
    // and AudioIO::Device classes to solve this problem in a
    // straightforward manner:
#if 0
    microphoneRenderer.setup( presetName.get() );
    microphoneRenderer.play();
#else
    using SW::Engine::ModuleProcessor;

    auto & processor( ModuleProcessor::singleton() );
    auto & device   ( AudioIO::Device::singleton() );

    processor.setAudioFormat                ( 1, 44100         ); // errchk
    processor.loadPreset<Utility::Resources>( presetName.get() ); // errchk
    processor.reset(); // flush any previous signal

    Utility::DSPProfiler::singleton().setSignalSampleRate( processor.sampleRate() );

    device.setup( processor.numberOfChannels(), processor.sampleRate() ); // errchk
    device.setCallback
    (
        []( AudioIO::Device::InputOutput data )
        {
            Utility::DSPProfiler::singleton().beginInterval();

            // So, how's this for a one-liner? ;-)
            ModuleProcessor::singleton().process( data.pInputOutput, data.numberOfSampleFrames );

            Utility::DSPProfiler::singleton().endInterval( data.numberOfSampleFrames );
        }
    ); // errchk
    device.start();
#endif
}


extern "C" JNIEXPORT
void JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_advancedLiveInputRender( JNIEnv *, jobject )
{
    processingExampleAdvanced_microphone();
}


extern "C" JNIEXPORT
void JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_advancedFileRender( JNIEnv *, jobject )
{
    processingExampleAdvanced_file( "samples/speech.m4a" );
}


extern "C" JNIEXPORT
void JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_stopAllRendering( JNIEnv *, jobject )
{
    fileRenderer                     .stop ();
    microphoneRenderer               .stop ();
    AudioIO::Device     ::singleton().stop ();
    Utility::DSPProfiler::singleton().reset();
}


extern "C" JNIEXPORT
jfloat JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_updateParameter
(
    JNIEnv  * const pEnv,
    jobject   const activity,
    jlong     const modulePtr,
    jint      const parameterIndex,
    jint      const newValuePercentage
)
{
    using Module = SW::Engine::ModuleBase;

    auto & module       ( *Utility::JNI::unmarshalPointer<Module>( modulePtr ) );
    auto & parameterInfo( module.parameterInfo( parameterIndex ) );
    float const newValue( parameterInfo.minimum + ( parameterInfo.maximum - parameterInfo.minimum ) * newValuePercentage / 100 );
    return module.setParameter( static_cast<std::uint8_t>( parameterIndex ), newValue );
}


extern "C" JNIEXPORT
jfloat JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_getCPUUsage( JNIEnv *, jobject )
{
    auto const result( Utility::DSPProfiler::singleton().cpuUsagePercentage() );
    // Reset the profiler so that we get a new value every time (instead of
    // a total average).
    Utility::DSPProfiler::singleton().reset();
    return result;
}


extern "C" JNIEXPORT
jboolean JNICALL Java_demo_littleendiandemo_app_LESoundEffectsExampleActivity_isReleaseBuild( JNIEnv *, jobject )
{
#ifdef NDEBUG
    return true;
#else
    return false;
#endif // NDEBUG
}


////////////////////////////////////////////////////////////////////////////////
// C++ -> UI callback implementations
////////////////////////////////////////////////////////////////////////////////

void exampleUICallback_processingStopped()
{
    Utility::JNI::env()->CallVoidMethod( activity.get(), processingStopped );
    Utility::DSPProfiler::singleton().reset();
}


void exampleUICallback_addParameterControl( Module & module, std::uint8_t parameterIndex, char const * userFriendlyName )
{
    Module::ParameterInfo const & parameterInfo( module.parameterInfo( parameterIndex ) );

    float const currentValue( module.getParameter( parameterIndex ) );
    float const currentValuePercentage
    (
        ( currentValue          - parameterInfo.minimum )
            /
        ( parameterInfo.maximum - parameterInfo.minimum )
            *
        100
    );

    Utility::JNI::EnvPtr const pJNI( Utility::JNI::env() ); // errchk
    pJNI->CallVoidMethod
    (
        activity.get(),
        addParameterWidget,
        pJNI->NewStringUTF( module.effectName() ), // errchk
        pJNI->NewStringUTF( userFriendlyName    ), // errchk
        pJNI->NewStringUTF( parameterInfo.name  ), // errchk
        pJNI->NewStringUTF( parameterInfo.unit  ), // errchk
        Utility::JNI::marshalPointer( &module ),
        parameterIndex,
        static_cast<int>( currentValuePercentage )
    );
}

//------------------------------------------------------------------------------
