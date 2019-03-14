include ../common.inc

CBMCGC = $(CBMC_GC_ROOT)/bin/cbmc-gc


circuit: gauss.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME}  

circuit-all: gauss.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME}  --all-variants

clean:
	rm -f *.circ
	rm -f *.dot
	rm -f *.ps
	rm -f *.stats

