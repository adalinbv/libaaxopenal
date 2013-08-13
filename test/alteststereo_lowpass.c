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

#define FILTER_SRC		1
#define FILE_PATH		SRC_PATH"/stereo.wav"
#define EXTENSION		"AL_AAX_frequency_filter"

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
         ALuint buffer, source;
         ALint q, status;
         ALenum format;

         if (!data) break;

         if      ((bps == 4) && (channels == 1)) format = AL_FORMAT_MONO_IMA4;
         else if ((bps == 4) && (channels == 2)) format = AL_FORMAT_STEREO_IMA4;
         else if ((bps == 8) && (channels == 1)) format = AL_FORMAT_MONO8;
         else if ((bps == 8) && (channels == 2)) format = AL_FORMAT_STEREO8;
         else if ((bps == 16) && (channels == 1)) format = AL_FORMAT_MONO16;
         else if ((bps == 16) && (channels == 2)) format = AL_FORMAT_STEREO16;
         else break;

         alGetError();
         alGenBuffers(1, &buffer);
         alBufferData(buffer, format, data, no_samples*bps*channels/8, freq);
         free(data);
         testForALError();

         alGenSources(1, &source);
         testForALError();

         alSourcei(source, AL_BUFFER, buffer);
         alSourcei(source, AL_LOOPING, AL_FALSE);
         testForALError();
#if FILTER_SRC
         alSourcef(source, AL_FREQUENCY_FILTER_GAINLF_AAX, 1.0);
         alSourcef(source, AL_FREQUENCY_FILTER_GAINHF_AAX, 0.0);
         alSourcef(source, AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX, 500.0);
         alSourcei(source, AL_FREQUENCY_FILTER_ENABLE_AAX, AL_TRUE);
         printf("frequency filter applied to source.\n");
#endif

         /* set listener values */
#if !FILTER_SRC
         alListenerf(AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX, 500.0);
         alListenerf(AL_FREQUENCY_FILTER_GAINLF_AAX, 1.0);
         alListenerf(AL_FREQUENCY_FILTER_GAINHF_AAX, 0.0);
         alListeneri(AL_FREQUENCY_FILTER_ENABLE_AAX, AL_TRUE);
         printf("frequency filter applied to listener.\n");
#endif
         testForALError();

         alSourcePlay(source);

         q = 0;
         do
         {
            msecSleep(50);

            q++;
            if (q > 10)
            {
               static int fc_changed = 0;
               ALint offs, offb;
               float off_s;
               q = 0;

               alGetSourcef(source, AL_SEC_OFFSET, &off_s);
               alGetSourcei(source, AL_SAMPLE_OFFSET, &offs);
               alGetSourcei(source, AL_BYTE_OFFSET, &offb);

               printf("buffer position: %5.2f seconds (%i samples, %i bytes)\n",
                      off_s, offs, offb);

               if ((off_s > 33.0) && !fc_changed)
               {
                  printf("adjusting cutoff frequency to 5000Hz\n");
#if FILTER_SRC
                  alSourcef(source, AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX, 5000.0);
#else
                  alListenerf(AL_FREQUENCY_FILTER_CUTOFF_FREQ_AAX, 5000.0);
#endif
                  fc_changed = 1;
               }
            }
            alGetSourcei(source, AL_SOURCE_STATE, &status);
         }
         while (status == AL_PLAYING);

         alDeleteSources(1, &source);
         alDeleteBuffers(1, &buffer);

      }
      while(0);

   }
   else
      printf("AL_AAX_frequency_filter extension not available.\n");

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   testForALCError(device);

   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}
