all: main


generated.%: generate.py
	python generate.py

generated.o: generated.c generated.h
	gcc -g -c -o $@ $<

dmsp430.o: dmsp430.c dmsp430.h generated.h
	gcc -g -c -o $@ $<

main.o: main.c dmsp430.h generated.h
	gcc -g -c -o $@ $<

main: main.o dmsp430.o generated.o
	gcc -g -o $@ $^
