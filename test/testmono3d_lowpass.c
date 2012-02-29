/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * This file is in the Public Domain and comes with no warranty.
 * Erik Hofman <erik@ehofman.com>
 *
 */
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <math.h>

#ifdef __APPLE__
# include <OpenAL/al.h>
# include <OpenAL/alc.h>
#else
# include <AL/al.h>
# include <AL/alc.h>
# include <AL/alext.h>
#endif

#ifndef AL_VERSION_1_1
# ifdef __APPLE__
#  include <OpenAL/altypes.h>
#  include <OpenAL/alctypes.h>
#else
#  include <AL/altypes.h>
#  include <AL/alctypes.h>
# endif
#endif

#include "base/types.h"
#include "base/geometry.h"
#include "driver.h"
#include "wavfile.h"

#define FILTER_SRC		1
#define FILE_PATH		SRC_PATH"/tictac.wav"
#define EXTENSION		"AL_AAX_frequency_filter"
#define RADIUS			20.0

ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };
ALfloat SourceVel[] = { 0.00, 0.0, 0.0 };

ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
ALfloat ListenerOri[] = { 0.0f, 0.0f, -1.0f,  0.0f, 1.0f, 0.0f };


int main(int argc, char **argv)
{
   ALCdevice *device = NULL;
   ALCcontext *context = NULL;
   char *devname, *infile;

   infile = getInputFile(argc, argv, FILE_PATH);
   devname = getDeviceName(argc, argv);

   device = alcOpenDevice(devname);
   testForError(device, "No default audio device available.");

   context = alcCreateContext(device, NULL);
   testForError(context, "Unable to create a valid context.");

   alcMakeContextCurrent(context);
   testForALCError(device);

   if (alIsExtensionPresent((ALchar *)EXTENSION))
   {
      unsigned int no_samples, fmt;
      char bps, channels;
      void *data;
      int freq;

      data = fileLoad(infile, &no_samples, &freq, &bps, &channels, &fmt);
      testForError(data, "Input file not found.\n");

      do
      {
         float ang, r = RADIUS;
         ALuint buffer, source;
         ALenum format;
         ALint deg;

         if (!data) break;

         if      ((bps == 4) && (channels == 1)) format = AL_FORMAT_MONO_IMA4;
         else if ((bps == 4) && (channels == 2)) format = AL_FORMAT_STEREO_IMA4;
         else if ((bps == 8) && (channels == 1)) format = AL_FORMAT_MONO8;
         else if ((bps == 8) && (channels == 2)) format = AL_FORMAT_STEREO8;
         else if ((bps == 16) && (channels == 1)) format = AL_FORMAT_MONO16;
         else if ((bps == 16) && (channels == 2)) format = AL_FORMAT_STEREO16;
         else continue;

         alGetError();
         alGenBuffers(1, &buffer);
         alBufferData(buffer, format, data, no_samples*bps/8, freq);
         free(data);
         testForALError();

         alGenSources(1, &source);
         testForALError();

         alSourcei(source, AL_BUFFER, buffer);
         alSourcei(source, AL_LOOPING, AL_TRUE);
         alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
         alSourcef(source, AL_REFERENCE_DISTANCE, 50.0);
         testForALError();

         alSourcefv(source, AL_POSITION, SourcePos);
#if FILTER_SRC
         alSourcef(source, AL_FREQUENCY_FILTER_GAINLF_AAX, 1.0);
         alSourcef(source, AL_FREQUENCY_FILTER_GAINHF_AAX, 0.0);
         alSourcef(source, AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX, 500.0);
         alSourcei(source, AL_FREQUENCY_FILTER_ENABLE_AAX, AL_TRUE);
         printf("frequency filter applied to source.\n");
#endif
         testForALError();

         /* set listener values */
         alListenerfv(AL_POSITION,    ListenerPos);
         alListenerfv(AL_VELOCITY,    ListenerVel);
         alListenerfv(AL_ORIENTATION, ListenerOri);
#if !FILTER_SRC
         alListenerf(AL_FREQUENCY_FILTER_GAINLF_AAX, 1.0);
         alListenerf(AL_FREQUENCY_FILTER_GAINHF_AAX, 0.0);
         alListenerf(AL_FREQUENCY_FILTER_CUTOFF_FREQY_AAX, 500.0);
         alListeneri(AL_FREQUENCY_FILTER_ENABLE_AAX, AL_TRUE);
         printf("frequency filter applied to listener.\n");
#endif
         testForALError();

         alSourcePlay(source);

         deg = 0;
         while(deg < 360)
         {
            nanoSleep(5e7);

            ang = (float)deg / 180.0f * GMATH_PI;
            SourcePos[0] =  r * sinf(ang);
            SourcePos[2] = -r * cosf(ang);
            /* SourcePos[2] = -r * cosf(ang); */
#if 1
            printf("deg: %03u\tpos (% f, % f, % f)\n", deg,
                     SourcePos[0], SourcePos[1], SourcePos[2]);
#endif

            alSourcefv(source, AL_POSITION, SourcePos);
            testForALError();
            deg += 3;
         }

         alSourceStop(source);

         alDeleteSources(1, &source);
         alDeleteBuffers(1, &buffer);

      }
      while(0);
   }
   else
      printf(EXTENSION" extension not available.\n");

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   testForALCError(device);

   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}
