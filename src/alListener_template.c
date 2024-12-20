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

# define __ALLISTENERV(NAME)	alListener##NAME##v
# define __ALLISTENER3(NAME)	alListener3##NAME
# define __ALLISTENER(NAME)	alListener##NAME
# define __ALGETLISTENERV(NAME)	alGetListener##NAME##v
# define __ALGETLISTENER3(NAME)	alGetListener3##NAME
# define __ALGETLISTENER(NAME)	alGetListener##NAME

# define ALLISTENERV(NAME)	__ALLISTENERV(NAME)
# define ALLISTENER3(NAME)	__ALLISTENER3(NAME)
# define ALLISTENER(NAME)	__ALLISTENER(NAME)
# define ALGETLISTENERV(NAME)	__ALGETLISTENERV(NAME)
# define ALGETLISTENER3(NAME)	__ALGETLISTENER3(NAME)
# define ALGETLISTENER(NAME)	__ALGETLISTENER(NAME)

AL_API void AL_APIENTRY
ALLISTENER3(N)(ALenum attrib, T v1, T v2, T v3)
{
    T Tv[3];

    Tv[0] = v1;
    Tv[1] = v2;
    Tv[2] = v3;
    ALLISTENERV(N)(attrib, (T *)&Tv);
}

AL_API void AL_APIENTRY
ALLISTENERV(N)(ALenum attrib, const T *values)
{
    _alBufferData *dptr_ctx;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr_ctx = _oalGetCurrentContext();
    if (dptr_ctx)
    {
        _oalListener *lst;
        _oalContext *ctx;

        ctx = _alBufGetDataPtr(dptr_ctx);
        lst = _oalGetListeners(ctx);
        if (lst)
        {
            aaxConfig config = lst->handle;
            aaxVec3f vec3f;
            aaxMtx4d mtx;

            switch(attrib)
            {
            case AL_POSITION:
                lst->pos[0] = (double)values[0];
                lst->pos[1] = (double)values[1];
                lst->pos[2] = (double)values[2];
                aaxMatrix64SetOrientation(mtx, lst->pos, lst->at, lst->up);
                aaxMatrix64Inverse(mtx);
                aaxSensorSetMatrix64(config, mtx);
                break;
            case AL_ORIENTATION:
                lst->at[0] = (float)values[0];
                lst->at[1] = (float)values[1];
                lst->at[2] = (float)values[2];
                lst->up[0] = (float)values[3];
                lst->up[1] = (float)values[4];
                lst->up[2] = (float)values[5];
                aaxMatrix64SetOrientation(mtx, lst->pos, lst->at, lst->up);
                aaxMatrix64Inverse(mtx);
                aaxSensorSetMatrix64(config, mtx);
                break;
            case AL_VELOCITY:
                vec3f[0] = (float)values[0];
                vec3f[1] = (float)values[1];
                vec3f[2] = (float)values[2];
                aaxSensorSetVelocity(config, vec3f);
                break;
            /* AL_AAX_frequency_filter */
            case AL_FREQUENCY_FILTER_PARAMS_AAX:
            {
                aaxFilter flt;
                flt = aaxSceneryGetFilter(config, AAX_FREQUENCY_FILTER);
                aaxFilterSetParam(flt, AAX_CUTOFF_FREQUENCY, AAX_LINEAR, (float)values[0]);
                aaxFilterSetParam(flt, AAX_LF_GAIN, AAX_LINEAR, (float)values[1]);
                aaxFilterSetParam(flt, AAX_HF_GAIN, AAX_LINEAR, (float)values[2]);
                aaxScenerySetFilter(config, flt);
                aaxFilterDestroy(flt);
                break;
            }
            /* AL_AAX_environment */
            case AL_SCENE_DIMENSIONS_AAX:
            case AL_SCENE_CENTER_AAX:
            case AL_WIND_DIRECTION_AAX:
            default:
                ALLISTENER(N)(attrib, *values);
            }
        }

        _alBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

AL_API void AL_APIENTRY
ALLISTENER(N)(ALenum attrib, T value)
{
    _oalListener *lst;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    lst = _oalGetListeners(NULL);
    if (lst != NULL)
    {
        aaxConfig config = lst->handle;
        float fval = (float)value;
        aaxFilter flt;

        switch(attrib)
        {
        case AL_GAIN:
        if (value >= 0) {
            flt = aaxMixerGetFilter(config, AAX_VOLUME_FILTER);
            aaxFilterSetParam(flt, AAX_GAIN, value, AAX_LINEAR);
            aaxMixerSetFilter(config, flt);
            aaxFilterDestroy(flt);
        }
        else {
            _oalStateSetError(AL_INVALID_VALUE);
        }
        break;
        /* AL_AAX_frequency_filter */
        case AL_FREQUENCY_FILTER_ENABLE_AAX:
            flt = aaxSceneryGetFilter(config, AAX_FREQUENCY_FILTER);
            aaxFilterSetState(flt, value ? AAX_TRUE : AAX_FALSE);
            aaxScenerySetFilter(config, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_FREQUENCY_FILTER_GAINLF_AAX:
            flt = aaxSceneryGetFilter(config, AAX_FREQUENCY_FILTER);
            aaxFilterSetParam(flt, AAX_LF_GAIN, fval, AAX_LINEAR);
            aaxScenerySetFilter(config, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_FREQUENCY_FILTER_GAINHF_AAX:
            flt = aaxSceneryGetFilter(config, AAX_FREQUENCY_FILTER);
            aaxFilterSetParam(flt, AAX_HF_GAIN, fval, AAX_LINEAR);
            aaxScenerySetFilter(config, flt);
            aaxFilterDestroy(flt);
            break;
        case AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX:
            flt = aaxSceneryGetFilter(config, AAX_FREQUENCY_FILTER);
            aaxFilterSetParam(flt, AAX_CUTOFF_FREQUENCY, fval, AAX_LINEAR);
            aaxScenerySetFilter(config, flt);
            aaxFilterDestroy(flt);
            break;
        /* AL_AAX_reverb */
        case AL_REVERB_ENABLE_AAX:
        {
            aaxEffect e = aaxMixerGetEffect(config, AAX_REVERB_EFFECT);
            aaxEffectSetState(e, value ? AAX_TRUE : AAX_FALSE);
            aaxMixerSetEffect(config, e);
            aaxEffectDestroy(e);
            break;
        }
        case AL_REVERB_PRE_DELAY_TIME_AAX:
            _oalSetReverb(config, fval, AAX_FPNONE, AAX_FPNONE, AAX_FPNONE, AAX_FPNONE);
            break;
        case AL_REVERB_REFLECTION_TIME_AAX:
            _oalSetReverb(config, AAX_FPNONE, fval, AAX_FPNONE, AAX_FPNONE, AAX_FPNONE);
            break;
        case AL_REVERB_REFLECTION_FACTOR_AAX:
            _oalSetReverb(config, AAX_FPNONE, AAX_FPNONE, fval, AAX_FPNONE, AAX_FPNONE);
            break;
        case AL_REVERB_DECAY_TIME_AAX:
            _oalSetReverb(config, AAX_FPNONE, AAX_FPNONE, AAX_FPNONE, fval, AAX_FPNONE);
            break;
        case AL_REVERB_DECAY_TIME_HF_AAX:
            _oalSetReverb(config, AAX_FPNONE, AAX_FPNONE, AAX_FPNONE,  AAX_FPNONE, fval);
            break;
        /* AL_AAX_environment */
        case AL_SCENE_LENGTH_AAX:
        case AL_SCENE_WIDTH_AAX:
        case AL_SCENE_HEIGHT_AAX:
        case AL_WIND_SPEED_AAX:
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }
}

AL_API void AL_APIENTRY
ALGETLISTENER3(N)(ALenum attrib, T *v1, T *v2, T *v3)
{
    T Tv[3];

    ALGETLISTENERV(N)(attrib, (T *)&Tv);
    *v1 = Tv[0];
    *v2 = Tv[1];
    *v3 = Tv[2];
}

AL_API void AL_APIENTRY
ALGETLISTENERV(N)(ALenum attrib, T *values)
{
    _oalListener *lst;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    lst = _oalGetListeners(0);
    if (lst)
    {
        aaxConfig config = lst->handle;

        switch(attrib)
        {
        case AL_POSITION:
            values[0] = (T)lst->pos[0];
            values[1] = (T)lst->pos[1];
            values[2] = (T)lst->pos[2];
            break;
        case AL_ORIENTATION:
            values[0] = (T)lst->at[0];
            values[1] = (T)lst->at[1];
            values[2] = (T)lst->at[2];
            values[3] = (T)lst->up[0];
            values[4] = (T)lst->up[1];
            values[5] = (T)lst->up[2];
            break;
        case AL_VELOCITY:
        {
            aaxVec3f vec3f;
            aaxSensorGetVelocity(config, vec3f);
            values[0] = (T)vec3f[0];
            values[1] = (T)vec3f[1];
            values[2] = (T)vec3f[2];
            break;
        }
        case AL_SCENE_DIMENSIONS_AAX:
        case AL_SCENE_CENTER_AAX:
        case AL_WIND_DIRECTION_AAX:
        default:
            ALGETLISTENER(N)(attrib, values);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

AL_API void AL_APIENTRY
ALGETLISTENER(N)(ALenum attrib, T *value)
{
    _oalListener *lst;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!value)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    lst = _oalGetListeners(0);
    if (lst)
    {
        aaxConfig config = lst->handle;

        switch(attrib)
        {
        case AL_GAIN:
        {
            aaxFilter flt = aaxMixerGetFilter(config, AAX_VOLUME_FILTER);
            *value = (T)aaxFilterGetParam(flt, AAX_GAIN, AAX_LINEAR);
            aaxFilterDestroy(flt);
            break;
        }
        case AL_SCENE_LENGTH_AAX:
        case AL_SCENE_WIDTH_AAX:
        case AL_SCENE_HEIGHT_AAX:
        case AL_WIND_SPEED_AAX:
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

# undef __ALLISTENERV
# undef __ALLISTENER3
# undef __ALLISTENER
# undef __ALGETLISTENERV
# undef __ALGETLISTENER3
# undef __ALGETLISTENER
# undef ALLISTENERV
# undef ALLISTENER3
# undef ALLISTENER
# undef ALGETLISTENERV
# undef ALGETLISTENER3
# undef ALGETLISTENER
# undef N
# undef T
#endif
