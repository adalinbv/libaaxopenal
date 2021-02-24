/*
 * Copyright (C) 2007-2016 by Erik Hofman.
 * Copyright (C) 2007-2016 by Adalin B.V.
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


#if defined(N) && defined(T)

# define __ALSOURCEV(NAME) 	alSource##NAME##v
# define __ALSOURCE3(NAME)	alSource3##NAME
# define __ALSOURCE(NAME)	alSource##NAME
# define __ALGETSOURCEV(NAME)	alGetSource##NAME##v
# define __ALGETSOURCE3(NAME)	alGetSource3##NAME
# define __ALGETSOURCE(NAME)	alGetSource##NAME

# define ALSOURCEV(NAME)	__ALSOURCEV(NAME)
# define ALSOURCE3(NAME)	__ALSOURCE3(NAME)
# define ALSOURCE(NAME)		__ALSOURCE(NAME)
# define ALGETSOURCEV(NAME)	__ALGETSOURCEV(NAME)
# define ALGETSOURCE3(NAME)	__ALGETSOURCE3(NAME)
# define ALGETSOURCE(NAME)	__ALGETSOURCE(NAME)

# ifndef BITSHIFT
#  define BITSHIFT 0
# endif

AL_API void AL_APIENTRY
ALSOURCE3(N)(ALuint id, ALenum attrib, T v1, T v2, T v3)
{
    T Tv[3];

    Tv[0] = v1;
    Tv[1] = v2;
    Tv[2] = v3;
    ALSOURCEV(N)(id, attrib, (T*)&Tv);
}

AL_API void AL_APIENTRY
ALSOURCEV(N)(ALuint id, ALenum attrib, const T *values)
{
    const _intBufferData *dptr;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (dptr)
    {
        _oalSource *src = _intBufGetDataPtr(dptr);
        aaxEmitter emitter = src->handle;
        aaxVec3f vec3f;
        aaxMtx4d mtx;

        switch(attrib)
        {
        case AL_POSITION:
            src->pos[0] = (double)values[0];
            src->pos[1] = (double)values[1];
            src->pos[2] = (double)values[2];
            aaxMatrix64SetDirection(mtx, src->pos, src->at);
            aaxEmitterSetMatrix64(emitter, mtx);
            break;
        case AL_DIRECTION:
            src->at[0] = (float)values[0];
            src->at[1] = (float)values[1];
            src->at[2] = (float)values[2];
            if (!values[0] && !values[1] && !values[2]) {
                aaxFilter flt;
                flt = aaxEmitterGetFilter(emitter, AAX_ANGULAR_FILTER);
                aaxFilterSetParam(flt, AAX_INNER_ANGLE, AAX_DEGREES, 360.0f);
                aaxEmitterSetFilter(emitter, flt);
                aaxFilterDestroy(flt);
            }
            aaxMatrix64SetDirection(mtx, src->pos, src->at);
            aaxEmitterSetMatrix64(emitter, mtx);
            break;
        case AL_VELOCITY:
            vec3f[0] = (float)values[0];
            vec3f[1] = (float)values[1];
            vec3f[2] = (float)values[2];
            aaxEmitterSetVelocity(emitter, vec3f);
            break;
        /* AL_AAX_frequency_filter */
        case AL_FREQUENCY_FILTER_PARAMS_AAX:
        {
            aaxFilter flt;
            flt = aaxEmitterGetFilter(emitter, AAX_FREQUENCY_FILTER );
            aaxFilterSetSlot(flt, 0, AAX_LINEAR, values[0],values[1],values[2],0.0f);
            aaxEmitterSetFilter(emitter, flt);
            aaxFilterDestroy(flt);
            break;
        }
        default:
            ALSOURCE(N)(id, attrib, *values);
            break;
        }
    }
    else
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }
}

AL_API void AL_APIENTRY
ALSOURCE(N)(ALuint id, ALenum attrib, T value)
{
    const _intBufferData *dptr;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (value < 0)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (dptr)
    {
        _oalSource *src = _intBufGetDataPtr(dptr);
        aaxEmitter emitter = src->handle;
        unsigned int ival = (unsigned int)value;
        float fval = (float)value;
        aaxEffect eff;
        aaxFilter flt;
        switch(attrib)
        {
        case AL_SOURCE_STATE:
            if (ival == AL_PLAYING) alSourcePlayv(1, &id);
            else if (ival == AL_STOPPED) alSourceStopv(1, &id);
            else if (ival == AL_PAUSED) alSourcePausev(1, &id);
            else _oalStateSetError(AL_INVALID_VALUE);
            break;
        case AL_SOURCE_RELATIVE:
            if (ival == AL_TRUE) {
                src->mode = AAX_RELATIVE;
            } else {
                src->mode = AAX_ABSOLUTE;
            }
            aaxEmitterSetMode(emitter, AAX_POSITION, src->mode);
            break;
        case AL_SOURCE_TYPE:
            break;
        case AL_SAMPLE_OFFSET:
        {
            unsigned tracks = aaxEmitterGetSetup(emitter, AAX_TRACKS);
            unsigned int offs = _oalOffsetInSamplesToAAXOffset(ival, tracks);
            aaxEmitterSetOffset(emitter, offs, AAX_SAMPLES);
            break;
        }    
        case AL_BYTE_OFFSET:
        {
            enum aaxFormat fmt = aaxEmitterGetSetup(emitter, AAX_FORMAT);
            unsigned tracks = aaxEmitterGetSetup(emitter, AAX_TRACKS);
            unsigned long offs;

            offs  = _oalOffsetInBytesToAAXOffset(ival, tracks, fmt);
            aaxEmitterSetOffset(emitter, offs, AAX_SAMPLES);
            break;
        }
        case AL_DISTANCE_MODEL:
            switch (ival)
            {
            case AL_NONE:
            case AL_INVERSE_DISTANCE:
            case AL_INVERSE_DISTANCE_CLAMPED:
            case AL_LINEAR_DISTANCE:
            case AL_LINEAR_DISTANCE_CLAMPED:
            case AL_EXPONENT_DISTANCE:
            case AL_EXPONENT_DISTANCE_CLAMPED:
            case AL_INVERSE_DISTANCE_DELAY_AAX:
            case AL_INVERSE_DISTANCE_DELAY_CLAMPED_AAX:
            case AL_LINEAR_DISTANCE_DELAY_AAX:
            case AL_LINEAR_DISTANCE_DELAY_CLAMPED_AAX:
            case AL_EXPONENT_DISTANCE_DELAY_AAX:
            case AL_EXPONENT_DISTANCE_DELAY_CLAMPED_AAX:
                if (alIsEnabled(AL_SOURCE_DISTANCE_MODEL))
                {
                    aaxFilter flt;
                    char ddelay = alIsEnabled(AL_DISTANCE_DELAY_MODEL_AAX);
                    ival = _oalDistanceModeltoAAXDistanceModel(ival, ddelay);

                    flt = aaxEmitterGetFilter(emitter, AAX_DISTANCE_FILTER);
                    aaxFilterSetState(flt, ival);
                    aaxEmitterSetFilter(emitter, flt);
                    aaxFilterDestroy(flt);
                }
                break;
            default:
                _oalStateSetError(AL_INVALID_ENUM);
                break;
            }
            break;
        case AL_LOOPING:
            aaxEmitterSetMode(emitter, AAX_LOOPING, ival);
            break;
        case AL_BUFFER:
            if (ival)
            {
                const _intBufferData *dptr_buf;
                unsigned int pos, mode;

                dptr_buf = _oalFindBufferById(ival, &pos);
                if (dptr_buf)
                {
                    aaxBuffer buf = _intBufGetDataPtr(dptr_buf);

                    aaxEmitterAddBuffer(emitter, buf);
                    if (aaxBufferGetSetup(buf, AAX_TRACKS) > 1) {
                        mode = AAX_MODE_NONE;
                    } else {
                        mode = src->mode;
                    }
                    aaxEmitterSetMode(emitter, AAX_POSITION, mode);
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
                unsigned int state;
                state = aaxEmitterGetState(emitter);
                if ((state == AAX_INITIALIZED) || (state == AAX_PROCESSED))
                {
                    unsigned int num;
                    num = aaxEmitterGetNoBuffers(emitter, AAX_MAXIMUM);
                    if (num > 0)
                    {
                        unsigned int i = num;
                        do {
                            aaxEmitterRemoveBuffer(emitter);
                        } while (--i != 0);
                    }
                } else {
                   _oalStateSetError(AL_INVALID_OPERATION);
                }
            }
            break;
        case AL_GAIN:
            flt = aaxEmitterGetFilter(src->handle, AAX_VOLUME_FILTER);
            aaxFilterSetParam(flt, AAX_GAIN, AAX_LINEAR, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_MIN_GAIN:
            flt = aaxEmitterGetFilter(src->handle, AAX_VOLUME_FILTER);
            aaxFilterSetParam(flt, AAX_MIN_GAIN, AAX_LINEAR, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_MAX_GAIN:
            flt = aaxEmitterGetFilter(src->handle, AAX_VOLUME_FILTER);
            aaxFilterSetParam(flt, AAX_MAX_GAIN, AAX_LINEAR, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_REFERENCE_DISTANCE:
            flt = aaxEmitterGetFilter(src->handle, AAX_DISTANCE_FILTER);
            aaxFilterSetParam(flt, AAX_REF_DISTANCE, AAX_LINEAR, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_ROLLOFF_FACTOR:
            flt = aaxEmitterGetFilter(src->handle, AAX_DISTANCE_FILTER);
            aaxFilterSetParam(flt, AAX_ROLLOFF_FACTOR, AAX_LINEAR, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_MAX_DISTANCE:
            flt = aaxEmitterGetFilter(src->handle, AAX_DISTANCE_FILTER);
            aaxFilterSetParam(flt, AAX_MAX_DISTANCE, AAX_LINEAR, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_PITCH:
            eff = aaxEmitterGetEffect(src->handle, AAX_PITCH_EFFECT);
            aaxEffectSetParam(eff, AAX_PITCH, AAX_LINEAR, fval);
            aaxEmitterSetEffect(src->handle, eff);
            aaxEffectDestroy(eff);
            break;
        case AL_CONE_INNER_ANGLE:
            flt = aaxEmitterGetFilter(src->handle, AAX_ANGULAR_FILTER);
            aaxFilterSetParam(flt, AAX_INNER_ANGLE, AAX_DEGREES, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_CONE_OUTER_ANGLE:
            flt = aaxEmitterGetFilter(src->handle, AAX_ANGULAR_FILTER);
            aaxFilterSetParam(flt, AAX_OUTER_ANGLE, AAX_DEGREES, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_CONE_OUTER_GAIN:
            flt = aaxEmitterGetFilter(src->handle, AAX_ANGULAR_FILTER);
            aaxFilterSetParam(flt, AAX_OUTER_GAIN, AAX_LINEAR, fval);
            aaxEmitterSetFilter(src->handle, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_SEC_OFFSET:
            aaxEmitterSetOffsetSec(emitter, fval);
            break;
        /* AL_AAX_distance_delay_model */
        case AL_DISTANCE_DELAY_MODEL_AAX:
            if (ival == AL_INITIAL) {
                 aaxEmitterSetState(src->handle, AAX_UPDATE);
            }
            break;
        /* AL_AAX_frequency_filter */
        case AL_FREQUENCY_FILTER_ENABLE_AAX:
            flt = aaxEmitterGetFilter(emitter, AAX_FREQUENCY_FILTER);
            aaxFilterSetState(flt, value ? AAX_TRUE : AAX_FALSE);
            aaxEmitterSetFilter(emitter, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_FREQUENCY_FILTER_GAINLF_AAX:
            flt = aaxEmitterGetFilter(emitter, AAX_FREQUENCY_FILTER );
            aaxFilterSetParam(flt, AAX_LF_GAIN, AAX_LINEAR, fval);
            aaxEmitterSetFilter(emitter, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_FREQUENCY_FILTER_GAINHF_AAX:
            flt = aaxEmitterGetFilter(emitter, AAX_FREQUENCY_FILTER );
            aaxFilterSetParam(flt, AAX_HF_GAIN, AAX_LINEAR, fval);
            aaxEmitterSetFilter(emitter, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX:
            flt = aaxEmitterGetFilter(emitter, AAX_FREQUENCY_FILTER );
            aaxFilterSetParam(flt, AAX_CUTOFF_FREQUENCY, AAX_LINEAR, fval);
            aaxEmitterSetFilter(emitter, flt);
            aaxFilterDestroy(flt);
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }
}


AL_API void AL_APIENTRY
ALGETSOURCE3(N)(ALuint id, ALenum attrib, T *v1, T *v2, T *v3)
{
    T Tv[3];

    ALGETSOURCEV(N)(id, attrib, (T*)&Tv);
    *v1 = Tv[0];
    *v2 = Tv[1];
    *v3 = Tv[2];
}

AL_API void AL_APIENTRY
ALGETSOURCEV(N)(ALuint id, ALenum attrib, T *values)
{
    const _intBufferData *dptr;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (dptr)
    {
        _oalSource *src = _intBufGetDataPtr(dptr);
        aaxEmitter emitter = src->handle;
        aaxVec3d vec3d;
        aaxVec3f vec3f;
        aaxMtx4d mtx;

        switch(attrib)
        {
        case AL_POSITION:
            aaxEmitterGetMatrix64(emitter, mtx);
            aaxMatrix64GetOrientation(mtx, vec3d, NULL, NULL);
            values[0] = (T)vec3d[0];
            values[1] = (T)vec3d[1];
            values[2] = (T)vec3d[2];
            break;
        case AL_DIRECTION:
            aaxEmitterGetMatrix64(emitter, mtx);
            aaxMatrix64GetOrientation(mtx, NULL, vec3f, NULL);
            values[0] = (T)vec3f[0];
            values[1] = (T)vec3f[1];
            values[2] = (T)vec3f[2];
            break;
        case AL_VELOCITY:
            aaxEmitterGetVelocity(emitter, vec3f);
            values[0] = (T)vec3f[0];
            values[1] = (T)vec3f[1];
            values[2] = (T)vec3f[2];
            break;
        default:
            ALGETSOURCE(N)(id, attrib, values);
            break;
        }
    }
    else
    {
      _oalStateSetError(AL_INVALID_NAME);
      return;
    }
}

AL_API void AL_APIENTRY
ALGETSOURCE(N)(ALuint id, ALenum attrib, T *value)
{
    const _intBufferData *dptr;
    ALuint pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!value)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    dptr = _oalFindSourceById(id, 0, &pos);
    if (dptr)
    {
        _oalSource *src = _intBufGetDataPtr(dptr);
        aaxEmitter emitter = src->handle;
        aaxEffect eff;
        aaxFilter flt;

        switch(attrib)
        {
        case AL_SOURCE_STATE:
        {
            enum aaxState state = aaxEmitterGetState(emitter);
            if (state == AAX_INITIALIZED) *value = (T)AL_INITIAL;
            else if (state == AAX_PLAYING) *value = (T)AL_PLAYING;
            else if (state == AAX_STOPPED) *value = (T)AL_STOPPED;
            else if (state == AAX_SUSPENDED) *value = (T)AL_PAUSED;
            else if (state == AAX_PROCESSED)
            {
                if (aaxEmitterGetNoBuffers(emitter, AAX_MAXIMUM) > 1) {
                    *value = (T)AL_PROCESSED;
                } else {
                    *value = (T)AL_STOPPED;
                }
            }
            break;
        }
        case AL_LOOPING:
            *value = (T)aaxEmitterGetMode(emitter, AAX_LOOPING);
            break;
        case AL_SOURCE_TYPE:
        {
            unsigned int num = aaxEmitterGetNoBuffers(emitter, AAX_PLAYING);
            if (num == 0) *value = (T)AL_UNDETERMINED;
            else if (num == 1) *value = (T)AL_STATIC;
            else *value = (T)AL_STREAMING;
            break;
        }
        case AL_SOURCE_RELATIVE:
            if (aaxEmitterGetMode(emitter, AAX_POSITION) == AAX_RELATIVE) {
                *value = (T)AL_TRUE;
            } else {
                *value = (T)AL_FALSE;
            }
            break;
        case AL_BUFFERS_QUEUED:
            *value = (T)aaxEmitterGetNoBuffers(emitter, AAX_PLAYING);
            break;
        case AL_BUFFERS_PROCESSED:
            *value = (T)aaxEmitterGetNoBuffers(emitter, AAX_PROCESSED);
            break;
        case AL_BUFFER:
        {
            const aaxBuffer buf = aaxEmitterGetBufferByPos(emitter,0,AAX_FALSE);
            if (buf)
            {
                _intBuffers *db = _oalGetBuffers(NULL);
                unsigned int pos = _intBufGetPos(db, _OAL_BUFFER, buf);
                *value = (T)_intBufPosToId(pos);
            } else {
                *value = 0;
            }
            break;
        }
        case AL_SAMPLE_OFFSET:
        {
            unsigned int offs = aaxEmitterGetOffset(emitter, AAX_SAMPLES);
            unsigned tracks = aaxEmitterGetSetup(emitter, AAX_TRACKS);
            *value = (T)_oalAAXOffsetToOffsetInSamples(offs, tracks);
            break;
        }
        case AL_BYTE_OFFSET:
        {
            unsigned int offs = aaxEmitterGetOffset(emitter, AAX_SAMPLES);
            unsigned tracks = aaxEmitterGetSetup(emitter, AAX_TRACKS);
            enum aaxFormat fmt = aaxEmitterGetSetup(emitter, AAX_FORMAT);
            *value = (T)_oalAAXOffsetToOffsetInBytes(offs, tracks, fmt);
            break;
        }
        case AL_SEC_OFFSET:
            *value = (T)aaxEmitterGetOffsetSec(emitter);
            break;
        case AL_GAIN:
            flt = aaxEmitterGetFilter(emitter, AAX_VOLUME_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_GAIN, AAX_LINEAR);
            aaxFilterDestroy(flt);
            break;
        case AL_MIN_GAIN:
            flt = aaxEmitterGetFilter(emitter, AAX_VOLUME_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_MIN_GAIN, AAX_LINEAR);
            aaxFilterDestroy(flt);
            break;
        case AL_MAX_GAIN:
            flt = aaxEmitterGetFilter(emitter, AAX_VOLUME_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_MAX_GAIN, AAX_LINEAR);
            aaxFilterDestroy(flt);
            break;
        case AL_PITCH:
            eff = aaxEmitterGetEffect(emitter, AAX_PITCH_EFFECT);
            *value = (T)aaxEffectGetParam(eff, AAX_PITCH, AAX_LINEAR);
            aaxEffectDestroy(eff);
            break;
        case AL_REFERENCE_DISTANCE:
            flt = aaxEmitterGetFilter(emitter, AAX_DISTANCE_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_REF_DISTANCE, AAX_LINEAR);
            aaxFilterDestroy(flt);
            break;
        case AL_ROLLOFF_FACTOR:
            flt = aaxEmitterGetFilter(emitter, AAX_DISTANCE_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_ROLLOFF_FACTOR, AAX_LINEAR);
            aaxFilterDestroy(flt);
            break;
        case AL_MAX_DISTANCE:
            flt = aaxEmitterGetFilter(emitter, AAX_DISTANCE_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_MAX_DISTANCE, AAX_LINEAR);
            aaxFilterDestroy(flt);
            break;
        case AL_CONE_INNER_ANGLE:
            flt = aaxEmitterGetFilter(emitter, AAX_ANGULAR_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_INNER_ANGLE, AAX_DEGREES);
            aaxFilterDestroy(flt);
            break;
        case AL_CONE_OUTER_ANGLE:
            flt = aaxEmitterGetFilter(emitter, AAX_ANGULAR_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_OUTER_ANGLE, AAX_DEGREES);
            aaxFilterDestroy(flt);
            break;
        case AL_CONE_OUTER_GAIN:
            flt = aaxEmitterGetFilter(emitter, AAX_ANGULAR_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_OUTER_GAIN, AAX_LINEAR);
            aaxFilterDestroy(flt);
            break;
        /* AL_EXT_source_latency */
        case AL_SAMPLE_OFFSET_LATENCY:
        {
            unsigned int offs = aaxEmitterGetOffset(emitter, AAX_SAMPLES);
            unsigned tracks = aaxEmitterGetSetup(emitter, AAX_TRACKS);
            ALCcontext *ctx = alcGetCurrentContext();
            ALCdevice *device = alcGetContextsDevice(ctx);
            uint32_t id = _oalDeviceToId(device);
            _oalDevice *dev = (_oalDevice *)_oalFindDeviceById(id);
            T freq_hz, latency_us, fact = (T)((ALint64)1 << BITSHIFT);

            freq_hz = fact*aaxMixerGetSetup(dev->lst.handle, AAX_FREQUENCY);
            latency_us = fact*aaxMixerGetSetup(dev->lst.handle, AAX_LATENCY);

            *value = (T)_oalAAXOffsetToOffsetInSamples(offs, tracks);
#if BITSHIFT
            *value <<= BITSHIFT;
#endif
            *value += (T)(freq_hz * latency_us*1e-6);
            break;
        }
        case AL_SEC_OFFSET_LATENCY:
        {
            ALCcontext *ctx = alcGetCurrentContext();
            ALCdevice *device = alcGetContextsDevice(ctx);
            uint32_t id = _oalDeviceToId(device);
            _oalDevice *dev = (_oalDevice *)_oalFindDeviceById(id);
            T latency;

            latency = (T)1e-3*aaxMixerGetSetup(dev->lst.handle, AAX_LATENCY);
            *value = (T)(aaxEmitterGetOffsetSec(emitter) + latency);
            break;
        }

        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else
    {
      _oalStateSetError(AL_INVALID_NAME);
      return;
    }
}

# undef BITSHIFT
# undef __ALGETSOURCEV
# undef __ALGETSOURCE3
# undef __ALGETSOURCE
# undef __ALSOURCEV
# undef __ALSOURCE3
# undef __ALSOURCE
# undef ALGETSOURCEV
# undef ALGETSOURCE3
# undef ALGETSOURCE
# undef ALSOURCEV
# undef ALSOURCE3
# undef ALSOURCE
# undef N
# undef T
#endif
