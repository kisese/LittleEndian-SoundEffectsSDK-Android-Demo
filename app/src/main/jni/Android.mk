################################################################################
#
# LittleEndian SW SoundEffects Example App Android make file
#
# Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
#
################################################################################

MY_LOCAL_PATH := $(call my-dir)

ifndef LE_SDK_PATH
    LE_SDK_PATH := $(call my-dir)/../../../..
endif

include $(MY_LOCAL_PATH)/le_soundeffects_sdk.mk
include $(MY_LOCAL_PATH)/le_audioio_sdk.mk
include $(MY_LOCAL_PATH)/le_utility.mk


################################################################################
# Define the example app module:
################################################################################

LOCAL_PATH := ${MY_LOCAL_PATH}
include $(CLEAR_VARS)

LOCAL_MODULE           := app
LOCAL_SRC_FILES        := Android_Java_interop.cpp exampleBasic.cpp exampleAdvanced.cpp
LOCAL_C_INCLUDES       += $(LE_SDK_PATH)/include
LOCAL_CFLAGS           += -std=c++14 -fno-rtti -Wall -Wno-non-template-friend -Wno-unused-local-typedefs -Wno-unknown-warning-option -Wno-multichar
LOCAL_LDLAGS           += --gc-sections --icf=all
LOCAL_STATIC_LIBRARIES := le_soundeffects_sdk le_audioio_sdk le_utility

include $(BUILD_SHARED_LIBRARY)
