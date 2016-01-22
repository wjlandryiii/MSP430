//
//  MSP430CPU.m
//  MSP430CPU
//
//  Created by wjl on 1/17/16.
//  Copyright © 2016 wjl. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "MSP430CPU.h"
#import "MSP430Ctx.h"

@implementation MSP430CPU {
	NSObject<HPHopperServices> *_services;
}

- (instancetype)initWithHopperServices:(NSObject<HPHopperServices> *)services {
	if(self = [super init]){
		_services = services;
	}
	return self;
}

- (NSObject<CPUContext> *)buildCPUContextForFile:(NSObject<HPDisassembledFile> *)file {
	return [[MSP430Ctx alloc]initWithCPU:self andFile:file];
}

- (HopperUUID *)pluginUUID {
	return [_services UUIDWithString:@"7E2BA746-FB0D-485C-934F-7477D0B0F852"];
}

- (HopperPluginType)pluginType {
	return Plugin_CPU;
}

- (NSString *)pluginName {
	return @"MSP430";
}

- (NSString *)pluginDescription {
	return @"MSP430 Microcontroller support";
}

- (NSString *)pluginAuthor {
	return @"Joseph Landry";
}

- (NSString *)pluginCopyright {
	return @"Copyright 2016 Joseph Landry";
}

- (NSArray*)cpuFamilies{
	return @[@"MSP430"];
}

- (NSString *)pluginVersion{
	return @"0.0.1";
}

- (NSArray *)cpuSubFamiliesForFamily:(NSString *)family {
	if([family isEqualToString:@"MSP430"]){
		return @[@"generic"];
	} else {
		return nil;
	}
}

- (int)addressSpaceWidthInBitsForCPUFamily:(NSString *)family andSubFamily:(NSString *)subFamily {
	return 16;
}

- (CPUEndianess)endianess{
	return CPUEndianess_Little;
}

- (NSUInteger)syntaxVariantCount{
	return 1;
}

- (NSUInteger)cpuModeCount{
	return 1;
}

- (NSArray *)syntaxVariantNames {
	return @[@"generic"];
}

- (NSArray *)cpuModeNames {
	return @[@"generic"];
}

- (NSString *)framePointerRegisterNameForFile:(NSObject<HPDisassembledFile> *)file{
	return nil;
}

- (NSUInteger)registerClassCount {
	return 1;
}

- (NSUInteger)registerCountForClass:(RegClass)reg_class {
	return 16;
}

- (BOOL)registerIndexIsStackPointer:(uint32_t)reg ofClass:(RegClass)reg_class {
	if(reg == 1 && reg_class == 0){
		return YES;
	} else {
		return NO;
	}
}

- (BOOL)registerIndexIsFrameBasePointer:(uint32_t)reg ofClass:(RegClass)reg_class {
	return NO;
}

- (BOOL)registerIndexIsProgramCounter:(uint32_t)reg {
	if(reg == 0){
		return YES;
	} else {
		return NO;
	}
}

- (NSString *)registerIndexToString:(int)reg ofClass:(RegClass)reg_class withBitSize:(int)size andPosition:(DisasmPosition)position {
	if(reg_class == 0){
		switch(reg){
			case 0: return @"pc";
			case 1: return @"sp";
			case 2: return @"sr";
			case 3: return @"r3";
			case 4: return @"r4";
			case 5: return @"r5";
			case 6: return @"r6";
			case 7: return @"r7";
			case 8: return @"r8";
			case 9: return @"r9";
			case 10: return @"r10";
			case 11: return @"r11";
			case 12: return @"r12";
			case 13: return @"r13";
			case 14: return @"r14";
			case 15: return @"r15";
			default: return nil;
		}
	}
	return nil;
}

- (NSString *)cpuRegisterStateMaskToString:(uint32_t)cpuState {
	return @"";
}

- (NSUInteger)translateOperandIndex:(NSUInteger)index operandCount:(NSUInteger)count accordingToSyntax:(uint8_t)syntaxIndex {
	return index;
}

- (NSAttributedString *)colorizeInstructionString:(NSAttributedString *)string {
	NSArray *reg_strings = @[@"pc", @"sp", @"sr", @"r3",
					@"r4", @"r5", @"r6", @"r7",
					@"r8", @"r9", @"r10", @"r11",
					@"r12", @"r13", @"r14", @"r15"];
	NSMutableAttributedString *colorized = [string mutableCopy];
	[_services colorizeASMString:colorized
		   operatorPredicate:^BOOL(unichar c) {
			   return (c == '#' || c == '@');
		   }
	       languageWordPredicate:^BOOL(NSString *s) {
		       return [reg_strings containsObject:string];
	       }
	    subLanguageWordPredicate:^BOOL(NSString *s) {
		    return NO;
	    }
	];
	return colorized;
}

- (NSData *)nopWithSize:(NSUInteger)size andMode:(NSUInteger)cpuMode forFile:(NSObject<HPDisassembledFile> *)file {
	if((size & 1) == 0) {
		NSUInteger i;
		NSMutableData *nopArray = [[NSMutableData alloc] initWithCapacity:size];
		[nopArray setLength:size];
		uint16_t *ptr = (uint16_t *)[nopArray mutableBytes];
		for(i = 0; i < size; i += 4){
			//3340 4600
			OSWriteLittleInt16(ptr, i, 0x4033);
			OSWriteLittleInt16(ptr, i+2, 0x0000);
		}
		for(; i < size; i += 2){
			//TODO: byteorder and is there a convention for NOPs?
			OSWriteLittleInt16(ptr, i, 0x4303);
		}
		return [NSData dataWithData:nopArray];
	} else {
		return nil;
	}

}

- (BOOL)canAssembleInstructionsForCPUFamily:(NSString *)family andSubFamily:(NSString *)subFamily {
	return NO;
}

- (BOOL)canDecompileProceduresForCPUFamily:(NSString *)family andSubFamily:(NSString *)subFamily {
	return NO;
}


@end