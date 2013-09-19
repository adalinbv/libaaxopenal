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

#include <aax/aax.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <base/types.h>

#include "api.h"
#include "aax_support.h"

static const char* aaxExtensions[];
static const char* aaxCtxExtensions[];
static const _oalEnumValue_s aaxEnums[];
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

enum aaxDistanceModel
_oalDistanceModeltoAAXDistanceModel(ALenum dist_model, char dist_delay)
{
    const char *enabled = getenv("OPENAL_ENABLE_DISTANCE_DELAY");

    if (enabled && atoi(enabled)) {
        dist_delay = AL_TRUE;
    }

    if ((dist_model > AL_DISTANCE_DELAY_MODEL_AAX) &&
        (dist_model <= AL_EXPONENT_DISTANCE_DELAY_CLAMPED_AAX))
    {
        dist_delay = AL_TRUE;
        dist_model -= 0x270000;
    }

    dist_model -= AL_INVERSE_DISTANCE;
    dist_model += AAX_AL_INVERSE_DISTANCE;

    if (dist_delay) dist_model |= AAX_DISTANCE_DELAY;
    else dist_model &= ~AAX_DISTANCE_DELAY;

    return dist_model;
}


const _oalEnumValue_s *
_oalAAXGetEnum(const char *name)
{
    const _oalEnumValue_s *e;
    unsigned int i;

    for (i=0; ((e = &aaxEnums[i]) != NULL) && e->name; i++)
    {
        if (!strcasecmp(name, e->name))
            break;
    }

    return e;
}

ALCboolean
_oalAAXGetExtensionSupport(const char *name)
{
    ALCboolean r = ALC_FALSE;
    unsigned int i;
    const char* s;

    for (i=0; (s = aaxExtensions[i]) != NULL; i++)
    {
        r = (!strcasecmp(name, s));
        if (r) break;
    }

    return (r) ? ALC_TRUE : ALC_FALSE;
}

const char*
_oalAAXGetExtensions(const char **ext)
{
    static char retstr[2048] = "\0\0";
    char* ptr = (char *)retstr;

    if (*ptr == '\0')
    {
        unsigned max_strlen = 2048;
        unsigned int i;
        const char *s;

        for (i=0; (s = ext[i]) != NULL; i++)
        {
            unsigned slen = strlen(s)+1;
            if (slen < max_strlen)
            {
                snprintf(ptr, max_strlen, "%s ", s);
                max_strlen -= slen;
                ptr += slen;
            }
        }

        ext = aaxExtensions;
        for (i=0; (s = ext[i]) != NULL; i++)
        {
            unsigned slen = strlen(s)+1;
            if (slen < max_strlen)
            {
                snprintf(ptr, max_strlen, "%s ", s);
                max_strlen -= slen;
                ptr += slen;
            }
        }

        *(ptr-1) = '\0';
        retstr[2046] = '\0';	/* alywas end with \0\0 no matter what */
        retstr[2047] = '\0';
    }

    return (const char *)retstr;
}

ALCboolean
_oalAAXGetCtxExtensionSupport(const char *name)
{
    ALCboolean r = ALC_FALSE;
    unsigned int i;
    const char *s;

    for (i=0; (s = aaxCtxExtensions[i]) != NULL; i++)
    {
        r = (!strcasecmp((const char *)name, s));
        if (r) break;
    }

    return (r) ? ALC_TRUE : ALC_FALSE;
}

const char*
_oalAAXGetCtxExtensions(const char** exts)
{
    static char retstr[2048] = "\0\0";
    char* ptr = (char *)retstr;

    if (*ptr == '\0')
    {
        unsigned max_strlen = 2048;
        unsigned int i;
        const char *s;

        if (exts)
        {
            for (i=0; (s = exts[i]) != NULL; i++)
            {
                unsigned slen = strlen(s)+1;

                if (slen < max_strlen)
                {
                    snprintf(ptr, max_strlen, "%s ", s);
                    max_strlen -= slen;
                    ptr += slen;
                }
            }
        }

        exts = aaxCtxExtensions;
        for (i=0; (s = exts[i]) != NULL; i++)
        {
            unsigned slen = strlen(s)+1;

            if (slen < max_strlen)
            {
                snprintf(ptr, max_strlen, "%s ", s);
                max_strlen -= slen;
                ptr += slen;
            }
        }
        *(ptr-1) = '\0';
        retstr[2046] = '\0';	/* alywas end with \0\0 no matter what */
        retstr[2047] = '\0';
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
        tmp = tracks * aaxGetBitsPerSample(fmt)/8;
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
        tmp = tracks * aaxGetBitsPerSample(fmt)/8;
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
        }
        else
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

static const char* aaxCtxExtensions[] =
{
  "ALC_EXT_capture",
  "ALC_AAX_capture_loopback",

  NULL				/* always last */
};

static const char* aaxExtensions[] =
{
//"AL_AAX_environment",
  "AL_AAX_distance_delay_model",
  "AL_AAX_frequency_filter",
//"AL_AAX_reverb",

  NULL				/* always last */
};

/**
 * Enums
 */
static const _oalEnumValue_s aaxEnums[] =
{
  {"AL_INVERSE_DISTANCE_DELAY_AAX",       AL_INVERSE_DISTANCE_DELAY_AAX},
  {"AL_INVERSE_DISTANCE_DELAY_CLAMPED_AAX",AL_INVERSE_DISTANCE_DELAY_CLAMPED_AAX},
  {"AL_LINEAR_DISTANCE_DELAY_AAX",        AL_LINEAR_DISTANCE_DELAY_AAX},
  {"AL_LINEAR_DISTANCE_DELAY_CLAMPED_AAX",AL_LINEAR_DISTANCE_DELAY_CLAMPED_AAX},
  {"AL_EXPONENT_DISTANCE_DELAY_AAX",      AL_EXPONENT_DISTANCE_DELAY_AAX},
  {"AL_EXPONENT_DISTANCE_DELAY_CLAMPED_AAX",AL_EXPONENT_DISTANCE_DELAY_CLAMPED_AAX},

  {NULL, 0}				  /* always last */
};

