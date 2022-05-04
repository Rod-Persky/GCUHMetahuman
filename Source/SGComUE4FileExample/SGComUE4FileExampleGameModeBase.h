#pragma once

#include "CommonStructs.h"
#include "SGComManager.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/ThreadSafeBool.h"
#include "Sound/SoundWave.h"
#include <vector>
#include <list>
#include <string>
#include <map>

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <Experimental/filesystem>
#undef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include "SG_Com.h"

#include "SGComUE4FileExampleGameModeBase.generated.h"


//DECLARE_DELEGATE(FStandardDelegateSignature)
UCLASS()
class SGCOMUE4FILEEXAMPLE_API ASGComUE4FileExampleGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    // Constructor
    ASGComUE4FileExampleGameModeBase();

    // Called when play is started
    void StartPlay() override;

    // Called when a Controller with a PlayerState leaves the game or is destroyed
    void Logout(AController* Exiting) override;

    // Called every frame
    void Tick(float DeltaSeconds) override;

private:
    // Loads the wav file data
    UFUNCTION(BlueprintCallable, Category = "SG")
    void LoadAudioFile(const FString FilePath);

    /** Start watching kernel file for changes */
    UFUNCTION(BlueprintCallable, Category = "OpenCL Functions")
    void WatchKernelFolder(const FString& ProjectRelativeFolder = TEXT("Kernels"));

    // Setup the SG_Com engine configuration
    void SetupEngineConfig(SG_COM_EngineConfig& EngineConfig);

    // Process audio data in the engine
    void ProcessFrameWorker();

    // Release resources
    void EndSession();

    // Engine status callback
    static void EngineStatusCallback(SG_COM_EngineHandle Handle,
                                     SG_COM_Status Status,
                                     const char* Message,
                                     void* CustomEngineData);

    static void EngineBroadcastCallback(SG_COM_EngineHandle Handle,
                                        char* packet,
                                        sg_size packet_bytes,
                                        void* custom_engine_data);

    // Audio asset pointer
    USoundWave* AudioClip;

    // Tracks if the ProcessFrameWorker thread should be processing audio
    FThreadSafeBool bProcessAudio = false;

    // Holds the future of the ProcessFrameWorker thread
    TFuture<void> FrameFuture;

    //IDirectoryWatcher::FDirectoryChanged Changed;
    //FStandardDelegateSignature DelegateHandle;
    //TArray<FString> WatchedFolders;
    FDateTime LastWatchEventCall;

    // For passing the audio data to SG Com for processing
    TArray<uint8> AudioFileData;

    std::vector<std::experimental::filesystem::path> watchPaths_;
    std::list<std::experimental::filesystem::path> fileQueue_;
    std::map< std::experimental::filesystem::path, bool> filesDiscovered_;
    std::experimental::filesystem::path currentFile_;

    uint32 SampleRate;
    uint32 BitsPerSample;
    uint32 AudioFormat;
    float AudioLength;
};
