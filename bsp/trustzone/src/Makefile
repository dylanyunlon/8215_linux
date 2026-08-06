CSRC     = pagetbl.c eabi_compat.c helloworld.c gic_config.c sm.c
SSRC     = monitor.S start.S memset.S sm_call.S

COBJ     = $(subst .c,.o,$(CSRC))
SOBJ		 = $(subst .S,.o,$(SSRC))	

#Use absolute path for axf finding the source code
NOWDIR  = $(PWD)
CURDIR = $(PWD)

CFLAGS += -I../inc
CFLAGS += -I$(CURDIR)/../../linux-local/arch/arm/mach-ac83xx/include/mach
CFLAGS += -I$(CURDIR)../hdcpdecrypt/include

INCLUDE += -I../inc
INCLUDE += -I../rsv
INCLUDE += -I$(CURDIR)/../../linux-local/arch/arm/mach-ac83xx/include/mach
INCLUDE += -I$(CURDIR)../hdcpdecrypt/include

SUBLINKOBJ	= tz_src
SUBLINKOBJDIR = $(OBJDIR)/$(SUBLINKOBJ)

ifeq ($(CONFIG_ARGS_START),)
$(error ***CONFIG_ARGS_START is NULL,please check***)
endif

all: $(SOBJ) $(COBJ)
	@echo "$(SUBLINKOBJ) link......."
	$(LINK) -nostdlib -r -o $(OBJDIR)/$(SUBLINKOBJ).o -T $(NOWDIR)/$(SUBLINKOBJ).lds -Bstatic -Map $(INFODIR)/$(SUBLINKOBJ).map --stats $(wildcard $(SUBLINKOBJDIR)/*.o)
	  
$(COBJ): Prepare $(CSRC) 
	$(CC) -c $(NOWDIR)/$(subst .o,.c,$@) $(GOPTION) $(GINCLUDE) $(INCLUDE) -o $(SUBLINKOBJDIR)/$@ -DCONFIG_ARGS_START=$(CONFIG_ARGS_START)

$(SOBJ): Prepare $(SSRC) 
	$(CC) -c $(NOWDIR)/$(subst .o,.S,$@) $(GOPTION) $(GINCLUDE) $(INCLUDE) -o $(SUBLINKOBJDIR)/$@	
	
Prepare:
	mkdir -p $(SUBLINKOBJDIR)
	
clean:
	rm -rf $(COBJ)
	rm -rf $(SOBJ)
