//
//  decode.h
//  MSP430CPU
//
//  Created by wjl on 1/17/16.
//  Copyright © 2016 wjl. All rights reserved.
//

#ifndef decode_h
#define decode_h

struct msp_inst {
	int format;
	
	union {
		struct single {
			int opcode;
			int bw;
			int ad;
			int reg;
		} single;
		
		struct double_ {
			int opcode;
			int src;
			int ad;
			int bw;
			int as;
			int dst;
		} double_;
		
		struct jump {
			int condition;
			int offset;
		} jump;
	};
};

enum {
	FMT_UNKNOWN = 0,
	FMT_DOUBLE,
	FMT_SINGLE,
	FMT_JUMP,
};

struct msp_inst decode_instruction(uint16_t inst);


#endif /* decode_h */
