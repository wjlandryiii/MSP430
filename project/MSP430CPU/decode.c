/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "decode.h"


struct msp_inst decode_instruction(uint16_t inst){
	struct msp_inst ret;

	if(inst >> 13 == 1){ // JUMP
		ret.format = FMT_JUMP;
		ret.jump.condition = (inst >> 10) & 7;
		ret.jump.offset = ((int16_t) (inst << 6)) >> 6;
	} else if(4 <= inst >> 12){
		ret.format = FMT_DOUBLE;
		ret.double_.opcode = (inst >> 12) & 0xf;
		ret.double_.src = (inst >> 8) & 0xf;
		ret.double_.ad = (inst >> 7) & 1;
		ret.double_.bw = (inst >> 6) & 1;
		ret.double_.as = (inst >> 4) & 3;
		ret.double_.dst = inst & 0xf;
	} else if(inst >> 10 == 4){
		ret.format = FMT_SINGLE;
		ret.single.opcode = (inst >> 7) & 0x1ff;
		ret.single.bw = (inst >> 6) & 1;
		ret.single.ad = (inst >> 4) & 0x03;
		ret.single.reg = inst & 0x0f;
	} else {
		ret.format = FMT_UNKNOWN;
	}
	return ret;
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

static int disassemble_single(char *out, struct single *s, uint16_t w1){
	char mnemonic[16];
	char *reg;

	strcpy(mnemonic, single_mnemonics[s->opcode & 7]);
	if(s->bw){
		strcat(mnemonic, ".b");
	}

	reg = reg_strings[s->reg];

	switch(s->ad){
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
	uint16_t w0, w1;//, w2;
	struct msp_inst inst;

	w0 = data[0] | data[1]<<8;

	inst = decode_instruction(w0);

	if(inst.format == FMT_JUMP){
		if(0 <= inst.jump.offset){
			sprintf(out, "%-8s #0x%x",
				jmp_mnemonics[inst.jump.condition],
				(inst.jump.offset * 2) & 0xffff);
		} else {
			sprintf(out, "%-8s #-0x%x",
				jmp_mnemonics[inst.jump.condition],
				((0 - inst.jump.offset) * 2) & 0xffff);
		}
		return 1;
	} else if(inst.format == FMT_DOUBLE){




		return 0;
	} else if(inst.format == FMT_SINGLE){
		struct single *s;

		s = &inst.single;
		if((s->opcode & 7) < 7){
			if((s->opcode & 7) != 6){
				if((s->opcode & 1) == 0){
					if(s->ad != 1){
						return disassemble_single(out, s, 0);
					} else {
						w1 = data[2] | data[3] << 8;
						return disassemble_single(out, s, w1);
					}
				} else {
					if(s->bw == 0){
						if(s->ad != 1){
							return disassemble_single(out, s, 0);
						} else {
							w1 = data[2] | data[3] << 8;
							return disassemble_single(out, s, w1);
						}
					} else {
						return 0;
					}
				}
			} else {
				if(s->bw == 0 && s->ad == 0 && s->reg == 0){
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

/*

int main(int argc, char *argv[]){
	uint8_t inst[16];
	char out[1024];

	inst[0] = 0xff; inst[1] = 0x3f;
	inst[0] = 0x8f; inst[1] = 0x10;
	inst[0] = 0x4f; inst[1] = 0x12;
	inst[0] = 0x9f; inst[1] = 0x10; inst[2] = 0x4; inst[3] = 0x00;
	inst[0] = 0x4f; inst[1] = 0x0f;

	if(0 < disassemble_single_instruction_simple((uint8_t *)inst, out)){
		puts(out);
		return 0;
	} else {
		printf("didn't disassemble\n");
		return 1;
	}
}
*/