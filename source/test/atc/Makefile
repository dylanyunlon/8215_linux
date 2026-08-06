#!/bin/sh

PWD:=$(shell pwd)

SUBDIR= integrationtest/gtest/demo
SUBDIR+= unittest/gtest/directrender

.phony: misc

all:
	make misc
	make tar

misc:
	for i in ${SUBDIR}; do make -C "$${i}"; done

tar:
	tar -cf test_suite.tar integrationtest/bin integrationtest/test_config integrationtest/test_scripts unittest/bin unittest/test_config unittest/test_scripts upgrade/test_config upgrade/test_scripts

clean:
	for i in ${SUBDIR}; do make clean -C "$${i}"; done

