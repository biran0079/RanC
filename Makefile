%.s: %.c ranc
	./ranc < $< > $@

ranc: ran.s base.s
	gcc -m32 ran.s base.s -o ranc

clean:
	rm *.s
  
test: ranc
	./run_tests

tokenizer_main: base.s tokenizer.s tokenizer_main.s
	gcc -m32 tokenizer_main.s tokenizer.s base.s -o ranc
 
coverage:
	gcc -fno-builtin -coverage -O0 ran.c base.c -o ranc
	./run_tests
	gcov ran.c base.c

all: ranc
