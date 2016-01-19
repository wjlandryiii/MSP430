enum {
	OPER_UNKNOWN = 0,
	OPER_RRC,
	OPER_SWPB,
	OPER_RRA,
	OPER_SXT,
	OPER_PUSH,
	OPER_CALL,
	OPER_RETI,
	OPER_JNE,
	OPER_JEQ,
	OPER_JNC,
	OPER_JC,
	OPER_JN,
	OPER_JGE,
	OPER_JL,
	OPER_JMP,
	OPER_MOV,
	OPER_ADD,
	OPER_ADDC,
	OPER_SUBC,
	OPER_SUB,
	OPER_CMP,
	OPER_DADD,
	OPER_BIT,
	OPER_BIC,
	OPER_BIS,
	OPER_XOR,
	OPER_AND,
};

enum {
	REG_UNKNOWN = 0,
	REG_PC,
	REG_SP,
	REG_SR,
	REG_CG,
	REG_R4,
	REG_R5,
	REG_R6,
	REG_R7,
	REG_R8,
	REG_R9,
	REG_R10,
	REG_R11,
	REG_R12,
	REG_R13,
	REG_R14,
	REG_R15,
};

char *lookup_mnemonic_for_operation(int operation){
	switch(operation){
		case OPER_RRC:	return "rrc";
		case OPER_SWPB:	return "swpb";
		case OPER_RRA:	return "rra";
		case OPER_SXT:	return "sxt";
		case OPER_PUSH:	return "push";
		case OPER_CALL:	return "call";
		case OPER_RETI:	return "reti";
		case OPER_JNE:	return "jne";
		case OPER_JEQ:	return "jeq";
		case OPER_JNC:	return "jnc";
		case OPER_JC:	return "jc";
		case OPER_JN:	return "jn";
		case OPER_JGE:	return "jge";
		case OPER_JL:	return "jl";
		case OPER_JMP:	return "jmp";
		case OPER_MOV:	return "mov";
		case OPER_ADD:	return "add";
		case OPER_ADDC:	return "addc";
		case OPER_SUBC:	return "subc";
		case OPER_SUB:	return "sub";
		case OPER_CMP:	return "cmp";
		case OPER_DADD:	return "dadd";
		case OPER_BIT:	return "bit";
		case OPER_BIC:	return "bic";
		case OPER_BIS:	return "bis";
		case OPER_XOR:	return "xor";
		case OPER_AND:	return "and";
		default:	return 0;
	}
}

char *lookup_reg_string(int reg){
	switch(reg){
		case REG_PC:	return "pc";
		case REG_SP:	return "sp";
		case REG_SR:	return "sr";
		case REG_CG:	return "cg";
		case REG_R4:	return "r4";
		case REG_R5:	return "r5";
		case REG_R6:	return "r6";
		case REG_R7:	return "r7";
		case REG_R8:	return "r8";
		case REG_R9:	return "r9";
		case REG_R10:	return "r10";
		case REG_R11:	return "r11";
		case REG_R12:	return "r12";
		case REG_R13:	return "r13";
		case REG_R14:	return "r14";
		case REG_R15:	return "r15";
		default:	return 0;
	}
}

