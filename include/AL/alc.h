/*
 * alc.h : OpenAL context header definitions.
 *
 * (C) Copyright 2005-2011 by Erik Hofman
 * (C) Copyright 2007-2011 by Adalin B.V.
 *
 */

#ifndef __alc_h
#define __alc_h 1

#if defined(__cplusplus)
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
# define ALC_APIENTRY __cdecl
# define ALC_API __attribute__((dllexport))
#else
# define ALC_APIENTRY
# if __GNUC__ >= 4
#  define ALC_API __attribute__((visibility("default")))
# else
#  define ALC_API extern
# endif
#endif

#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
# pragma export on
#endif


#define ALC_VERSION_0_1				1

struct _AL_device;
typedef struct _AL_device ALCdevice;
typedef void ALCcontext;


typedef char ALCboolean;
typedef char ALCchar;
typedef char ALCbyte;
typedef unsigned char ALCubyte;
typedef short ALCshort;
typedef unsigned short ALCushort;
typedef int ALCint;
typedef unsigned int ALCuint;
typedef int ALCsizei;
typedef int ALCenum;
typedef float ALCfloat;
typedef double ALCdouble;
typedef void ALCvoid;


#define ALC_FALSE				0
#define ALC_TRUE				1

#define ALC_FREQUENCY				0x1007
#define ALC_REFRESH				0x1008
#define ALC_SYNC				0x1009
#define ALC_MONO_SOURCES			0x1010
#define ALC_STEREO_SOURCES			0x1011

#define ALC_NO_ERROR				ALC_FALSE
#define ALC_INVALID_DEVICE			0xA001
#define ALC_INVALID_CONTEXT			0xA002
#define ALC_INVALID_ENUM			0xA003
#define ALC_INVALID_VALUE			0xA004
#define ALC_OUT_OF_MEMORY			0xA005

#define ALC_DEFAULT_DEVICE_SPECIFIER		0x1004
#define ALC_DEVICE_SPECIFIER			0x1005
#define ALC_EXTENSIONS				0x1006

#define ALC_ATTRIBUTES_SIZE			0x1002
#define ALC_ALL_ATTRIBUTES			0x1003

#define ALC_MAJOR_VERSION			0x1000
#define ALC_MINOR_VERSION			0x1001

#define ALC_CAPTURE_SAMPLES			0x312
#define ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER    0x311
#define ALC_CAPTURE_DEVICE_SPECIFIER		0x310

#ifndef ALC_ENUMERATE_ALL_EXT
#define ALC_ENUMERATE_ALL_EXT 1
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER	0x1012
#define ALC_ALL_DEVICES_SPECIFIER		0x1013
#endif

#ifndef ALC_EXT_EFX
#define ALC_EXT_EFX 1
#define ALC_EFX_MAJOR_VERSION			0x20001
#define ALC_EFX_MINOR_VERSION			0x20002

#endif

ALC_API ALCdevice* ALC_APIENTRY alcOpenDevice (const ALCchar *name);
ALC_API ALCboolean ALC_APIENTRY alcCloseDevice (ALCdevice *device);
ALC_API ALCcontext* ALC_APIENTRY alcCreateContext (const ALCdevice * device, const ALCint * attrList);
ALC_API ALCboolean ALC_APIENTRY alcMakeContextCurrent (ALCcontext * context);
ALC_API void ALC_APIENTRY alcProcessContext (ALCcontext *context);
ALC_API void ALC_APIENTRY alcSuspendContext (ALCcontext *context);
ALC_API void ALC_APIENTRY alcDestroyContext(ALCcontext * context);
ALC_API ALCcontext* ALC_APIENTRY alcGetCurrentContext(void);
ALC_API ALCdevice* ALC_APIENTRY alcGetContextsDevice(ALCcontext * context);
ALC_API ALCboolean ALC_APIENTRY alcIsExtensionPresent(const ALCdevice *device,  const ALCchar *extName);
ALC_API void* ALC_APIENTRY alcGetProcAddress (const ALCdevice *device, const ALCchar *funcName);
ALC_API ALCenum ALC_APIENTRY alcGetEnumValue(const ALCdevice *device, const ALCchar *enumName);
ALC_API ALCenum ALC_APIENTRY alcGetError(ALCdevice * device);
ALC_API const ALCchar* ALC_APIENTRY alcGetString(ALCdevice * device, ALCenum token);
ALC_API void ALC_APIENTRY alcGetIntegerv(ALCdevice * device, ALCenum token, ALCsizei size, ALCint *dest);


ALC_API void ALC_APIENTRY alcCaptureStart(ALCdevice *device);
ALC_API void ALC_APIENTRY alcCaptureStop(ALCdevice *device);
ALC_API void ALC_APIENTRY alcCaptureSamples(ALCdevice *device, ALCvoid *buf, ALCsizei samps);
ALC_API ALCdevice* ALC_APIENTRY alcCaptureOpenDevice(const ALCchar *deviceName, ALCuint freq, ALCenum fmt, ALCsizei bufsize);
ALC_API ALCboolean ALC_APIENTRY alcCaptureCloseDevice(ALCdevice *device);


#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
# pragma export off
#endif

#if defined(__cplusplus)
}
#endif

#endif /* !__alc_h */

