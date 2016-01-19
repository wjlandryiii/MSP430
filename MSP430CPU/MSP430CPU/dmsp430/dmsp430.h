/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#ifndef DMSP430_H

#include "generated.h"

enum {
	OPMODE_UNKNOWN,
	OPMODE_REGISTER,
	OPMODE_INDEXED,
	OPMODE_SYMBOLIC,
	OPMODE_ABSOLUTE,
	OPMODE_INDIRECT_REGISTER,
	OPMODE_INDIRECT_AUTOINC,
	OPMODE_IMMEDIATE,
	OPMODE_JUMP,
};

struct operand {
	int mode;
	int disp;
	int reg;
	int imm;
	int addr;
	int offset;
};

enum {
	OPSIZE_UNKNOWN,
	OPSIZE_8,
	OPSIZE_16,
};

struct instruction {
	int operation;
	int operand_size;
	int noperands;
	struct operand operands[2];
};

int unpack_instruction(uint8_t *start, uint8_t *end, struct instruction *out);
int string_for_operand(struct operand operand, char *out);
void disassemble_instruction(struct instruction inst, char *out);

#endif
