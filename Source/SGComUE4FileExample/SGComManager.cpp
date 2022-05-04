// Fill out your copyright notice in the Description page of Project Settings.


#include "SGComManager.h"

#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/FileHelper.h"

FString FSGComManager::LogPath;

SG_COM_EngineHandle FSGComManager::EngineHandle = nullptr;
SG_COM_PlayerHandle FSGComManager::PlayerHandle = nullptr;
float FSGComManager::TotalTime = 0.f;
bool FSGComManager::bAnimationStarted = false;

// ========================================================
void LogException(SG_COM_Error err) {
    if (err == SG_COM_Error::SG_COM_ERROR_EXCEPTION) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM Exception] : %s"), *FString(SG_COM_GetExceptionText()));
    }
}

// ========================================================
// Initialize the SG_Com API
// ========================================================
bool FSGComManager::Initialize(const FString& LogFilePath)
{
    FString LicenseString;
    if (IFileManager::Get().FileExists(*FString(FPaths::ProjectContentDir() + "Resources/sg.lic"))) {
        LicenseString = FPaths::ProjectContentDir() + "Resources";
    }
    else {
        FFileHelper::LoadFileToString(LicenseString, *FString(FPaths::ProjectContentDir() + "Resources/license"));
        if (LicenseString.IsEmpty()) {        
            UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : LicenseString is empty. A valid license string must be passed to SG_COM_Initialize"));
        }
    }

    FSGComManager::LogPath = LogFilePath;
    SG_COM_Error err = SG_COM_Initialize(SG_LOGLEVEL_DEBUG, &LoggingCallback, TCHAR_TO_ANSI(*LicenseString), nullptr, nullptr);
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to initialize SG_Com: %d"), err);
        LogException(err);
        return false;
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Initialised version: %s"), *FString(SG_COM_GetVersionString()));
    }

    return true;
}

// ========================================================
// Shut down the SG_Com API
// ========================================================
bool FSGComManager::Shutdown()
{
    SG_COM_Error err = SG_COM_Shutdown();
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to shut down SG_Com: %d"), err);
        LogException(err);
        return false;
    }

    return true;
}

// ========================================================
// Create an Engine
// ========================================================
bool FSGComManager::CreateEngine(SG_COM_EngineConfig EngineConfig)
{
    // Create the local Player
    SG_COM_PlayerConfig PlayerConfig;
    PlayerConfig.character_file_in_memory = EngineConfig.character_file_in_memory;
    PlayerConfig.character_file_bytes = EngineConfig.character_file_bytes;
    PlayerConfig.animation_type = SG_AnimationType::SG_NORMAL_ANIMATION;
    PlayerConfig.buffer_sec = EngineConfig.buffer_sec;

    SG_COM_Error err = SG_COM_CreatePlayer(&PlayerConfig, &PlayerHandle);
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to create local player: %d"), err);
        LogException(err);
        return false;
    }

    // Create the engine
    EngineConfig.local_player = PlayerHandle;
    err = SG_COM_CreateEngine(&EngineConfig, &EngineHandle);
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to create the engine: %d"), err);
        LogException(err);
        return false;
    }

    return true;
}

// ========================================================
// Destroy the Engine
// ========================================================
bool FSGComManager::DestroyEngine()
{
    SG_COM_Error err = SG_COM_DestroyEngine(EngineHandle);
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to destroy the engine: %d"), err);
        LogException(err);
        return false;
    }
    EngineHandle = nullptr;

    err = SG_COM_DestroyPlayer(PlayerHandle);
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to destroy the local player: %d"), err);
        LogException(err);
        return false;
    }
    PlayerHandle = nullptr;

    return false;
}

// ========================================================
// Input an array of audio data to the Engine
// ========================================================
bool FSGComManager::InputAudio(const TArray<uint8>& AudioData)
{
    SG_COM_Error err = SG_COM_InputAudio(EngineHandle, (void*)AudioData.GetData(), AudioData.Num());
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to input audio: %d"), err);
        LogException(err);
        return false;
    }

    return true;
}

// ========================================================
// Process any audio in the Engine input buffer
// ========================================================
bool FSGComManager::ProcessAudio(int* RemainingFrames_out)
{
    int ProcessedFrames = 0;
    int RemainingFrames = 1;
    SG_COM_Error err = SG_COM_Error::SG_COM_ERROR_OK;
    err = SG_COM_ProcessTick(EngineHandle, &ProcessedFrames, &RemainingFrames);

    if (RemainingFrames_out) {
        *RemainingFrames_out = RemainingFrames;
    }

    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Process tick failed: %d"), err);
        LogException(err);
        return false;
    }
    return true;
}

// ========================================================
// Update Animation Nodes for the local Player
// ========================================================
bool FSGComManager::UpdateAnimation(float DeltaSeconds)
{
    TotalTime += (DeltaSeconds * 1000.f); // Converts delta seconds to milliseconds

    double MinTimeMs = 0;
    double MaxTimeMs = 0;
    SG_COM_Error err = SG_COM_GetPlayableRange(PlayerHandle, &MinTimeMs, &MaxTimeMs);
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to get playable range: %d"), err);
        LogException(err);
        return false;
    }

    if (!bAnimationStarted && (MaxTimeMs - MinTimeMs) >= 20)
    {
        TotalTime = 0.f;
        bAnimationStarted = true;
    }
    
    if (bAnimationStarted)
    {
        double CurrentTimeMs;
        err = SG_COM_UpdateAnimation(PlayerHandle, TotalTime, &CurrentTimeMs); // Attempts to update the animation
        TotalTime = CurrentTimeMs; // Sets the time total to the clamped value

        if (err != SG_COM_Error::SG_COM_ERROR_OK) {
            UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to update animation: %d"), err);
            LogException(err);
            return false;
        }
    }

    return true;
}

// ========================================================
// Get the animation nodes for the local Player
// ========================================================
bool FSGComManager::GetAnimationNodes(FAvatarInfo& AvatarInfo)
{
    SG_COM_Error err = SG_COM_GetAnimationNodes(PlayerHandle, &AvatarInfo.AnimationNodes, &AvatarInfo.NumAnimationNodes);
    if (err != SG_COM_Error::SG_COM_ERROR_OK) {
        UE_LOG(LogTemp, Warning, TEXT("[SG_COM] : Failed to get animation nodes: %d"), err);
        LogException(err);
        return false;
    }

    return true;
}

// ========================================================
// Check if the engine handle is valid
// ========================================================
bool FSGComManager::IsEngineValid()
{
    return EngineHandle != nullptr;
}

// ========================================================
// Transceiver logging callback
// ========================================================
void FSGComManager::LoggingCallback(const char* message) {
    FFileHelper::SaveStringToFile(
        FString(message),
        *LogPath,
        FFileHelper::EEncodingOptions::AutoDetect,
        &IFileManager::Get(),
        FILEWRITE_Append);
}
