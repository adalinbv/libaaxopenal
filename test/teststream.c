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
#include <math.h>
#if HAVE_UNISTD_H
# include <unistd.h>
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

#include "base/logging.h"
#include "driver.h"
#include "wavfile.h"

#define NUM_BUFFERS		8
#define FILE_PATH		SRC_PATH"/tictac.wav"


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

   do {
      unsigned int no_samples, fmt;
      char bps, channels;
      void *data;
      int freq;

      data = fileLoad(infile, &no_samples, &freq, &bps, &channels, &fmt);
      testForError(data, "Input file not found.\n");

      if (data)
      {
         ALuint buffers[NUM_BUFFERS];
         ALuint source[2];
         ALuint i, num = 0;
         ALenum format;

         if      ((bps == 8) && (channels == 1)) format = AL_FORMAT_MONO8;
         else if ((bps == 8) && (channels == 2)) format = AL_FORMAT_STEREO8;
         else if ((bps == 16) && (channels == 1)) format = AL_FORMAT_MONO16;
         else if ((bps == 16) && (channels == 2)) format = AL_FORMAT_STEREO16;
         else break;

         alGetError();
         alGenBuffers(NUM_BUFFERS, buffers);
         for (i = 0; i < NUM_BUFFERS; i++) {
            alBufferData(buffers[i], format, data, no_samples*bps>>3, freq);
         }
         free(data);
         testForALError();

         alGenSources(1, source);
         testForALError();

         alSourcei(source[0], AL_BUFFER, 0);
         alSourcei(source[0], AL_LOOPING, AL_FALSE);
         alSourceQueueBuffers(source[0], NUM_BUFFERS, buffers);

         alSourcePlay(source[0]);
         while (num < 25)
         {
            ALint processed, state;

            testForALError();
            printf("playing buffer #%i\r", num);

            alGetSourcei(source[0], AL_BUFFERS_PROCESSED, &processed);
            if (processed > 1)
            {
               ALuint bufs[NUM_BUFFERS];

               if (processed > NUM_BUFFERS) processed = NUM_BUFFERS;

               alSourceUnqueueBuffers(source[0], 1, bufs);
               testForALError();

               alSourceQueueBuffers(source[0], 1, bufs);
               num += 1;
            }

            alGetSourcei(source[0], AL_SOURCE_STATE, &state);
            if (state == AL_STOPPED)
               break;

            msecSleep(50);
         }
         printf("\n");
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
