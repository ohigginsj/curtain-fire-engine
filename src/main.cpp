#if defined(__linux__)
    #include "main_linux.h"
#elif defined(_WIN32)
    #include "main_win32.h"
    #define game_code win32_game_code
#endif

real32
GetSecondsElapsed(uint64 OldCounter)
{
    return (real32)(SDL_GetPerformanceCounter() - OldCounter) / (real32)(SDL_GetPerformanceFrequency());
}

PlatformMain
{
    game_code GameCode = {};
    LoadGameCode(&GameCode, LibraryPath);

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_version CompiledVersion;
    SDL_version LinkedVersion;
    SDL_VERSION(&CompiledVersion);
    SDL_GetVersion(&LinkedVersion);
    printf("SDL compiled with version %d.%d.%d\n",
            CompiledVersion.major, CompiledVersion.minor, CompiledVersion.patch);
    printf("SDL linked with version %d.%d.%d\n",
            LinkedVersion.major, LinkedVersion.minor, LinkedVersion.patch);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_AllocateChannels(64);

    int32 WindowScale = 2;

    SDL_Window* Window = SDL_CreateWindow(
        "Curtain Fire Engine",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        LogicalWindowSizeX * WindowScale,
        LogicalWindowSizeY * WindowScale,
        SDL_WINDOW_OPENGL);

    // Init OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GLContext Context = SDL_GL_CreateContext(Window);

    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

    glViewport(0, 0, LogicalWindowSizeX * WindowScale, LogicalWindowSizeY * WindowScale);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Memory
    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(256);
    GameMemory.TransientStorageSize = Megabytes(512);
    size_t TotalSize = (size_t)(GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize);
    GameMemory.PermanentStorage = calloc(TotalSize, 1);
    Assert("Failed to allocate memory from OS", GameMemory.PermanentStorage);
    GameMemory.TransientStorage = (uint8*)(GameMemory.PermanentStorage) + GameMemory.PermanentStorageSize;

    real32 TargetSecondsPerFrame = 1.0f / (real32)GameUpdateHz;
    uint64 PerfCountFrequency = SDL_GetPerformanceFrequency();
    real32 LastFrameFPS = 0.0f;

    uint64 LastCounter = SDL_GetPerformanceCounter();
    uint64 ElapsedCounter = LastCounter;
    uint64 LastCycleCount = _rdtsc();

    SDL_Joystick* Joystick = SDL_JoystickOpen(0);
    if (!Joystick)
    {
        printf("Failed to open joystick: %s\n", SDL_GetError());
    }
    int32 JoystickInstanceId = SDL_JoystickInstanceID(Joystick);

    SDL_Event Event;
    bool32 Paused = false;
    bool32 Running = true;
    bool32 FirstLoop = true;
    while (Running)
    {
        game_input GameInput = {};

        file_time LibraryLastWriteTime = GetLastWriteTime(LibraryPath);
        if (FileTimesAreDifferent(LibraryLastWriteTime, GameCode.LastWriteTime))
        {
            printf("Reloading library... ");
            LoadGameCode(&GameCode, LibraryPath);
            GameInput.ReloadedGameLibrary = true;
            printf("Done.\n");
        }

        const uint8* CurrentKeyState = SDL_GetKeyboardState(0);
        bool32 ControlDown = CurrentKeyState[SDL_SCANCODE_LCTRL];
        while (SDL_PollEvent(&Event))
        {
            if (Event.type == SDL_QUIT)
            {
                Running = false;
            }
            else if (Event.type == SDL_WINDOWEVENT)
            {
            }
            else if (Event.type == SDL_KEYDOWN)
            {
                if (!Event.key.repeat)
                {
                    switch (Event.key.keysym.sym)
                    {
                        case SDLK_ESCAPE:
                            GameInput.Esc.Pressed = true;
                            break;
                        case SDLK_f:
                        {
                            //bool32 IsFullscreen = SDL_GetWindowFlags(Window) & SDL_WINDOW_FULLSCREEN;
                            //SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : SDL_WINDOW_FULLSCREEN);
                        } break;
                        case SDLK_r:
                            GameInput.R.Pressed = true;
                            break;
                        case SDLK_k:
                            GameInput.K.Pressed = true;
                            break;
                        case SDLK_i:
                            GameInput.I.Pressed = true;
                            break;
                        case SDLK_w:
                        {
                            GameInput.W.Pressed = true;

                            if (WindowScale == 1)
                            {
                                WindowScale = 2;
                            }
                            else if (WindowScale == 2)
                            {
                                WindowScale = 1;
                            }
                            SDL_SetWindowSize(Window,
                                              WindowScale * LogicalWindowSizeX,
                                              WindowScale * LogicalWindowSizeY);
                            glViewport(0, 0, LogicalWindowSizeX * WindowScale, LogicalWindowSizeY * WindowScale);
                            GameInput.ResizeWindow.Active = true;
                            GameInput.ResizeWindow.WindowScale = WindowScale;
                        } break;
                        case SDLK_a:
                            GameInput.A.Pressed = true;
                            break;
                        case SDLK_s:
                            GameInput.S.Pressed = true;
                            break;
                        case SDLK_d:
                            GameInput.D.Pressed = true;
                            break;
                        case SDLK_z:
                            GameInput.Z.Pressed = true;
                            break;
                        case SDLK_x:
                            GameInput.X.Pressed = true;
                            break;
                        case SDLK_p:
                            GameInput.P.Pressed = true;
                            break;
                        case SDLK_n:
                            GameInput.N.Pressed = true;
                            break;
                        case SDLK_UP:
                            GameInput.ArrowUp.Pressed = true;
                            break;
                        case SDLK_DOWN:
                            GameInput.ArrowDown.Pressed = true;
                            break;
                        case SDLK_LEFT:
                            GameInput.ArrowLeft.Pressed = true;
                            break;
                        case SDLK_RIGHT:
                            GameInput.ArrowRight.Pressed = true;
                            break;
                        case SDLK_SPACE:
                            GameInput.Space.Pressed = true;
                            break;
                        case SDLK_RETURN:
                            GameInput.Enter.Pressed = true;
                            break;
                    }
                }
            }
            else if (Event.type == SDL_JOYBUTTONDOWN)
            {
                if (EnableController)
                {
                    if (Event.jbutton.button == DualShock3_Select)
                    {
                        GameInput.R.Pressed = true;
                    }
                }
            }
            else if (Event.type == SDL_MOUSEBUTTONDOWN)
            {
                switch (Event.button.button)
                {
                    case SDL_BUTTON_LEFT:
                        GameInput.MousePressed_Left = true;
                        break;
                    case SDL_BUTTON_RIGHT:
                        GameInput.MousePressed_Right = true;
                        break;
                }
            }
        }

        if (EnableController)
        {
            int32 XAxisValue = SDL_JoystickGetAxis(Joystick, SDL_CONTROLLER_AXIS_LEFTX);
            if (XAxisValue < -Controller_AnalogDeadZone)
            {
                GameInput.ArrowLeft.Down = true;
            }
            if (XAxisValue > Controller_AnalogDeadZone)
            {
                GameInput.ArrowRight.Down = true;
            }
            int32 YAxisValue = SDL_JoystickGetAxis(Joystick, SDL_CONTROLLER_AXIS_LEFTY);
            if (YAxisValue < -Controller_AnalogDeadZone)
            {
                GameInput.ArrowUp.Down = true;
            }
            if (YAxisValue > Controller_AnalogDeadZone)
            {
                GameInput.ArrowDown.Down = true;
            }

            if (SDL_JoystickGetButton(Joystick, DualShock3_X))
            {
                GameInput.Shift.Down = true;
            }
            if (SDL_JoystickGetButton(Joystick, DualShock3_Square))
            {
                GameInput.Z.Down = true;
            }
        }

        if (CurrentKeyState[SDL_SCANCODE_Z])
        {
            GameInput.Z.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_W])
        {
            GameInput.W.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_A])
        {
            GameInput.A.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_S])
        {
            GameInput.S.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_D])
        {
            GameInput.D.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_UP])
        {
            GameInput.ArrowUp.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_DOWN])
        {
            GameInput.ArrowDown.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_LEFT])
        {
            GameInput.ArrowLeft.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_RIGHT])
        {
            GameInput.ArrowRight.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_LSHIFT])
        {
            GameInput.Shift.Down = true;
        }
        if (CurrentKeyState[SDL_SCANCODE_LCTRL])
        {
            GameInput.Control.Down = true;
        }

        if (FirstLoop)
        {
            GameInput.ResizeWindow.Active = true;
            GameInput.ResizeWindow.WindowScale = WindowScale;
        }

        GameInput.DeltaTime = TargetSecondsPerFrame;
        GameInput.LastFrameFPS = LastFrameFPS;
        if (Running)
        {
            Running = GameCode.GameLoopFunction(&GameMemory, &GameInput);
            SDL_GL_SwapWindow(Window);
        }

        // NOTE: Just let VSync do its job...
#if 0
        if (GetSecondsElapsed(LastCounter) < TargetSecondsPerFrame)
        {
            int BufferMS = 1;
            int TimeToSleep = ((TargetSecondsPerFrame - GetSecondsElapsed(LastCounter)) * 1000) - BufferMS;

            if (TimeToSleep > 0)
            {
                SDL_Delay(TimeToSleep);
            }
            while (GetSecondsElapsed(LastCounter) < TargetSecondsPerFrame)
            {
                // Wait for remaining time that SDL_Delay missed
            }
        }
        else
        {
            // Missed frame rate!
        }
#endif

        uint64 EndCounter = SDL_GetPerformanceCounter();
        uint64 CounterElapsed = EndCounter - LastCounter;

        uint64 EndCycleCount = _rdtsc();
        uint64 CyclesElapsed = EndCycleCount - LastCycleCount;

        real64 FrameMS = (1000.0f * (real64)CounterElapsed) / (real64)PerfCountFrequency;
        real64 FPS = (real64)PerfCountFrequency / (real64)CounterElapsed;
        real64 FrameMCycles = (real64)CyclesElapsed / (1000.0f * 1000.0f);
        LastFrameFPS = (real32)FPS;

        //printf("%0.10fms/f, %0.1ff/s, %0.2fMc/f\n", FrameMS, FPS, FrameMCycles);

        LastCycleCount = EndCycleCount;
        LastCounter = EndCounter;

        FirstLoop = false;
    }

    // TODO: Determine if it's actually necessary to call these on program termination...
    SDL_DestroyWindow(Window);
    SDL_Quit();

    return 0;
}
