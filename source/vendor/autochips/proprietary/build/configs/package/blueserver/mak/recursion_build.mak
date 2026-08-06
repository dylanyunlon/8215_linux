#
#
#
#    For build all dir. Wrote by Ke.Xu@ATC
#
#
#    make                      -- Will make all sub-folder in this folder
#    make sub-folder-name      -- Will just make sub-folder 
#    make clean                -- Will execute all sub-folder clean action
#                              -- Not support to clean a specify folder target

#    Please pay attention:  1. All sub-folder can't named with space. Such as "space name" is Forbidden!
#                           2. Don't create any tmp folder which without 'Makefile', it maybe cause some error.


# awk '{print $$9}', in some machine, it maybe not match folder name. Just a remind.

                                   
run_qmtk_cmd := \
  QTDIR=$(OSS_LIB_TOP)/gnuarm-4.8.2_vfp/qt/5.5.0/usr && \
  QMAKE=$$QTDIR/bin/qmake && \
  $$QMAKE CONFIG+=release

.PHONY: all $(SUB-TARGETS) clean install
all: $(SUB-TARGETS)

$(warning $(TARGET_TOP))
ifeq "$(BUILD_DIR)" ""
  BUILD_DIR := $(TARGET_TOP)
endif
$(warning $(BUILD_DIR))

$(SUB-TARGETS):
	@mkdir -p $(TARGET_TOP)
	@mkdir -p $(BUILD_DIR)
	@tmp=`find $@ -name '*.pro'`; if [ -n "$$tmp" ] && [ ! -e $@/Makefile ]; then \
	  cd $@; $(run_qmtk_cmd); cd ../;    \
	  $(MAKE) --directory=$@;       \
	elif [ -e $@/Makefile ]; then             \
	  $(MAKE) --directory=$@ TARGET_TOP=$(TARGET_TOP) BUILD_DIR=$(BUILD_DIR)/$@;  \
	else	\
	  echo "pls check makefile"; \
	fi;

clean: 
	@for d in $(SUB-TARGETS) ;                   \
	do                                   \
	    $(MAKE) --directory=$$d clean;   \
	done

install:
	@for d in $(SUB-TARGETS); 	\
	do 				\
		$(MAKE) --directory=$$d install TARGET_TOP=$(TARGET_TOP)  BUILD_DIR=$(BUILD_DIR)/$$d; \
	done	

uninstall:
	@for d in $(SUB-TARGETS); 	\
	do 				\
		$(MAKE) --directory=$$d uninstall; \
	done	

distclena:uninstall
distuninstall:uninstall

