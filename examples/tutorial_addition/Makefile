CBMC_GC = ../../bin/cbmc-gc
CIRCUIT_SIM = ../../bin/circuit-sim
ABY_CBMC_GC = ../../../ABY/build/bin/aby-hycc

.PHONY: clean run-sim run-aby

mpc_main.circ: main.c
	$(CBMC_GC) $^

run-sim:
	@$(CIRCUIT_SIM) mpc_main.circ --spec "INPUT_A := 24; INPUT_B := 42; print;"

run-aby:
	@$(ABY_CBMC_GC) mpc_main.circ -r 0 --spec "INPUT_B := 35; print;" &
	@$(ABY_CBMC_GC) mpc_main.circ -r 1 --spec "INPUT_A := 25; print;"

clean:
	rm -f mpc_main.circ mpc_main.stats
