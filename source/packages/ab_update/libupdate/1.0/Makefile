
CC=arm-buildroot-linux-gnueabi-gcc

LDFLAGS= -lpthread -shared -fPIC
DEFINES=
INCLUDE=-I./include/	\
		-I./../update-1.0/include/

CFLAGS= -std=c++1z -Wall -Wextra -O2 -g -fPIC
SRCS=./src/ATCUpdate.cpp \
	./src/ATCUpdateClient.cpp \
	./src/SocketClient.cpp \
	../update-1.0/src/ipc/socket/SocketProto.cpp \

OBJS=$(SRCS:.cpp=.o)
TARGET_LIB=libupdate.so

all: $(TARGET_LIB)

$(TARGET_LIB): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -L$(TARGET_DIR)/usr/lib

%.o: %.cpp
	$(CC)  $(CFLAGS) -Wall -o $@ -c $< $(INCLUDE)

clean:
	rm -f *.o
