MK           := mkdir -p
RM           := rm -rf
CMAKE        := cmake

GENERATOR     ?= Unix Makefiles
BUILD_DIR     ?= $(shell pwd)/cmake-build
SOURCE_DIR    = "$(shell pwd)/source"

default: check

configure:
	$(RM) $(BUILD_DIR) && $(MK) $(BUILD_DIR)
	cd $(BUILD_DIR) && $(CMAKE) -G"$(GENERATOR)" -DCMAKE_BUILD_TYPE=DEBUG $(SOURCE_DIR)

check: configure
	$(CMAKE) --build $(BUILD_DIR) -- -j$(shell nproc) check