/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dmsp430.h"


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


static int is_format_single(uint16_t w0){
	if((w0 & 0xf800) == 0x1000){
		return 1;
	} else {
		return 0;
	}
}

static int is_format_double(uint16_t w0){
	if(0x4000 <= (w0 & 0xf000)){
		return 1;
	} else {
		return 0;
	}
}

static int is_format_jump(uint16_t w0){
	if((w0 & 0xe000) == 0x2000){
		return 1;
	} else {
		return 0;
	}
}


static int is_bw_set(uint16_t w0){
	if((w0 & 0x0040) == 0x0040){
		return 1;
	} else {
		return 0;
	}
}

static int ad_for_single(uint16_t w0){
	return (w0 & 0x0030) >> 4;
}

static int reg_for_single(uint16_t w0){
	return (w0 & 0x000f);
}


int unpack_instruction(const uint8_t *start, const uint8_t *end, struct instruction *out){
	uint16_t w0, w1;

	struct instruction inst = {0};
	const uint8_t *p;
	int len;

	p = start;

	if(end < p+2){ return -1; } // ran out of input
	w0 = p[0] | p[1] << 8;
	p += 2;
	len = 2;

	if(is_format_single(w0)){
		uint8_t opcode = (w0 & 0x380) >> 7;
		int bw, ad, reg;

		bw = is_bw_set(w0);
		ad = ad_for_single(w0);
		reg = reg_for_single(w0);

		if(opcode < 6){
			switch(opcode){
			case 0:
				inst.operation = OPER_RRC;
				if(is_bw_set(w0)){
					inst.operand_size = OPSIZE_8;
				} else {
					inst.operand_size = OPSIZE_16;
				}
				break;
			case 1:
				inst.operation = OPER_SWPB;
				if(is_bw_set(w0)){
					return 0;
				} else {
					inst.operand_size = OPSIZE_16;
				}
				break;
			case 2:
				inst.operation = OPER_RRA;
				if(is_bw_set(w0)){
					inst.operand_size = OPSIZE_8;
				} else {
					inst.operand_size = OPSIZE_16;
				}
				break;
			case 3:
				inst.operation = OPER_SXT;
				if(is_bw_set(w0)){
					return 0;
				} else {
					inst.operand_size = OPSIZE_16;
				}
				break;
			case 4:
				inst.operation = OPER_PUSH;
				if(is_bw_set(w0)){
					inst.operand_size = OPSIZE_8;
				} else {
					inst.operand_size = OPSIZE_16;
				}
				break;
			case 5:
				inst.operation = OPER_CALL;
				if(is_bw_set(w0)){
					return 0;
				} else {
					inst.operand_size = OPSIZE_16;
				}
				break;
			default:
				return 0;
			}


			if(ad == 0){
				inst.noperands = 1;
				inst.operands[0] = reg_operand(reg+1);
				*out = inst;
				return len;
			} else if(ad == 1){
				if(end < p+2){
					return 0;
				}
				w1 = p[0] | p[1] << 8;
				p += 2;
				len += 2;
				inst.noperands = 1;
				if(reg == 0){
					inst.operands[0] = symbolic_operand(w1);
				} else {
					inst.operands[0] = indexed_operand(reg+1, w1);
				}
				*out = inst;
				return len;
			} else if(ad == 2){
				inst.noperands = 1;
				inst.operands[0] = indirect_register_operand(reg+1);
				*out = inst;
				return len;
			} else if(ad == 3){
				if(reg != 0){
					inst.noperands = 1;
					inst.operands[0] = indirect_autoinc_operand(reg+1);
					*out = inst;
					return len;
				} else {
					if(end < p+2){
						return 0;
					}
					w1 = p[0] | p[1] << 8;
					p += 2;
					len += 2;
					inst.noperands = 1;
					inst.operands[0] = immediate_operand(w1);
					*out = inst;
					return len;
				}
			} else {
				// never
				return 0;
			}
		} else {
			if(bw == 0 && ad == 0 && reg == 0){
				inst.operation = OPER_RETI;
				inst.noperands = 0;
				*out = inst;
				return len;
			} else {
				return 0;
			}
		}
	} else if(is_format_jump(w0)){
		int condition;
		int offset;

		condition = (w0 & 0x1c00) >> 10;
		offset = ((int16_t) ((w0 & 0x3ff) << 6)) >> 6;

		switch(condition){
			case 0: inst.operation = OPER_JNE; break;
			case 1: inst.operation = OPER_JEQ; break;
			case 2: inst.operation = OPER_JNC; break;
			case 3: inst.operation = OPER_JC;  break;
			case 4: inst.operation = OPER_JNE; break;
			case 5: inst.operation = OPER_JGE; break;
			case 6: inst.operation = OPER_JL;  break;
			case 7: inst.operation = OPER_JMP; break;
			default: return 0;
		}

		inst.noperands = 1;
		inst.operands[0] = jump_operand(offset * 2 + 2);
		*out = inst;
		return len;
	} else if(is_format_double(w0)){
		int opcode;
		int src, as;
		int dst, ad;
		int bw;
		int ate_w1 = 0;

		opcode = (w0 & 0xf000) >> 12;
		src = (w0 & 0x0f00) >> 8;
		ad = (w0 & 0x0080) >> 7;
		bw = (w0 & 0x0040) >> 6;
		as = (w0 & 0x0030) >> 4;
		dst = (w0 & 0x000f);

		switch(opcode){
			case 0x4: inst.operation = OPER_MOV;  break;
			case 0x5: inst.operation = OPER_ADD;  break;
			case 0x6: inst.operation = OPER_ADDC; break;
			case 0x7: inst.operation = OPER_SUBC; break;
			case 0x8: inst.operation = OPER_SUB;  break;
			case 0x9: inst.operation = OPER_CMP;  break;
			case 0xa: inst.operation = OPER_DADD; break;
			case 0xb: inst.operation = OPER_BIT;  break;
			case 0xc: inst.operation = OPER_BIC;  break;
			case 0xd: inst.operation = OPER_BIS;  break;
			case 0xe: inst.operation = OPER_XOR;  break;
			case 0xf: inst.operation = OPER_AND;  break;
			default: return 0;
		}
		if(bw == 0){
			inst.operand_size = OPSIZE_16;
		} else {
			inst.operand_size = OPSIZE_8;
		}

		if(as == 0){
			if(src != 3){
				inst.noperands = 2;
				inst.operands[0] = reg_operand(src+1);
			} else {
				inst.noperands = 2;
				inst.operands[0] = immediate_operand(0);
			}
		} else if(as == 1){
			if(src == 2){
				inst.noperands = 2;
				if(end < p+2){
					return 0;
				}
				w1 = p[0] | p[1] << 8;
				p += 2;
				len += 2;
				ate_w1 = 1;
				inst.operands[0] = absolute_operand(w1);
			} else if(src == 3){
				inst.noperands = 2;
				inst.operands[0] = immediate_operand(1);
			} else {
				inst.noperands = 2;
				if(end < p+2){
					return 0;
				}
				w1 = p[0] | p[1] << 8;
				p += 2;
				len += 2;
				ate_w1 = 1;
				inst.operands[0] = indexed_operand(src+1, w1);
			}
		} else if(as == 2){
			if(src == 2){
				inst.noperands = 2;
				inst.operands[0] = immediate_operand(4);
			} else if(src == 3){
				inst.noperands = 2;
				inst.operands[0] = immediate_operand(2);
			} else {
				inst.noperands = 2;
				inst.operands[0] = indirect_register_operand(src+1);
			}
		} else if(as == 3){
			if(src == 0){
				if(end < p+2){
					return 0;
				}
				w1 = p[0] | p[1] << 8;
				p += 2;
				len += 2;
				ate_w1 = 0;
				inst.noperands = 2;
				inst.operands[0] = immediate_operand(w1);
			} else if(src == 2){
				inst.noperands = 2;
				inst.operands[0] = immediate_operand(8);
			} else if(src == 3){
				inst.noperands = 2;
				inst.operands[0] = immediate_operand(-1);
			} else {
				inst.noperands = 2;
				inst.operands[0] = indirect_autoinc_operand(src+1);
			}
		} else {
			return 0;
		}

		if(ad == 0){
			inst.noperands = 2;
			inst.operands[1] = reg_operand(dst+1);
		} else {
			inst.noperands = 2;
			if(end < p+2){
				return 0;
			}
			w1 = p[0] | p[1] << 8;
			p += 2;
			len += 2;

			if(dst == 2){
				inst.operands[1] = absolute_operand(w1);
			} else {
				inst.operands[1] = indexed_operand(dst+1, w1);
			}
		}

		// EMULATED INSTRUCTION FIXUPS
		//
		if(inst.operation == OPER_MOV
				&& inst.operands[0].mode == OPMODE_IMMEDIATE
				&& inst.operands[1].mode == OPMODE_REGISTER
				&& inst.operands[1].reg == REG_CG){
			inst.operation = OPER_NOP;
			inst.noperands = 0;
		}
		if(inst.operation == OPER_MOV
				&& inst.operands[1].mode == OPMODE_REGISTER
				&& inst.operands[1].reg == REG_PC){
			if(inst.operands[0].mode == OPMODE_INDIRECT_AUTOINC && inst.operands[0].reg == REG_SP){
				inst.operation = OPER_RET;
				inst.noperands = 0;
			} else {
				inst.operation = OPER_BR;
				inst.noperands = 1;
			}
		}
		if(inst.operation == OPER_MOV
				&& inst.operands[0].mode == OPMODE_INDIRECT_AUTOINC
				&& inst.operands[0].reg == REG_SP){
			inst.operation = OPER_POP;
			inst.noperands = 1;
			inst.operands[0] = inst.operands[1];
		}

		*out = inst;
		return len;

	} else {
		// invalid instruction
		return -1;
	}

	return 0;
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
		sprintf(out, "0x%x(%s)", operand.disp & 0xffff, reg_str);
		return 0;
	case OPMODE_SYMBOLIC:
		reg_str = lookup_reg_string(REG_PC);
		sprintf(out, "0x%x(%s)", operand.disp & 0xffff, reg_str);
		return 0;
	case OPMODE_ABSOLUTE:
		sprintf(out, "&0x%x", operand.addr & 0xffff);
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
		sprintf(out, "#0x%x", operand.imm & 0xffff);
		return 0;
	case OPMODE_JUMP:
		if(0 <= operand.offset){
			sprintf(out, "$+0x%x", operand.offset & 0xffff);
		} else {
			sprintf(out, "$-0x%x", (-operand.offset) & 0xffff);
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
