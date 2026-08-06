TARGET := arm
export SYSROOT_DIR
CC=arm-buildroot-linux-gnueabi-g++

LOCAL_SRC_FILES := \
        ./thread/cautothread.cpp \
        ./filelock/cclient.cpp \
        ./filelock/cfilelock.cpp \
        ./filelock/cserver.cpp \
        ./conditionlock/cconditionlock.cpp \
        ./localsocket/clocalsocket.cpp \
        ./messages/cmessage.cpp \
        ./messages/cmessagehandler.cpp \
        ./serversocket/cserversocket.cpp \
        ./stringutils/crawstring.cpp \
        ./stringutils/stringutils.cpp \
        ./statemachine/cstate.cpp \
        ./statemachine/cstatemachine.cpp \
        ./log/clog.cpp \
        ./ringutils/ringutils.cpp \
        ./uuid/uuidutils.cpp \
        #./sharememory/sharememory.inl \
        ./synchronizedtaskexecution/synchronizedtaskexecution.cpp \

LOCAL_INCLUDES :=  -I./include \
        -I./sharememory \

        #        -I./property \

LOCAL_INCLUDES += -I$(SYSROOT_DIR)/usr/include/libxml2

LOCAL_INSTALL_FILES += $(wildcard ./include/*.h)
LOCAL_INSTALL_FILES += $(wildcard ./sharememory/*.inl)

LOCAL_LIB_PATH := $(TARGET_TOP)/lib

#CFLAGS += -fPIC -Wall -Wno-write-strings
CFLAGS += -shared $(LOCAL_INCLUDES)
#CFLAGS += -std=c++1y
#LDFLAGS= -std=c++14 -g
CPPFLAGS += -std=c++11

LOCAL_SHARE_LIBRARIES += -lpthread 
LOCAL_SHARE_LIBRARIES += -lxml2

LOCAL_MODULE := libuniversal_utils.so

#include $(BUILD_SHARED_LIBRARY)

.PHONY: all
all: $(LOCAL_MODULE)

$(LOCAL_MODULE): $(LOCAL_SRC_FILES)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -lstdc++ -lpthread -L$(TARGET_DIR)/usr/lib -o $@ $(LOCAL_SRC_FILES)

%.o:%.c
	$(CC) $(INCLUDE_FLAGS) $(CFLAGS) $(LFLAGS) -c $< -o $@
	
.PHONY : clean
clean:
	rm -f *.o
