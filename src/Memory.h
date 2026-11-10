#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstdint>
#include <string>
#include <TlHelp32.h>

class Memory
{
public:
    static HANDLE   hProcess;
    static uintptr_t BaseAddress;

    static bool Open(const wchar_t* processName)
    {
        DWORD pid = GetPID(processName);
        if (!pid) return false;

        hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION
                               | PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!hProcess || hProcess == INVALID_HANDLE_VALUE) return false;

        BaseAddress = GetModuleBase(pid, processName);
        return BaseAddress != 0;
    }

    static void Close()
    {
        if (hProcess && hProcess != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hProcess);
            hProcess = nullptr;
        }
    }

    template<typename T>
    static T Read(uintptr_t address)
    {
        T value{};
        ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
        return value;
    }

    template<typename T>
    static bool ReadInto(uintptr_t address, T& out)
    {
        return ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &out, sizeof(T), nullptr) != FALSE;
    }

    template<typename T>
    static bool Write(uintptr_t address, const T& value)
    {
        return WriteProcessMemory(hProcess, reinterpret_cast<LPVOID>(address), &value, sizeof(T), nullptr) != FALSE;
    }

    static uintptr_t ReadPtr(uintptr_t address)
    {
        return Read<uintptr_t>(address);
    }

    static bool IsValidPtr(uintptr_t ptr)
    {
        if (!ptr || ptr < 0x1000 || ptr > 0x7FFFFFFFFFFFFFFF) return false;
        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQueryEx(hProcess, reinterpret_cast<LPCVOID>(ptr), &mbi, sizeof(mbi)))
            return false;
        return mbi.State == MEM_COMMIT &&
               !(mbi.Protect & PAGE_NOACCESS) &&
               !(mbi.Protect & PAGE_GUARD);
    }

    static std::wstring ReadFString(uintptr_t address)
    {
        // FString: TArray<wchar_t> = { wchar_t* Data, int32 Count, int32 Max }
        uintptr_t data  = ReadPtr(address);
        int32_t   count = Read<int32_t>(address + 0x8);
        if (!data || count <= 0 || count > 512) return {};

        std::wstring result(static_cast<size_t>(count), L'\0');
        ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(data),
                          result.data(), count * sizeof(wchar_t), nullptr);
        // Strip null terminator
        while (!result.empty() && result.back() == L'\0')
            result.pop_back();
        return result;
    }

private:
    static DWORD GetPID(const wchar_t* name)
    {
        PROCESSENTRY32W pe{ sizeof(pe) };
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return 0;
        DWORD pid = 0;
        if (Process32FirstW(snap, &pe))
        {
            do {
                if (_wcsicmp(pe.szExeFile, name) == 0) { pid = pe.th32ProcessID; break; }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
        return pid;
    }

    static uintptr_t GetModuleBase(DWORD pid, const wchar_t* modName)
    {
        MODULEENTRY32W me{ sizeof(me) };
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap == INVALID_HANDLE_VALUE) return 0;
        uintptr_t base = 0;
        if (Module32FirstW(snap, &me))
        {
            do {
                if (_wcsicmp(me.szModule, modName) == 0) { base = reinterpret_cast<uintptr_t>(me.modBaseAddr); break; }
            } while (Module32NextW(snap, &me));
        }
        CloseHandle(snap);
        return base;
    }
};

inline HANDLE   Memory::hProcess    = nullptr;
inline uintptr_t Memory::BaseAddress = 0;
