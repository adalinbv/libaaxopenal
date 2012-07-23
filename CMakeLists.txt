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
SET(PACKAGE_NAME "AeonWave-OpenAL")
SET(PACKAGE "libaaxopenal")

OPTION(DLOPEN "Check for the dlopen API for loading optional libs" ON)
OPTION(UTILS  "Build and install utility programs" ON)
OPTION(WERROR "Treat compile warnings as errors"   OFF)

IF(WIN32)
    SET(LIBOPENAL OpenAL32)
    ADD_DEFINITIONS("-D_WIN32")
ELSE(WIN32)
    SET(LIBOPENAL openal)
    ADD_DEFINITIONS(-D_REENTRANT -D_XOPEN_SOURCE=600 -D_POSIX_C_SOURCE=199309L)
ENDIF(WIN32)

IF(NOT LIBTYPE)
    SET(LIBTYPE SHARED)
ENDIF(NOT LIBTYPE)

# read 'version' file into a variable (stripping any newlines or spaces)
FILE(READ version versionFile)
STRING(STRIP ${versionFile} AL_VERSION)
SET(VERSION AL_VERSION)

# split version string into components, note CMAKE_MATCH_0 is the entire regexp match
STRING(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" CPACK_PACKAGE_VERSION ${AL_VERSION})


SET(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1})
SET(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2})
SET(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_MATCH_3})

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenAL layer for the AeonWave 4D audio library")

# We have some custom .cmake scripts not in the official distribution.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

INCLUDE(CPack_Modules)
INCLUDE(CPack_Defaults)

# Used to create Start Menu shortcuts <EXEC> <LABEL>
#set(CPACK_PACKAGE_EXECUTABLES "alcinfo" "OpenAL information")

IF(WIN32)
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_INSTALL_PREFIX}")

ELSE(WIN32)

# DEBIAN
  SET(CPACK_DEBIAN_PACKAGE_SECTION "libs")
  SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.1), libaax2-runtime (>= 2.2)")
  SET(CPACK_DEBIAN_PACKAGE_ENHANCES "libopenal1")
  SET(CPACK_DEBIAN_PACKAGE_REPLACES "libopenal1, libopenal1-dev, libopenal, libopenal-dev, libaaxopenal")
#  SET(CPACK_DEBIAN_PACKAGE_CONFLICTS "libopenal1")
#  SET(CPACK_DEBIAN_PACKAGE_PROVIDES "libopenal1, libopenal-dev")

# RPM
  SET(CPACK_RPM_PACKAGE_LICENSE "LGPL v3")
  SET(CPACK_RPM_PACKAGE_GROUP "System/Libraries")
  SET(CPACK_RPM_PACKAGE_REQUIRES "libaax2-runtime >= 2.2")
  SET(CPACK_RPM_PACKAGE_PROVIDES "libopenal1, libopenal-dev")
  SET(CPACK_RPM_PACKAGE_CONFLICTS "libopenal1, libopenal-dev")
  SET(CPACK_RPM_PACKAGE_OBSOLETES "libopenal1, libopenal-dev, libopenal-data")

ENDIF(WIN32)

# ZIP
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${AL_VERSION}" CACHE INTERNAL "tarball basename")


INCLUDE (CPack)

# Required libraries
FIND_PACKAGE(AAX COMPONENTS aax REQUIRED)

# Add definitions, compiler switches, etc.
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}"
    include
    base
    ${AAX_INCLUDE_DIR}
    "${CMAKE_BINARY_DIR}/include"
)

ADD_DEFINITIONS(-DHAVE_CONFIG_H=1)
IF(WERROR)
  ADD_DEFINITIONS(-Werror)
ENDIF(WERROR)

IF(MSVC)
    SET(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -D_DEBUG /LD")
    SET(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -DNDEBUG /LD")
    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG /LD")
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG /LD")
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE)

    OPTION(FORCE_STATIC_VCRT "Force /MT for static VC runtimes" ON)
    IF(FORCE_STATIC_VCRT)
        FOREACH(flag_var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
            IF(${flag_var} MATCHES "/MD")
                STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
            ENDIF()
        ENDFOREACH(flag_var)
    ENDIF()
ELSE(MSVC)
    SET(C_FLAGS "${C_FLAGS} -Os -march=i586 -Werror=format-security -Wall -fomit-frame-pointer" CACHE STRING
        "Flags used by al build types."
        FORCE)
    SET(CMAKE_C_FLAGS_RELWITHDEBINFO "${C_FLAGS} -g -O2 -D_DEBUG" CACHE STRING
        "Flags used by the compiler during Release with Debug Info builds."
        FORCE)
    SET(CMAKE_C_FLAGS_MINSIZEREL "${C_FLAGS} -Os -DNDEBUG" CACHE STRING
        "Flags used by the compiler during release minsize builds."
        FORCE)
    SET(CMAKE_C_FLAGS_RELEASE "${C_FLAGS} -Os -fomit-frame-pointer -DNDEBUG"
        CACHE STRING "Flags used by the compiler during release builds"
        FORCE)
    SET(CMAKE_C_FLAGS_DEBUG "${C_FLAGS} -g3 -D_DEBUG" CACHE STRING
        "Flags used by the compiler during debug builds."
        FORCE)
ENDIF(MSVC)


# Check for the dlopen API (for alGetProcAddress)
CHECK_INCLUDE_FILE(time.h HAVE_TIME_H)
CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE(sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(strings.h HAVE_STRINGS_H)
CHECK_INCLUDE_FILE(unistd.h HAVE_UNISTD_H)
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
SET_TARGET_PROPERTIES(${LIBOPENAL}
     PROPERTIES DEFINE_SYMBOL ALC_BUILD_LIBRARY
     VERSION ${LIB_VERSION}
     SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}
)

IF(WIN32 AND NOT LIBTYPE STREQUAL "STATIC")
    SET_TARGET_PROPERTIES(${LIBOPENAL} PROPERTIES PREFIX "")
ENDIF(WIN32 AND NOT LIBTYPE STREQUAL "STATIC")

TARGET_LINK_LIBRARIES(${LIBOPENAL} ${EXTRA_LIBS} ${AAX_LIBRARY})

# Add an install target here
# Note Windows DLL are specified by RUNTIME
INSTALL(TARGETS ${LIBOPENAL}
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT Libraries
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT Libraries
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT Headers
)
INSTALL(FILES
        include/AL/al.h
        include/AL/alc.h
        include/AL/alext.h
        include/AL/alcext.h
        DESTINATION include/AL
        COMPONENT Headers
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
