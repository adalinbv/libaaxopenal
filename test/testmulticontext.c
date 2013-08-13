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

#include <base/types.h>
#include "driver.h"
#include "wavfile.h"

#define FILE_PATH		SRC_PATH"/stereo.wav"
#define DEVICE2			"AeonWave Loopback"

int main(int argc, char **argv)
{
   ALCdevice *d1, *device = NULL;
   ALCcontext *c1, *context = NULL;
   char *devname, *infile;

   infile = getInputFile(argc, argv, FILE_PATH);
   devname = getDeviceName(argc, argv);

   device = alcOpenDevice(devname);
   testForError(device, "No default audio device available.");

   context = alcCreateContext(device, NULL);
   testForError(context, "Unable to create a valid context.");

   alcMakeContextCurrent(context);

   d1 = alcOpenDevice(DEVICE2);
   testForError(d1, "Secondary device '"DEVICE2"' is not available.");

   c1 = alcCreateContext(device, NULL);
   testForError(c1, "Unable to create a valid context for secondary device.");

   do {
      unsigned int no_samples, fmt;
      char bps, channels;
      void *data;
      int freq;

      data = fileLoad(infile, &no_samples, &freq, &bps, &channels, &fmt);
      testForError(data, "Input file not found.\n");

      do
      {
         ALuint buffer, source;
         ALint q, status;
         ALenum format;

         if (!data) break;

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
         alGenSources(1, &source);
         testForALError();

         alSourcei(source, AL_BUFFER, buffer);
         alSourcePlay(source);
         testForALError();

         q = 0;
         do
         {
            msecSleep(50);

#if 1
            q++;
            if (q > 10)
            {
               ALint offs, offb;
               float off_s;
               q = 0;

               alGetSourcef(source, AL_SEC_OFFSET, &off_s);
               alGetSourcei(source, AL_SAMPLE_OFFSET, &offs);
               alGetSourcei(source, AL_BYTE_OFFSET, &offb);

               printf("buffer position: %5.2f seconds (%i samples, %i bytes)\n",
                      off_s, offs, offb);
#endif
            }
            alGetSourcei(source, AL_SOURCE_STATE, &status);
         }
         while (status == AL_PLAYING);

         alDeleteSources(1, &source);
         alDeleteBuffers(1, &buffer);

      }
      while(0);

   } while(0);

   alcDestroyContext(c1);
   alcCloseDevice(d1);

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   testForALCError(device);

   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}
