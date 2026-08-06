#Global Variable         


SUBDIRS = ddr_init \
         ddr_special \
         ddr_cal 

	
all: 
	@for i in $(SUBDIRS); \
	  do \
	    $(MAKE) -C $$i $@ || exit $(?) ; \
	  done
	
