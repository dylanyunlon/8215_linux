
##
CUSTPROJ=CUSTOMER_ATC

all: RECOVERY_LIB MTD_LIB EXT4_LIB LIBSPARSE ZLIB  MINIZIP_LIB
	@echo "Make all lib"

# Add this new target for minizip
MINIZIP_LIB:
	cd $(recovery_top)/lib/minizip-ng-develop; make clean; make; make install
	@echo "Make minizip-ng lib"

ifeq ($(CUSTPROJ), NAND_PROJ)
RECOVERY_LIB:  MTD_LIB EXT4_LIB LIBSPARSE ZLIB MINIZIP_LIB
	cd $(recovery_top)/lib/recovery;rm -rf build ;mkdir build; cd build; cmake -DNAND_PROJ=ON .. -DCMAKE_C_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)" -DCMAKE_CXX_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)"; make; make install

MTD_LIB: EXT4_LIB LIBSPARSE ZLIB
	cd $(recovery_top)/lib/mtdutils;rm -rf build ;mkdir build; cd build; cmake .. -DCMAKE_C_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)" -DCMAKE_CXX_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)"; make; make install

EXT4_LIB: LIBSPARSE ZLIB
	cd $(recovery_top)/lib/ext4_utils;rm -rf build ;mkdir build; cd build; cmake .. -DCMAKE_C_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)" -DCMAKE_CXX_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)"; make; make install

LIBSPARSE: ZLIB
	cd $(recovery_top)/lib/libsparse;rm -rf build ;mkdir build; cd build; cmake .. -DCMAKE_C_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)" -DCMAKE_CXX_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)"; make; make install

ZLIB:
	cd $(recovery_top)/lib/zlib-1.2.3;rm -rf build ;mkdir build; cd build; cmake .. -DCMAKE_C_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)" -DCMAKE_CXX_COMPILER_ARG1:STRING="-march=armv7-a -mthumb-interwork -mfpu=neon --sysroot=$(recovery_sysroot)"; make; make install

else
RECOVERY_LIB:  MTD_LIB EXT4_LIB LIBSPARSE ZLIB
	cd $(recovery_top)/lib/recovery;rm -rf build ;mkdir build; cd build; cmake ..; make; make install

MTD_LIB: EXT4_LIB LIBSPARSE ZLIB
	cd $(recovery_top)/lib/mtdutils;rm -rf build ;mkdir build; cd build; cmake ..; make; make install

EXT4_LIB: LIBSPARSE ZLIB
	cd $(recovery_top)/lib/ext4_utils;rm -rf build ;mkdir build; cd build; cmake ..; make; make install

LIBSPARSE: ZLIB
	cd $(recovery_top)/lib/libsparse;rm -rf build ;mkdir build; cd build; cmake ..; make; make install

ZLIB:
	cd $(recovery_top)/lib/zlib-1.2.3;rm -rf build ;mkdir build; cd build; cmake ..; make; make install
endif


