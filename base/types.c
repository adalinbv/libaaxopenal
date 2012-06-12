/*
 * Copyright (C) 2005-2012 by Erik Hofman.
 * Copyright (C) 2007-2012 by Adalin B.V.
 *
 * This file is part of OpenAL-AeonWave.
 *
 *  OpenAL-AeonWave is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  OpenAL-AeonWave is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with OpenAL-AeonWave.  If not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_TIME_H
# include <time.h>             /* for nanosleep */
#endif
#include "types.h"


#ifndef _WIN32
int msecSleep(unsigned int dt_ms)
{
   static struct timespec s;
   s.tv_sec = (dt_ms/1000);
   s.tv_nsec = (dt_ms-s.tv_sec*1000)*1000000;
   return nanosleep(&s, 0);
}
#endif

