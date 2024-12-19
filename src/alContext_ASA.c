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

#if HAVE_CONFIG_H
#include "config.h"
#endif


ALC_API ALenum ALC_APIENTRY
alcASAGetSource(ALuint prop, ALuint id, ALvoid *data, ALuint* size)
{
    const _alBufferData *dptr_ctx;

    if (!ids)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    dptr_ctx = _oalGetCurrentContext();
    if (dptr_ctx)
    {
        const _alBufferData *dptr_src;
        _oalContext *ctx = _alBufGetDataPtr(dptr_ctx);
        _alBuffers *cs = _oalGetSources(ctx);
        unsigned int pos;

        dptr_src = _oalFindSourceById(id, cs, &pos);
        if (dptr_src)
        {
            _oalSource *src = _alBufGetDataPtr(dptr_src);
// aaxEmitterSetGainMinMax(src->handle, 0.0f, 1.0f);

        }
        else {
            _oalStateSetError(AL_INVALID_NAME);
        }
        _alBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

ALC_API ALenum ALC_APIENTRY
alcASASetSource(ALuint prop, ALuint id, ALvoid *data, ALuint size)
{
    const _alBufferData *dptr_ctx;

    if (!ids)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    dptr_ctx = _oalGetCurrentContext();
    if (dptr_ctx)
    {
        const _alBufferData *dptr_src;
        _oalContext *ctx = _alBufGetDataPtr(dptr_ctx); 
        _alBuffers *cs = _oalGetSources(ctx);
        unsigned int pos;

        dptr_src = _oalFindSourceById(id, cs, &pos);
        if (dptr_src)
        {
            _oalSource *src = _alBufGetDataPtr(dptr_src);
// aaxEmitterSetGainMinMax(src->handle, 0.0f, 1.0f);

        }
        else {
            _oalStateSetError(AL_INVALID_NAME);
        }
        _alBufReleaseData(dptr_ctx, _OAL_CONTEXT);
    }
    else {
        _oalStateSetError(AL_INVALID_OPERATION);
    }
}

ALC_API ALenum ALC_APIENTRY
alcASAGetListener(ALuint prop, ALvoid *data, ALuint* size)
{
}

ALC_API ALenum ALC_APIENTRY
alcASASetListener(ALuint prop, ALvoid *data, ALuint size)
{
}


