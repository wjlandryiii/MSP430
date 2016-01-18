/*
 * Copyright 2015 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "generated.c"


enum {
	OPMODE_UNKNOWN,
	OPMODE_REGISTER,
	OPMODE_INDEXED,
	OPMODE_SYMBOLIC,
	OPMODE_ABSOLUTE,
	OPMODE_INDIRECT_REGISTER,
	OPMODE_INDIRECT_AUTOINC,
	OPMODE_IMMEDIATE,
};

struct operand {
	int mode;
	int disp;
	int reg;
	int imm;
	int addr;
};


struct operand reg_operand(int reg){
	struct operand operand = {0};

	operand.mode = OPMODE_REGISTER;
	operand.reg = reg;
	return operand;
}

struct operand indexed_operand(int reg, int disp){
	struct operand operand = {0};

	operand.mode = OPMODE_INDEXED;
	operand.reg = reg;
	operand.disp = disp;
	return operand;
}

struct operand symbolic_operand(int disp){
	struct operand operand = {0};

	operand.mode = OPMODE_SYMBOLIC;
	operand.disp = disp;
	return operand;
}

struct operand absolute_operand(int addr){
	struct operand operand = {0};

	operand.mode = OPMODE_ABSOLUTE;
	operand.addr = addr;
	return operand;
}

struct operand indirect_register_operand(int reg){
	struct operand operand = {0};

	operand.mode = OPMODE_INDIRECT_REGISTER;
	operand.reg = reg;
	return operand;
}

struct operand indirect_autoinc_operand(int reg){
	struct operand operand = {0};

	operand.mode = OPMODE_INDIRECT_AUTOINC;
	operand.reg = reg;
	return operand;
}

struct operand immediate_operand(int imm){
	struct operand operand = {0};

	operand.mode = OPMODE_IMMEDIATE;
	return operand;
}

enum {
	OP_UNKNOW = 0,
	OP_MOV,
	OP_ADD,
	OP_ADDC,
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

struct instruction init_instruction(int operation, int operand_size, int noperands){
	struct instruction instruction;
	instruction.operation = operation;
	instruction.operand_size = operand_size;
	instruction.noperands = noperands;
	return instruction;
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


int main(int argc, char *argv[]){
	char buff[1024];
	struct instruction inst;

	inst = init_instruction(OPER_MOV, OPSIZE_8, 2);
	inst.operands[0] = reg_operand(REG_R14);
	inst.operands[1] = reg_operand(REG_R15);
	disassemble_instruction(inst, buff);
	puts(buff);

	inst = init_instruction(OPER_RRA, OPSIZE_16, 1);
	inst.operands[0] = reg_operand(REG_R15);
	disassemble_instruction(inst, buff);
	puts(buff);

	inst = init_instruction(OPER_RRA, OPSIZE_16, 1);
	inst.operands[0] = indexed_operand(REG_R15, 0x055);
	disassemble_instruction(inst, buff);
	puts(buff);

	inst = init_instruction(OPER_RRA, OPSIZE_16, 1);
	inst.operands[0] = indirect_register_operand(REG_R15);
	disassemble_instruction(inst, buff);
	puts(buff);

	inst = init_instruction(OPER_RRA, OPSIZE_16, 1);
	inst.operands[0] = indirect_autoinc_operand(REG_R15);
	disassemble_instruction(inst, buff);
	puts(buff);

	inst = init_instruction(OPER_JMP, OPSIZE_16, 1);
	inst.operands[0] = symbolic_operand(0x10);
	disassemble_instruction(inst, buff);
	puts(buff);

	return 0;
}
