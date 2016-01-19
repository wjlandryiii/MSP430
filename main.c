/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "unpack.c"
#include "disassemble.c"


uint8_t inst_bin[] = {
	0x0e, 0x4f, 0x0f, 0x43, 0x0b, 0x3c, 0x6d, 0x4e, 0x8d, 0x11, 0x0d, 0x5f,
	0x0f, 0x4d, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f,
	0x0f, 0x8d, 0x1e, 0x53, 0xce, 0x93, 0x00, 0x00, 0xf2, 0x23, 0x30, 0x41,
	0x0b, 0x12, 0x0a, 0x12, 0x09, 0x12, 0x0b, 0x4f, 0x0a, 0x4e, 0x09, 0x4d,
	0x1e, 0x4f
};

/*
void disassemble(){
	uint8_t *p, *end;
	struct unpacked_inst unpacked;
	struct decoded_inst decoded;
	struct disassembled_inst disassembled;

	p = inst_bin;
	end = p + sizeof(inst_bin);

	while(p < e){
		unpacked = unpack_instruction(p, e);
		p += unpacked.length;

		decoded = decode_instruction(unpacked);
		disassembled = disassemble_instruction(decoded);

		string_for_instruction(disassembled);
		puts(diassembled);

	}
}
*/

/*
void test(){
	int i, j, rt;

	uint8_t *p;
	uint16_t buff[3] = {0};
	struct unpacked_inst unpacked = {0};

	p = (uint8_t *)buff;
	for(i = 0; i < 65536; i++){
		buff[0] = i;
		rt = unpack_instruction((uint8_t *) buff, 0, &unpacked);
		if(rt == 0){
			for(j = 0; j < unpacked.len; j += 2){
				if(j != 0){
					putchar(' ');
				}
				printf("%02x %02x", p[j], p[j+1]);
			}
			putchar('\n');
		}

	}
}
*/

int is_format_single(uint16_t w0){
	if((w0 & 0xf800) == 0x1000){
		return 1;
	} else {
		return 0;
	}
}

int is_format_double(uint16_t w0){
	if(0x4000 <= (w0 & 0xf000)){
		return 1;
	} else {
		return 0;
	}
}

int is_format_jump(uint16_t w0){
	if((w0 & 0xe000) == 0x2000){
		return 1;
	} else {
		return 0;
	}
}


int is_bw_set(uint16_t w0){
	if((w0 & 0x0040) == 0x0040){
		return 1;
	} else {
		return 0;
	}
}

int ad_for_single(uint16_t w0){
	return (w0 & 0x0030) >> 4;
}

int reg_for_single(uint16_t w0){
	return (w0 & 0x000f);
}

int single_has_extra_word(int ad, int reg){
	if(reg == 0 && ad == 3){
		return 1;
	} else if(reg == 3 && ad == 1){
		return 1;
	} else {
		return 0;
	}
}

int unpack_instructionB(uint8_t *start, uint8_t *end, struct instruction *out){
	uint16_t w0, w1, w2;
	uint16_t prefix;

	struct instruction inst = {0};
	uint8_t *p;
	int len;

	p = start;

	if(end <= p+2){ return -1; } // ran out of input
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
				if(end <= p+2){
					return 0;
				}
				w1 = p[0] | p[1] << 8;
				p += 2;
				len += 2;
				inst.noperands = 1;
				inst.operands[0] = indexed_operand(reg+1, w1);
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
					if(end <= p+2){
						return 0;
					}
					w1 = p[0] | p[1] << 8;
					p += 2;
					len += 2;
					inst.noperands = 1;
					inst.operands[0] = symbolic_operand(w1);
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
				if(end <= p+2){
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
				if(end <= p+2){
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
			} else {
				inst.noperands = 2;
				inst.operands[0] = indirect_register_operand(src+1);
			}
		} else if(as == 3){
			if(src == 0){
				if(end <= p+2){
					return 0;
				}
				w1 = p[0] | p[1] << 8;
				p += 2;
				len += 2;
				ate_w1 = 0;
				inst.noperands = 2;
				inst.operands[0] = symbolic_operand(w1);
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

		} else if(ad == 1){
			inst.noperands = 2;
			if(end <= p+2){
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
		} else {
			abort();
		}

		*out = inst;
		return len;

	} else {
		// invalid instruction
		return -1;
	}

	return 0;
}


uint16_t encode_single(int opcode, int bw, int ad, int reg){
	uint16_t ret;

	ret = 0x1000;
	ret |= (opcode & 7)<<7;
	ret |= (bw & 1)<<6;
	ret |= (ad & 3) << 4;
	ret |= (reg & 0xf);
	return ret;
}

uint16_t encode_jump(int condition, int offset){
	uint16_t ret;

	ret = 0x2000;
	ret |= (condition & 7) << 10;
	ret |= (offset & 0x3ff);
	return ret;
}

uint16_t encode_double(int opcode, int src, int ad, int bw, int as, int dst){
	uint16_t ret;

	ret = 0;
	ret |= (opcode & 0xf) << 12;
	ret |= (src & 0xf) << 8;
	ret |= (ad & 1) << 7;
	ret |= (bw & 1) << 6;
	ret |= (as & 3) << 4;
	ret |= (dst & 0xf);
	return ret;
}

void test_instruction(uint16_t *words){
	char buff[1024];
	struct instruction inst;
	int len, l;

	uint8_t *start = (uint8_t *)words;
	uint8_t *end = (uint8_t *)&words[4];

	len = unpack_instructionB(start, end, &inst);
	if(len > 0){
		for(l = 0; l < len; l++){
			printf("%02x", start[l]);
		}
		for(; l < 6; l++){
			printf("  ");
		}
		disassemble_instruction(inst, buff);
		printf("\t");
		puts(buff);
	} else {
		puts("fail");
	}
}


int main(int argc, char *argv[]){
	struct instruction inst;
	int i, j, k, l;

	uint16_t bin[3] = {0};

	/*
	for(i = 0; i < 6; i++){
		for(j = 0; j < 4; j++){
			for(k = 0; k < 5; k++){
				bin[0] = encode_single(i, 0, j, k);
				test_instruction(bin);
			}
		}
	}

	bin[0] = encode_single(6, 0, 0, 0);
	test_instruction(bin);

	for(i = 0; i < 8; i++){
		for(j = -0x8; j <= 0x8; j += 0x1){
			bin[0] = encode_jump(i, j);
			test_instruction(bin);
		}
	}
	*/

	for(j = 0; j < 4; j++){
		for(k = 0; k < 2; k++){
			bin[0] = encode_double(4, 2, k, 0, 0, j);
			test_instruction(bin);
		}
	}
	/*
	for(i = 0; i < 4; i++){
		for(j = 0; j < 2; j++){
			for(k = 0; k < 4; k++){
				for(l = 0; l < 4; l++){
					bin[0] = encode_double(4, i, j, 0, k, l);
					test_instruction(bin);
				}
			}
		}
	}
	*/

	return 0;
}
