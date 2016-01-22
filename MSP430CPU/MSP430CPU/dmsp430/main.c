/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>



#include "dmsp430.h"



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

	len = unpack_instruction(start, end, &inst);
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

uint8_t inst_bin[] = {
	0x0e, 0x4f, 0x0f, 0x43, 0x0b, 0x3c, 0x6d, 0x4e, 0x8d, 0x11, 0x0d, 0x5f,
	0x0f, 0x4d, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f, 0x0f, 0x5f,
	0x0f, 0x8d, 0x1e, 0x53, 0xce, 0x93, 0x00, 0x00, 0xf2, 0x23, 0x30, 0x41,
	0x0b, 0x12, 0x0a,// 0x12, 0x09, 0x12, //0x0b, 0x4f, //0x0a, 0x4e, // 0x09, 0x4d,
	//0x1e, 0x4f
};


void test_inst_bin(){
	uint8_t *start, *end;
	struct instruction inst;
	int len;
	char buff[1024];

	start = inst_bin;
	end = inst_bin + sizeof(inst_bin);

	while(start < end){
		len = unpack_instruction(start, end, &inst);
		if(0 < len){
			disassemble_instruction(inst, buff);
			puts(buff);
			printf("%d\n", len);
		} else {
			fprintf(stderr, "ERROR\n");
			fflush(stderr);
			abort();
		}

		start += len;
	}
}


int main(int argc, char *argv[]){

	//test_inst_bin();
	//

	/*
	int jz_condition = 1;
	int jmp_condition = 7;

	printf("jz  %02x\n", encode_jump(jz_condition, ((0x4402 - 0x442e) - 2) >> 1));
	printf("jmp %02x\n", encode_jump(jmp_condition, ((0x4406 - 0x4430) - 2) >> 1));
	*/

	char disassembly[1024];
	uint8_t buff[6] = {0};
	//3012 7F00
	buff[0] = 0x30;
	buff[1] = 0x12;
	buff[2] = 0x7f;
	buff[3] = 0x00;

	struct instruction inst;

	unpack_instruction(buff, buff+6, &inst);
	disassemble_instruction(inst, disassembly);
	puts(disassembly);


	return 0;
}
