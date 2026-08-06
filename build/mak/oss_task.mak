.PHONY : do_patch do_config

do_patch:
	@ echo "do patch for "$(M)"-"$(V) 
	@ python $(DA_TOP)/build/tools/tasks/do_patch.py $(M) $(V) $(E)
	@ echo "do patch done!"

do_config: 
	@ echo "do config for oss"
	@ sh $(DA_TOP)/build/tools/tasks/do_configure.sh $(M) $(V)
	@ echo "do config done!"

do_pc: do_patch do_config
