/*
 * Copyright (C) 2007-2012 by Erik Hofman.
 * Copyright (C) 2007-2012 by Adalin B.V.
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
#include <assert.h>
#endif
#include <errno.h>
#include <strings.h>	/* strncpy */
#ifndef NDEBUG
#include <unistd.h>
#else
#include <stdio.h>
#endif

#include <aaxdefs.h>
// #include <AL/al.h>
#include <AL/alc.h>

#include <base/types.h>

#include "api.h"
#include "aax_support.h"

/* forward declarations */
static const int _oalEFXVersion[];
static const int _oalContextVersion[];
static unsigned int _oalCurrentContext;
static const _intBuffers _oalContextEnumValues;
static const _intBufferData _oalContextExtensionsDeclaration[];
static const _intBuffers _oalContextExtensions;
static const _intBufferData _oalContextEnumValue[];
static const char *_oalContextErrorStrings[];

static _intBufferData *_oalDeviceContextAdd(_oalDevice *);

static _intBufferData *_oalFindContextByDeviceId(uint32_t);
static void _oalSourcesCreate(void *);

/**
 * The following function(s) require locked data, which will be unlocked
 * by the function automatically.
 */
static _intBufFreeCallback _oalDestroyContextByPos;

ALCdevice *
alcOpenDevice(const ALCchar *name)
{
    ALCdevice *device = 0;
    unsigned int pos = 0;
    aaxConfig handle;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (_oalDevices == 0)
    {
        pos = _intBufCreate(&_oalDevices, _OAL_DEVICE);
        if (pos == UINT_MAX)
        {
            _oalContextSetError(ALC_OUT_OF_MEMORY);
            return 0;
        }
    }

    handle = aaxDriverOpenByName(name, AAX_MODE_WRITE_STEREO);
    if (handle != NULL)
    {
        _oalDevice *d = calloc(1, sizeof(_oalDevice));
        if (d)
        {
            d->sync = 0;
            d->lst.handle = handle;
            pos = _intBufAddData(_oalDevices, _OAL_DEVICE, d);
            if (pos != UINT_MAX)
            {
                uint32_t i, id, devid;

                id = _intBufPosToId(pos);
                devid = _oalIdToDevice(id);
                device = INT_TO_PTR(devid);

                d->lst.frame_no = 0;
                d->lst.frame_max = _oalAAXGetNoCores();
                for(i=0; i<d->lst.frame_max-1; i++)
                {
                    aaxFrame frame = aaxAudioFrameCreate(handle);
                    aaxMtx4f mtx;

                    aaxMatrixSetIdentityMatrix(mtx);
                    aaxAudioFrameSetMatrix(frame, mtx);
                    aaxAudioFrameSetMode(frame, AAX_POSITION, AAX_RELATIVE);
                    aaxMixerRegisterAudioFrame(handle, frame);
                    aaxAudioFrameSetState(frame, AAX_PLAYING);

                    d->lst.frame[i] = frame;
                }
            }
        }
    }

    return device;
}

ALCboolean
alcCloseDevice(ALCdevice *device)
{
    uint32_t id, pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    id = _oalDeviceToId(device);
    pos  = _intBufIdToPos(id);
    if (pos != UINT_MAX)
    {
        _oalDevice *d;
        d = _intBufRemove(_oalDevices, _OAL_DEVICE, pos, AL_FALSE);
        if (d)
        {
            int32_t i;
            for(i=0; i<_oalAAXGetNoCores()-1; i++) 
            {
                aaxFrame frame = d->lst.frame[i];
                aaxMixerDeregisterAudioFrame(d->lst.handle, frame);
                aaxAudioFrameDestroy(frame);
            }

            aaxDriverClose(d->lst.handle);
            aaxDriverDestroy(d->lst.handle);
            _intBufErase(&d->contexts,_OAL_CONTEXT, _oalDestroyContextByPos, d);

            _intBufErase(&d->buffers, _OAL_BUFFER, _oalRemoveBufferByPos, d);
            free(d);

            if (_intBufGetNumNoLock(_oalDevices, _OAL_DEVICE) == 0) {
                _intBufErase(&_oalDevices, _OAL_DEVICE, 0, 0);
            }

            return ALC_TRUE;
        }
    }
    return ALC_FALSE;
}

ALCcontext *
alcCreateContext(const ALCdevice *device, const ALCint *attributes)
{
    _intBufferData *dptr_ctx = 0;
    aaxConfig handle;
    uint32_t id = 0;
    _oalContext *ctx;
    _oalDevice *d;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    d = _oalFindDeviceById(_oalDeviceToId(device));
    dptr_ctx = _oalDeviceContextAdd(d);
    if (!dptr_ctx)
    {
        _oalContextSetError(ALC_INVALID_DEVICE);
        return 0;
    }

    id = _oalDeviceMask(device);
    id |= _intBufPosToId(d->current_context);

    ctx = _intBufGetDataPtr(dptr_ctx);
    ctx->sync = d->sync;

    handle = d->lst.handle;
    if (attributes)
    {
        unsigned int n;

        for (n=0; attributes[n] !=0; n++)
        {
            switch(attributes[n++])                 /* Hint: No Typo. */
            {
            case ALC_SYNC:
                ctx->sync = attributes[n];
                break;
            case ALC_MONO_SOURCES:
                aaxMixerSetMonoSources(attributes[n]);
                break;
            case ALC_STEREO_SOURCES:
                aaxMixerSetStereoSources(attributes[n]);
                break;
            case ALC_FREQUENCY:
                aaxMixerSetFrequency(handle, (unsigned)attributes[n]);
                break;
            case ALC_REFRESH:
                aaxMixerSetRefreshRate(handle, (unsigned)attributes[n]);
                break;
            default:
                _oalContextSetError(ALC_INVALID_VALUE);
            }
        }
    }

    aaxMixerInit(handle);
    aaxMixerSetState(handle, AAX_PLAYING);

    _oalStateCreate(ctx);
    _oalSourcesCreate(ctx);

    return INT_TO_PTR(id);
}

ALCboolean
alcMakeContextCurrent(ALCcontext *context)
{
    unsigned int pos = UINT_MAX;

    _AL_LOG(LOG_INFO, __FUNCTION__);
 
    if (context)
    {
        unsigned int id;

        id = _oalDeviceToId(context);
        pos = _intBufIdToPos(id);
        if (pos != UINT_MAX)
        {
            _oalDevice *dev = _oalFindDeviceById(id);
            if (dev)
            {
                id = _oalContextMask(context);
                pos = _intBufIdToPos(id);
                if (pos != UINT_MAX)
                {
                    _oalCurrentContext = (unsigned long)context;
                    dev->current_context = _intBufIdToPos(id);
                }
            }
        }
        else {
            _oalContextSetError(ALC_INVALID_CONTEXT);
        }
    }
    else
    {
        unsigned int num, i;

        num = _intBufGetNumNoLock(_oalDevices, _OAL_DEVICE);
        for (i=0; i<num; i++)
        {
            _intBufferData *dptr;

            dptr =  _intBufGetNoLock(_oalDevices, _OAL_DEVICE, i);
            if (dptr)
            {
                _oalDevice *dev;

                dev = _intBufGetDataPtr(dptr);

                dev->current_context =  UINT_MAX;
                _oalCurrentContext = UINT_MAX;
            }
        }

        pos = 0;
    }

    return (pos == UINT_MAX) ? AL_FALSE : AL_TRUE;
}

void
alcProcessContext(ALCcontext *context)
{
#if 0
    _oalContext *ctx;
    uint32_t id;

    id = _oalContextMask(context);
    ctx = _oalFindContextById(id);
    if (ctx)
        ctx->suspend = ALC_FALSE;
#if 0
    else
        _oalContextSetError(ALC_INVALID_CONTEXT);
#endif
#endif
}

void
alcSuspendContext(ALCcontext *context)
{
#if 0
    _oalContext *ctx;
    uint32_t id;

    id = _oalContextMask(context);
    ctx = _oalFindContextById(id);
    if (ctx)
        ctx->suspend = ALC_TRUE;
    else
        _oalContextSetError(ALC_INVALID_CONTEXT);
#endif
}

void
alcDestroyContext(ALCcontext *context)
{
    _oalDevice *dev;
    uint32_t id;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    id = _oalDeviceToId(context);
    dev = _oalFindDeviceById(id);
    if (dev)
    {
        unsigned int pos;

        id = _oalContextMask(context);
        pos = _intBufIdToPos(id);
        if (pos != UINT_MAX)
        {
            _oalDestroyContextByPos(dev, pos);
            return;  
        }
    }
    _oalContextSetError(ALC_INVALID_CONTEXT);
}

ALCcontext *
alcGetCurrentContext(void)
{
    return INT_TO_PTR(_oalCurrentContext);
}

ALCdevice *
alcGetContextsDevice(ALCcontext *context)
{
    unsigned int ctx_id, dev_id = 0;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    ctx_id = _oalContextMask(context);
    if (ctx_id) {
        dev_id = _oalDeviceMask(context);
    }

    return INT_TO_PTR(dev_id);
}

void *
alcGetProcAddress(const ALCdevice *device, const ALCchar *name)
{
    void *p = 0;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (name) {
        p = alGetProcAddress((const ALchar *)name);
    }

    if (!p) {
        _oalContextSetError(ALC_INVALID_VALUE);
    }

    return p;
}

ALCenum
alcGetError(ALCdevice *device)
{
    ALCenum ret = ALC_NO_ERROR;
    if (device)
    {
        _intBufferData *dptr;
        uint32_t id;

        _AL_LOG(LOG_DEBUG, __FUNCTION__);

        id = _oalDeviceToId(device);
        dptr = _oalFindContextByDeviceId(id);
        if (dptr)
        {
            _oalContext *ctx;

            ctx = _intBufGetDataPtr(dptr);
            ret = ctx->error;
        }
        else
        {
            _oalDevice *dev = dev = (_oalDevice *)_oalFindDeviceById(id);
            if (dev)
            {
                ret = dev->error;
                dev->error = ALC_NO_ERROR;
            } else {
                ret = _oalContextSetError(ALC_INVALID_DEVICE);
            }
        }
    }
    else {
        ret = _oalContextSetError(ALC_NO_ERROR);
    }

    return ret;
}

void
alcGetIntegerv(ALCdevice *device, ALCenum attrib, ALCsizei size, ALCint *value)
{
    _oalDevice *dev;
    uint32_t id;
    char done = 1;

    if ((size <= 0) || !value)
    {
        _oalContextSetError(ALC_INVALID_VALUE);
        return;
    }

    switch(attrib)
    {
    case ALC_MONO_SOURCES:
        *value = _MIN((ALCuint)aaxMixerGetNoMonoSources(), 255);
        break;
    case ALC_STEREO_SOURCES:
        *value = _MIN((ALCuint)aaxMixerGetNoStereoSources(), 255);
        break;
    case ALC_MAJOR_VERSION:
        *value = _oalContextVersion[0];
        break;
    case ALC_MINOR_VERSION:
        *value = _oalContextVersion[1];
        break;
    case ALC_EFX_MAJOR_VERSION:
        *value = _oalEFXVersion[0];
        break;
    case ALC_EFX_MINOR_VERSION:
        *value = _oalEFXVersion[1];
        break;
    case ALC_ATTRIBUTES_SIZE:			/* TODO */
    case ALC_ALL_ATTRIBUTES:
        *value = 0;
        break;
    default:
        done = 0;
    }

    if (done) return;

    id = _oalDeviceToId(device);
    dev = (_oalDevice *)_oalFindDeviceById(id);
    if (dev)
    {
        aaxConfig config = dev->lst.handle;
        switch(attrib)
        {
        case ALC_FREQUENCY:
            *value = (ALint)aaxMixerGetFrequency(config);
            break;
        case ALC_REFRESH:
            *value = (ALint)aaxMixerGetSetup(config, AAX_REFRESHRATE);
            break;
        case ALC_CAPTURE_SAMPLES:
            *value = (ALint)aaxSensorGetOffset(config, AAX_SAMPLES);
            break;
        default:
            *value = 0;
            _oalContextSetError(ALC_INVALID_ENUM);
        }
    }
    else
        _oalContextSetError(ALC_INVALID_DEVICE);
}

ALCboolean
alcIsExtensionPresent(const ALCdevice *device,  const ALCchar *name)
{
    _intBuffers *exts = (_intBuffers *)&_oalContextExtensions;
    ALCboolean r = ALC_FALSE;
    unsigned int i, n;

    assert (exts->id == _OAL_EXTENSION);

    if (!name)
    {
        _oalContextSetError(ALC_INVALID_VALUE);
        return ALC_FALSE;
    }

    n = _intBufGetNumNoLock(exts, _OAL_EXTENSION);
    for (i=0; i<n; i++)
    {
        _intBufferData *dptr;
        const char *str;

        dptr = _intBufGetNoLock(exts, _OAL_EXTENSION, i);
        str = _intBufGetDataPtr(dptr);

        assert(str);
        r = (!strcasecmp((const char *)name, str));
        if (r) break;
    }

    if (i < n) return AL_TRUE;

    r = _oalAAXGetCtxExtensionSupport((const char*)name);

    return (r) ? AL_TRUE : AL_FALSE;
}

ALCenum
alcGetEnumValue(const ALCdevice *device, const ALCchar *name)
{
    _intBuffers *enums = (_intBuffers *)&_oalContextEnumValues;
    const _oalEnumValue_s *e;
    ALCenum rv = ALC_FALSE;
    unsigned int i, num;

    assert (enums->id == _OAL_ENUM);

    if (!name)
    {
        _oalContextSetError(ALC_INVALID_VALUE);
        return ALC_FALSE;
    }

    num = _intBufGetNumNoLock(enums, _OAL_ENUM);
    for (i=0; i<num; i++)
    {
        _intBufferData *dptr;

        dptr = _intBufGetNoLock(enums, _OAL_ENUM, i);
        e = _intBufGetDataPtr(dptr);
        if (!e) continue;

        assert(e->name);
        if (!strcasecmp((const char *)name, e->name)) {
            rv = e->enumVal;
            break;
        }
    }

    if (i == num)
    {
        e = _oalAAXGetEnum(name);
        if (e != NULL) {
            rv = e->enumVal;
        } else {
            _oalContextSetError(ALC_INVALID_VALUE);
        }
    }

    return rv;
}

const ALCchar *
alcGetString(ALCdevice *device, ALCenum attrib)
{
    _oalDevice *dev;
    unsigned int id;
    char *retstr;

    id = _oalDeviceToId(device);
    dev = _oalFindDeviceById(id);
    retstr = "";
    switch(attrib)
    {
    case ALC_NO_ERROR:
        retstr = "There is no current error.";
        break;
    case ALC_INVALID_DEVICE:
    case ALC_INVALID_CONTEXT:
    case ALC_INVALID_ENUM:
    case ALC_INVALID_VALUE:
    case ALC_OUT_OF_MEMORY:
    {
        unsigned pos = attrib - ALC_INVALID_DEVICE;
        retstr = (char *)_oalContextErrorStrings[pos];
        break;
    }
    case ALC_EXTENSIONS:
        retstr = (char *)_oalAAXGetCtxExtensions(&_oalContextExtensions);
        break;
    case ALC_CAPTURE_DEVICE_SPECIFIER:
        if (device == 0) {
            retstr = (char *)_oalAAXGetDeviceSpecifiersAll(AAX_MODE_READ);
        }
        else {
            aaxConfig config = dev->lst.handle;
            retstr = (char *)aaxDriverGetRenderer(config);
        }
        break;
    case ALC_DEVICE_SPECIFIER:
        if (device == 0) {
            retstr = (char *)_oalAAXGetDriverSpecifiers(AAX_MODE_WRITE_STEREO);
        }
        else {
            aaxConfig config = dev->lst.handle;
            retstr = (char *)aaxDriverGetRenderer(config);
        }
        break;
    case ALC_ALL_DEVICES_SPECIFIER:
        if (device == 0) {
            retstr = (char *)_oalAAXGetDeviceSpecifiersAll(AAX_MODE_WRITE_STEREO);
        }
        else {
            aaxConfig config = dev->lst.handle;
            retstr = (char *)aaxDriverGetRenderer(config);
        }
        break;
    case ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:
    {
        aaxConfig config = aaxDriverGetDefault(AAX_MODE_READ);
        retstr = (char *)aaxDriverGetRenderer(config);
        aaxDriverDestroy(config);
        break;
    }
    case ALC_DEFAULT_DEVICE_SPECIFIER:
    {
        aaxConfig config = aaxDriverGetDefault(AAX_MODE_WRITE_STEREO);
        retstr = (char *)aaxDriverGetRenderer(config);
        aaxDriverDestroy(config);
        break;
    }
    default:
        _oalContextSetError(ALC_INVALID_ENUM);
    }

    return retstr;
}


/*-------------------------------------------------------------------------- */

_intBuffers *_oalDevices = 0;

static unsigned int _oalCurrentContext = UINT_MAX;

static const int _oalContextVersion[2] = {1, 1};
static const int _oalEFXVersion[2] = {1, 0};

#define MAX_EXTENSIONS	2
static const _intBufferData _oalContextExtensionsDeclaration[MAX_EXTENSIONS]=
{
    {0, 1, "ALC_enumeration_EXT"},
    {0, 1, "ALC_enumerate_all_EXT"}
};

static const void *_oalContextExtensionsPtr[MAX_EXTENSIONS] =
{
    (void *)&_oalContextExtensionsDeclaration[0],
    (void *)&_oalContextExtensionsDeclaration[1]
};

static const _intBuffers _oalContextExtensions =
{
    0,
    _OAL_EXTENSION,
    MAX_EXTENSIONS,
    MAX_EXTENSIONS,
    MAX_EXTENSIONS,
    (_intBufferData **)&_oalContextExtensionsPtr
};

#define MAX_ENUM	19
static const _oalEnumValue_s _oalContextEnumValueDeclaration[MAX_ENUM] =
{
  {"ALC_FALSE",                                 ALC_FALSE},
  {"ALC_TRUE",                                  ALC_TRUE},
  {"ALC_NO_ERROR",                             ALC_NO_ERROR},
  {"ALC_INVALID_DEVICE",                     ALC_INVALID_DEVICE},
  {"ALC_INVALID_CONTEXT",                    ALC_INVALID_CONTEXT},
  {"ALC_INVALID_ENUM",                        ALC_INVALID_ENUM},
  {"ALC_INVALID_VALUE",                      ALC_INVALID_VALUE},
  {"ALC_OUT_OF_MEMORY",                      ALC_OUT_OF_MEMORY},
  {"ALC_DEFAULT_DEVICE_SPECIFIER",        ALC_DEFAULT_DEVICE_SPECIFIER},
  {"ALC_ALL_DEVICES_SPECIFIER", 	ALC_ALL_DEVICES_SPECIFIER},
  {"ALC_DEFAULT_ALL_DEVICES_SPECIFIER",	ALC_DEFAULT_ALL_DEVICES_SPECIFIER},
  {"ALC_DEVICE_SPECIFIER",                  ALC_DEVICE_SPECIFIER},
  {"ALC_EXTENSIONS",                          ALC_EXTENSIONS},
  {"ALC_ATTRIBUTES_SIZE",                    ALC_ATTRIBUTES_SIZE},
  {"ALC_ALL_ATTRIBUTES",                     ALC_ALL_ATTRIBUTES},
  {"ALC_MAJOR_VERSION",                      ALC_MAJOR_VERSION},
  {"ALC_MINOR_VERSION",                      ALC_MINOR_VERSION},
  {"ALC_EFX_MAJOR_VERSION",		ALC_EFX_MAJOR_VERSION},
  {"ALC_EFX_MINOR_VERSION",		ALC_EFX_MINOR_VERSION}
};

static const _intBufferData _oalContextEnumValue[MAX_ENUM] =
{
    {0, 1, (void *)&_oalContextEnumValueDeclaration[0]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[1]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[2]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[3]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[4]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[5]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[6]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[7]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[8]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[9]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[10]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[11]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[12]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[13]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[14]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[15]},
    {0, 1, (void *)&_oalContextEnumValueDeclaration[16]}
};

static const void *_oalContextEnumValuePtr[MAX_ENUM] =
{
    (void *)&_oalContextEnumValue[0],
    (void *)&_oalContextEnumValue[1],
    (void *)&_oalContextEnumValue[2],
    (void *)&_oalContextEnumValue[3],
    (void *)&_oalContextEnumValue[4],
    (void *)&_oalContextEnumValue[5],
    (void *)&_oalContextEnumValue[6],
    (void *)&_oalContextEnumValue[7],
    (void *)&_oalContextEnumValue[8],
    (void *)&_oalContextEnumValue[9],
    (void *)&_oalContextEnumValue[10],
    (void *)&_oalContextEnumValue[11],
    (void *)&_oalContextEnumValue[12],
    (void *)&_oalContextEnumValue[13],
    (void *)&_oalContextEnumValue[14],
    (void *)&_oalContextEnumValue[15],
    (void *)&_oalContextEnumValue[16]
};

static const _intBuffers _oalContextEnumValues =
{
    0,
    _OAL_ENUM,
    MAX_ENUM,
    MAX_ENUM,
    MAX_ENUM,
    (_intBufferData **)&_oalContextEnumValuePtr
};

static const char *_oalContextErrorStrings[] =
{
    "Invalid device specifier.",
    "Invalid context specifier.",
    "Invalid parameter",
    "Invalid enum parameter value.",
    "Unable to allocate memory.",
    0
};

ALCenum
__oalContextSetErrorNormal(ALCenum error)
{
    static char been_here_before = 0;
    static ALCenum _ret = ALC_NO_ERROR;
    ALCenum ret = _ret;

    if (!been_here_before)
    {
        _intBufferData *dptr;

        _AL_LOG(LOG_DEBUG, __FUNCTION__);

        /*
         * safegaurd to prevent calling this function within _oalGetCurrentContext
         */
        been_here_before = 1;

        dptr = _oalGetCurrentContext();
        if (dptr)
        {
            _oalContext *ctx;

            ctx = _intBufGetDataPtr(dptr);
            ret = ctx->error;
            ctx->error = error;

        } else {
            ret = _ret;
        }
        _ret = error;

        been_here_before = 0;
    }

    return ret;
}

ALCenum
__oalContextSetErrorReport(ALCenum error, char *file, int line)
{
    ALenum ret = __oalContextSetErrorNormal(error);
    if (error != ALC_NO_ERROR) printf("at %d in file %s:\n", line, file);
    return ret;
}

_intBufferData *
_oalGetCurrentDevice()
{
    _intBufferData *dptr_dev = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (_oalCurrentContext != UINT_MAX)
    {
        unsigned int dev_pos;

        dev_pos = _intBufIdToPos(_oalDeviceToId(_oalCurrentContext));
        if (dev_pos != UINT_MAX) {
            dptr_dev = _intBufGetNoLock(_oalDevices, _OAL_DEVICE, dev_pos);
        }
    }

    return dptr_dev;
}

_intBufferData *
_oalGetCurrentContext()
{
    _intBufferData *dptr_ctx = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (_oalCurrentContext != UINT_MAX)
    {
        unsigned int ctx_pos;

        ctx_pos = _intBufIdToPos(_oalContextMask(_oalCurrentContext));
        if (ctx_pos != UINT_MAX)
        {
            _intBufferData *dptr_dev = _oalGetCurrentDevice();
            if (dptr_dev)
            {
                _oalDevice *dev = _intBufGetDataPtr(dptr_dev);
                dptr_ctx = _intBufGetNoLock(dev->contexts,_OAL_CONTEXT, ctx_pos);
            }
        }
    }

    return dptr_ctx;
}

/**
 * Add a context to a device
 **/
static _intBufferData *
_oalDeviceContextAdd(_oalDevice *d)
{
    _intBufferData *dptr = 0;
    _oalContext *ctx = 0;
    unsigned int r = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!d) return 0;

    if (d->contexts == 0)
        r = _intBufCreate(&d->contexts, _OAL_CONTEXT);

    if (r != UINT_MAX)
    {
        ctx = calloc(1, sizeof(_oalContext));
        if (ctx)
        {
            ctx->parent_device = d;

            r = _intBufAddData(d->contexts, _OAL_CONTEXT, ctx);
            if (r != UINT_MAX)
            {
                d->current_context = r;
                dptr = _intBufGetNoLock(d->contexts, _OAL_CONTEXT, r);
            }
        }
    } 
    else {
        _oalContextSetError(ALC_OUT_OF_MEMORY);
    }

    if (!dptr)
    {
        _oalContextSetError(ALC_OUT_OF_MEMORY);
        if (ctx) free(ctx);
    }

    return dptr;
}

_oalDevice *
_oalFindDeviceById(uint32_t id)
{
    _oalDevice *dev = 0;
    unsigned int i, num;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    i = _intBufIdToPos(id);
    if (i != UINT_MAX)
    {
        num = _intBufGetNumNoLock(_oalDevices, _OAL_DEVICE);
        if (i < num)
        {
            _intBufferData *dptr;
            dptr = _intBufGetNoLock(_oalDevices, _OAL_DEVICE, i);
            dev = _intBufGetDataPtr(dptr);
        }
    }

    return dev;
}

static _intBufferData *
_oalFindContextByDeviceId(uint32_t id)
{
    const _intBuffers *ctx;
    _intBufferData *dptr = 0;
    _oalDevice *dev;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    dev = _oalFindDeviceById(id);
    if (dev)
    {
        if (dev->current_context != UINT_MAX)
        {
            ctx = dev->contexts;
            if (ctx) {
                dptr = _intBufGetNoLock(ctx, _OAL_CONTEXT, dev->current_context);
            }
        }
    }

    return dptr;
}

static void
_oalDestroyContextByPos(void *device, unsigned int ctx_num)
{
    _oalDevice *d = (_oalDevice *)device;
    _oalContext *ctx;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    assert(d);

    if (d->current_context == ctx_num) {
        d->current_context = UINT_MAX;
    }

    aaxMixerSetState(d->lst.handle, AAX_STOPPED);
    ctx = _intBufRemove(d->contexts, _OAL_CONTEXT, ctx_num, AL_FALSE);
    if (ctx)
    {
        _intBufErase(&ctx->sources, _OAL_SOURCE, _oalRemoveSourceByPos, ctx);
        free(ctx->state);
        free(ctx);
    }
}

static void
_oalSourcesCreate(void *context)
{
    _oalContext *ctx = (_oalContext *)context;

    assert(ctx);

    if (ctx->sources == 0)
    {
        unsigned int r;
        r = _intBufCreate(&ctx->sources, _OAL_SOURCE);
        if (r == UINT_MAX) _oalContextSetError(ALC_OUT_OF_MEMORY);
    }
}
