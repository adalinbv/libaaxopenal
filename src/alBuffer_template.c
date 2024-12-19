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

# define __ALBUFFERV(NAME)	alBuffer##NAME##v
# define __ALBUFFER3(NAME)	alBuffer3##NAME
# define __ALBUFFER(NAME)	alBuffer##NAME
# define __ALGETBUFFERV(NAME)	alGetBuffer##NAME##v
# define __ALGETBUFFER3(NAME)	alGetBuffer3##NAME
# define __ALGETBUFFER(NAME)	alGetBuffer##NAME

# define ALBUFFERV(NAME)	__ALBUFFERV(NAME)
# define ALBUFFER3(NAME)	__ALBUFFER3(NAME)
# define ALBUFFER(NAME)		__ALBUFFER(NAME)
# define ALGETBUFFERV(NAME)	__ALGETBUFFERV(NAME)
# define ALGETBUFFER3(NAME)	__ALGETBUFFER3(NAME)
# define ALGETBUFFER(NAME)	__ALGETBUFFER(NAME)

#define IMA4_SMP_TO_BLOCKSIZE(a)	(((a)/2)+4)

AL_API void AL_APIENTRY
ALBUFFER3(N)(ALuint id, ALenum attrib, T v1, T v2, T v3)
{
    T Tv[3];

    Tv[0] = v1;
    Tv[1] = v2;
    Tv[2] = v3;
    ALBUFFERV(N)(id, attrib, (T *)&Tv);
}

AL_API void AL_APIENTRY
ALBUFFERV(N)(ALuint id, ALenum attrib, const T *values)
{
    const _alBufferData *dptr;
    unsigned int pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!id) return;    /* nop */

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindBufferById(id, &pos);
    if (dptr)
    {
        aaxBuffer buf = _alBufGetDataPtr(dptr);
        switch (attrib)
        {
        case AL_LOOP_POINTS:
            aaxBufferSetSetup(buf, AAX_LOOP_START, (unsigned)values[0]);
            aaxBufferSetSetup(buf, AAX_LOOP_END, (unsigned)values[1]);
            break;
        default:
            ALBUFFER(N)(id, attrib, *values);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_NAME);
    }
}

AL_API void AL_APIENTRY
ALBUFFER(N)(ALuint id, ALenum attrib, T value)
{
    const _alBufferData *dptr;
    unsigned int pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!id) return;    /* nop */

    if (value < 0.0f)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindBufferById(id, &pos);
    if (dptr)
    {
        aaxBuffer buf = _alBufGetDataPtr(dptr);
        switch (attrib)
        {
        case AL_FREQUENCY:
            aaxBufferSetSetup(buf, AAX_FREQUENCY, (unsigned)value);
            break;
        /* AL_SOFT_block_alignment */
        case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
        case AL_PACK_BLOCK_ALIGNMENT_SOFT:
            aaxBufferSetSetup(buf, AAX_BLOCK_ALIGNMENT,
                                   IMA4_SMP_TO_BLOCKSIZE( (unsigned)value ));
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_NAME);
    }
}

AL_API void AL_APIENTRY
ALGETBUFFER3(N)(ALuint id, ALenum attrib, T *v1, T *v2, T *v3)
{
    T Tv[3];

    ALGETBUFFERV(N)(id, attrib, (T *)&Tv);
    *v1 = Tv[0];
    *v2 = Tv[1];
    *v3 = Tv[2];
}

AL_API void AL_APIENTRY
ALGETBUFFERV(N)(ALuint id, ALenum attrib, T *values)
{
    const _alBufferData *dptr;
    unsigned int pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!id) return;    /* nop */

    if (!values)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    dptr = _oalFindBufferById(id, &pos);
    if (dptr)
    {
//      aaxBuffer buf = _alBufGetDataPtr(dptr);
        switch (attrib)
        {
        default:
            ALGETBUFFER(N)(id, attrib, values);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_NAME);
    }
}

AL_API void AL_APIENTRY
ALGETBUFFER(N)(ALuint id, ALenum attrib, T *value)
{
    const _alBufferData *dptr;
    unsigned int pos;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (!id) return;    /* nop */

    if (!value)
    {
        _oalStateSetError(AL_INVALID_NAME);
        return;
    }

    dptr = _oalFindBufferById(id, &pos);
    if (dptr)
    {
        aaxBuffer buf = _alBufGetDataPtr(dptr);
        switch (attrib)
        {
        case AL_FREQUENCY:
            *value = (T)aaxBufferGetSetup(buf, AAX_FREQUENCY);
            break;
        case AL_SIZE:
            *value = (T)(aaxBufferGetSetup(buf, AAX_TRACK_SIZE)
                         * aaxBufferGetSetup(buf, AAX_TRACKS));
            break;
        case AL_BITS:
            *value = (T)aaxGetBitsPerSample(aaxBufferGetSetup(buf, AAX_FORMAT));
            break;
        case AL_CHANNELS:
            *value = (T)aaxBufferGetSetup(buf, AAX_TRACKS);
            break;
        default:
            _oalStateSetError(AL_INVALID_ENUM);
        }
    }
    else {
        _oalStateSetError(AL_INVALID_NAME);
    }
}

# undef __ALBUFFERV
# undef __ALBUFFER3
# undef __ALBUFFER
# undef __ALGETBUFFERV
# undef __ALGETBUFFER3
# undef __ALGETBUFFER
# undef ALBUFFERV
# undef ALBUFFER3
# undef ALBUFFER
# undef ALGETBUFFERV
# undef ALGETBUFFER3
# undef ALGETBUFFER
# undef N
# undef T
#endif
