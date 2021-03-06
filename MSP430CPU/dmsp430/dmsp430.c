/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "dmsp430.h"

static int reg_const_for_encoded(int reg){
	return reg + 1;
}

enum {
	MODE_UNKNOWN = 0,
	MODE_REGISTER_DIRECT,
	MODE_INDEXED,
	MODE_SYMBOLIC,
	MODE_ABSOLUTE,
	MODE_INDIRECT_REGISTER,
	MODE_INDIRECT_AUTOINC,
	MODE_IMMEDIATE,
	MODE_IMMEDIATE_0000,
	MODE_IMMEDIATE_0001,
	MODE_IMMEDIATE_0002,
	MODE_IMMEDIATE_0004,
	MODE_IMMEDIATE_0008,
	MODE_IMMEDIATE_ffff,
};

static int double_src_operand_mode(int as, int src){
	if(as == 0){
		if(src == 2){
			return MODE_REGISTER_DIRECT;
		} else if(src == 3){
			return MODE_IMMEDIATE_0000;
		} else {
			return MODE_REGISTER_DIRECT;
		}
	} else if(as == 1){
		if(src == 0){
			return MODE_SYMBOLIC;
		} else if(src == 2){
			return MODE_ABSOLUTE;
		} else if(src == 3){
			return MODE_IMMEDIATE_0001;
		} else {
			return MODE_INDEXED;
		}
	} else if(as == 2){
		if(src == 2){
			return MODE_IMMEDIATE_0004;
		} else if(src == 3){
			return MODE_IMMEDIATE_0002;
		} else {
			return MODE_INDIRECT_REGISTER;
		}
	} else if(as == 3){
		if(src == 0){
			return MODE_IMMEDIATE;
		} else if(src == 2){
			return MODE_IMMEDIATE_0008;
		} else if(src == 3){
			return MODE_IMMEDIATE_ffff;
		} else {
			return MODE_INDIRECT_AUTOINC;
		}
	} else {
		return MODE_UNKNOWN;
	}
}

static int double_dst_operand_mode(int ad, int dst){
	if(ad == 0){
		return MODE_REGISTER_DIRECT;
	} else if(ad == 1){
		if(dst == 0){
			return MODE_SYMBOLIC;
		} else if(dst == 2){
			return MODE_ABSOLUTE;
		} else {
			return MODE_INDEXED;
		}
	} else {
		return MODE_UNKNOWN;
	}
}

enum {
	FMT_UNKNOWN = 0,
	FMT_SINGLE,
	FMT_DOUBLE,
	FMT_JUMP,
};

static int format_for_instruction(uint16_t w0){
	if((w0 & 0xf800) == 0x1000){
		return FMT_SINGLE;
	} else if(0x4000 <= (w0 & 0xf000)){
		return FMT_DOUBLE;
	} else if((w0 & 0xe000) == 0x2000){
		return FMT_JUMP;
	} else {
		return FMT_UNKNOWN;
	}
}

static int bits(uint16_t word, int msb, int lsb){
	uint16_t mask;
	int nbits;

	nbits = (msb - lsb) + 1;

	mask = (1 << nbits) - 1;

	return (word >> lsb) & mask;
}

static void unpack_single(uint16_t w0, int *opcode, int *bw, int *ad, int *reg){
	*opcode = bits(w0, 15, 7);
	*bw     = bits(w0,  6, 6);
	*ad     = bits(w0,  5, 4);
	*reg    = bits(w0,  3, 0);
}

static void unpack_double(uint16_t w0, int *opcode, int *src, int *ad, int *bw, int *as, int *dst){
	*opcode = bits(w0, 15, 12);
	*src    = bits(w0, 11,  8);
	*ad     = bits(w0,  7,  7);
	*bw     = bits(w0,  6,  6);
	*as     = bits(w0,  5,  4);
	*dst    = bits(w0,  3,  0);
}

static void unpack_jump(uint16_t w0, int *opcode, int *condition, int *offset){
	*opcode    = bits(w0, 15, 13);
	*condition = bits(w0, 12, 10);
	*offset    = bits(w0,  9,  0);
}

static int decode_first_operand(int mode, int reg, int w1, struct operand *out){
	struct operand operand = {0};

	switch(mode){
	case MODE_REGISTER_DIRECT:
		operand.mode = OPMODE_REGISTER;
		operand.reg = reg_const_for_encoded(reg);
		break;
	case MODE_INDEXED:
		operand.mode = OPMODE_INDEXED;
		operand.reg = reg_const_for_encoded(reg);
		operand.constant = w1;
		break;
	case MODE_SYMBOLIC:
		operand.mode = OPMODE_SYMBOLIC;
		operand.constant = w1;
		break;
	case MODE_ABSOLUTE:
		operand.mode = OPMODE_ABSOLUTE;
		operand.constant = w1;
		break;
	case MODE_INDIRECT_REGISTER:
		operand.mode = OPMODE_INDIRECT_REGISTER;
		operand.reg = reg_const_for_encoded(reg);
		break;
	case MODE_INDIRECT_AUTOINC:
		operand.mode = OPMODE_INDIRECT_AUTOINC;
		operand.reg = reg_const_for_encoded(reg);
		break;
	case MODE_IMMEDIATE:
		operand.mode = OPMODE_IMMEDIATE;
		operand.constant = w1;
		break;
	case MODE_IMMEDIATE_0000:
		operand.mode = OPMODE_IMMEDIATE;
		operand.constant = 0;
		break;
	case MODE_IMMEDIATE_0001:
		operand.mode = OPMODE_IMMEDIATE;
		operand.constant = 1;
		break;
	case MODE_IMMEDIATE_0002:
		operand.mode = OPMODE_IMMEDIATE;
		operand.constant = 2;
		break;
	case MODE_IMMEDIATE_0004:
		operand.mode = OPMODE_IMMEDIATE;
		operand.constant = 4;
		break;
	case MODE_IMMEDIATE_0008:
		operand.mode = OPMODE_IMMEDIATE;
		operand.constant = 8;
		break;
	case MODE_IMMEDIATE_ffff:
		operand.mode = OPMODE_IMMEDIATE;
		operand.constant = 0xffff;
		break;
	case MODE_UNKNOWN:
		return -1;
	}
	if(out){
		*out = operand;
	}
	return 0;
}

static int decode_second_operand(int mode, int reg, uint16_t w1, struct operand *out_operand){
	struct operand operand = {0};

	switch(mode){
	case MODE_REGISTER_DIRECT:
		operand.mode = OPMODE_REGISTER;
		operand.reg = reg_const_for_encoded(reg);
		break;
	case MODE_INDEXED:
		operand.mode = OPMODE_INDEXED;
		operand.reg = reg_const_for_encoded(reg);
		operand.constant = w1;
		break;
	case MODE_SYMBOLIC:
		operand.mode = OPMODE_SYMBOLIC;
		operand.constant = w1;
		break;
	case MODE_ABSOLUTE:
		operand.mode = OPMODE_ABSOLUTE;
		operand.constant = w1;
		break;
	default:
		return -1;
	}

	if(out_operand){
		*out_operand = operand;
	}
	return 0;
}

static int single_operation_noperands(int opcode, int bw, int as, int reg, int *out_operation, int *out_noperands){
	int operation = OPER_UNKNOWN;
	int noperands = 0;

	switch(opcode & 7){
	case 0:
		operation = OPER_RRC;
		noperands = 1;
		break;
	case 1:
		operation = OPER_SWPB;
		noperands = 1;
		if(bw != 0){
			return -1;
		}
		break;
	case 2:
		operation = OPER_RRA;
		noperands = 1;
		break;
	case 3:
		operation = OPER_SXT;
		noperands = 1;
		if(bw != 0){
			return -1;
		}
		break;
	case 4:
		operation = OPER_PUSH;
		noperands = 1;
		break;
	case 5:
		operation = OPER_CALL;
		noperands = 1;
		if(bw != 0){
			return -1;
		}
		break;
	case 6:
		operation = OPER_RETI;
		noperands = 0;
		if(bw != 0 && as != 0 && reg != 0){
			return -1;
		}
		break;
	default:
		return -1;
	}

	if(out_operation){
		*out_operation = operation;
	}
	if(out_noperands){
		*out_noperands = noperands;
	}
	return 0;
}

static int double_operation_nopcodes(int opcode, int *out_operation, int *out_noperands){
	int operation = OPER_UNKNOWN;
	int noperands = 0;

	switch(opcode){
	case 4:
		operation = OPER_MOV;
		noperands = 2;
		break;
	case 5:
		operation = OPER_ADD;
		noperands = 2;
		break;
	case 6:
		operation = OPER_ADDC;
		noperands = 2;
		break;
	case 7:
		operation = OPER_SUBC;
		noperands = 2;
		break;
	case 8:
		operation = OPER_SUB;
		noperands = 2;
		break;
	case 9:
		operation = OPER_CMP;
		noperands = 2;
		break;
	case 10:
		operation = OPER_DADD;
		noperands = 2;
		break;
	case 11:
		operation = OPER_BIT;
		noperands = 2;
		break;
	case 12:
		operation = OPER_BIC;
		noperands = 2;
		break;
	case 13:
		operation = OPER_BIS;
		noperands = 2;
		break;
	case 14:
		operation = OPER_XOR;
		noperands = 2;
		break;
	case 15:
		operation = OPER_AND;
		noperands = 2;
		break;
	default:
		return -1;
	}
	if(out_operation){
		*out_operation = operation;
		*out_noperands = noperands;
	}
	return 0;
}

static int jump_operation_for_condition(int condition, int *out_operation){
	int operation;
	switch(condition){
		case 0: operation = OPER_JNE; break;
		case 1: operation = OPER_JEQ; break;
		case 2: operation = OPER_JNC; break;
		case 3: operation = OPER_JC;  break;
		case 4: operation = OPER_JN;  break;
		case 5: operation = OPER_JGE; break;
		case 6: operation = OPER_JL;  break;
		case 7: operation = OPER_JMP; break;
		default: return -1;
	}
	if(out_operation){
		*out_operation = operation;
	}
	return 0;
}

static int jump_fixup_offset(uint16_t offset, uint16_t *out_offset){
	uint16_t fixed;

	fixed = ((int16_t) ((offset & 0x3ff) << 6)) >> 6;
	fixed = fixed * 2 + 2;
	if(out_offset){
		*out_offset = fixed;
	}
	return 0;
}

static int instruction_mode_has_encoded_word(int mode){
	switch(mode){
		case MODE_INDEXED:
		case MODE_SYMBOLIC:
		case MODE_ABSOLUTE:
		case MODE_IMMEDIATE:
			return 1;
		default:
			return 0;
	}
}

static void fix_emulated_instructions(struct instruction *i){
	if(i->operation == OPER_MOV && i->noperands == 2 && i->operands[1].mode == OPMODE_REGISTER && i->operands[1].reg == REG_PC){
		if(i->operands[0].mode == OPMODE_INDIRECT_AUTOINC && i->operands[0].reg == REG_SP){
			i->operation = OPER_RET;
			i->noperands = 0;
			i->operands[0].mode = MODE_UNKNOWN;
			i->operands[0].reg = REG_UNKNOWN;
			i->operands[0].constant = 0;
			i->operands[1].mode = MODE_UNKNOWN;
			i->operands[1].reg = REG_UNKNOWN;
			i->operands[1].constant = 0;
			return;
		} else {
			i->operation = OPER_BR;
			i->noperands = 1;
			i->operands[1].mode = MODE_UNKNOWN;
			i->operands[1].reg = REG_UNKNOWN;
			i->operands[1].constant = 0;
			return;
		}
	}

	if(i->operation == OPER_MOV && i->noperands == 2 && i->operands[1].mode == OPMODE_REGISTER && i->operands[1].reg == REG_CG){
		i->operation = OPER_NOP;
		i->noperands = 0;
		i->operands[0].mode = MODE_UNKNOWN;
		i->operands[0].reg = REG_UNKNOWN;
		i->operands[0].constant = 0;
		i->operands[1].mode = MODE_UNKNOWN;
		i->operands[1].reg = REG_UNKNOWN;
		i->operands[1].constant = 0;
		return;
	}

	if(i->operation == OPER_MOV && i->noperands == 2 && i->operands[0].mode == OPMODE_INDIRECT_AUTOINC && i->operands[0].reg == REG_SP){
		i->operation = OPER_POP;
		i->noperands = 1;
		i->operands[0] = i->operands[1];
		i->operands[1].mode = MODE_UNKNOWN;
		i->operands[1].reg = REG_UNKNOWN;
		i->operands[1].constant = 0;
	}

	if(i->operation == OPER_MOV && i->noperands == 2 && i->operands[0].mode == OPMODE_IMMEDIATE && i->operands[0].constant == 0){
		i->operation = OPER_CLR;
		i->noperands = 1;
		i->operands[0] = i->operands[1];
		i->operands[1].mode = MODE_UNKNOWN;
		i->operands[1].reg = REG_UNKNOWN;
		i->operands[1].constant = 0;
		return;
	}
}


int unpack_instruction(const uint8_t *start, const uint8_t *end, struct instruction *out){
	struct instruction inst = {0};
	uint16_t w0, w1, w2;
	int format;
	const uint8_t *p;

	p = start;
	if(end <= p+1){
		// not enough input
		return 0;
	}
	w0 = p[0] | p[1]<<8;
	p += 2;

	format = format_for_instruction(w0);

	if(format == FMT_SINGLE){
		int opcode, bw, ad, reg;
		int mode;
		int operation, noperands;

		unpack_single(w0, &opcode, &bw, &ad, &reg);

		if(single_operation_noperands(opcode, bw, ad, reg, &operation, &noperands) != 0){
			return 0;
		}

		inst.operation = operation;
		inst.noperands = noperands;
		inst.operand_size = bw == 0 ? OPSIZE_16 : OPSIZE_8;

		if(noperands == 1){
			mode = double_src_operand_mode(ad, reg);

			if(instruction_mode_has_encoded_word(mode)){
				if(end <= p+1){
					// not enough input
					return 0;
				}
				w1 = p[0] | p[1]<<8;
				p += 2;
			} else {
				w1 = 0;
			}
			if(decode_first_operand(mode, reg, w1, &inst.operands[0]) != 0){
				return 0;
			}
		}
	} else if(format == FMT_DOUBLE){
		int opcode, src, ad, bw, as, dst;
		int src_mode, dst_mode;
		int operation, noperands;

		unpack_double(w0, &opcode, &src, &ad, &bw, &as, &dst);

		if(double_operation_nopcodes(opcode, &operation, &noperands) != 0){
			return 0;
		}

		inst.operation = operation;
		inst.noperands = noperands;
		inst.operand_size = bw == 0 ? OPSIZE_16 : OPSIZE_8;

		src_mode = double_src_operand_mode(as, src);
		dst_mode = double_dst_operand_mode(ad, dst);

		if(instruction_mode_has_encoded_word(src_mode)){
			if(end <= p+1){
				// not enough input
				return 0;
			}
			w1 = p[0] | p[1]<<8;
			p += 2;
		} else {
			w1 = 0;
		}

		if(instruction_mode_has_encoded_word(dst_mode)){
			if(end <= p+1){
				return 0;
			}
			w2 = p[0] | p[1]<<8;
			p += 2;
		} else {
			w2 = 0;
		}

		if(decode_first_operand(src_mode, src, w1, &inst.operands[0]) != 0){
			return 0;
		}

		if(decode_second_operand(dst_mode, dst, w2, &inst.operands[1]) != 0){
			return 0;
		}
	} else if(format == FMT_JUMP){
		int opcode, condition, offset;
		int operation;
		uint16_t fixed_offset;

		unpack_jump(w0, &opcode, &condition, &offset);

		if(jump_operation_for_condition(condition, &operation) != 0){
			return 0;
		}
		if(jump_fixup_offset(offset, &fixed_offset) != 0){
			return 0;
		}

		inst.operation = operation;
		inst.operand_size = OPSIZE_16;
		inst.noperands = 1;
		inst.operands[0].mode = OPMODE_JUMP;
		inst.operands[0].constant = fixed_offset;
	} else {
		return 0;
	}

	fix_emulated_instructions(&inst);

	if(out){
		*out = inst;
	}
	return (int)(p - start);
}

int string_for_operation(const struct instruction inst, char *out){
	if(inst.operand_size == OPSIZE_16){
		strcpy(out, lookup_mnemonic_for_operation(inst.operation));
		return 0;
	} else if(inst.operand_size == OPSIZE_8){
		strcpy(out, lookup_mnemonic_for_operation(inst.operation));
		strcat(out, ".b");
		return 0;
	} else {
		return -1;
	}
}

int string_for_operand(const struct operand operand, char *out){
	char *reg_str;

	switch(operand.mode){
	case OPMODE_REGISTER:
		reg_str = lookup_reg_string(operand.reg);
		strcpy(out, reg_str);
		return 0;
	case OPMODE_INDEXED:
		reg_str = lookup_reg_string(operand.reg);
		sprintf(out, "0x%x(%s)", operand.constant & 0xffff, reg_str);
		return 0;
	case OPMODE_SYMBOLIC:
		if(0 <= (uint16_t)operand.constant){
			sprintf(out, "$+0x%x", operand.constant);
		} else {
			sprintf(out, "$-0x%x", (-operand.constant));
		}
		return 0;
	case OPMODE_ABSOLUTE:
		sprintf(out, "&0x%x", operand.constant & 0xffff);
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
		sprintf(out, "#0x%x", operand.constant & 0xffff);
		return 0;
	case OPMODE_JUMP:
		if(0 <= (int16_t)operand.constant){
			sprintf(out, "$+0x%x", operand.constant & 0xffff);
		} else {
			sprintf(out, "$-0x%x", (-operand.constant) & 0xffff);
		}
		return 0;
	default:
		out[0] = 0;
		return -1;
	}
}

void disassemble_instruction(const struct instruction inst, char *out){
	char mnemonic[16];
	char operand[2][16];
	int i;

	string_for_operation(inst, mnemonic);

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
