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
#include <base/geometry.h>
#include "driver.h"
#include "wavfile.h"

#define FILE_PATH		SRC_PATH"/tictac.wav"

ALfloat SourcePos[] = { 0.0, 0.0, 10.0 };
ALfloat SourceDir[] = { 0.0, 0.0, 1.0 };
ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };

ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
ALfloat ListenerOri[] = { 0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f };


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
         float ang;
         ALuint buffer, source;
         ALenum format;
         ALint deg;

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
         alSourcef(source, AL_CONE_INNER_ANGLE, 160.0);
         alSourcef(source, AL_CONE_OUTER_ANGLE, 300.0);
         alSourcef(source, AL_CONE_OUTER_GAIN, 0.0);
         alSourcefv(source, AL_POSITION, SourcePos);
         testForALError();

         /* set listener values */
         alListenerfv(AL_POSITION,    ListenerPos);
         alListenerfv(AL_VELOCITY,    ListenerVel);
         alListenerfv(AL_ORIENTATION, ListenerOri);
         testForALError();

         alSourcePlay(source);
         testForALError();

         deg = 0;
         while(deg < 360)
         {
            msecSleep(50);

            ang = (float)deg * GMATH_DEG_TO_RAD;
            SourceDir[0] = sinf(ang);
            SourceDir[2] = -cosf(ang);
            /* SourceDir[2] = -cosf(ang); */
            alSourcefv(source, AL_DIRECTION, SourceDir);
#if 1
            printf("deg: %03u\tpos (% f, % f, % f)\n", deg,
                     SourceDir[0], SourceDir[1], SourceDir[2]);
#endif

            testForALError();
            deg += 1;
         }

         alSourceStop(source);

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
