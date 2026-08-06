#Global Variable         


SUBDIRS = boot \
          lib \
          main \
		  sys_manager \
		  libhardware \
	  suspend  

ifeq ($(bootdevice), nand)
	SUBDIRS += nfi
endif

ifeq ($(bootdevice), sd)
	SUBDIRS += msdc
endif

ifeq ($(bootdevice), emmc)   
	SUBDIRS += msdc          
endif                      

ifeq ($(bootdevice), nor)
	SUBDIRS += nor
endif

ifeq ($(target),realchip)
	SUBDIRS += dramk 
endif

ifeq ($(target),fpga)
	SUBDIRS += dramk 
endif

ifeq ($(target),sim)
	SUBDIRS += dramk 
endif

ifeq ($(project),AndroidM)
SUBDIRS += security
endif
	
all: 
	@for i in $(SUBDIRS); \
	  do \
	    $(MAKE) -C $$i $@ || exit $(?) ; \
	  done
	
