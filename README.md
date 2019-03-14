# HyCC


This repository contains HyCC, a compiler for optimized circuits for hybrid MPC from ANSI-C, which is built as follow-up work to CBMC-GC [1] and the ABY framework [2].
Details to the underlying techniques are described in the corresponding paper [3].

 * [1] http://forsyte.at/software/cbmc-gc/
 * [2] https://github.com/encryptogroup/ABY
 * [3] Niklas Büscher, Daniel Demmler, Stefan Katzenbeisser, David Kretzmer, Thomas Schneider:  "HyCC: Compilation of Hybrid Protocols for Practical Secure Computation". ACM CCS 2018.

## Workflow

The workflow is currently as follows:
  1. clone this repository and compile HyCC
  2. compile your C application using HyCC into a circuit consisting of modules
  3. create and select a circuit module bundle file
  4. evaluate the generated circuit (modules) using ABY

In the following sections we explain each step in more detail.


### 1. Compiling HyCC


Run in the root directory:

    make minisat2-download
    make


### 2. Compiling Circuits with HyCC


Example programs are given in examples/ and can be compiled with make.
Please have a look at the detailed documentation and paper listed at [2].
Make sure that your program is using CBMC-GC's i/o variable notation and
that the program is bound (loops/recursion) or specify an upper bound for
the loop unrolling:
  `--unwind X`
 where X is the upper bound on unrolling loops. To limit the circuit
 minimization phase please use (this is essential for large programs!):
  `--minimization-time-limit X`
where X is an integer value in seconds. To compile circuits with minimal
depth (ShallowCC) add the following flag:
  `--low-depth`

Note that the (depth) reduction preprocessing step described in [3] is
not contained in this release. Sequential arithmetics are compressed when
written in a single expression (t = a * b + c)!

The following looks for a function called `mpc_main` in `main.c` and generates a
single boolean circuit file `mpc_main.circ` for it:

    cbmc-gc main.c

By specifying `--bool <func_name>` or `--arith <func_name>` you can compile
individual functions into separate boolean or arithmetic circuit files (called
modules). By specifying `--all-variants` you can compile all functions into
separate boolean and arithmetic circuits (compilation to arithmetic circuits is
not always possible). Use this option when using Protocol Selection (see further
below).


#### Evaluating Circuits


The `circuit-sim` utility allows to evaluate (simulate) circuits in the clear.
(This is mostly useful for testing a generated circuit.) If you want to execute
the circuit in the file `adder.circ` that has two integer inputs `INPUT_A` and
`INPUT_B`, you can run the following command:

    circuit-sim adder.circ --spec "INPUT_A := 42; INPUT_B := 99; print;"

This will execute the circuit and then print the values of all input and output
variables.

Arrays can be specified with `[1, 2, 3]` and structs with `{a: 23; b: 48;}`
(don't forget the semicolon after the last attribute).

You can also specify assertions:

    circuit-sim adder.circ --spec "INPUT_A := 42; INPUT_B := 99; return_value == 141;"

This will print a error message if the output of the circuit is not 141.

Multiple runs of a circuit can be specified in a file.

    INPUT_A := 42;
    INPUT_B := 99;
    return_value == 141;
    ---
    INPUT_A := 100;
    INPUT_B := 200;
    return_value == 300;

If the above is saved to `test.spec` it can be run like so:

    circuit-sim adder.circ --spec-file test.spec


### 3. Circuit Module Bundle Generation and Protocol Selection

(Note: This step is currently done partially by hand, but will be integrated into the compilation process in the future.
We are currently in the process of fine-tuning these steps, in order to have a fully automated solution soon.)

Run `module_bundle.py module-directory/` to generate up to 4 possible module configurations, for example:

    ./module_bundle.py examples/biomatch/

This will generate `all.cmb`,  `gmwhybrid.cmb`, `gmwonly.cmb`, `yaohybrid.cmb`, and `yaoonly.cmb`, which can be passed to ABY in the next step. Currently users have to manually choose the .cmb file and thus the (hybrid) protocol that works best for their use case. This will happen automatically in the future.


### 4. Evaluating Circuits with ABY


To use a generated circuit with ABY you need to:

1. Clone and build the ABY framework (detailed instructions can be found on
   <https://github.com/encryptogroup/ABY>).
2. The framework needs to be put next to the `cbmc-gc/` folder (i.e. one directory up from this README).
3. Copy the folder `cbmc-gc/aby-hycc/` to `ABY/src/examples/aby-hycc/`.
4. Append `add_subdirectory(aby-hycc)` to `ABY/src/examples/CMakeLists.txt`
5. Create a build directory in the ABY directory: `mkdir build && cd build`
6. Run cmake in the `ABY/build` directory `cmake .. -DABY_BUILD_EXE=On`
7. Run make in the `ABY/build` directory: `make`
6. `ABY/build/bin/` now contains a binary called `aby-hycc`.

To use `aby-hycc` you first need to copy it into the directory containing
the circuit files, e.g. `cp bin/aby-hycc ../../cbmc-gc/examples/biomatch/`.

Example usage:

    aby-hycc -r 0 -c modules.cmb &
    aby-hycc -r 1 -c modules.cmb

The option `-r 0` starts `aby-hycc` as a server and `-r 1` as a client.

The parameter `--spec` is used to pass inputs to the parties, e.g.: `--spec "INPUT_B_sample := [0, 1, 2, 3];"
`.
Inputs starting with `INPUT_B` are associated with the server and the ones
starting with `INPUT_A` with the client.


### Export to other formats


You can use `circuit-utils` to convert a circuit generated by CBMC-GC to another
format.

Currently supported export options:

- `--as-bristol`: Bristol format (<https://homes.esat.kuleuven.be/~nsmart/MPC/>).
  Note that currently it is not possible to use the Bristol format for circuits
  that directly connect input gates to output gates.
- `--as-shdl`: Fairplay's SHDL format.
- `--as-scd`: justGarble's Simple Circuit Description format.

Example:

    circuit-utils adder.circ --as-bristol bristol_circuit.txt

to convert the circuit to the Bristol format. If your framework is only
supporting AND, XOR and NOT gates (e.g., the EMP-toolkit), use

    circuit-utils adder.circ --remove-or-gates --as-bristol bristol_circuit.txt


## Tests


Building all tests: `make test`.

Running all tests: `make run-test`.

The above commands will create two copies of `test-src/`: `test-default/` and
`test-lowdepth/`. The only difference between them is that the tests in
`test-lowdepth/` will be compiled using `--low-depth`.

The basic idea is that `test-src/` contains only the source files of the tests
and that copies of the whole directory are made for each specific set of
command-line options to `cbmc-gc`. So do not run `make` directly in `test-src`
but only in copies of it.

For example, to test `cbmc-gc` with depth-optimization enabled you would copy
`test-src/` to e.g. `test-lowdepth/` and append `--low-depth` to
`test-lowdepth/CBMC_GC_FLAGS`. Then you would `cd` into `test-lowdepth` and run
`make run` to compile and execute all tests. (Note that this specific test-case
will be created by default so you don't need to actually do it.)

You can also build or run individual tests. Example: From a test directory
(e.g. `test-default/`) run:

    make build-addition_int32
    make run-addition_int32

This will first build and then run the test-case `addition_int32`.


#### Comparing test results


To get a quick overview of the number of non-XOR gates of each test use
`testcmp` to generate a HTML table:

    ./testcmp test-default test-lowdepth > comparison.html

This will create a table containing the results from the directories
`test-default/` and `test-lowdepth/` and write it to `comparison.html`.
You can specify an arbitrary number of test directories to compare.


#### Differential testing:


CBMC-GC has built support for differential testing of c code against
circuits. There are currently two possible ways to test a circuit: (1)
you can create a test-runner that generates random inputs and compares
the output of the circuit with the output of the original C version or
(2) you can *verify* that the circuit behaves exactly the same as the
C code. Whether you can use (2) depends on the complexity of the
circuit: if the circuit contains no multiplication or division then you
are often fine. 8-bit multiplication/division shouldn't cause any
problems either, but anything more is probably too much.

To generate a random-input test-runner for the circuit in file `adder.circ`,
execute the following:

    circuit-utils adder.circ --create-tester tester.cpp --reference adder.c

where `adder.c` must contain the C code for the circuit in `adder.circ`. The
output is a C++ file which, when compiled and executed, will test the circuit
and output any values that differ from the output of the C version.

To create a verifier, run:

    circuit-utils adder.circ --create-verifier verifier.c --reference adder.c

The resulting `verifier.c` file can then be fed into `cbmc`:

    cbmc verifier.c

If the verification is successful then the circuit should be equivalent to the C
code.

NOTE: Currently, creating tests poses some restrictions on the C code:

  1. The circuit must have exactly two input variables (one for each party).
     If a party must provide data in more than one variable put them into a
     struct.
  2. The types of `INPUT_A`, `INPUT_B` and of the output variable must be named
     `InputA`, `InputB` and `Output`, respectively.

Take a look at the test-cases for examples.

