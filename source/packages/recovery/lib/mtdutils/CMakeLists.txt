cmake_minimum_required(VERSION 2.8)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++0x")
SET(CMAKE_C_COMPILER arm-buildroot-linux-gnueabi-gcc)
SET( CMAKE_CXX_COMPILER arm-buildroot-linux-gnueabi-g++ ) 
SET( PROJDIR ${CMAKE_CURRENT_SOURCE_DIR} )
SET (CMAKE_SYSROOT "${PROJDIR}/../../out/sysroot/")

include_directories(
./
)

FILE(GLOB SOURCES "./*.c")
add_library( mtdutils SHARED ${SOURCES})
install(TARGETS mtdutils DESTINATION "${PROJDIR}/../../out/sysroot/lib/" )


