#include "SGComUE4FileExampleGameModeBase.h"

#include "HAL/PlatformFilemanager.h"
#include "Kismet/GameplayStatics.h"
#include "Audio.h"


const FString CharacterFileDirectory = FPaths::ProjectContentDir() + "Resources/Characters/";
const FString AudioFileDirectory = FPaths::ProjectContentDir() + "Resources/Audio/";

// ========================================================
// Map audio sample rate from Hz to SG_AudioSampleRate
// ========================================================
static SG_AudioSampleRate MapSampleRate(int SampleRate)
{
    switch (SampleRate) {
        case 8000:
            return SG_AUDIO_8_KHZ;
        case 12000:
            return SG_AUDIO_12_KHZ;
        case 16000:
            return SG_AUDIO_16_KHZ;
        case 24000:
            return SG_AUDIO_24_KHZ;
        case 32000:
            return SG_AUDIO_32_KHZ;
        case 44100:
            return SG_AUDIO_44_1_KHZ;
        case 48000:
            return SG_AUDIO_48_KHZ;
        default:
            return SG_AUDIO_8_KHZ;
    };
}

// ========================================================
// Map audio sample type to SG_AudioSampleType
// ========================================================
static SG_AudioSampleType MapSampleType(const uint16 AudioFormat, const uint16 BitsPerSample)
{
    switch (AudioFormat) {
        case 0x0001:
            switch (BitsPerSample) {
                case 16:
                    return SG_AUDIO_INT_16;
                case 32:
                    return SG_AUDIO_INT_32;
                default:
                    UE_LOG(LogTemp, Warning, TEXT("[APP] : PCM samples must be either 16 or 32 bit"));
                    checkNoEntry();
                    return SG_AUDIO_INT_16;
            };
        case 0x0003:
            switch (BitsPerSample) {
                case 32:
                    return SG_AUDIO_FLOAT_32;
                default:
                    UE_LOG(LogTemp, Warning, TEXT("[APP] : IEEE float samples must be 32 bit"));
                    checkNoEntry();
                    return SG_AUDIO_FLOAT_32;
            };
        default:
            UE_LOG(LogTemp, Warning, TEXT("[APP] : Sample type must be either PCM or IEEE float"));
            checkNoEntry();
            return SG_AUDIO_INT_16;
    };
}

// ========================================================
// Constructor
// ========================================================
ASGComUE4FileExampleGameModeBase::ASGComUE4FileExampleGameModeBase()
{
    PrimaryActorTick.bCanEverTick = true;
    LastWatchEventCall = FDateTime::Now();
}

// ========================================================
// Called when play is started
// ========================================================
void ASGComUE4FileExampleGameModeBase::StartPlay()
{
    Super::StartPlay();

    // Make sure that the directory exists. Used for logging
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*FPaths::ProjectPersistentDownloadDir())) {
        PlatformFile.CreateDirectory(*FPaths::ProjectPersistentDownloadDir());
    }

    FString LogFileName = "SG_COM_log_" + FDateTime::Now().ToString();
    FString SGComLogPath = FPaths::ProjectPersistentDownloadDir() + "/" + LogFileName + ".txt";
    FSGComManager::Initialize(SGComLogPath);

    SG_COM_EngineConfig EngineConfig;
    SetupEngineConfig(EngineConfig);
    bool success = FSGComManager::CreateEngine(EngineConfig);

    if (success) {
        // Input audio file data        
        FSGComManager::InputAudio(AudioFileData);
        bProcessAudio = true;

        // Launch the process frame thread
        FrameFuture = Async(EAsyncExecution::Thread, [&]() { ProcessFrameWorker(); });

        // Play audio
        UGameplayStatics::PlaySound2D(this, AudioClip);
    }
}

// ========================================================
// Called when a Controller with a PlayerState leaves the
// game or is destroyed
// ========================================================
void ASGComUE4FileExampleGameModeBase::Logout(AController* Exiting)
{
    EndSession();
    FSGComManager::Shutdown();
}

// ========================================================
// Called every frame
// ========================================================
void ASGComUE4FileExampleGameModeBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    
    FSGComManager::UpdateAnimation(DeltaSeconds);

    // When the Frame Worker is finished, it sets the state of the
    // FrameFuture to IsReady to indicate there is a return code
    // we don't exactly care for the return code and instead will
    // play the next queued track
    if (FrameFuture.IsReady()) {

        // Check there is actually a file
        if (fileQueue_.empty()) {
            return;
        }

        currentFile_ = fileQueue_.front();
        fileQueue_.pop_front();
        FString cf{ currentFile_.c_str() };
        LoadAudioFile(cf);
        std::error_code ec;
        std::experimental::filesystem::remove(currentFile_, ec);

        UE_LOG(LogTemp, Warning, TEXT("[APP] : master tick"));
        // Input audio file data        
        FSGComManager::InputAudio(AudioFileData);
        bProcessAudio = true;

        // Launch the process frame thread
        FrameFuture = Async(EAsyncExecution::Thread, [&]() { ProcessFrameWorker(); });

        // Play audio
        UGameplayStatics::PlaySound2D(this, AudioClip);
    }

    // RP: I don't like this method - we should not poll the FS every second
    auto delta = FDateTime::Now() - LastWatchEventCall;
    if (delta.GetTotalSeconds() > 1) {
        LastWatchEventCall = FDateTime::Now();
        for (auto& p : watchPaths_) {
            for (auto& f : std::experimental::filesystem::directory_iterator(p)) {
                const auto& ext = f.path().extension().string();
                if (ext != ".wav") {
                    continue;
                }

                if (std::experimental::filesystem::is_regular_file(f)) {
                    auto itr = filesDiscovered_.find(f);
                    if (itr == filesDiscovered_.end()) {
                        filesDiscovered_.emplace(f, false);
                        fileQueue_.push_back(f);
                    }
                }
            }
        }
    }

}

// ========================================================
// Loads the wav file data
// ========================================================
void ASGComUE4FileExampleGameModeBase::LoadAudioFile(const FString FilePath)
{   
    // Creates the USoundWave object used to play the audio
    AudioClip = NewObject<USoundWave>(USoundWave::StaticClass());

    // Resets audio related member variable values
    SampleRate = 0;
    BitsPerSample = 0;
    AudioFormat = 0;
    AudioLength = 0.f;

    // Loads the audio file into an array
    TArray<uint8> RawAudioData;
    auto pcd = FPaths::ProjectContentDir() + FilePath;
    if (!IFileManager::Get().FileExists(*pcd)) {
        pcd = FilePath;
    }


    if (FFileHelper::LoadFileToArray(RawAudioData, *pcd))
    {
        UE_LOG(LogTemp, Warning, TEXT("[APP] : File read succeeded"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[APP] : File read failed"));
    }

    // Extracts necessary data from the file
    FWaveModInfo WaveInfo;
    if (WaveInfo.ReadWaveInfo(RawAudioData.GetData(), RawAudioData.Num()))
    {
        // Gets audio file info
        AudioFormat = *WaveInfo.pFormatTag;
        BitsPerSample = *WaveInfo.pBitsPerSample;
        SampleRate = *WaveInfo.pSamplesPerSec;

        // The number of audio channels used by the file
        uint16 NumOfChannels = *WaveInfo.pChannels;

        // The size in bytes of the raw audio data in the file
        uint32 AudioDataSize = WaveInfo.SampleDataSize;
        
        // Checks the file is mono
        check(NumOfChannels == 1);
        
        // Sets the duration of the clip
        uint32 ByteRate = NumOfChannels * (BitsPerSample / 8) * SampleRate;
        check(ByteRate)
        AudioClip->Duration = *WaveInfo.pWaveDataSize / ByteRate;
        
        // Sets the sample rate, number of channels and data size of the sound wave asset
        AudioClip->SetSampleRate(SampleRate);
        AudioClip->NumChannels = NumOfChannels;
        AudioClip->RawPCMDataSize = AudioDataSize;

        // Copies the audio data to the sound wave asset
        AudioClip->RawPCMData = static_cast<uint8*>(FMemory::Malloc(AudioDataSize));
        FMemory::Memcpy(AudioClip->RawPCMData, WaveInfo.SampleDataStart, AudioDataSize);

        // Copies the audio data to the array which is to be input to SG Com
        AudioFileData = TArray<uint8>(WaveInfo.SampleDataStart, AudioDataSize);

        // Gets the length of the audio
        uint32 SizeOfSample = BitsPerSample / 8;
        uint32 NumSamples = AudioDataSize / SizeOfSample;
        AudioLength = (float)NumSamples / (float)SampleRate;
    }

    // Checks the audio clip is valid
    check(AudioClip);
}

// ========================================================
// Setup the SG_Com engine configuration
// ========================================================
TArray<uint8> CharacterFileData;
void ASGComUE4FileExampleGameModeBase::SetupEngineConfig(SG_COM_EngineConfig& EngineConfig)
{
    // Load character file
    FFileHelper::LoadFileToArray(CharacterFileData, *(CharacterFileDirectory + "Avatar.k"));
    UE_LOG(LogTemp, Warning, TEXT("[APP] : Character file size: %d"), CharacterFileData.Num());

    EngineConfig.character_file_in_memory = (sg_byte*)CharacterFileData.GetData();
    EngineConfig.character_file_bytes = CharacterFileData.Num();
    EngineConfig.audio_sample_type = MapSampleType(AudioFormat, BitsPerSample);
    EngineConfig.audio_sample_rate = MapSampleRate(SampleRate);
    EngineConfig.engine_broadcast_callback = nullptr;
    EngineConfig.engine_status_callback = &ASGComUE4FileExampleGameModeBase::EngineStatusCallback;
    EngineConfig.buffer_sec = 200;
    EngineConfig.flag = SG_COM_EngineConfigFlag::SG_COM_ENGINE_CONFIG_ENABLE_IDLE;
    EngineConfig.custom_engine_data = this;
}


// ========================================================
// Process audio data in the engine
// ========================================================
void ASGComUE4FileExampleGameModeBase::ProcessFrameWorker()
{
    double StartTime;
    double TimeTaken;
    float SleepTime;

    while (bProcessAudio) {
        StartTime = FPlatformTime::Seconds();
        int remaining_frames{ 0 };
        FSGComManager::ProcessAudio(&remaining_frames);
        TimeTaken = FPlatformTime::Seconds() - StartTime;
        
        if (TimeTaken < 0.01)
        {
            SleepTime = (float)(0.01 - TimeTaken);
            FPlatformProcess::Sleep(SleepTime);
        }

        if (remaining_frames == 0 && !fileQueue_.empty()) {
            //done = true;
            //LoadAudioFile("Resources\\Audio\\traci1.wav");
            //FSGComManager::InputAudio(AudioFileData);
            //UGameplayStatics::PlaySound2D(this, AudioClip);
            bProcessAudio = false;
        }
    }
    

}

// ========================================================
// Release resources
// ========================================================
void ASGComUE4FileExampleGameModeBase::EndSession()
{
    bProcessAudio = false;

    // Wait for threads to complete
    if (FrameFuture.IsValid()) {
        FrameFuture.Get();
    }

    // Destroy the transceiver
    FSGComManager::DestroyEngine();
}

// ========================================================
// Engine status callback
// ========================================================
void ASGComUE4FileExampleGameModeBase::EngineStatusCallback(SG_COM_EngineHandle Handle,
                                                            SG_COM_Status Status,
                                                            const char* Message,
                                                            void* CustomEngineData)
{
    if (Status == SG_COM_Status::SG_COM_STATUS_MOOD_CHANGED) {
        FString Mood{Message};
        UE_LOG(LogTemp, Warning, TEXT("[APP] : Mood changed to: %s"), *Mood);
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("[APP] : Other status change"));
    }
}


void ASGComUE4FileExampleGameModeBase::WatchKernelFolder(const FString& ProjectRelativeFolder) {
    std::string pstr = TCHAR_TO_UTF8(*ProjectRelativeFolder);
    using namespace std::experimental::filesystem;

    auto p = std::experimental::filesystem::path(pstr).native();
    auto p_exists = std::experimental::filesystem::exists(p);
    if (!p_exists) {
        return;
    }

    for (auto& f : std::experimental::filesystem::directory_iterator(p)) {
        const auto& ext = f.path().extension().string();
        if (ext != ".wav") {
            continue;
        }

        if (std::experimental::filesystem::is_regular_file(f)) {
            auto itr = filesDiscovered_.find(f);
            if (itr == filesDiscovered_.end()) {
                filesDiscovered_.emplace(f, false);
            }
        }
    }

    watchPaths_.push_back(pstr);

    
    //FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::Get().LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
    //IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get();

    //auto Changed = IDirectoryWatcher::FDirectoryChanged::CreateLambda([&](const TArray<FFileChangeData>& FileChanges) {

        //FTimespan Difference = FDateTime::Now() - LastWatchEventCall;
        //UE_LOG(LogTemp, Warning, TEXT("[APP] : FOUND A NEW FILE :)"));

        ////Rate limit file change callbacks
        //if (Difference.GetTotalSeconds() > WatchedNotificationLockout)
        //{
        //    FTimerHandle UniqueHandle;
        //    FTimerDelegate TimerCallback;
        //    TimerCallback.BindLambda([this, FileChanges]
        //        {
        //            for (auto Change : FileChanges)
        //            {
        //                FPaths::NormalizeFilename(Change.Filename);
        //                OnKernelSourceChanged.Broadcast(Change.Filename, (EKernelFileChangeAction)Change.Action);
        //            }
        //        });
        //    GetWorld()->GetTimerManager().SetTimer(UniqueHandle, TimerCallback, WatchedDiskReadDelay, false);
        //    LastWatchEventCall = FDateTime::Now();
        //}
    //});

    //if (IFileManager::Get().DirectoryExists(*ProjectRelativeFolder))
    //{
    //    FDelegateHandle DelegateHandle;
    //    DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(ProjectRelativeFolder, Changed, DelegateHandle, true);
    //}
}

