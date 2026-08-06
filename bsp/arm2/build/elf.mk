#
#
#  ARM2 elf compile rules
#  Wrote by Ke Xu@atc0036

DBG_FUNCTION ?= disable
FIRST_OBJ := $(ARM2OBJDIR)/arm2_src.o

TARGETOBJ: RSV_LIBS  FDT_LIBS COMPILER
	@echo arm2.bin Link...
	$(LINK) $(FIRST_OBJ) $(filter-out $(FIRST_OBJ), $(wildcard $(ARM2OBJDIR)/*.o)) -T $(LDSCRIPT)  -Bstatic -Map $(ARM2INFODIR)/map.txt --verbose --stats  --start-group $(GLOBAL_LDFLAG) $(STATICLIBS)  --end-group -o  $(ARM2OBJDIR)/arm2
	${OBJCPY} -O binary -R .comment -R .note -S $(ARM2OBJDIR)/arm2 $(ARM2OBJDIR)/arm2.bin
	$(NM) $(ARM2OBJDIR)/arm2 | grep -v '\(compiled\)\|\(\.oi$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | LC_ALL=C sort >  $(ARM2OBJDIR)/System.map
	@if [ "$(DBG_FUNCTION)" = "enable" ]; then \
		FILE_SIZE=$$(wc -c < "$(ARM2OBJDIR)/arm2.bin");REMAINDER=$$(($${FILE_SIZE} % 1024));\
		if [ $${REMAINDER} -ne 0 ]; then dd if=/dev/zero of=$(ARM2OBJDIR)/arm2_padding.bin bs=$$((1024-$${REMAINDER})) count=1; else touch $(ARM2OBJDIR)/arm2_padding.bin; fi; \
		$(PY) $(ARM2TOPDIR)/tools/gen_func_table/gen_func_table.py $(ARM2OBJDIR)/System.map addr2line $(ARM2OBJDIR)/arm2 $(ARM2OBJDIR)/table.bin $(ARM2OBJDIR)/table.txt;\
		if [ $$? -eq 0 ]; then \
			echo "#####################################"; \
			echo "####Generate function table pass!####"; \
			echo "#####################################"; \
			$(COPY) $(ARM2OBJDIR)/arm2.bin $(ARM2OBJDIR)/arm2_resc.bin; \
			$(CAT) $(ARM2OBJDIR)/arm2_padding.bin >> $(ARM2OBJDIR)/arm2.bin; \
			$(CAT) $(ARM2OBJDIR)/table.bin >> $(ARM2OBJDIR)/arm2.bin; \
		else \
			echo "#####################################"; \
			echo "####Generate function table failed!##"; \
			echo "#####################################"; \
		fi; \
	fi
	@$(OBJDUMP) -Mreg-names-raw -d $(ARM2OBJDIR)/arm2 > $(ARM2OBJDIR)/arm2.debug.lst

RSV_LIBS:
	mkdir -p $(ARM2OBJDIR)/lib/rsv
	$(MAKE) -C rsv libs "O=$(ARM2OBJDIR)/lib/rsv" "G_INCLUDE=$(GLOBAL_INC)" "SHELL=$(SHELL)" "COMPILER=$(CC)" "ARCHIVER=$(AR)" "COMPILER_FLAGS=$(GLOBAL_CFLAG)  -O2" "EXTRA_COMPILER_FLAGS=-g"
FDT_LIBS:
	mkdir -p $(ARM2OBJDIR)/lib/libfdt
	$(MAKE) -C lib/libfdt libs "O=$(ARM2OBJDIR)/lib/libfdt" "G_INCLUDE=$(GLOBAL_INC)" "SHELL=$(SHELL)" "COMPILER=$(CC)" "ARCHIVER=$(AR)" "COMPILER_FLAGS=$(GLOBAL_CFLAG)  -O2" "EXTRA_COMPILER_FLAGS=-g"

# Display/vcp/wch/tvd clean obj first
GEN_HEADER:
	$(ARM2BUILDDIR)/gen_autover.sh $(target_project)

COMPILER: GEN_HEADER
	@echo Compiler...
	mkdir -p $(ARM2OBJDIR)
	mkdir -p $(ARM2INFODIR)
	@for i in $(SUBDIRS); \
	  do \
	    cd $$i  && make || exit; \
	done

clean:
	-rm -rf $(ARM2OBJDIR)

