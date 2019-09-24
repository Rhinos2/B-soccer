#include "Log.h"
#include <stdarg.h>
#include <android/log.h>

void Log::info(const char* pMessage, ...) {
    va_list varArgs;
    va_start(varArgs, pMessage);
    __android_log_vprint(ANDROID_LOG_INFO, "SIMSOCCER", pMessage,
        varArgs);
    __android_log_print(ANDROID_LOG_INFO, "SIMSOCCER", "\n");
    va_end(varArgs);
}

void Log::error(const char* pMessage, ...) {
    va_list varArgs;
    va_start(varArgs, pMessage);
    __android_log_vprint(ANDROID_LOG_ERROR, "SIMSOCCER", pMessage,
        varArgs);
    __android_log_print(ANDROID_LOG_ERROR, "SIMSOCCER", "\n");
    va_end(varArgs);
}

void Log::warn(const char* pMessage, ...) {
    va_list varArgs;
    va_start(varArgs, pMessage);
    __android_log_vprint(ANDROID_LOG_WARN, "SIMSOCCER", pMessage,
        varArgs);
    __android_log_print(ANDROID_LOG_WARN, "SIMSOCCER", "\n");
    va_end(varArgs);
}

void Log::debug(const char* pMessage, ...) {
    va_list varArgs;
    va_start(varArgs, pMessage);
    __android_log_vprint(ANDROID_LOG_DEBUG, "SIMSOCCER", pMessage,
        varArgs);
    __android_log_print(ANDROID_LOG_DEBUG, "SIMSOCCER", "\n");
    va_end(varArgs);
}
