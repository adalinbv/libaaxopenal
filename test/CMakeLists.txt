
SET( DRIVER_OBJS
     driver.c
     wavfile.c
   )

SET(LIBDRIVER driver)
SET(LIBTYPE STATIC)
ADD_LIBRARY(${LIBDRIVER} ${LIBTYPE} ${DRIVER_OBJS})

FUNCTION(CREATE_TEST TEST_NAME)
    ADD_EXECUTABLE(${TEST_NAME} ${TEST_NAME}.c)
    TARGET_LINK_LIBRARIES(${TEST_NAME} ${LIBOPENAL} ${LIBDRIVER})
    SET_TARGET_PROPERTIES(${TEST_NAME} PROPERTIES
            COMPILE_DEFINITIONS  "SRC_PATH=\"${PROJECT_SOURCE_DIR}/../sounds\"")
ENDFUNCTION()


CREATE_TEST(alcinfo)
INSTALL(TARGETS alcinfo
          RUNTIME DESTINATION bin
       )

CREATE_TEST(testcapture)
CREATE_TEST(testloopback)
CREATE_TEST(testcone)
CREATE_TEST(testdistance)
CREATE_TEST(testerrors)
CREATE_TEST(testleftright)
CREATE_TEST(testlistener3d)
CREATE_TEST(testlooping)
CREATE_TEST(testmixer)
CREATE_TEST(testmono3d)
CREATE_TEST(testmono3d_lowpass)
CREATE_TEST(testmono3d_multi)
CREATE_TEST(testmono3d_reverb)
CREATE_TEST(testmulticontext)
CREATE_TEST(testpitchvolume)
CREATE_TEST(testqueue)
CREATE_TEST(testsource)
CREATE_TEST(teststereo)
CREATE_TEST(teststereo_highpass)
CREATE_TEST(teststereo_lowpass)
CREATE_TEST(teststereo_reverb)
CREATE_TEST(teststream)
CREATE_TEST(teststrings)
CREATE_TEST(testupdown)

