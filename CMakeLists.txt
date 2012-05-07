cmake_minimum_required(VERSION 2.8.3)
SET(cmake_verbose_makefile TRUE)

SET(CMAKE_MAKEFILE_DEPENDS
    "CMakeModules/Symlink.cmake"
)

IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)


INCLUDE(CheckLibraryExists)
INCLUDE(CheckIncludeFile)
INCLUDE (CheckIncludeFiles)

PROJECT(aaxopenal C)

OPTION(DLOPEN "Check for the dlopen API for loading optional libs" ON)
OPTION(UTILS  "Build and install utility programs" ON)
OPTION(WERROR "Treat compile warnings as errors"   OFF)

IF(WIN32)
    SET(LIBOPENAL OpenAL32)
    ADD_DEFINITIONS("-D_WIN32")
ELSE(WIN32)
    SET(LIBOPENAL openal)
ENDIF(WIN32)

IF(NOT LIBTYPE)
    SET(LIBTYPE SHARED)
ENDIF(NOT LIBTYPE)

# read 'version' file into a variable (stripping any newlines or spaces)
file(READ version versionFile)
STRING(STRIP ${versionFile} AAXOPENAL_VERSION)
SET(VERSION AAXOPENAL_VERSION)

# detect system type
IF(NOT DEFINED CPACK_SYSTEM_NAME)
  SET(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
ENDIF(NOT DEFINED CPACK_SYSTEM_NAME)
IF (UNIX AND NOT WIN32)
  IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(CPACK_PACKAGE_ARCHITECTURE "x86_64")
  ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(CPACK_PACKAGE_ARCHITECTURE "i386")
  ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)

  If(NOT DEFINED CMAKE_LIBRARY_ARCHITECTURE)
     SET(CMAKE_LIBRARY_ARCHITECTURE "${CPACK_PACKAGE_ARCHITECTURE}-linux-gnu")
  ENDIF(NOT DEFINED CMAKE_LIBRARY_ARCHITECTURE)
ENDIF(UNIX AND NOT WIN32)

SET(CMAKE_INSTALL_LIBDIR lib/${CMAKE_LIBRARY_ARCHITECTURE} CACHE PATH "Output directory for libraries")

#  read 'description` file into a variable
file(STRINGS description descriptionFile)
STRING(REGEX REPLACE ";" ".\n " descriptionFile "${descriptionFile}")

# split version string into components, note CMAKE_MATCH_0 is the entire regexp match
STRING(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" CPACK_PACKAGE_VERSION ${AAXOPENAL_VERSION})

SET(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
SET(CPACK_PACKAGE_VENDOR "Adalin B.V.")
SET(CPACK_PACKAGE_CONTACT "tech@adalin.org")
SET(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1})
SET(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2})
SET(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_MATCH_3})
SET(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.el6.${CPACK_PACKAGE_ARCHITECTURE}")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenAL layer for the AeonWave 4D audio library")
#SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${descriptionFile})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
#SET(CPACK_MONOLITHIC_INSTALL 1)# ignore CMAKE install components

SET(CPACK_COMPONENTS_ALL applications libraries headers)
SET(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "alcinfo")
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "OpenAL context information utility")
SET(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Library")
SET(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "OpenAL library")
SET(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Headers files")
SET(CPACK_COMPONENT_HEADERS_DESCRIPTION "Development header files")
SET(CPACK_COMPONENT_HEADERS_DEPENDS libraries)
SET(CPACK_COMPONENT_APPLICATIONS_GROUP "Runtime")
SET(CPACK_COMPONENT_LIBRARIES_GROUP "Runtime")
SET(CPACK_COMPONENT_HEADERS_GROUP "Development")

SET(CPACK_ALL_INSTALL_TYPES Runtime Full)
SET(CPACK_COMPONENT_LIBRARIES_INSTALL_TYPES Runtime Full)
SET(CPACK_COMPONENT_APPLICATIONS_INSTALL_TYPES Runtime Full)
SET(CPACK_COMPONENT_HEADERS_INSTALL_TYPES Full)

# Used to create Start Menu shortcuts <EXEC> <LABEL>
#set(CPACK_PACKAGE_EXECUTABLES "alcinfo" "OpenAL information")

# DEBIAN
IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
ELSE(CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
ENDIF(CMAKE_SIZEOF_VOID_P EQUAL 8)
SET(CPACK_DEBIAN_PACKAGE_SECTION "libs")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${descriptionFile})
SET(CPACK_DEBIAN_PACKAGE_PREDEPENDS "multiarch-support")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1), libaax2 (>= 2.2)")
SET(CPACK_DEBIAN_PACKAGE_ENHANCES "libopenal1")
SET(CPACK_DEBIAN_PACKAGE_REPLACES "libopenal1, libopenal1-dev")
#SET(CPACK_DEBIAN_PACKAGE_CONFLICTS "libopenal1")
#SET(CPACK_DEBIAN_PACKAGE_PROVIDES "libopenal1, libopenal-dev")

# RPM
SET(CPACK_RPM_PACKAGE_ARCHITECTURE ${PACK_PACKAGE_ARCHITECTURE})
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/description")
SET(CPACK_RPM_CHANGELOG_FILE "${CMAKE_SOURCE_DIR}/ChangeLog")
SET(CPACK_RPM_PACKAGE_GROUP "System/Libraries")
SET(CPACK_RPM_PACKAGE_REQUIRES "libaax2 >= 2.2")
SET(CPACK_RPM_PACKAGE_PROVIDES "libopenal1, libopenal-dev")
SET(CPACK_RPM_PACKAGE_CONFLICTS "libopenal1, libopenal-dev")
SET(CPACK_RPM_PACKAGE_OBSOLETES "libopenal1, libopenal-dev, libopenal-data")


SET(CPACK_SOURCE_GENERATOR ZIP)
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${AAXOPENAL_VERSION}" CACHE INTERNAL "tarball basename")
SET(CPACK_SOURCE_IGNORE_FILES
  "^${PROJECT_BINARY_DIR};${CMAKE_SOURCE_DIR}/.git;\\\\.gitignore;Makefile.am;~$;${CPACK_SOURCE_IGNORE_FILES}")

SET(CPACK_GENERATOR "DEB;RPM")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Adalin B.V.")

INCLUDE (CPack)

# We have some custom .cmake scripts not in the official distribution.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

# Required libraries
FIND_PACKAGE(AAX COMPONENTS aax REQUIRED)

# Add definitions, compiler switches, etc.
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}" include base "${CMAKE_BINARY_DIR}/include")

ADD_DEFINITIONS(-Wall -DHAVE_CONFIG_H=1)
IF(WERROR)
  ADD_DEFINITIONS(-Werror)
ENDIF(WERROR)

SET(C_FLAGS "${C_FLAGS} -march=i386 -Wall -Werror=format-security" CACHE STRING
    "Flags used by al build types."
    FORCE)
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "${C_FLAGS} -g -O2 -D_DEBUG" CACHE STRING
    "Flags used by the compiler during Release with Debug Info builds."
    FORCE)
SET(CMAKE_C_FLAGS_MINSIZEREL "${C_FLAGS} -Os -DNDEBUG" CACHE STRING
    "Flags used by the compiler during release minsize builds."
    FORCE)
SET(CMAKE_C_FLAGS_RELEASE "${C_FLAGS} -Os -fomit-frame-pointer -s -DNDEBUG"
    CACHE STRING "Flags used by the compiler during release builds"
    FORCE)
SET(CMAKE_C_FLAGS_DEBUG "${C_FLAGS} -g3 -D_DEBUG" CACHE STRING
    "Flags used by the compiler during debug builds."
    FORCE)

# Check for the dlopen API (for alGetProcAddress)
CHECK_INCLUDE_FILE(syslog.h HAVE_SYSLOG_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILE(assert.h HAVE_ASSERT_H)
CHECK_INCLUDE_FILE(dlfcn.h HAVE_DLFCN_H)
IF(HAVE_DLFCN_H)
  CHECK_LIBRARY_EXISTS(dl dlopen "" HAVE_LIBDL)
  IF(HAVE_LIBDL)
    SET(EXTRA_LIBS dl ${EXTRA_LIBS})
  ENDIF(HAVE_LIBDL)
ENDIF(HAVE_DLFCN_H)

CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/config.h")


SET(BASE_OBJS
     base/buffers.c
     base/dlsym.c
     base/logging.c
     base/types.c
   )

SET( OPENAL_OBJS
     src/alContext.c
     src/alCapture.c
     src/alSource.c
     src/alBuffer.c
     src/alListener.c
     src/alState.c
     src/aax_support.c
     src/api.c
   )

# Build a library
SET(LIB_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
ADD_LIBRARY(${LIBOPENAL} ${LIBTYPE} ${BASE_OBJS} ${OPENAL_OBJS})
SET_TARGET_PROPERTIES(${LIBOPENAL} PROPERTIES DEFINE_SYMBOL AL_BUILD_LIBRARY
                                       COMPILE_FLAGS -DAL_ALEXT_PROTOTYPES
                                       VERSION ${LIB_VERSION}
                                       SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR})
IF(WIN32 AND NOT LIBTYPE STREQUAL "STATIC")
    SET_TARGET_PROPERTIES(${LIBOPENAL} PROPERTIES PREFIX "")
ENDIF(WIN32 AND NOT LIBTYPE STREQUAL "STATIC")

TARGET_LINK_LIBRARIES(${LIBOPENAL} ${EXTRA_LIBS} ${AAX_LIBRARY})

# Add an install target here
INSTALL(TARGETS ${LIBOPENAL}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
INSTALL(FILES include/AL/al.h
              include/AL/alc.h
              include/AL/alext.h
              include/AL/alcext.h
        DESTINATION include/AL
)

IF (UNIX AND NOT WIN32)
    macro(install_symlink _dest _source)
      install(CODE "
             include(\"${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/Symlink.cmake\")
             InstallSymlink(\"${_dest}\" \"\$ENV{DESTDIR}${_source}\")")
    endmacro(install_symlink)

    install_symlink(${CMAKE_LIBRARY_ARCHITECTURE}/lib${LIBOPENAL}.so.${CPACK_PACKAGE_VERSION_MAJOR} ${CMAKE_INSTALL_PREFIX}/lib/lib${LIBOPENAL}.so.${CPACK_PACKAGE_VERSION_MAJOR})
    install_symlink(${CMAKE_LIBRARY_ARCHITECTURE}/lib${LIBOPENAL}.so ${CMAKE_INSTALL_PREFIX}/lib/lib${LIBOPENAL}.so)
ENDIF(UNIX AND NOT WIN32)

IF(UTILS)
  ADD_SUBDIRECTORY(test)
ENDIF(UTILS)

#-----------------------------------------------------------------------------
### uninstall target
#-----------------------------------------------------------------------------
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
