include ../common.inc

CBMCGC = $(CBMC_GC_ROOT)/bin/cbmc-gc

circuit: biomatch.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --all-variants

decomposed: biomatch.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --function mpc_main_decomposed --all-variants

clean:
	rm -f *.circ
	rm -f *.dot
	rm -f *.ps
	rm -f *.stats

