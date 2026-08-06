all: ZLIB PARSE OS BINS
	@echo "Recovery construct done"

BINS: APP LIBRARY OS
	@sh $(recovery_build)/mkrootfs.sh

APP: LIBRARY OS
	@make -C $(recovery_app)

LIBRARY: OS
	@make -C $(recovery_lib)

OS:
	@sh $(recovery_build)/mkos.sh