////////////////////////////////////////////////////////////////////////////////
///
/// exampleAdvanced.hpp
/// -------------------
///
/// LE example app contents (not to be confused with the official SDK API).
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef exampleAdvanced_hpp__0ED104D9_547C_4E13_9231_195D5A51DA74
#define exampleAdvanced_hpp__0ED104D9_547C_4E13_9231_195D5A51DA74
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/engine/moduleBase.hpp"

#include <cstdint>
//------------------------------------------------------------------------------

void processingExampleAdvanced_microphone(                             );
void processingExampleAdvanced_file      ( char const * inputAudioPath );


////////////////////////////////////////////////////////////////////////////////
//
// Callbacks used by the shared/cross-platform C++ part of the example to
// communicate with platform-specific UI code: implemented in the platform
// specific part/sources of the example app.
//
////////////////////////////////////////////////////////////////////////////////

void exampleUICallback_processingStopped();
void exampleUICallback_addParameterControl( LE::SW::Engine::ModuleBase & module, std::uint8_t parameterIndex, char const * userFriendlyName );

//------------------------------------------------------------------------------
#endif // exampleAdvanced_hpp
