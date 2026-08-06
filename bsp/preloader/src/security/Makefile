
CSRC     = tz_init.c


COBJ     = $(subst .c,.o,$(CSRC))

#Use absolute path for axf finding the source code
NOWDIR  = $(CURDIR)

SUBLINKOBJ	= preloader_sec
SUBLINKOBJDIR = $(OBJDIR)/$(SUBLINKOBJ)

all: $(COBJ) $(SOBJ)
	@echo "$(SUBLINKOBJ) link......."
	$(LINK) -nostdlib -r -o $(OBJDIR)/$(SUBLINKOBJ).o -T $(NOWDIR)/$(SUBLINKOBJ).lds -Bstatic -Map $(INFODIR)/$(SUBLINKOBJ).map --stats $(wildcard $(SUBLINKOBJDIR)/*.o)
	  
$(COBJ): Prepare $(CSRC) 
	$(CC) -c $(NOWDIR)/$(subst .o,.c,$@) $(GOPTION) $(GINCLUDE) $(INCLUDE) -o $(SUBLINKOBJDIR)/$@	  

$(SOBJ): Prepare $(SSRC) 
	$(CC) -c $(NOWDIR)/$(subst .o,.S,$@) $(GOPTION) $(GINCLUDE) $(INCLUDE) -o $(SUBLINKOBJDIR)/$@	
	
Prepare:
	mkdir -p $(SUBLINKOBJDIR)
	
