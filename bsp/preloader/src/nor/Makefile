CSRC  := nor_loader.c
SSRC     =

COBJ     = $(subst .c,.o,$(CSRC))
SOBJ		 = $(subst .S,.o,$(SSRC))

#Use absolute path for axf finding the source code
NOWDIR  = $(CURDIR)

SUBLINKOBJ	= preloader_nor
SUBLINKOBJDIR = $(OBJDIR)/$(SUBLINKOBJ)

all: $(COBJ) $(SOBJ)
	@echo "$(SUBLINKOBJ) link......."
	$(LINK) -r -o $(OBJDIR)/$(SUBLINKOBJ).o -T $(NOWDIR)/$(SUBLINKOBJ).lds -Bstatic -Map $(INFODIR)/$(SUBLINKOBJ).map --stats $(wildcard $(SUBLINKOBJDIR)/*.o)
	  
$(COBJ): Prepare $(CSRC) 
	$(CC) $(NOWDIR)/$(subst .o,.c,$@) -DTEST_LOOP=$(testloop) $(GOPTION) $(GINCLUDE) $(INCLUDE) -o $(SUBLINKOBJDIR)/$@  
  
$(SOBJ): Prepare $(SSRC) 
	$(CC) -c $(NOWDIR)/$(subst .o,.S,$@) -DTEST_LOOP=$(testloop) $(GOPTION) $(GINCLUDE) $(INCLUDE) -o $(SUBLINKOBJDIR)/$@ 	
  
Prepare:
	mkdir -p $(SUBLINKOBJDIR)
