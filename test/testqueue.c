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

#include "base/types.h"
#include "driver.h"
#include "wavfile.h"

#define MAX_SOURCES		256
#define FILE_PATH		SRC_PATH"/tictac.wav"

int main(int argc, char **argv)
{
   ALCdevice *device = NULL;
   ALCcontext *context = NULL;
   char *devname, *infile;
   int q, nsrc;

   infile = getInputFile(argc, argv, FILE_PATH);
   devname = getDeviceName(argc, argv);
   nsrc = getNumSources(argc, argv);
   if (nsrc == 0) nsrc = 1;
   else if (nsrc > MAX_SOURCES) nsrc = MAX_SOURCES;
   printf("number of streaming sources: %i\n", nsrc);

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
         ALuint source[MAX_SOURCES];
         ALuint buffer, buffers[5];
         ALint status;
         ALenum format;
         ALfloat pitch;

         if      ((bps == 8) && (channels == 1)) format = AL_FORMAT_MONO8;
         else if ((bps == 8) && (channels == 2)) format = AL_FORMAT_STEREO8;
         else if ((bps == 16) && (channels == 1)) format = AL_FORMAT_MONO16;
         else if ((bps == 16) && (channels == 2)) format = AL_FORMAT_STEREO16;
         else break;

         alGetError();
         alGenBuffers(1, &buffer);
         alBufferData(buffer, format, data, no_samples*bps>>3, freq);
         free(data);
         testForALError();

         alGenSources(nsrc, source);
         testForALError();

         pitch = getPitch(argc, argv);
         buffers[0]=buffers[1]=buffers[2]=buffers[3]=buffers[4] = buffer;
         for (q=0; q<nsrc; q++)
         {
            alSourcef(source[q], AL_PITCH, pitch);
            alSourcei(source[q], AL_LOOPING, AL_FALSE);
            alSourceQueueBuffers(source[q], 5, buffers);
            alSourcePlay(source[q]);
         }
         testForALError();

         do
         {
            msecSleep(50);
            alGetSourcei(source[0], AL_SOURCE_STATE, &status);
         }
         while (status == AL_PLAYING);

         alDeleteSources(nsrc, source);
         alDeleteBuffers(1, &buffer);

      }

   } while(0);

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   testForALCError(device);

   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}
