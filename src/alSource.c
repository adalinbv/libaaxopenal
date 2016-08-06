/*
 * Copyright (C) 2007-2015 by Erik Hofman.
 * Copyright (C) 2007-2015 by Adalin B.V.
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
# include "config.h"
#endif

#if HAVE_ASSERT_H
#include <assert.h>
#endif
#include <math.h>

#include <aax/aax.h>
#include <AL/al.h>
#include <AL/alext.h>

#include <base/types.h>
#include <base/geometry.h>

#include "api.h"
#include "aax_support.h"

static _intBuffers *_oalGetSources(void *);
static const _intBufferData *_oalFindSourceById(ALuint, _intBuffers*, ALuint *);

AL_API ALboolean AL_APIENTRY
alIsSource (ALuint id)
{
    const _intBufferData *dptr;
    ALboolean ret = AL_FALSE;

    if (id)
    {
        ALuint pos;
        dptr = _oalFindSourceById(id, 0, &pos);
        if (dptr) {
            ret = AL_TRUE;
        }
    }

    return ret;
}

AL_API void AL_APIENTRY
alGenSources(ALsizei num, ALuint *ids)
{
    _intBufferData *dptr = 0;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!num) return;	/* nop */

    if (ids == 0)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalGetCurrentContext();
    if (dptr)
    {
        _oalContext *ctx = _intBufGetDataPtr(dptr);
        _intBuffers *cs = _oalGetSources(ctx);
        if (cs)
        {
            ALuint pos = UINT_MAX;
            ALsizei i = 0;
            ALint nsrcs;

            nsrcs = _MIN(aaxMixerGetSetup(NULL, AAX_MONO_EMITTERS), 255);
            if (nsrcs < num) num = 0;
            for (i=0; i<num; i++)
            {
                _oalSource *src = calloc(1, sizeof(_oalSource));
                if (src != NULL)
                {
                    aaxFilter flt;

                    src->handle = aaxEmitterCreate();
                    if (!src->handle)
                    {
                        --i;
                        pos = UINT_MAX;
                        break;
                    }
                    src->mode = AAX_ABSOLUTE;
                    aaxEmitterSetMode(src->handle, AAX_POSITION, src->mode);

                    flt = aaxEmitterGetFilter(src->handle, AAX_VOLUME_FILTER);
                    aaxFilterSetParam(flt, AAX_MIN_GAIN, AAX_LINEAR, 0.0f);
                    aaxFilterSetParam(flt, AAX_MAX_GAIN, AAX_LINEAR, 1.0f);
                    aaxEmitterSetFilter(src->handle, flt);

                    flt = aaxEmitterGetFilter(src->handle, AAX_DISTANCE_FILTER);
                    aaxFilterSetState(flt, AAX_AL_INVERSE_DISTANCE_CLAMPED);
                    aaxEmitterSetFilter(src->handle, flt);

                    pos = _intBufAddData(cs, _OAL_SOURCE, src);
                    if (pos == UINT_MAX) break;

                    ids[i] = _intBufPosToId(pos);
                }
                else {
                    _oalStateSetError(AL_OUT_OF_MEMORY);
                }
            }

            if (pos == UINT_MAX)
            {
                ALsizei pos, r;
                for (r=0; r<i; r++)
                {
                    _oalSource *src;
                    pos = _intBufIdToPos(ids[r]);
                    src = _intBufRemove(cs, _OAL_SOURCE, pos, AL_FALSE);
                    _oalFreeSource(ctx, src);
                }

                _oalStateSetError(AL_OUT_OF_MEMORY);
            }
        }
        _intBufReleaseData(dptr, _OAL_CONTEXT);
    }
    else {
        _oalStateSetError(AL_OUT_OF_MEMORY);
    }
}

AL_API void AL_APIENTRY
alDeleteSources(ALsizei num, const ALuint *ids)
{
    const _intBufferData *dptr_ctx;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!num) return;	/* nop */
    if (ids == 0)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    dptr_ctx = _oalGetCurrentContext();
    if (dptr_ctx)
    {
        _oalContext *ctx = _intBufGetDataPtr(dptr_ctx);
        _intBuffers *cs = ctx->sources;
        unsigned int *pos;

        if ((unsigned int)num > _intBufGetMaxNumNoLock(cs, _OAL_SOURCE))
        {
            _oalStateSetError(AL_INVALID_VALUE);
            return;
        }

        pos = malloc(num * sizeof(unsigned int));
        if (pos)
        {
            const _intBufferData *dptr_src = 0;
            ALsizei i;

            for (i=0; i<num; i++)
            {
                dptr_src = _oalFindSourceById(ids[i], cs, &pos[i]);
                if (dptr_src == 0)
                    break;
            }

            /*
             * if no errors occurred, start deleting.
             */
            if (dptr_src)
            {
                for (i=0; i<num; i++)
                {
                    _oalSource *src;
                    src = _intBufRemove(cs, _OAL_SOURCE, pos[i], AL_FALSE);
                    _oalFreeSource(ctx, src);
                }
            }
            else {
                _oalStateSetError(AL_INVALID_NAME);
            }

            free(pos);
        }
        _intBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
        _oalStateSetError(AL_OUT_OF_MEMORY);
    }
}

AL_API void AL_APIENTRY
alSourceQueueBuffers(ALuint id, ALsizei num, const ALuint *ids)
{
    const _intBufferData *dptr_ctx;

    if (!num) return;	/* nop */

    if (!ids)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    dptr_ctx = _oalGetCurrentContext();
    if (dptr_ctx)
    {
        const _intBufferData *dptr_src;
        _oalContext *ctx = _intBufGetDataPtr(dptr_ctx);
        _intBuffers *cs = _oalGetSources(ctx);
        unsigned int pos;

        dptr_src = _oalFindSourceById(id, cs, &pos);
        if (dptr_src)
        {
            _oalSource *src = _intBufGetDataPtr(dptr_src);
            int i;

            for (i=0; i<num; i++)
            {
                const _intBufferData *dptr_buf;
                ALuint value = ids[i];

                dptr_buf = _oalFindBufferById(value, &pos);
                if (dptr_buf)
                {
                    aaxBuffer *buf = _intBufGetDataPtr(dptr_buf);
                    aaxEmitterAddBuffer(src->handle, buf);
                }
                else {
                    _oalStateSetError(AL_INVALID_NAME);
                }
            }
        }
        else {
            _oalStateSetError(AL_INVALID_NAME);
        }
        _intBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

AL_API void AL_APIENTRY
alSourceUnqueueBuffers(ALuint id, ALsizei num, ALuint *ids)
{
    const _intBufferData *dptr_ctx;

    if (!num) return;     /* nop */

    if (!ids)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    dptr_ctx = _oalGetCurrentContext();
    if (dptr_ctx)
    {
        const _intBufferData *dptr_src;
        _oalContext *ctx = _intBufGetDataPtr(dptr_ctx);
        _intBuffers *cs = _oalGetSources(ctx);
        unsigned int pos;

        dptr_src = _oalFindSourceById(id, cs, &pos);
        if (dptr_src)
        {
            _oalSource *src = _intBufGetDataPtr(dptr_src);

            if (num <= (int)aaxEmitterGetNoBuffers(src->handle, AAX_PROCESSED))
            {
                _intBuffers *db = _oalGetBuffers(NULL);
                unsigned int i;
                aaxBuffer buf;

                i=num;
                do
                {
                    buf = aaxEmitterGetBufferByPos(src->handle, --i, AAX_FALSE);
                    pos = _intBufGetPos(db, _OAL_BUFFER, buf);
                    ids[i] = _intBufPosToId(pos);
                    if (ids[i] == 0) break;
                }
                while (i);

                if (i == 0)
                {
                    i = num;
                    do {
                        aaxEmitterRemoveBuffer(src->handle);
                    }
                    while (--i);
                }
                else {
                    _oalStateSetError(AL_INVALID_VALUE);
                }
            }
            else {
                _oalStateSetError(AL_INVALID_VALUE);
            }
        }
        else {
            _oalStateSetError(AL_INVALID_NAME);
        }
        _intBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

AL_API void AL_APIENTRY
alSourcePlay(ALuint id)
{
    alSourcePlayv(1, &id);
}

AL_API void AL_APIENTRY
alSourcePlayv(ALsizei num, const ALuint *ids)
{
    const _intBufferData *dptr_ctx;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (num == 0) return;

    if (ids == NULL)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr_ctx = _oalGetCurrentContext();
    if (dptr_ctx)
    {
        _oalContext *ctx = _intBufGetDataPtr(dptr_ctx);
        _intBuffers *cs = _oalGetSources(ctx);
        unsigned int pos;
        ALsizei i = num;

        do
        {
            const _intBufferData *dptr;

            dptr = _oalFindSourceById(ids[--i], cs, &pos);
            if (dptr)
            {
                _oalSource *src = _intBufGetDataPtr(dptr);
                if (!src->parent)
                {
                    _oalDevice *dev = (_oalDevice *)ctx->parent_device;

                    aaxMixerRegisterEmitter(dev->lst.handle, src->handle);
                    src->parent = dev->lst.handle;
                }

                if (aaxEmitterGetState(src->handle) == AAX_PLAYING) {
                    aaxEmitterSetState(src->handle, AAX_INITIALIZED);
                }
                aaxEmitterSetState(src->handle, AAX_PLAYING);
            }
        }
        while (i);

        _intBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
        _oalStateSetError(AL_OUT_OF_MEMORY);
    }
}

AL_API void AL_APIENTRY
alSourcePause(ALuint id)
{
    alSourcePausev(1, &id);
}

AL_API void AL_APIENTRY
alSourcePausev(ALsizei num, const ALuint *ids)
{
    ALsizei i;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    assert(ids);

    for (i=0; i<num; i++)
    {
        const _intBufferData *dptr;
        unsigned int pos;
        _oalSource *src;

        dptr = _oalFindSourceById(ids[i], 0, &pos);
        if (dptr)
        {
            src = _intBufGetDataPtr(dptr);
            aaxEmitterSetState(src->handle, AAX_SUSPENDED);
        }
    }
}

AL_API void AL_APIENTRY
alSourceStop(ALuint id)
{
    alSourceStopv(1, &id);
}

AL_API void AL_APIENTRY
alSourceStopv(ALsizei num, const ALuint *ids)
{
    const _intBufferData *dptr_ctx;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (num == 0) return;

    if (ids == NULL)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr_ctx = _oalGetCurrentContext();
    if (dptr_ctx)
    {
        _oalContext *ctx = _intBufGetDataPtr(dptr_ctx);
        _intBuffers *cs = _oalGetSources(ctx);
        unsigned int pos;
        ALsizei i = num;

        do
        {
            const _intBufferData *dptr;

            dptr = _oalFindSourceById(ids[--i], cs, &pos);
            if (dptr)
            {
                const _oalDevice *dev = ctx->parent_device;
                _oalSource *src = _intBufGetDataPtr(dptr);

                aaxEmitterSetState(src->handle, AAX_STOPPED);
                aaxMixerDeregisterEmitter(dev->lst.handle, src->handle);
                src->parent = NULL;
            }
        }
        while (i);

        _intBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
    }
}

AL_API void AL_APIENTRY
alSourceRewind(ALuint id)
{
    alSourceRewindv(1, &id);
}

AL_API void AL_APIENTRY
alSourceRewindv(ALsizei num, const ALuint *ids)
{
    ALsizei i;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    assert(ids);

    for (i=0; i<num; i++)
    {
        const _intBufferData *dptr;
        _oalSource *src;
        unsigned int pos;

        dptr = _oalFindSourceById(ids[i], 0, &pos);
        if (dptr)
        {
            src = _intBufGetDataPtr(dptr);
            aaxEmitterSetState(src->handle, AAX_INITIALIZED);
        }
    }
}

/*
 * void alSourcef(ALuint id, ALenum attr, ALfloat value)
 * void alSourcefv(ALuint id, ALenum attr, const ALfloat *values)
 * void alSourcef3(ALuint id, ALenum attr, ALfloat v1, ALfloat v2, ALfloat v3)
 *
 * void alGetSourcef(ALuint id, ALenum attr, ALfloat *value)
 * void alGetSourcefv(ALuint id, ALenum attrib, ALfloat *values)
 * void alGetSource3f(ALuint id, ALenum attr,
 *                               ALfloat *v1, ALfloat *v2, ALfloat *v3)
 */
#define N f
#define T ALfloat
#include "alSource_template.c"

/*
 * void alSourced(ALuint id, ALenum attr, ALdouble value)
 * void alSourcedv(ALuint id, ALenum attr, const ALdouble *values)
 * void alSourced3(ALuint id, ALenum attr,
 *                            ALdouble v1, ALdouble v2, ALdouble v3)
 *
 * void alGetSourced(ALuint id, ALenum attr, ALdouble *value)
 * void alGetSourcedv(ALuint id, ALenum attrib, ALdouble *values)
 * void alGetSource3d(ALuint id, ALenum attr,
 *                               ALdouble *v1, ALdouble *v2, ALdouble *v3)
 */
#define N d
#define T ALdouble
#include "alSource_template.c"

/*
 * void alSourcei(ALuint id, ALenum attr, ALint value)
 * void alSourceiv(ALuint id, ALenum attr, const ALint *values)
 * void alSourcei3(ALuint id, ALenum attr, ALint v1, ALint v2, ALint v3)
 *
 * void alGetSourcei(ALuint id, ALenum attr, ALint *value)
 * void alGetSourceiv(ALuint id, ALenum attrib, ALint *values)
 * void alGetSource3i(ALuint id, ALenum attr,
 *                               ALint *v1, ALint *v2, ALint *v3)
 */
#define N i
#define T ALint
#include "alSource_template.c"

/*
 * void alSourcei64(ALuint id, ALenum attr, ALint64 value)
 * void alSourcei64v(ALuint id, ALenum attr, const ALint64 *values)
 * void alSourcei643(ALuint id, ALenum attr, ALint64 v1, ALint64 v2, ALint64 v3)
 *
 * void alGetSourcei64(ALuint id, ALenum attr, ALint64 *value)
 * void alGetSourcei64v(ALuint id, ALenum attrib, ALint64 *values)
 * void alGetSource3i64(ALuint id, ALenum attr,
 *                               ALint64 *v1, ALint64 *v2, ALint64 *v3)
 */
#define N i64
#define T ALint64
#define BITSHIFT 32
#include "alSource_template.c"

/* -------------------------------------------------------------------------- */

/* AL_SOFT_source_latency */
ALEXT_API void ALEXT_APIENTRY
alSourcedSOFT(ALuint source, ALenum param, ALdouble value) {
    alSourced(source, param, value);
}

ALEXT_API void ALEXT_APIENTRY
alSource3dSOFT(ALuint source, ALenum param, ALdouble value1, ALdouble value2, ALdouble value3) {
    alSource3d(source, param, value1, value2, value3);
}

ALEXT_API void ALEXT_APIENTRY
alSourcedvSOFT(ALuint source, ALenum param, const ALdouble *values) {
    alSourcedv(source, param, values);
}

ALEXT_API void ALEXT_APIENTRY
alGetSourcedSOFT(ALuint source, ALenum param, ALdouble *value) {
    alGetSourced(source, param, value);
}

ALEXT_API void ALEXT_APIENTRY
alGetSource3dSOFT(ALuint source, ALenum param, ALdouble *value1, ALdouble *value2, ALdouble *value3) {
    alGetSource3d(source, param, value1, value2, value3);
}

ALEXT_API void ALEXT_APIENTRY
alGetSourcedvSOFT(ALuint source, ALenum param, ALdouble *values) {
    alGetSourcedv(source, param, values);
}

ALEXT_API void ALEXT_APIENTRY
alSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT value) {
    alSourcei64(source,  param, value);
}

ALEXT_API void ALEXT_APIENTRY
alSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT value1, ALint64SOFT value2, ALint64SOFT value3) {
    alSource3i64(source, param, value1, value2, value3);
}

ALEXT_API void ALEXT_APIENTRY
alSourcei64vSOFT(ALuint source, ALenum param, const ALint64SOFT *values) {
    alSourcei64v(source, param, values);
}

ALEXT_API void ALEXT_APIENTRY
alGetSourcei64SOFT(ALuint source, ALenum param, ALint64SOFT *value) {
    alGetSourcei64(source, param, value);
}

ALEXT_API void ALEXT_APIENTRY
alGetSource3i64SOFT(ALuint source, ALenum param, ALint64SOFT *value1, ALint64SOFT *value2, ALint64SOFT *value3) {
    alGetSource3i64(source, param, value1, value2, value3);
} 

ALEXT_API void ALEXT_APIENTRY
alGetSourcei64vSOFT(ALuint source, ALenum param, ALint64SOFT *values) {
    alGetSourcei64v(source, param, values);
}
/* AL_SOFT_source_latency */

static _intBuffers *
_oalGetSources(void *context)
{
    _oalContext *ctx = (_oalContext *)context;
    _intBufferData *dptr = NULL;
    _intBuffers *cs = NULL;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!ctx && ((dptr = _oalGetCurrentContext()) != NULL)) {
        ctx = _intBufGetDataPtr(dptr);
    }

    if (ctx) {
        cs = ctx->sources;
    }
    else {
        _oalContextSetError(ALC_INVALID_CONTEXT);
    } 

    if (dptr) {
        _intBufReleaseData(dptr, _OAL_CONTEXT);
    }

    return cs;
}

static const _intBufferData *
_oalFindSourceById(ALuint id, _intBuffers *scs, ALuint *rpos)
{
    _intBufferData *dptr_src = NULL;
    _intBuffers *cs = scs;
    ALuint pos;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    *rpos = UINT_MAX;

    pos = _intBufIdToPos(id);
    if (pos != UINT_MAX)
    {
        _intBufferData *dptr_ctx = NULL;
        _oalContext *ctx = NULL;

        if (!scs && ((dptr_ctx = _oalGetCurrentContext()) != NULL))
        {
            ctx = _intBufGetDataPtr(dptr_ctx);
            cs = _oalGetSources(ctx);
        }

        if (cs)
        {
            ALuint num = _intBufGetMaxNumNoLock(cs, _OAL_SOURCE);
            if (pos < num)
            {
                dptr_src = _intBufGetNoLock(cs, _OAL_SOURCE, pos);
                if (dptr_src) {
                    *rpos = pos;
                }
            }
        }

        if (dptr_ctx) {
            _intBufReleaseData(dptr_ctx, _OAL_CONTEXT);
        }
    }

    return dptr_src;
}

void
_oalFreeSource(void *context, void *source)
{
    _oalContext *ctx = (_oalContext*)context;
    _oalSource *src = (_oalSource*)source;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (src)
    {
        if (src->parent)
        {
            const _oalDevice *dev = ctx->parent_device;
            aaxMixerDeregisterEmitter(dev->lst.handle, src->handle);
            src->parent = NULL; 
        }
        aaxEmitterSetState(src->handle, AAX_STOPPED);
        aaxEmitterDestroy(src->handle);
        free(src);
    }
}

