include ../common.inc

CPP = clang  

CBMCGC = $(CBMC_GC_ROOT)/bin/cbmc-gc

circuit_merge: db_merge.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --all-variants

circuit_join: db_join.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --function mpc_main --all-variants

circuit_join2: db_join2.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --function mpc_main --all-variants

join: db_join.c
	${CPP} -g  -DGCC $^ -o join

merge: db_merge.c
	${CPP} -DGCC $^ -o merge


clean:
	rm -f *.circ
	rm -f *.dot
	rm -f *.ps
	rm -f *.stats

