/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * This file is in the Public Domain and comes with no warranty.
 * Erik Hofman <erik@ehofman.com>
 *
 */
#ifndef __DRIVER_H_
#define __DRIVER_H_

#include <base/types.h>
#include <base/logging.h>

// #undef NDEBUG
// #ifdef NDEBUG
# include <malloc.h>
// #else
// # include <rmalloc.h>
// #endif

char *getDeviceName(int, char **);
char *getCommandLineOption(int, char **, char *);
char *getInputFile(int, char **, const char *);
int getNumSources(int, char **);
float getPitch(int, char **);

char *getRenderer(int, char **);

void testForState(int, const char *);
void testForError(void *, char *);
void testForALCError(void *);
void testForALError();

char *strDup(const char *);

#endif

