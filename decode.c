/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

struct instruction {
	int operation;
	int operand_size;
	int noperands;
	struct operand operands[2];
};

struct instruction double_instruction(int operation, int operand_size,
		struct operand src, struct operand dst){
	struct instruction instruction = {0};

	instruction.operation = operation;
	instruction.operand_size = operand_size;
	instruction.noperands = 2;
	instruction.operands[0] = src;
	instruction.operands[1] = dst;
	return instruction;
}

struct instruction single_instruction(int operation, int operand_size, struct operand operand){
	struct instruction instruction = {0};

	instruction.operation = operation;
	instruction.operand_size = operand_size;
	instruction.noperands = 1;
	instruction.operands[0] = operand;
	return instruction;
}




/*
struct unpacked_inst {
	int format;

	union {
		struct unpacked_single {
			int opcode;
			int bw;
			int ad;
			int reg;
		} unpacked_single;

		struct unpacked_double {
			int opcode;
			int src;
			int ad;
			int bw;
			int as;
			int dst;
		} unpacked_double;

		struct jump {
			int condition;
			int offset;
		} jump;
	};
};
*/


struct unpacked_inst {
	int format;
	int opcode;
	int ad;
	int bw;
	int as;
	int reg0;
	int reg1;
	int jmp_condition;
	int jmp_offset;
};

enum {
	FMT_UNKNOWN = 0,
	FMT_DOUBLE,
	FMT_SINGLE,
	FMT_JUMP,
};

struct unpacked_inst unpack_instruction(uint16_t w0){
	struct unpacked_inst ret;

	uint16_t prefix;

	prefix = (w0 >> 12) & 0xf;

	if(prefix == 1 && (w0 & 0xc00) == 0){
		ret.format = FMT_SINGLE;
		ret.opcode = (w0 >> 7) & 0x1ff;
		ret.bw = (w0 >> 6) & 1;
		ret.ad = (w0 >> 4) & 0x03;
		ret.reg0 = w0 & 0x0f;
	} else if(prefix == 2 || prefix == 3){
		ret.format = FMT_JUMP;
		ret.opcode = (w0 >> 13) & 7;
		ret.jmp_condition = (w0 >> 10) & 7;
		ret.jmp_offset = ((int16_t) (w0 << 6)) >> 6;
	} else if(4 <= prefix && prefix <= 15){
		ret.format = FMT_DOUBLE;
		ret.opcode = (w0 >> 12) & 0xf;
		ret.reg0 = (w0 >> 8) & 0xf;
		ret.ad = (w0 >> 7) & 1;
		ret.bw = (w0 >> 6) & 1;
		ret.as = (w0 >> 4) & 3;
		ret.reg1 = w0 & 0xf;
	} else {
		ret.format = FMT_UNKNOWN;
	}
	return ret;
}


struct instruction unknown_instruction(){
	struct instruction unknown = {0};
	return unknown;
}


struct instruction decode_instruction(struct unpacked_inst w0, uint16_t w1, uint16_t w2){
	if(w0.format == FMT_DOUBLE){

	} else if(w0.format == FMT_SINGLE){
		if(w0.opcode == 0 || w0.opcode == 2 ||  w0.opcode == 4){

		} else if(w0.opcode == 1 || w0.opcode == 3 || w0.opcode == 5){
			if(w0.bw == 0){
				
			} else {
				return unknown_instruction();
			}
		} else if(w0.opcode == 6){

		} else {
			return unknown_instruction();
		}
	} else if(w0.format == FMT_JUMP){
	} else {
		return unknown_instruction();
	}
	return unknown_instruction();
}


static char *jmp_mnemonics[] = {
	"jne",
	"jeq",
	"jnc",
	"jc",
	"jn",
	"jge",
	"jl",
	"jmp",
};

static char *single_mnemonics[] = {
	"rrc",
	"swpb",
	"rra",
	"sxt",
	"push",
	"call",
	"reti",
};

static char *double_mnemonics[] = {
	"mov",
	"add",
	"addc",
	"subc",
	"sub",
	"cmp",
	"dadd",
	"bit",
	"bic",
	"bis",
	"xor",
	"and",
};

static char *reg_strings[] = {
	"pc",
	"sp",
	"sr",
	"r3",
	"r4",
	"r5",
	"r6",
	"r7",
	"r8",
	"r9",
	"r10",
	"r11",
	"r12",
	"r13",
	"r14",
	"r15",
};

static int disassemble_unpacked_single(char *out, struct unpacked_inst *inst, uint16_t w1){
	char mnemonic[16];
	char *reg;

	strcpy(mnemonic, single_mnemonics[inst->opcode & 7]);
	if(inst->bw){
		strcat(mnemonic, ".b");
	}

	reg = reg_strings[inst->reg0];

	switch(inst->ad){
	case 0:
		sprintf(out, "%-8s %s", mnemonic, reg);
		return 1;
	case 1:
		if(0 <= (int16_t)w1){
			sprintf(out, "%-8s 0x%x(%s)", mnemonic, w1, reg);
		} else {
			sprintf(out, "%-8s -0x%x(%s)", mnemonic, w1, reg);
		}
		return 2;
	case 2:
		sprintf(out, "%-8s @%s", mnemonic, reg);
		return 1;
	case 3:
		sprintf(out, "%-8s @%s+", mnemonic, reg);
		return 1;
	default:
		return 0;
	}
}

/*
char *double_mnemonics[] = {

static int disassemble_double(char *out, struct double_ *d, uint16_t w1){
	char *src_reg, *dst_reg;
	char op1[64];
	char op2[64];


	//mnemonic = 
		return 0;

}
*/
int disassemble_single_instruction_simple(uint8_t *data, char *out){
	uint16_t w0, w1, w2;
	struct unpacked_inst inst;

	w0 = data[0] | data[1]<<8;

	inst = unpack_instruction(w0);

	if(inst.format == FMT_JUMP){
		if(0 <= inst.jmp_offset){
			sprintf(out, "%-8s #0x%x",
				jmp_mnemonics[inst.jmp_condition],
				(inst.jmp_offset * 2) & 0xffff);
		} else {
			sprintf(out, "%-8s #-0x%x",
				jmp_mnemonics[inst.jmp_condition],
				((0 - inst.jmp_offset) * 2) & 0xffff);
		}
		return 1;
	} else if(inst.format == FMT_DOUBLE){




		return 0;
	} else if(inst.format == FMT_SINGLE){

		if((inst.opcode & 7) < 7){
			if((inst.opcode & 7) != 6){
				if((inst.opcode & 1) == 0){
					if(inst.ad != 1){
						return disassemble_unpacked_single(out, &inst, 0);
					} else {
						w1 = data[2] | data[3] << 8;
						return disassemble_unpacked_single(out, &inst, w1);
					}
				} else {
					if(inst.bw == 0){
						if(inst.ad != 1){
							return disassemble_unpacked_single(out, &inst, 0);
						} else {
							w1 = data[2] | data[3] << 8;
							return disassemble_unpacked_single(out, &inst, w1);
						}
					} else {
						return 0;
					}
				}
			} else {
				if(inst.bw == 0 && inst.ad == 0 && inst.reg0 == 0){
					strcpy(out, "reti");
					return 1;
				} else {
					return 0;
				}
			}
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

unsigned char fake_bin[] = {
	0x0e, 0x4f, 0x0f, 0x43, 0x0b, 0x3c, 0x6d, 0x4e, 0x8d, 0x11, 0x0d, 0x5f,
	0x0f, 0x4d, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f,
	0x0f, 0x8d, 0x1e, 0x53, 0xce, 0x93, 0x00, 0x00, 0xf2, 0x23, 0x30, 0x41,
	0x0b, 0x12, 0x0a, 0x12, 0x09, 0x12, 0x0b, 0x4f, 0x0a, 0x4e, 0x09, 0x4d,
	0x1e, 0x4f
};

int test1(){
	uint8_t inst[16];
	char out[1024];

	inst[0] = 0xff; inst[1] = 0x3f;
	inst[0] = 0x8f; inst[1] = 0x10;
	inst[0] = 0x4f; inst[1] = 0x12;
	inst[0] = 0x9f; inst[1] = 0x10; inst[2] = 0x4; inst[3] = 0x00;
	inst[0] = 0x4f; inst[1] = 0x0f;
	inst[0] = 0x2f; inst[1] = 0x83;

	if(0 < disassemble_single_instruction_simple((uint8_t *)inst, out)){
		puts(out);
		return 0;
	} else {
		printf("didn't disassemble\n");
		return 1;
	}
}


void dumb(){


}


int main(int argc, char *argv[]){

}
