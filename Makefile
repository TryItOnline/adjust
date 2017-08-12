all:
	cc -Wall -Wno-pointer-sign -std=c11 -O2 -o adjust adjust.c

clean:
	rm adjust
