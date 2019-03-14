include ../common.inc

CPP = g++  -std=c++11

CBMCGC = $(CBMC_GC_ROOT)/bin/cbmc-gc


circuit: kmeans.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME}  

circuit-all: kmeans.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME}  --all-variants

binary: kmeans.c
	${CPP} -g --sanitize=address $^ -o kmeans

debug: kmeans.c
	${CPP} -g $^ -o kmeans

clean:
	rm -f *.circ
	rm -f *.dot
	rm -f *.ps
	rm -f *.stats

