TARGET :=hello

#CC ?=
#LIBS ?=
#LDFLAGS ?=
#DEFINES ?=
#INCLUDE ?= -I.
CFLAGS += -g -Wall -fPIC
CXXFLAGS += $(CFLAGS) -DHAVE_CONFIG_H
#SHARE := -shared -o
SHARE := -o


SOURCE :=$(wildcard *.c) $(wildcard *.cpp)
OBJS   := $(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SOURCE)))

.PHONY : everything objs clean rebuild veryclean

everything: $(TARGET)

all:$(TARGET)

objs: $(OBJS)

rebuild: veryclean everythings

clean:
	rm -fr *.o

veryclean: clean
	rm -fr $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $(SHARE) $@ $(OBJS) $(LDFLAGS) $(LIBS)
