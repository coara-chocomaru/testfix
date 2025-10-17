LOCAL_PATH := $(call my-dir)

APP_ALLOW_MISSING_DEPS=true
include $(CLEAR_VARS)
LOCAL_MODULE := shim_gui_ui
LOCAL_SRC_FILES := sm.cpp
LOCAL_CPPFLAGS := -std=gnu++11 -fno-exceptions -fno-rtti -D_GLIBCXX_USE_CXX11_ABI=1 -DANDROID
LOCAL_CFLAGS := -fPIC
LOCAL_LDLIBS := -ldl
include $(BUILD_SHARED_LIBRARY)
