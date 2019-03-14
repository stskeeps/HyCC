SRC = simple_circuit.cpp \
      simple_circuit_clustering.cpp \
      simple_circuit_converting.cpp \
      simple_circuit_dot.cpp \
      simple_circuit_evaluate.cpp \
      simple_circuit_framework_adapter.cpp \
      simple_circuit_gate.cpp \
      simple_circuit_rewriting.cpp \
      simple_circuit_rewriting_old.cpp \
      simple_circuit_structurizing.cpp \
      simple_circuit_sweep.cpp \
      simple_circuit_theorems.cpp \
      simple_circuit_merge.cpp \
      simple_circuit_file.cpp \
      type.cpp \
      runner.cpp \
	  logger.cpp \
	  simulation.cpp \
      parallel_execution_graph.cpp


include ../cbmc/src/config.inc
include ../cbmc/src/common

INCLUDES = -I../cbmc/src
INCLUDES += -I..

CP_CXXFLAGS += -std=c++14

ifdef SIMPLE_CIRCUIT_DEBUG
  CP_CXXFLAGS += -DSIMPLE_CIRCUIT_DEBUG
endif

CLEANFILES = libcircuit$(LIBEXT)

all: libcircuit$(LIBEXT)

# Write the compiler flags used to compile libcircuit to compiler_flags.h
.PHONY: force
compiler_flags.h: force
	echo '#define CIRCUIT_COMPILE_FLAGS "$(CP_CXXFLAGS)"' | cmp -s - $@ || echo '#define CIRCUIT_COMPILE_FLAGS "$(CP_CXXFLAGS)"' > $@

# Make all objects depend on compiler_flags.h so they will be rebuild when the flags change
$(SRC:.cpp=.o): compiler_flags.h

###############################################################################

libcircuit$(LIBEXT): $(OBJ)
	$(LINKLIB)

