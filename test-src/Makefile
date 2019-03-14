# Just using * would match directories AND files. */. matches only directories.
SUBDIRS := $(wildcard */.)
# Remove trailing "/.".
SUBDIR_NAMES := $(SUBDIRS:/.=)

export CBMC = $(realpath ../bin/cbmc)
export CBMC_GC = $(realpath ../bin/cbmc-gc)
export CIRCUIT_UTILS = $(realpath ../bin/circuit-utils)

export CBMC_GC_FLAGS_FILE = $(realpath CBMC_GC_FLAGS)
export CBMC_GC_FLAGS = $(shell cat $(CBMC_GC_FLAGS_FILE)) $(CBMC_GC_OPTS)
export CIRCUIT_UTILS_FLAGS


.PHONY: all run clean $(SUBDIR_NAMES)

all: $(SUBDIR_NAMES)

run: $(SUBDIR_NAMES)

clean: $(SUBDIR_NAMES)

$(SUBDIR_NAMES):
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(addprefix run-,$(SUBDIR_NAMES)):
	$(MAKE) -C $(@:run-%=%) run

$(addprefix build-,$(SUBDIR_NAMES)):
	$(MAKE) -C $(@:build-%=%)

$(addprefix clean-,$(SUBDIR_NAMES)):
	$(MAKE) -C $(@:clean-%=%) clean

