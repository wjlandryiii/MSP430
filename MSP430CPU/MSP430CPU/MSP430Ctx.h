//
//  MSP430Ctx.h
//  MSP430CPU
//
//  Created by wjl on 1/17/16.
//  Copyright © 2016 wjl. All rights reserved.
//


#import <Foundation/Foundation.h>
#import <Hopper/Hopper.h>


@class MSP430CPU;

@interface MSP430Ctx : NSObject<CPUContext>
- (instancetype)initWithCPU:(MSP430CPU *)cpu andFile:(NSObject<HPDisassembledFile> *)file;

@end