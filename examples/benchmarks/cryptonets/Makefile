include ../common.inc

CBMCGC = $(CBMC_GC_ROOT)/bin/cbmc-gc

cryptonets: cryptonets.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --all-variants

cryptonetsfp: cryptonets_fixpt.c fp.h
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --all-variants

