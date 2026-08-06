VERSION := 1.0

all: compile
	@echo "sample build end"

configure:
	@cd $(VERSION);

compile: configure
	make -C $(VERSION)

install:
	make -C $(VERSION) install