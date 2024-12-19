/*
 * SPDX-FileCopyrightText: Copyright © 2005-2023 by Erik Hofman.
 * SPDX-FileCopyrightText: Copyright © 2009-2023 by Adalin B.V.
 *
 * Package Name: AeonWave Audio eXtentions library.
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_ASSERT_H
#include <assert.h>
#endif
#include <stdarg.h>

#include "buffers.h"

#ifdef NDEBUG
# include <stdlib.h>
# include <string.h>
#else
# include "logging.h"

# ifndef OPENAL_SUPPORT
const char *_alBufNames[] =
{
 "None",
 "Backend",
 "Device",
 "Buffer",
 "Emitter",
 "EmitterBuffer",
 "Sensor",
 "Frame",
 "Ringbuffer",
 "Extension",
 "Max"
};
# else
const char *_alBufNames[] =
{
 "None",
 "Backend",
 "Extension",
 "Enum",
 "Device",
 "Context",
 "State",
 "Buffer",
 "Source",
 "Listener",
 "SourceBuf",
 "Max"
};
# endif
#endif

#ifdef BUFFER_DEBUG
# include <stdio.h>
# undef DISREGARD
# if 0
#  define DISREGARD(x) x
#  define PRINT(...)		printf(...)
# else
#  define DISREGARD(x) x __attribute__((unused))
#  define PRINT(...)
# endif
#endif

static int __alBufFreeSpace(_alBuffers *, int, char);


#ifdef BUFFER_DEBUG
unsigned int
_alBufCreateDebug(_alBuffers **buffer, unsigned int id, DISREGARD(char *file), DISREGARD(int line))
{
    unsigned int r = _alBufCreateNormal(buffer, id);
    PRINT("create: %s at line %i: %x\n", file, line, r);
    return r;
}
#endif

unsigned int
_alBufCreateNormal(_alBuffers **buffer, unsigned int id)
{
    unsigned int rv = UINT_MAX;

    assert(buffer != 0);
    assert(*buffer == 0);
    assert(id > 0);

    *buffer = calloc(1, sizeof(_alBuffers));
    if (*buffer)
    {
        unsigned int num = BUFFER_RESERVE;

        (*buffer)->data = calloc(num, sizeof(_alBufferData*));
        if ((*buffer)->data)
        {
#ifndef _AL_NOTHREADS
            (*buffer)->mutex =
# ifndef NDEBUGTHREADS
                _aaxMutexCreateDebug(0, _alBufNames[id], __func__);
# else
                _aaxMutexCreate(0);
# endif
#endif
            (*buffer)->lock_ctr = 0;
            (*buffer)->start = 0;
            (*buffer)->first_free = 0;		/* relative to start */
            (*buffer)->num_allocated = 0;	/* relative to start */
            (*buffer)->max_allocations = num;	/* absolute          */
            (*buffer)->id = id;
            rv = 0;
        }
        else
        {
            free(*buffer);
            *buffer = 0;
        }
    }

    return rv;
}

int
_alBufDestroyDataNoLock(_alBufferData *ptr)
{
    int rv;

    assert(ptr);

    if ((rv = --ptr->reference_ctr) == 0)
    {
#ifndef _AL_NOTHREADS
        _aaxMutexDestroy(ptr->mutex);
#endif
        free(ptr);
    }

    if (!rv) rv = UINT_MAX;

    return rv;
}

unsigned int
_alBufAddDataNormal(_alBuffers *buffer, unsigned int id, const void *data, char locked)
{
    unsigned int rv = UINT_MAX;

    assert(data != 0);
    assert(buffer != 0);
    assert(buffer->id == id);
    assert(buffer->data != 0);
    assert(buffer->start <= buffer->max_allocations);
    assert(buffer->start+buffer->first_free <= buffer->max_allocations);
    assert(buffer->start+buffer->num_allocated <= buffer->max_allocations);

    if (__alBufFreeSpace(buffer, id, locked))
    {
        _alBufferData *b = malloc(sizeof(_alBufferData));
        if (b)
        {
            unsigned int pos;

#ifndef _AL_NOTHREADS
            b->mutex =
# ifndef NDEBUGTHREADS
                _aaxMutexCreateDebug(0, _alBufNames[id], __func__);
# else
                _aaxMutexCreate(0);
# endif
#endif
            b->reference_ctr = 1;
            b->ptr = data;

            if (!locked) {
               _alBufGetNum(buffer, id);
            }

            rv = buffer->first_free++;
            pos = buffer->start + rv;

//          assert(buffer->data[pos] == NULL);

            buffer->data[pos] = b;
            buffer->num_allocated++;

            assert(pos+1 < buffer->max_allocations);
            if (buffer->data[++pos] != 0)
            {
                unsigned int i, max = buffer->max_allocations - buffer->start;
                for(i=buffer->first_free; i<max; i++)
                {
                    if (buffer->data[buffer->start+i] == 0)
                        break;
                }
                buffer->first_free = i;
            }

            if (!locked) {
               _alBufReleaseNum(buffer, id);
            }
        }
    }

    return rv;
}

unsigned int
_alBufAddReference(_alBuffers *buffer, unsigned int id,
                        const _alBuffers *data, unsigned int n)
{
    unsigned int rv = UINT_MAX;

    assert(buffer != 0);
    assert(buffer->id == id);
    assert(buffer->data != 0);
    assert(buffer->start <= buffer->max_allocations);
    assert(buffer->start+buffer->first_free <= buffer->max_allocations);
    assert(buffer->start+buffer->num_allocated <= buffer->max_allocations);

    assert(data != 0);
    assert(data->id == id);
    assert(data->data != 0);

    n += data->start;

    assert(data->data[n] != 0);

    if (__alBufFreeSpace(buffer, id, 0))
    {
        _alBufferData *b = data->data[n];
        if (b)
        {
            unsigned int i, num;

#ifndef _AL_NOTHREADS
            _aaxMutexLock(b->mutex);
#endif
            b->reference_ctr++;
#ifndef _AL_NOTHREADS
            _aaxMutexUnLock(b->mutex);
#endif

            _alBufGetNum(buffer, id);

            buffer->num_allocated++;
            buffer->data[buffer->start+buffer->first_free] = b;
            rv = buffer->first_free;

            num = buffer->max_allocations - buffer->start;
            for(i=buffer->first_free; i<num; i++)
            {
                if (buffer->data[buffer->start+i] == 0)
                    break;
            }
            buffer->first_free = i;

            _alBufReleaseNum(buffer, id);
        }

    }

    return rv;
}

/* Replaces the buffer's data pointer */
/* needed for OpenAL */
const void *
_alBufReplace(_alBuffers *buffer, unsigned int id, unsigned int n, void *data)
{
    _alBufferData *buf;
    const void *rv = NULL;

    assert(data != 0);
    assert(buffer != 0);
    assert(buffer->id == id);
    assert(buffer->start+n < buffer->max_allocations);
    assert(buffer->data[buffer->start+n] != 0);
    assert(buffer->data[buffer->start+n]->ptr != 0);

    buf = _alBufGet(buffer, id, n);
    if (buf)
    {
        rv = buf->ptr;
        buf->ptr = data;
        _alBufReleaseData(buf, id);
    }

    return rv;
}

#ifdef BUFFER_DEBUG
_alBufferData *
_alBufGetDebug(_alBuffers *buffer, unsigned int id, unsigned int n, char locked, char *file, int line)
{
    if (n == UINT_MAX) return NULL;

    assert(buffer != 0);
    assert(buffer->id == id);

    n += buffer->start;

    assert(n < buffer->max_allocations);
    assert(buffer->data != 0);

#ifndef _AL_NOTHREADS
    if (!locked && buffer->data[n] != 0)
    {
        int r = _aaxMutexLockDebug(buffer->data[n]->mutex, file, line);
        if (r < 0) { PRINT("error: %i at %s line %i\n", -r, file, line); }
    }
#endif

    return buffer->data[n];
}
#endif

_alBufferData *
_alBufGetNormal(_alBuffers *buffer, unsigned int id, unsigned int n, char locked)
{
    if (n == UINT_MAX) return NULL;

    assert(buffer);
    assert(buffer->id == id);

    n += buffer->start;

#ifndef _AL_NOTHREADS
    if (!locked && buffer->data[n] != NULL) {
        _aaxMutexLock(buffer->data[n]->mutex);
    }
#endif

    assert(n < buffer->max_allocations);
    assert(buffer->data != 0);

    return buffer->data[n];
}

#ifndef _AL_NOTHREADS
void
_alBufRelease(_alBuffers *buffer, unsigned int id, unsigned int n)
{
    assert(buffer != 0);
    assert(buffer->id == id);

    n += buffer->start;

    assert(n < buffer->max_allocations);
    assert(buffer->data != 0);
    assert(buffer->data[n] != 0);
    assert(buffer->data[n]->ptr != 0);

    _aaxMutexUnLock(buffer->data[n]->mutex);
}
#endif

void *
_alBufGetDataPtr(const _alBufferData *data)
{
    assert(data != 0);

    return (void*)data->ptr;
}

void *
_alBufSetDataPtr(_alBufferData *data, void *user_data)
{
    void *ret = NULL;

    assert(data != 0);
    assert(user_data != 0);

    ret = (void*)data->ptr;
    data->ptr = user_data;
    
    return ret;
}

#ifndef _AL_NOTHREADS
# ifndef NDEBUGTHREADS
void
_alBufReleaseDataDebug(const _alBufferData *data, DISREGARD(unsigned int id), char *file, int line)
{
    assert(data != 0);

    _aaxMutexUnLockDebug(data->mutex, file, line);
}
# endif

void
_alBufReleaseDataNormal(const _alBufferData *data, DISREGARD(unsigned int id))
{
    _aaxMutexUnLock(data->mutex);
}
#endif

unsigned int
_alBufGetNumNoLock(const _alBuffers *buffer, unsigned int id)
{
    assert(buffer != 0);
    assert(buffer->id == id);

    return buffer->num_allocated;
}

#ifdef BUFFER_DEBUG
unsigned int
_alBufGetNumDebug(_alBuffers *buffer, DISREGARD(unsigned int id), DISREGARD(char lock), char *file, int line)
{
    assert(buffer != 0);
    assert(buffer->id == id);

#ifndef _AL_NOTHREADS
    _aaxMutexLockDebug(buffer->mutex, file, line);
#endif

    return buffer->num_allocated;

}
#endif

unsigned int
_alBufGetNumNormal(_alBuffers *buffer, DISREGARD(unsigned int id), DISREGARD(char lock))
{
#ifndef _AL_NOTHREADS
    _aaxMutexLock(buffer->mutex);
#endif

    return buffer->num_allocated;
}

unsigned int
_alBufGetMaxNumNoLock(const _alBuffers *buffer, DISREGARD(unsigned int id))
{
    assert(buffer != 0);
    assert(buffer->id == id);

    return buffer->max_allocations;
}

unsigned int
_alBufGetMaxNumNormal(_alBuffers *buffer, DISREGARD(unsigned int id), DISREGARD(char lock))
{
    assert(buffer != 0);
    assert(buffer->id == id);

#ifndef _AL_NOTHREADS
    _aaxMutexLock(buffer->mutex);
#endif

    return buffer->max_allocations;
}

#ifndef _AL_NOTHREADS
void
_alBufReleaseNumNormal(_alBuffers *buffer, DISREGARD(unsigned int id), DISREGARD(char lock))
{
    assert(buffer);
    assert(buffer->id == id);

    _aaxMutexUnLock(buffer->mutex);
}
#endif


/* needed for OpenAL */
unsigned int
_alBufGetPos(_alBuffers *buffer, unsigned int id, void *data)
{
    unsigned int i, start, num, max;

    assert(data != 0);
    assert(buffer != 0);
    assert(buffer->id == id);

    _alBufGetNum(buffer, id);

    start = buffer->start;
    num = buffer->num_allocated;
    max = buffer->max_allocations - start;
    for (i=0; i<max; i++)
    {
        if (buffer->data[start+i] && buffer->data[start+i]->ptr == data) break;
        if (--num == 0) break;
    }
    if (!num || (i == max)) {
        i = UINT_MAX;
    }

    _alBufReleaseNum(buffer, id);

    return i;
}

#ifdef BUFFER_DEBUG
_alBufferData *
_alBufPopDebug(_alBuffers *buffer, unsigned int id, char locked, char *file, int line)
{
    _alBufferData *rv;

    assert(buffer != 0);
    assert(buffer->id == id);

    if (!locked) {
        _alBufGetNum(buffer, id);
    }

    if (((buffer->num_allocated == 0) || (buffer->data[buffer->start] == 0))
        && (buffer->num_allocated || buffer->data[buffer->start]))
    {
        unsigned int i, start = buffer->start;
        printf("start: %i, num: %i, max: %i\n", start, buffer->num_allocated,
                                                 buffer->max_allocations);
        // if (buffer->data[start] == 0)
        {
            printf("buffer->data[%i] == 0 in file '%s' at line %i\n",
                    start, file, line);
            for(i=0; i<buffer->max_allocations; i++)
                printf("%zx ", (size_t)buffer->data[i]);
            printf("\n");
          //return NULL;
        }
    }

    rv = _alBufPopNormal(buffer, id, 1);

    if (!locked) {
        _alBufReleaseNum(buffer, id);
    }

    return rv;
}
#endif

_alBufferData *
_alBufPopNormal(_alBuffers *buffer, unsigned int id, char locked)
{
    _alBufferData *rv = NULL;

    assert(buffer != 0);
    assert(buffer->id == id);

    if (!locked) {
        _alBufGetNum(buffer, id);
    }

    if (buffer->num_allocated > 0)
    {
        unsigned int start = buffer->start;

        rv = _alBufGet(buffer, id, 0);
        buffer->data[start] = NULL;
        _alBufReleaseData(rv, id);

        /*
         * Initially the starting pointer is increased, if the buffer is full
         * shift the remaining buffers from src to dst.
         * Always decrease the num_allocated counter accordingly.
         */
        if (--buffer->num_allocated > 0)
        {
            buffer->start++;
            if (buffer->first_free) {
                buffer->first_free--;
            }
        }
        else
        {
            buffer->first_free = 0;
            buffer->start = 0;
        }
    }

    if (!locked) {
        _alBufReleaseNum(buffer, id);
    }

    return rv;
}

void
_alBufPushNormal(_alBuffers *buffer, unsigned int id, const _alBufferData *data, char locked)
{
    assert(buffer != 0);
    assert(buffer->id == id);

    if (__alBufFreeSpace(buffer, id, locked))
    {
        unsigned int pos; 

        if (!locked) {
            _alBufGetNum(buffer, id);
        }

        pos = buffer->start + buffer->first_free++;

        assert(buffer->data[pos] == NULL);

        buffer->data[pos] = (_alBufferData *)data;
        buffer->num_allocated++;

        if (buffer->data[++pos] != 0)
        {
            unsigned int i, max = buffer->max_allocations - buffer->start;
            for(i=buffer->first_free; i<max; i++)
            {
                if (buffer->data[buffer->start+i] == 0)
                    break;
            }
            buffer->first_free = i;
        }

        if (!locked) {
            _alBufReleaseNum(buffer, id);
        }
    }
}

#ifdef BUFFER_DEBUG
void *
_alBufRemoveDebug(_alBuffers *buffer, unsigned int id, unsigned int n,
                        char locked, char num_locked, char *file, int lineno)
{
#if 0
    void * r = _alBufRemoveNormal(buffer, id, n, locked, num_locked);
    PRINT("remove: %s at line %i: %x\n", file, lineno, n);
    return r;
#else
   _alBufferData *buf;
    void *rv = 0;

    assert(buffer != 0);
    assert(buffer->id == id);
    assert(buffer->start+n < buffer->max_allocations);
    assert(buffer->data != 0);

    if (num_locked) {
        _alBufReleaseNum(buffer, id);
    }
    _alBufGetNumDebug(buffer, id, 1, file, lineno);

    buf = _alBufGetDebug(buffer, id, n, locked, file, lineno);
    if (buf)
    {
        assert(buf->reference_ctr > 0);

        /*
         * If the counter doesn't equal to zero this buffer was referenced
         * by another buffer. So just detach it and let the last referer
         * take care of it.
         */
        if (--buf->reference_ctr == 0)
        {
# ifndef _AL_NOTHREADS
            _aaxMutexDestroy(buf->mutex);
# endif
            rv = (void*)buf->ptr;
            free(buf);
            buf = 0;
        }
        else {
            _alBufReleaseData(buf, id);
        }

        buffer->data[buffer->start+n] = NULL;
        buffer->num_allocated--;
        if (buffer->first_free > n) {
            buffer->first_free = n;
        }
    }

# ifndef _AL_NOTHREADS
    _alBufReleaseNumNormal(buffer, id, 1);
# endif
    if (num_locked) {
        _alBufGetNum(buffer, id);
    }

    return rv;
#endif
}
#endif

void *
_alBufRemoveNormal(_alBuffers *buffer, unsigned int id, unsigned int n,
                                         char locked, char num_locked)
{
    _alBufferData *buf;
    void *rv = 0;

    assert(buffer != 0);
    assert(buffer->id == id);
    assert(buffer->start+n < buffer->max_allocations);
    assert(buffer->data != 0);

    if (num_locked) {
        _alBufReleaseNum(buffer, id);
    }
    _alBufGetNumNormal(buffer, id, 1);

    buf = _alBufGetNormal(buffer, id, n, locked);
    if (buf)
    {
        assert(buf->reference_ctr > 0);

        /*
         * If the counter doesn't equal to zero this buffer was referenced
         * by another buffer. So just detach it and let the last referer
         * take care of it.
         */
        if (--buf->reference_ctr == 0)
        {
#ifndef _AL_NOTHREADS
            _aaxMutexDestroy(buf->mutex);
#endif
            rv = (void*)buf->ptr;
            free(buf);
            buf = 0;
        }
        else {
            _alBufReleaseData(buf, id);
        }

        buffer->data[buffer->start+n] = NULL;
        buffer->num_allocated--;
        if (buffer->first_free > n) {
            buffer->first_free = n;
        }
    }

#ifndef _AL_NOTHREADS
    _alBufReleaseNumNormal(buffer, id, 1);
#endif
    if (num_locked) {
        _alBufGetNum(buffer, id);
    }
    
    return rv;
}


#ifdef BUFFER_DEBUG
void 
_alBufClearDebug(_alBuffers *buffer, unsigned int id,
                      _alBufFreeCallback cb_free,
                      DISREGARD(char *file), DISREGARD(int lineno))
{
    _alBufClearNormal(buffer, id, cb_free);
    PRINT("clear: %s at line %i\n", file, lineno);
}
#endif

void
_alBufClearNormal(_alBuffers *buffer, unsigned int id,
                     _alBufFreeCallback cb_free)
{
    unsigned int n, max, start;

    assert(buffer != 0);
    assert(buffer->id == id);
    assert(buffer->data != 0);

    _alBufGetNum(buffer, id);

    start = buffer->start;
    max = buffer->max_allocations - start;
    for (n=0; n<max; n++)
    {
        void *rv = _alBufRemoveNormal(buffer, id, n, 0, 1);
        if (rv && cb_free) cb_free(rv);
    }
    buffer->start = 0;

    _alBufReleaseNum(buffer, id);
}


#ifdef BUFFER_DEBUG
void
_alBufEraseDebug(_alBuffers **buffer, unsigned int id,
                      _alBufFreeCallback cb_free,
                      DISREGARD(char *file), DISREGARD(int lineno))
{
    _alBufEraseNormal(buffer, id, cb_free);
    PRINT("erase: %s at line %i\n", file, lineno);
}
#endif

void
_alBufEraseNormal(_alBuffers **buf, unsigned int id,
                     _alBufFreeCallback cb_free)
{
    assert(buf != 0);

    if (*buf)
    {
        _alBuffers *buffer = *buf;
#if 1
        _alBufClear(buffer, id, cb_free);
#else
        unsigned int max;

        assert(buffer->id == id);

        _alBufGetNumNormal(buffer, id, 1);

        max = buffer->max_allocations - buffer->start;
        if (max)
        {
            do
            {
                _alBufferData *dptr = _alBufPopNormal(buffer, id, 1);
                if (dptr && dptr->ptr)
                {
                    dptr->reference_ctr--;
                    if (cb_free)
                    {
                        if (!dptr->reference_ctr) {
                            cb_free((void*)dptr->ptr);
                        }
                        _alBufDestroyDataNoLock(dptr);
                    }
                }
            }
            while (--max);
        }
#endif
        free(buffer->data);

#ifndef _AL_NOTHREADS
        _aaxMutexDestroy(buffer->mutex);
#endif
        free(buffer);
        *buf = 0;
    }
}

/* -------------------------------------------------------------------------- */

#define BUFFER_INCREMENT(a)	((((a)/BUFFER_RESERVE)+1)*BUFFER_RESERVE)

static int
__alBufFreeSpace(_alBuffers *buffer, int id, char locked)
{
    unsigned int start, num, max;
    int rv = 0;

    if (!locked) {
        _alBufGetNum(buffer, id);
    }
    
    start = buffer->start;
    num = buffer->num_allocated;
    max = buffer->max_allocations;

    assert((start+num) <= max);
    if (((start+num+1) == max) || ((start+buffer->first_free+1) == max))
    {
        _alBufferData **ptr = buffer->data;

        if (start)
        {
            max -= start;

            memmove(ptr, ptr+start, max*sizeof(void*));
            memset(ptr+max, 0, start*sizeof(void*));
            buffer->start = 0;
            rv = -1;
        }
        else			/* increment buffer size */
        {
            max = BUFFER_INCREMENT(buffer->max_allocations);

            ptr = realloc(buffer->data, max*sizeof(_alBufferData*));
            if (ptr)
            {
                unsigned int size;

                buffer->data = ptr;

                ptr += buffer->max_allocations;
                size = max - buffer->max_allocations;
                memset(ptr, 0, size*sizeof(_alBufferData*));

                buffer->max_allocations = max;
                rv = -1;
            }
        }
    }
    else {
        rv = -1;
    }

#ifdef BUFFER_DEBUG
    if (buffer->data[buffer->start+buffer->first_free] != 0)
    {
        unsigned int i;

        printf("add, buffer->first_free: %i\n", buffer->first_free);
        for(i=0; i<buffer->max_allocations; i++)
            printf("%zx ", (size_t)buffer->data[i]);
        printf("\n");
    }

    assert(buffer->start+buffer->first_free < buffer->max_allocations);
//  assert(buffer->data[buffer->start+buffer->first_free] == 0);
#endif

    if (!locked) {
        _alBufReleaseNum(buffer, id);
    }

    return rv;
}

