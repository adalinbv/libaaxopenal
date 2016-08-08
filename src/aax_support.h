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

#ifndef _AAX_SUPPORT_H
#define _AAX_SUPPORT_H 1

#include <aax/aax.h>
#include <AL/al.h>
#include <AL/alc.h>

#include <base/buffers.h>

typedef struct
{
    const char *name;
    int enumVal;
} _oalEnumValue_s;

const _oalEnumValue_s *_oalAAXGetEnum(const char *name);

const char* _oalAAXGetExtensions(const char**);
const char* _oalAAXGetCtxExtensions(const char**);
const char* _oalAAXGetDriverSpecifiers(enum aaxRenderMode);
const char* _oalAAXGetDeviceSpecifiersAll(enum aaxRenderMode);
ALCboolean _oalAAXGetExtensionSupport(const char *);
ALCboolean _oalAAXGetCtxExtensionSupport(const char *);
const char *_oalAAXGetVersionString(const void *);
const char * _oalAAXGetRendererString(const void*);

ALenum _oalAAXFormatToFormat(enum aaxFormat, unsigned char);
ALuchar _oalGetBitsPerSampleFromFormat(ALenum);
enum aaxFormat _oalFormatToAAXFormat(ALenum);
enum aaxDistanceModel _oalDistanceModeltoAAXDistanceModel(ALenum, char);
ALsizei _oalGetChannelsFromFormat(ALenum);

unsigned int _oalOffsetInBytesToAAXOffset(unsigned long, unsigned, enum aaxFormat);
unsigned int _oalOffsetInSamplesToAAXOffset(unsigned int, unsigned);
unsigned long _oalAAXOffsetToOffsetInBytes(unsigned int, unsigned, enum aaxFormat);
unsigned int _oalAAXOffsetToOffsetInSamples(unsigned int, unsigned);

void _oalSetReverb(aaxConfig, float, float, float, float, float);

#endif

