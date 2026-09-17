LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ai_service
LOCAL_SRC_FILES := \
    src/main.cpp \
    src/common.cpp \
    src/logger.cpp \
    src/clean.cpp \
    src/suppress.cpp \
    src/service.cpp \
    src/action.cpp \
    src/boot.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src
LOCAL_CFLAGS := -O2 -Wall
LOCAL_CPPFLAGS := -std=c++17 -O2 -Wall
include $(BUILD_EXECUTABLE)
