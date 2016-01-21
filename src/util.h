#pragma once

void
LoadFileAsString(memory_zone* Memory, const char* Filename, char** Buffer)
{
    uint64 Length;
    FILE* File = fopen(Filename, "rb");

	Assert("Could not load file.", File);

    if (File)
    {
        fseek(File, 0, SEEK_END);
        Length = ftell(File);
        fseek(File, 0, SEEK_SET);
        *Buffer = PushArray(Memory, Length + 1, char);
        if (*Buffer)
        {
            fread(*Buffer, 1, Length, File);
            (*Buffer)[Length] = '\0';
        }
        fclose(File);
    }
}
