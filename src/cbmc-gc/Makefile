SRC = cbmc_gc_main.cpp \
      cbmc_gc_parse_options.cpp \
      cbmc_languages.cpp \
      boolean_expr_lowering.cpp \
      circuit_creator_default.cpp \
      building_blocks/boolean_size_optimized.cpp \
      building_blocks/boolean_depth_optimized.cpp \
      ssa_to_circuit/boolean.cpp \
      ssa_to_circuit/common.cpp \
      ssa_to_circuit/ssa_to_circuit.cpp \
      building_blocks/common.cpp \
      goto_conversion_invocation.cpp \
      sat_equivalence_checker.cpp \
      circuit_creator.cpp \
      ssa_to_circuit/arithmetic.cpp \
      dot/dot_reader.cpp \
      ir/instr.cpp \
      ir/basic_block.cpp \
      ir/function.cpp \
      ir/dominators.cpp \
      ir/debug.cpp \
      ir/pointer_analysis.cpp \
      ir/reaching_definitions.cpp \
      ir/program_dependence_graph.cpp \
      ir/symbol_table.cpp \
      ir/instruction_outliner.cpp \
      cbmc_to_ir.cpp \
      ir_to_cbmc.cpp \
      simple_lexer.cpp \


OBJ += ../cbmc/src/ansi-c/ansi-c$(LIBEXT) \
      ../cbmc/src/cpp/cpp$(LIBEXT) \
      ../cbmc/src/linking/linking$(LIBEXT) \
      ../cbmc/src/big-int/big-int$(LIBEXT) \
      ../cbmc/src/goto-programs/goto-programs$(LIBEXT) \
      ../cbmc/src/goto-symex/goto-symex$(LIBEXT) \
      ../cbmc/src/pointer-analysis/value_set$(OBJEXT) \
      ../cbmc/src/pointer-analysis/value_set_analysis_fi$(OBJEXT) \
      ../cbmc/src/pointer-analysis/value_set_domain_fi$(OBJEXT) \
      ../cbmc/src/pointer-analysis/value_set_fi$(OBJEXT) \
      ../cbmc/src/pointer-analysis/value_set_dereference$(OBJEXT) \
      ../cbmc/src/pointer-analysis/dereference_callback$(OBJEXT) \
      ../cbmc/src/pointer-analysis/add_failed_symbols$(OBJEXT) \
      ../cbmc/src/pointer-analysis/rewrite_index$(OBJEXT) \
      ../cbmc/src/pointer-analysis/goto_program_dereference$(OBJEXT) \
      ../cbmc/src/analyses/analyses$(LIBEXT) \
      ../cbmc/src/langapi/langapi$(LIBEXT) \
      ../cbmc/src/xmllang/xmllang$(LIBEXT) \
      ../cbmc/src/assembler/assembler$(LIBEXT) \
      ../cbmc/src/solvers/solvers$(LIBEXT) \
      ../cbmc/src/util/util$(LIBEXT) \
      ../cbmc/src/cbmc/xml_interface$(OBJEXT) \
      ../cbmc/src/cbmc/symex_bmc$(OBJEXT) \
      ../cbmc/src/cbmc/bmc$(OBJEXT) \
      ../cbmc/src/cbmc/symex_coverage$(OBJEXT) \
      ../cbmc/src/cbmc/fault_localization$(OBJEXT) \
      ../cbmc/src/cbmc/all_properties$(OBJEXT) \
      ../cbmc/src/cbmc/show_vcc$(OBJEXT) \
      ../cbmc/src/cbmc/bv_cbmc$(OBJEXT) \
      ../cbmc/src/cbmc/counterexample_beautification$(OBJEXT) \
      ../cbmc/src/cbmc/bmc_cover$(OBJEXT)


include ../cbmc/src/config.inc
include ../cbmc/src/common

INCLUDES = -I../cbmc/src
INCLUDES += -I..

LIBS = -L../libcircuit -lcircuit

ifeq ($(USE_ABC),yes)
  LIBS = -L../abc -labc -ldl -lpthread -lreadline
  INCLUDES += -I../abc/src
  CP_CXXFLAGS += -DCBMC_GC_USE_ABC $(shell ../abc/arch_flags)
endif

ifdef SIMPLE_CIRCUIT_DEBUG
  CP_CXXFLAGS += -DSIMPLE_CIRCUIT_DEBUG
endif

CLEANFILES = cbmc-gc$(EXEEXT)

all: cbmc-gc$(EXEEXT)

ifneq ($(wildcard ../bv_refinement/Makefile),)
  OBJ += ../bv_refinement/bv_refinement$(LIBEXT)
  CP_CXXFLAGS += -DHAVE_BV_REFINEMENT
endif

ifneq ($(wildcard ../java_bytecode/Makefile),)
  OBJ += ../java_bytecode/java_bytecode$(LIBEXT)
  CP_CXXFLAGS += -DHAVE_JAVA_BYTECODE
  ifneq ($(wildcard $(LIBZIPINC)),)
    LIBS += $(LIBZIPLIB)
  endif
endif

ifneq ($(wildcard ../jsil/Makefile),)
  OBJ += ../jsil/jsil$(LIBEXT)
  CP_CXXFLAGS += -DHAVE_JSIL
endif

ifneq ($(wildcard ../specc/Makefile),)
  OBJ += ../specc/specc$(LIBEXT)
  CP_CXXFLAGS += -DHAVE_SPECC
endif

ifneq ($(wildcard ../php/Makefile),)
  OBJ += ../php/php$(LIBEXT)
  CP_CXXFLAGS += -DHAVE_PHP
endif

# Write the compiler flags used to compile CBMC-GC to compiler_flags.h
.PHONY: force
compiler_flags.h: force
	echo '#define CBMC_GC_COMPILE_FLAGS "$(CP_CXXFLAGS)"' | cmp -s - $@ || echo '#define CBMC_GC_COMPILE_FLAGS "$(CP_CXXFLAGS)"' > $@

# Make all objects depend on compiler_flags.h so they will be rebuild when the flags change
$(SRC:.cpp=.o): compiler_flags.h

###############################################################################

cbmc-gc$(EXEEXT): ../libcircuit/libcircuit.a

cbmc-gc$(EXEEXT): $(OBJ)
	$(LINKBIN)



TEST_SRC = test/main.cpp \
           test/dot_reader.cpp \
           test/parallel_execution_graph.cpp \
           test/pointer_analysis.cpp \
           test/idom.cpp \


TEST_OBJ += $(patsubst %.cpp, %$(OBJEXT), $(TEST_SRC))
TEST_OBJ += $(patsubst %.cpp, %$(OBJEXT), $(SRC))
TEST_OBJ += $(OBJ)

# Remove CBMC-GC's main function
TEST_OBJ := $(filter-out cbmc_gc_main$(OBJEXT),$(TEST_OBJ))

test/test: $(TEST_OBJ)
	$(LINKBIN)
