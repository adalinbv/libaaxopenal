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

# define __ALGETV(NAME) 	alGet##NAME##v
# define __ALGET(NAME)		alGet##NAME

# define ALGETV(NAME)		__ALGETV(NAME)
# define ALGET(NAME)		__ALGET(NAME)

AL_API void AL_APIENTRY
ALGETV(N)(ALenum attrib, T *value)
{
    if (!value)
    {
        _oalStateSetError(AL_INVALID_VALUE);
        return;
    }

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        *value = (T)_oalGetDopplerFactor();
        break;
    case AL_SPEED_OF_SOUND:
        *value = (T)_oalGetSoundVelocity();
        break;
    case AL_DISTANCE_MODEL:
        *value = (T)_oalGetDistanceModel();
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        *value = (T)_oalGetDopplerVelocity();
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }
}

AL_API T AL_APIENTRY
ALGET(N)(ALenum attrib)
{
    T ret = 0;

    switch (attrib)
    {
    case AL_DOPPLER_FACTOR:
        ret = (T)_oalGetDopplerFactor();
        break;
    case AL_SPEED_OF_SOUND:
        ret = (T)_oalGetSoundVelocity();
        break;
    case AL_DISTANCE_MODEL:
        ret = (T)_oalGetDistanceModel();
        break;
#ifdef AL_VERSION_1_0
    case AL_DOPPLER_VELOCITY:
        ret = (T)_oalGetDopplerVelocity();
        break;
#endif
    default:
        _oalStateSetError(AL_INVALID_ENUM);
    }

    return ret;
}

# undef __ALGETV
# undef __ALGET
# undef ALGETV
# undef ALGET
# undef N
# undef T
#endif
