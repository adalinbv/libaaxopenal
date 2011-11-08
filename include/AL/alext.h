/*
 * alext.h : OpenAL extensions header definitions.
 *
 * (C) Copyright 2005-2011 by Erik Hofman
 * (C) Copyright 2007-2011 by Adalin B.V.
 *
 */

#ifndef __alext_h
#define __alext_h 1

#include <AL/al.h>

#ifdef __cplusplus
extern "C" {
#endif

/** own extensions */

/** predefined extensions */

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
#define AL_AAX_FREQUENCY_FILTER_ENABLE			0x270000
#define AL_AAX_FREQUENCY_FILTER_GAINLF			0x270001
#define AL_AAX_FREQUENCY_FILTER_GAINHF			0x270002
#define AL_AAX_FREQUENCY_FILTER_CUTOFF_FREQUENCY	0x270003
#define AL_AAX_FREQUENCY_FILTER_PARAMS			0x270004
#endif

#ifndef AL_AAX_reverb
#define AL_AAX_reverb 1
#define AL_AAX_REVERB_ENABLE				0x270008
#define AL_AAX_REVERB_PRE_DELAY_TIME			0x270009
#define AL_AAX_REVERB_REFLECTION_TIME			0x27000A
#define AL_AAX_REVERB_REFLECTION_FACTOR			0x27000B
#define AL_AAX_REVERB_DECAY_TIME			0x27000C
#define AL_AAX_REVERB_DECAY_TIME_HF			0x27000D

extern void alReverbfAAX(ALenum attirb, ALfloat value);
extern void alReverbvfAAX(ALenum attrib, ALfloat *values);
extern void alReverbiAAX(ALenum attrib, ALint value);
#endif

#ifndef AL_AAX_environment
#define AL_AAX_environment 1
#define AL_AAX_SCENE_ENABLE				0x270010
#define AL_AAX_SCENE_DIMENSIONS				0x270011
#define AL_AAX_SCENE_CENTER				0x270012
#define AL_AAX_SCENE_LENGTH				0x270013
#define AL_AAX_SCENE_WIDTH				0x270014
#define AL_AAX_SCENE_HEIGHT				0x270015
#define AL_AAX_SCENE_REFLECTION				0x270016
#define AL_AAX_SCENE_DIFFUSE				0x270017
#define AL_AAX_WIND_SPEED				0x270018
#define AL_AAX_WIND_DIRECTION				0x270019
#define AL_AAX_DISTANCE_DELAY				0x27001A

extern void alWindfAAX(ALenum attirb, ALfloat value);
extern void alWind3fAAX(ALenum attirb, ALfloat v1, ALfloat v2, ALfloat v3);
extern void alWindvfAAX(ALenum attirb, ALfloat *values);
#endif

#ifndef AL_EXT_EFX
#define AL_EXT_EFX 1
#define ALC_EFX_MAJOR_VERSION			0x20001
#define ALC_EFX_MINOR_VERSION 			0x20002
#define LC_MAX_AUXILIARY_SENDS			0x20003
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


#ifdef __cplusplus
}
#endif

#endif /* !__alext_h */

