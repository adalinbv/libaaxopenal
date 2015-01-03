/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * This file is in the Public Domain and comes with no warranty.
 * Erik Hofman <erik@ehofman.com>
 *
 */
#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>	/* nanosleep */
#if HAVE_STRINGS_H
# include <strings.h>
#endif

#include <aax/aax.h>
#include <AL/alc.h>
#include <AL/al.h>

#include "driver.h"

enum
{
   NULL_DEVICE=0,
   SOFTWARE_DEVICE,
   OSS_DEVICE,
   ALSASOFT_DEVICE,
   ALSA_DEVICE,
   DMEDIA_DEVICE,
   MAX_DEVICES
};

char *
getDeviceName(int argc, char **argv)
{
   static char devname[255];
   int len = 255;
   char *s;

   s = getCommandLineOption(argc, argv, "-d");
   if (s)
   {
      strncpy((char *)&devname, s, len);
      len -= strlen(s);

      s = getCommandLineOption(argc, argv, "-r");
      if (s)
      {
         strncat((char *)&devname, " on ", len);
         len -= 4;

         strncat((char *)&devname, s, len);
      }
      s = (char *)&devname;
   }

   return s;
}

char *
getRenderer(int argc, char **argv)
{
   char *renderer = 0;
   renderer = getCommandLineOption(argc, argv, "-r");
   return renderer;
}

int
getNumSources(int argc, char **argv)
{
   int num = 0;
   char *ret = getCommandLineOption(argc, argv, "-n");
   if (ret) num = atoi(ret);
   return num;
}

float
getPitch(int argc, char **argv)
{
   float num = 1.0;
   char *ret = getCommandLineOption(argc, argv, "-p");
   if (ret) num = (float)atof(ret);
   return num;
}

char *
getInputFile(int argc, char **argv, const char *filename)
{
   char *fn;

   fn = getCommandLineOption(argc, argv, "-i");
   if (!fn) fn = (char *)filename;

   return fn;
}

void
testForError(void *p, char *s)
{
   if (p == NULL)
   {
      printf("\nError: %s\n\n", s);
      exit(-1);
   }
}

void
testForState(int res, const char *func)
{
   if (res -= AAX_TRUE)
   {
      int err = aaxGetErrorNo();
      printf("%s:\t\t%i\n", func, res);
      printf("(%i) %s\n\n", err, aaxGetErrorString(err));
   }
}

void
testForALCError(void *dev)
{
   ALCdevice *device = (ALCdevice*)dev;
   ALenum error;
   error = alcGetError(device);
   if (error != ALC_NO_ERROR)
      printf("\nALC Error %x occurred: %s\n", error, alcGetString(device, error));
}

void
testForALError()
{
   ALenum error;
   error = alGetError();
   if (error != AL_NO_ERROR)
      printf("\nAL Error %x occurred: %s\n", error, alGetString(error));
}


char *
getCommandLineOption(int argc, char **argv, char *option)
{
   int slen = strlen(option);
   char *rv = 0;
   int i;
   
   for (i=0; i<argc; i++)
   {
      if (strncmp(argv[i], option, slen) == 0)
      {
         i++;
         if (i<argc) rv = argv[i];
      }
   }

   return rv;
}

char *strDup(const char *s)
{
   unsigned int len = strlen(s)+1;
   char *p = malloc(len);
   if (p) memcpy(p, s,len);
   return p;
}
