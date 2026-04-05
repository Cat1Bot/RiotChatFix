#include "pch.h"
#include <Windows.h>
#include <stdint.h>

static uintptr_t FindPattern(uintptr_t base, size_t size, const uint8_t* pattern, const char* mask)
{
    for (size_t i = 0; i < size; i++)
    {
        bool found = true;

        for (size_t j = 0; mask[j]; j++)
        {
            if (mask[j] == 'x' && pattern[j] != *(uint8_t*)(base + i + j))
            {
                found = false;
                break;
            }
        }

        if (found)
            return base + i;
    }

    return 0;
}

static bool PatchTLS(uintptr_t moduleBase)
{
    auto dos = (PIMAGE_DOS_HEADER)moduleBase;
    auto nt = (PIMAGE_NT_HEADERS)(moduleBase + dos->e_lfanew);
    size_t size = nt->OptionalHeader.SizeOfImage;

    uint8_t pattern[] = {0x66, 0x41, 0xC7, 0x86, 0xBD, 0x0B, 0x00, 0x00, 0x00, 0x00};

    const char* mask = "xxxxxxxx??";

    uintptr_t instr = FindPattern(moduleBase, size, pattern, mask);
    if (!instr)
        return false;

    DWORD oldProtect;
    if (!VirtualProtect((LPVOID)instr, 10, PAGE_EXECUTE_READWRITE, &oldProtect))
        return false;

    *(uint16_t*)(instr + 8) = 0;

    VirtualProtect((LPVOID)instr, 10, oldProtect, &oldProtect);
    FlushInstructionCache(GetCurrentProcess(), (LPCVOID)instr, 10);

    return true;
}

static DWORD WINAPI InitThread(LPVOID param)
{
    HMODULE self = (HMODULE)param;

    HMODULE target = nullptr;
    while (!(target = GetModuleHandleA("RiotGamesApi.dll")))
    {
        Sleep(50);
    }

    bool success = PatchTLS((uintptr_t)target);
    if (success)
    {
        FreeLibraryAndExitThread(self, 0);
    }

    TerminateProcess(GetCurrentProcess(), 1);

    return 0;
}

static BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, InitThread, hModule, 0, nullptr);
    }
    return TRUE;
}