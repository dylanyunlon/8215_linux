CC=arm-buildroot-linux-gnueabi-g++

TARGET := arm

CPPFLAGS += -std=c++11

all: main.c
	$(CC) $(CFLAGS) -Wall  -Wint-conversion $^ -o cluster

clean:
	rm -f *.o cluster
