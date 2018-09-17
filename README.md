# RanC [![Build Status](https://travis-ci.org/biran0079/RanC.svg?branch=master)](https://travis-ci.org/biran0079/RanC) [![codecov](https://codecov.io/gh/biran0079/RanC/branch/master/graph/badge.svg)](https://codecov.io/gh/biran0079/RanC)
A self-hosting compiler for a subset of C. 

## How to use
```bash
$ make ranc                      # bootstrap ranc
$ ./ranc > a.s <<EOF             
void printf();
int main() {
  printf("hello world\n");
  return 0;
}
EOF                              # compile C code to assembly
$ gcc -m32 a.s                   # use gcc to assemble and link into binary executable
$ ./a.out                        # run your program
```
## How to test
```bash
$ make test
```
## Acknowledge
Many ideas are borrowed from https://github.com/Fedjmike/mini-c.
