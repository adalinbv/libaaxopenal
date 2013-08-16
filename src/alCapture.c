/*
 * Copyright (C) 2007-2013 by Erik Hofman.
 * Copyright (C) 2007-2013 by Adalin B.V.
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

#include <AL/alc.h>
#include <AL/alcext.h>

#include <aaxdefs.h>

#include <base/types.h>

#include "aax_support.h"
#include "api.h"

ALCdevice *
alcCaptureOpenDevice(const ALCchar *name, ALCuint freq, ALCenum fmt, ALCsizei bufsize)
{
    ALCdevice *device = 0;
    unsigned int pos = 0;
    aaxConfig handle;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    if (_oalDevices == 0)
    {
        pos = _intBufCreate(&_oalDevices, _OAL_DEVICE);
        if (pos == UINT_MAX)
        {
            _oalContextSetError(ALC_OUT_OF_MEMORY);
            return 0;
        }
    }

    handle = aaxDriverOpenByName(name, AAX_MODE_READ);
    if (handle != NULL)
    {
        _oalDevice *d = calloc(1, sizeof(_oalDevice));
        if (d)
        {
            enum aaxFormat format;
            ALsizei tracks;
#ifndef AAX_DEFINITIONS
            float refrate;
#endif

            format = _oalFormatToAAXFormat(fmt);
            tracks = _oalGetChannelsFromFormat(fmt);
#if AAX_DEFINITIONS
            aaxMixerSetup(handle, freq, tracks, format, bufsize);
#else
            aaxMixerSetSetup(handle, AAX_FREQUENCY, freq);
            aaxMixerSetSetup(handle, AAX_TRACKS, tracks);
            aaxMixerSetSetup(handle, AAX_FORMAT, format);

            refrate = (float)freq;
            refrate *= (float)tracks;
            refrate *= (float)aaxGetBytesPerSample(format);
            refrate /= (float)bufsize;
            aaxMixerSetSetup(handle, AAX_REFRESHRATE, refrate);
            aaxMixerSetState(handle, AAX_INITIALIZED);
#endif

            d->sync = 0;
            d->lst.handle = handle;
            pos = _intBufAddData(_oalDevices, _OAL_DEVICE, d);

            if (pos != UINT_MAX)
            {
                uint32_t id, devid;
                id = _intBufPosToId(pos);
                devid = _oalIdToDevice(id);
                device = INT_TO_PTR(devid);
            }
        }
    }

    return device;
}

ALCboolean
alcCaptureCloseDevice(ALCdevice *device)
{
    return alcCloseDevice(device);
}

void
alcCaptureStart(ALCdevice *device)
{
    _oalDevice *d;
    uint32_t id;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    id = _oalDeviceToId(device);
    d = _oalFindDeviceById(id);
    if (!d)
    {
        _oalContextSetError(ALC_INVALID_DEVICE);
        return;
    }

    aaxSensorSetState(d->lst.handle, AAX_CAPTURING);
}

void
alcCaptureStop(ALCdevice *device)
{
    _oalDevice *d;
    uint32_t id;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    id = _oalDeviceToId(device);
    d = _oalFindDeviceById(id);
    if (!d)
    {
        _oalContextSetError(ALC_INVALID_DEVICE);
        return;
    }

    aaxSensorSetState(d->lst.handle, AAX_STOPPED);
}

void
alcCaptureiAAX(ALCdevice *device, ALCenum attrib, ALCint value)
{
    _oalDevice *d;
    uint32_t id;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    id = _oalDeviceToId(device);
    d = _oalFindDeviceById(id);
    if (!d)
    {
        _oalContextSetError(ALC_INVALID_DEVICE);
        return;
    }

    switch(attrib)
    {
    case ALC_FORMAT_AAX:
    {
        enum aaxFormat format;
        ALsizei tracks;
        int res;

        format = _oalFormatToAAXFormat(value);
        res = aaxMixerSetSetup(d->lst.handle, AAX_FORMAT, format);
        if (res)
        {
           tracks = _oalGetChannelsFromFormat(value);
           res = aaxMixerSetSetup(d->lst.handle, AAX_TRACKS, tracks);
        }

        if (res) {
            d->format = value;
        } else {
            _oalContextSetError(ALC_INVALID_VALUE);
        }
        break;
    }
    case ALC_FREQUENCY_AAX:
    {
        int res = aaxMixerSetSetup(d->lst.handle, AAX_FREQUENCY, value);
        if (res) {
            d->frequency = value;
        } else {
            _oalContextSetError(ALC_INVALID_VALUE);
        }
        break;
    }
    default:
        _oalContextSetError(ALC_INVALID_ENUM);
        break;
    }
}

void
alcGetCaptureivAAX(ALCdevice *device, ALCenum attrib, ALCint *value)
{
    _oalDevice *d;
    uint32_t id;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    id = _oalDeviceToId(device);
    d = _oalFindDeviceById(id);
    if (!d)
    {
        _oalContextSetError(ALC_INVALID_DEVICE);
        return;
    }

    switch(attrib)
    {
    case ALC_BITS_AAX:
        *value = _oalGetBitsPerSampleFromFormat(d->format);
        break;
    case ALC_CHANNELS_AAX:
        *value = _oalGetChannelsFromFormat(d->format);
        break;
    case ALC_FORMAT_AAX:
        *value = d->format;
        break;
    case ALC_FREQUENCY_AAX:
        *value = d->frequency;
        break;
    default:
        _oalContextSetError(ALC_INVALID_ENUM);
        break;
    }
}

void
alcCaptureSamples(ALCdevice *device, ALCvoid *sdata, ALCsizei samps)
{
    char *data = (char*)sdata;
    _oalDevice *d;
    uint32_t id;

    _AL_LOG(LOG_INFO, __FUNCTION__);

    id = _oalDeviceToId(device);
    d = _oalFindDeviceById(id);
    if (d)
    {
        unsigned tracks = aaxMixerGetSetup(d->lst.handle, AAX_TRACKS);
        int format = aaxMixerGetSetup(d->lst.handle, AAX_FORMAT);
        unsigned bps = aaxGetBytesPerSample(format);
        unsigned int frame_size = bps*tracks;
        aaxBuffer buf = d->lst.buf;

        d->lst.buf = NULL;
        do
        {
            if (!buf) {
               buf = aaxSensorGetBuffer(d->lst.handle);
            }

            if (buf)
            {
                void **ptr;

                aaxBufferSetSetup(buf, AAX_FORMAT, format);
                aaxBufferSetSetup(buf, AAX_TRACKS, tracks);
                aaxBufferSetSetup(buf, AAX_FREQUENCY, d->frequency);

                ptr = aaxBufferGetData(buf);
                if (ptr)
                {
                    int chunk_size, no_samples;

                    no_samples = aaxBufferGetSetup(buf, AAX_NO_SAMPLES);
                    if (no_samples > samps)
                    {
                       unsigned int pos = aaxBufferGetSetup(buf, AAX_POSITION);

                       aaxBufferSetSetup(buf, AAX_POSITION, pos+samps);
                       no_samples = samps;
                       d->lst.buf = buf;
                    }
                    else
                    {
                       aaxBufferDestroy(buf);
                       buf = 0;
                    }

                    chunk_size = no_samples*frame_size;
                    memcpy(data, *ptr, chunk_size);
                    aaxFree(ptr);

                    data += chunk_size;
                    samps -= no_samples;
                }
                else
                {
                    _oalContextSetError(ALC_INVALID_VALUE);
                    break;
                }
            }
            else break;
        }
        while(samps);

        return;
    }

    _oalContextSetError(ALC_INVALID_DEVICE);
}

