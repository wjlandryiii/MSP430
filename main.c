/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>



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
	uint16_t w0;
	uint16_t w1;
	uint16_t w2;
	int len;
};

enum {
	FMT_UNKNOWN = 0,
	FMT_DOUBLE,
	FMT_SINGLE,
	FMT_JUMP,
};



int unpack_instruction(uint8_t *start, uint8_t *end, struct unpacked_inst *out){
	struct unpacked_inst ret = {0};

	uint16_t w0, w1, w2;
	uint16_t prefix;


	w0 = start[0] | start[1] << 8;
	ret.w0 = w0;
	ret.len = 2;

	prefix = (w0 >> 12) & 0xf;

	if(prefix == 1 && (w0 & 0xc00) == 0){
		ret.format = FMT_SINGLE;
		ret.opcode = (w0 >> 7) & 0x1ff;
		ret.bw = (w0 >> 6) & 1;
		ret.ad = (w0 >> 4) & 0x03;
		ret.reg0 = w0 & 0x0f;

		switch(ret.opcode & 7){
		case 1:
		case 3:
		case 5:
			if(ret.bw != 0){
				return -1;
			}
		case 0:
		case 2:
		case 4:
			if((ret.ad == 1 && ret.reg0 != 3) || (ret.ad == 3 && ret.reg0 == 0)){
				ret.w1 = start[2] | start[3]<<8;
				ret.len += 2;
			}
			break;
		case 6:
			if(ret.bw != 0 && ret.ad != 0 && ret.reg0 != 0){
				return -1;
			}
		default:
			return -1;
		}
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

		if((ret.as == 1 && ret.reg0 != 3) || (ret.as == 3 && ret.reg0 == 0)){
			ret.w1 = start[2] | start[3] << 8;
			ret.len += 2;
			if(ret.ad == 1){
				ret.w2 = start[4] | start[5] << 8;
				ret.len += 2;
			}
		} else if(ret.ad == 1){
			ret.w1 = start[2] | start[3] << 8;
			ret.len += 2;
		}
	} else {
		return -1;
	}

	*out = ret;
	return 0;
}




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

int main(int argc, char *argv[]){
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
	return 0;
}
