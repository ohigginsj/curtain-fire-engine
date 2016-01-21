#include <sys/stat.h>
#include <dlfcn.h>
#include <x86intrin.h>

#include "game.h"

#define PlatformMain \
    int main(int argc, char* argv[])

#define   file_time               time_t
#define   LibraryPath             Linux_LibraryPath
#define   game_code               linux_game_code
#define   LoadGameCode            Linux_LoadGameCode
#define   GetLastWriteTime        Linux_GetLastWriteTime
#define   FileTimesAreDifferent   Linux_FileTimesAreDifferent
#define   Cleanup                 Linux_Cleanup

const char* Linux_LibraryPath = "bin/cfe.so";

struct linux_game_code
{
    void* LibraryHandle;
    time_t LastWriteTime;

    game_loop_function* GameLoopFunction;
};

time_t
Linux_GetLastWriteTime(const char* path)
{
    struct stat Stat = {};
    stat(path, &Stat);
    time_t LastWriteTime = Stat.st_mtime;
    return LastWriteTime;
}

void
Linux_LoadGameCode(linux_game_code* GameCode,
             const char* Path)
{
    if (GameCode->LibraryHandle)
    {
        dlclose(GameCode->LibraryHandle);
    }
    GameCode->LibraryHandle = dlopen(Path, RTLD_NOW);
    if (!GameCode->LibraryHandle)
    {
        printf("Could not load library.\n");
        printf("%s\n", dlerror());
    }

    GameCode->GameLoopFunction = (game_loop_function*)dlsym(GameCode->LibraryHandle, "GameLoop");
    if (!GameCode->GameLoopFunction)
    {
        printf("Could not load game loop!\n");
        printf("%s\n", dlerror());
    }

    GameCode->LastWriteTime = Linux_GetLastWriteTime(Path);
}

int
Linux_FileTimesAreDifferent(time_t A, time_t B)
{
    return A != B;
}

void
Linux_Cleanup()
{
}

