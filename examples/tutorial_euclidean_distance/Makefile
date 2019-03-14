CBMC_GC = ../../bin/cbmc-gc
CIRCUIT_SIM = ../../bin/circuit-sim
ABY_CBMC_GC = ../../../ABY/build/bin/aby-hycc

.PHONY: clean run-sim

mpc_main.circ: main.c
	$(CBMC_GC) $^ --minimization-time-limit 10

run-sim:
	@$(CIRCUIT_SIM) mpc_main.circ --spec "INPUT_A := {x: 3; y: 9;}; INPUT_B := {x: -4; y: 5;}; print;"

run-aby:
	@$(ABY_CBMC_GC) mpc_main.circ -r 0 --spec "INPUT_B := {x: -4; y: 5;}; print;" &
	@$(ABY_CBMC_GC) mpc_main.circ -r 1 --spec "INPUT_A := {x: 3; y: 9;}; print;"

clean:
	rm -f mpc_main.circ mpc_main.stats
