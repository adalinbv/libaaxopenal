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
#include <assert.h>
#endif
#include <errno.h>
#ifndef NDEBUG
#if HAVE_UNISTD_H
#  include <unistd.h>
# endif
#else
# include <stdio.h>
#endif

#include <aax/defines.h>
#include <AL/alc.h>

#include <base/types.h>
#include <base/buffers.h>

#include "api.h"
#include "aax_support.h"

/* forward declarations */
const int _oalEFXVersion[];
const int _oalContextVersion[];
const char *_oalContextErrorStrings[];
static const _oalEnumValue_s _oalContextEnums[];
static const char* _oalContextExtensions[];

static unsigned int _oalCurrentContext;

static _intBufferData *_oalDeviceContextAdd(_oalDevice *);

static _intBufferData *_oalFindContextByDeviceId(uint32_t);
static void _oalSourcesCreate(void *);
static void _oalFreeContext(void*);

ALC_API ALCdevice * ALC_APIENTRY
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
                uint32_t id, devid;

                id = _intBufPosToId(pos);
                devid = _oalIdToDevice(id);
                device = INT_TO_PTR(devid);
            }
        }
    }

    return device;
}

ALC_API ALCboolean ALC_APIENTRY
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
            d->current_context = UINT_MAX;
            aaxMixerSetState(d->lst.handle, AAX_STOPPED);

            aaxDriverClose(d->lst.handle);
            aaxDriverDestroy(d->lst.handle);
            _intBufErase(&d->contexts, _OAL_CONTEXT, _oalFreeContext);
            _intBufErase(&d->buffers, _OAL_BUFFER, _oalFreeBuffer);
            free(d);

            _intBufErase(&_oalDevices, _OAL_DEVICE, free);

            return ALC_TRUE;
        }
    }
    return ALC_FALSE;
}

ALC_API ALCcontext * ALC_APIENTRY
alcCreateContext(const ALCdevice *device, const ALCint *attributes)
{
    _intBufferData *dptr_ctx = 0;
    enum aaxFormat format;
    aaxConfig handle;
    uint32_t id = 0;
    _oalContext *ctx;
    _oalDevice *d;
    int tracks;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    d = _oalFindDeviceById(_oalDeviceToId(device));
    dptr_ctx = _oalDeviceContextAdd(d);
    if (dptr_ctx)
    {
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

        format = aaxMixerGetSetup(handle, AAX_FORMAT);
        tracks = aaxMixerGetSetup(handle, AAX_TRACKS);
        d->format = _oalAAXFormatToFormat(format, tracks);
        d->frequency = aaxMixerGetSetup(handle, AAX_FREQUENCY);

        _oalStateCreate(handle, ctx);
        _oalSourcesCreate(ctx);

        _intBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
        _oalContextSetError(ALC_INVALID_DEVICE);
    }

    return INT_TO_PTR(id);
}

ALC_API ALCboolean ALC_APIENTRY
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

        num = _intBufGetNum(_oalDevices, _OAL_DEVICE);
        for (i=0; i<num; i++)
        {
            _intBufferData *dptr;

            dptr =  _intBufGet(_oalDevices, _OAL_DEVICE, i);
            if (dptr)
            {
                _oalDevice *dev;

                dev = _intBufGetDataPtr(dptr);

                dev->current_context =  UINT_MAX;
                _oalCurrentContext = UINT_MAX;

                _intBufReleaseData(dptr, _OAL_DEVICE);
            }
        }
        _intBufReleaseNum(_oalDevices, _OAL_DEVICE);

        pos = 0;
    }

    return (pos == UINT_MAX) ? AL_FALSE : AL_TRUE;
}

ALC_API void ALC_APIENTRY
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

ALC_API void ALC_APIENTRY
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

ALC_API void ALC_APIENTRY
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
            _oalContext *ctx;

            if (dev->current_context == pos) {
                dev->current_context = UINT_MAX;
            }
            aaxMixerSetState(dev->lst.handle, AAX_STOPPED);

            ctx = _intBufRemove(dev->contexts, _OAL_CONTEXT, pos, AL_FALSE);
            if (ctx) _oalFreeContext(ctx);
            return;  
        }
    }
    _oalContextSetError(ALC_INVALID_CONTEXT);
}

ALC_API ALCcontext * ALC_APIENTRY
alcGetCurrentContext(void)
{
    return INT_TO_PTR(_oalCurrentContext);
}

ALC_API ALCdevice * ALC_APIENTRY
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

ALC_API void * ALC_APIENTRY
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

ALC_API ALCenum ALC_APIENTRY
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
            _oalContext *ctx = _intBufGetDataPtr(dptr);

            ret = ctx->error;

            _intBufReleaseData(dptr, _OAL_CONTEXT);
        }
        else
        {
            _oalDevice *dev = (_oalDevice *)_oalFindDeviceById(id);
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

ALC_API ALCboolean ALC_APIENTRY
alcIsExtensionPresent(const ALCdevice *device,  const ALCchar *name)
{
    ALCboolean r = ALC_FALSE;
    unsigned int i;
    const char *e;

    if (!name)
    {
        _oalContextSetError(ALC_INVALID_VALUE);
        return ALC_FALSE;
    }

    for (i=0; (e = _oalContextExtensions[i]) != NULL; i++)
    {
        r = (!strcasecmp((const char *)name, e));
        if (r) break;
    }  

    if (!r) {
        r = _oalAAXGetCtxExtensionSupport((const char*)name);
    }

    return (r) ? AL_TRUE : AL_FALSE;
}

ALC_API ALCenum ALC_APIENTRY
alcGetEnumValue(const ALCdevice *device, const ALCchar *name)
{
    const _oalEnumValue_s *e;
    ALCenum rv = ALC_FALSE;
    unsigned int i;

    if (!name)
    {
        _oalContextSetError(ALC_INVALID_VALUE);
        return ALC_FALSE;
    }

    for (i=0; ((e = &_oalContextEnums[i]) != NULL) && e->name; i++)
    {
        if (!strcasecmp((const char *)name, e->name))
        {
            rv = e->enumVal;
            break;
        }
    } 

    if (!rv)
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

ALC_API const ALCchar * ALC_APIENTRY
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
        retstr = (char *)_oalAAXGetCtxExtensions(_oalContextExtensions);
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

/*
 * void alcGetIntegerv(ALCdevice *device, ALCenum attrib, ALCsizei size,
 *                     ALCint *value)
 */
#define N Integer
#define T ALCint
#include "alContext_template.c"


/*-------------------------------------------------------------------------- */

_intBuffers *_oalDevices = 0;

static unsigned int _oalCurrentContext = UINT_MAX;

const int _oalContextVersion[2] = {1, 1};
const int _oalEFXVersion[2] = {1, 0};

static const char* _oalContextExtensions[] =
{
  "ALC_enumeration_EXT",
  "ALC_enumerate_all_EXT",

  NULL				/* always last */
};

static const _oalEnumValue_s _oalContextEnums[] =
{
  {"ALC_FALSE",				ALC_FALSE},
  {"ALC_TRUE",				ALC_TRUE},

  {"ALC_FREQUENCY",			ALC_FREQUENCY},
  {"ALC_REFRESH",			ALC_REFRESH},
  {"ALC_SYNC",				ALC_SYNC},
  {"ALC_MONO_SOURCES",			ALC_MONO_SOURCES},
  {"ALC_STEREO_SOURCES",		ALC_STEREO_SOURCES},

  {"ALC_NO_ERROR",			ALC_NO_ERROR},
  {"ALC_INVALID_DEVICE",		ALC_INVALID_DEVICE},
  {"ALC_INVALID_CONTEXT",		ALC_INVALID_CONTEXT},
  {"ALC_INVALID_ENUM",			ALC_INVALID_ENUM},
  {"ALC_INVALID_VALUE",			ALC_INVALID_VALUE},
  {"ALC_OUT_OF_MEMORY",			ALC_OUT_OF_MEMORY},

  {"ALC_DEFAULT_DEVICE_SPECIFIER",	ALC_DEFAULT_DEVICE_SPECIFIER},
  {"ALC_DEVICE_SPECIFIER",		ALC_DEVICE_SPECIFIER},
  {"ALC_EXTENSIONS",			ALC_EXTENSIONS},

  {"ALC_ATTRIBUTES_SIZE",		ALC_ATTRIBUTES_SIZE},
  {"ALC_ALL_ATTRIBUTES",		ALC_ALL_ATTRIBUTES},

  {"ALC_MAJOR_VERSION",			ALC_MAJOR_VERSION},
  {"ALC_MINOR_VERSION",			ALC_MINOR_VERSION},

  {"ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER",ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER},
  {"ALC_CAPTURE_DEVICE_SPECIFIER",	ALC_CAPTURE_DEVICE_SPECIFIER},

  {"ALC_DEFAULT_ALL_DEVICES_SPECIFIER",	ALC_DEFAULT_ALL_DEVICES_SPECIFIER},
  {"ALC_ALL_DEVICES_SPECIFIER",		ALC_ALL_DEVICES_SPECIFIER},

  {"ALC_CAPTURE_SAMPLES",		ALC_CAPTURE_SAMPLES},

  /* extensions */
  {"ALC_FORMAT_AAX",			ALC_FORMAT_AAX},
  {"ALC_FREQUENCY_AAX",			ALC_FREQUENCY_AAX},
  {"ALC_BITS_AAX",			ALC_BITS_AAX},
  {"ALC_CHANNELS_AAX",			ALC_CHANNELS_AAX},

  {"ALC_EFX_MAJOR_VERSION",		ALC_EFX_MAJOR_VERSION},
  {"ALC_EFX_MINOR_VERSION",		ALC_EFX_MINOR_VERSION},
  {"ALC_MAX_AUXILIARY_SENDS",		ALC_MAX_AUXILIARY_SENDS},

  {NULL, 0}				/* always last */
};

const char *_oalContextErrorStrings[] =
{
    "Invalid device specifier.",
    "Invalid context specifier.",
    "Invalid enum parameter value.",
    "Invalid parameter",
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
         * safegaurd to prevent calling this function from within
         * _oalGetCurrentContext
         */
        been_here_before = 1;

        dptr = _oalGetCurrentContext();
        if (dptr)
        {
            _oalContext *ctx;

            ctx = _intBufGetDataPtr(dptr);
            ret = ctx->error;
            ctx->error = error;

            _intBufReleaseData(dptr, _OAL_CONTEXT);

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
            dptr_dev = _intBufGet(_oalDevices, _OAL_DEVICE, dev_pos);
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
                dptr_ctx =_intBufGet(dev->contexts,_OAL_CONTEXT, ctx_pos);
                _intBufReleaseData(dptr_dev, _OAL_DEVICE);
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
    unsigned int r = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!d) return 0;

    if (d->contexts == 0) {
        r = _intBufCreate(&d->contexts, _OAL_CONTEXT);
    }

    if (r != UINT_MAX)
    {
        _oalContext *ctx = calloc(1, sizeof(_oalContext));
        if (ctx)
        {
            ctx->parent_device = d;

            r = _intBufAddData(d->contexts, _OAL_CONTEXT, ctx);
            if (r != UINT_MAX)
            {
                d->current_context = r;
                dptr = _intBufGet(d->contexts, _OAL_CONTEXT, r);
            }
        }

        if (!dptr)
        {
            _oalContextSetError(ALC_OUT_OF_MEMORY);
            if (ctx) free(ctx);
        }
    } 
    else {
        _oalContextSetError(ALC_OUT_OF_MEMORY);
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
        num = _intBufGetNum(_oalDevices, _OAL_DEVICE);
        if (i < num)
        {
            _intBufferData *dptr;
            dptr = _intBufGetNoLock(_oalDevices, _OAL_DEVICE, i);
            dev = _intBufGetDataPtr(dptr);
        }
        _intBufReleaseNum(_oalDevices, _OAL_DEVICE);
    }

    return dev;
}

static _intBufferData *
_oalFindContextByDeviceId(uint32_t id)
{
    _intBuffers *ctx;
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
                dptr =_intBufGet(ctx, _OAL_CONTEXT, dev->current_context);
            }
        }
    }

    return dptr;
}

static void
_oalCtxFreeSource(void *source)
{
    _oalSource *src = (_oalSource*)source;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (src)
    {
        if (src->parent)
        {
            const _intBufferData *dptr_ctx = _oalGetCurrentContext();
            if (dptr_ctx)
            {
                _oalContext *ctx = _intBufGetDataPtr(dptr_ctx);
                const _oalDevice *dev = ctx->parent_device;
                aaxMixerDeregisterEmitter(dev->lst.handle, src->handle);
                src->parent = NULL;
            }
        }
        aaxEmitterSetState(src->handle, AAX_STOPPED);
        aaxEmitterDestroy(src->handle);
        free(src);
    }
}

static void
_oalFreeContext(void *context)
{
    _oalContext *ctx = (_oalContext*)context;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    _intBufErase(&ctx->sources, _OAL_SOURCE, _oalCtxFreeSource);
    free(ctx->state);
    free(ctx);
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
