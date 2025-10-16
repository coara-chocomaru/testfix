LOCAL_PATH := $(call my-dir)
APP_ALLOW_MISSING_DEPS=true
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
LOCAL_MODULE := libshim2
LOCAL_SRC_FILES := shim2.cpp
LOCAL_CPPFLAGS := -fPIC -std=gnu++11 -DANDROID -O2 -Wall -Wno-unused-parameter -Wno-missing-field-initializers
LOCAL_LDLIBS := -llog -ldl
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libshim3
LOCAL_SRC_FILES := shim3.cpp
LOCAL_CFLAGS := -fPIC -DANDROID -O2
LOCAL_CPPFLAGS := -fPIC -std=gnu++11 -DANDROID -O2 -Wall -Wno-unused-parameter -Wno-missing-field-initializers -std=c++11
LOCAL_LDLIBS := -llog -ldl -lc++
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
include $(BUILD_SHARED_LIBRARY)
