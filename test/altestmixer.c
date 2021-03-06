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
#include <stdlib.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_TIME_H
# include <time.h>              /* for time, nanosleep */
#endif

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

#include <math.h>

#include <base/types.h>
#include "driver.h"
#include "wavfile.h"

#define DTYPE			float
#define DNAME			"float"
#define DFORMAT			AL_FORMAT_MONO_FLOAT32

#define FILE_PATH		SRC_PATH"/tictac.wav"
#define MAX_SOURCES		1024
#define NO_SOURCES		41
#define TESTING_TIME		30

/* Test Function prototypes */
ALvoid testMultiplesource(ALuint, ALint);

int main (int argc, char *argv[])
{
   const ALCint attr[] = { ALC_FREQUENCY, 44100, ALC_REFRESH, 46 };
   ALfloat listenerPos[] = { 0.0, 0.0, 0.0 };
   ALfloat listenerVel[] = { 0.0, 0.0, 0.0 };
   ALfloat listenerOri[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };
   ALCdevice *device = NULL;
   ALCcontext *context = NULL;
   char *devname, *infile;
   ALCint nsrc, src_supp;

   infile = getInputFile(argc, argv, FILE_PATH);
   devname = getDeviceName(argc, argv);
   printf("Device: %s\n", devname);

   device = alcOpenDevice(devname);
   testForError(device, "No default audio device available.");

   context = alcCreateContext(device, attr);
   testForError(context, "Unable to create a valid context.");

   alcMakeContextCurrent(context);
   testForALCError(device);

   do {
      int rr, f;
      alcGetIntegerv(device, ALC_REFRESH, 1, &rr);
      alcGetIntegerv(device, ALC_FREQUENCY,1, &f);
 
      printf("Playback frequency: %i, requested %i\n", f, 44100);
      printf("Refresh rate: %i, requested: %i\n", rr, 46);
      if (f != 44100 || rr != 46) {
         printf("=== Warning: this may affect CPU usage by a factor of : %4.3f ===\n\n",
                 ((float)f*rr)/(44100.0f*46.0f));
      }
   }
   while (0);

   nsrc = getNumSources(argc, argv);
   if (nsrc == 0) nsrc = NO_SOURCES;

   alcGetIntegerv(device, ALC_MONO_SOURCES, 1, &src_supp);
   if (nsrc > src_supp) {
      printf("Warning: Requested %i sources, but only %i sources are available.\n", nsrc, src_supp);
   }

   do {
      unsigned int no_samples, fmt;
      char bps, channels;
      void *data;
      int freq;

      data = fileLoad(infile, &no_samples, &freq, &bps, &channels, &fmt);
      testForError(data, "Input file not found.\n");

      do
      {
         ALuint buffer;
         ALenum format;

         if (!data) break;

         if      ((bps == 4) && (channels == 1)) format = AL_FORMAT_MONO_IMA4;
         else if ((bps == 4) && (channels == 2)) format = AL_FORMAT_STEREO_IMA4;
         else if ((bps == 8) && (channels == 1)) format = AL_FORMAT_MONO8;
         else if ((bps == 8) && (channels == 2)) format = AL_FORMAT_STEREO8;
         else if ((bps == 16) && (channels == 1)) format = AL_FORMAT_MONO16;
         else if ((bps == 16) && (channels == 2)) format = AL_FORMAT_STEREO16;
         else break;

         printf("sample bps: %i, channels: %i\n", bps, channels);

         alGetError();
         alListenerfv (AL_POSITION, listenerPos);
         alListenerfv (AL_VELOCITY, listenerVel);
         alListenerfv (AL_ORIENTATION, listenerOri);
         testForALError();

#if 1
         alGenBuffers(1, &buffer);
         alBufferData(buffer, format, data, no_samples*bps*channels/8, freq);
         testForALError();
         testMultiplesource(buffer, nsrc);
         alDeleteBuffers(1, &buffer);
#else
         printf("Converting sample to %i-bit "DNAME"\n", 8*sizeof(DTYPE));
         i = no_samples * channels;
         ptr = dst = (DTYPE *)malloc(i * sizeof(DTYPE));
         if (ptr)
         {
            src = data;
            do {
               int16_t s = *src++;
               *dst++ = (DTYPE)s / (DTYPE)(1<<15);
            } while (--i);
            free(data);

            data = ptr;
            bps = 8*sizeof(DTYPE);
            format = DFORMAT;

            alGenBuffers(1, &buffer);
            alBufferData(buffer, format, data, no_samples*channels*bps/8, freq);
            testForALError();
            testMultiplesource(buffer, nsrc);
            alDeleteBuffers(1, &buffer);
         }
#endif
         free(data);
      }
      while(0);

   } while(0);

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   testForALCError(device);

   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}

ALvoid
testMultiplesource (ALuint buffer, ALint sources)
{
   ALuint source[MAX_SOURCES] = { 0 };
   ALint no_sources = 0;
   float dt = 0.0;
   ALfloat radius;
   double anglestep;
   ALfloat pos[3];
   ALint error;
   ALint i;

   srand(time(NULL));

   if (sources > MAX_SOURCES)
   {
      printf("Error: no_sources larger than %i.\n", MAX_SOURCES);
      sources = MAX_SOURCES;
   }

   for (i=0; i<sources; i++)
   {
      alGenSources(1, &source[i]);
      if ((error = alGetError()) != AL_NO_ERROR)
         break;
   }
   no_sources = i;

   printf ("generated %d sources\n", no_sources);

   /* Set sources to located in a circle around the listener */
   anglestep = (2 * 3.1416) / (ALfloat)no_sources;
   radius = 2.0f;

   for (i=0; i<no_sources; i++)
   {
      float pitch, r;

      alSourcei(source[i], AL_BUFFER, buffer);

#if 0
      alSourcef(source[i], AL_AAX_FREQUENCY_FILTER_GAINLF, 1.0);
      alSourcef(source[i], AL_AAX_FREQUENCY_FILTER_GAINHF, 0.0);
      alSourcef(source[i], AL_AAX_FREQUENCY_FILTER_CUTOFF_FREQUENCY, 1000.0);
      alSourcei(source[i], AL_AAX_FREQUENCY_FILTER_ENABLE, AL_TRUE);
#endif

      pos[0] = (float)(cos(anglestep * i) * radius);
      pos[1] = 0.0f;
      pos[2] = (float)(sin(anglestep * i) * radius);
      alSourcefv(source[i], AL_POSITION, pos);
      alSourcei(source[i], AL_LOOPING, AL_TRUE);

      r = (float)rand()/RAND_MAX;
      pitch = 0.33f + r * 0.8f;
      alSourcef(source[i], AL_PITCH, pitch);
   }

   do
   {
      printf("completed: %5.6f\n", dt/30.0);
      for (i=0; i<no_sources; i++)
      {
         alSourcePlay(source[i]);
         testForALError();
         msecSleep(100);
         dt += 0.1f;
      }
      msecSleep(300);
      dt += 0.3f;
   }
   while (dt < 30.0);

   alDeleteSources(no_sources, source);
   testForALError();
}

