/*
 * alext.h : OpenAL extensions header definitions.
 *
 * (C) Copyright 2005-2013 by Erik Hofman
 * (C) Copyright 2007-2013 by Adalin B.V.
 *
 */

#ifndef __alext_h
#define __alext_h 1

#include <AL/al.h>
#include <AL/alcext.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
# define ALEXT_APIENTRY __cdecl
# ifdef ALC_BUILD_LIBRARY
#  define ALEXT_API __declspec(dllexport)
# else
#  define ALEXT_API __declspec(dllimport)
# endif
#else
# define ALEXT_APIENTRY
# if __GNUC__ >= 4
#  define ALEXT_API __attribute__((visibility("default")))
# else
#  define ALEXT_API extern
# endif
#endif

#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
# pragma export on
#endif

#ifndef AL_EXT_source_distance_model
#define AL_EXT_source_distance_model 1
#define AL_SOURCE_DISTANCE_MODEL		0x200
#endif

/* output channel formats */
#ifndef AL_EXT_MCFORMATS
#define AL_EXT_MCFORMATS 1
#define AL_FORMAT_QUAD8				0x1204
#define AL_FORMAT_QUAD16			0x1205
#define AL_FORMAT_QUAD32			0x1206
#define AL_FORMAT_REAR8				0x1207
#define AL_FORMAT_REAR16			0x1208
#define AL_FORMAT_REAR32			0x1209
#define AL_FORMAT_51CHN8			0x120A
#define AL_FORMAT_51CHN16			0x120B
#define AL_FORMAT_51CHN32			0x120C
#define AL_FORMAT_61CHN8			0x120D
#define AL_FORMAT_61CHN16			0x120E
#define AL_FORMAT_61CHN32			0x120F
#define AL_FORMAT_71CHN8			0x1210
#define AL_FORMAT_71CHN16			0x1211
#define AL_FORMAT_71CHN32			0x1212
#endif

#ifndef AL_EXT_MULAW_MCFORMATS
#define AL_EXT_MULAW_MCFORMATS 1
#define AL_FORMAT_MONO_MULAW			0x10014
#define AL_FORMAT_STEREO_MULAW			0x10015
#define AL_FORMAT_QUAD_MULAW			0x10021
#define AL_FORMAT_REAR_MULAW			0x10022
#define AL_FORMAT_51CHN_MULAW			0x10023
#define AL_FORMAT_61CHN_MULAW			0x10024
#define AL_FORMAT_71CHN_MULAW			0x10025
#endif

#ifndef AL_EXT_loop_points
#define AL_EXT_loop_points 1
#define AL_LOOP_POINTS				0x2015
#endif

/* buffer formats */
#ifndef AL_EXT_IMA4
#define AL_EXT_IMA4 1
#define AL_FORMAT_MONO_IMA4			0x1300
#define AL_FORMAT_STEREO_IMA4			0x1301
#endif

#ifndef AL_LOKI_IMA_ADPCM_format
#define AL_LOKI_IMA_ADPCM_format 1
#define AL_FORMAT_IMA_ADPCM_MONO16_EXT		0x10000
#define AL_FORMAT_IMA_ADPCM_STEREO16_EXT	0x10001
#endif

#ifndef AL_LOKI_WAVE_format
#define AL_LOKI_WAVE_format 1
#define AL_FORMAT_WAVE_EXT			0x10002
#endif

#ifndef AL_EXT_vorbis
#define AL_EXT_vorbis 1
#define AL_FORMAT_VORBIS_EXT			0x10003
#endif

/** four speaker setup */
#ifndef AL_LOKI_quadriphonic
#define AL_LOKI_quadriphonic 1
#define AL_FORMAT_QUAD8_LOKI			0x10004
#define AL_FORMAT_QUAD16_LOKI			0x10005
#endif

#ifndef AL_EXT_float32
#define AL_EXT_float32 1
#define AL_FORMAT_MONO_FLOAT32			0x10010
#define AL_FORMAT_STEREO_FLOAT32		0x10011
#endif

#ifndef AL_EXT_double
#define AL_EXT_double 1
#define AL_FORMAT_MONO_DOUBLE_EXT		0x10012
#define AL_FORMAT_STEREO_DOUBLE_EXT		0x10013
#endif

#ifndef AL_EXT_MULAW
#define AL_EXT_MULAW 1
#define AL_FORMAT_MONO_MULAW_EXT		0x10014
#define AL_FORMAT_STEREO_MULAW_EXT		0x10015
#endif

#ifndef AL_EXT_ALAW
#define AL_EXT_ALAW 1
#define AL_FORMAT_MONO_ALAW_EXT			0x10016
#define AL_FORMAT_STEREO_ALAW_EXT		0x10017
#endif

#ifndef AL_EXT_STATIC_BUFFER
#define AL_EXT_STATIC_BUFFER 1
typedef ALvoid (ALEXT_APIENTRY*PFNALBUFFERDATASTATICPROC)(const ALint,ALenum,ALvoid*,ALsizei,ALsizei);
# ifdef AL_ALEXT_PROTOTYPES
ALEXT_API ALvoid ALEXT_APIENTRY alBufferDataStatic(const ALint buffer, ALenum format, ALvoid *data, ALsizei len, ALsizei freq);
# endif
#endif

#ifndef AL_SOFT_buffer_sub_data
#define AL_SOFT_buffer_sub_data 1
#define AL_BYTE_RW_OFFSETS_SOFT			0x1031
#define AL_SAMPLE_RW_OFFSETS_SOFT		0x1032
typedef ALvoid (ALEXT_APIENTRY*PFNALBUFFERSUBDATASOFTPROC)(ALuint,ALenum,const ALvoid*,ALsizei,ALsizei);
# ifdef AL_ALEXT_PROTOTYPES
ALEXT_API ALvoid ALEXT_APIENTRY alBufferSubDataSOFT(ALuint buffer,ALenum format,const ALvoid *data,ALsizei offset,ALsizei length);
# endif
#endif

#ifndef AL_EXT_FOLDBACK
#define AL_EXT_FOLDBACK 1
#define AL_EXT_FOLDBACK_NAME			"AL_EXT_FOLDBACK"
#define AL_FOLDBACK_EVENT_BLOCK			0x4112
#define AL_FOLDBACK_EVENT_START			0x4111
#define AL_FOLDBACK_EVENT_STOP			0x4113
#define AL_FOLDBACK_MODE_MONO			0x4101
#define AL_FOLDBACK_MODE_STEREO			0x4102
typedef void (ALEXT_APIENTRY*LPALFOLDBACKCALLBACK)(ALenum,ALsizei);
typedef void (ALEXT_APIENTRY*LPALREQUESTFOLDBACKSTART)(ALenum,ALsizei,ALsizei,ALfloat*,LPALFOLDBACKCALLBACK);
typedef void (ALEXT_APIENTRY*LPALREQUESTFOLDBACKSTOP)(void);
#ifdef AL_ALEXT_PROTOTYPES
ALEXT_API void ALEXT_APIENTRY alRequestFoldbackStart(ALenum mode,ALsizei count,ALsizei length,ALfloat *mem,LPALFOLDBACKCALLBACK callback);
ALEXT_API void ALEXT_APIENTRY alRequestFoldbackStop(void);
#endif
#endif

#ifndef AL_SOFT_buffer_samples
#define AL_SOFT_buffer_samples 1
/* Channel configurations */
#define AL_MONO_SOFT				0x1500
#define AL_STEREO_SOFT				0x1501
#define AL_REAR_SOFT				0x1502
#define AL_QUAD_SOFT				0x1503
#define AL_5POINT1_SOFT				0x1504
#define AL_6POINT1_SOFT				0x1505
#define AL_7POINT1_SOFT				0x1506

/* Sample types */
#define AL_BYTE_SOFT				0x1400
#define AL_UNSIGNED_BYTE_SOFT			0x1401
#define AL_SHORT_SOFT				0x1402
#define AL_UNSIGNED_SHORT_SOFT			0x1403
#define AL_INT_SOFT				0x1404
#define AL_UNSIGNED_INT_SOFT			0x1405
#define AL_FLOAT_SOFT				0x1406
#define AL_DOUBLE_SOFT				0x1407
#define AL_BYTE3_SOFT				0x1408
#define AL_UNSIGNED_BYTE3_SOFT			0x1409

/* Storage formats */
#define AL_MONO8_SOFT				0x1100
#define AL_MONO16_SOFT				0x1101
#define AL_MONO32F_SOFT				0x10010
#define AL_STEREO8_SOFT				0x1102
#define AL_STEREO16_SOFT			0x1103
#define AL_STEREO32F_SOFT			0x10011
#define AL_QUAD8_SOFT				0x1204
#define AL_QUAD16_SOFT				0x1205
#define AL_QUAD32F_SOFT				0x1206
#define AL_REAR8_SOFT				0x1207
#define AL_REAR16_SOFT				0x1208
#define AL_REAR32F_SOFT				0x1209
#define AL_5POINT1_8_SOFT			0x120A
#define AL_5POINT1_16_SOFT			0x120B
#define AL_5POINT1_32F_SOFT			0x120C
#define AL_6POINT1_8_SOFT			0x120D
#define AL_6POINT1_16_SOFT			0x120E
#define AL_6POINT1_32F_SOFT			0x120F
#define AL_7POINT1_8_SOFT			0x1210
#define AL_7POINT1_16_SOFT			0x1211
#define AL_7POINT1_32F_SOFT			0x1212

/* Buffer attributes */
#define AL_INTERNAL_FORMAT_SOFT			0x2008
#define AL_BYTE_LENGTH_SOFT			0x2009
#define AL_SAMPLE_LENGTH_SOFT			0x200A
#define AL_SEC_LENGTH_SOFT			0x200B

typedef void (ALEXT_APIENTRY*LPALBUFFERSAMPLESSOFT)(ALuint,ALuint,ALenum,ALsizei,ALenum,ALenum,const ALvoid*);
typedef void (ALEXT_APIENTRY*LPALBUFFERSUBSAMPLESSOFT)(ALuint,ALsizei,ALsizei,ALenum,ALenum,const ALvoid*);
typedef void (ALEXT_APIENTRY*LPALGETBUFFERSAMPLESSOFT)(ALuint,ALsizei,ALsizei,ALenum,ALenum,ALvoid*);
typedef ALboolean (ALEXT_APIENTRY*LPALISBUFFERFORMATSUPPORTEDSOFT)(ALenum);
# ifdef AL_ALEXT_PROTOTYPES
ALEXT_API void ALEXT_APIENTRY alBufferSamplesSOFT(ALuint buffer, ALuint samplerate, ALenum internalformat, ALsizei samples, ALenum channels, ALenum type, const ALvoid *data);
ALEXT_API void ALEXT_APIENTRY alBufferSubSamplesSOFT(ALuint buffer, ALsizei offset, ALsizei samples, ALenum channels, ALenum type, const ALvoid *data);
ALEXT_API void ALEXT_APIENTRY alGetBufferSamplesSOFT(ALuint buffer, ALsizei offset, ALsizei samples, ALenum channels, ALenum type, ALvoid *data);
ALEXT_API ALboolean ALEXT_APIENTRY alIsBufferFormatSupportedSOFT(ALenum format);
# endif
#endif

#ifndef AL_SOFT_direct_channels
#define AL_SOFT_direct_channels 1
#define AL_DIRECT_CHANNELS_SOFT			0x1033
#endif

#ifndef AL_EXT_STEREO_ANGLES
#define AL_EXT_STEREO_ANGLES 1
#define AL_STEREO_ANGLES			0x1030
#endif

#ifndef AL_EXT_SOURCE_RADIUS
#define AL_EXT_SOURCE_RADIUS 1
#define AL_SOURCE_RADIUS			0x1031
#endif

#ifndef AL_AAX_frequency_filter
#define AL_AAX_frequency_filter 1
#define AL_FREQUENCY_FILTER_ENABLE_AAX		0x270000
#define AL_FREQUENCY_FILTER_GAINLF_AAX		0x270001
#define AL_FREQUENCY_FILTER_GAINHF_AAX		0x270002
#define AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX	0x270003
#define AL_FREQUENCY_FILTER_PARAMS_AAX		0x270004
#endif

#ifndef AL_AAX_reverb
#define AL_AAX_reverb 1
#define AL_REVERB_ENABLE_AAX			0x270008
#define AL_REVERB_PRE_DELAY_TIME_AAX		0x270009
#define AL_REVERB_REFLECTION_TIME_AAX		0x27000A
#define AL_REVERB_REFLECTION_FACTOR_AAX		0x27000B
#define AL_REVERB_DECAY_TIME_AAX		0x27000C
#define AL_REVERB_DECAY_TIME_HF_AAX		0x27000D

ALEXT_API void ALEXT_APIENTRY alReverbfAAX(ALenum attirb, ALfloat value);
ALEXT_API void ALEXT_APIENTRY alReverbvfAAX(ALenum attrib, ALfloat *values);
ALEXT_API void ALEXT_APIENTRY alReverbiAAX(ALenum attrib, ALint value);
#endif

#ifndef AL_AAX_distance_delay_model
#define AL_AAX_distance_delay_model 1
#define AL_DISTANCE_DELAY_MODEL_AAX		0x27D000
#define AL_INVERSE_DISTANCE_DELAY_AAX		0x27D001
#define AL_INVERSE_DISTANCE_DELAY_CLAMPED_AAX	0x27D002
#define AL_LINEAR_DISTANCE_DELAY_AAX		0x27D003
#define AL_LINEAR_DISTANCE_DELAY_CLAMPED_AAX	0x27D004
#define AL_EXPONENT_DISTANCE_DELAY_AAX		0x27D005
#define AL_EXPONENT_DISTANCE_DELAY_CLAMPED_AAX	0x27D006
#endif

#ifndef AL_AAX_environment
#define AL_AAX_environment 1
#define AL_SCENE_ENABLE_AAX			0x270010
#define AL_SCENE_DIMENSIONS_AAX			0x270011
#define AL_SCENE_CENTER_AAX			0x270012
#define AL_SCENE_LENGTH_AAX			0x270013
#define AL_SCENE_WIDTH_AAX			0x270014
#define AL_SCENE_HEIGHT_AAX			0x270015
#define AL_SCENE_REFLECTION_AAX			0x270016
#define AL_SCENE_DIFFUSE_AAX			0x270017
#define AL_WIND_SPEED_AAX			0x270018
#define AL_WIND_DIRECTION_AAX			0x270019

/* #define ALC_FORMAT				0x270020 */

ALEXT_API void ALEXT_APIENTRY alWindfAAX(ALenum attirb, ALfloat value);
ALEXT_API void ALEXT_APIENTRY alWind3fAAX(ALenum attirb, ALfloat v1, ALfloat v2, ALfloat v3);
ALEXT_API void ALEXT_APIENTRY alWindvfAAX(ALenum attirb, ALfloat *values);
#endif

#ifndef AL_EXT_source_latency
#define AL_EXT_source_latency 1
# define AL_SAMPLE_OFFSET_LATENCY		0x1200
# define AL_SEC_OFFSET_LATENCY			0x1201
typedef long long ALint64;
typedef unsigned long long ALuint64;
ALEXT_API void ALEXT_APIENTRY alSourced(ALuint source, ALenum param, ALdouble value);
ALEXT_API void ALEXT_APIENTRY alSource3d(ALuint source, ALenum param, ALdouble value1, ALdouble value2, ALdouble value3);
ALEXT_API void ALEXT_APIENTRY alSourcedv(ALuint source, ALenum param, const ALdouble *values);
ALEXT_API void ALEXT_APIENTRY alGetSourced(ALuint source, ALenum param, ALdouble *value);
ALEXT_API void ALEXT_APIENTRY alGetSource3d(ALuint source, ALenum param, ALdouble *value1, ALdouble *value2, ALdouble *value3);
ALEXT_API void ALEXT_APIENTRY alGetSourcedv(ALuint source, ALenum param, ALdouble *values);
ALEXT_API void ALEXT_APIENTRY alSourcei64(ALuint source, ALenum param, ALint64 value);
ALEXT_API void ALEXT_APIENTRY alSource3i64(ALuint source, ALenum param, ALint64 value1, ALint64 value2, ALint64 value3);
ALEXT_API void ALEXT_APIENTRY alSourcei64v(ALuint source, ALenum param, const ALint64 *values);
ALEXT_API void ALEXT_APIENTRY alGetSourcei64(ALuint source, ALenum param, ALint64 *value);
ALEXT_API void ALEXT_APIENTRY alGetSource3i64(ALuint source, ALenum param, ALint64 *value1, ALint64 *value2, ALint64 *value3);
ALEXT_API void ALEXT_APIENTRY alGetSourcei64v(ALuint source, ALenum param, ALint64 *values);
#endif

#ifndef AL_SOFT_source_latency
#define AL_SOFT_source_latency 1
# define AL_SAMPLE_OFFSET_LATENCY_SOFT		0x1200
# define AL_SEC_OFFSET_LATENCY_SOFT		0x1201
typedef ALint64 ALint64SOFT;
typedef ALuint64 ALuint64SOFT;
ALEXT_API void ALEXT_APIENTRY alSourcedSOFT(ALuint source, ALenum param, ALdouble value);
ALEXT_API void ALEXT_APIENTRY alSource3dSOFT(ALuint source, ALenum param, ALdouble value1, ALdouble value2, ALdouble value3);
ALEXT_API void ALEXT_APIENTRY alSourcedvSOFT(ALuint source, ALenum param, const ALdouble *values);
ALEXT_API void ALEXT_APIENTRY alGetSourcedSOFT(ALuint source, ALenum param, ALdouble *value);
ALEXT_API void ALEXT_APIENTRY alGetSource3dSOFT(ALuint source, ALenum param, ALdouble *value1, ALdouble *value2, ALdouble *value3);
ALEXT_API void ALEXT_APIENTRY alGetSourcedvSOFT(ALuint source, ALenum param, ALdouble *values);
ALEXT_API void ALEXT_APIENTRY alSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT value);
ALEXT_API void ALEXT_APIENTRY alSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT value1, ALint64SOFT value2, ALint64SOFT value3);
ALEXT_API void ALEXT_APIENTRY alSourcei64vSOFT(ALuint source, ALenum param, const ALint64SOFT *values);
ALEXT_API void ALEXT_APIENTRY alGetSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT *value);
ALEXT_API void ALEXT_APIENTRY alGetSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT *value1, ALint64SOFT *value2, ALint64SOFT *value3);
ALEXT_API void ALEXT_APIENTRY alGetSourcei64vSOFT(ALuint source, ALenum param, ALint64SOFT *values);
# ifdef AL_ALEXT_PROTOTYPES
ALEXT_API void ALEXT_APIENTRY alSourcedSOFT(ALuint source, ALenum param, ALdouble value);
ALEXT_API void ALEXT_APIENTRY alSource3dSOFT(ALuint source, ALenum param, ALdouble value1, ALdouble value2, ALdouble value3);
ALEXT_API void ALEXT_APIENTRY alSourcedvSOFT(ALuint source, ALenum param, const ALdouble *values);
ALEXT_API void ALEXT_APIENTRY alGetSourcedSOFT(ALuint source, ALenum param, ALdouble *value);
ALEXT_API void ALEXT_APIENTRY alGetSource3dSOFT(ALuint source, ALenum param, ALdouble *value1, ALdouble *value2, ALdouble *value3);
ALEXT_API void ALEXT_APIENTRY alGetSourcedvSOFT(ALuint source, ALenum param, ALdouble *values);
ALEXT_API void ALEXT_APIENTRY alSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT value);
ALEXT_API void ALEXT_APIENTRY alSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT value1, ALint64SOFT value2, ALint64SOFT value3);
ALEXT_API void ALEXT_APIENTRY alSourcei64vSOFT(ALuint source, ALenum param, const ALint64SOFT *values);
ALEXT_API void ALEXT_APIENTRY alGetSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT *value);
ALEXT_API void ALEXT_APIENTRY alGetSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT *value1, ALint64SOFT *value2, ALint64SOFT *value3);
ALEXT_API void ALEXT_APIENTRY alGetSourcei64vSOFT(ALuint source, ALenum param, ALint64SOFT *values);
# endif
#endif


#if defined(__cplusplus)
}
#endif

#endif /* !__alext_h */

