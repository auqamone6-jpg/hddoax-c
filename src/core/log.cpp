#include "core/log.h"

#include <windows.h>

#include <cstdarg>
#include <cstdio>
#include <mutex>

namespace {
std::mutex g_mutex;
FILE* g_file = nullptr;
}

void Log::Init(void* dllModule) {
    wchar_t path[MAX_PATH];
    DWORD len = GetModuleFileNameW(static_cast<HMODULE>(dllModule), path, MAX_PATH);
    while (len > 0 && path[len - 1] != L'\\' && path[len - 1] != L'/') --len;
    path[len] = 0;
    wcsncat(path, L"fnutil.log", MAX_PATH - len - 1);
    std::lock_guard lock(g_mutex);
    g_file = _wfopen(path, L"w");
}

void Log::Shutdown() {
    std::lock_guard lock(g_mutex);
    if (g_file) fclose(g_file);
    g_file = nullptr;
}

void Log::Write(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    std::lock_guard lock(g_mutex);
    if (g_file) {
        fprintf(g_file, "%s\n", buf);
        fflush(g_file);
    }
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
}
