build: ran.c
	./bootstrap

clean:
	rm ran.s
  
test: build
	./run_tests
 
coverage:
	gcc -fno-builtin -coverage -O0 ran.c -o ranc
	./ranc < ran.c > /dev/null
	gcov ran.c

all: build
