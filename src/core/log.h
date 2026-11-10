#pragma once

// Appends to fnutil.log next to the DLL and mirrors to the debugger output.
namespace Log {
void Init(void* dllModule);
void Shutdown();
void Write(const char* fmt, ...);
}

#define LOG(...) ::Log::Write(__VA_ARGS__)
