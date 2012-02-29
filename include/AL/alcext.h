/*
 * alcext.h : OpenAL extensions header definitions.
 *
 * (C) Copyright 2012 by Erik Hofman
 * (C) Copyright 2012 by Adalin B.V.
 *
 */

#ifndef __alcext_h
#define __alcext_h 1

#include <AL/al.h>
#include <AL/alc.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
# define ALCEXT_APIENTRY __cdecl
# define ALCEXT_API __attribute__((dllexport))
#else
# define ALCEXT_APIENTRY
# if __GNUC__ >= 4
#  define ALCEXT_API __attribute__((visibility("default")))
# else
#  define ALCEXT_API extern
# endif
#endif

#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
# pragma export on
#endif

#ifndef ALC_AAX_capture_loopback
#define ALC_AAX_capture_loopback 1
#define ALC_FORMAT_AAX				0x270020
#define ALC_FREQUENCY_AAX			ALC_FREQUENCY
#define ALC_CHANNELS_AAX			AL_CHANNELS
#define ALC_BITS_AAX				AL_BITS
ALCEXT_API void ALCEXT_APIENTRY alcCaptureiAAX(ALCdevice * device, ALCenum attrib, ALCint value);
ALCEXT_API void ALCEXT_APIENTRY alcGetCaptureivAAX(ALCdevice * device, ALCenum attrib, ALCint *value);
#endif

#ifndef ALC_EXT_EFX
#define ALC_EXT_EFX 1
#define ALC_EFX_MAJOR_VERSION			0x20001
#define ALC_EFX_MINOR_VERSION 			0x20002
#define ALC_MAX_AUXILIARY_SENDS			0x20003
#endif


#ifdef __cplusplus
}
#endif

#endif /* !__alcext_h */

