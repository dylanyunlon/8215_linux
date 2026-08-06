cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++98")
SET(CMAKE_C_COMPILER arm-buildroot-linux-gnueabi-gcc)
SET( CMAKE_CXX_COMPILER arm-buildroot-linux-gnueabi-g++ ) 
SET( PROJDIR ${CMAKE_CURRENT_SOURCE_DIR} )
SET (CMAKE_SYSROOT "${PROJDIR}/../../out/sysroot/")

OPTION(NAND_PROJ "Build project1 for NAND" OFF)
IF(NAND_PROJ)
    MESSAGE("Adding NAND_PROJ macro definition...")
    ADD_DEFINITIONS(-DNAND_PROJ)
ENDIF(NAND_PROJ)


include_directories(
 ./
 ./include
 ../libsparse/include/
 ../mtdutils/
 ../ext4_utils/
 ../../../../../atc_linux/kernel/kernel-3.18/tools/include/linux/
 ../minizip-ng-develop/
)

  
link_directories(
   ../mtdutils/build/
   ../ext4_utils/build/
   ../minizip-ng-develop/build/
)
 
target_link_libraries(
  libminizip
   )

FILE(GLOB CPLUSSOURCE "./*.cpp")
add_library(recovery SHARED ${CPLUSSOURCE})
install(TARGETS recovery DESTINATION "${PROJDIR}/../../out/sysroot/lib/" )

