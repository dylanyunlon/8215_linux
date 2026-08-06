cmake_minimum_required(VERSION 2.8)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++0x")
SET(CMAKE_C_COMPILER arm-buildroot-linux-gnueabi-gcc)
SET( CMAKE_CXX_COMPILER arm-buildroot-linux-gnueabi-g++ ) 
SET( PROJDIR ${CMAKE_CURRENT_SOURCE_DIR} )
SET (ZLIB  libz.so )
SET (CMAKE_SYSROOT "${PROJDIR}/../../out/sysroot/")
include_directories(
 ./
 ./include/
 ../mtdutils/
 ../ext4_utils/
 ../zlib-1.2.3/
)

  
link_directories(
   ${PROJDIR}/../../out/sysroot/lib/    
)
 

FILE(GLOB CPLUSSOURCE "./*.c")
add_library(sparse SHARED ${CPLUSSOURCE})
install(TARGETS sparse DESTINATION "${PROJDIR}/../../out/sysroot/lib/" )

