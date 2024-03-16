
# RAIM Assembly Code

This directory contains the Assembly files for the algorithm along with the code to convert PCSpim log files into binary files readable in the SystemC model.

## `s2bin.c`

This is the main source file.

### Running instructions

To run the code, use the following Makefile command. If Make is not installed, use the GCC commands below that.

```
make clean run LOG_FILE=<LOG_FILE> OUT_FILE=<OUT_FILE>
```

```
gcc s2bin.c -o system
./system <LOG_FILE> <OUT_FILE>
```

* `<LOG_FILE>` is the file output from PCSpim. PCSpim is not directly required, but it outputs it in the appropriate format. See the [specifications below](#log-file-format).
* `<OUT_FILE>` is the file to which to write the memory binary. The main SystemC program then uses this file to run the RAIM algorithm. See the [running instructions](../../model/README.md).

## Log file format
