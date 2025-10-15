LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libgfxshim
LOCAL_SRC_FILES := shim.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS := -llog -ldl
LOCAL_CPPFLAGS := -fPIC -std=c++11 -D_GNU_SOURCE -pthread
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libgfxshim
LOCAL_SRC_FILES := libgfxshim.cpp
# build as C++ shared lib
LOCAL_CPPFLAGS := -D_GNU_SOURCE -fPIC -std=gnu++11
LOCAL_LDLIBS := -ldl -llog -lpthread
LOCAL_MODULE_TAGS := optional
# Build for both 32/64
LOCAL_MULTILIB := both
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := gfxshim_loader
LOCAL_SRC_FILES := gfxshim_loader.c
LOCAL_CFLAGS := -D_GNU_SOURCE
LOCAL_LDLIBS := -ldl -llog -lpthread
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
include $(BUILD_EXECUTABLE)


