build: ran.c
	./ranc < ran.c > ran.s
	gcc -m32 ran.s -o ranc

clean:
	rm ran.s
  
all: build
