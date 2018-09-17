FLAG=-g

%.s: %.c ranc
	./ranc < $< > $@

ranc: base.c tokenizer.c parser.c ranc_main.c codegen_x86.c
	./ranc < base.c > base.s 
	./ranc < tokenizer.c > tokenizer.s 
	./ranc < parser.c > parser.s 
	./ranc < codegen_x86.c > codegen_x86.s
	./ranc < ranc_main.c > ranc_main.s 
	gcc $(FLAG) -m32 $^ -o $@

clean:
	rm *.s
  
test: ranc
	./run_tests

tokenizer: base.s tokenizer.s tokenizer_main.s
	gcc $(FLAG) -m32 $^ -o $@
 
parser: base.s parser.s parser_main.s tokenizer.s
	gcc $(FLAG) -m32 $^ -o $@

coverage:
	gcc -fno-builtin -coverage -O0 -m32 base.c tokenizer.c parser.c codegen_x86.c ranc_main.c -o ranc
	./run_tests
	gcov base.c tokenizer.c parser.c codegen_x86.c ranc_main.c

all: ranc

.PHONY: clean test coverage ranc
