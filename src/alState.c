/*L_EXT_ima4
 * Copyright (C) 2007-2011 by Erik Hofman.
 * Copyright (C) 2007-2011 by Adalin B.V.
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
static const _intBuffers _oalExtensions;
static const _intBuffers _oalEnumValues;

static ALfloat _oalGetDopplerFactor();
static void _oalSetDopplerFactor(ALfloat f);
static ALfloat _oalGetDopplerVelocity();
static void _oalSetDopplerVelocity(ALfloat f);
static ALfloat _oalGetSoundVelocity();
static void _oalSetSoundVelocity(ALfloat f);
static ALenum _oalGetDistanceModel();
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
        default:
            _oalStateSetError(AL_INVALID_ENUM);
            break;
        }
    }
    return rv;
}

void
alGetBooleanv(ALenum attrib, ALboolean *value)
{
    if (!value)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        *value = (_oalGetDopplerFactor() == 0.0) ? AL_FALSE : AL_TRUE;
        break;
    case AL_SPEED_OF_SOUND:
        *value = (_oalGetSoundVelocity() == 0.0) ? AL_FALSE : AL_TRUE;
        break;
    case AL_DISTANCE_MODEL:
        *value = (_oalGetDistanceModel() == AL_NONE) ? AL_FALSE : AL_TRUE;
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        *value = (_oalGetDopplerVelocity() == 0.0) ? AL_FALSE : AL_TRUE;
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alGetIntegerv(ALenum attrib, ALint *value)
{
    if (!value)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }
 
    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        *value = (ALint)_oalGetDopplerFactor();
        break;
    case AL_SPEED_OF_SOUND:
        *value = (ALint)_oalGetSoundVelocity();
        break;
    case AL_DISTANCE_MODEL:
        *value = _oalGetDistanceModel();
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        *value = (ALint)_oalGetDopplerVelocity();
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alGetFloatv(ALenum attrib, ALfloat *value)
{
    if (!value)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        *value = _oalGetDopplerFactor();
        break;
    case AL_SPEED_OF_SOUND:
        *value = _oalGetSoundVelocity();
        break;
    case AL_DISTANCE_MODEL:
        *value = (ALfloat)_oalGetDistanceModel();
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        *value = _oalGetDopplerVelocity();
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alGetDoublev(ALenum attrib, ALdouble *value)
{
    if (!value)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        *value = _oalGetDopplerFactor();
        break;
    case AL_SPEED_OF_SOUND:
        *value = _oalGetSoundVelocity();
        break;
    case AL_DISTANCE_MODEL:
        *value = (ALdouble)_oalGetDistanceModel();
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        *value = _oalGetDopplerVelocity();
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

ALboolean
alGetBoolean(ALenum attrib)
{
    ALboolean ret = AL_FALSE;

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        ret = (_oalGetDopplerFactor() == 0.0) ? AL_FALSE : AL_TRUE;
        break;
    case AL_SPEED_OF_SOUND:
        ret = (_oalGetSoundVelocity() == 0.0) ? AL_FALSE : AL_TRUE;
        break;
    case AL_DISTANCE_MODEL:
        ret = (_oalGetDistanceModel() == 0) ? AL_FALSE : AL_TRUE;
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        ret = (_oalGetDopplerVelocity() == 0.0) ? AL_FALSE : AL_TRUE;
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }

    return ret;
}

ALint
alGetInteger(ALenum attrib)
{
    ALint ret = 0;

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        ret = (ALint)_oalGetDopplerFactor();
        break;
    case AL_SPEED_OF_SOUND:
        ret = (ALint)_oalGetSoundVelocity();
        break;
    case AL_DISTANCE_MODEL:
        ret = _oalGetDistanceModel();
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        ret = (ALint)_oalGetDopplerVelocity();
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }

    return ret;
}

ALfloat
alGetFloat(ALenum attrib)
{
    ALfloat ret = 0.0f;

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        ret = _oalGetDopplerFactor();
        break;
    case AL_SPEED_OF_SOUND:
        ret = _oalGetSoundVelocity();
        break;
    case AL_DISTANCE_MODEL:
        ret = (ALfloat)_oalGetDistanceModel();
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        ret = _oalGetDopplerVelocity();
        break;
#endif
    default:
         _oalStateSetError(AL_INVALID_ENUM);
    }

    return ret;
}

ALdouble
alGetDouble(ALenum attrib)
{
    ALdouble ret = 0;

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        ret = _oalGetDopplerFactor();
        break;
    case AL_SPEED_OF_SOUND:
        ret = _oalGetSoundVelocity();
        break;
    case AL_DISTANCE_MODEL:
        ret = (ALdouble)_oalGetDistanceModel();
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        ret = _oalGetDopplerVelocity();
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }

    return ret;
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
            retstr = (ALchar *)_oalAAXGetExtensions(&_oalExtensions);
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
    const _intBuffers *exts;
    ALCboolean r = ALC_FALSE;
    unsigned int i, n;

    exts = &_oalExtensions;
    n = _intBufGetNumNoLock(exts, _OAL_EXTENSION);
    for (i=0; i<n; i++)
    {
        _intBufferData *dptr;
        const char *str;

        dptr = _intBufGetNoLock(exts, _OAL_EXTENSION, i);
        str = _intBufGetDataPtr(dptr);

        r = (!strcasecmp((const char *)name, str));
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
    const _intBuffers *exts = &_oalEnumValues;
    const _oalEnumValue_s *e;
    ALenum rv = AL_FALSE;
    unsigned int i, num;

    if (!name)
    {
        _oalContextSetError(ALC_INVALID_VALUE);
        return ALC_FALSE;
    }

    num = _intBufGetNumNoLock(&_oalEnumValues, _OAL_ENUM);
    for (i=0; i<num; i++)
    {
        _intBufferData *dptr;
 
        dptr = _intBufGetNoLock(exts, _OAL_ENUM, i);
        e = _intBufGetDataPtr(dptr);

        if (!strcasecmp((const char *)name, e->name)) {
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

/* -------------------------------------------------------------------------- */

/**
 * extensions
 */
#define MAX_EXTENSIONS     11
static const _intBufferData _oalExtensionsDeclaration[MAX_EXTENSIONS] =
{
    {0, 1, "AL_EXT_exponent_distance"},
    {0, 1, "AL_EXT_linear_distance"},
    {0, 1, "AL_EXT_float32"},
    {0, 1, "AL_EXT_double"},
    {0, 1, "AL_EXT_mulaw"},
    {0, 1, "AL_EXT_alaw"},
    {0, 1, "AL_EXT_ima4"},
    {0, 1, "AL_EXT_offset"},
    {0, 1, "AL_EXT_mcformats"},
    {0, 1, "AL_EXT_loop_points"},
    {0, 1, "AL_EXT_source_distance_model"}
};

static const void *_oalExtensionsPtr[MAX_EXTENSIONS] =
{
    (void *)&_oalExtensionsDeclaration[0],
    (void *)&_oalExtensionsDeclaration[1],
    (void *)&_oalExtensionsDeclaration[2],
    (void *)&_oalExtensionsDeclaration[3],
    (void *)&_oalExtensionsDeclaration[4],
    (void *)&_oalExtensionsDeclaration[5],
    (void *)&_oalExtensionsDeclaration[6],
    (void *)&_oalExtensionsDeclaration[7],
    (void *)&_oalExtensionsDeclaration[8],
    (void *)&_oalExtensionsDeclaration[9],
    (void *)&_oalExtensionsDeclaration[10]
};

static const _intBuffers _oalExtensions =
{
    0,
    _OAL_EXTENSION,
    MAX_EXTENSIONS,
    MAX_EXTENSIONS,
    MAX_EXTENSIONS,
    (_intBufferData **)&_oalExtensionsPtr
};

/**
 * Enum
 */
#ifdef AL_VERSION_1_0
# define MAX_ENUM	102
#else
# define MAX_ENUM	101
#endif
static const _oalEnumValue_s _oalEnumValueDeclaration[MAX_ENUM] =
{
  {"AL_FALSE",                                  AL_FALSE},
  {"AL_TRUE",                                    AL_TRUE},
  {"AL_NO_ERROR",                              AL_NO_ERROR},
  {"AL_INVALID_NAME",                         AL_INVALID_NAME},
  {"AL_INVALID_ENUM",                         AL_INVALID_ENUM},
  {"AL_INVALID_VALUE",                        AL_INVALID_VALUE},
  {"AL_INVALID_OPERATION",                  AL_INVALID_OPERATION},
  {"AL_OUT_OF_MEMORY",                        AL_OUT_OF_MEMORY},
  {"AL_VENDOR",                                 AL_VENDOR},
  {"AL_VERSION",                                AL_VERSION},
  {"AL_RENDERER",                              AL_RENDERER},
  {"AL_EXTENSIONS",                            AL_EXTENSIONS},
  {"AL_FORMAT_MONO8",                         AL_FORMAT_MONO8},
  {"AL_FORMAT_MONO16",                        AL_FORMAT_MONO16},
  {"AL_FORMAT_STEREO8",                      AL_FORMAT_STEREO8},
  {"AL_FORMAT_STEREO16",                     AL_FORMAT_STEREO16},
  {"AL_DOPPLER_FACTOR",                      AL_DOPPLER_FACTOR},
#ifdef AL_VERSION_1_0
  {"AL_DOPPLER_VELOCITY",                    AL_DOPPLER_VELOCITY},
#endif
  {"AL_SPEED_OF_SOUND",                      AL_SPEED_OF_SOUND},
  {"AL_DISTANCE_MODEL",                      AL_DISTANCE_MODEL},
  {"AL_INVERSE_DISTANCE",                    AL_INVERSE_DISTANCE},
  {"AL_INVERSE_DISTANCE_CLAMPED",         AL_INVERSE_DISTANCE_CLAMPED},
  {"AL_LINEAR_DISTANCE",                     AL_LINEAR_DISTANCE},
  {"AL_LINEAR_DISTANCE_CLAMPED",          AL_LINEAR_DISTANCE_CLAMPED},
  {"AL_EXPONENT_DISTANCE",                  AL_EXPONENT_DISTANCE},
  {"AL_EXPONENT_DISTANCE_CLAMPED",        AL_EXPONENT_DISTANCE_CLAMPED},
  {"AL_SOURCE_RELATIVE",                     AL_SOURCE_RELATIVE},
  {"AL_CONE_INNER_ANGLE",                    AL_CONE_INNER_ANGLE},
  {"AL_CONE_OUTER_ANGLE",                    AL_CONE_OUTER_ANGLE},
  {"AL_PITCH",                                  AL_PITCH},
  {"AL_POSITION",                              AL_POSITION},
  {"AL_DIRECTION",                             AL_DIRECTION},
  {"AL_VELOCITY",                              AL_VELOCITY},
  {"AL_LOOPING",                                AL_LOOPING},
  {"AL_BUFFER",                                 AL_BUFFER},
  {"AL_GAIN",                                    AL_GAIN},
  {"AL_MIN_GAIN",                              AL_MIN_GAIN},
  {"AL_MAX_GAIN",                              AL_MAX_GAIN},
  {"AL_ORIENTATION",                          AL_ORIENTATION},
  {"AL_SOURCE_STATE",                         AL_SOURCE_STATE},
  {"AL_INITIAL",                                AL_INITIAL},
  {"AL_PLAYING",                                AL_PLAYING},
  {"AL_PAUSED",                                 AL_PAUSED},
  {"AL_STOPPED",                                AL_STOPPED},
  {"AL_BUFFERS_QUEUED",                      AL_BUFFERS_QUEUED},
  {"AL_BUFFERS_PROCESSED",                  AL_BUFFERS_PROCESSED},
  {"AL_REFERENCE_DISTANCE",                 AL_REFERENCE_DISTANCE},
  {"AL_ROLLOFF_FACTOR",                      AL_ROLLOFF_FACTOR},
  {"AL_CONE_OUTER_GAIN",                     AL_CONE_OUTER_GAIN},
  {"AL_MAX_DISTANCE",                         AL_MAX_DISTANCE},
  {"AL_SEC_OFFSET",                            AL_SEC_OFFSET},
  {"AL_SAMPLE_OFFSET",                        AL_SAMPLE_OFFSET},
  {"AL_BYTE_OFFSET",                          AL_BYTE_OFFSET},
  {"AL_SOURCE_TYPE",                          AL_SOURCE_TYPE},
  {"AL_STATIC",                                 AL_STATIC},
  {"AL_STREAMING",                             AL_STREAMING},
  {"AL_UNDETERMINED",                         AL_UNDETERMINED},
  {"AL_CHANNEL_MASK",                         AL_CHANNEL_MASK},
  {"AL_FREQUENCY",                             AL_FREQUENCY},
  {"AL_BITS",                                    AL_BITS},
  {"AL_CHANNELS",                              AL_CHANNELS},
  {"AL_SIZE",                                    AL_SIZE},
  {"AL_UNUSED",                                 AL_UNUSED},
  {"AL_PENDING",                                AL_PENDING},
  {"AL_PROCESSED",                             AL_PROCESSED},
  {"AL_FORMAT_MONO_IMA4",                    AL_FORMAT_MONO_IMA4},
  {"AL_FORMAT_STEREO_IMA4",                 AL_FORMAT_STEREO_IMA4},

/* Extensions */
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
  {"AL_FORMAT_IMA_ADPCM_MONO16_EXT",     AL_FORMAT_IMA_ADPCM_MONO16_EXT},
  {"AL_FORMAT_IMA_ADPCM_STEREO16_EXT",  AL_FORMAT_IMA_ADPCM_STEREO16_EXT},
  {"AL_FORMAT_MONO_FLOAT32",                AL_FORMAT_MONO_FLOAT32},
  {"AL_FORMAT_STEREO_FLOAT32",             AL_FORMAT_STEREO_FLOAT32},
  {"AL_FORMAT_MONO_DOUBLE_EXT",            AL_FORMAT_MONO_DOUBLE_EXT},
  {"AL_FORMAT_STEREO_DOUBLE_EXT",         AL_FORMAT_STEREO_DOUBLE_EXT},
  {"AL_FORMAT_MONO_MULAW_EXT",             AL_FORMAT_MONO_MULAW_EXT},
  {"AL_FORMAT_STEREO_MULAW_EXT",          AL_FORMAT_STEREO_MULAW_EXT},
  {"AL_FORMAT_MONO_ALAW_EXT",              AL_FORMAT_MONO_ALAW_EXT},
  {"AL_FORMAT_STEREO_ALAW_EXT",            AL_FORMAT_STEREO_ALAW_EXT},
  {"AL_FORMAT_QUAD8_LOKI",                  AL_FORMAT_QUAD8_LOKI},
  {"AL_FORMAT_QUAD16_LOKI",                 AL_FORMAT_QUAD16_LOKI},

  /* Source EFX */
  {"AL_DIRECT_FILTER",			AL_DIRECT_FILTER},
  {"AL_AUXILIARY_SEND_FILTER",		AL_AUXILIARY_SEND_FILTER},
  {"AL_AIR_ABSORPTION_FACTOR",		AL_AIR_ABSORPTION_FACTOR},
  {"AL_ROOM_ROLLOFF_FACTOR",		AL_ROOM_ROLLOFF_FACTOR},
  {"AL_CONE_OUTER_GAINHF",		AL_CONE_OUTER_GAINHF},
  {"AL_DIRECT_FILTER_GAINHF_AUTO",	AL_DIRECT_FILTER_GAINHF_AUTO},
  {"AL_AUXILIARY_SEND_FILTER_GAIN_AUTO",AL_AUXILIARY_SEND_FILTER_GAIN_AUTO},
  {"AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO",AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO} 
};

static const _intBufferData _oalEnumValue[MAX_ENUM] =
{
    {0, 1, (void *)&_oalEnumValueDeclaration[0]},
    {0, 1, (void *)&_oalEnumValueDeclaration[1]},
    {0, 1, (void *)&_oalEnumValueDeclaration[2]},
    {0, 1, (void *)&_oalEnumValueDeclaration[3]},
    {0, 1, (void *)&_oalEnumValueDeclaration[4]},
    {0, 1, (void *)&_oalEnumValueDeclaration[5]},
    {0, 1, (void *)&_oalEnumValueDeclaration[6]},
    {0, 1, (void *)&_oalEnumValueDeclaration[7]},
    {0, 1, (void *)&_oalEnumValueDeclaration[8]},
    {0, 1, (void *)&_oalEnumValueDeclaration[9]},
    {0, 1, (void *)&_oalEnumValueDeclaration[10]},
    {0, 1, (void *)&_oalEnumValueDeclaration[11]},
    {0, 1, (void *)&_oalEnumValueDeclaration[12]},
    {0, 1, (void *)&_oalEnumValueDeclaration[13]},
    {0, 1, (void *)&_oalEnumValueDeclaration[14]},
    {0, 1, (void *)&_oalEnumValueDeclaration[15]},
    {0, 1, (void *)&_oalEnumValueDeclaration[16]},
    {0, 1, (void *)&_oalEnumValueDeclaration[17]},
    {0, 1, (void *)&_oalEnumValueDeclaration[18]},
    {0, 1, (void *)&_oalEnumValueDeclaration[19]},
    {0, 1, (void *)&_oalEnumValueDeclaration[20]},
    {0, 1, (void *)&_oalEnumValueDeclaration[21]},
    {0, 1, (void *)&_oalEnumValueDeclaration[22]},
    {0, 1, (void *)&_oalEnumValueDeclaration[23]},
    {0, 1, (void *)&_oalEnumValueDeclaration[24]},
    {0, 1, (void *)&_oalEnumValueDeclaration[25]},
    {0, 1, (void *)&_oalEnumValueDeclaration[26]},
    {0, 1, (void *)&_oalEnumValueDeclaration[27]},
    {0, 1, (void *)&_oalEnumValueDeclaration[28]},
    {0, 1, (void *)&_oalEnumValueDeclaration[29]},
    {0, 1, (void *)&_oalEnumValueDeclaration[30]},
    {0, 1, (void *)&_oalEnumValueDeclaration[31]},
    {0, 1, (void *)&_oalEnumValueDeclaration[32]},
    {0, 1, (void *)&_oalEnumValueDeclaration[33]},
    {0, 1, (void *)&_oalEnumValueDeclaration[34]},
    {0, 1, (void *)&_oalEnumValueDeclaration[35]},
    {0, 1, (void *)&_oalEnumValueDeclaration[36]},
    {0, 1, (void *)&_oalEnumValueDeclaration[37]},
    {0, 1, (void *)&_oalEnumValueDeclaration[38]},
    {0, 1, (void *)&_oalEnumValueDeclaration[39]},
    {0, 1, (void *)&_oalEnumValueDeclaration[40]},
    {0, 1, (void *)&_oalEnumValueDeclaration[41]},
    {0, 1, (void *)&_oalEnumValueDeclaration[42]},
    {0, 1, (void *)&_oalEnumValueDeclaration[43]},
    {0, 1, (void *)&_oalEnumValueDeclaration[44]},
    {0, 1, (void *)&_oalEnumValueDeclaration[45]},
    {0, 1, (void *)&_oalEnumValueDeclaration[46]},
    {0, 1, (void *)&_oalEnumValueDeclaration[47]},
    {0, 1, (void *)&_oalEnumValueDeclaration[48]},
    {0, 1, (void *)&_oalEnumValueDeclaration[49]},
    {0, 1, (void *)&_oalEnumValueDeclaration[50]},
    {0, 1, (void *)&_oalEnumValueDeclaration[51]},
    {0, 1, (void *)&_oalEnumValueDeclaration[52]},
    {0, 1, (void *)&_oalEnumValueDeclaration[53]},
    {0, 1, (void *)&_oalEnumValueDeclaration[54]},
    {0, 1, (void *)&_oalEnumValueDeclaration[55]},
    {0, 1, (void *)&_oalEnumValueDeclaration[56]},
    {0, 1, (void *)&_oalEnumValueDeclaration[57]},
    {0, 1, (void *)&_oalEnumValueDeclaration[58]},
    {0, 1, (void *)&_oalEnumValueDeclaration[59]},
    {0, 1, (void *)&_oalEnumValueDeclaration[60]},
    {0, 1, (void *)&_oalEnumValueDeclaration[61]},
    {0, 1, (void *)&_oalEnumValueDeclaration[62]},
    {0, 1, (void *)&_oalEnumValueDeclaration[63]},
    {0, 1, (void *)&_oalEnumValueDeclaration[64]},
    {0, 1, (void *)&_oalEnumValueDeclaration[65]},
    {0, 1, (void *)&_oalEnumValueDeclaration[66]},
    {0, 1, (void *)&_oalEnumValueDeclaration[67]},
    {0, 1, (void *)&_oalEnumValueDeclaration[68]},
    {0, 1, (void *)&_oalEnumValueDeclaration[69]},
    {0, 1, (void *)&_oalEnumValueDeclaration[70]},
    {0, 1, (void *)&_oalEnumValueDeclaration[71]},
    {0, 1, (void *)&_oalEnumValueDeclaration[72]},
    {0, 1, (void *)&_oalEnumValueDeclaration[73]},
    {0, 1, (void *)&_oalEnumValueDeclaration[74]},
    {0, 1, (void *)&_oalEnumValueDeclaration[75]},
    {0, 1, (void *)&_oalEnumValueDeclaration[76]},
    {0, 1, (void *)&_oalEnumValueDeclaration[77]},
    {0, 1, (void *)&_oalEnumValueDeclaration[78]},
    {0, 1, (void *)&_oalEnumValueDeclaration[79]},
    {0, 1, (void *)&_oalEnumValueDeclaration[80]},
    {0, 1, (void *)&_oalEnumValueDeclaration[81]},
    {0, 1, (void *)&_oalEnumValueDeclaration[82]},
    {0, 1, (void *)&_oalEnumValueDeclaration[83]},
    {0, 1, (void *)&_oalEnumValueDeclaration[84]},
    {0, 1, (void *)&_oalEnumValueDeclaration[85]},
    {0, 1, (void *)&_oalEnumValueDeclaration[86]},
    {0, 1, (void *)&_oalEnumValueDeclaration[87]},
    {0, 1, (void *)&_oalEnumValueDeclaration[88]},
    {0, 1, (void *)&_oalEnumValueDeclaration[89]},
    {0, 1, (void *)&_oalEnumValueDeclaration[90]},
    {0, 1, (void *)&_oalEnumValueDeclaration[91]},
    {0, 1, (void *)&_oalEnumValueDeclaration[92]}
#ifdef AL_VERSION_1_0
    , {0, 1, (void *)&_oalEnumValueDeclaration[93]}
#endif
};

static const void *_oalEnumValuePtr[MAX_ENUM] =
{
    (void *)&_oalEnumValue[0],
    (void *)&_oalEnumValue[1],
    (void *)&_oalEnumValue[2],
    (void *)&_oalEnumValue[3],
    (void *)&_oalEnumValue[4],
    (void *)&_oalEnumValue[5],
    (void *)&_oalEnumValue[6],
    (void *)&_oalEnumValue[7],
    (void *)&_oalEnumValue[8],
    (void *)&_oalEnumValue[9],
    (void *)&_oalEnumValue[10],
    (void *)&_oalEnumValue[11],
    (void *)&_oalEnumValue[12],
    (void *)&_oalEnumValue[13],
    (void *)&_oalEnumValue[14],
    (void *)&_oalEnumValue[15],
    (void *)&_oalEnumValue[16],
    (void *)&_oalEnumValue[17],
    (void *)&_oalEnumValue[18],
    (void *)&_oalEnumValue[19],
    (void *)&_oalEnumValue[20],
    (void *)&_oalEnumValue[21],
    (void *)&_oalEnumValue[22],
    (void *)&_oalEnumValue[23],
    (void *)&_oalEnumValue[24],
    (void *)&_oalEnumValue[25],
    (void *)&_oalEnumValue[26],
    (void *)&_oalEnumValue[27],
    (void *)&_oalEnumValue[28],
    (void *)&_oalEnumValue[29],
    (void *)&_oalEnumValue[30],
    (void *)&_oalEnumValue[31],
    (void *)&_oalEnumValue[32],
    (void *)&_oalEnumValue[33],
    (void *)&_oalEnumValue[34],
    (void *)&_oalEnumValue[35],
    (void *)&_oalEnumValue[36],
    (void *)&_oalEnumValue[37],
    (void *)&_oalEnumValue[38],
    (void *)&_oalEnumValue[39],
    (void *)&_oalEnumValue[40],
    (void *)&_oalEnumValue[41],
    (void *)&_oalEnumValue[42],
    (void *)&_oalEnumValue[43],
    (void *)&_oalEnumValue[44],
    (void *)&_oalEnumValue[45],
    (void *)&_oalEnumValue[46],
    (void *)&_oalEnumValue[47],
    (void *)&_oalEnumValue[48],
    (void *)&_oalEnumValue[49],
    (void *)&_oalEnumValue[50],
    (void *)&_oalEnumValue[51],
    (void *)&_oalEnumValue[52],
    (void *)&_oalEnumValue[53],
    (void *)&_oalEnumValue[54],
    (void *)&_oalEnumValue[55],
    (void *)&_oalEnumValue[56],
    (void *)&_oalEnumValue[57],
    (void *)&_oalEnumValue[58],
    (void *)&_oalEnumValue[59],
    (void *)&_oalEnumValue[60],
    (void *)&_oalEnumValue[61],
    (void *)&_oalEnumValue[62],
    (void *)&_oalEnumValue[63],
    (void *)&_oalEnumValue[64],
    (void *)&_oalEnumValue[65],
    (void *)&_oalEnumValue[66],
    (void *)&_oalEnumValue[67],
    (void *)&_oalEnumValue[68],
    (void *)&_oalEnumValue[69],
    (void *)&_oalEnumValue[70],
    (void *)&_oalEnumValue[71],
    (void *)&_oalEnumValue[72],
    (void *)&_oalEnumValue[73],
    (void *)&_oalEnumValue[74],
    (void *)&_oalEnumValue[75],
    (void *)&_oalEnumValue[76],
    (void *)&_oalEnumValue[77],
    (void *)&_oalEnumValue[78],
    (void *)&_oalEnumValue[79],
    (void *)&_oalEnumValue[80],
    (void *)&_oalEnumValue[81],
    (void *)&_oalEnumValue[82],
    (void *)&_oalEnumValue[83],
    (void *)&_oalEnumValue[84],
    (void *)&_oalEnumValue[85],
    (void *)&_oalEnumValue[86],
    (void *)&_oalEnumValue[87],
    (void *)&_oalEnumValue[88],
    (void *)&_oalEnumValue[89],
    (void *)&_oalEnumValue[90],
    (void *)&_oalEnumValue[91],
    (void *)&_oalEnumValue[92]
#ifdef AL_VERSION_1_0
    , (void *)&_oalEnumValue[93]
#endif
};

static const _intBuffers _oalEnumValues =
{
    0,
    _OAL_ENUM,
    MAX_ENUM,
    MAX_ENUM,
    MAX_ENUM,
    (void *)&_oalEnumValuePtr
};
#undef MAX_ENUM

static const char *_oalStateErrorStrings[] =
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
_oalGetDistanceModel()
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
        ret = cs->distanceModel;
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
        aaxScenerySetDistanceModel(handle, e);
    }
}
