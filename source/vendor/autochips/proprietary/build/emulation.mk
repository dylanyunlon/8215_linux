#
#  For ac8x emulation user-case build
#
EMULATION_SRC := $(TOP_DIR)emulation
USERCASE_SRC := $(TOP_DIR)user-case

$(INSTALLED_EMULATION_TARGET): emulation usercase

.PHONY: emulation usercase
emulation: 
	cd $(EMULATION_SRC); ./env_setup.sh
	@echo "emulation build done ->:)"

usercase:
	cd $(USERCASE_SRC);	./env_setup.sh
	@echo "usercase build done ->:)"

clean: emulation-clean usercase-clean
emulation-clean:
	rm -rf $(EMULATION_SRC)/modules/v2/sim/run/mem0_0.vmf
	rm -rf $(EMULATION_SRC)/modules/v2/sim/run/rom0.hex
	rm -rf $(EMULATION_SRC)/modules/v2/sim/run/rom1.hex
	rm -rf $(EMULATION_SRC)/modules/v2/sim/run/software

usercase-clean:
	rm -rf $(USERCASE_SRC)/modules/ac55_core/sim/z1run/mem0_0.vmf
	rm -rf $(USERCASE_SRC)/modules/ac55_core/sim/z1run/rom0.hex
	rm -rf $(USERCASE_SRC)/modules/ac55_core/sim/z1run/rom1.hex
	rm -rf $(USERCASE_SRC)/modules/ac55_core/sim/z1run/software
