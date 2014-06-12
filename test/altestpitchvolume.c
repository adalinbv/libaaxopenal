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

#include <base/types.h>
#include "driver.h"
#include "wavfile.h"

#define FILE_PATH		SRC_PATH"/tictac.wav"

#ifndef M_E
# define M_E			2.7182818284590452354
#endif

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

      do
      {
         ALfloat pitch, gain, pf, gf;
         ALuint buffer, source;
         ALenum format;
         ALint q, status;
         float _time = 0.0;

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
         alSourcei(source, AL_LOOPING, AL_TRUE);
         alSourcePlay(source);

         q = 0;
         pf = -0.0011f;
         gf = 0.0007f;
         pitch = 1.0f;
         gain = 0.1f;
         do {
            msecSleep(10);
            _time += 0.01f;

            if ((pitch <= 0.001f) || (pitch >= 2.0f)) pf = -pf;
            if ((gain <= 0.001f) || (gain >= 1.0f)) gf = -gf;

#if 1
            pitch += pf;
            alSourcef(source, AL_PITCH, pitch);
#endif
#if 1
            gain += gf;
            alSourcef(source, AL_GAIN, (exp(gain)-1)/(M_E-1));
#endif
            if (q++ > 10)
            {
               ALint offs, offb;
               float off_s;
               q = 0;

               alGetSourcef(source, AL_SEC_OFFSET, &off_s);
               alGetSourcei(source, AL_SAMPLE_OFFSET, &offs);
               alGetSourcei(source, AL_BYTE_OFFSET, &offb);
#if 1
               printf("gain: %5.4f, pitch: %5.4f\n", gain, pitch);
#endif
            }
            alGetSourcei(source, AL_SOURCE_STATE, &status);
         } while (_time < 21);

         alDeleteSources(1, &source);
         alDeleteBuffers(1, &buffer);

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
