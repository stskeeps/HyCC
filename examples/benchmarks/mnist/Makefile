include ../common.inc

CBMCGC = $(CBMC_GC_ROOT)/bin/cbmc-gc


minst: mnist.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} 

mnist-all: mnist.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --all-variants

mnist-main: mnist_decomposed_main.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --all-variants

mnist-convs: mnist_decomposed_convolution.c
	${CBMCGC} $^ --minimization-time-limit ${MINIMIZATION_TIME} --all-variants

clean:
	rm -f *.circ
	rm -f *.dot
	rm -f *.ps
	rm -f *.stats
