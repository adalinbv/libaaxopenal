/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * alcinfo.x
 *
 * alcinfo display info about a ALC extension and OpenAL renderer
 *
 * This file is in the Public Domain and comes with no warranty.
 * Erik Hofman <erik@ehofman.com>
 *
 */

#if HAVE_CONFIG_H
# include "config.h"
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


#include <stdio.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#if _WIN32
# include <Windows.h>
#endif

#include <base/logging.h>
#include "driver.h"

#define MAX_DATA	16

static int indentation = 2;
static int maximumWidth = 80;

static void
printChar (int c, int *width)
{
  putchar (c);
  *width = (c == '\n') ? 0 : (*width + 1);
}

static void
indent (int *width)
{
  int i;
  for (i = 0; i < indentation; i++)
    {
      printChar (' ', width);
    }
}

static void
printList(const char *header, char separator, const char eol, const char *extensions)
{
   int width = 0, start = 0, end = 0;

   printf ("%s:\n", header);
   if (extensions == NULL || extensions[0] == '\0') {
      return;
   }

   indent (&width);
   while (1)
   {
      if (extensions[end] == separator || extensions[end] == '\0')
      {
         if (width + end - start + 2 >= maximumWidth)
         {
            printChar ('\n', &width);
            indent (&width);
         }
         while (start < end)
         {
            printChar (extensions[start], &width);
            start++;
         }
         if (extensions[end] == '\0')
         {
            if (separator != '\0' || extensions[end+1] == '\0')
               break;
         }

         start++;
         end++;
         if (extensions[end] == '\0')
         {
            break;
         }
         printChar (eol, &width);
         if (eol == '\n') {
            indent (&width);
         } else {
            printChar (' ', &width);
         }
      }
      end++;
   }
   printChar ('\n', &width);
}

#if _WIN32
static int
terminalWidth()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int ret, rv = maximumWidth;
    ret = GetConsoleScreenBufferInfo(GetStdHandle( STD_OUTPUT_HANDLE ),&csbi);
    if (ret) rv = csbi.dwSize.X;
    return rv;
}
#elif HAVE_SYS_IOCTL_H
static int
terminalWidth()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}
#else
#pragma warning Implement terminal width
static int
terminalWidth() {
    return maximumWidth;
}
#endif

int main(int argc, char **argv)
{
   ALCint data[MAX_DATA];
   ALCdevice *device = NULL;
   ALCcontext *context = NULL;
   ALenum error;
   char *s;

   maximumWidth = terminalWidth()-1;

   if (alcIsExtensionPresent(NULL, "ALC_enumerate_all_EXT") == AL_TRUE)
   {
      s = (char *)alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
      printList("Available output devices", '\0', '\n', s);

      s = (char *)alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
      printList("Available input devices", '\0', '\n', s);
   }
   else if (alcIsExtensionPresent(NULL, "ALC_enumeration_EXT") == AL_TRUE)
   {
      s = (char *)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
      printList("Available output backends", '\0', '\n', s);

      s = (char *)alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
      printList("Available input backends", '\0', '\n', s);
   }

   s = getDeviceName(argc, argv);
   device = alcOpenDevice(s);
   testForError(device, "Audio device not available.");

   context = alcCreateContext(device, NULL);
   testForError(context, "Unable to create a valid context.");

   alcMakeContextCurrent(context);
   testForALCError(device);

   s = (char *)alcGetString(device, ALC_DEFAULT_DEVICE_SPECIFIER);
   printf("Current output device: %s\n", s);
   testForALCError(device);

   error = alcIsExtensionPresent(device, "ALC_EXT_capture");
   if (error)
   {
      s = (char *)alcGetString(device, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
      printf("Current input device:  %s\n", s);
      testForALCError(device);
   }
   printf("capture support: %s\n", (error) ? "yes" : "no");

   alcGetIntegerv(device, ALC_FREQUENCY, 1, data);
   printf("mixer frequency: %u hz\n", data[0]);
   testForError(device, "invalid context");

   alcGetIntegerv(device, ALC_REFRESH, 1, data+1);
   printf("refresh rate:\t%3u hz\n", data[1]);
   testForError(device, "invalid context");

   data[0] = 0;
   alcGetIntegerv(device, ALC_MONO_SOURCES, 1, data);
   printf("mono sources:\t%3u\n", data[0]);
   testForError(device, "invalid context");

   data[0] = 0;
   alcGetIntegerv(device, ALC_STEREO_SOURCES, 1, data);
   printf("stereo sources:\t%3u\n", data[0]);
   testForError(device, "invalid context");

   printf("ALC version:\t");
   alcGetIntegerv(device, ALC_MAJOR_VERSION, 1, data);
   printf("%i.", *data);
   alcGetIntegerv(device, ALC_MINOR_VERSION, 1, data);
   printf("%i\n", *data);
   testForALCError(device);

   s = (char *)alcGetString(device, ALC_EXTENSIONS);
   printList("ALC extensions", ' ', ',', s);
   testForALCError(device);

   s = (char *)alGetString(AL_VENDOR);
   error = alGetError();
   if ((error = alGetError()) != AL_NO_ERROR)
      printf("Error #%x: %s\n", error, alGetString(error));
   else
      printf("OpenAL vendor string  : %s\n", s);

   s = (char *)alGetString(AL_VERSION);
   if ((error = alGetError()) != AL_NO_ERROR)
      printf("Error #%x: %s\n", error, alGetString(error));
   else if (!s)
      printf("Quering AL_VERSION returned NULL pointer!\n");
   else
      printf("OpenAL version string : %s\n", s);

   s = (char *)alGetString(AL_RENDERER);
   if ((error = alGetError()) != AL_NO_ERROR)
      printf("Error #%x: %s\n", error, alGetString(error));
   else
      printf("OpenAL renderer string: %s\n", s);

   s = (char *)alGetString(AL_EXTENSIONS);
   printList("OpenAL extensions", ' ', ',', s);
   testForALCError(device);

/* alut testing mechanism */
   context = alcGetCurrentContext();
   if (context != NULL)
   {
      if (alGetError () != AL_NO_ERROR) {
         printf("Alert: AL error on entry\n");
      }
      else
      {
         if (alcGetError (alcGetContextsDevice (context)) != ALC_NO_ERROR) {
            printf("Alert: ALC error on entry\n");
         }
      }
   }
   else {
      printf("Error: no current context\n");
   }
/* end of alut test */
 
   
   if (alcMakeContextCurrent(NULL) == 0)
      printf("alcMakeContextCurrent failed.\n");

   device = alcGetContextsDevice(context);
   alcDestroyContext(context);
   testForALCError(device);

   if (alcCloseDevice(device) == 0)
      printf("alcCloseDevice failed.\n");

   return 0;
}
