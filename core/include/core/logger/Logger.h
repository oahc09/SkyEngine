//
// Created by Zach Lee on 2021/11/7.
//
#pragma once

#include <cstdint>
#if __ANDROID__
#include <android/log.h>
#endif

//#define ENABLE_FILE_LINE

namespace sky {

    class Logger {
    public:
        static void Print(const char *tag, const char *type, const char *fmt, ...);
        static void PrintW(const wchar_t *tag, const wchar_t *type, const wchar_t *fmt, ...);
    };

} // namespace sky

#if __ANDROID__

#define LOG_E(tag, fmt, ...) __android_log_print(ANDROID_LOG_ERROR, tag, fmt, ##__VA_ARGS__)
#define LOG_W(tag, fmt, ...) __android_log_print(ANDROID_LOG_WARN, tag, fmt, ##__VA_ARGS__)
#define LOG_I(tag, fmt, ...) __android_log_print(ANDROID_LOG_INFO, tag, fmt, ##__VA_ARGS__)

#define LOGW_E(tag, fmt, ...)
#define LOGW_W(tag, fmt, ...)
#define LOGW_I(tag, fmt, ...)

#else
#define LOG_E(tag, fmt, ...) Logger::Print(tag, "ERROR", fmt, ##__VA_ARGS__)
#define LOG_W(tag, fmt, ...) Logger::Print(tag, "WARNING", fmt, ##__VA_ARGS__)
#define LOG_I(tag, fmt, ...) Logger::Print(tag, "INFO", fmt, ##__VA_ARGS__)

#define LOGW_E(tag, fmt, ...) Logger::PrintW(tag, L"ERROR", fmt, ##__VA_ARGS__)
#define LOGW_W(tag, fmt, ...) Logger::PrintW(tag, L"WARNING", fmt, ##__VA_ARGS__)
#define LOGW_I(tag, fmt, ...) Logger::PrintW(tag, L"INFO", fmt, ##__VA_ARGS__)

#endif