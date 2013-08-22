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

#ifndef AL_EXT_loop_points
#define AL_EXT_loop_points 1
#define AL_LOOP_POINTS				0x2015
#endif

/** four speaker setup */
#ifndef AL_LOKI_quadriphonic
#define AL_LOKI_quadriphonic 1
#define AL_FORMAT_QUAD8_LOKI			0x10004
#define AL_FORMAT_QUAD16_LOKI			0x10005
#endif

/* buffer formats */
#ifndef AL_EXT_IMA4
#define AL_EXT_IMA4 1
#define AL_FORMAT_MONO_IMA4			0x1300
#define AL_FORMAT_STEREO_IMA4			0x1301
#endif

#ifndef AL_LOKI_IMA_ADPCM_format
#define AL_LOKI_IMA_ADPCM_format 1
#define AL_FORMAT_IMA_ADPCM_MONO16_EXT          0x10000
#define AL_FORMAT_IMA_ADPCM_STEREO16_EXT        0x10001
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

#ifndef AL_EXT_mulaw
#define AL_EXT_mulaw 1
#define AL_FORMAT_MONO_MULAW_EXT		0x10014
#define AL_FORMAT_STEREO_MULAW_EXT		0x10015
#endif

#ifndef AL_EXT_alaw
#define AL_EXT_alaw 1
#define AL_FORMAT_MONO_ALAW_EXT			0x10016
#define AL_FORMAT_STEREO_ALAW_EXT		0x10017
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

#ifndef AL_AAX_source_distance_delay
#define AL_AAX_source_distance_delay 1
#define AL_DISTANCE_DELAY_AAX                   0x27001A
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

#ifndef AL_EXT_EFX
#define AL_EXT_EFX 1
#define AL_METERS_PER_UNIT			0x20004
#define AL_DIRECT_FILTER			0x20005
#define AL_AUXILIARY_SEND_FILTER		0x20006
#define AL_AIR_ABSORPTION_FACTOR		0x20007
#define AL_ROOM_ROLLOFF_FACTOR			0x20008
#define AL_CONE_OUTER_GAINHF			0x20009
#define AL_DIRECT_FILTER_GAINHF_AUTO		0x2000A
#define AL_AUXILIARY_SEND_FILTER_GAIN_AUTO	0x2000B
#define AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO	0x2000C

#define AL_EFFECT_TYPE				0x8001
#define AL_EFFECT_NULL				0x0000
#define AL_EFFECT_REVERB			0x0001
#define AL_EFFECT_CHORUS			0x0002
#define AL_EFFECT_DISTORTION			0x0003
#define AL_EFFECT_ECHO				0x0004
#define AL_EFFECT_FLANGER			0x0005
#define AL_EFFECT_FREQUENCY_SHIFTER		0x0006
#define AL_EFFECT_VOCAL_MORPHER			0x0007
#define AL_EFFECT_PITCH_SHIFTER			0x0008
#define AL_EFFECT_RING_MODULATOR		0x0009
#define AL_EFFECT_AUTOWAH			0x000A
#define AL_EFFECT_COMPRESSOR			0x000B
#define AL_EFFECT_EQUALIZER			0x000C

#define AL_REVERB_DENSITY			0x0001
#define AL_REVERB_DIFFUSION			0x0002
#define AL_REVERB_GAIN				0x0003
#define AL_REVERB_GAINHF			0x0004
#define AL_REVERB_DECAY_TIME			0x0005
#define AL_REVERB_DECAY_HFRATIO			0x0006
#define AL_REVERB_REFLECTIONS_GAIN		0x0007
#define AL_REVERB_REFLECTIONS_DELAY		0x0008
#define AL_REVERB_LATE_REVERB_GAIN		0x0009
#define AL_REVERB_LATE_REVERB_DELAY		0x000A
#define AL_REVERB_AIR_ABSORPTION_GAINHF		0x000B
#define AL_REVERB_ROOM_ROLLOFF_FACTOR		0x000C
#define AL_REVERB_DECAY_HFLIMIT			0x000D

#define AL_FILTER_TYPE				0x8001
#define AL_FILTER_NULL				0x0000
#define AL_FILTER_LOWPASS			0x0001
#define AL_FILTER_HIGHPASS			0x0002
#define AL_FILTER_BANDPASS			0x0003
#define AL_LOWPASS_GAIN				0x0001
#define AL_LOWPASS_GAINHF			0x0002

#define AL_EFFECTSLOT_EFFECT			0x0001
#define AL_EFFECTSLOT_GAIN			0x0002
#define AL_EFFECTSLOT_AUXILIARY_SEND_AUTO	0x0003
#define AL_EFFECTSLOT_NULL			0x0000

extern void alGenFilters(ALsizei num, ALuint *ids);
extern void alDeleteFilters(ALsizei num, ALuint *ids);
extern ALboolean alIsFilter(ALuint id);

extern void alFilteri(ALuint id, ALenum attrib, ALint value);
extern void alFilteriv(ALuint id, ALenum attrib, ALint *values);
extern void alFilterf(ALuint id, ALenum attrib, ALfloat value);
extern void alFilterfv(ALuint id, ALenum attrib, ALfloat *values);

extern void alGetFilteri(ALuint id, ALenum attrib, ALint *values);
extern void alGetFilteriv(ALuint id, ALenum attrib, ALint *values);
extern void alGetFilterf(ALuint id, ALenum attrib, ALfloat *values);
extern void alGetFilterfv(ALuint id, ALenum attrib, ALfloat *values);
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

#endif


#if defined(__cplusplus)
}
#endif

#endif /* !__alext_h */

