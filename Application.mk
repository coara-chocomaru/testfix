APP_ABI := armeabi-v7a arm64-v8a
APP_PLATFORM := android-21
APP_STL := c++_static
APP_CPPFLAGS := -fPIC -std=gnu++17 -std=c++11 -D_GNU_SOURCE -fexceptions -frtti -O2
APP_LDFLAGS := -Wl,--exclude-libs,libgcc.a
ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
APP_CPPFLAGS += -D__aarch64__ 
endif


ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
APP_CPPFLAGS += -mfpu=neon -mfloat-abi=softfp
APP_CPPFLAGS += -D__arm__
APP_ARM_NEON := true
endif
