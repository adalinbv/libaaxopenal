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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>	/* for snprintf */
#include <stdlib.h>	/* for getenv, atoi */
#include <math.h>	/* for floorf */
#if HAVE_STRINGS_H
#include <strings.h>	/* for strcasecmp */
# include <math.h>	/* for floorf */
#endif
#if HAVE_UNISTD_H
# include <unistd.h>	/* for sysconf */
#endif

#include <aax.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <base/types.h>

#include "api.h"
#include "aax_support.h"

static const _intBuffers aaxExtensionsString;
static const _intBuffers aaxCtxExtensionsString;
static const _intBuffers aaxEnumValues;
void cpuID(unsigned, unsigned[4]);

ALsizei
_oalGetChannelsFromFormat(ALenum format)
{
    ALsizei rv = 0;

    switch(format)
    {
    case AL_FORMAT_MONO8:
    case AL_FORMAT_MONO16:
    case AL_FORMAT_MONO_IMA4:
    case AL_FORMAT_IMA_ADPCM_MONO16_EXT:
    case AL_FORMAT_MONO_FLOAT32:
    case AL_FORMAT_MONO_DOUBLE_EXT:
    case AL_FORMAT_MONO_MULAW_EXT:
    case AL_FORMAT_MONO_ALAW_EXT:
        rv = 1;
        break;
    case AL_FORMAT_STEREO8:
    case AL_FORMAT_STEREO16:
    case AL_FORMAT_STEREO_IMA4:
    case AL_FORMAT_IMA_ADPCM_STEREO16_EXT:
    case AL_FORMAT_STEREO_FLOAT32:
    case AL_FORMAT_STEREO_DOUBLE_EXT:
    case AL_FORMAT_STEREO_MULAW_EXT:
    case AL_FORMAT_STEREO_ALAW_EXT:
        rv = 2;
        break;
    case AL_FORMAT_QUAD8_LOKI:
    case AL_FORMAT_QUAD16_LOKI:
    case AL_FORMAT_QUAD8:
    case AL_FORMAT_QUAD16:
    case AL_FORMAT_QUAD32:
        rv = 4;
        break;
    case AL_FORMAT_51CHN8:
    case AL_FORMAT_51CHN16:
    case AL_FORMAT_51CHN32:
        rv = 6;
        break;
    case AL_FORMAT_61CHN8:
    case AL_FORMAT_61CHN16:
    case AL_FORMAT_61CHN32:
        rv = 7;
        break;
    case AL_FORMAT_71CHN8:
    case AL_FORMAT_71CHN16:
    case AL_FORMAT_71CHN32:
        rv = 8;
        break;
    default:
        rv = 0;
        break;
    }

    return rv;
}

ALuchar
_oalGetBitsPerSampleFromFormat(ALenum format)
{
    ALuchar rv = 0;

    switch(format)
    {
    case AL_FORMAT_MONO_IMA4:
    case AL_FORMAT_STEREO_IMA4:
    case AL_FORMAT_IMA_ADPCM_MONO16_EXT:
    case AL_FORMAT_IMA_ADPCM_STEREO16_EXT:
        rv = 4;
        break;
    case AL_FORMAT_MONO8:
    case AL_FORMAT_MONO_MULAW_EXT:
    case AL_FORMAT_MONO_ALAW_EXT:
    case AL_FORMAT_STEREO_MULAW_EXT:
    case AL_FORMAT_STEREO_ALAW_EXT:
    case AL_FORMAT_STEREO8:
    case AL_FORMAT_QUAD8_LOKI:
    case AL_FORMAT_QUAD8:
    case AL_FORMAT_REAR8:
    case AL_FORMAT_51CHN8:
    case AL_FORMAT_61CHN8:
    case AL_FORMAT_71CHN8:
        rv = 8;
        break;
    case AL_FORMAT_MONO16:
    case AL_FORMAT_STEREO16:
    case AL_FORMAT_QUAD16_LOKI:
    case AL_FORMAT_REAR16:
    case AL_FORMAT_51CHN16:
    case AL_FORMAT_61CHN16:
    case AL_FORMAT_71CHN16:
        rv = 16;
        break;
    case AL_FORMAT_MONO_FLOAT32:
    case AL_FORMAT_STEREO_FLOAT32:
    case AL_FORMAT_REAR32:
    case AL_FORMAT_51CHN32:
    case AL_FORMAT_61CHN32:
    case AL_FORMAT_71CHN32:
        rv = 32;
        break;
    case AL_FORMAT_MONO_DOUBLE_EXT:
    case AL_FORMAT_STEREO_DOUBLE_EXT:
        rv = 64;
        break;
    default:
        rv = 0;
        break;
    }

    return rv;
}

enum aaxFormat
_oalFormatToAAXFormat(ALenum format)
{
    enum aaxFormat ret = AAX_PCM16S;

    switch(format)
    {
    case AL_FORMAT_MONO16:
    case AL_FORMAT_STEREO16:
    case AL_FORMAT_QUAD16_LOKI:
    case AL_FORMAT_QUAD16:
    case AL_FORMAT_REAR16:
    case AL_FORMAT_51CHN16:
    case AL_FORMAT_61CHN16:
    case AL_FORMAT_71CHN16:
        break;

    case AL_FORMAT_MONO8:
    case AL_FORMAT_STEREO8:
    case AL_FORMAT_QUAD8_LOKI:
    case AL_FORMAT_REAR8:
    case AL_FORMAT_51CHN8:
    case AL_FORMAT_61CHN8:
    case AL_FORMAT_71CHN8:
        ret = AAX_PCM8U;
        break;

    case AL_FORMAT_REAR32:
    case AL_FORMAT_51CHN32:
    case AL_FORMAT_61CHN32:
    case AL_FORMAT_71CHN32:
        ret = AAX_PCM32S;
        break;

    case AL_FORMAT_MONO_MULAW_EXT:
    case AL_FORMAT_STEREO_MULAW_EXT:
        ret = AAX_MULAW;
        break;

    case AL_FORMAT_MONO_ALAW_EXT:
    case AL_FORMAT_STEREO_ALAW_EXT:
        ret = AAX_ALAW;
        break;

    case AL_FORMAT_MONO_IMA4:
    case AL_FORMAT_STEREO_IMA4:
    case AL_FORMAT_IMA_ADPCM_MONO16_EXT:
    case AL_FORMAT_IMA_ADPCM_STEREO16_EXT:
        ret = AAX_IMA4_ADPCM;
        break;

    case AL_FORMAT_MONO_FLOAT32:
    case AL_FORMAT_STEREO_FLOAT32:
        ret = AAX_FLOAT;
        break;

    case AL_FORMAT_MONO_DOUBLE_EXT:
    case AL_FORMAT_STEREO_DOUBLE_EXT:
        ret = AAX_DOUBLE;
        break;
    }

    return ret;
}

ALenum
_oalAAXFormatToFormat(enum aaxFormat format, unsigned char tracks)
{
    ALenum ret = AL_FORMAT_STEREO16;

    switch(format)
    {
    case AAX_PCM16S:
        if (tracks == 1) ret = AL_FORMAT_MONO16;
        else if (tracks == 2) ret = AL_FORMAT_STEREO16;
        else if (tracks == 4) ret = AL_FORMAT_QUAD16;
        else if (tracks == 6) ret = AL_FORMAT_51CHN16;
        else if (tracks == 7) ret = AL_FORMAT_61CHN16;
        else if (tracks == 8) ret = AL_FORMAT_71CHN16;
        else ret = 0;
        break;

    case AAX_PCM8U:
        if (tracks == 1) ret = AL_FORMAT_MONO8;
        else if (tracks == 2) ret = AL_FORMAT_STEREO8;
        else if (tracks == 4) ret = AL_FORMAT_QUAD8;
        else if (tracks == 6) ret = AL_FORMAT_51CHN8;
        else if (tracks == 7) ret = AL_FORMAT_61CHN8;
        else if (tracks == 8) ret = AL_FORMAT_71CHN8;
        else ret = 0;
        break;

    case AAX_PCM32S:
        if (tracks == 4) ret = AL_FORMAT_QUAD32;
        else if (tracks == 6) ret = AL_FORMAT_51CHN32;
        else if (tracks == 7) ret = AL_FORMAT_61CHN32;
        else if (tracks == 8) ret = AL_FORMAT_71CHN32;
        else ret = 0;
        break;

    case AAX_FLOAT:
        if (tracks == 1) ret = AL_FORMAT_MONO_FLOAT32;
        else if (tracks == 2) ret = AL_FORMAT_STEREO_FLOAT32;
        else ret = 0;
        break;

    case AAX_DOUBLE:
        if (tracks == 1) ret = AL_FORMAT_MONO_DOUBLE_EXT;
        else if (tracks == 2) ret = AL_FORMAT_STEREO_DOUBLE_EXT;
        else ret = 0;
        break;

    case AAX_MULAW:
        if (tracks == 1) ret = AL_FORMAT_MONO_MULAW_EXT;
        else if (tracks == 2) ret = AL_FORMAT_STEREO_MULAW_EXT;
        else ret = 0;
        break;

    case AAX_ALAW:
        if (tracks == 1) ret = AL_FORMAT_MONO_ALAW_EXT;
        else if (tracks == 2) ret = AL_FORMAT_STEREO_ALAW_EXT;
        else ret = 0;
        break;

    case AAX_IMA4_ADPCM:
        if (tracks == 1) ret = AL_FORMAT_MONO_IMA4;
        else if (tracks == 2) ret = AL_FORMAT_STEREO_IMA4;
        else ret = 0;
        break;

    case AAX_PCM24S:
    default:
      ret = 0;
    }

    return ret;
}

const _oalEnumValue_s *
_oalAAXGetEnum(const char *name)
{
    const _oalEnumValue_s *e = NULL;
    const _intBuffers *enums;
    unsigned int i, n;

    enums = &aaxEnumValues;
    n = _intBufGetNumNoLock(enums, _OAL_ENUM);
    for (i=0; i<n; i++)
    {
        _intBufferData *dptr;

        dptr = _intBufGetNoLock(enums, _OAL_ENUM, i);
        e = _intBufGetDataPtr(dptr);

        if (!strcasecmp((const char *)name, e->name))
            break;
    }

    return e;
}

ALCboolean
_oalAAXGetExtensionSupport(const char *name)
{
    const _intBuffers *exts;
    ALCboolean r = ALC_FALSE;
    unsigned int i, n;

    exts = &aaxExtensionsString;
    n = _intBufGetNumNoLock(exts, _OAL_EXTENSION);
    for (i=0; i<n; i++)
    {
        _intBufferData *dptr;
        const char *str;

        dptr = _intBufGetNoLock(exts, _OAL_EXTENSION, i);
        str = _intBufGetDataPtr(dptr);

        r = (!strcasecmp((const char *)name, str));
        if (r) break;
    }

    return (r) ? ALC_TRUE : ALC_FALSE;
}

const char*
_oalAAXGetExtensions(const _intBuffers *exts)
{
    static char retstr[2048] = "\0";
    char* ptr = (char *)retstr;

    if (*ptr == '\0')
    {
        unsigned max_strlen = 2048;
        unsigned int i, n;

        if (exts)
        {
            n = _intBufGetNumNoLock(exts, _OAL_EXTENSION);
            for (i=0; i<n; i++)
            {
                _intBufferData *dptr;
                const char *s;
                unsigned slen;

                dptr = _intBufGetNoLock(exts, _OAL_EXTENSION, i);
                s = _intBufGetDataPtr(dptr);
                slen = strlen(s) + 1;
                if (slen < max_strlen)
                {
                    snprintf(ptr, max_strlen, "%s ", s);
                    max_strlen -= slen;
                    ptr += slen;
                }
            }
        }

        exts = &aaxExtensionsString;
        n = _intBufGetNumNoLock(exts, _OAL_EXTENSION);
        for (i=0; i<n; i++)
        {
            _intBufferData *dptr;
            const char *s;
            unsigned slen;

            dptr = _intBufGetNoLock(exts, _OAL_EXTENSION, i);
            s = _intBufGetDataPtr(dptr);
            slen = strlen(s) + 1;
            if (slen < max_strlen)
            {
                snprintf(ptr, max_strlen, "%s ", s);
                max_strlen -= slen;
                ptr += slen;
            }
        }
        *(ptr-1) = '\0';
    }

    return (const char *)retstr;
}

ALCboolean
_oalAAXGetCtxExtensionSupport(const char *name)
{
    const _intBuffers *exts;
    ALCboolean r = ALC_FALSE;
    unsigned int i, n;

    exts = &aaxCtxExtensionsString;
    n = _intBufGetNumNoLock(exts, _OAL_EXTENSION);
    for (i=0; i<n; i++)
    {
        _intBufferData *dptr;
        const char *str;

        dptr = _intBufGetNoLock(exts, _OAL_EXTENSION, i);
        str = _intBufGetDataPtr(dptr);

        r = (!strcasecmp((const char *)name, str));
        if (r) break;
    }

    return (r) ? ALC_TRUE : ALC_FALSE;
}

const char*
_oalAAXGetCtxExtensions(const _intBuffers *exts)
{
    static char retstr[2048] = "\0";
    char* ptr = (char *)retstr;

    if (*ptr == '\0')
    {
        unsigned max_strlen = 2048;
        unsigned int i, n;

        if (exts)
        {
            n = _intBufGetNumNoLock(exts, _OAL_EXTENSION);
            for (i=0; i<n; i++)
            {
                _intBufferData *dptr;
                const char *s;
                unsigned slen;

                dptr = _intBufGetNoLock(exts, _OAL_EXTENSION, i);
                s = _intBufGetDataPtr(dptr);
                slen = strlen(s) + 1;
                if (slen < max_strlen)
                {
                    snprintf(ptr, max_strlen, "%s ", s);
                    max_strlen -= slen;
                    ptr += slen;
                }
            }
        }

        exts = &aaxCtxExtensionsString;
        n = _intBufGetNumNoLock(exts, _OAL_EXTENSION);
        for (i=0; i<n; i++)
        {
            _intBufferData *dptr;
            const char *s;
            unsigned slen;

            dptr = _intBufGetNoLock(exts, _OAL_EXTENSION, i);
            s = _intBufGetDataPtr(dptr);
            slen = strlen(s) + 1;
            if (slen < max_strlen)
            {
                snprintf(ptr, max_strlen, "%s ", s);
                max_strlen -= slen;
                ptr += slen;
            }
        }
        *(ptr-1) = '\0';
    }

    return (const char *)retstr;
}

const char*
_oalAAXGetDriverSpecifiers(enum aaxRenderMode mode)
{
    static char retstr[2][2048] = { "\0\0", "\0\0" };
    int m = (mode == AAX_MODE_READ) ? 0 : 1;
    char* ptr;

    ptr = (char*)retstr[m];
    if (*ptr == '\0' && *(ptr+1) == '\0')
    {
        unsigned max_strlen = 2048;
        unsigned i, max;

        max = aaxDriverGetCount(mode);
        for (i=0; i<max; i++)
        {
            aaxConfig cfg = aaxDriverGetByPos(i, mode);

#if 0
            if (!aaxDriverGetSupport(cfg, mode))
            {
                aaxDriverDestroy(cfg);
                continue;
            }
#endif

            if (cfg)
            {
                unsigned dlen;
                const char *d;

                d = aaxDriverGetSetup(cfg, AAX_RENDERER_STRING);
                dlen = strlen(d) + 2;		/* strlen("\0\0") */
                aaxDriverDestroy(cfg);

                if (dlen < max_strlen)
                {
                    snprintf(ptr, max_strlen, "%s", d);
                    max_strlen -= dlen;
                    ptr += (dlen-1);
                    *ptr = '\0';
                }
                else
                    break;
            }
        }
    }

    return (const char *)retstr[m];
}

const char*
_oalAAXGetDeviceSpecifiersAll(enum aaxRenderMode mode)
{
    static char retstr[2][2048] = { "\0\0", "\0\0" };
    int m = (mode == AAX_MODE_READ) ? 0 : 1;
    char* ptr;

    ptr = (char *)retstr[m];
    if (*ptr == '\0' && *(ptr+1) == '\0')
    {
        unsigned max_strlen = 2048;
        unsigned i, max;

        max = aaxDriverGetCount(mode);
        for (i=0; i<max; i++)
        {
            aaxConfig cfg = aaxDriverGetByPos(i, mode);

#if 0
            if (!aaxDriverGetSupport(cfg, mode))
            {
                aaxDriverDestroy(cfg);
                continue;
            }
#endif

            if (cfg)
            {
                unsigned j, max_device, belen;
                const char *be, *d, *s;

                be = aaxDriverGetSetup(cfg, AAX_RENDERER_STRING);
                belen = strlen(be);

                max_device = aaxDriverGetDeviceCount(cfg, mode);
                if (max_device)
                {
                    for (j=0; j<max_device; j++)
                    {
                        unsigned k, max_interface, dlen;

                        d = aaxDriverGetDeviceNameByPos(cfg, j, mode);
						/* 6 = strlen(" on "+"\0\0") */
                        dlen = belen + strlen(d) + 6;
                        if (dlen < max_strlen)
                        {
                            max_interface = aaxDriverGetInterfaceCount(cfg, d, mode);
                            if (max_interface)
                            {
                                for(k=0; k<max_interface; k++)
                                {
                                   unsigned slen;

                                   s = aaxDriverGetInterfaceNameByPos(cfg, d, k, mode);
                                   slen = dlen + strlen(s) + 2;
                                   if (slen < max_strlen)
                                   {
                                       snprintf(ptr, max_strlen, "%s on %s: %s", be, d, s);
                                       max_strlen -= slen;
                                       ptr += (slen-1);
                                       *ptr = '\0';
                                   }
                                }
                            }
                            else 
                            {
                                snprintf(ptr, max_strlen, "%s on %s", be, d);
                                max_strlen -= dlen;
                                ptr += (dlen-1);
                                *ptr = '\0';
                            }
                        }
                        else
                        {
                            aaxDriverDestroy(cfg);
                            break;
                        }
                    }
                }
                else
                {
                    belen += 2; // 2 = strlen("\0\0") */
                    if (belen < max_strlen)
                    {
                        snprintf(ptr, max_strlen, "%s", be);
                        max_strlen -= belen;
                        ptr += (belen-1);
                        *ptr = '\0';
                    }
                }
                aaxDriverDestroy(cfg);
            }
        }
    }

    return (const char *)retstr[m];
}

unsigned int
_oalOffsetInBytesToAAXOffset(unsigned long offs, unsigned tracks, enum aaxFormat fmt)
{
    ALenum format = _oalAAXFormatToFormat(fmt, tracks);
    unsigned int tmp, rv = 0;

    switch (format)
    {
    case AL_FORMAT_MONO_IMA4:
    case AL_FORMAT_STEREO_IMA4:
    case AL_FORMAT_IMA_ADPCM_MONO16_EXT:
    case AL_FORMAT_IMA_ADPCM_STEREO16_EXT:
        /* 65 samples per block of 36 bytes */
        rv = 65*( offs/(36*tracks) );
        offs %= 36;				/* position within one block  */
        if (offs > 4)				/* first sample takes 4 bytes */
        {
            offs -= 4;
            rv += 1 + 2*offs;			/* 2 samples per byte         */
        }
        break;
    default:
        tmp = tracks * _oalGetBitsPerSampleFromFormat(format)/8;
        rv = offs/tmp;
        break;
    }
    return rv;
}

unsigned int
_oalOffsetInSamplesToAAXOffset(unsigned int offs, unsigned tracks)
{
    return offs/tracks;
}

unsigned long
_oalAAXOffsetToOffsetInBytes(unsigned int offs, unsigned tracks, enum aaxFormat fmt)
{
    ALenum format = _oalAAXFormatToFormat(fmt, tracks);
    unsigned long tmp, rv = 0;

    switch (format)
    {
    case AL_FORMAT_MONO_IMA4:
    case AL_FORMAT_STEREO_IMA4:
    case AL_FORMAT_IMA_ADPCM_MONO16_EXT:
    case AL_FORMAT_IMA_ADPCM_STEREO16_EXT:
        tmp = (unsigned int)floorf(offs/65.0);
        offs -= tmp;
        if (offs > 1) {
            offs = 4 + (offs-1)/2;
        }
        rv = (36*tmp + offs) * tracks;
        break;
    default:
        tmp = tracks * _oalGetBitsPerSampleFromFormat(format)/8;
        rv = offs*tmp;
        break;
    }

    return rv;
}

unsigned int
_oalAAXOffsetToOffsetInSamples(unsigned int offs, unsigned tracks)
{
    return offs * tracks;
}

const char *
_oalAAXGetVersionString(const void* config)
{
    static char version[40] = VERSION;
    static char init = 0;

    if (!init)
    {
        aaxConfig cfg = (aaxConfig)config;
        snprintf(version, 40, "%s %s", VERSION, aaxGetVersionString(cfg));
        init = -1;
    }

    return (char*)&version;
} 

const char *
_oalAAXGetRendererString(const void* config)
{
    aaxConfig cfg = (aaxConfig)config;
    static char renderer[80] = "";
    static char init = 0;

    if (!init && cfg)
    {
        const char *rstr = aaxDriverGetSetup(cfg, AAX_RENDERER_STRING);
        unsigned int ncpu = _oalAAXGetNoCores(cfg);
        if (ncpu == 1) {
            snprintf(renderer, 80, "%s", rstr);
        } else
        {
            if (ncpu > _MAX_THREADS) ncpu = _MAX_THREADS;
            snprintf(renderer, 80, "%s using %i cores", rstr, ncpu);
        }
        init = -1;
    }

    return (char*)&renderer;
}

unsigned int
_oalAAXGetNoCores(const void* config)
{
    const char *enabled = getenv("OPENAL_ENABLE_MULTICORE");
    int cores = 1;

    if (!enabled || atoi(enabled))
    {
        aaxConfig cfg = (aaxConfig)config;
        cores = aaxGetNoCores(cfg);
    }

    return cores;
}

/*-------------------------------------------------------------------------- */

/**
 * Context Extensions
 */

#define _MAX_CTX_EXTENSIONS      2
static const _intBufferData aaxCtxExtensions[_MAX_CTX_EXTENSIONS] =
{
    {0, 1, "ALC_EXT_capture"},

    {0, 1, "ALC_AAX_capture_loopback"}
};

static const void *aaxCtxExtensionsPtr[_MAX_CTX_EXTENSIONS] =
{
    (void *)&aaxCtxExtensions[0],
    (void *)&aaxCtxExtensions[1]
};

static const _intBuffers aaxCtxExtensionsString =
{
    0,
    _OAL_EXTENSION,
    _MAX_CTX_EXTENSIONS,
    _MAX_CTX_EXTENSIONS,
    _MAX_CTX_EXTENSIONS,
    (void*)&aaxCtxExtensionsPtr
};

#define _MAX_EXTENSIONS		1
static const _intBufferData aaxExtensions[_MAX_EXTENSIONS] =
{
// {0, 1, "AL_AAX_environment"},
    {0, 1, "AL_AAX_frequency_filter"},
// {0, 1, "AL_AAX_reverb"}
};

static const void *aaxExtensionsPtr[_MAX_EXTENSIONS] =
{
    (void *)&aaxExtensions[0]
// (void *)&aaxExtensions[1]
// (void *)&aaxExtensions[2]
};

static const _intBuffers aaxExtensionsString =
{
    0,
    _OAL_EXTENSION,
    _MAX_EXTENSIONS,
    _MAX_EXTENSIONS,
    _MAX_EXTENSIONS,
    (void*)&aaxExtensionsPtr
};

/**
 * Enums
 */
#define MAX_ENUM          8
static const _oalEnumValue_s _aaxEnumValueDeclaration[MAX_ENUM] =
{
  {"ALC_FREQUENCY",                       ALC_FREQUENCY},
  {"ALC_REFRESH",                         ALC_REFRESH},
  {"ALC_SYNC",                            ALC_SYNC},
  {"ALC_MONO_SOURCES",                    ALC_MONO_SOURCES},
  {"ALC_STEREO_SOURCES",                  ALC_STEREO_SOURCES},
  {"ALC_CAPTURE_SAMPLES",                 ALC_CAPTURE_SAMPLES},
  {"ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER",ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER},
  {"ALC_CAPTURE_DEVICE_SPECIFIER",        ALC_CAPTURE_DEVICE_SPECIFIER}
};

static const _intBufferData _aaxEnumValue[MAX_ENUM] =
{
    {0, 1, (void *)&_aaxEnumValueDeclaration[0]},
    {0, 1, (void *)&_aaxEnumValueDeclaration[1]},
    {0, 1, (void *)&_aaxEnumValueDeclaration[2]},
    {0, 1, (void *)&_aaxEnumValueDeclaration[3]},
    {0, 1, (void *)&_aaxEnumValueDeclaration[4]},
    {0, 1, (void *)&_aaxEnumValueDeclaration[5]},
    {0, 1, (void *)&_aaxEnumValueDeclaration[6]},
    {0, 1, (void *)&_aaxEnumValueDeclaration[7]}
};

static const void *_aaxEnumValuePtr[MAX_ENUM] =
{
    (void *)&_aaxEnumValue[0],
    (void *)&_aaxEnumValue[1],
    (void *)&_aaxEnumValue[2],
    (void *)&_aaxEnumValue[3],
    (void *)&_aaxEnumValue[4],
    (void *)&_aaxEnumValue[5],
    (void *)&_aaxEnumValue[6],
    (void *)&_aaxEnumValue[7]
};

static const _intBuffers aaxEnumValues =
{
    0,
    _OAL_ENUM,
    MAX_ENUM,
    MAX_ENUM,
    MAX_ENUM,
    (void*)&_aaxEnumValuePtr
};

