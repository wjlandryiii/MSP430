all: main tests


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

tests.o: tests.c
	gcc -g -c -o $@ $<

tests: tests.o dmsp430.o generated.o
	gcc -g -o $@ $^

.PHONY: clean

clean:
	rm -f *.o
	rm -f tests
	rm -f main
