#pragma once

#include "game.h"

const uint32 MenuEntryCountMax = 32;
const uint32 MenuEntryTextMax = 32;

typedef bool32 (*menu_function)(game_state* GameState);

struct menu_entry
{
    char Text[MenuEntryTextMax];
    char TextSelected[MenuEntryTextMax];
    menu_function Function;
};

struct menu_grid
{
    uint32 Columns;
    bool32 Wrap;

    v2 Position;
    int32 Layer;

    uint32 EntryCount;
    menu_entry Entries[MenuEntryCountMax];
    uint32 HotEntryIndex;
};

void
AddMenuEntry(menu_grid* Menu,
             const char* Text,
             const char* TextSelected,
             menu_function Function)
{
    Assert("Out of menu entries.", Menu->EntryCount < MenuEntryCountMax);
    menu_entry* NewEntry = &Menu->Entries[Menu->EntryCount];
    strcpy(NewEntry->Text, Text);
    strcpy(NewEntry->TextSelected, TextSelected);
    NewEntry->Function = Function;
    ++Menu->EntryCount;
}

void
InitializeMenu(menu_grid* Menu,
               int32 Columns,
               bool32 Wrap,
               v2 Position,
               int32 Layer)
{
    Menu->Columns = Columns;
    Menu->Wrap = Wrap;
    Menu->Position = Position;
    Menu->Layer = Layer;
    Menu->EntryCount = 0;
    Menu->HotEntryIndex = 0;
}

bool32
UpdateMenu(game_state* GameState,
           menu_grid* Menu, game_input* Input)
{
    int32 HotColumn = Menu->HotEntryIndex % Menu->Columns;

    if (Input->ArrowDown.Pressed)
    {
        if (Menu->HotEntryIndex < Menu->EntryCount - Menu->Columns)
        {
            Menu->HotEntryIndex += Menu->Columns;
        }
        else if (Menu->Wrap)
        {
            Menu->HotEntryIndex = HotColumn;
        }
    }
    if (Input->ArrowUp.Pressed)
    {
        if (Menu->HotEntryIndex >= Menu->Columns)
        {
            Menu->HotEntryIndex -= Menu->Columns;
        }
        else if (Menu->Wrap)
        {
            Menu->HotEntryIndex = Menu->EntryCount - (Menu->Columns - Menu->HotEntryIndex);
        }
    }
    if (Input->ArrowRight.Pressed)
    {
        if ((Menu->HotEntryIndex + 1) % Menu->Columns != 0)
        {
            Menu->HotEntryIndex += 1;
        }
        else if (Menu->Wrap)
        {
            Menu->HotEntryIndex -= Menu->Columns - 1;
        }
    }
    if (Input->ArrowLeft.Pressed)
    {
        if (HotColumn != 0)
        {
            Menu->HotEntryIndex -= 1;
        }
        else if (Menu->Wrap)
        {
            Menu->HotEntryIndex += Menu->Columns - 1;
        }
    }

    for (uint32 EntryIndex = 0;
         EntryIndex < Menu->EntryCount;
         ++EntryIndex)
    {
        menu_entry* Entry = &Menu->Entries[EntryIndex];
        char* Text = (EntryIndex == Menu->HotEntryIndex) ?
            Entry->TextSelected :
            Entry->Text;
        uint32 Column = EntryIndex % Menu->Columns;
        uint32 Row = FloorReal32((real32)EntryIndex / (real32)Menu->Columns);
        //PushRect(GameState,
                 //Rect(V2(WorldSizeX / 2.0f + 50.0f * Column - 50.0f, (WorldSizeY / 2.0f) + 20.0f * Row),
                      //V2(100.0f, 10.0f)),
                 //Colors.Black,
                 //true,
                 //1);
        PushText(GameState->Renderer,
                 Text,
                 Menu->Position + V2(50.0f * Column, 20.0f * Row),
                 Colors.White,
                 true,
                 Menu->Layer);
    }

    bool32 ModeChanged = false;

    if (Input->Z.Pressed || Input->Space.Pressed || Input->Enter.Pressed)
    {
        menu_entry* HotEntry = &Menu->Entries[Menu->HotEntryIndex];
        ModeChanged = HotEntry->Function(GameState);
    }

    return ModeChanged;
}
