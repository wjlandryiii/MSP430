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

#include "dmsp430/dmsp430.h"

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
	disasm->instruction.mnemonic[0] = 0;
	disasm->instruction.addressValue = 0;
	disasm->instruction.branchType = DISASM_BRANCH_NONE;
	disasm->instruction.condition = DISASM_INST_COND_AL;
	bzero(&disasm->instruction.eflags, sizeof(DisasmEFLAGS));
	bzero(&disasm->prefix, sizeof(DisasmPrefix));
	for (int i=0; i<DISASM_MAX_OPERANDS; i++) {
		disasm->operand[i].type = DISASM_OPERAND_NO_OPERAND;
		disasm->operand[i].immediateValue = 0;
	}

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
	
	if([_file readUInt16AtVirtualAddress:address] == 0x4303){
		return 1;
	} else if([_file readUInt16AtVirtualAddress:address] == 0x4033){
		return 1;
	} else {
		return 0;
	}
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


- (int)disassembleSingleInstruction:(DisasmStruct *)disasm usingProcessorMode:(NSUInteger)mode {
	struct instruction *inst;
	int len, i;
	
	inst = (struct instruction *)calloc(1, sizeof(*inst));
	disasm->instruction.userData = (unsigned long)inst;
	
	len = unpack_instruction(disasm->bytes, disasm->bytes+6, inst);
	
	if(len % 2 != 0){
		NSLog(@"WTF?? %04llx %d", disasm->virtualAddr, len);
	}
	
	if(0 < len){
		strcpy(disasm->instruction.mnemonic, lookup_mnemonic_for_operation(inst->operation));
		
		
		switch(inst->operation){
			case OPER_CALL:
				disasm->instruction.branchType = DISASM_BRANCH_CALL;
				break;
			case OPER_JNE:
				disasm->instruction.branchType = DISASM_BRANCH_JNE;
				break;
			case OPER_JEQ:
				disasm->instruction.branchType = DISASM_BRANCH_JE;
				break;
			case OPER_JNC:
				disasm->instruction.branchType = DISASM_BRANCH_JNC;
				break;
			case OPER_JC:
				disasm->instruction.branchType = DISASM_BRANCH_JC;
				break;
			case OPER_JN:
				disasm->instruction.branchType = DISASM_BRANCH_JL;
				break;
			case OPER_JGE:
				disasm->instruction.branchType = DISASM_BRANCH_JGE;
				break;
			case OPER_JL:
				disasm->instruction.branchType = DISASM_BRANCH_JL;
				break;
			case OPER_JMP:
			case OPER_BR:
				disasm->instruction.branchType = DISASM_BRANCH_JMP;
				break;
			case OPER_RETI:
			case OPER_RET:
				disasm->instruction.branchType = DISASM_BRANCH_RET;
				break;
			default:
				disasm->instruction.branchType = DISASM_BRANCH_NONE;
				break;
		}
		
		for(i = 0; i < inst->noperands; i++){
			string_for_operand(inst->operands[i], disasm->operand[i].mnemonic);
		}
		return len;
	} else {
		return DISASM_UNKNOWN_OPCODE;
	}
}

- (BOOL)instructionHaltsExecutionFlow:(DisasmStruct *)disasm {
	return NO;
}

- (void)performBranchesAnalysis:(DisasmStruct *)disasm
	   computingNextAddress:(Address *)next
		    andBranches:(NSMutableArray *)branches
		   forProcedure:(NSObject<HPProcedure> *)procedure
		     basicBlock:(NSObject<HPBasicBlock> *)basicBlock
		      ofSegment:(NSObject<HPSegment> *)segment
		calledAddresses:(NSMutableArray *)calledAddresses
		      callsites:(NSMutableArray *)callSitesAddresses {
	struct instruction *inst;
	
	inst = (struct instruction *)disasm->instruction.userData;
	
	switch(inst->operation){
		case OPER_BR:
			*next = BAD_ADDRESS;
			[branches addObject:[NSNumber numberWithUnsignedLongLong:inst->operands[0].constant]];
			break;
		case OPER_JMP:
			*next = BAD_ADDRESS;
		case OPER_JNE:
		case OPER_JEQ:
		case OPER_JNC:
		case OPER_JC:
		case OPER_JN:
		case OPER_JGE:
		case OPER_JL:
			if(0 <= (int16_t)inst->operands[0].constant){
				[branches addObject:[NSNumber numberWithUnsignedLongLong:disasm->virtualAddr + inst->operands[0].constant]];
			} else {
				[branches addObject:[NSNumber numberWithUnsignedLongLong:disasm->virtualAddr - inst->operands[0].constant]];
			}
			break;
		
		case OPER_CALL:
			//[calledAddresses addObject:[NSNumber numberWithUnsignedLongLong:inst->operands[0].addr]];
			if(inst->operands[0].mode == OPMODE_IMMEDIATE){
				unsigned long long callto = inst->operands[0].constant;
				[calledAddresses addObject:[NSNumber numberWithUnsignedLongLong:callto]];
			}
			break;
			
	}
	
	NSLog(@"branch analysis: %llx", disasm->virtualAddr);
	
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
			//TODO: figure out how to use symbol labels
			//NSString *sym = [_file nameForVirtualAddress:disasm->operand[i].immediateValue];
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
