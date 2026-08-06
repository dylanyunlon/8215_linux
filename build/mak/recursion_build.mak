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

ifeq ($(SUB-TARGETS),)
#  SUB-TARGETS := $(shell ls -l | grep ^d | awk '{print $$9}')
endif

#run_qtmk_cmd := \
#  QTDIR=$(OSS_LIB_TOP)/gnuarm-4.8.2_vfp/qt/5.5.0/usr && \
#  QMAKE=$$QTDIR/bin/qmake && \
#  QMKSPEC=$$QTDIR/gnuarm-4.8.2_vfp/qt/4.8.6/usr/mkspecs/qws/linux-arm-mediatek-g++ && \
#  $$QMAKE CONFIG+=release
                                   
#run_qmtk_cmd := \
#  QTDIR=$(OSS_LIB_TOP)/gnuarm-4.8.2_vfp/qt/5.5.0/usr && \
#  QMAKE=$$QTDIR/bin/qmake && \
#  $$QMAKE CONFIG+=release
  
run_qmtk_cmd := \
  QTDIR=$(OECORE_NATIVE_SYSROOT)/usr && \
  QMAKE=$$QTDIR/bin/qt5/qmake && \
  $$QMAKE CONFIG+=release

$(warning $(run_qmtk_cmd))
.PHONY: all $(SUB-TARGETS) clean install
all: $(SUB-TARGETS)

$(SUB-TARGETS):
	@tmp=`find $@ -name '*.pro'`; if [ -n "$$tmp" ] && [ ! -e $@/Makefile ]; then \
	  cd $@; $(run_qmtk_cmd); cd ../;    \
	  $(MAKE) --directory=$@;       \
	elif [ -e $@/Makefile ]; then             \
	  $(MAKE) --directory=$@;  \
	else	\
	  echo "pls check makefile"; \
	fi;

clean: 
	@for d in $(SUB-TARGETS) ;                   \
	do                                   \
	    $(MAKE) --directory=$$d clean;   \
	done

