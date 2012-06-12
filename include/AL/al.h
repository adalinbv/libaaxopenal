/*
 * al.h : OpenAL header definitions.
 * 
 * (C) Copyright 2005-2011 by Adalin B.V.
 * (C) Copyright 2007-2011 by Erik Hofman
 *
 */

#ifndef __al_h
#define __al_h 1

#ifdef __cplusplus
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
# define AL_APIENTRY __cdecl
# ifdef ALC_BUILD_LIBRARY
#  define AL_API __declspec(dllexport)
# else
#  define AL_API __declspec(dllimport)
# endif
#else
# define AL_APIENTRY
# if __GNUC__ >= 4
#  define AL_API __attribute__((visibility("default")))
# else
#  define AL_API extern
# endif
#endif

#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
# pragma export on
#endif

typedef unsigned char ALboolean;
typedef signed char ALchar;
typedef unsigned char ALuchar;
typedef signed char ALbyte;
typedef unsigned char ALubyte;
typedef short ALshort;
typedef unsigned short ALushort;
typedef int ALint;
typedef unsigned int ALuint;
typedef int ALsizei;
typedef unsigned int ALenum;
typedef float ALfloat;
typedef double ALdouble;
typedef void ALvoid;



/* Extensions */
#define AL_VERSION_1_0			1
#define AL_VERSION_1_1			1



/* Boolean */
#define AL_FALSE				0
#define AL_TRUE					1

/** Return values */
#define AL_NONE					0

/* ErrorCode */
#define AL_NO_ERROR				0
#define AL_INVALID_NAME				0xA001
#define AL_ILLEGAL_ENUM				0xA002
#define AL_INVALID_ENUM				0xA002
#define AL_INVALID_VALUE			0xA003
#define AL_ILLEGAL_COMMAND			0xA004
#define AL_INVALID_OPERATION			0xA004
#define AL_OUT_OF_MEMORY			0xA005

/* StringName */
#define AL_VENDOR				0xB001
#define AL_VERSION				0xB002
#define AL_RENDERER				0xB003
#define AL_EXTENSIONS				0xB004


/** nativelly supported sound formats */
#define AL_FORMAT_MONO8				0x1100
#define AL_FORMAT_MONO16			0x1101
#define AL_FORMAT_STEREO8			0x1102
#define AL_FORMAT_STEREO16			0x1103



/* *** state related declarations *** */

#define AL_DOPPLER_FACTOR			0xC000
#ifdef AL_VERSION_1_0
# define AL_DOPPLER_VELOCITY			0xC001
#endif
#define AL_SPEED_OF_SOUND			0xC003
#define AL_DISTANCE_MODEL			0xD000
#define AL_INVERSE_DISTANCE			0xD001
#define AL_INVERSE_DISTANCE_CLAMPED		0xD002
#define AL_LINEAR_DISTANCE			0xD003
#define AL_LINEAR_DISTANCE_CLAMPED		0xD004
#define AL_EXPONENT_DISTANCE			0xD005
#define AL_EXPONENT_DISTANCE_CLAMPED		0xD006

AL_API ALenum AL_APIENTRY alGetError(void);

AL_API void AL_APIENTRY alEnable(ALenum attrib);
AL_API void AL_APIENTRY alDisable(ALenum attrib);
AL_API ALboolean AL_APIENTRY alIsEnabled (ALenum attrib);

AL_API void AL_APIENTRY alGetBooleanv (ALenum attrib, ALboolean *value);
AL_API void AL_APIENTRY alGetIntegerv(ALenum attrib, ALint *value);
AL_API void AL_APIENTRY alGetFloatv(ALenum attrib, ALfloat *value);
AL_API void AL_APIENTRY alGetDoublev(ALenum attrib, ALdouble *value);
AL_API ALboolean AL_APIENTRY alGetBoolean (ALenum attrib);
AL_API ALint AL_APIENTRY alGetInteger(ALenum attrib);
AL_API ALfloat AL_APIENTRY alGetFloat(ALenum attrib);
AL_API ALdouble AL_APIENTRY alGetDouble(ALenum attrib);
AL_API const ALchar* AL_APIENTRY alGetString(ALenum attrib);

AL_API void AL_APIENTRY alDistanceModel(ALenum attrib);
AL_API void AL_APIENTRY alDopplerFactor(ALfloat factor);
AL_API void AL_APIENTRY alSpeedOfSound(ALfloat speed);
#ifdef AL_VERSION_1_0
AL_API void AL_APIENTRY alDopplerVelocity(ALfloat velocity);
#endif


/* *** listener and source related declarations *** */
#define AL_SOURCE_RELATIVE			0x202

#define AL_CONE_INNER_ANGLE			0x1001
#define AL_CONE_OUTER_ANGLE			0x1002
#define AL_PITCH				0x1003
#define AL_POSITION				0x1004
#define AL_DIRECTION				0x1005
#define AL_VELOCITY				0x1006
#define AL_LOOPING				0x1007
#define AL_BUFFER				0x1009
#define AL_GAIN					0x100A
#define AL_MIN_GAIN				0x100D
#define AL_MAX_GAIN				0x100E
#define AL_ORIENTATION				0x100F

#define AL_SOURCE_STATE				0x1010
#define AL_INITIAL				0x1011
#define AL_PLAYING				0x1012
#define AL_PAUSED				0x1013
#define AL_STOPPED				0x1014

#define AL_BUFFERS_QUEUED			0x1015
#define AL_BUFFERS_PROCESSED			0x1016

#define AL_REFERENCE_DISTANCE			0x1020
#define AL_ROLLOFF_FACTOR			0x1021
#define AL_CONE_OUTER_GAIN			0x1022
#define AL_MAX_DISTANCE				0x1023

#define AL_SEC_OFFSET				0x1024
#define AL_SAMPLE_OFFSET			0x1025
#define AL_BYTE_OFFSET				0x1026

#define AL_SOURCE_TYPE				0x1027
#define AL_STATIC				0x1028
#define AL_STREAMING				0x1029
#define AL_UNDETERMINED				0x1030

#define AL_CHANNEL_MASK				0x3000

/**
 * void AL_APIENTRY alListener{n}{if}{v}(ALenum paramName, T values);
 */
AL_API void AL_APIENTRY alListenerf(ALenum attrib, ALfloat value);
AL_API void AL_APIENTRY alListenerfv(ALenum attrib, const ALfloat *values);
AL_API void AL_APIENTRY alListener3f(ALenum attrib, ALfloat value1, ALfloat value2, ALfloat value3);
AL_API void AL_APIENTRY alListeneri(ALenum attrib, ALint value);
AL_API void AL_APIENTRY alListeneriv(ALenum attrib, const ALint *values);
AL_API void AL_APIENTRY alListener3i(ALenum attrib, ALint value1, ALint value2, ALint value3);

/**
 * void AL_APIENTRY alGetListener{n}{if}{v}(ALenum param, T *values);
 */
AL_API void AL_APIENTRY alGetListenerf(ALenum attrib, ALfloat *value);
AL_API void AL_APIENTRY alGetListener3f(ALenum attrib, ALfloat *value1, ALfloat *value2, ALfloat *value3);
AL_API void AL_APIENTRY alGetListenerfv(ALenum attrib, ALfloat *values);
AL_API void AL_APIENTRY alGetListeneri(ALenum attrib, ALint *value);
AL_API void AL_APIENTRY alGetListener3i(ALenum attrib, ALint *value1, ALint *value2, ALint *value3);
AL_API void AL_APIENTRY alGetListeneriv(ALenum attrib, ALint *values);

AL_API ALboolean AL_APIENTRY alIsSource (ALuint id);
AL_API void AL_APIENTRY alGenSources(ALsizei num, ALuint *ids);
AL_API void AL_APIENTRY alDeleteSources(ALsizei num, const ALuint *ids);

/**
 * void AL_APIENTRY alSource{n}{if} (ALuint sourceName, ALenum paramName, T value);
 * void AL_APIENTRY alSource{n}{if}v (ALuint sourceName, ALenum paramName, T *values);
 */
AL_API void AL_APIENTRY alSourcef(ALuint id, ALenum attrib, ALfloat value);
AL_API void AL_APIENTRY alSourcefv(ALuint id, ALenum attrib, const ALfloat *values);
AL_API void AL_APIENTRY alSource3f(ALuint id, ALenum attrib, ALfloat value1, ALfloat value2, ALfloat value3);
AL_API void AL_APIENTRY alSourcei(ALuint id, ALenum attrib, ALint value);
AL_API void AL_APIENTRY alSourceiv(ALuint id, ALenum attrib, const ALint *values);
AL_API void AL_APIENTRY alSource3i(ALuint id, ALenum attrib, ALint value1, ALint value2, ALint value3);

/**
 * void AL_APIENTRY alGetSource{n}{if}{v} (ALuint sourceName, ALenum paramName, T *values);
 */
AL_API void AL_APIENTRY alGetSourcef(ALuint id, ALenum attrib, ALfloat *value);
AL_API void AL_APIENTRY alGetSource3f(ALuint id, ALenum attrib, ALfloat *value1, ALfloat *value2, ALfloat *value3);
AL_API void AL_APIENTRY alGetSourcefv(ALuint id, ALenum attrib, ALfloat *values);
AL_API void AL_APIENTRY alGetSourcei(ALuint id, ALenum attrib, ALint *value);
AL_API void AL_APIENTRY alGetSource3i(ALuint id, ALenum attrib, ALint *value1, ALint *value2, ALint *value3);
AL_API void AL_APIENTRY alGetSourceiv(ALuint id, ALenum attrib, ALint *values);

AL_API void AL_APIENTRY alSourceQueueBuffers (ALuint id, ALsizei num, const ALuint *ids);
AL_API void AL_APIENTRY alSourceUnqueueBuffers (ALuint id, ALsizei num,  ALuint *ids);

AL_API void AL_APIENTRY alSourcePlay (ALuint id);
AL_API void AL_APIENTRY alSourcePlayv (ALsizei num, const ALuint *ids);
AL_API void AL_APIENTRY alSourcePause (ALuint id);
AL_API void AL_APIENTRY alSourcePausev (ALsizei num, const ALuint *ids);
AL_API void AL_APIENTRY alSourceStop (ALuint id);
AL_API void AL_APIENTRY alSourceStopv (ALsizei num, const ALuint *ids);
AL_API void AL_APIENTRY alSourceRewind (ALuint id);
AL_API void AL_APIENTRY alSourceRewindv (ALsizei num, const ALuint *ids);



/* *** buffer related declarations *** */

/** sound data description */
#define AL_FREQUENCY				0x2001
#define AL_BITS					0x2002
#define AL_CHANNELS				0x2003
#define AL_SIZE					0x2004

/** Buffer state */
#define AL_UNUSED				0x2010
#define AL_PENDING				0x2011
#define AL_PROCESSED				0x2012

/**
 * Requesting Buffers Names
 */
AL_API void AL_APIENTRY alGenBuffers(ALsizei num, ALuint *ids);

/**
 * Releasing Buffer Names
 */
AL_API void AL_APIENTRY alDeleteBuffers(ALsizei num, const ALuint *ids);

/**
 * Validating a Buffer Name
 */
AL_API ALboolean alIsBuffer(ALuint id);

/**
 * Changing Buffer Attributes
 *
 * void AL_APIENTRY alBuffer{n}{if}(ALuint bufferName, ALenum paramName, T value);
 * void AL_APIENTRY alBuffer{n}{if}v(ALuint bufferName, ALenum paramName, T *values);
 */
AL_API void AL_APIENTRY alBufferf(ALuint id, ALenum attrib, ALfloat value);
AL_API void AL_APIENTRY alBuffer3f(ALuint id, ALenum attrib, ALfloat value1, ALfloat value2, ALfloat value3);
AL_API void AL_APIENTRY alBufferfv(ALuint id, ALenum attrib, const ALfloat* values);

AL_API void AL_APIENTRY alBufferi(ALuint id, ALenum attrib, ALint value);
AL_API void AL_APIENTRY alBuffer3i(ALuint id, ALenum attrib, const ALint, const ALint, const ALint);
AL_API void AL_APIENTRY alBufferiv(ALuint id, ALenum attrib, const ALint* values);

/**
 * Querying Buffer Attributes
 *
 * void AL_APIENTRY alGetBuffer{n}{if}{v}(ALuint bufferName, ALenum paramName, T *values);
 */
AL_API void AL_APIENTRY alGetBufferf(ALuint id, ALenum attrib, ALfloat* value);
AL_API void AL_APIENTRY alGetBuffer3f(ALuint id, ALenum attrib, ALfloat* value1, ALfloat* value2, ALfloat* value3);
AL_API void AL_APIENTRY alGetBufferfv(ALuint id, ALenum attrib, ALfloat* values);

AL_API void AL_APIENTRY alGetBufferi(ALuint id, ALenum attrib, ALint* value);
AL_API void AL_APIENTRY alGetBuffer3i(ALuint id, ALenum attrib, ALint* value1, ALint* value2, ALint* value3);
AL_API void AL_APIENTRY alGetBufferiv(ALuint id, ALenum attrib, ALint* values);

/**
 * Specifying Buffer Content
 */
AL_API void AL_APIENTRY alBufferData(ALuint id, ALenum format, const ALvoid *data, ALsizei size, ALsizei frequency);


/**
 * Extension support
 */
AL_API ALboolean AL_APIENTRY alIsExtensionPresent( const ALchar* name );
AL_API void* AL_APIENTRY alGetProcAddress( const ALchar* name );
AL_API ALenum AL_APIENTRY alGetEnumValue( const ALchar* name );

/**
 * Rendering hints
 */
#define AL_FASTEST		AL_FALSE
#define AL_NICEST		AL_TRUE
#define AL_DONT_CARE		AL_FALSE

AL_API void AL_APIENTRY alHint(ALenum target, ALenum mode);

#if defined(TARGET_OS_MAC) && TARGET_OS_MAC
# pragma export off
#endif

#ifdef __cplusplus
}
#endif

#endif /* __al_h */

