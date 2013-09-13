
#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#if HAVE_ASSERT_H
# include <assert.h>
#endif
#if HAVE_TIME_H
# include <time.h>
#endif
#if HAVE_MATH_H
# include <math.h>
#endif
#include <errno.h>

#include "threads.h"
#include "logging.h"
#include "types.h"

#define DEBUG_TIMEOUT		3

#if HAVE_PTHREAD_H
# include <string.h>	/* for memcpy */

#define _TH_SYSLOG(a) __oal_log(LOG_SYSLOG, 0, (a), 0, LOG_SYSLOG);

#ifdef NDEBUG
void *
_oalMutexCreate(void *mutex)
{
   _oalMutex *m = (_oalMutex *)mutex;

   if (!m) {
      m = calloc(1, sizeof(_oalMutex));
      if (m) {
         pthread_mutex_init(&m->mutex, NULL);
      }
   }

   return m;
}

#else
void *
_oalMutexCreateDebug(void *mutex, const char *name, const char *fn)
{
   _oalMutex *m = (_oalMutex *)mutex;

   if (!m)
   {
      m = calloc(1, sizeof(_oalMutex));
      if (m) {
         pthread_mutex_init(&m->mutex, NULL);
         m->name = (char *)name;
         m->function = fn;
      }
   }

   return m;
}
#endif

_oalMutex *
_oalMutexCreateInt(_oalMutex *m)
{
   if (m && m->initialized == 0)
   {
      int status;
#if 1
      pthread_mutexattr_t mta;

      status = pthread_mutexattr_init(&mta);
      if (!status)
      {
#ifndef NDEBUG
         status = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
#else
         status = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_NORMAL);
#endif
         if (!status) {
            status = pthread_mutex_init(&m->mutex, &mta);
         }
      }
#else
      status = pthread_mutex_init(&m->mutex, NULL);
#endif

      if (!status) {
         m->initialized = 1;
      }
   }

   return m;
}

void
_oalMutexDestroy(void *mutex)
{
   _oalMutex *m = (_oalMutex *)mutex;

   if (m)
   {
      pthread_mutex_destroy(&m->mutex);
      free(m);
   }

   m = 0;
}

#ifdef NDEBUG
int
_oalMutexLock(void *mutex)
{
   _oalMutex *m = (_oalMutex *)mutex;
   int r = 0;

   if (m)
   {
      if (m->initialized == 0) {
         m = _oalMutexCreateInt(m);
      }

      if (m->initialized != 0)
      {
         r = pthread_mutex_lock(&m->mutex);
      }
   }
   return r;
}
#else

int
_oalMutexLockDebug(void *mutex, char *file, int line)
{
   _oalMutex *m = (_oalMutex *)mutex;
   int r = 0;

   if (m)
   {
      if (m->initialized == 0) {
         mutex = _oalMutexCreateInt(m);
      }

      if (m->initialized != 0)
      {
         struct timespec to;
#ifdef __GNUC__
         unsigned int mtx;
 
         mtx = m->mutex.__data.__count;
         
         if (mtx > 1) {
            printf("lock mutex > 1 (%i) in %s line %i, for: %s in %s\n",
                                         mtx, file, line, m->name, m->function);
            r = -mtx;
            abort();
         }
#endif

         to.tv_sec = time(NULL) + DEBUG_TIMEOUT;
         to.tv_nsec = 0;
         r = pthread_mutex_timedlock(&m->mutex, &to);
         // r = pthread_mutex_lock(&m->mutex);

         if (r == ETIMEDOUT) {
            printf("mutex timed out in %s line %i\n", file, line);
            abort();
         } else if (r == EDEADLK) {
            printf("dealock in %s line %i\n", file, line);
            abort();
         } else if (r) {
            printf("mutex lock error %i in %s line %i\n", r, file, line);
         }

#ifdef __GNUC__
         mtx = m->mutex.__data.__count;
         if (mtx != 1) {
            printf("lock mutex != 1 (%i) in %s line %i, for: %s in %s\n", mtx, file, line, m->name, m->function);
            r = -mtx;
            abort();
         }
#endif
      }
   }
   return r;
}
#endif

#ifdef NDEBUG
int
_oalMutexUnLock(void *mutex)
{
   _oalMutex *m = (_oalMutex *)mutex;
   int r = 0;

   if (m)
   {
      r = pthread_mutex_unlock(&m->mutex);
   }
   return r;
}

#else
int
_oalMutexUnLockDebug(void *mutex, char *file, int line)
{
   _oalMutex *m = (_oalMutex *)mutex;
   int r = 0;

   if (m)
   {
#ifdef __GNUC__
      unsigned int mtx;

      mtx = m->mutex.__data.__count;
      if (mtx != 1) {
         if (mtx == 0)
            printf("mutex already unlocked in %s line %i, for: %s\n",
                                              file, line, m->name);
         else
            printf("unlock mutex != 1 (%i) in %s line %i, for: %s in %s\n",
                                       mtx, file, line, m->name, m->function);
         r = -mtx;
         abort();
      }
#endif

      r = pthread_mutex_unlock(&m->mutex);
#ifndef NDEBUG
#endif
   }
   return r;
}
#endif


#elif defined( _WIN32 )	/* HAVE_PTHREAD_H */

#include <base/dlsym.h> 
							/* --- WINDOWS --- */
#define _TH_SYSLOG(a)

/*
 * In debugging mode we use real mutexes with a timeout value.
 * In release mode use critical sections which could be way faster
 *    for single process applications.
 */
#ifndef NDEBUG
void *
_oalMutexCreateDebug(void *mutex, const char *name, const char *fn)
{
   _oalMutex *m = (_oalMutex *)mutex;

   if (!m) {
      m = calloc(1, sizeof(_oalMutex));
   }

   if (m && !m->mutex) {
      m->mutex = CreateMutex(NULL, FALSE, NULL);
   }

   return m;
}
#else /* !NDEBUG */
void *
_oalMutexCreate(void *mutex)
{
   _oalMutex *m = (_oalMutex *)mutex;

   if (!m) {
      m = calloc(1, sizeof(_oalMutex));
   }

   if (m && !m->ready)
   {
      InitializeCriticalSection(&m->mutex);
      m->ready = 1;
   }

   return m;
}
#endif

void
_oalMutexDestroy(void *mutex)
{
   _oalMutex *m = (_oalMutex *)mutex;

   if (m)
   {
#ifndef NDEBUG
      CloseHandle(m->mutex);
#else
      DeleteCriticalSection(&m->mutex);
      m->ready = 0;
#endif
      free(m);
   }

   m = 0;
}

#ifndef NDEBUG
int
_oalMutexLockDebug(void *mutex, char *file, int line)
{
   _oalMutex *m = (_oalMutex *)mutex;
   int r = 0;

   if (m)
   {
      if (!m->mutex) {
         m = _oalMutexCreate(m);
      }

      if (m->mutex) {
         r = WaitForSingleObject(m->mutex, DEBUG_TIMEOUT*1000);
         switch (r)
         {
         case WAIT_OBJECT_0:
            break;
         case WAIT_TIMEOUT:
            printf("mutex timed out in %s line %i\n", file, line);
            abort();
            r = ETIMEDOUT;
            break;
         case WAIT_ABANDONED:
         case WAIT_FAILED:
         default:
            printf("mutex lock error %i in %s line %i\n", r, file, line);
            abort();
         }
      }
   }
   return r;
}

int
_oalMutexUnLockDebug(void *mutex, char *file, int line)
{
   _oalMutex *m = (_oalMutex *)mutex;
   int r = EINVAL;

   if (m)
   {
      ReleaseMutex(m->mutex);
      r = 0;
   }
   return r;
}
#else	/* !NDEBUG */
int
_oalMutexLock(void *mutex)
{
   _oalMutex *m = (_oalMutex *)mutex;
   int r = 0;

   if (m)
   {
      if (!m->ready) {
         m = _oalMutexCreate(m);
      }

      if (m->ready) {
         EnterCriticalSection(&m->mutex);
      }
   }
   return r;
}

int
_oalMutexUnLock(void *mutex)
{
   _oalMutex *m = (_oalMutex *)mutex;
   int r = 0;

   if (m) {
      LeaveCriticalSection(&m->mutex);
   }
   return r;
}
#endif

void *
_oalConditionCreate()
{
   void *p = CreateEvent(NULL, FALSE, FALSE, NULL);
   return p;
}

void
_oalConditionDestroy(void *c)
{
   assert(c);

   CloseHandle(c);
   c = 0;
}

#else
# error "threads not implemented for this platform"
#endif /* HAVE_PTHREAD_H */

