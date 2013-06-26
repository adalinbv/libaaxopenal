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
#include <config.h>
#endif

#include <AL/al.h>
#include <AL/alext.h>

#include <aaxdefs.h>

#include "api.h"
#include "aax_support.h"

static void* null_buf = "\0";

ALboolean
alIsBuffer(ALuint id)
{
    ALuint pos;
    return _oalFindBufferById(id, &pos) ? AL_TRUE : AL_FALSE;
}

void
alGenBuffers(ALsizei num, ALuint *ids)
{
    _intBuffers *db;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!num) return;	/* nop */

    if (ids == 0)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    db = _oalGetBuffers();
    if (db)
    {
        ALuint pos = UINT_MAX;
        int i;

        for (i=0; i<num; i++)
        {
            if ((pos = _intBufAddData(db, _OAL_BUFFER, null_buf)) == UINT_MAX) {
                break;
            }

            ids[i] = _intBufPosToId(pos);
        }

        if (pos == UINT_MAX)
        {
            unsigned int r;
            for (r=i; r != 0; --r)
            {
                pos = _intBufIdToPos(ids[r]);
                _intBufRemove(db, _OAL_BUFFER, pos, AL_FALSE);
                ids[r] = 0;
            }
            _oalStateSetError(AL_OUT_OF_MEMORY);
        }
    }
}

void
alDeleteBuffers(ALsizei num, const ALuint *ids)
{
    ALuint *pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!num) return;	/* nop */

    if (ids == 0)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    pos = malloc(num * sizeof(unsigned int));
    if (pos)
    {
        const _intBufferData *dptr = 0;
        int i;

        for (i=0; i<num; i++)
        {
            if ((dptr = _oalFindBufferById(ids[i], &pos[i])) == 0) {
                break;
            }
        }

        /*
         * if no errors occurred, start deleting.
         */
        if (i == num)
        {
            _intBuffers *db = _oalGetBuffers();
            i--;
            do
            {
                aaxBuffer buf;

                buf = _intBufRemove(db, _OAL_BUFFER, pos[i], AL_FALSE);
                if (buf) {
                    aaxBufferDestroy(buf);
                }
            }
            while (i--);
        }
        else {
            _oalStateSetError(AL_INVALID_NAME);
        }
        free(pos);
    }
}

void
alBufferData(ALuint id, ALenum format,
                 const ALvoid *data, ALsizei size, ALsizei frequency)
{
    const _intBufferData *dptr;
    unsigned int pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (data == 0)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindBufferById(id, &pos);
    if (dptr)
    {
        aaxBuffer buf = _intBufGetDataPtr(dptr);
        size_t no_samples = size;
        unsigned char channels;
        enum aaxFormat aaxfmt;
        unsigned bps;

        channels = _oalGetChannelsFromFormat(format);
        aaxfmt = _oalFormatToAAXFormat(format);
        bps = aaxGetBytesPerSample(aaxfmt);
        no_samples /= (channels*bps);

        if (buf == null_buf)
        {
            const _intBufferData *dptr_dev = _oalGetCurrentDevice();
            if (dptr_dev)
            {
                _oalDevice *d = _intBufGetDataPtr(dptr_dev);
                aaxConfig config = d->lst.handle;
                aaxBuffer new_buf;

                new_buf = aaxBufferCreate(config, no_samples, channels, aaxfmt);
                if (new_buf)
                {
                    _intBuffers *db = _oalGetBuffers();
                    unsigned int pos = _intBufIdToPos(id);

                    aaxBufferSetFrequency(new_buf, frequency);
                    aaxBufferSetData(new_buf, data);
                    _intBufReplace(db, _OAL_BUFFER, pos, new_buf);
                }
                else {
                  _oalStateSetError(AL_OUT_OF_MEMORY);
                }
            }
        }
        else if (aaxfmt == aaxBufferGetFormat(buf)
             && channels == aaxBufferGetNoTracks(buf)
             && no_samples == aaxBufferGetNoSamples(buf))
        {
            aaxBufferSetData(buf, data);
        } else {
            _oalStateSetError(AL_INVALID_VALUE);
        }
    } else {
        _oalStateSetError(AL_INVALID_VALUE);
    }
}


/*
 * void alBufferi(ALuint id, ALenum attrib, ALint value)
 * void alBufferiv(ALuint id, ALenum attrib, const ALint *values) 
 * void alBuffer3i(ALuint id, ALenum attrib, ALint v1, ALint v2,ALint v3)
 *
 * void alGetBufferi(ALuint id, ALenum attrib, ALint *value)
 * void alGetBufferiv(ALuint id, ALenum attrib, ALint *values)
 * void alGetBuffer3i(ALuint id, ALenum attrib, ALint *v1, ALint *v2, ALint *v3)
 */
#define N i
#define T ALint
#include "alBuffer_template.c"

/*
 * void alBufferf(ALuint id, ALenum attrib, ALfloat value)
 * void alBufferfv(ALuint id, ALenum attrib, const ALfloat *values) 
 * void alBuffer3f(ALuint id, ALenum attrib, ALfloat v1, ALfloat v2,ALfloat v3)
 *
 * void alGetBufferf(ALuint id, ALenum attrib, ALfloat *value)
 * void alGetBufferfv(ALuint id, ALenum attrib, ALfloat *values)
 * void alGetBuffer3f(ALuint id, ALenum attrib, ALfloat *v1, ALfloat *v2, ALfloat *v3)
 */
#define N f
#define T ALfloat
#include "alBuffer_template.c"

/* -------------------------------------------------------------------------- */

_intBuffers *
_oalGetBuffers()
{
    _intBufferData *dptr;
    _intBuffers *bufs = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    dptr = _oalGetCurrentDevice();
    if (dptr)
    {
        _oalDevice *d = _intBufGetDataPtr(dptr);
        if (d->buffers == 0)
        {
            unsigned int r =  _intBufCreate(&d->buffers, _OAL_BUFFER);
            if (r == UINT_MAX) {
                _oalContextSetError(ALC_OUT_OF_MEMORY);
            }
        }
        bufs = d->buffers;
    } else {
        _oalContextSetError(ALC_INVALID_DEVICE);
    }

    return bufs;
}

_intBufferData *
_oalFindBufferById(ALuint id, ALuint *pos)
{
    _intBufferData *dptr = 0;
    ALuint n;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    n = _intBufIdToPos(id);
    if (n != UINT_MAX)
    {
        _intBuffers *db = _oalGetBuffers();
        if (db)
        {
            ALuint num = _intBufGetMaxNumNoLock(db, _OAL_BUFFER);
            if (n < num)
            {
                dptr = _intBufGetNoLock(db, _OAL_BUFFER, n);
                *pos = n;
            }
        }
    }

    return dptr;
}

void
_oalRemoveBufferByPos(void *device, unsigned int pos)
{
    _oalDevice *d = (_oalDevice *)device;
    _intBufferData *dptr_dev = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!device)
    {
        dptr_dev = _oalGetCurrentDevice();
        if (dptr_dev) {
            d = _intBufGetDataPtr(dptr_dev);
        }
    }

    if (d)
    {
        aaxBuffer buf;
        buf = _intBufRemove(d->buffers, _OAL_BUFFER, pos, AL_FALSE);
        if (buf) {
            aaxBufferDestroy(buf);
        }
    }
}
