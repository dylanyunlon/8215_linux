CSRC  := serial.c printf.c aeabi_uidiv.c Simulation_log.c ArmCycleCount.c rand.c timer.c metazone_inter.c
SSRC		 = memcmp.S rt_memcpy_v6.S

PRECOMPILEOBJ = memset.o rt_memclr.o rt_memclr_w.o rt_memcpy_w.o strcmp.o memcpy.o




COBJ     = $(subst .c,.o,$(CSRC))
SOBJ		 = $(subst .S,.o,$(SSRC))	

#Use absolute path for axf finding the source code
NOWDIR  = $(CURDIR)

SUBLINKOBJ	= preloader_lib
SUBLINKOBJDIR = $(OBJDIR)/$(SUBLINKOBJ)

all: $(COBJ) $(SOBJ) Precompile
	@echo "$(SUBLINKOBJ) link......."
	$(LINK) -nostdlib -r -o $(OBJDIR)/$(SUBLINKOBJ).o -T $(NOWDIR)/$(SUBLINKOBJ).lds -Bstatic -Map $(INFODIR)/$(SUBLINKOBJ).map --stats $(wildcard $(SUBLINKOBJDIR)/*.o)
	
	  
$(COBJ): Prepare $(CSRC) 
	$(CC) -c $(NOWDIR)/$(subst .o,.c,$@) $(GOPTION) $(GINCLUDE) $(INCLUDE) -o $(SUBLINKOBJDIR)/$@	  

$(SOBJ): Prepare $(SSRC) 
	$(CC) -c $(NOWDIR)/$(subst .o,.S,$@) $(GOPTION) $(GINCLUDE) $(INCLUDE) -o $(SUBLINKOBJDIR)/$@	
	

		  
Precompile: Prepare $(PRECOMPILEOBJ)
	@for i in $(PRECOMPILEOBJ); \
	  do \
	    cp -f $$i $(SUBLINKOBJDIR)/$$i; \
	  done
		
Prepare:
	mkdir -p $(SUBLINKOBJDIR)
