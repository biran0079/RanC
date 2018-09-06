# RanC [![Build Status](https://travis-ci.org/biran0079/RanC.svg?branch=master)](https://travis-ci.org/biran0079/RanC) [![codecov](https://codecov.io/gh/biran0079/RanC/branch/master/graph/badge.svg)](https://codecov.io/gh/biran0079/RanC)
A self-hosting compiler for a subset of C. 

## How to use
```bash
$ gcc ran.c -o ranc              # build compiler
$ ./ranc < input.c > output.s    # compile C code to assembly
$ gcc -m32 output.s -o output    # use gcc to assemble and link into binary executable
$ ./output                       # run your program
```
## How to test
```bash
$ ./run_test
```
## Acknowledge
Many ideas are borrowed from https://github.com/Fedjmike/mini-c.
