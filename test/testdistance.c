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
#include "driver.h"
#include "wavfile.h"

#define FILE_PATH               SRC_PATH"/wasp.wav"

const ALfloat NullVec[3] = { 0.0, 0.0, 0.0 };

ALfloat SourcePos[] = { 100.0, 100.0, 0.0 };
ALfloat SourceVel[] = { -10.0, 0.0, 10.0 };

ALfloat ListenerPos[] = { 100.0, 90.0, 5.0 };
ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
ALfloat ListenerOri[] = { 0.707f, 0.0f, -0.707f,  0.0f, 1.0f, 0.0f };


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

   /* set listener values */
   alListenerfv(AL_POSITION, NullVec);
   alListenerfv(AL_VELOCITY, NullVec);
   alListenerfv(AL_ORIENTATION, ListenerOri);

   do {
      unsigned int no_samples, fmt;
      char bps, channels;
      void *data;
      int freq;

      data = fileLoad(infile, &no_samples, &freq, &bps, &channels, &fmt);
      testForError(data, "Input file not found.\n");

      do {
         ALuint buffer, source[2];
         ALenum format;
         float dist;

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

         alGenSources(2, (ALuint *)&source);
         testForALError();
         alSourcei(source[1], AL_BUFFER, buffer);
         alSourcei(source[1], AL_LOOPING, AL_TRUE);
         alSourcefv(source[1], AL_VELOCITY, SourceVel);
         alSourcei(source[1], AL_SOURCE_RELATIVE, AL_FALSE);
         alSourcePlay(source[1]);
         testForALError();

         dist = 100.0f;
         while(dist > -100)
         {
            ALfloat pos[3];

            nanoSleep(4e7);

            SourcePos[0] = 100 + dist;
            dist -= 0.33f;
#if 0
            printf("dist: %5.4f\tpos (% f, % f, % f)\n", dist,
                     SourcePos[0], SourcePos[1], SourcePos[2]);
#endif

            pos[0] = SourcePos[0] - ListenerPos[0];
            pos[1] = SourcePos[1] - ListenerPos[1];
            pos[2] = SourcePos[2] - ListenerPos[2];
printf("pos: %f %f %f\n", pos[0], pos[1], pos[2]);

            alSourcefv(source[1], AL_POSITION, pos);

         };

         alSourceStop(source[1]);

         alDeleteSources(2, (ALuint *)&source);
         alDeleteBuffers(1, &buffer);

      } while (0);

   } while(0);

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   testForALCError(device);

   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}
