cmake_minimum_required(VERSION 2.8)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++0x")
SET(CMAKE_C_COMPILER arm-buildroot-linux-gnueabi-gcc)
SET( CMAKE_CXX_COMPILER arm-buildroot-linux-gnueabi-g++ ) 
SET( PROJDIR ${CMAKE_CURRENT_SOURCE_DIR} )
SET (CMAKE_SYSROOT "${PROJDIR}/../../out/sysroot/")

include_directories(
  ./
 ../libsparse/include/
 ../mtdutils/
 ../ext4_utils/
 ../../../../kernel/kernel-3.18/tools/include/linux/
 ../../os/glibc/image/usr/include
 
)

  
link_directories(
   ../mtdutils/build/
   ../ext4_utils/build/
)
 

FILE(GLOB CPLUSSOURCE "./*.c")
add_library(z SHARED ${CPLUSSOURCE})
install(TARGETS z DESTINATION "${PROJDIR}/../../out/sysroot/lib/" )

