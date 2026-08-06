VERSION := 1.0

all: compile
	@echo "deivcemanager build end"

configure:
	@echo "commonapi generator start....."
	@cd $(VERSION);
	@echo "commonapi generator end......"

compile: configure
	make -C $(VERSION)

install:
	make -C $(VERSION) install

