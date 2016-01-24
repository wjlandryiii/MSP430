/*
 * Copyright 2016 Joseph Landry All Rights Reserved
 */


struct test_case {
	char *test_name;
	char *assembly;
	uint8_t packed[6];
	int len;
	struct instruction unpacked;
};

struct test_case test_cases[] = {
	{
		"double reg to reg",
		"mov      r14, r15",
		{0xf, 0x4e, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2,{
						 {OPMODE_REGISTER, REG_R14, 0},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double indexed to reg",
		"mov      0x10(r14), r15",
		{0x1f, 0x4e, 0x10, 0x00, 0x00, 0x00}, 4,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_INDEXED, REG_R14, 0x10},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double indirect_reg to reg",
		"mov      @r14, r15",
		{0x2f, 0x4e, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_INDIRECT_REGISTER, REG_R14, 0},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double indirect_autoinc to reg",
		"mov      @r14+, r15",
		{ 0x3f, 0x4e, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_INDIRECT_AUTOINC, REG_R14, 0},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double symbolic to reg",
		"mov      $+0x55, r15",
		{ 0x1f, 0x40, 0x55, 0x00, 0x00, 0x00}, 4,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_SYMBOLIC, REG_UNKNOWN, 0x55},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double immediate to reg",
		"mov      #0xaaaa, r15",
		{0x3f, 0x40, 0xaa, 0xaa, 0x00, 0x00}, 4,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_IMMEDIATE, REG_UNKNOWN, 0xaaaa},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double absolute to reg",
		"mov      &0xaaaa, r15",
		{0x1f, 0x42, 0xaa, 0xaa, 0x00, 0x00}, 4,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_ABSOLUTE, REG_UNKNOWN, 0xaaaa},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double reg to indexed",
		"mov      r14, 0x10(r15)",
		{0x8f, 0x4e, 0x10, 0x00, 0x00, 0x00}, 4,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_REGISTER, REG_R14, 0},
						 {OPMODE_INDEXED, REG_R15, 0x10},
					 }
		}
	},{
		"double reg to symbolic",
		"mov      r14, $+0x55",
		{0x80, 0x4e, 0x55, 0x00, 0x00, 0x00}, 4,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_REGISTER, REG_R14, 0},
						 {OPMODE_SYMBOLIC, REG_UNKNOWN, 0x55},
					 }
		}
	},{
		"double cg 0 to reg",
		"mov      #0x0, r15",
		{0x0f, 0x43, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_IMMEDIATE, REG_UNKNOWN, 0x0000},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double cg 1 to reg",
		"mov      #0x1, r15",
		{0x1f, 0x43, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_IMMEDIATE, REG_UNKNOWN, 0x0001},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double cg 2 to reg",
		"mov      #0x2, r15",
		{0x2f, 0x43, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_IMMEDIATE, REG_UNKNOWN, 0x0002},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double cg 4 to reg",
		"mov      #0x4, r15",
		{0x2f, 0x42, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_IMMEDIATE, REG_UNKNOWN, 0x0004},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double cg 8 to reg",
		"mov      #0x8, r15",
		{0x3f, 0x42, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_IMMEDIATE, REG_UNKNOWN, 0x0008},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double cg -1 to reg",
		"mov      #0xffff, r15",
		{0x3f, 0x43, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_16, 2, {
						 {OPMODE_IMMEDIATE, REG_UNKNOWN, 0xFFFF},
						 {OPMODE_REGISTER, REG_R15, 0},
					 }
		}
	},{
		"double reg to reg byte",
		"mov.b    r14, r15",
		{0x4f, 0x4e, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_MOV, OPSIZE_8, 2, {
						{OPMODE_REGISTER, REG_R14, 0},
						{OPMODE_REGISTER, REG_R15, 0},
					}
		}
	},{
		"single reg",
		"rrc      r15",
		{0x0f, 0x10, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_RRC, OPSIZE_16, 1, { {OPMODE_REGISTER, REG_R15, 0}, } }
	},{
		"single indexed",
		"rrc      0x10(r15)",
		{0x1f, 0x10, 0x10, 0x00, 0x00, 0x00}, 4,
		{OPER_RRC, OPSIZE_16, 1, { {OPMODE_INDEXED, REG_R15, 0x10}, } }
	},{
		"single symbolic",
		"rrc      $+0x55",
		{0x10, 0x10, 0x55, 0x00, 0x00, 0x00}, 4,
		{OPER_RRC, OPSIZE_16, 1, { {OPMODE_SYMBOLIC, REG_UNKNOWN, 0x55}, } }
	},{
		"single absolute",
		"rrc      &0xaaaa",
		{0x12, 0x10, 0xaa, 0xaa}, 4,
		{OPER_RRC, OPSIZE_16, 1, { {OPMODE_ABSOLUTE, REG_UNKNOWN, 0xaaaa}, } }
	},{
		"single indirect_register",
		"rrc      @r15",
		{0x2f, 0x10, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_RRC, OPSIZE_16, 1, { {OPMODE_INDIRECT_REGISTER, REG_R15, 0}, }}
	},{
		"single indirect_autoinc",
		"rrc      @r15+",
		{0x3f, 0x10, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_RRC, OPSIZE_16, 1, { {OPMODE_INDIRECT_AUTOINC, REG_R15, 0}, }}
	},{
		"single immediate",
		"call     #0xaaaa",
		{0xb0, 0x12, 0xaa, 0xaa}, 4,
		{OPER_CALL, OPSIZE_16, 1, { {OPMODE_IMMEDIATE, REG_UNKNOWN, 0xaaaa}, } }
	},{
		"jump forward",
		"jmp      $+0x50",
		{0x27, 0x3c, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_JMP, OPSIZE_16, 1, { {OPMODE_JUMP, REG_UNKNOWN, 0x50}, } }
	},{
		"jump backward",
		"jmp      $-0x50",
		{0xd7, 0x3f, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_JMP, OPSIZE_16, 1, { {OPMODE_JUMP, REG_UNKNOWN, 0xffb0}, } }
	},{
		"jump equal",
		"jeq      $+0x50",
		{0x27, 0x24, 0x00, 0x00, 0x00, 0x00}, 2,
		{OPER_JEQ, OPSIZE_16, 1, { {OPMODE_JUMP, REG_UNKNOWN, 0x50}, }}
	}
};


void print_instruction(struct instruction inst){
	int i;

	printf("   operation: %s\n", lookup_operation_const_name(inst.operation));
	printf("operand size: %s\n", lookup_operand_size_const_name(inst.operand_size));
	printf("   noperands: %d\n", inst.noperands);

	for(i = 0; i < inst.noperands; i++){
		printf("\toperand: %d\n", i);
		printf("\t\t mode: %s\n", lookup_operand_mode_const_name(inst.operands[i].mode));
		printf("\t\t  reg: %s\n", lookup_reg_const_name(inst.operands[i].reg));
		printf("\t\tconst: 0x%04x\n", inst.operands[i].constant);
	}
	printf("\n");
}


void test_unpack(void){
	struct test_case c;
	struct instruction inst;
	int i;

	for(i = 0; i < sizeof(test_cases) / sizeof(*test_cases); i++){
		c = test_cases[i];
		memset(&inst, 0, sizeof(inst));
		printf("test: %2d %-32s %-32s", i, c.test_name, c.assembly);
		fflush(stdout);
		unpack_instructionB(c.packed, c.packed+6, &inst);
		if(memcmp(&c.unpacked, &inst, sizeof(inst)) != 0){
			printf("FAIL\n");
			printf("===========================================\n");
			printf("Test Instruction:\n");
			print_instruction(c.unpacked);
			printf("===========================================\n");
			printf("Resulting instruction:\n");
			print_instruction(inst);
			abort();
		} else {
			printf("OK\n");
		}
	}
}

void test_disassemble(void){
	struct test_case c;
	char buff[1024];
	int i, j, l;

	for(i = 0; i < sizeof(test_cases) / sizeof(*test_cases); i++){
		c = test_cases[i];
		memset(buff, 0, sizeof(buff));

		printf("test: %2d %-32s %-32s", i, c.test_name, c.assembly);
		fflush(stdout);

		disassemble_instruction(c.unpacked, buff);

		if(strcmp(c.assembly, buff) != 0){
			printf("FAIL!\n");
			printf("Got disassembly: \"%s\"\n", buff);
			printf("                   ");
			if(strlen(buff) < strlen(c.assembly)){
				l = strlen(c.assembly);
			} else {
				l = strlen(buff);
			}
			for(j = 0; j < l; j++){
				if(buff[j] != c.assembly[j]){
					putchar('^');
					break;
				} else {
					putchar(' ');
				}
			}
			putchar('\n');
			abort();
		} else {
			printf("OK\n");
		}
	}
}
