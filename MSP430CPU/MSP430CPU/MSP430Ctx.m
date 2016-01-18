//
//  MSP430Ctx.m
//  MSP430CPU
//
//  Created by wjl on 1/17/16.
//  Copyright © 2016 wjl. All rights reserved.
//

#import "MSP430Ctx.h"
#import "MSP430CPU.h"
#import <Hopper/CommonTypes.h>
#import <Hopper/CPUDefinition.h>
#import <Hopper/HPDisassembledFile.h>

#include "decode.h"

/*
 pc  4400  sp  0000  sr  0000  cg  0000
 r04 0000  r05 0000  r06 0000  r07 0000
 r08 0000  r09 0000  r10 0000  r11 0000
 r12 0000  r13 0000  r14 0000  r15 0000
 */

@implementation MSP430Ctx {
	MSP430CPU *_cpu;
	NSObject<HPDisassembledFile> *_file;
}

-(instancetype)initWithCPU:(MSP430CPU *)cpu andFile:(NSObject<HPDisassembledFile> *)file {
	if(self = [super init]){
		_cpu = cpu;
		_file = file;
	}
	return self;
}

-(NSObject<CPUDefinition> *)cpuDefinition{
	return _cpu;
}

-(void)initDisasmStructure:(DisasmStruct *)disasm withSyntaxIndex:(NSUInteger)syntaxIndex {
	bzero(disasm, sizeof(*disasm));
}

-(Address)adjustCodeAddress:(Address)address {
	return address & ~1;
}

- (uint8_t)cpuModeFromAddress:(Address)address {
	return 0;
}

-(BOOL)addressForcesACPUMode:(Address)address {
	return NO;
}

-(Address)nextAddressToTryIfInstructionFailedToDecodeAt:(Address)address forCPUMode:(uint8_t)mode {
	return ((address & ~1) + 2);
}

-(int)isNopAt:(Address)address {
	//TODO: 0x4f0f  ? byteorder?
	return 0;
}

- (BOOL)hasProcedurePrologAt:(Address)address{
	return NO;
}

- (void)analysisBeginsAt:(Address)entryPoint {}
- (void)analysisEnded {}
- (void)procedureAnalysisBeginsForProcedure:(NSObject<HPProcedure> *)procedure atEntryPoint:(Address)entryPoint {}
- (void)procedureAnalysisOfPrologForProcedure:(NSObject<HPProcedure> *)procedure atEntryPoint:(Address)entryPoint {}
- (void)procedureAnalysisOfEpilogForProcedure:(NSObject<HPProcedure> *)procedure atEntryPoint:(Address)entryPoint {}
- (void)procedureAnalysisEndedForProcedure:(NSObject<HPProcedure> *)procedure atEntryPoint:(Address)entryPoint {}
- (void)procedureAnalysisContinuesOnBasicBlock:(NSObject<HPBasicBlock> *)basicBlock {}
- (void)resetDisassembler {}

- (Address)getThunkDestinationForInstructionAt:(Address)address {
	return BAD_ADDRESS;
}

- (uint8_t)estimateCPUModeAtVirtualAddress:(Address)address {
	return 0;
}

uint16_t memory_read_callback(uint32_t address, void* private) {
	MSP430Ctx *ctx = (__bridge MSP430Ctx *)private;
	return [ctx readWordAt:address];
}

- (uint16_t)readWordAt:(uint32_t)address {
	return [_file readUInt16AtVirtualAddress:address];
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


int single_fmt(DisasmStruct * disasm, struct single *s, uint16_t w1){
	char *reg;
	
	strcpy(disasm->instruction.mnemonic, single_mnemonics[s->opcode & 7]);
	if(s->bw){
		strcat(disasm->instruction.mnemonic, ".b");
	}
	
	reg = reg_strings[s->reg];
	
	switch(s->ad){
		case 0:
			strcpy(disasm->operand[0].mnemonic, reg);
			disasm->operand[0].type = DISASM_OPERAND_REGISTER_TYPE;
			return 2;
		case 1:
			sprintf(disasm->operand[0].mnemonic, "0x%x(%s)", w1, reg);
			disasm->operand[0].type = DISASM_OPERAND_REGISTER_TYPE | DISASM_OPERAND_RELATIVE;
			return 4;
		case 2:
			sprintf(disasm->operand[0].mnemonic, "@%s", reg);
			disasm->operand[0].type = DISASM_OPERAND_MEMORY_TYPE;
			return 2;
		case 4:
			sprintf(disasm->operand[0].mnemonic, "@%s+", reg);
			disasm->operand[0].type = DISASM_OPERAND_MEMORY_TYPE;
			return 2;
		default:
			return 0;
	}
}


int double_fmt(DisasmStruct *disasm, struct double_ *d, uint16_t w1, uint16_t w2){
	char *src_reg;
	char *dst_reg;
	int len = 2;
	
	
	strcpy(disasm->instruction.mnemonic, double_mnemonics[d->opcode - 4]);
	if(d->bw){
		strcat(disasm->instruction.mnemonic, ".b");
	}
	
	src_reg = reg_strings[d->src];
	dst_reg = reg_strings[d->dst];
	
	if(d->src == 2 && ((d->as == 1 && d->ad == 1) || d->as == 2 || d->ad == 3)){
		switch(d->as){
			case 1:
				strcpy(disasm->operand[0].mnemonic, "&sr");
				break;
			case 2:
				strcpy(disasm->operand[0].mnemonic, "#4");
				break;
			case 3:
				strcpy(disasm->operand[0].mnemonic, "#8");
				break;
			default:
				return 0;
		}
	} else if(d->src == 3){
		switch(d->as){
			case 0:
				strcpy(disasm->operand[0].mnemonic, "#0");
				break;
			case 1:
				strcpy(disasm->operand[0].mnemonic, "#1");
				break;
			case 2:
				strcpy(disasm->operand[0].mnemonic, "#2");
				break;
			case 3:
				strcpy(disasm->operand[0].mnemonic, "#-1");
				break;
			default:
				return 0;
		}
	} else {
		switch(d->as){
			case 0:
				strcpy(disasm->operand[0].mnemonic, src_reg);
				disasm->operand[0].type = DISASM_OPERAND_REGISTER_TYPE;
				break;
			case 1:
				sprintf(disasm->operand[0].mnemonic, "0x%x(%s)", w1, src_reg);
				disasm->operand[1].type = DISASM_OPERAND_RELATIVE | DISASM_OPERAND_MEMORY_TYPE;
				len += 2;
				break;
			case 2:
				sprintf(disasm->operand[0].mnemonic, "@%s", src_reg);
				disasm->operand[1].type = DISASM_OPERAND_MEMORY_TYPE;
				break;
			case 3:
				sprintf(disasm->operand[0].mnemonic, "@%s+", src_reg);
				disasm->operand[1].type = DISASM_OPERAND_MEMORY_TYPE;
				break;
			default:
				return 0;
		}
	}
	
	if(d->ad == 0){
		strcpy(disasm->operand[1].mnemonic, dst_reg);
		disasm->operand[1].type = DISASM_OPERAND_REGISTER_TYPE;
	} else {
		int disp;
		if(len == 2){
			disp = w1;
		} else {
			disp = w2;
		}
		sprintf(disasm->operand[1].mnemonic, "0x%x(%s)", disp, src_reg);
		disasm->operand[1].type = DISASM_OPERAND_RELATIVE | DISASM_OPERAND_REGISTER_TYPE;
		len += 2;
	}
	return len;
}

- (int)disassembleSingleInstruction:(DisasmStruct *)disasm usingProcessorMode:(NSUInteger)mode {
	struct msp_inst inst;
	uint16_t w0, w1, w2;

	w0 = disasm->bytes[0] | disasm->bytes[1] << 8;

	inst = decode_instruction(w0);

	if(inst.format == FMT_SINGLE){
		struct single *s;
		int opcode;
		
		s = &inst.single;
		opcode = s->opcode & 7;
		if(opcode < 7){
			if(opcode != 6){
				if((opcode & 1) == 0){
					if(s->ad != 1){
						return single_fmt(disasm, s, 0);
					} else {
						w1 = disasm->bytes[2] | disasm->bytes[3]<<8;
						return single_fmt(disasm, s, w1);
					}
				} else {
					if(s->bw == 0){
						if(s->ad != 1){
							return single_fmt(disasm, s, 0);
						} else {
							w1 = disasm->bytes[2] | disasm->bytes[3]<<8;
							return single_fmt(disasm, s, w1);
						}
					} else {
						return 0;
					}
				}
			} else {
				if(s->bw == 0 && s->ad == 0 && s->reg == 0){
					strcpy(disasm->instruction.mnemonic, "reti");
					disasm->instruction.branchType = DISASM_BRANCH_RET;
					return 2;
				} else {
					return 0;
				}
			}
			
		} else {
			return 0;
		}
	} else if(inst.format == FMT_DOUBLE){
		struct double_ *d = &inst.double_;
		if(d->opcode < 4){
			return 0;
		} else {
			if(d->as == 1 && d->ad == 1){
				w1 = disasm->bytes[2] | disasm->bytes[3]<<8;
				w2 = disasm->bytes[4] | disasm->bytes[5]<<8;
			} else if(d->as == 1 || d->ad == 1){
				w1 = disasm->bytes[2] | disasm->bytes[3]<<8;
				w2 = disasm->bytes[4] | disasm->bytes[5]<<8;
			} else {
				w1 = w2 = 0;
			}
			return double_fmt(disasm, d, w1, w2);
		}
	} else if(inst.format == FMT_JUMP){
		if(inst.jump.condition == 7){
			strcpy(disasm->instruction.mnemonic, "jmp");
			disasm->instruction.branchType = DISASM_BRANCH_JMP;

			disasm->instruction.addressValue = (disasm->virtualAddr + inst.jump.offset * 2) & 0xffff;
			disasm->operand[0].type = DISASM_OPERAND_CONSTANT_TYPE | DISASM_OPERAND_RELATIVE;
			disasm->operand[0].immediateValue = disasm->instruction.addressValue;

			if(0 <= inst.jump.offset){
				sprintf(disasm->operand[0].mnemonic, "#0x%x", inst.jump.offset * 2);
			} else {
				sprintf(disasm->operand[0].mnemonic, "#-0x%x", inst.jump.offset * 2);
			}
			return 2;
		} else {
			return 0;
		}

	} else {
		return 0;
	}
}

- (BOOL)instructionHaltsExecutionFlow:(DisasmStruct *)disasm {
	return NO;
}

- (void)performBranchesAnalysis:(DisasmStruct *)disasm computingNextAddress:(Address *)next andBranches:(NSMutableArray *)branches forProcedure:(NSObject<HPProcedure> *)procedure basicBlock:(NSObject<HPBasicBlock> *)basicBlock ofSegment:(NSObject<HPSegment> *)segment calledAddresses:(NSMutableArray *)calledAddresses callsites:(NSMutableArray *)callSitesAddresses {
	
}

- (void)performInstructionSpecificAnalysis:(DisasmStruct *)disasm forProcedure:(NSObject<HPProcedure> *)procedure inSegment:(NSObject<HPSegment> *)segment {
	
}

- (void)performProcedureAnalysis:(NSObject<HPProcedure> *)procedure basicBlock:(NSObject<HPBasicBlock> *)basicBlock disasm:(DisasmStruct *)disasm {
	
}

- (void)updateProcedureAnalysis:(DisasmStruct *)disasm {
	
}

// Printing

- (void)buildInstructionString:(DisasmStruct *)disasm forSegment:(NSObject<HPSegment> *)segment populatingInfo:(NSObject<HPFormattedInstructionInfo> *)formattedInstructionInfo {
	char *p = disasm->completeInstructionString;
	int i;
	
	if(disasm->operand[0].mnemonic[0] != 0){
		sprintf(p, "%-8s ", disasm->instruction.mnemonic);
		
		i = 0;
		while(disasm->operand[i].mnemonic[0] != 0){
			strcat(p, disasm->operand[i].mnemonic);
			i++;
			if(disasm->operand[i].mnemonic[0] != 0){
				strcat(p, ", ");
			}
		}
	} else {
		strcpy(p, disasm->instruction.mnemonic);
	}
}

// Decompiler

- (BOOL)canDecompileProcedure:(NSObject<HPProcedure> *)procedure {
	return NO;
}

- (Address)skipHeader:(NSObject<HPBasicBlock> *)basicBlock ofProcedure:(NSObject<HPProcedure> *)procedure {
	return basicBlock.from;
}

- (Address)skipFooter:(NSObject<HPBasicBlock> *)basicBlock ofProcedure:(NSObject<HPProcedure> *)procedure {
	return basicBlock.to;
}

- (ASTNode *)rawDecodeArgumentIndex:(int)argIndex
			   ofDisasm:(DisasmStruct *)disasm
		  ignoringWriteMode:(BOOL)ignoreWrite
		    usingDecompiler:(Decompiler *)decompiler {
	return nil;
}

- (ASTNode *)decompileInstructionAtAddress:(Address)a
				    disasm:(DisasmStruct)d
				 addNode_p:(BOOL *)addNode_p
			   usingDecompiler:(Decompiler *)decompiler {
	return nil;
}

// Assembler

- (NSData *)assembleRawInstruction:(NSString *)instr atAddress:(Address)addr forFile:(NSObject<HPDisassembledFile> *)file withCPUMode:(uint8_t)cpuMode usingSyntaxVariant:(NSUInteger)syntax error:(NSError **)error {
	return nil;
}

- (BOOL)instructionCanBeUsedToExtractDirectMemoryReferences:(DisasmStruct *)disasmStruct {
	return YES;
}

- (BOOL)instructionMayBeASwitchStatement:(DisasmStruct *)disasmStruct {
	return NO;
}


@end
