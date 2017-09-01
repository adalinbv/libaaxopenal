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

#ifndef AAX_API_H
#define AAX_API_H 1

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <aax/aax.h>
#include <AL/al.h>
#include <AL/alc.h>

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

     char src_dist_model;
     char distance_delay;

} _oalState;

void _oalStateCreate(aaxConfig, void *);

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
#define _SRC_PER_THREAD		16

typedef struct
{
    aaxConfig handle;
    aaxVec3f at, up;
    aaxVec3d pos;
    void *buf;

    struct
    {
        float pre_delay_time;
        float decay_time, decay_time_hf;
        float reflection_time, reflection_factor;
    } reverb;

} _oalListener;

/* --- Source -- */

typedef struct
{
    void *parent;
    aaxEmitter handle;
    aaxVec3f at, up;
    aaxVec3d pos;
    int mode;
} _oalSource;

void _oalFreeSource(void *, void*);

/* -- Contexts --- */

#define _oalDeviceToId(a)	(uint32_t)((long)(a) >> 20)
#define _oalIdToDevice(a)	(uint32_t)((long)(a) << 20)
#define _oalContextMask(a)	(uint32_t)((long)(a) & 0x000FFFFF)
#define _oalDeviceMask(a)	(uint32_t)((long)(a) & 0xFFF00000)
#define INT_TO_PTR(a)		(void*)(long)(a)

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
    ALCenum format;
    int frequency;

    _intBuffers *buffers;

    /* dynamic data */
    unsigned int current_context;

    _oalListener lst;
    _intBuffers *contexts;

} _oalDevice;

_intBufferData *_oalGetCurrentDevice();
_intBufferData *_oalGetCurrentContext();
_oalDevice *_oalFindDeviceById(unsigned int);

extern _intBuffers *_oalDevices;

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

/* --- Buffers --- */

_intBuffers *_oalGetBuffers(_oalDevice *d);
_intBufferData *_oalFindBufferById(ALuint, ALuint*);
void _oalFreeBuffer(void*);

#endif

