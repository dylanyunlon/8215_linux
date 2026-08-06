COMPILER=
ARCHIVER=
COMPILER_FLAGS=
EXTRA_COMPILER_FLAGS=
G_INCLUDE=
LIB=librsv.a

CC_FLAGS = $(COMPILER_FLAGS)
ECC_FLAGS = $(EXTRA_COMPILER_FLAGS)

RELEASEDIR=$(O)/
INCLUDEDIR=$(O)/
INCLUDES=-I./. -I${INCLUDEDIR} $(G_INCLUDE)
OUTS = *.o
OBJECT_PATH = $(O)/

LIBSOURCES:=*.c
INCLUDEFILES:=*.h

OBJS =	$(addsuffix .o, $(basename $(wildcard *.c)))
OBJECTS =$(addprefix $(OBJECT_PATH)/, $(OBJS))
.PHONY: all libs banner rsv_libs
all:
	echo "Compiling do nothing"

libs: banner rsv_libs

$(OBJECT_PATH)/%.o: %.c
	@ echo Compiling $<
	@${COMPILER} $(CC_FLAGS) $(ECC_FLAGS) $(INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

banner:
	echo "Compiling librsv.a"

rsv_libs: ${OBJECTS}
	$(ARCHIVER) -r ${RELEASEDIR}/${LIB} ${OBJECTS}

clean:
	rm -rf ${OBJECTS}
	@rm -rf $(OBJECT_PATH)/$(LIB)
