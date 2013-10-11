
#ifndef __THREADS_H
#define __THREADS_H 1

#if defined(__cplusplus)
extern "C" {
#endif


#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_TIME_H
#include <time.h>
#endif

#include "types.h"


#if HAVE_PTHREAD_H
# include <pthread.h>			/* UNIX */

 typedef struct _oalMutex
 { 
   char initialized;
   pthread_mutex_t mutex;
# ifndef NDEBUG
   const char *name;
   const char *function;
# endif
 } _oalMutex;

#elif defined( _WIN32 )
# include <Windows.h>			/* WINDOWS */

 typedef struct _oalMutex
 {
#ifndef NDEBUG
   HANDLE mutex;
#else
   CRITICAL_SECTION mutex;
   char ready;
#endif
 } _oalMutex;

#endif


#ifndef NDEBUG
#define _oalMutexCreate(a) _oalMutexCreateDebug(a, __FILE__, __FUNCTION__);
#define _oalMutexLock(a) _oalMutexLockDebug(a, __FILE__, __LINE__)
#define _oalMutexUnLock(a) _oalMutexUnLockDebug(a, __FILE__, __LINE__)
void *_oalMutexCreateDebug(void *, const char *, const char *);
int _oalMutexLockDebug(void *, char *, int);
int _oalMutexUnLockDebug(void *, char *, int);
#else
void *_oalMutexCreate(void *);
int _oalMutexLock(void *);
int _oalMutexUnLock(void *);
#endif
void _oalMutexDestroy(void *);
int _oalThreadSwitch();


#if defined(__cplusplus)
}  /* extern "C" */
#endif

#endif /* !__THREADS_H */

