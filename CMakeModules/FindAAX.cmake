# Locate AAX
# This module defines
# AAX_LIBRARIES
# AAX_FOUND, if false, do not try to link to AAX 
# AAX_INCLUDE_DIR, where to find the headers
#
# $AAXDIR is an environment variable that would
# correspond to the ./configure --prefix=$AAXDIR
# used in building AAX.
#
# Created by Erik Hofman.

FIND_PATH(AAX_INCLUDE_DIR aax.h
  HINTS
  $ENV{AAXDIR}
  $ENV{ProgramFiles}/aax
  $ENV{ProgramFiles}/AeonWave
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /opt
)

FIND_LIBRARY(AAX_LIBRARY 
  NAMES AAX aax AAX32
  HINTS
  $ENV{AAXDIR}
  $ENV{ProgramFiles}/AAX
  $ENV{ProgramFiles}/AeonWave
  PATH_SUFFIXES lib lib/${CMAKE_LIBRARY_ARCHITECTURE} lib64 libs64 libs libs/Win32 libs/Win64
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /opt
)

IF(AAX_LIBRARY AND AAX_INCLUDE_DIR)
  SET(AAX_FOUND "YES")
ELSE(AAX_LIBRARY AND AAX_INCLUDE_DIR)
  SET(AAX_FOUND "NO")
  IF(NOT AAX_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "Unable to find the AeonWave Audio eXtensions library development files.")
  ENDIF(NOT AAX_INCLUDE_DIR)
  IF(NOT AAX_LIBRARY)
    MESSAGE(FATAL_ERROR "Unable to find the AeonWave Audio eXtensions library binary files.")
  ENDIF(AAX_LIBRARY)
ENDIF(AAX_LIBRARY AND AAX_INCLUDE_DIR)

