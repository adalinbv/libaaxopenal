/*
 * Copyright (C) 2007-2013 by Erik Hofman.
 * Copyright (C) 2007-2013 by Adalin B.V.
 *
 * This file is part of AeonWave-OpenAL.
 *
 *  AeonWave-OpenAL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AeonWave-OpenAL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AeonWave-OpenAL.  If not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_ASSERT_H
#include <math.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#include <assert.h>

#include <aax.h>
#include <aaxdefs.h> 

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <base/dlsym.h>
#include <base/types.h>

#include "api.h"
#include "aax_support.h"

/* forward declarations */
const char *_oalStateErrorStrings[];
static const char* _oalExtensions[];
static const _oalEnumValue_s _oalEnumValues[];

static ALfloat _oalGetDopplerFactor();
static void _oalSetDopplerFactor(ALfloat f);
static ALfloat _oalGetDopplerVelocity();
static void _oalSetDopplerVelocity(ALfloat f);
static ALfloat _oalGetSoundVelocity();
static void _oalSetSoundVelocity(ALfloat f);
static ALenum _oalGetDistanceModel(ALboolean b);
static void _oalSetDistanceModel(ALenum e);

ALenum alGetError(void)
{
    return _oalStateSetError(AL_NO_ERROR);
}

void alEnable(ALenum attrib)
{
    _intBufferData *dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx = _intBufGetDataPtr(dptr);
        _oalState *cs = ctx->state;
        switch(attrib)
        {
        case AL_SOURCE_DISTANCE_MODEL:
            cs->src_dist_model = AL_TRUE;
            break;
        case AL_DISTANCE_DELAY_AAX:
            cs->distance_delay = AL_TRUE;
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
            break;
        }
    }
}

void alDisable(ALenum attrib)
{
    _intBufferData *dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx  =_intBufGetDataPtr(dptr);
        _oalState *cs = ctx->state;
        switch(attrib)
        {
        case AL_SOURCE_DISTANCE_MODEL:
            cs->src_dist_model = AL_FALSE;
            break;
        case AL_DISTANCE_DELAY_AAX:
            cs->distance_delay = AL_FALSE;
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
            break;
        }
    }
}

ALboolean
alIsEnabled (ALenum attrib)
{
    ALboolean rv = AL_FALSE;
    _intBufferData *dptr;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx = _intBufGetDataPtr(dptr);
        _oalState *cs = ctx->state;
        switch(attrib)
        {
        case AL_SOURCE_DISTANCE_MODEL:
            rv = cs->src_dist_model;
            break;
        case AL_DISTANCE_DELAY_AAX:
            rv = cs->distance_delay;
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
            break;
        }
    }
    return rv;
}

const ALchar *
alGetString(ALenum attrib)
{
    _intBufferData *dptr;
    _oalDevice *dev;
    ALchar *retstr;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    retstr = (ALchar *)"Invalid parameter in alGetString.";

    dptr = _oalGetCurrentDevice();
    if (dptr)
    {
        dev = _intBufGetDataPtr(dptr);
        assert(dev);

        retstr = (ALchar *)"";
        switch (attrib)
        {
        case AL_NO_ERROR:
            retstr = (ALchar *)"There is no current error.";
            break;
        case AL_INVALID_NAME:
        case AL_INVALID_ENUM:
        case AL_INVALID_VALUE:
        case AL_INVALID_OPERATION:
        case AL_OUT_OF_MEMORY:
            retstr = (ALchar *)_oalStateErrorStrings[attrib - AL_INVALID_NAME];
            break;
        case AL_EXTENSIONS:
            retstr = (ALchar *)_oalAAXGetExtensions(_oalExtensions);
            break;
        case AL_VERSION:
            retstr = (ALchar *)_oalAAXGetVersionString(dev->lst.handle);
            break;
        case AL_RENDERER:
            retstr = (ALchar *)_oalAAXGetRendererString(dev->lst.handle);
            break;
        case AL_VENDOR:
            retstr = (ALchar *)aaxDriverGetVendor(dev->lst.handle);
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }

    return retstr;
}

void
alDistanceModel(ALenum attrib)
{
    if (attrib == AL_NONE ||
        (attrib >= AL_INVERSE_DISTANCE && attrib <= AL_EXPONENT_DISTANCE_CLAMPED))
    {
        _oalSetDistanceModel(attrib);
    } else {
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alDopplerFactor(ALfloat factor)
{
    if (factor >= 0.0f) {
        _oalSetDopplerFactor(factor);
    } else {
        _oalStateSetError(AL_INVALID_VALUE);
    }
}

void
alSpeedOfSound(ALfloat speed)
{
    if (speed >= 0.0f) {
        _oalSetSoundVelocity(speed);
    } else {
        _oalStateSetError(AL_INVALID_VALUE);
    }
}

/*
 * Need to keep this function for version 1.0 backwards compatibility
 */
void
alDopplerVelocity(ALfloat velocity)
{
    if (velocity >= 0.0f) {
        _oalSetDopplerVelocity(velocity);
    } else {
        _oalStateSetError(AL_INVALID_VALUE);
    }
}

ALboolean
alIsExtensionPresent(const ALchar *name)
{
    ALCboolean r = ALC_FALSE;
    unsigned int i;
    const char *e;

    for (i=0; (e = _oalExtensions[i]) != NULL; i++)
    {
        r = (!strcasecmp((const char *)name, e));
        if (r) break;
    }

    if (!r) {
        r = _oalAAXGetExtensionSupport((const char*)name);
    }

    return r;
}

void *
alGetProcAddress(const ALchar *name)
{
    return _oalGetGlobalProcAddress((const char *)name);
}

ALenum
alGetEnumValue(const ALchar* name)
{
    const _oalEnumValue_s *e;
    ALenum rv = AL_FALSE;
    unsigned int i;

    if (!name)
    {
        _oalContextSetError(ALC_INVALID_VALUE);
        return ALC_FALSE;
    }

    for (i=0; ((e = &_oalEnumValues[i]) != NULL) && e->name; i++)
    {
        if (!strcasecmp((const char *)name, e->name))
        {
            rv = e->enumVal;
            break;
        }
    }

    return rv;
}

void
alHint(ALenum target, ALenum mode)
{
    switch(target)
    {
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

/*
 * void alGetBooleanv(ALenum attrib, ALboolean *value)
 * ALboolean alGetBoolean(ALenum attrib)
 */
#define N Boolean
#define T ALboolean
#include "alState_template.c"

/*
 * void alGetIntegerv(ALenum attrib, ALint *value)
 * ALint alGetInteger(ALenum attrib)
 */
#define N Integer
#define T ALint
#include "alState_template.c"

/*
 * void alGetFloatv(ALenum attrib, ALfloat *value)
 * ALfloat alGetFloat(ALenum attrib)
 */
#define N Float
#define T ALfloat
#include "alState_template.c"

/*
 * void alGetDoublev(ALenum attrib, ALdouble *value)
 * ALdouble alGetDouble(ALenum attrib)
 */
#define N Double
#define T ALdouble
#include "alState_template.c"


/* -------------------------------------------------------------------------- */

/**
 * extensions
 */
static const char* _oalExtensions[] =
{
  "AL_EXT_exponent_distance",
  "AL_EXT_linear_distance",
  "AL_EXT_float32",
  "AL_EXT_double",
  "AL_EXT_mulaw",
  "AL_EXT_alaw",
  "AL_EXT_ima4",
  "AL_EXT_offset",
  "AL_EXT_mcformats",
  "AL_EXT_loop_points",
  "AL_EXT_source_distance_model",
  "AL_SOFT_source_latency",

  NULL					/* always last */
};

/**
 * Enum
 */
static const _oalEnumValue_s _oalEnumValues[] =
{
  {"AL_FALSE",				AL_FALSE},
  {"AL_TRUE",				AL_TRUE},
  {"AL_NO_ERROR",			AL_NO_ERROR},
  {"AL_INVALID_NAME",			AL_INVALID_NAME},
  {"AL_INVALID_ENUM",			AL_INVALID_ENUM},
  {"AL_INVALID_VALUE",			AL_INVALID_VALUE},
  {"AL_INVALID_OPERATION",		AL_INVALID_OPERATION},
  {"AL_OUT_OF_MEMORY",			AL_OUT_OF_MEMORY},
  {"AL_VENDOR",				AL_VENDOR},
  {"AL_VERSION",			AL_VERSION},
  {"AL_RENDERER",			AL_RENDERER},
  {"AL_EXTENSIONS",			AL_EXTENSIONS},
  {"AL_FORMAT_MONO8",			AL_FORMAT_MONO8},
  {"AL_FORMAT_MONO16",			AL_FORMAT_MONO16},
  {"AL_FORMAT_STEREO8",			AL_FORMAT_STEREO8},
  {"AL_FORMAT_STEREO16",		AL_FORMAT_STEREO16},
  {"AL_DOPPLER_FACTOR",			AL_DOPPLER_FACTOR},
#ifdef AL_VERSION_1_0
  {"AL_DOPPLER_VELOCITY",		AL_DOPPLER_VELOCITY},
#endif
  {"AL_SPEED_OF_SOUND",			AL_SPEED_OF_SOUND},
  {"AL_DISTANCE_MODEL",			AL_DISTANCE_MODEL},
  {"AL_INVERSE_DISTANCE",		AL_INVERSE_DISTANCE},
  {"AL_INVERSE_DISTANCE_CLAMPED",	AL_INVERSE_DISTANCE_CLAMPED},
  {"AL_LINEAR_DISTANCE",		AL_LINEAR_DISTANCE},
  {"AL_LINEAR_DISTANCE_CLAMPED",	AL_LINEAR_DISTANCE_CLAMPED},
  {"AL_EXPONENT_DISTANCE",		AL_EXPONENT_DISTANCE},
  {"AL_EXPONENT_DISTANCE_CLAMPED",	AL_EXPONENT_DISTANCE_CLAMPED},

  {"AL_SOURCE_RELATIVE",		AL_SOURCE_RELATIVE},
  {"AL_CONE_INNER_ANGLE",		AL_CONE_INNER_ANGLE},
  {"AL_CONE_OUTER_ANGLE",		AL_CONE_OUTER_ANGLE},
  {"AL_PITCH",				AL_PITCH},
  {"AL_POSITION",			AL_POSITION},
  {"AL_DIRECTION",			AL_DIRECTION},
  {"AL_VELOCITY",			AL_VELOCITY},
  {"AL_LOOPING",			AL_LOOPING},
  {"AL_BUFFER",				AL_BUFFER},
  {"AL_GAIN",				AL_GAIN},
  {"AL_MIN_GAIN",			AL_MIN_GAIN},
  {"AL_MAX_GAIN",			AL_MAX_GAIN},
  {"AL_ORIENTATION",			AL_ORIENTATION},
  {"AL_SOURCE_STATE",			AL_SOURCE_STATE},
  {"AL_INITIAL",			AL_INITIAL},
  {"AL_PLAYING",			AL_PLAYING},
  {"AL_PAUSED",				AL_PAUSED},
  {"AL_STOPPED",			AL_STOPPED},
  {"AL_BUFFERS_QUEUED",			AL_BUFFERS_QUEUED},
  {"AL_BUFFERS_PROCESSED",		AL_BUFFERS_PROCESSED},
  {"AL_REFERENCE_DISTANCE",		AL_REFERENCE_DISTANCE},
  {"AL_ROLLOFF_FACTOR",			AL_ROLLOFF_FACTOR},
  {"AL_CONE_OUTER_GAIN",		AL_CONE_OUTER_GAIN},
  {"AL_MAX_DISTANCE",			AL_MAX_DISTANCE},
  {"AL_SEC_OFFSET",			AL_SEC_OFFSET},
  {"AL_SAMPLE_OFFSET",			AL_SAMPLE_OFFSET},
  {"AL_BYTE_OFFSET",			AL_BYTE_OFFSET},
  {"AL_SOURCE_TYPE",			AL_SOURCE_TYPE},
  {"AL_STATIC",				AL_STATIC},
  {"AL_STREAMING",			AL_STREAMING},
  {"AL_UNDETERMINED",			AL_UNDETERMINED},
  {"AL_CHANNEL_MASK",			AL_CHANNEL_MASK},
  {"AL_FREQUENCY",			AL_FREQUENCY},
  {"AL_BITS",				AL_BITS},
  {"AL_CHANNELS",			AL_CHANNELS},
  {"AL_SIZE",				AL_SIZE},
  {"AL_UNUSED",				AL_UNUSED},
  {"AL_PENDING",			AL_PENDING},
  {"AL_PROCESSED",			AL_PROCESSED},

  {"AL_FORMAT_MONO_IMA4",		AL_FORMAT_MONO_IMA4},
  {"AL_FORMAT_STEREO_IMA4",		AL_FORMAT_STEREO_IMA4},

/* Extensions */
  {"AL_SOURCE_DISTANCE_MODEL",		AL_SOURCE_DISTANCE_MODEL},
  {"AL_FORMAT_QUAD8",			AL_FORMAT_QUAD8},
  {"AL_FORMAT_QUAD16",			AL_FORMAT_QUAD16},
  {"AL_FORMAT_QUAD32",			AL_FORMAT_QUAD32},
  {"AL_FORMAT_REAR8",			AL_FORMAT_REAR8},
  {"AL_FORMAT_REAR16",			AL_FORMAT_REAR16},
  {"AL_FORMAT_REAR32",			AL_FORMAT_REAR32},
  {"AL_FORMAT_51CHN8",			AL_FORMAT_51CHN8},
  {"AL_FORMAT_51CHN16",			AL_FORMAT_51CHN16},
  {"AL_FORMAT_51CHN32",			AL_FORMAT_51CHN32},
  {"AL_FORMAT_61CHN8",			AL_FORMAT_61CHN8},
  {"AL_FORMAT_61CHN16",			AL_FORMAT_61CHN16},
  {"AL_FORMAT_61CHN32",			AL_FORMAT_61CHN32},
  {"AL_FORMAT_71CHN8",			AL_FORMAT_71CHN8},
  {"AL_FORMAT_71CHN16",			AL_FORMAT_71CHN16},
  {"AL_FORMAT_71CHN32",			AL_FORMAT_71CHN32},

  {"AL_LOOP_POINTS",			AL_LOOP_POINTS},
  {"AL_FORMAT_QUAD8_LOKI",		AL_FORMAT_QUAD8_LOKI},
  {"AL_FORMAT_QUAD16_LOKI",		AL_FORMAT_QUAD16_LOKI},
  {"AL_FORMAT_MONO_IMA4",		AL_FORMAT_MONO_IMA4},
  {"AL_FORMAT_STEREO_IMA4",		AL_FORMAT_STEREO_IMA4},
  {"AL_FORMAT_IMA_ADPCM_MONO16_EXT",	AL_FORMAT_IMA_ADPCM_MONO16_EXT},
  {"AL_FORMAT_IMA_ADPCM_STEREO16_EXT",	AL_FORMAT_IMA_ADPCM_STEREO16_EXT},
  {"AL_FORMAT_MONO_FLOAT32",		AL_FORMAT_MONO_FLOAT32},
  {"AL_FORMAT_STEREO_FLOAT32",		AL_FORMAT_STEREO_FLOAT32},
  {"AL_FORMAT_MONO_DOUBLE_EXT",		AL_FORMAT_MONO_DOUBLE_EXT},
  {"AL_FORMAT_STEREO_DOUBLE_EXT",	AL_FORMAT_STEREO_DOUBLE_EXT},
  {"AL_FORMAT_MONO_MULAW_EXT",		AL_FORMAT_MONO_MULAW_EXT},
  {"AL_FORMAT_STEREO_MULAW_EXT",	AL_FORMAT_STEREO_MULAW_EXT},
  {"AL_FORMAT_MONO_ALAW_EXT",		AL_FORMAT_MONO_ALAW_EXT},
  {"AL_FORMAT_STEREO_ALAW_EXT",		AL_FORMAT_STEREO_ALAW_EXT},
  /* AL_AAX_frequency_filter */
  {"AL_FREQUENCY_FILTER_ENABLE_AAX",	AL_FREQUENCY_FILTER_ENABLE_AAX},
  {"AL_FREQUENCY_FILTER_GAINLF_AAX",	AL_FREQUENCY_FILTER_GAINLF_AAX},  // 100
  {"AL_FREQUENCY_FILTER_GAINHF_AAX",	AL_FREQUENCY_FILTER_GAINHF_AAX},
  {"AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX",AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX},
  {"AL_FREQUENCY_FILTER_PARAMS_AAX",	AL_FREQUENCY_FILTER_PARAMS_AAX},
  /* AL_AAX_reverb */
  {"AL_REVERB_ENABLE_AAX",		AL_REVERB_ENABLE_AAX},
  {"AL_REVERB_PRE_DELAY_TIME_AAX",	AL_REVERB_PRE_DELAY_TIME_AAX},
  {"AL_REVERB_REFLECTION_TIME_AAX",	AL_REVERB_REFLECTION_TIME_AAX},
  {"AL_REVERB_REFLECTION_FACTOR_AAX",	AL_REVERB_REFLECTION_FACTOR_AAX},
  {"AL_REVERB_DECAY_TIME_AAX",		AL_REVERB_DECAY_TIME_AAX},
  {"AL_REVERB_DECAY_TIME_HF_AAX",	AL_REVERB_DECAY_TIME_HF_AAX},
  /* Source EFX */
  {"AL_DIRECT_FILTER",			AL_DIRECT_FILTER},
  {"AL_AUXILIARY_SEND_FILTER",		AL_AUXILIARY_SEND_FILTER},
  {"AL_AIR_ABSORPTION_FACTOR",		AL_AIR_ABSORPTION_FACTOR},
  {"AL_ROOM_ROLLOFF_FACTOR",		AL_ROOM_ROLLOFF_FACTOR},
  {"AL_CONE_OUTER_GAINHF",		AL_CONE_OUTER_GAINHF},
  {"AL_DIRECT_FILTER_GAINHF_AUTO",	AL_DIRECT_FILTER_GAINHF_AUTO},
  {"AL_AUXILIARY_SEND_FILTER_GAIN_AUTO",AL_AUXILIARY_SEND_FILTER_GAIN_AUTO},
  {"AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO",AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO},
  {"AL_EFFECT_TYPE",			AL_EFFECT_TYPE},
  {"AL_EFFECT_NULL",			AL_EFFECT_NULL},
  {"AL_EFFECT_REVERB",			AL_EFFECT_REVERB},
  {"AL_EFFECT_CHORUS",			AL_EFFECT_CHORUS},
  {"AL_EFFECT_DISTORTION",		AL_EFFECT_DISTORTION},
  {"AL_EFFECT_ECHO",			AL_EFFECT_ECHO},
  {"AL_EFFECT_FLANGER",			AL_EFFECT_FLANGER},
  {"AL_EFFECT_FREQUENCY_SHIFTER",	AL_EFFECT_FREQUENCY_SHIFTER},
  {"AL_EFFECT_VOCAL_MORPHER",		AL_EFFECT_VOCAL_MORPHER},
  {"AL_EFFECT_PITCH_SHIFTER",		AL_EFFECT_PITCH_SHIFTER},
  {"AL_EFFECT_RING_MODULATOR",		AL_EFFECT_RING_MODULATOR},
  {"AL_EFFECT_AUTOWAH",			AL_EFFECT_AUTOWAH},
  {"AL_EFFECT_COMPRESSOR",		AL_EFFECT_COMPRESSOR},
  {"AL_EFFECT_EQUALIZER",		AL_EFFECT_EQUALIZER},
  {"AL_REVERB_DENSITY",			AL_REVERB_DENSITY},
  {"AL_REVERB_DIFFUSION",		AL_REVERB_DIFFUSION},
  {"AL_REVERB_GAIN",			AL_REVERB_GAIN},
  {"AL_REVERB_GAINHF",			AL_REVERB_GAINHF},
  {"AL_REVERB_DECAY_TIME",		AL_REVERB_DECAY_TIME},
  {"AL_REVERB_DECAY_HFRATIO",		AL_REVERB_DECAY_HFRATIO},
  {"AL_REVERB_REFLECTIONS_GAIN",	AL_REVERB_REFLECTIONS_GAIN},
  {"AL_REVERB_REFLECTIONS_DELAY",	AL_REVERB_REFLECTIONS_DELAY},
  {"AL_REVERB_LATE_REVERB_GAIN",	AL_REVERB_LATE_REVERB_GAIN},
  {"AL_REVERB_LATE_REVERB_DELAY",	AL_REVERB_LATE_REVERB_DELAY},
  {"AL_REVERB_AIR_ABSORPTION_GAINHF",	AL_REVERB_AIR_ABSORPTION_GAINHF},
  {"AL_REVERB_ROOM_ROLLOFF_FACTOR",	AL_REVERB_ROOM_ROLLOFF_FACTOR},
  {"AL_REVERB_DECAY_HFLIMIT",		AL_REVERB_DECAY_HFLIMIT},
  {"AL_FILTER_TYPE",			AL_FILTER_TYPE},
  {"AL_FILTER_NULL",			AL_FILTER_NULL},
  {"AL_FILTER_LOWPASS",			AL_FILTER_LOWPASS},
  {"AL_FILTER_HIGHPASS",		AL_FILTER_HIGHPASS},
  {"AL_FILTER_BANDPASS",		AL_FILTER_BANDPASS},
  {"AL_LOWPASS_GAIN",			AL_LOWPASS_GAIN},
  {"AL_LOWPASS_GAINHF",			AL_LOWPASS_GAINHF},
  {"AL_EFFECTSLOT_EFFECT",		AL_EFFECTSLOT_EFFECT},
  {"AL_EFFECTSLOT_GAIN",		AL_EFFECTSLOT_GAIN},
  {"AL_EFFECTSLOT_AUXILIARY_SEND_AUTO",	AL_EFFECTSLOT_AUXILIARY_SEND_AUTO},
  {"AL_EFFECTSLOT_NULL",		AL_EFFECTSLOT_NULL},

  {NULL, 0}				/* always last */
};

const char *_oalStateErrorStrings[] =
{
    "Invalid name parameter.",
    "Invalid enum parameter value.",
    "Invalid source id.",
    "Illegal call.",
    "Unable to allocate memory.",
    0
};

void
_oalStateCreate(void *context)
{
    _oalContext *ctx = (_oalContext *)context;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    assert(ctx);
    assert(ctx);

    if (!ctx->state)
    {
        _oalState *cs = (_oalState *)malloc(sizeof(_oalState));
        if (cs)
        {
            cs->error = AL_NONE;
            cs->maxDistance = 1.0f;
            cs->dopplerFactor = 1.0f;
            cs->dopplerVelocity = 1.0f;
            cs->soundVelocity = 343.3f;
            cs->distanceModel = AL_INVERSE_DISTANCE_CLAMPED;
            cs->src_dist_model = AL_FALSE;
            ctx->state = cs;
        }
        else
        {
            _oalContextSetError(ALC_OUT_OF_MEMORY);
        }
    }
}

ALenum
__oalStateSetErrorNormal(ALenum error)
{
    _intBufferData *dptr;
    ALenum ret = AL_INVALID_OPERATION;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);

        cs = ctx->state;
        ret = cs->error;
        cs->error = error;
    }

    return ret;
}

#ifndef NDEBUG
ALenum
__oalStateSetErrorReport(ALenum error, char *file, int line)
{
    ALenum ret = __oalStateSetErrorNormal(error);
    if (error != AL_NO_ERROR) printf("at %d in file %s:\n", line, file);
    return ret;
}
#endif

ALfloat
_oalGetDopplerFactor()
{
    _intBufferData *dptr;
    ALfloat ret = 0.0f;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);

        cs = ctx->state;
        ret = cs->dopplerFactor;
    }

    return ret;
}

void
_oalSetDopplerFactor(ALfloat f)
{
    _intBufferData *dptr;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        aaxConfig handle;
        _oalContext *ctx;
        _oalDevice *dev;
        _oalState *cs;
 
        ctx = _intBufGetDataPtr(dptr);
        dev = (_oalDevice *)ctx->parent_device;
        handle = dev->lst.handle;

        cs = ctx->state;
        cs->dopplerFactor = f;
        aaxScenerySetDopplerFactor(handle, f);
    }
}

ALfloat
_oalGetDopplerVelocity()
{
    _intBufferData *dptr;
    ALfloat ret = 0.0f;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);

        cs = ctx->state;
        ret = cs->dopplerVelocity;
    }

    return ret;
}

void
_oalSetDopplerVelocity(ALfloat f)
{
    _intBufferData *dptr;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);

        cs = ctx->state;
        cs->dopplerVelocity = f;
    }
}

ALfloat
_oalGetSoundVelocity()
{
    _intBufferData *dptr;
    ALfloat ret = 0.0f;
 
    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);

        cs = ctx->state;
        ret = cs->soundVelocity;
    }

    return ret;
}

void
_oalSetSoundVelocity(ALfloat f)
{
    _intBufferData *dptr;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        aaxConfig handle;
        _oalContext *ctx;
        _oalDevice *dev;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);
        dev = (_oalDevice *)ctx->parent_device;
        handle = dev->lst.handle;

        cs = ctx->state;
        cs->soundVelocity = f;
        aaxScenerySetSoundVelocity(handle, f);
    }
}

#if 0
ALfloat
_oalGetMaxDistance()
{
    _intBufferData *dptr;
    ALfloat ret = 0.0f;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);

        cs = ctx->state;
        ret = cs->maxDistance;
    }

    return ret;
}

void
_oalSetMaxDistance(ALfloat f)
{
    _intBufferData *dptr;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);

        cs = ctx->state;
        cs->maxDistance = f;
    }
}
#endif

ALenum
_oalGetDistanceModel(ALboolean b)
{
    _intBufferData *dptr;
    ALenum ret = AL_NONE;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        _oalContext *ctx;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);

        cs = ctx->state;
        if (b) ret = cs->distanceModel & AAX_DISTANCE_DELAY;
        else ret = cs->distanceModel & ~AAX_DISTANCE_DELAY;
        
    }

    return ret;
}

void
_oalSetDistanceModel(ALenum e)
{
    _intBufferData *dptr;

    dptr = _oalGetCurrentContext(0);
    if (dptr)
    {
        aaxConfig handle;
        _oalContext *ctx;
        _oalDevice *dev;
        _oalState *cs;

        ctx = _intBufGetDataPtr(dptr);
        dev = (_oalDevice *)ctx->parent_device;
        handle = dev->lst.handle;

        cs = ctx->state;
        cs->distanceModel = e;

        e -= AL_INVERSE_DISTANCE;
        e += AAX_AL_INVERSE_DISTANCE;
        if (alIsEnabled(AL_DISTANCE_DELAY_AAX)) {
            e |= AAX_DISTANCE_DELAY;
        }
        aaxScenerySetDistanceModel(handle, e);
    }
}
