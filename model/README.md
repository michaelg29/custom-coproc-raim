
# RAIM Modeling

## Running instructions

Use the following command to build the program:
```
make clean system
```

Use one of the following commands to run the program:
```
make run ASM_FILE=<ASM_FILE> [TRACE_FILE=<TRACE_FILE>] [SIM_STEP_TIME=<SIM_STEP_TIME>] [SIM_STEP_UNIT=(m|u|n|p)]
```

```
./system <ASM_FILE> [-t<TRACE_FILE>] [-s<SIM_STEP_TIME> [[(m|u|n|p)]]]
```

* `ASM_FILE`: The binary file containing the assembled code and data sections. The format must comply with the one output from the [`s2bin.c` program](../../alg/asm/README.md). Defaults to `"../../alg/asm/raim.bin"`.
* `TRACE_FILE`: Output file to which to write a VCD trace. Defaults to none, indicating no trace.
* `SIM_STEP_TIME`: Amount of time to simulate before injecting faults. Defaults to `10`.
* `SIM_STEP_UNIT`: Time unit corresponding to `SIM_STEP_TIME`. Defaults to `n`, must be one of the following values:
  * `m`: milliseconds
  * `u`: microseconds
  * `n`: nanoseconds
  * `p`: picoseconds

## Files

The folders contain code to model and simulate the RAIM system with different configurations.

### `include`

Common classes and interfaces.

### `src`

Common source code defining the functionality in the `include` classes.
