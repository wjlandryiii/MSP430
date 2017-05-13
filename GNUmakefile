include $(GNUSTEP_MAKEFILES)/common.make

BUNDLE_NAME = MSP430CPU
MSP430CPU_OBJC_FILES = \
	MSP430CPU/MSP430CPU.m \
	MSP430CPU/MSP430Ctx.m

#NOWARN=-Wno-format -Wno-return-type -Wno-unused-value -Wno-unused-variable -Wno-self-assign
MSP430CPU_OBJCFLAGS=$(NOWARN) \
	-I./MSP430CPU/HopperSDK/include \
	-I${HOME}/GNUstep/Library/ApplicationSupport/Hopper/HopperSDK/include \
	-I./Capstone/include/ \
	-DLINUX

include $(GNUSTEP_MAKEFILES)/bundle.make
