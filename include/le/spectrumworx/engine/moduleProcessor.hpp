////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleProcessor.hpp
/// -------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleProcessor_hpp__FE5806D3_6E72_428F_A861_E6282A232943
#define moduleProcessor_hpp__FE5806D3_6E72_428F_A861_E6282A232943
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "configuration.hpp"

#include "le/utility/abi.hpp"
#ifndef _MSC_VER
#include "le/utility/filesystem.hpp"
#endif // _MSC_VER
#include "le/utility/pimpl.hpp"

#include "boost/noncopyable.hpp"
#include "boost/smart_ptr/intrusive_ptr.hpp"

#include <cstdint>
#include <memory>
#include <string>

#include "boost/config/abi_prefix.hpp"
//------------------------------------------------------------------------------
#if defined( _MSC_VER ) && !defined( LE_SDK_NO_AUTO_LINK )
    #ifdef _WIN64
        #pragma comment( lib, "LE_SoundEffects_SDK_Win64_x86-64_SSE3.lib" )
    #else // _WIN32
        #pragma comment( lib, "LE_SoundEffects_SDK_Win32_x86-32_SSE2.lib" )
    #endif // _WIN32/64
#endif // _MSC_VER && !LE_SDK_NO_AUTO_LINK
//------------------------------------------------------------------------------
namespace LE /// \brief Umbrella namespace for all Little Endian provided functionality
{
//------------------------------------------------------------------------------
#ifdef _MSC_VER
namespace Utility { enum SpecialLocations; }
#endif // _MSC_VER
//------------------------------------------------------------------------------
namespace SW /// \brief Root namespace for all things SpectrumWorx
{
//------------------------------------------------------------------------------
/// \addtogroup Engine
/// @{

/// \brief SpectrumWorx core components
namespace Engine
{
//------------------------------------------------------------------------------

/// \addtogroup Engine
/// @{

/***************************************************************************//**

 \class ModuleProcessor

 \brief The core of the spectral DSP engine. Provides signal processing with an
        unlimited number of modules.

  - processes and transforms input time domain data according to set
    parameters
  - passes the prepared data through attached module(s)
  - post-processes and transforms the resulting data back to the time domain
    as the output data.

 Each ModuleProcessor instance as well as any attached modules maintain
 internal state corresponding to the current signal and its history. For this
 reason it is necessary to call the reset() member function on the concrete
 ModuleProcessor instance after processing a stream of data is finished and
 before starting the processing of a new stream of data.

 Please consult the documentation for the moduleChain() member function, the
 ModuleChain class and the example code for more information on how to chain/
 attach modules to a ModuleProcessor instance.

 \note
  - each ModuleProcessor instance is self contained: any number can be
    created and used from different threads
  - the contents of the module chain may freely be (concurrently) altered
    during a call to the process() member function(s)
  - allowed values of configuration parameters:
    - gain is a plain linear parameter and can be any positive value (default
      is 1)
    - wetness percentage can be any value in the [0, 100] range (default is
      100)
  - please consult the values in \link Engine::Constants \endlink for valid
    ranges of engine parameters
  - to avoid a redundant engine initialization the respective parameter
    defaults are not automatically set on creation and it is therefore
    required that you call the setEngineParameters() member function before
    using a ModuleProcessor instance even if you are satisfied with the
    engine parameters' defaults. In case the
    ModuleProcessor::loadPreset() member function is used, a call to
    setEngineParameters() is not necessary (as the engine parameters will be
    loaded from the preset) rather only the audio signal parameters need to be
    specified (samplerate and the number of channels).

  \nosubgrouping

*******************************************************************************/

class ModuleChain;

class ModuleProcessor
    :
#ifndef DOXYGEN_ONLY
    public Utility::StackPImpl<ModuleProcessor, 15 * sizeof( int ) + 20 * sizeof( void * )>
#endif // DOXYGEN_ONLY
{
public:
    /// \name Processing
    /// @{

    typedef float const * LE_RESTRICT const * LE_RESTRICT InputData            ; ///< A pointer to an array of pointers to read-only input channel data
    typedef float       * LE_RESTRICT const * LE_RESTRICT OutputData           ; ///< A pointer to an array of pointers to output channel data
    typedef float const * LE_RESTRICT                     InterleavedInputData ; ///< A pointer to an array of interleaved read-only input channel data
    typedef float       * LE_RESTRICT                     InterleavedOutputData; ///< A pointer to an array of interleaved output channel data

    /// @}

    typedef Constants::Window Window;

public:
    /// \name Audio format setup
    /// @{

    /// Sets the number of channels.
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI setNumberOfChannels( std::uint8_t );

    /// Sets the sampling rate.
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI setSampleRate( std::uint32_t );

    /// Sets the number of channels and sampling rate in a single call.
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI setAudioFormat( std::uint8_t numberOfChannels, std::uint32_t sampleRate );

    /// @}

    /// \name Mixer setup
    /// @{

    /// \brief Returns the gain applied to the output.
    /// Default value 1.
    LE_NOTHROWNOALIAS float LE_FASTCALL_ABI gain             () const;

    /// \brief Returns the gain applied to the output in decibels.
    /// Default value 0 dB.
    LE_NOTHROWNOALIAS float LE_FASTCALL_ABI gaindB           () const;

    /// \brief Returns the wetness amount, mix between original and transformed
    /// signal.
    /// Supported range is 0 to 1. Default value is 1.
    LE_NOTHROWNOALIAS float LE_FASTCALL_ABI wetness          () const;

    /// \brief Returns the wetness percentage, mix between the original and
    /// the transformed signal.
    /// Supported range is 0% to 100%. Default value is 100% wet.
    LE_NOTHROWNOALIAS float LE_FASTCALL_ABI wetnessPercentage() const;

    /// Sets the gain to be applied to output.
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setGain             ( float );
    /// Sets the gain to be applied to output in decibels.
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setGaindB           ( float );

    /// Sets the wetness.
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setWetness          ( float );
    /// Sets the wetness in percentages.
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setWetnessPercentage( float );

    /// @}

    /// \name LFO positioning and timing (ADVANCED)
    /// @see Parameters::LFO
    /// @{

    /// \details Automatically updated by the process() function
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setPosition( std::uint32_t absolutePositionInSamples );
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setPosition( float         absolutePositionInSeconds ); /// \overload
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI resetPosition();

    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setBPM  ( float BPM                                                   ); ///< 120 BPM default
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setMeter(            std::uint8_t numerator, std::uint8_t denominator ); ///< 4/4 default
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setTempo( float BPM, std::uint8_t numerator, std::uint8_t denominator );

    /// @}

    /// \name Engine setup (ADVANCED)
    /// @{

    /// \brief The size of input data blocks that are sent to the Fourier
    /// transform.
    /// Shorter values provide lower latency and higher time resolution (faster
    /// changes in the input signal can be captured) while larger values provide
    /// a more accurate frequency analysis at the expense of higher latency and
    /// lower time resolution.
    /// <BR>Supported values are: 128, 256, 512, 1024, 2048, 4096, 8192.
    /// <BR>Default value: 2048.
    LE_NOTHROWNOALIAS std::uint16_t LE_FASTCALL_ABI fftSize                () const;

    /// \brief The amount of overlap between consecutive frames of audio data
    /// sent to the Fourier analysis pass. This helps offset the reduced time
    /// resolution with larger frame/FFT sizes and is vital (i.e. the more the
    /// better) for phase vocoder based effects (such as the pitch shifter).
    /// Please note that increasing the window overlapping factor increases the
    /// CPU time consumption accordingly.
    /// <BR>Supported values are: 1 (no overlap), 2, 4, and 8.
    /// <BR>Default value: 4.
    LE_NOTHROWNOALIAS std::uint8_t  LE_FASTCALL_ABI windowOverlappingFactor() const;

    /// \brief The amount of time advance from frame to frame (effectively
    /// stepSize = fftSize / windowOverlappingFactor). In other words, it is the
    /// time quantum with which the current engine setup operates: dynamic
    /// parameter changes (whether to the ModuleProcessor or the individual
    /// effects) with finer granularity than the value returned by stepSize()
    /// will have no effect (i.e. only the parameter values at the beginning of
    /// each step will be captured/take effect).
    LE_NOTHROWNOALIAS std::uint16_t LE_FASTCALL_ABI stepSize               () const;

    /// \brief Returns the type of "window" used to shape the spectral leakage
    /// and frequency resolution.
    /// <BR>Supported values are: Hann, Hamming, Blackman, Blackman Harris,
    /// Gaussian, Flat top, Welch, Triangle and Rectangle.
    /// <BR>Default value: Hann.
    LE_NOTHROWNOALIAS Window        LE_FASTCALL_ABI windowFunction         () const;

    /// \brief Returns the sampling rate of input signal.
    /// Usable values are limited only by available memory.
    LE_NOTHROWNOALIAS std::uint32_t LE_FASTCALL_ABI sampleRate             () const;

    /// \brief Returns the number of channels.
    /// Usable values are limited only by available memory.
    LE_NOTHROWNOALIAS std::uint8_t  LE_FASTCALL_ABI numberOfChannels       () const;

    /// \brief Returns the "ripple" value.
    /// Tells if the current engine settings
    /// will result in a so-called "perfect reconstruction" of the sound - or if
    /// some ripple will be introduced by the engine regardless of the effects
    /// being used. Successive windowed frames should overlap in time in such a
    /// way that all the data is weighted equally - this produces no ripple.
    LE_NOTHROWNOALIAS float          LE_FASTCALL_ABI ripplePercentage      () const;

    /// \brief Returns the latency of the current engine setup in samples
    /// (currently equivalent to the FFT size).
    LE_NOTHROWNOALIAS std::uint16_t  LE_FASTCALL_ABI latencyInSamples      () const;

    /// \brief Returns the engine latency in milliseconds.
    LE_NOTHROWNOALIAS std::uint16_t  LE_FASTCALL_ABI latencyInMilliseconds () const;


    /// Sets the analysis frame size.
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI setFFTSize          ( std::uint16_t );

    /// Sets the overlapping factor.
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI setOverlappingFactor( std::uint8_t  );

    /// Sets the window function.
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI setWindowFunction   ( Window        );

    ////////////////////////////////////////////////////////////////////////////
    //
    // ModuleProcessor::setWOLAParameters()
    // ------------------------------------
    //
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Sets the WOLA engine parameters.
    ///
    /// \param fftSize       - frame size
    /// \param overlapFactor - overlapping factor
    /// \param window        - window type
    ///
    /// Changes to Windowed-Overlap-And-Add parameters (frame size, overlap
    /// factor and window type) are potentially long operations. If possible, it
    /// is more efficient to change them all at once with a single call to
    /// setWOLAParameters() than doing it separately with the dedicated setters.
    /// Note that for the FFT size and overlap factor only power-of-two values
    /// are accepted as valid input.
    ///
    ////////////////////////////////////////////////////////////////////////////

    LE_NOTHROWNOALIAS
    bool LE_FASTCALL_ABI setWOLAParameters
    (
        std::uint16_t fftSize       = Constants::defaultFFTSize,
        std::uint8_t  overlapFactor = Constants::defaultOverlapFactor,
        Window        window        = Constants::defaultWindow
    );

    ////////////////////////////////////////////////////////////////////////////
    //
    // ModuleProcessor::setEngineParameters()
    // --------------------------------------
    //
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Sets all of the engine parameters.
    ///
    /// Allows changing all of the parameters that require memory allocation
    /// and/or non-trivial interdependent calculations in a single call.
    /// <BR>
    /// For more information on the individual parameters please consult the
    /// documentation for the respective individual getter functions.
    ///
    ////////////////////////////////////////////////////////////////////////////

    LE_NOTHROWNOALIAS
    bool LE_FASTCALL_ABI setEngineParameters
    (
        std::uint8_t  numberOfChannels = 1,
        std::uint32_t sampleRate       = Constants::defaultSampleRate,
        std::uint16_t fftSize          = Constants::defaultFFTSize,
        std::uint8_t  overlapFactor    = Constants::defaultOverlapFactor,
        Window        window           = Constants::defaultWindow
    );

    /// @}

public:
    /// \name Module chain access
    /// @{
    LE_NOTHROWNOALIAS ModuleChain       & LE_FASTCALL_ABI moduleChain()      ;
    LE_NOTHROWNOALIAS ModuleChain const & LE_FASTCALL_ABI moduleChain() const;
    /// @}

public:
    /// \name Processing
    /// @{

    ////////////////////////////////////////////////////////////////////////////
    //
    // ModuleProcessor::process()
    // --------------------------
    //
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Processes the input samples as separated channels.
    ///
    /// \param mainInputs   - pointer to an array of float const pointers, the
    ///                       input data of individual channels (each of which
    ///                       must have at least numberOfChannels() elements)
    /// \param sideInputs   - pointer to an array of float const pointers, the
    ///                       input data of individual side channels (each of
    ///                       which must have at least numberOfChannels()
    ///                       elements), can be null
    /// \param outputs      - pointer to an array of float pointers, the output
    ///                       data for individual channels (each of which must
    ///                       have at least numberOfChannels() elements)
    /// \param sampleFrames - number of sample frames to process from
    ///                       <VAR>mainChannels</VAR> and <VAR>sideInputs</VAR>
    ///                       (if not null) and to save to outputs (there is no
    ///                       limit on the number of samples processed in one
    ///                       call).
    ///
    /// \note
    ///  - in-place operation is fully supported (i.e. <VAR>mainInputs</VAR> and
    ///    <VAR>outputs</VAR> may point to the same buffers)
    ///  - the optional <VAR>sideInputs</VAR> parameter is used for providing a
    ///    <a href="http://en.wikipedia.org/wiki/Dynamic_range_compression#Side-chaining">
    ///    side-chain signal</a> required by some effects (whose static member
    ///    constant usesSideChannel equals to true), please consult the
    ///    particular effect's documentation for information on how or what for
    ///    it uses the side-chain signal
    ///  - the process() member function is not reentrant and not thread safe
    ///    with respect to any member function that changes one of the engine
    ///    parameters (WOLA, number of channels and/or the sample rate)
    ///  - there is no limit on the amount of data that can be processed in a
    ///    single process() call
    ///
    ////////////////////////////////////////////////////////////////////////////
    LE_NOTHROWNOALIAS
    void LE_FASTCALL_ABI process
    (
        InputData     mainInputs,
        InputData     sideInputs,
        OutputData    outputs,
        std::uint32_t sampleFrames
    );

    /// \overload
    /// <BR>Simplified overload for in-place processing without a side chain.
    LE_NOTHROWNOALIAS
    void LE_FASTCALL_ABI process
    (
        OutputData    inputsAndOutputs,
        std::uint32_t sampleFrames
    );

    ////////////////////////////////////////////////////////////////////////////
    //
    // ModuleProcessor::process()
    // --------------------------
    //
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Processes the input samples as interleaved channels.
    ///
    /// \note This overload has additional overhead compared to the separated
    /// channels version so it is more efficient to use the separated version if
    /// possible.
    ///
    /// \param mainInputs   - pointer to an input array of interleaved channels
    ///                       (must have at least numberOfChannels() *
    ///                       <VAR>sampleFrames</VAR> elements)
    /// \param sideInputs   - pointer to an input array of interleaved channels
    ///                       (must have at least numberOfChannels() *
    ///                       <VAR>sampleFrames</VAR> elements), can be null
    /// \param outputs      - pointer to an output array of interleaved channels
    ///                        (must have at least numberOfChannels()
    ///                       <VAR>sampleFrames</VAR> elements)
    /// \param sampleFrames - number of sample frames to process from
    ///                       <VAR>mainChannels</VAR> and <VAR>sideInputs</VAR>
    ///                       (if not null) and to save to outputs (there is no
    ///                       limit on the number of samples processed in one
    ///                       call).
    ///
    ////////////////////////////////////////////////////////////////////////////
    LE_NOTHROWNOALIAS
    void LE_FASTCALL_ABI process
    (
        InterleavedInputData  mainInputs,
        InterleavedInputData  sideInputs,
        InterleavedOutputData outputs,
        std::uint32_t         sampleFrames
    );

    /// \overload
    /// <BR>Simplified overload for in-place processing without a side chain.
    LE_NOTHROWNOALIAS
    void LE_FASTCALL_ABI process
    (
        InterleavedOutputData inputsAndOutputs,
        std::uint32_t         sampleFrames
    );

    ////////////////////////////////////////////////////////////////////////////
    //
    // ModuleProcessor::reset()
    // ------------------------
    //
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Resets the processor and the attached modules.
    ///
    ////////////////////////////////////////////////////////////////////////////
    LE_NOTHROWNOALIAS
    void LE_FASTCALL_ABI reset();

    /// @}

public:
    /// \name XML presets
    /// @{

    typedef std::unique_ptr<char const[]> SamplePath;

    /// \brief Utility function to ease sound design/prototyping with the SW
    /// plugin/"GUI tool".
    ///
    /// \details
    /// - any number of presets can be loaded during processing (minding
    /// threading issues of course)
    /// - it is perfectly safe to use both XML presets (this function) and the
    /// rest of the ModuleProcessor API, one does not exclude the other
    /// (e.g. after loading a preset it may be further customised in code by
    /// adding or removing effects, adjusting the gain or any other parameter).
    ///
    /// \param  presetFile      Path to the preset file that is to be loaded
    ///                         relative to <VAR>rootLocation</VAR>.
    /// \param  pExternalSample Pointer to a smart pointer that is to be reset
    ///                         to the path of an external (side chain) sample
    ///                         file specified by the preset, if any (optional).
    ///
    /// \tparam rootLocation The filesystem location at which to look for
    ///         <VAR>presetFile</VAR>. @see Utility::SpecialLocations
    /// \return true if the preset was parsed successfully or false otherwise
    ///         (not enough memory or a corrupt preset file).
    ///
    /// <B>Effect:</B>
    ///     Loads the preset file pointed to by <VAR>presetFile</VAR>
    ///     within/relative to <VAR>rootLocation</VAR>.<BR>
    /// <B>Postconditions:</B>
    ///     Unless an error is reported:
    ///     - effects specified in the preset were loaded into the processor's
    ///       module chain (replacing any effects previously existing in the
    ///       chain)<BR>
    ///     - the processor's engine parameters and each module's parameters
    ///       were setup as specified in the preset<BR>
    ///     - if the preset specifies an external sample file and
    ///       pExternalSample != nullptr, *pExternalSample will contain:
    ///       - only the sample's file name if the preset contains an absolute
    ///         path
    ///       - the full path specified in the preset if it is a relative
    ///         path<BR>
    ///       (this distinction is made in order to simplify loading presets
    ///       on mobile devices that were created using the SW plugin/"GUI tool"
    ///       on desktop systems).<BR>
    ///     - if the preset does not specify an external sample file and
    ///       pExternalSample != nullptr, *pExternalSample will be reset to
    ///       null.<BR>
    ///
    /// \note The SW SDK does not support separate input and output gain
    /// parameters (as does the SW plugin) so those two will be merged into the
    /// single gain parameter present in the SDK.
    /// .
    /// \note If you are concerned about efficiency please note that, in
    /// addition to the obvious XML deserialization overhead, this function
    /// essentially performs runtime effect selection which in turn means that
    /// using it will cause code for all provided effects to be included in the
    /// final binary.

    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW bool LE_FASTCALL_ABI loadPreset( char const * presetFile, SamplePath * pExternalSample = nullptr );

    template <Utility::SpecialLocations rootLocation>
    bool LE_FASTCALL_ABI loadPreset( char const * presetFile, std::string & externalSamplePath ) /// \overload
    {
        SamplePath pExternalSample;
        if ( !loadPreset<rootLocation>( presetFile, &pExternalSample ) )
            return false;
        externalSamplePath = pExternalSample.get();
        pExternalSample.release();
        return true;
    }

    /// @}

public:
    /// \name Construction
    /// @{

    LE_NOTHROWNOALIAS  ModuleProcessor(); ///< Constructor (ModuleProcessors can be created on the stack)
    LE_NOTHROWNOALIAS ~ModuleProcessor();

    typedef boost::intrusive_ptr<ModuleProcessor      >  Ptr; ///< Shared pointer to a mutable ModuleProcessor instance
    typedef boost::intrusive_ptr<ModuleProcessor const> CPtr; ///< Shared pointer to a const ModuleProcessor instance

    /// \brief Factory function
    /// \details Convenience function to make this class consistent with other
    /// SoundEffects SDK objects that can only be created on the heap.
    static LE_NOTHROWNOALIAS Ptr LE_FASTCALL_ABI create();

    /// \brief Convenience global instance for apps that use only a single
    /// ModuleProcessor and would otherwise use a static variable.
    static ModuleProcessor & singleton() { return singleton_; }

    /// @}

private:
#if defined( __ANDROID__ ) && !defined( DOXYGEN_ONLY )
    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW bool LE_FASTCALL_ABI loadPresetAux( char const * presetFile, void * pExternalSample );
#endif // __ANDROID__ && !DOXYGEN_ONLY

    friend void LE_NOTHROWNOALIAS LE_FASTCALL_ABI intrusive_ptr_add_ref( ModuleProcessor const * );
    friend void LE_NOTHROW        LE_FASTCALL_ABI intrusive_ptr_release( ModuleProcessor const * );

#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
private:
    ModuleProcessor( ModuleProcessor const  & );
    ModuleProcessor( ModuleProcessor       && );
#else
    ModuleProcessor( ModuleProcessor const  & ) = delete;
    ModuleProcessor( ModuleProcessor       && ) = delete;
#endif // _MSC_VER

private:
    static ModuleProcessor singleton_;
}; // class ModuleProcessor

typedef ModuleProcessor::Ptr ModuleProcessorPtr;

/** @} */ // group Engine

#if defined( __ANDROID__ ) && !defined( _LIBCPP_VERSION ) && !defined( DOXYGEN_ONLY )
static_assert( sizeof( ModuleProcessor::SamplePath ) == sizeof( void * ), "" );

template <Utility::SpecialLocations rootLocation>
LE_NOTHROW bool ModuleProcessor::loadPreset( char const * const presetFile, SamplePath * const pExternalSample )
{
    return loadPresetAux<rootLocation>( presetFile, static_cast<void *>( pExternalSample ) );
}
#endif // __ANDROID__ && !_LIBCPP_VERSION && !DOXYGEN_ONLY

//------------------------------------------------------------------------------
} /// @} // namespace Engine
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "boost/config/abi_suffix.hpp"

#endif // moduleProcessor_hpp
