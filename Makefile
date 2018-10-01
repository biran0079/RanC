FLAG=-g -m32

CC=./compile.sh

%.s: %.c ranc
	$(CC) $< > $@

ranc: base.c tokenizer.c parser.c ranc_main.c codegen_x86.c list.c string_map.c ast_processor.c
	$(CC) base.c > base.s 
	$(CC) list.c > list.s 
	$(CC) string_map.c > string_map.s 
	$(CC) tokenizer.c > tokenizer.s 
	$(CC) parser.c > parser.s 
	$(CC) codegen_x86.c > codegen_x86.s
	$(CC) ranc_main.c > ranc_main.s 
	$(CC) ast_processor.c > ast_processor.s
	$(CC) string_buffer.c > string_buffer.s
	gcc $(FLAG) base.s list.s string_map.s string_buffer.s ast_processor.s tokenizer.s parser.s codegen_x86.s ranc_main.s -o $@

clean:
	rm -f *.s *.gcno *.gcda *.gcov
  
test: ranc
	./run_tests

tokenizer: base.s tokenizer.s tokenizer_main.s list.s
	gcc $(FLAG) $^ -o $@
 
parser: base.s parser.s parser_main.s tokenizer.s list.s
	gcc $(FLAG) $^ -o $@

ranc_gcc: base.c tokenizer.c parser.c ranc_main.c codegen_x86.c list.c string_map.c string_buffer.c ast_processor.c
	gcc -fno-builtin -g -std=gnu99 -m32 base.c list.c string_map.c string_buffer.c ast_processor.c tokenizer.c parser.c codegen_x86.c ranc_main.c -o ranc_gcc

coverage: 
	gcc -fno-builtin -coverage -O0 -std=gnu99 -m32 base.c list.c string_map.c string_buffer.c ast_processor.c tokenizer.c parser.c codegen_x86.c ranc_main.c -o ranc
	./run_tests
	gcov base.c list.c string_map.c string_buffer.c ast_processor.c tokenizer.c parser.c codegen_x86.c ranc_main.c

all: ranc

.PHONY: clean test coverage ranc ranc_gcc
