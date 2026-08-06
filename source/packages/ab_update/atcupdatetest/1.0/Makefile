CC=arm-buildroot-linux-gnueabi-gcc

LDFLAGS= -lpthread
DEFINES=
INCLUDE=-I./../../libupdate/1.0/include/

CFLAGS= -std=c++1z -Wall -Wextra -O2 -g -fPIC
SRCS= ./ATCUpdateTest.cpp

OBJS=$(SRCS:.cpp=.o)
TARGET=atcupdatetest.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -lstdc++ -lupdate -L$(TARGET_DIR)/usr/lib

%.o: %.cpp
	$(CC)  $(CFLAGS) -Wall -o $@ -c $< $(INCLUDE)

clean:
	rm -f *.o
