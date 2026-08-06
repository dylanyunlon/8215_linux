#
#
#  ARM2 obj compile rules
#  Wrote by Ke Xu@atc0036


# makes sure the target dir exists
MKDIR = if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi

$(TARGETOBJDIR)/%.o: %.c 
	@$(MKDIR)
	@echo compiling $<
	$(NOECHO)$(CC) $(GLOBAL_CFLAG) $(THUMBCFLAGS) -Os --std=c99 $(GLOBAL_INC) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(TARGETOBJDIR)/%.o: %.cpp 
	@$(MKDIR)
	@echo compiling $<
	$(NOECHO)$(CC) $(GLOBAL_CFLAG) $(CPPFLAGS) $(THUMBCFLAGS) -Os $(GLOBAL_INC) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@ -fexceptions -frtti

$(TARGETOBJDIR)/%.o: %.S
	@$(MKDIR)
	@echo compiling $<
	$(NOECHO)$(CC) $(GLOBAL_CFLAG) $(ASMFLAGS) $(GLOBAL_INC) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

