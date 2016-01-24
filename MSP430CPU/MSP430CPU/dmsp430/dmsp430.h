/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#ifndef DMSP430_H

#include "generated.h"

struct operand {
	int mode;
	int reg;
	uint16_t constant;
};

struct instruction {
	int operation;
	int operand_size;
	int noperands;
	struct operand operands[2];
};

int unpack_instruction(const uint8_t *start, const uint8_t *end, struct instruction *out);
int string_for_operand(struct operand operand, char *out);
void disassemble_instruction(struct instruction inst, char *out);

#endif
