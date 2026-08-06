SRCS    = vectors.S

OBJS    = $(subst .S,.o,$(SRCS))

#Use absolute path for axf finding the source code
NOWDIR  = $(CURDIR)

all: $(OBJS) 
# 	$(LINK) -o $(OBJDIR)/preloader_init.o -T $(NOWDIR)/preloader_init.lds -Map $(INFODIR)/preloader_init.map $(OBJDIR)/$(OBJS)

$(OBJS): $(SRCS)
	$(CC) $(NOWDIR)/$(subst .o,.S,$@) $(GOPTION) $(GINCLUDE) $(DEFINES) -o $(OBJDIR)/$@
