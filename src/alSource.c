/*
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
# include "config.h"
#endif

#if HAVE_ASSERT_H
#include <assert.h>
#endif
#include <math.h>

#include <aax.h>
#include <aaxdefs.h>
#include <AL/al.h>
#include <AL/alext.h>

#include <base/types.h>
#include <base/geometry.h>

#include "api.h"
#include "aax_support.h"

static _intBuffers *_oalGetSources(void *);
static const _intBufferData *_oalFindSourceById(ALuint, _intBuffers*, ALuint *);

ALboolean
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

void
alGenSources(ALsizei num, ALuint *ids)
{
    _intBufferData *dptr = 0;
    _intBuffers *cs = 0;
    _oalContext *ctx = 0;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!num) return;	/* nop */

    if (ids == 0)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalGetCurrentContext();
    if (!dptr) return;

    ctx = _intBufGetDataPtr(dptr);
    cs = _oalGetSources(ctx);
    if (cs)
    {
        ALuint pos = UINT_MAX;
        ALsizei nsrcs, i = 0;

        nsrcs = aaxMixerGetNoMonoSources();
        if (nsrcs < num) num = 0;
        for (i=0; i<num; i++)
        {
            _oalSource *src = calloc(1, sizeof(_oalSource));
            if (src != NULL)
            {
                src->handle = aaxEmitterCreate();
                if (!src->handle)
                {
                    --i;
                    pos = UINT_MAX;
                    break;
                }
                src->mode =  AAX_ABSOLUTE;
                aaxEmitterSetMode(src->handle, AAX_POSITION, src->mode);
                if ((pos = _intBufAddData(cs, _OAL_SOURCE, src)) == UINT_MAX)
                {
                    break;
                }
                ids[i] = _intBufPosToId(pos);
            }
            else _oalStateSetError(AL_OUT_OF_MEMORY);
        }

        if (pos == UINT_MAX)
        {
            ALsizei pos, r;
            for (r=0; r<i; r++)
            {
                _oalSource *src;
                pos = _intBufIdToPos(ids[r]);
                src = _intBufRemove(cs, _OAL_SOURCE, pos, AAX_FALSE);
                if (src->handle) {
                    aaxEmitterDestroy(src->handle);
                }
                free(src);
            }

            _oalStateSetError(AL_OUT_OF_MEMORY);
        }
    }
}

void
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
                    src = _intBufRemove(cs, _OAL_SOURCE, pos[i], AAX_FALSE);
                    if (src)
                    {
                        aaxEmitterDestroy(src->handle);
                        free(src);
                    }
                }
            }
            else {
                _oalStateSetError(AL_INVALID_NAME);
            }

            free(pos);
        }
    }
    else _oalStateSetError(AL_OUT_OF_MEMORY);
}

void
alSourcef(ALuint id, ALenum attrib, ALfloat value)
{
    unsigned int ival = (unsigned int)value;
    const _intBufferData *dptr;
    _oalSource *src;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (value < 0.0f)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (!dptr)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    src = _intBufGetDataPtr(dptr);
    switch(attrib)
    {
    case AL_GAIN:
        aaxEmitterSetGain(src->handle, value);
        break;
    case AL_MIN_GAIN:
        aaxEmitterSetGainMinMax(src->handle,value,AAX_FPNONE);
        break;
    case AL_MAX_GAIN:
        aaxEmitterSetGainMinMax(src->handle,AAX_FPNONE,value);
        break;
    case AL_REFERENCE_DISTANCE:
        aaxEmitterSetReferenceDistance(src->handle, value);
        break;
    case AL_ROLLOFF_FACTOR:
        aaxEmitterSetRolloffFactor(src->handle, value);
        break;
    case AL_MAX_DISTANCE:
        aaxEmitterSetMaxDistance(src->handle, value);
        break;
    case AL_PITCH:
        aaxEmitterSetPitch(src->handle, value);
        break;
    case AL_CONE_INNER_ANGLE:
        aaxEmitterSetAudioCone(src->handle, value*GMATH_DEG_TO_RAD, AAX_FPNONE,
                                                        AAX_FPNONE);
        break;
    case AL_CONE_OUTER_ANGLE:
        aaxEmitterSetAudioCone(src->handle, AAX_FPNONE, value*GMATH_DEG_TO_RAD,
                                                        AAX_FPNONE);
        break;
    case AL_CONE_OUTER_GAIN:
        aaxEmitterSetAudioCone(src->handle, AAX_FPNONE, AAX_FPNONE, value);
        break;
    case AL_SEC_OFFSET:
        aaxEmitterSetOffsetSec(src->handle, value);
        break;
    case AL_SAMPLE_OFFSET:
    {
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        unsigned int offs = _oalOffsetInSamplesToAAXOffset(ival, tracks);
         aaxEmitterSetOffset(src->handle, offs, AAX_SAMPLES);
        break;
    }
    case AL_BYTE_OFFSET:
    {
        enum aaxFormat fmt = aaxEmitterGetFormat(src->handle);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        unsigned long offs = _oalOffsetInBytesToAAXOffset(ival, tracks, fmt);
        aaxEmitterSetOffset(src->handle, offs, AAX_SAMPLES);
        break;
    }
    /* AL_AAX_frequency_filter */
    case AL_AAX_FREQUENCY_FILTER_GAINLF:
        aaxEmitterSetFrequencyFilter(src->handle,AAX_FPNONE,value,AAX_FPNONE);
        break;
    case AL_AAX_FREQUENCY_FILTER_GAINHF:
        aaxEmitterSetFrequencyFilter(src->handle,AAX_FPNONE,AAX_FPNONE,value);
        break;
    case AL_AAX_FREQUENCY_FILTER_CUTOFF_FREQUENCY:
        aaxEmitterSetFrequencyFilter(src->handle,value,AAX_FPNONE,AAX_FPNONE);
        break;
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alSourcefv(ALuint id, ALenum attrib, const ALfloat *values)
{
    unsigned int ival = (unsigned int)*values;
    const _intBufferData *dptr;
    char finished = 1;
    _oalSource *src;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (!dptr)
    {
      _oalStateSetError(AL_INVALID_NAME);
      return;
    }

    src = _intBufGetDataPtr(dptr);
    switch(attrib)
    {
    case AL_POSITION:
    {
        aaxMtx4f mtx;
        memcpy(src->pos, values, sizeof(aaxVec3f));
        aaxMatrixSetDirection(mtx, src->pos, src->at);
        aaxEmitterSetMatrix(src->handle, mtx);
        break;
    }
    case AL_DIRECTION:
    {
        aaxMtx4f mtx;
        memcpy(src->at, values, sizeof(aaxVec3f));
        aaxMatrixSetDirection(mtx, src->pos, src->at);
        aaxEmitterSetMatrix(src->handle, mtx);
        if (!values[0] && !values[1] && !values[2]) {
            aaxEmitterSetAudioCone(src->handle, 360.0f, AAX_FPNONE, AAX_FPNONE);
        }
        break;
    }
    case AL_VELOCITY:
        aaxEmitterSetVelocity(src->handle, values);
        break;
    default:
        finished = 0;
    }

    if (finished || (*values < 0.0f))
    {
        if (!finished) {
            _oalStateSetError(AL_INVALID_VALUE);
        }
        return;
    }

    switch (attrib)
    {
    case AL_GAIN:
        aaxEmitterSetGain(src->handle, *values);
        break;
    case AL_MIN_GAIN:
        aaxEmitterSetGainMinMax(src->handle, *values, AAX_FPNONE);
        break;
    case AL_MAX_GAIN:
        aaxEmitterSetGainMinMax(src->handle, AAX_FPNONE, *values);
        break;
    case AL_REFERENCE_DISTANCE:
        aaxEmitterSetReferenceDistance(src->handle, *values);
        break;
    case AL_ROLLOFF_FACTOR:
        aaxEmitterSetRolloffFactor(src->handle, *values);
        break;
    case AL_MAX_DISTANCE:
        aaxEmitterSetMaxDistance(src->handle, *values);
        break;
    case AL_PITCH:
        aaxEmitterSetPitch(src->handle, *values);
        break;
    case AL_CONE_INNER_ANGLE:
        aaxEmitterSetAudioCone(src->handle,
                                      *values*GMATH_DEG_TO_RAD, AAX_FPNONE, AAX_FPNONE);
        break;
    case AL_CONE_OUTER_ANGLE:
        aaxEmitterSetAudioCone(src->handle,
                                      AAX_FPNONE, *values*GMATH_DEG_TO_RAD, AAX_FPNONE);
        break;
    case AL_CONE_OUTER_GAIN:
        aaxEmitterSetAudioCone(src->handle, AAX_FPNONE, AAX_FPNONE, *values);
        break;
    case AL_SEC_OFFSET:
        aaxEmitterSetOffsetSec(src->handle, *values);
        break;
    case AL_SAMPLE_OFFSET:
    {
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        unsigned int offs = _oalOffsetInSamplesToAAXOffset(ival, tracks);
        aaxEmitterSetOffset(src->handle, offs, AAX_SAMPLES);
        break;
    }
    case AL_BYTE_OFFSET:
    {
        enum aaxFormat fmt = aaxEmitterGetFormat(src->handle);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        unsigned long offs = _oalOffsetInBytesToAAXOffset(ival, tracks, fmt);
        aaxEmitterSetOffset(src->handle, offs, AAX_SAMPLES);
        break;
    }
    /* AL_AAX_frequency_filter */
    case AL_AAX_FREQUENCY_FILTER_PARAMS:
        aaxEmitterSetFrequencyFilter(src->handle,values[0],values[1],values[2]);
        break;
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alSource3f(ALuint id, ALenum attrib, ALfloat v1, ALfloat v2, ALfloat v3)
{
    ALfloat fv[3];

    fv[0] = v1;
    fv[1] = v2;
    fv[2] = v3;
    alSourcefv(id, attrib, (ALfloat *)&fv);
}

void
alSourcei(ALuint id, ALenum attrib, ALint value)
{
    const _intBufferData *dptr;
    float fval = (float)value;
    _oalSource *src;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (value < 0)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (!dptr)
    {
      _oalStateSetError(AL_INVALID_NAME);
      return;
    }

    src = _intBufGetDataPtr(dptr);
    switch(attrib)
    {
    case AL_SOURCE_STATE:
        if (value == AL_PLAYING) alSourcePlayv(1, &id);
        else if (value == AL_STOPPED) alSourceStopv(1, &id);
        else if (value == AL_PAUSED) alSourcePausev(1, &id);
#if 0
        else if (value == AL_PROCESSED) _SRC_SET_PROCESSED(src->handle);
#endif
        else _oalStateSetError(AL_INVALID_VALUE);
        break;
    case AL_SOURCE_RELATIVE:
        if (value == AL_TRUE) {
            src->mode = AAX_RELATIVE;
        } else {
            src->mode = AAX_ABSOLUTE;
        }
        aaxEmitterSetMode(src->handle, AAX_POSITION, src->mode);
        break;
    case AL_SOURCE_TYPE:
        break;
    case AL_REFERENCE_DISTANCE:
        aaxEmitterSetReferenceDistance(src->handle, fval);
        break;
    case AL_ROLLOFF_FACTOR:
        aaxEmitterSetRolloffFactor(src->handle, fval);
        break;
    case AL_MAX_DISTANCE:
        aaxEmitterSetMaxDistance(src->handle, fval);
        break;
    case AL_CONE_INNER_ANGLE:
        aaxEmitterSetAudioCone(src->handle, fval*GMATH_DEG_TO_RAD, AAX_FPNONE,
                                                        AAX_FPNONE);
        break;
    case AL_CONE_OUTER_ANGLE:
        aaxEmitterSetAudioCone(src->handle, AAX_FPNONE, fval*GMATH_DEG_TO_RAD,
                                                        AAX_FPNONE);
        break;
    case AL_BUFFER:
        if (value)
        {
            const _intBufferData *dptr_buf;
            unsigned int pos;

            dptr_buf = _oalFindBufferById(value, &pos);
            if (dptr_buf)
            {
                aaxBuffer *buf = _intBufGetDataPtr(dptr_buf);
                aaxEmitterAddBuffer(src->handle, buf);
                if (aaxBufferGetNoTracks(buf) > 1) {
                    aaxEmitterSetMode(src->handle, AAX_POSITION, AAX_MODE_NONE);
                } else {
                    aaxEmitterSetMode(src->handle, AAX_POSITION, src->mode);
                }
            }
            else {
                _oalStateSetError(AL_INVALID_VALUE);
            }
        }
        else	
        {
            /*
             * specifying a NULL buffer means removing all attached buffers 
             */
            unsigned int num;
            num = aaxEmitterGetNoBuffers(src->handle, AAX_PROCESSED);
            if (num > 0)
            {
                unsigned int i = num;
                do {
                    aaxEmitterRemoveBuffer(src->handle);
                } while (--i != 0);
            }
        }
        break;
    case AL_LOOPING:
        aaxEmitterSetLooping(src->handle, value);
        break;
    case AL_SEC_OFFSET:
        aaxEmitterSetOffsetSec(src->handle, fval);
        break;
    case AL_SAMPLE_OFFSET:
    {
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        unsigned int offs = _oalOffsetInSamplesToAAXOffset(value, tracks);
        aaxEmitterSetOffset(src->handle, offs, AAX_SAMPLES);
        break;
    }
    case AL_BYTE_OFFSET:
    {
        enum aaxFormat fmt = aaxEmitterGetFormat(src->handle);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        unsigned long offs = _oalOffsetInBytesToAAXOffset(value, tracks, fmt);
        aaxEmitterSetOffset(src->handle, offs, AAX_SAMPLES);
        break;
    }
    /* AL_AAX_frequency_filter */
    case AL_AAX_FREQUENCY_FILTER_ENABLE:
    {
        aaxFilter f = aaxEmitterGetFilter(src->handle, AAX_FREQUENCY_FILTER);
        aaxFilterSetState(f, value ? AAX_TRUE : AAX_FALSE);
        aaxEmitterSetFilter(src->handle, f);
        aaxFilterDestroy(f);
        break;
    }
    case AL_AAX_FREQUENCY_FILTER_CUTOFF_FREQUENCY:
        aaxEmitterSetFrequencyFilter(src->handle, value, AAX_FPNONE, AAX_FPNONE);
        break;
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alSourceiv(ALuint id, ALenum attrib, const ALint *values)
{
    const _intBufferData *dptr;
    float fval = (float)*values;
    _oalSource *src;
    aaxVec3f vec3f;
    char finished;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (!dptr)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    finished = 1;
    src = _intBufGetDataPtr(dptr);
    switch(attrib)
    {
    case AL_POSITION:
    {
        aaxMtx4f mtx;
        src->pos[0] = (float)values[0];
        src->pos[1] = (float)values[1];
        src->pos[2] = (float)values[2];
        aaxMatrixSetDirection(mtx, src->pos, src->at);
        aaxEmitterSetMatrix(src->handle, mtx);
        break;
    }
    case AL_DIRECTION:
    {
        aaxMtx4f mtx;
        src->at[0] = (float)values[0];
        src->at[1] = (float)values[1];
        src->at[2] = (float)values[2];
        aaxMatrixSetDirection(mtx, src->pos, src->at);
        aaxEmitterSetMatrix(src->handle, mtx);
        if (!values[0] && !values[1] && !values[2]) {
            aaxEmitterSetAudioCone(src->handle, 360.0f, AAX_FPNONE, AAX_FPNONE);
        }
        break;
    }
    case AL_VELOCITY:
        vec3f[0] = (float)values[0];
        vec3f[1] = (float)values[1];
        vec3f[2] = (float)values[2];
        aaxEmitterSetVelocity(src->handle, vec3f);
        break;
    case AL_BUFFER:
        if (*values)
        {
            const _intBufferData *dptr_buf;
            unsigned int pos;

            dptr_buf = _oalFindBufferById(*values, &pos);
            if (dptr_buf)
            {
                aaxBuffer *buf = _intBufGetDataPtr(dptr_buf);
                aaxEmitterAddBuffer(src->handle, buf);
                if (aaxBufferGetNoTracks(buf) > 1) {
                    aaxEmitterSetMode(src->handle, AAX_POSITION, AAX_MODE_NONE);
                } else {
                    aaxEmitterSetMode(src->handle, AAX_POSITION, src->mode);
                }
            }
            else {
                _oalStateSetError(AL_INVALID_VALUE);
            }
        }
        else
        {
            /*
             * specifying a NULL buffer means removing all attached buffers 
             */
            unsigned int num;
            num = aaxEmitterGetNoBuffers(src->handle, AAX_PROCESSED);
            if (num > 0)
            {
                unsigned int i = num;
                do {
                    aaxEmitterRemoveBuffer(src->handle);
                } while (--i != 0);
            }
        }
        break;
    default:
        finished = 0;
    }

    if (finished || (*values < 0.0f))
    {
        if (!finished) {
            _oalStateSetError(AL_INVALID_VALUE);
        }
        return;
    }

    switch (attrib)
    {
    case AL_SOURCE_RELATIVE:
        if (*values == AL_TRUE) {
            aaxEmitterSetMode(src->handle, AAX_POSITION, AAX_RELATIVE);
        } else {
            aaxEmitterSetMode(src->handle, AAX_POSITION, AAX_ABSOLUTE);
        }
        break;
    case AL_SOURCE_TYPE:
        break;
    case AL_REFERENCE_DISTANCE:
        aaxEmitterSetReferenceDistance(src->handle, fval);
        break;
    case AL_ROLLOFF_FACTOR:
        aaxEmitterSetRolloffFactor(src->handle, fval);
        break;
    case AL_MAX_DISTANCE:
        aaxEmitterSetMaxDistance(src->handle, fval);
        break;
    case AL_CONE_INNER_ANGLE:
        aaxEmitterSetAudioCone(src->handle, fval*GMATH_DEG_TO_RAD, AAX_FPNONE,
                                                        AAX_FPNONE);
        break;
    case AL_CONE_OUTER_ANGLE:
        aaxEmitterSetAudioCone(src->handle, AAX_FPNONE, fval*GMATH_DEG_TO_RAD,
                                                        AAX_FPNONE);
        break;
    case AL_LOOPING:
        aaxEmitterSetLooping(src->handle, *values);
        break;
    case AL_SEC_OFFSET:
        aaxEmitterSetOffsetSec(src->handle, fval);
        break;
    case AL_SAMPLE_OFFSET:
    {
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        unsigned int offs = _oalOffsetInSamplesToAAXOffset(*values, tracks);
        aaxEmitterSetOffset(src->handle, offs, AAX_SAMPLES);
        break;
    }
    case AL_BYTE_OFFSET:
    {
        enum aaxFormat fmt = aaxEmitterGetFormat(src->handle);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        unsigned long offs =_oalOffsetInBytesToAAXOffset(*values, tracks, fmt);
        aaxEmitterSetOffset(src->handle, offs, AAX_SAMPLES);
        break;
    }
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alSource3i(ALuint id, ALenum attrib, ALint v1, ALint v2, ALint v3)
{
    ALint iv[3];

    iv[0] = v1;
    iv[1] = v2;
    iv[2] = v3;
    alSourceiv(id, attrib, (ALint *)&iv);
}

void
alGetSourcef(ALuint id, ALenum attrib, ALfloat *value)
{
    const _intBufferData *dptr;
    _oalSource *src;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!value)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (!dptr)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    src = _intBufGetDataPtr(dptr);
    switch(attrib)
    {
    case AL_GAIN:
        *value = aaxEmitterGetGain(src->handle, AAX_LINEAR);
        break;
    case AL_MIN_GAIN:
        aaxEmitterGetGainMinMax(src->handle, value, NULL, AAX_LINEAR);
        break;
    case AL_MAX_GAIN:
        aaxEmitterGetGainMinMax(src->handle, NULL, value, AAX_LINEAR);
        break;
    case AL_REFERENCE_DISTANCE:
        *value = aaxEmitterGetReferenceDistance(src->handle);
        break;
    case AL_ROLLOFF_FACTOR:
        *value = aaxEmitterGetRolloffFactor(src->handle);
        break;
    case AL_MAX_DISTANCE:
        *value = aaxEmitterGetMaxDistance(src->handle);
        break;
    case AL_PITCH:
        *value = aaxEmitterGetPitch(src->handle);
        break;
    case AL_CONE_INNER_ANGLE:
        aaxEmitterGetAudioCone(src->handle, value, NULL, NULL);
        *value *= GMATH_RAD_TO_DEG;
        break;
    case AL_CONE_OUTER_ANGLE:
        aaxEmitterGetAudioCone(src->handle, NULL, value, NULL);
        *value *= GMATH_RAD_TO_DEG;
        break;
    case AL_CONE_OUTER_GAIN:
        aaxEmitterGetAudioCone(src->handle, NULL, NULL, value);
        break;
    case AL_SEC_OFFSET:
        *value = aaxEmitterGetOffsetSec(src->handle);
        break;
    case AL_SAMPLE_OFFSET:
    {
        unsigned int offs = aaxEmitterGetOffset(src->handle, AAX_SAMPLES);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        *value = (float)_oalAAXOffsetToOffsetInSamples(offs, tracks);
        break;
    }
    case AL_BYTE_OFFSET:
    {
        unsigned int offs = aaxEmitterGetOffset(src->handle, AAX_SAMPLES);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        enum aaxFormat fmt = aaxEmitterGetFormat(src->handle);
        *value = (float)_oalAAXOffsetToOffsetInBytes(offs, tracks, fmt);
        break;
    }
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alGetSourcefv(ALuint id, ALenum attrib, ALfloat *values)
{
    const _intBufferData *dptr;
    _oalSource *src;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (!dptr)
    {
      _oalStateSetError(AL_INVALID_NAME);
      return;
    }

    src = _intBufGetDataPtr(dptr);
    switch(attrib)
    {
    case AL_GAIN:
        *values = aaxEmitterGetGain(src->handle, AAX_LINEAR);
        break;
    case AL_MIN_GAIN:
        aaxEmitterGetGainMinMax(src->handle, values, NULL, AAX_LINEAR);
        break;
    case AL_MAX_GAIN:
        aaxEmitterGetGainMinMax(src->handle, NULL, values, AAX_LINEAR);
        break;
    case AL_REFERENCE_DISTANCE:
        *values = aaxEmitterGetReferenceDistance(src->handle);
        break;
    case AL_ROLLOFF_FACTOR:
        *values = aaxEmitterGetRolloffFactor(src->handle);
        break;
    case AL_MAX_DISTANCE:
        *values = aaxEmitterGetMaxDistance(src->handle);
        break;
    case AL_PITCH:
        *values = aaxEmitterGetPitch(src->handle);
        break;
    case AL_CONE_INNER_ANGLE:
        aaxEmitterGetAudioCone(src->handle, values, NULL, NULL);
        *values *= GMATH_RAD_TO_DEG;
        break;
    case AL_CONE_OUTER_ANGLE:
        aaxEmitterGetAudioCone(src->handle, NULL, values, NULL);
        *values *= GMATH_RAD_TO_DEG;
        break;
    case AL_CONE_OUTER_GAIN:
        aaxEmitterGetAudioCone(src->handle, NULL, NULL, values);
        break;
    case AL_POSITION:
    {
        aaxMtx4f mtx;
        aaxEmitterGetMatrix(src->handle, mtx);
        aaxMatrixGetOrientation(mtx, values, NULL, NULL);
        break;
    }
    case AL_DIRECTION:
    {
        aaxMtx4f mtx;
        aaxEmitterGetMatrix(src->handle, mtx);
        aaxMatrixGetOrientation(mtx, NULL, values, NULL);
        break;
    }
    case AL_VELOCITY:
        aaxEmitterGetVelocity(src->handle, values);
        break;
    case AL_SEC_OFFSET:
        *values = aaxEmitterGetOffsetSec(src->handle);
        break;
    case AL_SAMPLE_OFFSET:
    {
        unsigned int offs = aaxEmitterGetOffset(src->handle, AAX_SAMPLES);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        *values = (float)_oalAAXOffsetToOffsetInSamples(offs, tracks);
        break;
    }
    case AL_BYTE_OFFSET:
    {
        unsigned int offs = aaxEmitterGetOffset(src->handle, AAX_SAMPLES);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        enum aaxFormat fmt = aaxEmitterGetFormat(src->handle);
        *values = (float)_oalAAXOffsetToOffsetInBytes(offs, tracks, fmt);
        break;
    }
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alGetSource3f(ALuint id, ALenum attrib, ALfloat *v1, ALfloat *v2, ALfloat *v3)
{
    ALfloat vf[3];

    alGetSourcefv(id, attrib, (ALfloat *)&vf);
    *v1 = vf[0];
    *v2 = vf[1];
    *v3 = vf[2];
}

void
alGetSourcei(ALuint id, ALenum attrib, ALint *value)
{
    const _intBufferData *dptr;
    _oalSource *src;
    float fval;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!value)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    dptr= _oalFindSourceById(id, 0, &pos);
    if (!dptr)
    {
      _oalStateSetError(AL_INVALID_NAME);
      return;
    }

    src = _intBufGetDataPtr(dptr);
    switch(attrib)
    {
    case AL_SOURCE_STATE:
    {
        enum aaxState state = aaxEmitterGetState(src->handle);
        if (state == AAX_INITIALIZED) *value = AL_INITIAL;
        else if (state == AAX_PLAYING) *value = AL_PLAYING;
        else if (state == AAX_STOPPED) *value = AL_STOPPED;
        else if (state == AAX_SUSPENDED) *value = AL_PAUSED;
        else if (state == AAX_PROCESSED) *value = AL_PROCESSED;
        break;
    }
    case AL_LOOPING:
        *value = aaxEmitterGetLooping(src->handle);
        break;
    case AL_SOURCE_TYPE:
    {
        unsigned int num = aaxEmitterGetNoBuffers(src->handle, AAX_PLAYING);
        if (num == 0) *value = AL_UNDETERMINED;
        else if (num == 1) *value = AL_STATIC;
        else *value = AL_STREAMING;
        break;
    }
    case AL_SOURCE_RELATIVE:
        if (aaxEmitterGetMode(src->handle, AAX_POSITION) == AAX_RELATIVE) {
            *value = AL_TRUE;
        } else *value = AL_FALSE;
        break;
    case AL_BUFFERS_QUEUED:
        *value = aaxEmitterGetNoBuffers(src->handle, AAX_PLAYING);
        break;
    case AL_BUFFERS_PROCESSED:
        *value = aaxEmitterGetNoBuffers(src->handle, AAX_PROCESSED);
        break;
    case AL_REFERENCE_DISTANCE:
        *value = (ALint)aaxEmitterGetReferenceDistance(src->handle);
        break;
    case AL_ROLLOFF_FACTOR:
        *value = (ALint)aaxEmitterGetRolloffFactor(src->handle);
        break;
    case AL_MAX_DISTANCE:
        *value = (ALint)aaxEmitterGetMaxDistance(src->handle);
        break;
    case AL_CONE_INNER_ANGLE:
        aaxEmitterGetAudioCone(src->handle, &fval, NULL, NULL);
        *value = (ALint)(GMATH_RAD_TO_DEG*fval);
        break;
    case AL_CONE_OUTER_ANGLE:
        aaxEmitterGetAudioCone(src->handle, NULL, &fval, NULL);
        *value = (ALint)(GMATH_RAD_TO_DEG*fval);
        break;
    case AL_BUFFER:
    {
        const aaxBuffer buf = aaxEmitterGetBuffer(src->handle);
        if (buf)
        {
            _intBuffers *db = _oalGetBuffers();
            unsigned int pos = _intBufGetPosNoLock(db, _OAL_BUFFER, buf);
            *value = _intBufPosToId(pos);
        } else {
            *value = 0;
        }
        break;
    }
    case AL_SEC_OFFSET:
        *value = (ALint)aaxEmitterGetOffsetSec(src->handle);
        break;
    case AL_SAMPLE_OFFSET:
    {
        unsigned int offs = aaxEmitterGetOffset(src->handle, AAX_SAMPLES);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        *value = (ALint)_oalAAXOffsetToOffsetInSamples(offs, tracks);
        break;
    }
    case AL_BYTE_OFFSET:
    {
        unsigned int offs = aaxEmitterGetOffset(src->handle, AAX_SAMPLES);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        enum aaxFormat fmt = aaxEmitterGetFormat(src->handle);
        *value = (ALint)_oalAAXOffsetToOffsetInBytes(offs, tracks, fmt);
        break;
    }
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alGetSourceiv(ALuint id, ALenum attrib, ALint *values)
{
    const _intBufferData *dptr;
    _oalSource *src;
    aaxVec3f vec3f;
    float fval;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (!dptr)
    {
      _oalStateSetError(AL_INVALID_NAME);
      return;
    }

    src = _intBufGetDataPtr(dptr);
    switch(attrib)
    {
    case AL_LOOPING:
        *values = aaxEmitterGetLooping(src->handle);
        break;
    case AL_SOURCE_TYPE:
    {
        unsigned int num = aaxEmitterGetNoBuffers(src->handle, AAX_PLAYING);
        if (num == 0) *values = AL_UNDETERMINED;
        else if (num == 1) *values = AL_STATIC;
        else *values = AL_STREAMING;
        break;
    }
    case AL_SOURCE_RELATIVE:
        if (aaxEmitterGetMode(src->handle, AAX_POSITION) == AAX_RELATIVE) {
            *values = AL_TRUE;
        } else *values = AL_FALSE;
        break;
    case AL_BUFFERS_QUEUED:
        *values = aaxEmitterGetNoBuffers(src->handle, AAX_PLAYING);
        break;
    case AL_BUFFERS_PROCESSED:
        *values = aaxEmitterGetNoBuffers(src->handle, AAX_PROCESSED);
        break;
    case AL_REFERENCE_DISTANCE:
        *values = (ALint)aaxEmitterGetReferenceDistance(src->handle);
        break;
    case AL_ROLLOFF_FACTOR:
        *values = (ALint)aaxEmitterGetRolloffFactor(src->handle);
        break;
    case AL_MAX_DISTANCE:
        *values = (ALint)aaxEmitterGetMaxDistance(src->handle);
        break;
    case AL_CONE_INNER_ANGLE:
        aaxEmitterGetAudioCone(src->handle, &fval, NULL, NULL);
        *values = (ALint)(GMATH_RAD_TO_DEG*fval);
        break;
    case AL_CONE_OUTER_ANGLE:
        aaxEmitterGetAudioCone(src->handle, NULL, &fval, NULL);
        *values = (ALint)(GMATH_RAD_TO_DEG*fval);
        break;
    case AL_POSITION:
    {
        aaxMtx4f mtx;
        aaxEmitterGetMatrix(src->handle, mtx);
        aaxMatrixGetOrientation(mtx, vec3f, NULL, NULL);
        values[0] = (ALint)vec3f[0];
        values[1] = (ALint)vec3f[1];
        values[2] = (ALint)vec3f[2];
        break;
    }
    case AL_DIRECTION:
    {
        aaxMtx4f mtx;
        aaxEmitterGetMatrix(src->handle, mtx);
        aaxMatrixGetOrientation(mtx, NULL, vec3f, NULL);
        values[0] = (ALint)vec3f[0];
        values[1] = (ALint)vec3f[1];
        values[2] = (ALint)vec3f[2];
        break;
    }
    case AL_VELOCITY:
        aaxEmitterGetVelocity(src->handle, vec3f);
        values[0] = (ALint)vec3f[0];
        values[1] = (ALint)vec3f[1];
        values[2] = (ALint)vec3f[2];
        break;
    case AL_BUFFER:
    {
        const aaxBuffer buf = aaxEmitterGetBuffer(src->handle);
        if (buf)
        {
            _intBuffers *db = _oalGetBuffers();
            unsigned int pos = _intBufGetPosNoLock(db, _OAL_BUFFER, buf);
            *values = _intBufPosToId(pos);
        } else {
            *values = 0;
        }
        break;
    }
    case AL_SEC_OFFSET:
        *values = (ALint)aaxEmitterGetOffsetSec(src->handle);
        break;
    case AL_SAMPLE_OFFSET:
    {
        unsigned int offs = aaxEmitterGetOffset(src->handle, AAX_SAMPLES);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        *values = (ALint)_oalAAXOffsetToOffsetInSamples(offs, tracks);
        break;
    }
    case AL_BYTE_OFFSET:
    {
        unsigned int offs = aaxEmitterGetOffset(src->handle, AAX_SAMPLES);
        unsigned tracks = aaxEmitterGetNoTracks(src->handle);
        enum aaxFormat fmt = aaxEmitterGetFormat(src->handle);
        *values = (ALint)_oalAAXOffsetToOffsetInBytes(offs, tracks, fmt);
        break;
    }
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alGetSource3i(ALuint id, ALenum attrib, ALint *v1, ALint *v2, ALint *v3)
{
    ALint iv[3];

    alGetSourceiv(id, attrib, (ALint *)&iv);
    *v1 = iv[0];
    *v2 = iv[1];
    *v3 = iv[2];
}

void
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
        _intBuffers *cs;
        _oalContext *ctx;
        unsigned int pos;

        ctx = _intBufGetDataPtr(dptr_ctx);
        cs = _oalGetSources(ctx);

        dptr_src = _oalFindSourceById(id, cs, &pos);
        if (dptr_src)
        {
            _oalSource *src = _intBufGetDataPtr(dptr_src);
            unsigned int i;

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
        } else {
            _oalStateSetError(AL_INVALID_NAME);
        }
    } else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

void
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
        _intBuffers *cs;
        _oalContext *ctx;
        unsigned int pos;

        ctx = _intBufGetDataPtr(dptr_ctx);
        cs = _oalGetSources(ctx);

        dptr_src = _oalFindSourceById(id, cs, &pos);
        if (dptr_src)
        {
            _oalSource *src = _intBufGetDataPtr(dptr_src);

            if (num <= aaxEmitterGetNoBuffers(src->handle, AAX_PROCESSED))
            {
                _intBuffers *db = _oalGetBuffers();
                unsigned int i;

                i=num;
                do
                {
                    aaxBuffer buf = aaxEmitterGetBufferByPos(src->handle, --i, AAX_FALSE);
                    pos = _intBufGetPosNoLock(db, _OAL_BUFFER, buf);
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
                } else {
                    _oalStateSetError(AL_INVALID_VALUE);
                }
            } else {
                _oalStateSetError(AL_INVALID_VALUE);
            }
        } else {
            _oalStateSetError(AL_INVALID_NAME);
        }
    } else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

void
alSourcePlay(ALuint id)
{
    alSourcePlayv(1, &id);
}

void
alSourcePlayv(ALsizei num, const ALuint *ids)
{
    ALsizei i = num;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (num == 0) return;

    if (ids == NULL)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    do
    {
        const _intBufferData *dptr_ctx = _oalGetCurrentContext();
        if (dptr_ctx)
        {
            _oalContext *ctx = _intBufGetDataPtr(dptr_ctx);
            _intBuffers *cs = _oalGetSources(ctx);
            const _intBufferData *dptr;
            unsigned int pos;
            _oalSource *src;

            dptr = _oalFindSourceById(ids[--i], cs, &pos);
            if (dptr)
            {
                _oalDevice *dev = (_oalDevice *)ctx->parent_device;
                src = _intBufGetDataPtr(dptr);

                if (dev->lst.frame_no == 0) {
                    aaxMixerRegisterEmitter(dev->lst.handle, src->handle);
                }
                else
                {
                     int pos = dev->lst.frame_no-1;
                     aaxAudioFrameRegisterEmitter(dev->lst.frame[pos],
                                                  src->handle);
                     aaxEmitterSetMode(src->handle, AAX_POSITION, AAX_ABSOLUTE);
                }
                dev->lst.frame_no++;
                if (dev->lst.frame_no > dev->lst.frame_max-1) {
                   dev->lst.frame_no = 0;
                }
                aaxEmitterSetState(src->handle, AAX_PLAYING);
            }
        }
    }
    while (i);
}

void
alSourcePause(ALuint id)
{
    alSourcePausev(1, &id);
}

void
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

void
alSourceStop(ALuint id)
{
    alSourceStopv(1, &id);
}

void
alSourceStopv(ALsizei num, const ALuint *ids)
{
    ALsizei i = num;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (num == 0) return;

    if (ids == NULL)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    do
    {
        const _intBufferData *dptr_ctx = _oalGetCurrentContext();
        if (dptr_ctx)
        {
            _oalContext *ctx = _intBufGetDataPtr(dptr_ctx);
            _intBuffers *cs = _oalGetSources(ctx);
            const _intBufferData *dptr;
            unsigned int pos;
            _oalSource *src;

            dptr = _oalFindSourceById(ids[--i], cs, &pos);
            if (dptr)
            {
                const _oalDevice *dev = ctx->parent_device;
                aaxConfig handle = dev->lst.handle;

                src = _intBufGetDataPtr(dptr);
                aaxEmitterSetState(src->handle, AAX_STOPPED);
                aaxMixerDeregisterEmitter(handle, src->handle);
            }
        }
    }
    while (i);
}

void
alSourceRewind(ALuint id)
{
    alSourceRewindv(1, &id);
}

void
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


/* -------------------------------------------------------------------------- */

static _intBuffers *
_oalGetSources(void *context)
{
    _oalContext *ctx = (_oalContext *)context;
    _intBufferData *dptr = 0;
    _intBuffers *cs = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!context)
    {
        dptr = _oalGetCurrentContext();
        if (dptr) ctx = _intBufGetDataPtr(dptr);
    }

    if (ctx) {
        cs = ctx->sources;
    }
    else
        _oalContextSetError(ALC_INVALID_CONTEXT);

    return cs;
}

static const _intBufferData *
_oalFindSourceById(ALuint id, _intBuffers *scs, ALuint *pos)
{
    _intBufferData *dptr_ctx = 0;
    _intBufferData *dptr_src = 0;
    _intBuffers *cs = scs;
    _oalContext *ctx;
    ALuint i, num;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    *pos = UINT_MAX;
    i = _intBufIdToPos(id);
    if (i == UINT_MAX) return 0;

    if (!scs)
    {
        dptr_ctx = _oalGetCurrentContext();
        if (dptr_ctx)
        {
            ctx = _intBufGetDataPtr(dptr_ctx);
            cs = _oalGetSources(ctx);
        }
    }

    if (cs)
    {
        num = _intBufGetMaxNumNoLock(cs, _OAL_SOURCE);
        if (i < num)
        {
            dptr_src = _intBufGetNoLock(cs, _OAL_SOURCE, i);
            if (dptr_src)
            {
                *pos = i;
            }
        }
    }

    return dptr_src;
}

void
_oalRemoveSourceByPos(void *context, unsigned int pos)
{
    _oalContext *ctx = (_oalContext *)context;
    _intBufferData *dptr_ctx = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!context)
    {
        dptr_ctx = _oalGetCurrentContext(0);
        if (dptr_ctx) ctx = _intBufGetDataPtr(dptr_ctx);
    }

    if (ctx)
    {
        _oalSource *src;
        src = _intBufRemove(ctx->sources, _OAL_SOURCE, pos, AAX_FALSE);
        if (src)
        {
            aaxEmitterSetState(src->handle, AAX_STOPPED);
            aaxEmitterDestroy(src->handle);
            free(src);
        }
    }
}

