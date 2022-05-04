// This is a UE4 wrapper for the SG_Com API.

#pragma once

#include "CommonStructs.h"
#include "SG_Com.h"

#include "CoreMinimal.h"


class SGCOMUE4FILEEXAMPLE_API FSGComManager
{
public:
    // Initialize the SG_Com API
    static bool Initialize(const FString& LogFilePath);

    // Shut down the SG_Com API
    static bool Shutdown();

    // Create the Engine
    static bool CreateEngine(SG_COM_EngineConfig EngineConfig);

    // Destroy the Engine
    static bool DestroyEngine();

    // Input an array of audio data to the Engine
    static bool InputAudio(const TArray<uint8>& AudioData);

    // Process any audio in the Engine input buffer
    static bool ProcessAudio(int* RemainingFrames = nullptr);

    // Update Animation Nodes for the local Player
    static bool UpdateAnimation(float DeltaSeconds);

    // Get the animation nodes for the local Player
    static bool GetAnimationNodes(FAvatarInfo& AvatarInfo);

    // Check if the engine handle is valid
    static bool IsEngineValid();

private:
    FSGComManager(){};
    ~FSGComManager(){};

    // SG_Com logging callback
    static void LoggingCallback(const char* message);

    static FString LogPath;

    static SG_COM_EngineHandle EngineHandle;
    static SG_COM_PlayerHandle PlayerHandle;

    // Tracks the total tick time
    static float TotalTime;

    static bool bAnimationStarted;
};
