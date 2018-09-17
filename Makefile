FLAG=-g

%.s: %.c ranc
	./ranc < $< > $@

ranc: ran.c base.c
	./ranc < ran.c > ran.s 
	./ranc < base.c > base.s 
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
	gcc -fno-builtin -coverage -O0 ran.c base.c -o ranc
	./run_tests
	gcov ran.c base.c

all: ranc
