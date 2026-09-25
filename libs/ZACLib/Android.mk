# ZACLib/Android.mk
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ZACLib
LOCAL_SRC_FILES := ZACLib.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_CPPFLAGS := -std=c++11 # or higher

include $(BUILD_STATIC_LIBRARY)