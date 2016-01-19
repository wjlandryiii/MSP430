all: main


generated.%: generate.py
	python generate.py

generated.o: generated.c generated.h
	gcc -c -o $@ $<

dmsp430.o: dmsp430.c dmsp430.h generated.h
	gcc -c -o $@ $<

main.o: main.c dmsp430.h generated.h
	gcc -c -o $@ $<

main: main.o dmsp430.o generated.o
	gcc -o $@ $^
