
# custom-coproc-raim

## Introduction

This project designed and implemented a model of a MIPS co-processor to accelerate and autonomize computations required in GNSS receivers for receiver autonomous integrity monitoring (RAIM). This file explains some of the code i this repository along with running instructions.

## Simulation instructions

The simulation requires the following tools on a Linux system:
* `g++`
* `make`
* `SystemC` - installation script provided in `sysc_install.sh` and specific instructions [below](#systemc-installation-for-linux)

To run the simulation with the pre-provided `PCSpim.log` and `raim.bin` files, use the following command sequence:
```
cd ./model/src
make run ../../alg/asm/raim.bin
```

## `alg`

The `alg` directory holds the preliminary code used to explore the RAIM algorithm along with the proof-of-concept for the implemented functions on the RAIM processing unit (RPU).

### `alg/asm`

The `alg/asm` directory contains the assembly code that runs on the simulated MIPS processor.

* `raim.s` is the initial source code file containing instructions that execute on a traditional MIPS processor. It has `nop` placeholders for instruction slots that further programs will replace with instructions for the RPU.
* `PCSpim.log` is the corresponding log file for `raim.s` after it has passed through PCSpim. To generate this, start up the PCSpim program, load in `raim.s`, then save the program state as a `PCSpim.log` file in this directory.
* The files `s2bin.c` and `Makefile` are the C-source code file and the compilation file for the extended assembler, respectively. The program expects to take in a log file from PCSpim, which PCSpim generated from `raim.s`. Compile and run the `s2bin.c` program with the command `make clean run PCSpim.log raim.bin`. This generates the memory binary file for simulation in `raim.bin`.

### `alg/doc`

This directory contains Latex documentation for the project.

* `alg.tex` is the functional description of what the RPU must accomplish. It serves as a reference of equations pulled from the original ARAIM description. This is the set of functional requirements corresponding to the algorithm analysis section in the final report.
* `rpu.tex` holds tables detailing the register file in the RPU. It shows what virtual registers are accessible, the physical registers stored in memory, and possible values for the condition or exception codes.
* `isa.tex` is a list of instructions that the RPU supports. It follows a similar format to the MIPS instruction set architecture (ISA), so displays instruction encodings and target functionality in register transfer language (RTL) for each instruction.

To view any of these files as a PDF, run the command `make (alg|rpu|isa).pdf` to compile the corresponding Latex file.

### `alg/MATLAB`

This directory is the golden model for the simulation. It does not model the intricacies in the RPU, rather it walks through the functionality that the instructions of the RPU must attain. The files `init_matrices.m`, `compute_ls_matrices.m`, and `compute_var_bias.m` all execute different functionalities for the set of inputs and outputs. The only standalone file is `alg_test.m`, which runs unit and integration tests on the scripts. It plugs in test input data from the ARAIM description appendix, and checks the output of specific values to ensure compliance. To run this functional evaluation in MATLAB, run the script `alg_test(log_asm=false)`. The argument `log_asm` indicates whether to output the test data as assembly data to plug into `raim.s` for the test program.

### `alg/MATLAB-reference`

The `MATLAB-reference` directory contains code used for initial exploration of the RAIM algorithm, using test data from various papers.

## `model`

The `model` directory contains the C++ code for the RPU simulation in SystemC (installation instructions below). The simulation command is `make run ../../alg/asm/raim.bin`, which takes the memory binary file and feeds the instructions starting at `0x00400000` into the model of the MIPS processor. The code files are as follows:

* `include/cpu.h` and `src/cpu.cpp`: The SystemC module for the processor and the main datapath. It defines a virtual class `coprocessor_if` to interact with external co-processors.
* `include/fp_cop.h` and `src/fp_cop.cpp`: The SystemC module for the floating-point unit co-processor (COP1).
* `include/isa.h` and `src/isa.cpp`: Enums defining MIPS instructions to execute on the main data path and the co-processors, along with register IDs, and codes for exceptions and conditions.
* `include/memory.h` and `src/memory.cpp`: Interface with the CPU memory. Reads and parses the binary file, and translates CPU memory transactions to locations in the binary file.
* `include/queue.hpp`: Models a FIFO buffer for instruction queuing.
* `include/raim_cop.h` and `src/raim_cop.cpp`: The SystemC module for the RAIM co-processor (COP2).
* `include/system.h` and `src/system.cpp`: Utility functions to parse command line utilities and delay macros used in the modules.
* `include/sc_fault_inject.hpp` and `include/sc_trace.hpp`: SystemC simulation utilities not used in this model.

### SystemC installation for Linux

Adapted from this [guide on GitHub](https://github.com/accellera-official/systemc/blob/main/INSTALL.md). The script in this directory, `sysc_install.sh` automates this step.

1) Install support tools (gcc/g++ and make)
```
sudo apt-get update
sudo apt install gcc build-essential -y
```

2) Download and extract the SystemC library

```
# go to the directory in which to install (i.e. $HOME)
cd $HOME

# download the library
wget https://github.com/accellera-official/systemc/archive/refs/tags/2.3.4.tar.gz

# unzip
tar -xzf 2.3.4.tar.gz -C ./systemc-2.3.4 --strip-components=1
```

3) Compile and install the SystemC library
```
cd ./systemc-2.3.4
mkdir objdir
cd objdir
export CXX=g++
../configure
make
make install
cd ../
rm -rf objdir
```

4) Update environment variables

To run from any directory, set the following environment variables. Ideally, these go in the user's `~/.bashrc` file.

```
echo "export SYSTEMC_HOME=\"\$HOME/systemc-2.3.4\"" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=\"\${LD_LIBRARY_PATH}:${SYSTEMC_HOME}/lib-linux64\"" >> ~/.bashrc
```
