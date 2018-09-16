build: ran.c base.c
	./bootstrap

clean:
	rm ran.s
  
test: build
	./run_tests
 
coverage:
	gcc -fno-builtin -coverage -O0 ran.c base.c -o ranc
	./run_tests
	gcov ran.c

all: build
