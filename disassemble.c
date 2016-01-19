/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "generated.c"




static struct operand reg_operand(int reg){
	struct operand operand = {0};

	operand.mode = OPMODE_REGISTER;
	operand.reg = reg;
	return operand;
}

static struct operand indexed_operand(int reg, int disp){
	struct operand operand = {0};

	operand.mode = OPMODE_INDEXED;
	operand.reg = reg;
	operand.disp = disp;
	return operand;
}

static struct operand symbolic_operand(int disp){
	struct operand operand = {0};

	operand.mode = OPMODE_SYMBOLIC;
	operand.disp = disp;
	return operand;
}

static struct operand absolute_operand(int addr){
	struct operand operand = {0};

	operand.mode = OPMODE_ABSOLUTE;
	operand.addr = addr;
	return operand;
}

static struct operand indirect_register_operand(int reg){
	struct operand operand = {0};

	operand.mode = OPMODE_INDIRECT_REGISTER;
	operand.reg = reg;
	return operand;
}

static struct operand indirect_autoinc_operand(int reg){
	struct operand operand = {0};

	operand.mode = OPMODE_INDIRECT_AUTOINC;
	operand.reg = reg;
	return operand;
}

static struct operand immediate_operand(int imm){
	struct operand operand = {0};

	operand.mode = OPMODE_IMMEDIATE;
	operand.imm = imm;
	return operand;
}

static struct operand jump_operand(int offset){
	struct operand operand = {0};

	operand.mode = OPMODE_JUMP;
	operand.offset = offset;
	return operand;
}




int string_for_operand(struct operand operand, char *out){
	char *reg_str;

	switch(operand.mode){
	case OPMODE_REGISTER:
		reg_str = lookup_reg_string(operand.reg);
		strcpy(out, reg_str);
		return 0;
	case OPMODE_INDEXED:
		reg_str = lookup_reg_string(operand.reg);
		sprintf(out, "0x%x(%s)", operand.disp, reg_str);
		return 0;
	case OPMODE_SYMBOLIC:
		reg_str = lookup_reg_string(REG_PC);
		sprintf(out, "0x%x(%s)", operand.disp, reg_str);
		return 0;
	case OPMODE_ABSOLUTE:
		sprintf(out, "&0x%x", operand.addr);
		return 0;
	case OPMODE_INDIRECT_REGISTER:
		reg_str = lookup_reg_string(operand.reg);
		sprintf(out, "@%s", reg_str);
		return 0;
	case OPMODE_INDIRECT_AUTOINC:
		reg_str = lookup_reg_string(operand.reg);
		sprintf(out, "@%s+", reg_str);
		return 0;
	case OPMODE_IMMEDIATE:
		sprintf(out, "#0x%x", operand.imm);
		return 0;
	case OPMODE_JUMP:
		if(0 <= operand.offset){
			sprintf(out, "$+0x%x", operand.offset);
		} else {
			sprintf(out, "$-0x%x", -operand.offset);
		}
		return 0;
	default:
		out[0] = 0;
		return -1;
	}
}

void disassemble_instruction(struct instruction inst, char *out){
	char mnemonic[16];
	char operand[2][16];
	int i;

	strcpy(mnemonic, lookup_mnemonic_for_operation(inst.operation));
	if(inst.operand_size == OPSIZE_8){
		strcat(mnemonic, ".b");
	}

	for(i = 0; i < inst.noperands; i++){
		string_for_operand(inst.operands[i], operand[i]);
	}

	if(inst.noperands == 0){
		strcpy(out, mnemonic);
	} else {
		sprintf(out, "%-8s ", mnemonic);
		for(i = 0; i < inst.noperands; i++){
			strcat(out, operand[i]);
			if(i + 1 != inst.noperands){
				strcat(out, ", ");
			}
		}
	}
}
