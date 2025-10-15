#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <android/log.h>
#include <signal.h>

#define LOG_TAG "gfxshim_loader"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static volatile int running = 1;

static void sigterm_handler(int sig) {
    running = 0;
}

int main(int argc, char** argv) {
    signal(SIGTERM, sigterm_handler);
    const char* paths[] = {
        "/system/lib64/libgfxshim.so",
        "/vendor/lib64/libgfxshim.so",
        "/system/lib/libgfxshim.so",
        "/vendor/lib/libgfxshim.so",
        NULL
    };

    for (const char** p = paths; *p; ++p) {
        void* h = dlopen(*p, RTLD_NOW | RTLD_GLOBAL);
        if (h) {
            ALOGI("gfxshim_loader: dlopen succeeded: %s", *p);
        } else {
            ALOGE("gfxshim_loader: dlopen failed: %s -> %s", *p, dlerror());
        }
    }

    ALOGI("gfxshim_loader: entering main loop (pid=%d)", getpid());
    while (running) {
        sleep(60);
    }
    ALOGI("gfxshim_loader: exiting");
    return 0;
}
