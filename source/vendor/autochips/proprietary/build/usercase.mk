#
#  For ac8x user-case build
#
USERCASE_SRC := $(TOP_DIR)user-case

$(INSTALLED_EMULATION_TARGET): usercase

.PHONY: usercase

usercase:
	cd $(USERCASE_SRC);	./env_setup.sh
	@echo "usercase build done ->:)"
	@echo "add fake files for autobuild"
	mkdir -p $(TOP_DIR)emulation/modules/v2/sim/run
	cp $(USERCASE_SRC)/modules/ac55_core/sim/z1run/mem0_0.vmf $(TOP_DIR)emulation/modules/v2/sim/run/mem0_0.vmf
	cp $(USERCASE_SRC)/modules/ac55_core/sim/z1run/rom0.hex $(TOP_DIR)emulation/modules/v2/sim/run/rom0.hex
	cp $(USERCASE_SRC)/modules/ac55_core/sim/z1run/rom1.hex $(TOP_DIR)emulation/modules/v2/sim/run/rom1.hex

clean: usercase-clean

usercase-clean:
	rm -rf $(USERCASE_SRC)/modules/ac55_core/sim/z1run/mem0_0.vmf
	rm -rf $(USERCASE_SRC)/modules/ac55_core/sim/z1run/rom0.hex
	rm -rf $(USERCASE_SRC)/modules/ac55_core/sim/z1run/rom1.hex
	rm -rf $(USERCASE_SRC)/modules/ac55_core/sim/z1run/software
