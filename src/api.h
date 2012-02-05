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

#ifndef AAX_API_H
#define AAX_API_H 1

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <aax.h>

#include <base/buffers.h>
#include <base/logging.h>

enum
{
     _OAL_NONE = 0,
     _OAL_BACKEND,
     _OAL_EXTENSION,
     _OAL_ENUM,
     _OAL_DEVICE,
     _OAL_CONTEXT,
     _OAL_STATE,
     _OAL_BUFFER,
     _OAL_SOURCE,
     _OAL_LISTENER,
     _OAL_SBUFFER,

     _OAL_MAX_ID
};

extern const char* _al_id_s[_OAL_MAX_ID];

#ifndef NDEBUG
# define _AL_LOG(a, c)	__oal_log((a), 0, (const char*)(c), _al_id_s, LOG_ERR)
#else
# include <string.h>
# include <stdlib.h>
# define _AL_LOG(a, c)
#endif

/* --- State --- */

typedef struct
{
     ALenum error;

     ALfloat dopplerFactor;
     ALfloat dopplerVelocity;
     ALfloat soundVelocity;
     ALfloat maxDistance;
     ALenum distanceModel;
} _oalState;

void _oalStateCreate(void *);

/**
 * State Error reporting
 */

#ifndef __FUNCTION__
# define __FUNCTION__ __func__
#endif


#ifndef NDEBUG
ALenum __oalStateSetErrorReport(ALenum, char *, int);
# define _oalStateSetError(a)  __oalStateSetErrorReport((a), __FILE__, __LINE__)
#else
ALenum __oalStateSetErrorNormal(ALenum);
# define _oalStateSetError(a)    __oalStateSetErrorNormal(a)
#endif

/* --- Listener --- */
#define _MAX_THREADS		4

typedef struct
{
    aaxConfig handle;
    aaxFrame frame[_MAX_THREADS-1];
    unsigned int frame_no, frame_max;
    aaxVec3f pos, at, up;

} _oalListener;

/* --- Source -- */

typedef struct
{
    aaxEmitter handle;
    aaxVec3f pos, at, up;
    int mode;

} _oalSource;

/* -- Contexts --- */

typedef struct
{
    /* static data */
    ALCboolean sync;

    /* dynamic data */
    ALCboolean suspend;
    ALCenum error;

    _oalState *state;
    const void *parent_device;

    _intBuffers *sources;

} _oalContext;

typedef struct
{
    ALCboolean sync;
    ALCenum error;

    _intBuffers *buffers;

    /* dynamic data */
    unsigned int current_context;

    _oalListener lst;
    _intBuffers *contexts;

} _oalDevice;

_intBufferData *_oalGetCurrentDevice();
_intBufferData *_oalGetCurrentContext();

/**
 * Context Error reporting
 */
ALCenum __oalContextSetErrorNormal(ALCenum);
ALCenum __oalContextSetErrorReport(ALCenum, char *, int);
#ifndef NDEBUG
# define _oalContextSetError(a)  __oalContextSetErrorReport((a), __FILE__, __LINE__)
#else
# define _oalContextSetError(a)  __oalContextSetErrorNormal(a)
#endif

/* --- Sources --- */

void _oalRemoveSourceByPos(void *context, unsigned int);

/* --- Buffers --- */

_intBuffers *_oalGetBuffers();
_intBufferData *_oalFindBufferById(ALuint, ALuint*);
_intBufFreeCallback _oalRemoveBufferByPos;

#endif

