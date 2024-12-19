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

#include <math.h>

#include <aax/aax.h>
#include <AL/alext.h>

#include <base/types.h>

#include "aax_support.h"
#include "api.h"

_oalListener* _oalGetListeners(_oalContext *);

/*
 * void alListeneri(ALenum attrib, ALint value)
 * void alListeneriv(ALenum attrib, const ALint *values)
 * void alListener3i(ALenum attrib, ALint v1, ALint v2, ALint v3)
 *
 * void alGetListeneri(ALenum attrib, ALint *value)
 * void alGetListeneriv(ALenum attrib, ALint *values)
 * void alGetListener3i(ALenum attrib, ALint *v1, ALint *v2, ALint *v3)
 */
#define N i
#define T ALint
#include "alListener_template.c"

/*
 * void alListenerf(ALenum attrib, ALfloat value)
 * void alListenerfv(ALenum attrib, const ALfloat *values)
 * void alListener3f(ALenum attrib, ALfloat v1, ALfloat v2, ALfloat v3)
 *
 * void alGetListenerf(ALenum attrib, ALfloat *value)
 * void alGetListenerfv(ALenum attrib, ALfloat *values)
 * void alGetListener3f(ALenum attrib, ALfloat *v1, ALfloat *v2, ALfloat *v3)
 */
#define N f
#define T ALfloat
#include "alListener_template.c"

/* -------------------------------------------------------------------------- */

_oalListener *
_oalGetListeners(_oalContext *context)
{
    _oalContext *ctx = context;
    _alBufferData *dptr = NULL;
    _oalListener *lst = 0;

    _AL_LOG(LOG_DEBUG, __FUNCTION__);

    if (!ctx && ((dptr = _oalGetCurrentContext()) != NULL)) {
        ctx = _alBufGetDataPtr(dptr);
    }

    if (ctx)
    {
        const _oalDevice *dev = ctx->parent_device;
        lst = (_oalListener *)&dev->lst;
    }
    else {
        _oalContextSetError(ALC_INVALID_CONTEXT);
    }

    if (dptr) {
        _alBufReleaseData(dptr, _OAL_CONTEXT);
    }

    return lst;
}

