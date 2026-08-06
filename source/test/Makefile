
PWD:=$(shell pwd)

export UNITTEST_DIR = $(PWD)
export GTEST_DIR = $(PWD)/gtest-1.7.0
export GTEST_ATC_DIR = $(PWD)/atc

SUBDIR= atc gtest-1.7.0/make

.phony: misc gtest

all:
	$(info ************begin*********************)
	make gtest
	make misc
	$(info *************end********************)

misc:
	make -C atc

gtest:
	make -C gtest-1.7.0/make

clean:
	for i in ${SUBDIR}; do make clean -C "$${i}"; done

