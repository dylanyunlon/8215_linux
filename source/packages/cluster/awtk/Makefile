SHELL := /bin/bash

TARGET := arm

all: build release

build:
	@if [ ! -d awtk ]; then \
		echo "copy awtk lib directory"; \
		mkdir -p awtk; \
	fi
	@rsync -az --delete $(TOPDIR)/../bsp/arm2/lib/libawtk/src/awtk-1.8/ awtk/ || (echo "sync awtk fail" && exit 1); \
	
	@if [ ! -d ridedash ]; then \
		echo "copy app directory"; \
		mkdir -p ridedash; \
	fi
	@rsync -az --delete $(TOPDIR)/../bsp/arm2/app/awtk/ridedash/ ridedash/ || (echo "sync ridedash fail" && exit 1); \
	
	cd awtk-linux-fb/ && scons APP=../ridedash
	
release: build
	cd awtk-linux-fb/ && sh ./release.sh ../ridedash/res demo
	
.PHONY : clean
clean:
	scons -c
	rm awtk ridedash
