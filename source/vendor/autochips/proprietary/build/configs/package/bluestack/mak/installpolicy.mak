#  
#  ATC Install Policy
#
#  Default, all atc lib & bin will be installed in /usr/lib & /usr/bin, conf file will be installed in /etc, 
#  and include file will be installed in /usr/include.
#  Destination is defined by libdir bindir sysconfdir includedir and so on.
#  LOCAL_INCLUDE_PRE will configure the inlcude file prefix. If you want install your header file in /usr/include/myinc, you must set #  LOCAL_INCLUDE_PRE as myinc.  
#  The var LOCAL_LIB_PRE & LOCAL_BIN_PRE are the same meaning as LOCAL_INCLUDE_PRE
#
INSTALL_INC_FILES := $(LOCAL_INSTALL_FILES)
INSTALL_LIB_FILES := $(LOCAL_INSTALL_LIBS)
INSTALL_EXE_FILES := $(LOCAL_INSTALL_EXES)
INSTALL_DOC_FILES := $(LOCAL_INSTALL_DOCS)
INSTALL_CONF_FILES := $(LOCAL_INSTALL_CONF)

.PHONY: install distclean distuninstall

DESTDIR ?= $(DA_SYSROOT)

install: inc_install lib_install exe_install doc_install conf_install 

inc_install:$(INSTALL_INC_FILES)
	@test -n "$(INSTALL_INC_FILES)" || exit 0; $(INSTALL_DIR) $(DESTDIR)$(includedir)/$(LOCAL_INCLUDE_PRE)
	@test -n "$(INSTALL_INC_FILES)" || exit 0; $(INSTALL_DIR) $(localincludedir)/$(LOCAL_INCLUDE_PRE)
	for i in $(INSTALL_INC_FILES) ; \
	do \
		$(INSTALL_FILE) $$i $(DESTDIR)$(includedir)/$(LOCAL_INCLUDE_PRE); \
#		$(INSTALL_FILE) $$i $(localincludedir)/$(LOCAL_INCLUDE_PRE) ; \
	done

lib_install:$(INSTALL_LIB_FILES)
	@test -n "$(INSTALL_LIB_FILES)" || exit 0; $(INSTALL_DIR) $(DESTDIR)$(libdir)/$(LOCAL_LIB_PRE)
	for i in $(INSTALL_LIB_FILES) ; \
	do \
		$(INSTALL_PROGRAM) $$i $(DESTDIR)$(libdir)/$(LOCAL_LIB_PRE); \
	done

exe_install:$(INSTALL_EXE_FILES)
	test -n "$(INSTALL_EXE_FILES)" || exit 0; $(INSTALL_DIR) $(DESTDIR)$(bindir)/$(LOCAL_BIN_PRE)
	for i in $(INSTALL_EXE_FILES) ; \
	do \
		$(INSTALL_PROGRAM) $$i $(DESTDIR)$(bindir)/$(LOCAL_BIN_PRE); \
	done

doc_install:$(INSTALL_DOC_FILES)
	@test -n "$(INSTALL_DOC_FILES)" || exit 0; $(INSTALL_DIR) $(DESTDIR)$(docdir)/$(LOCAL_DOC_DIR)
	for i in $(INSTALL_DOC_FILES) ; \
	do \
		$(INSTALL_FILE) $$i $(DESTDIR)$(docdir)/$(LOCAL_DOC_DIR); \
	done

conf_install:$(LOCAL_INSTALL_CONF)
	@test -n "$(LOCAL_INSTALL_CONF)" || exit 0; $(INSTALL_DIR) $(DESTDIR)$(sysconfdir)/$(LOCAL_CONF_DIR)
	for i in $(INSTALL_CONF_FILES) ; \
	do \
		$(INSTALL_FILE) $$i $(DESTDIR)$(sysconfdir)/$(LOCAL_CONF_DIR); \
	done

uninstall:
	test -d $(DESTDIR)$(includedir)/$(LOCAL_INC_PRE)/ || exit 0; cd $(DESTDIR)$(includedir)/$(LOCAL_INC_PRE)/;  $(RM) $(INSTALL_EXE_FILES)
	test -d $(DESTDIR)$(bindir)/$(LOCAL_BIN_PRE)/ || exit 0; cd $(DESTDIR)$(bindir)/$(LOCAL_BIN_PRE)/;  $(RM) $(INSTALL_EXE_FILES)
	test -d $(DESTDIR)$(libdir)/$(LOCAL_LIB_PRE)/ || exit 0; cd $(DESTDIR)$(libdir)/$(LOCAL_LIB_PRE)/;  $(RM) $(INSTALL_LIB_FILES)
	test -d $(DESTDIR)$(docdir)/$(LOCAL_DOC_DIR)/ || exit 0; cd $(DESTDIR)$(docdir)/$(LOCAL_DOC_DIR)/;  $(RM) $(INSTALL_DOC_FILES)
	test -d $(DESTDIR)$(sysconfdir)/$(LOCAL_CONF_DIR) || exit 0; cd $(DESTDIR)$(sysconfdir)/$(LOCAL_CONF_DIR) || exit 0;  $(RM) $(INSTALL_CONF_FILES)
	
distclean:uninstall
distuninstall:uninstall

