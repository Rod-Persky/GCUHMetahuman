#ifndef SG_H
#define SG_H

#include <stdint.h>

/// Used to determine if we are building a shared library, as opposed
/// to building a static library or consuming the library
#ifdef BUILDING_DLL
/// Set the appropriate export declaration
#if defined(_WIN32) | defined(__ORBIS__) | defined(__PROSPERO__)
#define SG_DYN __declspec(dllexport)
#else
/// all GCC/Clang builds should use fvisibility=hidden
#define SG_DYN __attribute__ ((visibility ("default"))) 
#endif
#else
#define SG_DYN
#endif

typedef unsigned long int sg_uniqueid;
typedef unsigned int sg_size;
typedef unsigned char sg_byte;
typedef unsigned int sg_bool;

#define SG_ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#ifdef __cplusplus
extern "C" {
#endif

    ///
    /// @brief Enum of audio sample types
    ///
    typedef enum {
        SG_AUDIO_INT_16,    ///< 16 bit integer
        SG_AUDIO_INT_32,    ///< 32 bit integer
        SG_AUDIO_FLOAT_32,     ///< 32 bit float (IEEE 745 single precision binary32)
    } SG_AudioSampleType;

    ///
    /// @brief Enum of audio sample rates
    ///
    typedef enum {
        SG_AUDIO_8_KHZ,      ///< 8 kHz
        SG_AUDIO_12_KHZ,     ///< 12 kHz
        SG_AUDIO_16_KHZ,     ///< 16 kHz
        SG_AUDIO_24_KHZ,     ///< 24 kHz
        SG_AUDIO_32_KHZ,     ///< 32 kHz
        SG_AUDIO_44_1_KHZ,   ///< 44.1 kHz
        SG_AUDIO_48_KHZ      ///< 48 kHz
    } SG_AudioSampleRate;

    ///
    /// @brief Enum for logging level
    ///
    typedef enum {
        SG_LOGLEVEL_NONE,    ///< No logging enabled, default
        SG_LOGLEVEL_ERROR,   ///< Only errors will be logged
        SG_LOGLEVEL_DEBUG     ///< All information will be logged
    } SG_LoggingLevel;

    ///
    /// @brief Enum of animation types that can be output. SG_BAKED_ANIMATION requires rig logic to have been stored in
    /// the character setup (see SG Studio user guide).
    ///
    typedef enum {
        SG_NORMAL_ANIMATION,
        SG_BAKED_ANIMATION
    } SG_AnimationType;

    ///
    /// @brief Enum of animation node types
    ///
    typedef enum {
        SG_JOINT,
        SG_BLENDSHAPE,
        SG_OTHER_ANIMATION_NODE
    } SG_AnimationNodeType;

    ///
    /// @brief Structure containing an animation node.
    ///
    typedef struct SG_AnimationNode {
        const char *name; /**< Unique name of the node.*/
        SG_AnimationNodeType type; /**< Type of the node.*/
        sg_size num_channels; /**< Number of animation channels.*/
        const char **channel_names; /**< Names of the animation channels.*/
        float *channel_values; /**< Current values of the animation channels.*/
    } SG_AnimationNode;

    ///
    /// @brief Callback that will be called by an Engine to output log messages.
    /// @param message pointer to temporary null terminated string
    ///
    typedef void(*SG_LoggingCallback) (const char *message);

	///
    /// @brief Function to convert SG_AudioSampleRate into an integer.
	/// @param audio_sample_rate The SG_AudioSampleRate.
	/// @return The sample rate as an integer.
    ///
    static inline unsigned int get_audio_sample_rate( SG_AudioSampleRate audio_sample_rate ) {
        switch ( audio_sample_rate ) {
            case SG_AUDIO_8_KHZ:
                return 8000;
            case SG_AUDIO_12_KHZ:
                return 12000;
            case SG_AUDIO_16_KHZ:
                return 16000;
            case SG_AUDIO_24_KHZ:
                return 24000;
            case SG_AUDIO_32_KHZ:
                return 32000;
            case SG_AUDIO_44_1_KHZ:
                return 44100;
            case SG_AUDIO_48_KHZ:
                return 48000;
            default:
                return 16000;
        }
    }

#ifdef __cplusplus
}
#endif

#endif // SG_H
