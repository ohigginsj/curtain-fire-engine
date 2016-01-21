#pragma once

#include <stdint.h>

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef int32_t  bool32;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float    real32;
typedef double   real64;

//#define Assert(Expression) if(!(Expression)) { *(int*)(0) = 0; }
#define Assert(Message, ...) do { \
    if (!(__VA_ARGS__)) { \
        fprintf(stderr, "%s\n", Message); \
        abort(); \
    } \
} while(0)

#define Warn(Message, ...) do { \
    if (!(__VA_ARGS__)) { \
        fprintf(stderr, "%s\n", Message); \
    } \
} while(0)

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

// Functions
#define internal static
// Local variables
#define local_persist static
// Global variables
#define global static

struct game_memory
{
    uint64 PermanentStorageSize;
    void* PermanentStorage;

    uint64 TransientStorageSize;
    void* TransientStorage;
};
