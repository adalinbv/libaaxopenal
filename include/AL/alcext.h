/*
 * alcext.h : OpenAL extensions header definitions.
 *
 * (C) Copyright 2012-2013 by Erik Hofman
 * (C) Copyright 2012-2013 by Adalin B.V.
 *
 */

#ifndef __alcext_h
#define __alcext_h 1

#if defined(__cplusplus)
extern "C" {
#endif

#if defined _WIN32 || defined __CYGWIN__
# define ALCEXT_APIENTRY __cdecl
# ifdef ALC_BUILD_LIBRARY
#  define ALCEXT_API __declspec(dllexport)
# else
#  define ALCEXT_API __declspec(dllimport)
# endif
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

struct _AL_device;
typedef struct _AL_device ALCdevice;

typedef int ALCenum;
typedef int ALCint;

#ifndef ALC_EXT_EFX
# define ALC_EXT_EFX 1
# include "efx.h"
#endif

#ifndef ALC_AAX_capture_loopback
# define ALC_AAX_capture_loopback 1
# define ALC_FORMAT_AAX				0x270020
# define ALC_FREQUENCY_AAX			0x1007
# define ALC_BITS_AAX				0x2002
# define ALC_CHANNELS_AAX			0x2003
ALCEXT_API void ALCEXT_APIENTRY alcCaptureiAAX(ALCdevice * device, ALCenum attrib, ALCint value);
ALCEXT_API void ALCEXT_APIENTRY alcGetCaptureivAAX(ALCdevice * device, ALCenum attrib, ALCint *value);
#endif

#ifndef ALC_EXT_thread_local_context
#define ALC_EXT_thread_local_context 1
typedef ALCboolean  (ALCEXT_APIENTRY *PFNALCSETTHREADCONTEXTPROC)(ALCcontext *context);
typedef ALCcontext* (ALCEXT_APIENTRY *PFNALCGETTHREADCONTEXTPROC)(void);
# ifdef AL_ALEXT_PROTOTYPES
ALC_API ALCboolean  ALCEXT_APIENTRY alcSetThreadContext(ALCcontext *context);
ALC_API ALCcontext* ALCEXT_APIENTRY alcGetThreadContext(void);
# endif
#endif

#ifndef ALC_SOFT_loopback
#define ALC_SOFT_loopback 1
#define ALC_FORMAT_CHANNELS_SOFT		0x1990
#define ALC_FORMAT_TYPE_SOFT			0x1991

/* Sample types */
#define ALC_BYTE_SOFT				0x1400
#define ALC_UNSIGNED_BYTE_SOFT			0x1401
#define ALC_SHORT_SOFT				0x1402
#define ALC_UNSIGNED_SHORT_SOFT			0x1403
#define ALC_INT_SOFT				0x1404
#define ALC_UNSIGNED_INT_SOFT			0x1405
#define ALC_FLOAT_SOFT				0x1406

/* Channel configurations */
#define ALC_MONO_SOFT				0x1500
#define ALC_STEREO_SOFT				0x1501
#define ALC_QUAD_SOFT				0x1503
#define ALC_5POINT1_SOFT			0x1504
#define ALC_6POINT1_SOFT			0x1505
#define ALC_7POINT1_SOFT			0x1506

typedef ALCdevice* (ALCEXT_APIENTRY*LPALCLOOPBACKOPENDEVICESOFT)(const ALCchar*);
typedef ALCboolean (ALCEXT_APIENTRY*LPALCISRENDERFORMATSUPPORTEDSOFT)(ALCdevice*,ALCsizei,ALCenum,ALCenum);
typedef void (ALCEXT_APIENTRY*LPALCRENDERSAMPLESSOFT)(ALCdevice*,ALCvoid*,ALCsizei);
# ifdef AL_ALEXT_PROTOTYPES
ALC_API ALCdevice* ALCEXT_APIENTRY alcLoopbackOpenDeviceSOFT(const ALCchar *deviceName);
ALC_API ALCboolean ALCEXT_APIENTRY alcIsRenderFormatSupportedSOFT(ALCdevice *device, ALCsizei freq, ALCenum channels, ALCenum type);
ALC_API void ALCEXT_APIENTRY alcRenderSamplesSOFT(ALCdevice *device, ALCvoid *buffer, ALCsizei samples);
# endif
#endif

#ifndef ALC_LOKI_audio_channel
#define ALC_LOKI_audio_channel 1
#define ALC_CHAN_MAIN_LOKI			0x500001
#define ALC_CHAN_PCM_LOKI			0x500002
#define ALC_CHAN_CD_LOKI			0x500003
#endif

#ifndef ALC_EXT_DEFAULT_FILTER_ORDER
#define ALC_EXT_DEFAULT_FILTER_ORDER 1
#define ALC_DEFAULT_FILTER_ORDER		0x1100
#endif

#ifndef ALC_EXT_DEDICATED
#define ALC_EXT_DEDICATED 1
#define AL_DEDICATED_GAIN			 0x0001
#define AL_EFFECT_DEDICATED_DIALOGUE		 0x9001
#define AL_EFFECT_DEDICATED_LOW_FREQUENCY_EFFECT 0x9000
#endif

#ifndef ALC_EXT_disconnect
#define ALC_EXT_disconnect 1
#define ALC_CONNECTED				0x313
#endif


#ifndef ALC_EXT_ASA
#define ALC_EXT_ASA 1
#define ALC_ASA_REVERB_ON			'rvon'
#define ALC_ASA_REVERB_GLOBAL_LEVEL		'rvgl'
#define ALC_ASA_REVERB_ROOM_TYPE		'rvrt'
#define ALC_ASA_REVERB_ROOM_TYPE_SmallRoom	0
#define ALC_ASA_REVERB_ROOM_TYPE_MediumRoom	1
#define ALC_ASA_REVERB_ROOM_TYPE_LargeRoom	2
#define ALC_ASA_REVERB_ROOM_TYPE_MediumHall	3
#define ALC_ASA_REVERB_ROOM_TYPE_LargeHall	4
#define ALC_ASA_REVERB_ROOM_TYPE_Plate		5
#define ALC_ASA_REVERB_ROOM_TYPE_MediumChamber	6
#define ALC_ASA_REVERB_ROOM_TYPE_LargeChamber	7
#define ALC_ASA_REVERB_ROOM_TYPE_Cathedral	8
#define ALC_ASA_REVERB_ROOM_TYPE_LargeRoom2	9
#define ALC_ASA_REVERB_ROOM_TYPE_MediumHall2	10
#define ALC_ASA_REVERB_ROOM_TYPE_MediumHall3	11
#define ALC_ASA_REVERB_ROOM_TYPE_LargeHall2	12
#define ALC_ASA_REVERB_PRESET			'rvps'
#define ALC_ASA_REVERB_EQ_GAIN			'rveg'
#define ALC_ASA_REVERB_EQ_BANDWITH		'rveb'
#define ALC_ASA_REVERB_EQ_FREQ			'rvef'
#define ALC_ASA_REVERB_QUALITY			'rvqt'
#define ALC_ASA_REVERB_QUALITY_Max		0x7F
#define ALC_ASA_REVERB_QUALITY_High		0x60
#define ALC_ASA_REVERB_QUALITY_Medium		0x40
#define ALC_ASA_REVERB_QUALITY_Low		0x20
#define ALC_ASA_REVERB_QUALITY_Min		0

#define ALC_ASA_REVERB_SEND_LEVEL		'rvsl'
#define ALC_ASA_OCCLUSION			'occl'
#define ALC_ASA_OBSTRUCTION			'obst'

ALC_API ALenum ALCEXT_APIENTRY alcASAGetSource(ALuint property, ALuint source, ALvoid *data, ALuint* dataSize);
ALC_API ALenum  ALCEXT_APIENTRY alcASASetSource(ALuint property, ALuint source, ALvoid *data, ALuint dataSize);
ALC_API ALenum  ALCEXT_APIENTRY alcASAGetListener(ALuint property, ALvoid *data, ALuint* dataSize);
ALC_API ALenum  ALCEXT_APIENTRY alcASASetListener(ALuint property, ALvoid *data, ALuint dataSize);

typedef ALenum  (*alcASAGetSourceProcPtr) (ALuint property, ALuint source, ALvoid *data, ALuint* dataSize);
typedef ALenum  (*alcASASetSourceProcPtr) (ALuint property, ALuint source, ALvoid *data, ALuint dataSize);
typedef ALenum  (*alcASAGetListenerProcPtr) (ALuint property, ALvoid *data, ALuint* dataSize);
typedef ALenum  (*alcASASetListenerProcPtr) (ALuint property, ALvoid *data, ALuint dataSize);
#endif

#ifndef ALC_EXT_ASA_DISTORTION
#define ALC_EXT_ASA_DISTORTION
#define ALC_ASA_DISTORTION_ENABLE		'dsen'
#define ALC_ASA_DISTORTION_ON			'dson'
#define ALC_ASA_DISTORTION_MIX			'dsmx'
#define ALC_ASA_DISTORTION_TYPE			'dstp'
#define ALC_ASA_DISTORTION_PRESET		'dsps'
#define ALC_ASA_DISTORTION_TYPE_BitBrush	    0
#define ALC_ASA_DISTORTION_TYPE_BufferBeats	    1
#define ALC_ASA_DISTORTION_TYPE_LoFi		    2
#define ALC_ASA_DISTORTION_TYPE_BrokenSpeaker	    3
#define ALC_ASA_DISTORTION_TYPE_Cellphone	    4
#define ALC_ASA_DISTORTION_TYPE_Decimated1	    5
#define ALC_ASA_DISTORTION_TYPE_Decimated2	    6
#define ALC_ASA_DISTORTION_TYPE_Decimated3	    7
#define ALC_ASA_DISTORTION_TYPE_Decimated4	    8
#define ALC_ASA_DISTORTION_TYPE_DistortedFunk	    9
#define ALC_ASA_DISTORTION_TYPE_DistortionCubed	   10
#define ALC_ASA_DISTORTION_TYPE_DistortionSquared  11
#define ALC_ASA_DISTORTION_TYPE_Echo1		   12
#define ALC_ASA_DISTORTION_TYPE_Echo2		   13
#define ALC_ASA_DISTORTION_TYPE_EchoTight1	   14
#define ALC_ASA_DISTORTION_TYPE_EchoTight2	   15
#define ALC_ASA_DISTORTION_TYPE_EverythingBroken   16
#define ALC_ASA_DISTORTION_TYPE_AlienChatter	   17
#define ALC_ASA_DISTORTION_TYPE_CosmicInteference  18
#define ALC_ASA_DISTORTION_TYPE_GoldenPi	   19
#define ALC_ASA_DISTORTION_TYPE_RadioTower	   20
#define ALC_ASA_DISTORTION_TYPE_Waves		   21
#endif

#if defined(__cplusplus)
}
#endif

#endif /* !__alcext_h */

