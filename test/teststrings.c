
#include <stdio.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "driver.h"

#define MAX_DATA        16
#define TEST_STRING "The quick brown fox jumped over the lazy dog"
const char *tstr = TEST_STRING;

int main()
{
   ALCint data[MAX_DATA];
   ALCdevice *device = NULL;
   ALCcontext *context = NULL;
   ALenum error;
   int status;
   char *s;

   int attributes[] = { ALC_FREQUENCY, 44100,
                        ALC_SYNC, ALC_FALSE,
                        ALC_MONO_SOURCES, 256,
                        ALC_STEREO_SOURCES, 8,
                        0 };

   device = alcOpenDevice("Null");
   if (device == NULL)
   {
      printf("\nNo default audio device available.\n");
      return -1;
   }

   /* Just a small test to see if this works properly */
#if 0
   alcCloseDevice(device);
   device = alcOpenDevice("OSS Software");
#endif

   context = alcCreateContext(device, attributes);
   if (context == NULL)
   {
      printf("\nUnable to create a valid context.\n");
      return -2;
   }

   alcMakeContextCurrent(context);

   alcGetIntegerv(device, ALC_MONO_SOURCES, 1, data);
   printf("no. supported mono sources: %u\n", data[0]);
   testForError(device, "invalid context");

   alcGetIntegerv(device, ALC_STEREO_SOURCES, 1, data);
   printf("no. supported stereo sources: %u\n", data[0]);
   testForError(device, "invalid context");

   if ((error = alcGetError(device)) != ALC_NO_ERROR)
      printf("\nALC error #%x: %s\n", error, alcGetString(device, error));

   s = (char *)alGetString(AL_VENDOR);
   printf("OpenAL vendor string: %s\n", s);
   if ((error = alGetError()) != AL_NO_ERROR)
      printf("AL error #%x: %s\n", error, alGetString(error));

   s = (char *)alGetString(AL_RENDERER);
   printf("OpenAL renderer string: %s\n", s);
   if ((error = alGetError()) != AL_NO_ERROR)
      printf("AL error #%x: %s\n", error, alGetString(error));

   s = (char *)alGetString(AL_VERSION);
   printf("OpenAL version string: %s\n", s);
   if ((error = alGetError()) != AL_NO_ERROR)
      printf("AL error #%x: %s\n", error, alGetString(error));

   if (alcIsExtensionPresent(device, "ALC_EXT_capture"))
      printf("ALC_EXT_capture extension found.\n");

   status = alGetEnumValue((ALchar *)"AL_FORMAT_MONO_FLOAT32");
   printf("#AL_FORMAT_MONO_FLOAT32 = %x\n", status);

   status = alcGetEnumValue(device, "ALC_DEFAULT_DEVICE_SPECIFIER");
   printf("#ALC_DEFAULT_DEVICE_SPECIFIER = %x\n", status);
   status = alcGetEnumValue(device, "ALC_CAPTURE_SAMPLES");
   printf("#ALC_CAPTURE_SAMPLES = %x\n", status);

   if (alIsExtensionPresent((ALchar *)"AL_LOKI_quadriphonic"))
      printf("AL_LOKI_quadriphonic extension found.\n");

   if (!alIsExtensionPresent((ALchar *)"AL_ARBITRARY_extension"))
      printf("AL_ARBITRARY_extension not found (as expected).\n");
   else
      printf("Error: None exsisting extension found!\n");

   /* Generate an error */
   printf("The following execution should generate an error:\n\t");
   alGetSourcei(1, -1, 0);
   error = alGetError();
   printf("   AL error #%X: %s\n", error, alGetString(error));

   context = alcGetCurrentContext();
   device = alcGetContextsDevice(context);
   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);

   return 0;
}
