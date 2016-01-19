all: main

decode: decode.c
	gcc -g -o $@ $<

disassemble: disassemble.c
	gcc -g -o $@ $<

main: main.c
	gcc -g -o $@ $<
