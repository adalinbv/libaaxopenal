/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * This file is in the Public Domain and comes with no warranty.
 * Erik Hofman <erik@ehofman.com>
 *
 */
#ifndef __WAVEFILE_H
#define __WAVEFILE_H

#include <aax/aax.h>

#define _OPENAL_SUPPORT		1

aaxBuffer bufferFromFile(aaxConfig, const char *);
void *fileLoad(const char *, unsigned int *, int *, char *, char *, unsigned int *);
#define fileWrite(a, b, c, d, e, f) \
        aaxWritePCMToFile(a, b, c, d, e, f)

void *fileDataConvertToInterleaved(void *, char, char, unsigned int);
enum aaxFormat getFormatFromFileFormat(unsigned int, int);

#endif
