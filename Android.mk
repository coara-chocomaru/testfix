LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libgfxshim
LOCAL_SRC_FILES := shim.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS := -llog -ldl
LOCAL_CPPFLAGS := -fPIC -std=c++11 -D_GNU_SOURCE -pthread
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libgfxshimv2
LOCAL_SRC_FILES := libgfxshim_advanced.cpp
LOCAL_CPP_EXTENSION := .cpp
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_CFLAGS := -fPIC -D_GNU_SOURCE
LOCAL_CPPFLAGS += -fPIC -std=c++11 -D_GNU_SOURCE -Wall -Wno-write-strings
LOCAL_LDLIBS := -llog -ldl
LOCAL_ALLOW_MULTIPLE_DEFINES := true
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libui_shim
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := test.c
LOCAL_SHARED_LIBRARIES := libui liblog libbase libc++
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_CFLAGS := -O3 -Wno-unused-variable -Wno-unused-parameter
LOCAL_PROPRIETARY_MODULE := true
LOCAL_32_BIT_ONLY := false
include $(BUILD_SHARED_LIBRARY)
