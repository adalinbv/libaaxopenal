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
#include "config.h"
#endif

#include <math.h>

#include <aax.h>
#include <aaxdefs.h>
#include <AL/alext.h>

#include <base/types.h>

#include "api.h"

_oalListener* _oalGetListeners(_oalContext *);

void
alListenerf(ALenum attrib, ALfloat value)
{
    _oalListener *lst;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    lst = _oalGetListeners(NULL);
    if (lst == NULL)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    switch(attrib)
    {
    case AL_GAIN:
        if (value >= 0.0f) {
             aaxMixerSetGain(lst->handle, value);
        }
        else {
            _oalStateSetError(AL_INVALID_VALUE);
        }
        break;
    /* AL_AAX_freq_filter */
    case AL_AAX_FREQUENCY_FILTER_GAINLF:
        aaxScenerySetFrequencyFilter(lst->handle, AAX_FPNONE, value, AAX_FPNONE);
        break;
    case AL_AAX_FREQUENCY_FILTER_GAINHF:
        aaxScenerySetFrequencyFilter(lst->handle, AAX_FPNONE, AAX_FPNONE, value);
        break;
    case AL_AAX_FREQUENCY_FILTER_CUTOFF_FREQUENCY:
        aaxScenerySetFrequencyFilter(lst->handle, value, AAX_FPNONE, AAX_FPNONE);
        break;
#if 0
    /* AL_AAX_environment */
    case AL_AAX_SCENE_LENGTH:
        lst->scene.dimension[0] = value;
        _PROP_SCENE_SET_CHANGED(lst->props);
        break;
    case AL_AAX_SCENE_WIDTH:
        lst->scene.dimension[1] = value;
        _PROP_SCENE_SET_CHANGED(lst->props);
        break;
    case AL_AAX_SCENE_HEIGHT:
        lst->scene.dimension[2] = value;
        _PROP_SCENE_SET_CHANGED(lst->props);
        break;
    case AL_AAX_SCENE_REFLECTION:
        lst->scene.reflection = value;
        _PROP_REVERB_SET_CHANGED(lst->props);
        break;
    case AL_AAX_SCENE_DIFFUSE:
        lst->scene.diffuse = value;
        _PROP_REVERB_SET_CHANGED(lst->props);
        break;
    case AL_AAX_DISTANCE_DELAY:
        lst->distance_delay = (char)value;
         _PROP_DISTDELAY_SET_DEFINED(lst->props);
        break;
    case AL_AAX_WIND_SPEED:
        lst->wind.speed = value;
        _PROP_WIND_SET_CHANGED(lst->props);
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alListenerfv(ALenum attrib, const ALfloat *values)
{
    _intBufferData *dptr_ctx;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr_ctx = _oalGetCurrentContext(0);
    if (dptr_ctx)
    {
        _oalContext *ctx;
        _oalListener *lst;

        ctx = _intBufGetDataPtr(dptr_ctx);
        lst = _oalGetListeners(ctx);
        if (lst)
        {
            switch(attrib)
            {
            case AL_GAIN:
                if (*values >= 0.0f) {
                    aaxMixerSetGain(lst->handle, *values);
                }
                else {
                    _oalStateSetError(AL_INVALID_VALUE);
                }
                break;
            case AL_POSITION:
            {
                aaxMtx4f mtx;
                memcpy(lst->pos, values, sizeof(aaxVec3f));
                aaxMatrixSetOrientation(mtx, lst->pos, lst->at, lst->up);
                aaxMatrixInverse(mtx);
                aaxSensorSetMatrix(lst->handle, mtx);
                break;
            }
            case AL_ORIENTATION:
            {
                aaxMtx4f mtx;
                memcpy(lst->at, values, sizeof(aaxVec3f));
                memcpy(lst->up, values+3, sizeof(aaxVec3f));
                aaxMatrixSetOrientation(mtx, lst->pos, lst->at, lst->up);
                aaxMatrixInverse(mtx);
                aaxSensorSetMatrix(lst->handle, mtx);
                break;
            }
            case AL_VELOCITY:
                aaxSensorSetVelocity(lst->handle, values);
                break;
            case AL_AAX_FREQUENCY_FILTER_PARAMS:
                aaxScenerySetFrequencyFilter(lst->handle,values[0],values[1],values[2]);
                break;
#if 0
            /* AL_AAX_environment */
            case AL_AAX_SCENE_DIMENSIONS:
                memcpy(&lst->scene.dimension, values, 3*sizeof(ALfloat));
                _PROP_SCENE_SET_CHANGED(lst->props);
                break;
            case AL_AAX_SCENE_CENTER:
                memcpy(&lst->scene.pos, values, 3*sizeof(ALfloat));
                _PROP_SCENE_SET_CHANGED(lst->props);
                break;
            case AL_AAX_WIND_DIRECTION:
                memcpy(&lst->wind.speed, values, 3*sizeof(ALfloat));
                _PROP_WIND_SET_CHANGED(lst->props);
                break;
#endif
            default:
                _oalStateSetError(AL_INVALID_ENUM);
            }
        }
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

void
alListener3f(ALenum attrib, ALfloat v1, ALfloat v2, ALfloat v3)
{
    ALfloat fv[3];

    fv[0] = v1;
    fv[1] = v2;
    fv[2] = v3;
    alListenerfv(attrib, (ALfloat *)&fv);
}

void
alListeneri(ALenum attrib, ALint value)
{
    _oalListener *lst;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    lst = _oalGetListeners(NULL);
    if (lst == NULL)
    {
        _oalStateSetError(AL_INVALID_OPERATION);
        return;
    }

    switch(attrib)
    {
    /* AL_AAX_frequency_filter */
    case AL_AAX_FREQUENCY_FILTER_ENABLE:
    {
        aaxFilter f = aaxSceneryGetFilter(lst->handle, AAX_FREQUENCY_FILTER);
        aaxFilterSetState(f, value ? AAX_TRUE : AAX_FALSE);
        aaxScenerySetFilter(lst->handle, f);
        aaxFilterDestroy(f);
        break;
    }
    case AL_AAX_FREQUENCY_FILTER_CUTOFF_FREQUENCY:
        aaxScenerySetFrequencyFilter(lst->handle, value, AAX_FPNONE, AAX_FPNONE);
        break;
#if 0
    /* AL_AAX_environment */
    case AL_AAX_SCENE_LENGTH:
        lst->scene.dimension[0] = (float)value;
        _PROP_SCENE_SET_CHANGED(lst->props);
        break;
    case AL_AAX_SCENE_WIDTH:
        lst->scene.dimension[1] = (float)value;
        _PROP_SCENE_SET_CHANGED(lst->props);
        break;
    case AL_AAX_SCENE_HEIGHT:
        lst->scene.dimension[2] = (float)value;
        _PROP_SCENE_SET_CHANGED(lst->props);
        break;
    case AL_AAX_DISTANCE_DELAY:
        lst->distance_delay = (char)value;
        _PROP_DISTDELAY_SET_DEFINED(lst->props);
        break;
    case AL_AAX_WIND_SPEED:
        lst->wind.speed = (float)value;
        _PROP_WIND_SET_CHANGED(lst->props);
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alListeneriv(ALenum attrib, const ALint *values)
{
    _intBufferData *dptr_ctx;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr_ctx = _oalGetCurrentContext(0);
    if (dptr_ctx)
    {
        _oalListener *lst;
        _oalContext *ctx;

        ctx = _intBufGetDataPtr(dptr_ctx);
        lst = _oalGetListeners(ctx);
        if (lst)
        {
            switch(attrib)
            {
            case AL_POSITION:
            {
                aaxMtx4f mtx;
                lst->pos[0] = (float)values[0];
                lst->pos[1] = (float)values[1];
                lst->pos[2] = (float)values[2];
                aaxMatrixSetOrientation(mtx, lst->pos, lst->at, lst->up);
                aaxMatrixInverse(mtx);
                aaxSensorSetMatrix(lst->handle, mtx);
                break;
            }
            case AL_ORIENTATION:
            {
                aaxMtx4f mtx;
                lst->at[0] = (float)values[0];
                lst->at[1] = (float)values[1];
                lst->at[2] = (float)values[2];
                lst->up[0] = (float)values[3];
                lst->up[1] = (float)values[4];
                lst->up[2] = (float)values[5];
                aaxMatrixSetOrientation(mtx, lst->pos, lst->at, lst->up);
                aaxMatrixInverse(mtx);
                aaxSensorSetMatrix(lst->handle, mtx);
                break;
            }
            case AL_VELOCITY:
            {
                aaxVec3f vec3f;
                vec3f[0] = (float)values[0];
                vec3f[1] = (float)values[1];
                vec3f[2] = (float)values[2];
                aaxSensorSetVelocity(lst->handle, vec3f);
                break;
            }
#if 0
            /* AL_AAX_environment */
            case AL_AAX_SCENE_DIMENSIONS:
                lst->scene.dimension[0] = (float)values[0];
                lst->scene.dimension[1] = (float)values[1];
                lst->scene.dimension[2] = (float)values[2];
                _PROP_SCENE_SET_CHANGED(lst->props);
                break;
            case AL_AAX_SCENE_CENTER:
                lst->scene.pos[0] = (float)values[0];
                lst->scene.pos[1] = (float)values[1];
                lst->scene.pos[2] = (float)values[2];
                _PROP_SCENE_SET_CHANGED(lst->props);
                break;
#endif
            default:
                _oalStateSetError(AL_INVALID_ENUM);
            }
        }
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

void
alListener3i(ALenum attrib, ALint v1, ALint v2, ALint v3)
{
    ALint iv[3];

    iv[0] = v1;
    iv[1] = v2;
    iv[2] = v3;
    alListeneriv(attrib, (ALint *)&iv);
}

void
alGetListenerf(ALenum attrib, ALfloat *value)
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
        switch(attrib)
        {
        case AL_GAIN:
            *value = aaxMixerGetGain(lst->handle);
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

void
alGetListenerfv(ALenum attrib, ALfloat *values)
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
        switch(attrib)
        {
        case AL_POSITION:
            memcpy(values, lst->pos, sizeof(aaxVec3f));
            break;
        case AL_ORIENTATION:
            memcpy(values, lst->at, sizeof(aaxVec3f));
            memcpy(values+3, lst->up, sizeof(aaxVec3f));
            break;
        case AL_VELOCITY:
            aaxSensorGetVelocity(lst->handle, values);
            break;
        case AL_GAIN:
            *values = aaxMixerGetGain(lst->handle);
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

void
alGetListener3f(ALenum attrib, ALfloat *v1, ALfloat *v2, ALfloat *v3)
{
    ALfloat fv[3];

    alGetListenerfv(attrib, (ALfloat *)&fv);
    *v1 = fv[0];
    *v2 = fv[1];
    *v3 = fv[2];
}

void
alGetListeneri(ALenum attrib, ALint *value)
{
    switch(attrib)
    {
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

void
alGetListeneriv(ALenum attrib, ALint *values)
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
        switch(attrib)
        {
        case AL_POSITION:
            values[0] = (ALint)lst->pos[0];
            values[1] = (ALint)lst->pos[1];
            values[2] = (ALint)lst->pos[2];
            break;
        case AL_ORIENTATION:
            values[0] = (ALint)lst->at[0];
            values[1] = (ALint)lst->at[1];
            values[2] = (ALint)lst->at[2];
            values[3] = (ALint)lst->up[0];
            values[4] = (ALint)lst->up[1];
            values[5] = (ALint)lst->up[2];
            break;
        case AL_VELOCITY:
        {
            aaxVec3f vec3f;
            aaxSensorGetVelocity(lst->handle, vec3f);
            values[0] = (ALint)vec3f[0];
            values[1] = (ALint)vec3f[1];
            values[2] = (ALint)vec3f[2];
            break;
        }
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

void
alGetListener3i(ALenum attrib, ALint *v1, ALint *v2, ALint *v3)
{
    ALint iv[3];

    alGetListeneriv(attrib, (ALint *)&iv);
    *v1 = iv[0];
    *v2 = iv[1];
    *v3 = iv[2];
}

/* -------------------------------------------------------------------------- */

_oalListener *
_oalGetListeners(_oalContext *context)
{
    _oalContext *ctx = context;
    _intBufferData *dptr = 0;
    _oalListener *lst = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!context)
    {
        dptr = _oalGetCurrentContext(0);
        if (dptr) ctx = _intBufGetDataPtr(dptr);
    }

    if (ctx)
    {
        const _oalDevice *dev = ctx->parent_device;
        lst = (_oalListener *)&dev->lst;
    }
    else {
        _oalContextSetError(ALC_INVALID_CONTEXT);
    }

    return lst;
}

