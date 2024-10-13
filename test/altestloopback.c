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
# include <AL/alcext.h>
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

int main(int argc, char **argv)
{
   ALCdevice *device = NULL;
   ALCcontext *context = NULL;
   char *devname, *infile;
   ALCint alci;

   infile = getInputFile(argc, argv, FILE_PATH);
   devname = getDeviceName(argc, argv);

   device = alcOpenDevice(devname);
   testForError(device, "No default audio device available.");

   if (!alcIsExtensionPresent(device, "ALC_AAX_capture_loopback"))
   {
      printf("No ALC_AAX_capture_loopback support available.\n");
      alcCloseDevice(device);
      return(-1);
   }

   context = alcCreateContext(device, NULL);
   testForError(context, "Unable to create a valid context.");

   alcMakeContextCurrent(context);
   testForALCError(device);

   if (context)
   {
      unsigned int no_samples, fmt;
      char no_bits, tracks;
      void *data;
      int freq;

      /* read the audio file data */
      data = fileLoad(infile, &no_samples, &freq, &no_bits, &tracks, &fmt);
      testForError(data, "Input file not found.\n");

      if(data)
      {
         ALenum format, capture_fmt;
         ALuint buffer[2], source[1];
         ALint q, status;

         if      ((no_bits == 4) && (tracks == 1)) format = AL_FORMAT_MONO_IMA4;
         else if ((no_bits == 4) && (tracks == 2)) format = AL_FORMAT_STEREO_IMA4;
         else if ((no_bits == 8) && (tracks == 1)) format = AL_FORMAT_MONO8;
         else if ((no_bits == 8) && (tracks == 2)) format = AL_FORMAT_STEREO8;
         else if ((no_bits == 16) && (tracks == 1)) format = AL_FORMAT_MONO16;
         else if ((no_bits == 16) && (tracks == 2)) format = AL_FORMAT_STEREO16;
         else exit(-1);

         /* generate a new buffer */
         alGetError();
         alGenBuffers(2, buffer);

         /* fill the buffer with the previously read audio data */
         alBufferData(buffer[0], format, data, no_samples*tracks*no_bits/8, freq);
         free(data);

         testForALError();
         alGenSources(1, source);
         testForALError();

         /* assing the buffer to the source */
         alSourcei(source[0], AL_BUFFER, buffer[0]);
         alSourcePlay(source[0]);
         testForALError();

         printf("Start capturing using the looback mechanism\n");
         /*
          * set the requested format for the data returned by the
          * capture functions
          */
         capture_fmt = (tracks == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
         alcCaptureiAAX(device, ALC_FORMAT_AAX, capture_fmt);
         testForALCError(device);

         /* start capturing on the playback device */
         alcCaptureStart(device);
         testForALCError(device);

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

               alGetSourcef(source[0], AL_SEC_OFFSET, &off_s);
               alGetSourcei(source[0], AL_SAMPLE_OFFSET, &offs);
               alGetSourcei(source[0], AL_BYTE_OFFSET, &offb);

               printf("buffer position: %5.2f seconds (%i samples, %i bytes)\n",
                      off_s, offs, offb);
#endif
            }
            alGetSourcei(source[0], AL_SOURCE_STATE, &status);
         }
         while (status == AL_PLAYING);

         printf("Capturing and playback stopped\n");
         alcCaptureStop(device);
         testForALCError(device);

         printf("Play back captured samples\n");
         alcGetCaptureivAAX(device, ALC_FREQUENCY_AAX, &alci);
         freq = alci;

         alcGetCaptureivAAX(device, ALC_FORMAT_AAX, &alci);
         format = alci;
         switch(format)
         {
         case AL_FORMAT_MONO8:
             no_bits = 8;
             tracks = 1;
             break;
         case AL_FORMAT_STEREO8:
             no_bits = 8;
             tracks = 2;
             break;
         case AL_FORMAT_MONO16:
             no_bits = 16;
             tracks = 1;
             break;
         case AL_FORMAT_STEREO16:
             no_bits = 16;
             tracks = 2;
             break;
         default:
             printf("Unknow capture format: %s\n", alGetString(format));
             exit(-1);
         }

         data = malloc(no_samples*tracks*no_bits/8);
         if (data)
         {
             /* retrieve some or all of the available captured samples */
             alcCaptureSamples(device, data, no_samples);

             alBufferData(buffer[1],format,data,no_samples*tracks*no_bits/8,freq);
             free(data);

             alSourcei(source[0], AL_BUFFER, 0); // detach the associated buffer
             alSourcei(source[0], AL_BUFFER, buffer[1]);
             alSourcePlay(source[0]);

             do
             {
                msecSleep(10);
                alGetSourcei(source[0], AL_SOURCE_STATE, &status);
             }
             while (status == AL_PLAYING);
         }
         else {
             printf("Insufficient memroy\n");
         }

         alDeleteSources(1, source);
         alDeleteBuffers(2, buffer);
      }
   }

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   testForALCError(device);

   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}
