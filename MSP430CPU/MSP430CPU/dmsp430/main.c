/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "dmsp430.h"

#include "unpack.c"

#include "tests.c"



int main(int argc, char *argv[]){
	test_unpack();
	puts("");
	test_disassemble();
	return 0;
}
