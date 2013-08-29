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


#if defined(N) && defined(T)

# define __ALGETCONTEXTV(NAME)	_imp__alcGet##NAME##v
# define ALGETCONTEXTV(NAME)	__ALGETCONTEXTV(NAME)

ALC_API void ALC_APIENTRY
ALGETCONTEXTV(N)(ALCdevice *device, ALCenum attrib, ALCsizei size, T *value)
{
    char done = 1;

    if ((size <= 0) || !value)
    {
        _oalContextSetError(ALC_INVALID_VALUE);
        return;
    }

    switch(attrib)
    {
    case ALC_MONO_SOURCES:
        *value = (T)_MIN((unsigned)aaxMixerGetNoMonoSources(), 255);
        break;
    case ALC_STEREO_SOURCES:
        *value = (T)_MIN((unsigned)aaxMixerGetNoStereoSources(), 255);
        break;
    case ALC_MAJOR_VERSION:
        *value = (T)_oalContextVersion[0];
        break;
    case ALC_MINOR_VERSION:
        *value = (T)_oalContextVersion[1];
        break;
    case ALC_EFX_MAJOR_VERSION:
        *value = (T)_oalEFXVersion[0];
        break;
    case ALC_EFX_MINOR_VERSION:
        *value = (T)_oalEFXVersion[1];
        break;
    case ALC_ATTRIBUTES_SIZE:                   /* TODO */
    case ALC_ALL_ATTRIBUTES:
        *value = 0;
        break;
    default:
        done = 0;
    }

    if (!done)
    {
        uint32_t id = _oalDeviceToId(device);
        _oalDevice *dev = (_oalDevice *)_oalFindDeviceById(id);
        if (dev)
        {
            aaxConfig config = dev->lst.handle;
            switch(attrib)
            {
            case ALC_FREQUENCY:
                *value = (T)aaxMixerGetFrequency(config);
                break;
            case ALC_REFRESH:
                *value = (T)aaxMixerGetSetup(config, AAX_REFRESHRATE);
                break;
            case ALC_CAPTURE_SAMPLES:
                *value = (T)aaxSensorGetOffset(config, AAX_SAMPLES);
                break;
            default:
                *value = 0;
                _oalContextSetError(ALC_INVALID_ENUM);
            }
        }
        else {
            _oalContextSetError(ALC_INVALID_DEVICE);
        }
    }
}

# undef __ALGETCONTEXTV
# undef ALGETCONTEXTV
# undef N
# undef T
#endif
