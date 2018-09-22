FLAG=-g

%.s: %.c ranc
	./compile.sh $< > $@

ranc: base.c tokenizer.c parser.c ranc_main.c codegen_x86.c
	gcc -E base.c | ./ranc > base.s 
	./compile.sh tokenizer.c > tokenizer.s 
	./compile.sh parser.c > parser.s 
	./compile.sh codegen_x86.c > codegen_x86.s
	./compile.sh ranc_main.c > ranc_main.s 
	gcc $(FLAG) -m32 base.s tokenizer.s parser.s codegen_x86.s ranc_main.s -o $@

clean:
	rm -f *.s *.gcno *.gcda *.gcov
  
test: ranc
	./run_tests

tokenizer: base.s tokenizer.s tokenizer_main.s
	gcc $(FLAG) -m32 $^ -o $@
 
parser: base.s parser.s parser_main.s tokenizer.s
	gcc $(FLAG) -m32 $^ -o $@

coverage:
	gcc -fno-builtin -coverage -O0 -std=gnu99 -m32 base.c tokenizer.c parser.c codegen_x86.c ranc_main.c -o ranc
	./run_tests
	gcov base.c tokenizer.c parser.c codegen_x86.c ranc_main.c

all: ranc

.PHONY: clean test coverage ranc
