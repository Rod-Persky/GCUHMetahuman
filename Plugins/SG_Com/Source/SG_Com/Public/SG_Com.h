///
/// @file SG_Com.h
/// @date 2019/08/31
///
/// Copyright (c) 2015-2019 Speech Graphics Ltd. All rights reserved.
///

#ifndef SG_COM_H
#define SG_COM_H

#include "SG.h"

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// @brief Enum of error codes
    ///
    typedef enum {
        SG_COM_ERROR_OK, ///< No error.
        SG_COM_ERROR_LOW_MEMORY, ///< A low memory condition was detected.
        SG_COM_ERROR_INVALID_LICENSE, ///< The license is invalid.
        SG_COM_ERROR_INVALID_HANDLE, ///< An invalid Engine or Player was created or referenced.
        SG_COM_ERROR_INVALID_PACKET, ///< An invalid data packet was received.
        SG_COM_ERROR_INVALID_PARAM, ///< An invalid parameter was passed to a function.
        SG_COM_ERROR_TICK_IN_PROGRESS, /// Process tick was called while the previous tick was in progress.
        SG_COM_ERROR_OUT_OF_ORDER_PACKET_DISCARDED, ///< A data packet was received out of order and was discarded.
        SG_COM_ERROR_INPUT_OVERRUN, ///< The input buffer does not have sufficient free space to perform the requested operation.
        SG_COM_ERROR_INPUT_UNDERRUN, ///< The input buffer does not contain sufficient data to perform the requested operation.
        SG_COM_ERROR_EXCEPTION, ///<An exception was raised, call SG_COM_GetExceptionText() for details
        SG_COM_ERROR_NOT_IMPLEMENTED, ///< This operation is not implemented.
        SG_COM_ERROR_UNDEFINED ///< An unclassified error occurred.
    } SG_COM_Error;

    ///
    /// @brief Enum of status codes
    ///
    typedef enum {
        SG_COM_STATUS_MOOD_CHANGED ///< The current mood of the character changed, either manually or automatically.
    } SG_COM_Status;

    ///
    /// @brief Enum of Engine controls
    ///
    typedef enum SG_COM_EngineControl {
        SG_COM_CTRL_SCALE, ///< Scale factor of muscle motion. Default is 1.0.
        SG_COM_CTRL_SPEED, ///< Speed factor of muscle motion. Default is 1.0.
        SG_COM_CTRL_EXPRESSION_FREQ ///< Increase or decrease in frequency of expression change. Default is 1.0.
    } SG_COM_EngineControl;

    ///
    /// @brief Enum of Engine roles
    ///
    typedef enum SG_COM_EngineRole {
        SG_COM_ROLE_SPEAK, ///< Speaking role.
        SG_COM_ROLE_LISTEN ///< Listening role.
    } SG_COM_EngineRole;

    ///
    /// @brief Structure containing a Player configuration.
    ///
    typedef struct SG_COM_PlayerConfig {
        sg_byte* character_file_in_memory;
        sg_size character_file_bytes;
        SG_AnimationType animation_type; /**< The animation type of the output, SG_COM_NORMAL_ANIMATION or SG_COM_BAKED_ANIMATION.*/
        float buffer_sec; /**< The maximum duration (in seconds) that the output buffer can hold.*/
    } SG_COM_PlayerConfig;

    ///
    /// @brief Handle type
    ///
    typedef struct SG_COM_Player* SG_COM_PlayerHandle;

    ///
    /// @brief Handle type
    ///
    typedef struct SG_COM_Engine* SG_COM_EngineHandle;

    ///
    /// @brief Callback that will be called by an Engine to output packets.
    /// @param engine_handle The Engine that triggered the callback.
    /// @param packet The packet.
    /// @param packet_bytes Packet size in bytes.
    /// @param custom_engine_data A custom pointer that was provided to the Engine at construction.
    ///
    typedef void(*SG_COM_EngineBroadcastCallback) (
        SG_COM_EngineHandle engine_handle,
        char* packet,
        sg_size packet_bytes,
        void* custom_engine_data);

    ///
    /// @brief Callback that will be called in the event of Engine status update.
    /// @param engine_handle The Engine that triggered the callback.
    /// @param status The status code.
    /// @param msg The status message string.
    /// @param custom_engine_data A custom pointer that was provided to the Engine at construction.
    ///
    typedef void(*SG_COM_EngineStatusCallback) (
        SG_COM_EngineHandle engine_handle,
        SG_COM_Status status,
        const char *message,
        void* custom_engine_data);

    ///
    /// @brief Enum of Engine configuration flags
    ///
    typedef enum {
        SG_COM_ENGINE_CONFIG_NONE = 0,
        SG_COM_ENGINE_CONFIG_ENABLE_IDLE = 1,  ///< Enable idle mode.
        SG_COM_ENGINE_CONFIG_FIXED_RANDOM_SEED = 2  ///< Use a fixed seed in random number generators for deterministic output.
    } SG_COM_EngineConfigFlag;

    ///
    /// @brief Structure containing an Engine configuration.
    ///
    typedef struct SG_COM_EngineConfig {
        sg_byte* character_file_in_memory; /**< The character file loaded into a memory buffer.*/
        sg_size character_file_bytes; /**< The size of the loaded character file in bytes.*/
        SG_AudioSampleType audio_sample_type; /**< The input audio sample type.*/
        SG_AudioSampleRate audio_sample_rate; /**< The input audio sample rate.*/
        SG_COM_PlayerHandle local_player; /**< A handle to a Player object that can be used to create local animation output from this engine.*/
        SG_COM_EngineBroadcastCallback engine_broadcast_callback; /**< Callback to broadcast packets to remote Players (nullptr if not broadcasting).*/
        SG_COM_EngineStatusCallback engine_status_callback; /**< Callback for Engine status updates.*/
        float buffer_sec; /**< The maximum duration (in seconds) that the input buffer can hold.*/
        SG_COM_EngineConfigFlag flag; /**< Combination of engine configuration flags to enable/disable features.*/
        void *custom_engine_data; /**< Custom user data (or nullptr).*/
    } SG_COM_EngineConfig;

    ///
    /// @brief Retrieve exception text after SG_ERROR_EXCEPTION error.
    /// @return Pointer to null terminated string.
    ///
    SG_DYN const char *SG_COM_GetExceptionText(void);

    ///
    /// @brief Initialize the internal context of SG Com. This must be called
    /// once at startup before using any other functions in this API.
    /// @param log_level The logging level to set, from SG_LoggingLevel.
    /// @param logging_callback A callback that can be used for logging (pass nullptr if not used).
    /// @param license_data For floating license builds, directory of license file; otherwise license string.
    /// @param license_unique_id The unique id of the application for licensing purposes.
    /// @param license_custom_data Custom data passed to the license server - can be nullptr.
    /// @return Error code indicating success or reason for failure.
    ///
    /// The `license_unique_id` can null if any of the following are true:
    ///    - your license string (`license_data`) contains `license_type:KSO`  (not `RPP`, `RPT` or `EQU`).
    ///    - you are using a floating build of SG Com (it is shipped as `SG_Com-....dynamic.Release_floating.zip`).
    ///
    /// On Windows, OSX and Linux, if you do not provide a logging callback, SG Com will log to file.
    /// Log files will be written to following directories:
    /// - Windows  : `C:\Users\<User Name>\AppData\Local\Speech Graphics\logs`
    /// - OSX      : `/Users/<User Name>/Library/Application Support/Speech Graphics/logs`
    /// - Linux    : `~/.carnival/logs/`
    ///
    /// On all other platforms, SG_Com will not log to file. Instead, you must provide a callback to
    /// accept logging messages. Your callback is responsible for directing the message to a logging system
    /// such as the Android or iOS log, or perhaps your applications' own custom logger.
    ///

    SG_DYN SG_COM_Error SG_COM_Initialize(SG_LoggingLevel logging_level, SG_LoggingCallback logging_callback, const char* license_data, const char* license_unique_id, const char* license_custom_data);

    ///
    /// @brief Shut down and dispose of the internal context of SG Com.
    /// This must be the last function call to SG Com.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_Shutdown(void);

    ///
    /// @brief Get the runtime version number for this library in string format.
    /// @return String representation of the version of this library
    ///
    /// The format of the retured value is "major.minor.patch-label", for example
    /// 0.1.0-alpha.
    ///
    SG_DYN const char* SG_COM_GetVersionString(void);

    ///
    /// @brief Get the runtime version number for this library.
    /// @return The runtime version number for this library.
    ///
    /// The format of the returned value is vvvmmmppp where v is major version, m is
    /// minor version and p is patch version.
    ///
    SG_DYN unsigned int SG_COM_GetVersionNumber(void);

    ///
    /// @brief Create an Engine.
    /// @param engine_config Pointer to an SG_COM_EngineConfig structure.
    /// @param[out] engine_handle Output handle to the created Engine.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_CreateEngine(
        const SG_COM_EngineConfig *engine_config,
        SG_COM_EngineHandle *engine_handle
    );

    ///
    /// @brief Destroy an Engine.
    /// @param engine_handle The Engine to destroy.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_DestroyEngine(
        SG_COM_EngineHandle engine_handle
    );

    ///
    /// @brief Input audio data for processing.
    /// @param engine_handle The Engine to receive the input.
    /// @param data Pointer to the audio data.
    /// @param data_bytes The number of bytes of audio data.
    /// @return Error code indicating success or reason for failure.
    ///
    /// The audio data must be be in linear PCM format, mono, and
    /// must conform to the specifications supplied in the SG_EngineConfig struct
    /// that was used to create the Engine.
    ///
    SG_DYN SG_COM_Error SG_COM_InputAudio(
        SG_COM_EngineHandle engine_handle,
        const void* data,
        sg_size data_bytes
    );

	///
    /// @brief Input auxiliary data for processing.
    /// @param engine_handle The Engine to receive the input.
    /// @param data Pointer to the data.
    /// @param data_bytes The number of bytes of aux data.
    /// @return Error code indicating success or reason for failure.
    ///
    /// Auxiliary data is currently assumed to have a sample rate of 100 Hz.
    ///
    SG_DYN SG_COM_Error SG_COM_InputAuxData(
        SG_COM_EngineHandle engine_handle,
        const void* data,
        sg_size data_bytes
    );

    ///
    /// @brief Performs processing necessary to move one 10ms frame through the pipeline.
	/// If there is no input and idle is enabled, will generate idle animation.
    /// @param engine_handle The Engine on which to execute processing.
    /// @param[out] processed_frames The number of frames that were processed (0 or 1).
	/// @param[out] remaining_frames The number of frames that remain in the input buffer.
    /// @return Error code indicating success or reason for failure.
    ///
    /// This function is designed to be called on a background thread or threadpool, and in that case
    /// can return SG_COM_ERROR_TICK_IN_PROGRESS if the tick is called before the previous tick
    /// (on another thread) has had time to complete.
    ///
    SG_DYN SG_COM_Error SG_COM_ProcessTick(
        SG_COM_EngineHandle engine_handle,
        int *processed_frames,
        int *remaining_frames
    );

    ///
    /// @brief Clears an Engine and resets to initial state.
    /// @param engine_handle The Engine to reset.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_Reset(
        SG_COM_EngineHandle engine_handle
    );

    ///
    /// @brief Set the current mood of an Engine. The mood name should match one of the moods in the character setup.
	/// Alternatively, instead of naming a mood directly, use the special mood name "auto" to engage automatic mood detection.
	/// This requires three moods -- "neutral", "positive" and "negative" -- to be defined in the character setup.
    /// @param engine_handle The Engine.
    /// @param mood The string name of the mood, or "auto" for automatic mood detection.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_SetMood(
        SG_COM_EngineHandle engine_handle,
        const char* mood
    );

    ///
    /// @brief Get the current mood of an Engine.
    /// @param engine_handle The Engine.
    /// @param[out] mood User-allocated buffer into which the mood name is written.
    /// @param buffersize The size of the user-allocated buffer.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_GetMood(
        SG_COM_EngineHandle engine_handle,
        char* mood,
        sg_size buffersize
    );

    ///
    /// @brief Get a list of the available moods in an Engine.
    /// @param engine_handle The Engine.
    /// @param[out] mood_list User-allocated buffer into which a comma-separated mood list is written.
    /// @param buffersize The size of the user-allocated buffer.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_GetMoodList(
        SG_COM_EngineHandle engine_handle,
        char* mood_list,
        sg_size buffersize
    );

    ///
    /// @brief Set the role of an Engine vis-a-vis the incoming speech: to move as if speaking, or to move as if listening.
    /// @param engine_handle The Engine.
    /// @param role The role.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_SetRole(
        SG_COM_EngineHandle engine_handle,
        SG_COM_EngineRole role
    );

    ///
    /// @brief Get the current role of an Engine
    /// @param engine_handle The Engine.
    /// @param[out] role The role.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_GetRole(
        SG_COM_EngineHandle engine_handle,
        SG_COM_EngineRole *role
    );

    ///
    /// @brief Get the value of an Engine control.
    /// @param engine_handle The Engine.
    /// @param engine_control The control key.
    /// @param[out] value The returned control value.
    SG_DYN SG_COM_Error SG_COM_GetEngineControl(
        SG_COM_EngineHandle engine_handle,
        SG_COM_EngineControl engine_control,
        float *value
    );

    ///
    /// @brief Set the value of an Engine control.
    /// @param engine_handle The Engine.
    /// @param engine_control The control key.
    /// @param value The control value.
    SG_DYN SG_COM_Error SG_COM_SetEngineControl(
        SG_COM_EngineHandle engine_handle,
        SG_COM_EngineControl engine_control,
        float value
    );

    ///
    /// @brief Create a Player to play the output from a Broadcast Engine.
    /// @param player_config Pointer to an SG_COM_PlayerConfig structure.
    /// @param[out] player_handle Output handle to the created Player.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_CreatePlayer(
        const SG_COM_PlayerConfig *player_config,
        SG_COM_PlayerHandle *player_handle
    );

    ///
    /// @brief Destroy a Player.
    /// @param player_handle The Player to be destroyed.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_DestroyPlayer(
        SG_COM_PlayerHandle player_handle
    );

    ///
    /// @brief Receive an output packet from an Engine into the corresponding Player.
    /// @param player_handle The Player.
    /// @param packet The packet.
    /// @param packet_bytes The packet size in bytes.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_ReceivePacket(
        SG_COM_PlayerHandle player_handle,
        const char *packet,
        sg_size packet_bytes
    );

    ///
    /// @brief Get the animation nodes for a given Player.
    /// @param player_handle The Player.
    /// @param[out] animation_nodes An array of animation nodes managed by SG Com.
    /// @param[out] num_animation_nodes The number of animation nodes.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_GetAnimationNodes(
        SG_COM_PlayerHandle player_handle,
        SG_AnimationNode **animation_nodes,
        sg_size *num_animation_nodes
    );

    ///
    /// Set the current play time and update the animation nodes.
    /// The time value will be clamped to the playable range.
    /// @param player_handle The Player.
    /// @param time_ms The time (in milliseconds).
    /// @param current_time_ms[out] The new current time.
    /// If range clamping occurred, this will be different from the input time. May be nullptr.
    /// @return Error code.
    ///
    SG_DYN SG_COM_Error SG_COM_UpdateAnimation(
        SG_COM_PlayerHandle player_handle,
        double time_ms,
        double *current_time_ms
    );

    /// Get the range of buffered animation.
    /// @param player_handle The Player.
    /// @param min_time_ms[out] The minimum time of the playback window in milliseconds.
    /// @param max_time_ms[out] The maximum time of the playback window in millseconds.
    /// @return Error code indicating success or reason for failure.
    ///
    SG_DYN SG_COM_Error SG_COM_GetPlayableRange(
        SG_COM_PlayerHandle player_handle,
        double* min_time_ms,
        double* max_time_ms
    );

#ifdef __cplusplus
}
#endif

#endif // SG_ANIMATION_DLL_H
