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
#include "driver.h"
#include "wavfile.h"

#define RECORD_TIME_SEC		5.0f

int main(int argc, char **argv)
{
   ALCdevice *device = NULL;
   ALCcontext *context = NULL;
   char *devname;

   devname = getDeviceName(argc, argv);

   device = alcOpenDevice(devname);
   testForError(device, "No default audio device available.");

   if (!alcIsExtensionPresent(device, "ALC_EXT_capture"))
   {
      printf("No ALC_EXT_capture support available.\n");
      alcCloseDevice(device);
      return(-1);
   }

   context = alcCreateContext(device, NULL);
   testForError(context, "Unable to create a valid context.");

   alcMakeContextCurrent(context);
   testForALCError(device);

   do {
      unsigned int no_samples, bytes;
      char bps, channels;
      ALCdevice *capture;
      ALenum format = 0;
      int16_t *data;
      ALsizei freq;

      freq = 44100;
      channels = 1;
      bps = 16;

      if      ((bps == 8) && (channels == 1)) format = AL_FORMAT_MONO8;
      else if ((bps == 8) && (channels == 2)) format = AL_FORMAT_STEREO8;
      else if ((bps == 16) && (channels == 1)) format = AL_FORMAT_MONO16;
      else if ((bps == 16) && (channels == 2)) format = AL_FORMAT_STEREO16;
      else
      {
         printf("ALERT: Unsupported format\n");
         exit(-1);
      }
  
      no_samples = RECORD_TIME_SEC * freq;
      bytes = no_samples * channels * bps >> 3;
      data = malloc(bytes);
      if (!data) break;

      capture = alcCaptureOpenDevice(devname, freq, format, bytes);
      if (capture)
      {
         ALuint i, buffer, source;
         ALint status = 0;
         int max;

         printf("Capturing %5.1f seconds of audio\n", RECORD_TIME_SEC);
         alcCaptureStart(capture);
         do
         {
            nanoSleep(1e7);
            alcGetIntegerv(capture, ALC_CAPTURE_SAMPLES, 1, &status);
#if 1
            printf("Record buffer position: %3i%%\r", (status*100)/no_samples);
#endif
         }
         while (status < no_samples);
         printf("\n");

         alcCaptureSamples(capture, data, no_samples);
         alcCaptureStop(capture);
         alcCaptureCloseDevice(capture);

         max = 0;
         for (i=0; i<no_samples; i++)
            if (max < data[i]) max = data[i];
         printf("Maximum recorded value: %i\n", max);

         printf("Play back the recorded audio sample\n");
         alGenSources(1, &source);
         alGenBuffers(1, &buffer);

         alBufferData(buffer, format, data, no_samples*bps*channels>>3, freq);
         alSourcei(source, AL_BUFFER, buffer);
         alSourcePlay(source);

         do
         {
            nanoSleep(1e7);
            alGetSourcei(source, AL_SOURCE_STATE, &status);
         }
         while (status == AL_PLAYING);

         alDeleteSources(1, &source);
         alDeleteBuffers(1, &buffer);
      }
      else
      {
         printf("Unable to open capture device.\n");
         break;
      }

      free(data);
   }
   while(0);

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   testForALCError(device);

   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}

