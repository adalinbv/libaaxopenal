/*
 * Copyright (C) 2007-2011 by Erik Hofman.
 * Copyright (C) 2007-2011 by Adalin B.V.
 *
 * This file is part of AeonWave-OpenAL.
 *
 *  AeonWave-OpenAL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  AeonWave-OpenAL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AeonWave-OpenAL.  If not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef NDEBUG
#include "api.h"

const char *_al_id_s[_OAL_MAX_ID] =
{
    "NONE",
    "_OAL_BACKEND",
    "_OAL_EXTENSION",
    "_OAL_ENUM",
    "_OAL_DEVICE",
    "_OAL_CONTEXT",
    "_OAL_STATE",
    "_OAL_BUFFER",
    "_OAL_SOURCE",
    "_OAL_LISTENER",
    "_OAL_SBUFFER"
};
#endif

