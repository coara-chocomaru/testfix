APP_ABI := armeabi-v7a arm64-v8a
APP_STL := c++_static
APP_CPPFLAGS := -fno-exceptions -fno-rtti -D_GLIBCXX_USE_CXX11_ABI=1
APP_PLATFORM := android-21

ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
APP_CPPFLAGS += -D__aarch64__ 
endif


ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
APP_CPPFLAGS += -mfpu=neon -mfloat-abi=softfp
APP_CPPFLAGS += -D__arm__
APP_ARM_NEON := true
endif
