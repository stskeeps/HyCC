CBMC_GC_ROOT_DIR = ../..
BUILD_DIR = ${CBMC_GC_ROOT_DIR}/bin

INCLUDE_DIRS = -I${CBMC_GC_ROOT_DIR}/src -I${CBMC_GC_ROOT_DIR}/src/cbmc/src
LIBS = -L${CBMC_GC_ROOT_DIR}/src/libcircuit -lcircuit

CPP = g++ -std=c++11 -Wall -Wextra -O2


.PHONY: all clean

all: ${BUILD_DIR}/callstack

${BUILD_DIR}/callstack: callstack.cpp ${CBMC_GC_ROOT_DIR}/src/libcircuit
	${CPP} callstack.cpp -o $@ ${INCLUDE_DIRS} ${LIBS}

clean:
	rm -f ${BUILD_DIR}/callstack
