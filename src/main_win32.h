#include <intrin.h>

#include "game.h"

#define PlatformMain \
    int CALLBACK \
    WinMain(HINSTANCE Instance, \
            HINSTANCE PrevInstance, \
            LPSTR CommandLine, \
            int ShowCode)

#define _rdtsc __rdtsc

#define   file_time               FILETIME
#define   LibraryPath             Win32_LibraryPath
#define   game_code               win32_game_code
#define   LoadGameCode            Win32_LoadGameCode
#define   GetLastWriteTime        Win32_GetLastWriteTime
#define   FileTimesAreDifferent   Win32_FileTimesAreDifferent
#define   Cleanup                 Win32_Cleanup

const char* Win32_LibraryPath = "cfe.dll";

struct win32_game_code
{
    HMODULE LibraryHandle;
    FILETIME LastWriteTime;

    game_loop_function* GameLoopFunction;
};

inline FILETIME
Win32_GetLastWriteTime(const char* Filename)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }

    return(LastWriteTime);
}

void
Win32_LoadGameCode(win32_game_code* GameCode,
                   const char* Path)
{
    if(GameCode->LibraryHandle)
    {
        FreeLibrary(GameCode->LibraryHandle);
        GameCode->LibraryHandle = 0;
    }

    GameCode->LibraryHandle = LoadLibraryA(Path);
    if(GameCode->LibraryHandle)
    {
        GameCode->GameLoopFunction = (game_loop_function *)
            GetProcAddress(GameCode->LibraryHandle, "GameLoop");
        if (!GameCode->GameLoopFunction)
        {
            Assert("Could not load Game loop", false);
        }
    }
    GameCode->LastWriteTime = Win32_GetLastWriteTime(Path);
}

int
Win32_FileTimesAreDifferent(FILETIME A, FILETIME B)
{
    return CompareFileTime(&A, &B);
}

void
Win32_Cleanup()
{
}
