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

#define FILE_PATH		SRC_PATH"/wasp.wav"
#define DISTANCE		1000.0
#define SPEED			120.0

ALfloat SourcePos[] = { -DISTANCE, 200.0, 200.0 };
ALfloat SourceVel[] = {  SPEED, 0.00, 0.00 };

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

   do {
      unsigned int no_samples, fmt;
      char bps, channels;
      void *data;
      int freq;

      data = fileLoad(infile, &no_samples, &freq, &bps, &channels, &fmt);
      testForError(data, "Input file not found.\n");

      do
      {
         float d = -DISTANCE;
         ALuint buffer, source[2];
         ALenum format;

         if (!data) break;

         if      ((bps == 8) && (channels == 1)) format = AL_FORMAT_MONO8;
         else if ((bps == 8) && (channels == 2)) format = AL_FORMAT_STEREO8;
         else if ((bps == 16) && (channels == 1)) format = AL_FORMAT_MONO16;
         else if ((bps == 16) && (channels == 2)) format = AL_FORMAT_STEREO16;
         else break;

         alGetError();
         alGenBuffers(1, &buffer);
         alBufferData(buffer, format, data, no_samples*bps/8, freq);
         free(data);
         testForALError();

         alGenSources(1, source);
         testForALError();

         alSourcei(source[0], AL_BUFFER, buffer);
         alSourcei(source[0], AL_LOOPING, AL_TRUE);
         alSourcei(source[0], AL_SOURCE_RELATIVE, AL_TRUE);
         alSourcef(source[0], AL_REFERENCE_DISTANCE, 50.0);
         testForALError();

         alSourcefv(source[0], AL_POSITION, SourcePos);
         alSourcefv(source[0], AL_VELOCITY, SourceVel);
         testForALError();

         /* set listener values */
         alListenerfv(AL_POSITION,    ListenerPos);
         alListenerfv(AL_VELOCITY,    ListenerVel);
         alListenerfv(AL_ORIENTATION, ListenerOri);
         testForALError();

         alDistanceModel(AL_INVERSE_DISTANCE);

         alSourcePlay(source[0]);

         while(d < DISTANCE)
         {
            msecSleep(50);

            d += SPEED*1000.0f*0.05f/3600.0f;
            SourcePos[0] = d;
#if 1
            printf("dist: %3.4f\tpos (% f, % f, % f)\n", d,
                     SourcePos[0], SourcePos[1], SourcePos[2]);
#endif

            alSourcefv(source[0], AL_POSITION, SourcePos);
            testForALError();
         }

         alSourceStop(source[0]);

         alDeleteSources(1, source);
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
