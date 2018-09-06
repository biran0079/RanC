build: ran.c
	./bootstrap

clean:
	rm ran.s
  
test: build
	./run_tests

all: build
