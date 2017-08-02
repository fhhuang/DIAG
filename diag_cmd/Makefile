#include $(TOPDIR)/config.mk

####################################################################
# Tools
#####################################################################
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib
STRIP=$(CROSS_COMPILE)strip

ifeq ($(FOX_KERNEL_FLAGS), )
FOX_KERNEL_EXT_FLAGS = 
export FOX_KERNEL_EXT_FLAGS
else
FOX_KERNEL_EXT_FLAGS = -DFOX_KERNEL -DFOX_NAND_SUPPORT -DCPU_LE -fno-strict-aliasing
#FOX_KERNEL_EXT_FLAGS = -DFOX_KERNEL -Wall
export FOX_KERNEL_EXT_FLAGS
endif

ifeq ($(DIAG_DEBUG_FLAGS), )
FOX_DEBUG_FLAGS = -g -O2
export FOX_DEBUG_FLAGS
else
FOX_DEBUG_FLAGS = -DFOX_KERNEL -DFOX_NAND_SUPPORT -DCPU_LE -fno-strict-aliasing -g -O0
export FOX_DEBUG_FLAGS
endif

export FOX_CMD_ROOT_DIR=$(shell pwd)
#export KERNEL_DIR=$(shell pwd)/../MSYS/msys_lsp_3_4/linux-3.4.69/
export FOX_CMD_DIR=$(shell pwd)/fox/cmd
export FOX_HAL_DIR=$(shell pwd)/fox/hal
export FOX_GENERIC_DIR=$(shell pwd)/fox/generic

####################################################################
# Objects
#####################################################################
DIAG_VER_HEADER_FILE = fox_diagVer.h
DIAG_VERSION = $(shell cat ./version.txt)

LIBS = foxcmd.o

#common/gnu_getopt.o

GENERIC_OBJS = foxCli.o foxCommand.o foxCommandHist.o cmdif.o cmdif_ext.o

####################################################################
# include path
#####################################################################
INC_PATH += -I./ -I$(FOX_HAL_DIR)/init -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/sys -I$(FOX_HAL_DIR)/port -I$(FOX_GENERIC_DIR)/port -I$(FOX_HAL_DIR)/mcu -I$(FOX_HAL_DIR)/i2c

INC_LIB_NAME =
####################################################################
# ALL
#####################################################################

CFLAGS += $(FOX_KERNEL_EXT_FLAGS) $(FOX_DEBUG_FLAGS) $(INC_PATH)

ALL_OBJS = $(foreach f,$(GENERIC_OBJS),./$(f))

all: CreateDirectory GenerateFoxDiagVer BuildObject

CreateDirectory:
	@-mkdir -p ./obj

GenerateFoxDiagVer:
	@rm -f $(DIAG_VER_HEADER_FILE)
	@echo "****** Auto-Generate $(DIAG_VER_HEADER_FILE) ******"
	@echo -n "#define FOX_DIAG_VERSION \"" > $(DIAG_VER_HEADER_FILE); \
		echo -n "$(DIAG_VERSION)" >> $(DIAG_VER_HEADER_FILE); \
		echo "\"" >> $(DIAG_VER_HEADER_FILE);
	@rm -f foxCommand.o

BuildObject:
	${MAKE} -C ./fox/cmd/common/kernel
	${MAKE} -C ./fox/cmd/mem/kernel
#	${MAKE} -C ./fox/cmd/sflash/kernel
	${MAKE} -C ./fox/cmd/i2c/kernel
	${MAKE} -C ./fox/hal/i2c/kernel
#	${MAKE} -C ./fox/hal/ioexp/kernel
#	${MAKE} -C ./fox/cmd/ioexp/kernel
	${MAKE} -C ./fox/cmd/switch/kernel
	${MAKE} -C ./fox/hal/port/kernel
	${MAKE} -C ./fox/hal/sys/kernel
#	${MAKE} -C ./fox/hal/net/kernel
	${MAKE} -C ./fox/cmd/gpio/kernel
	${MAKE} -C ./fox/cmd/fan/kernel
	${MAKE} -C ./fox/hal/fan/kernel
	${MAKE} -C ./fox/cmd/led/kernel
	${MAKE} -C ./fox/cmd/eeprom/kernel
	${MAKE} -C ./fox/cmd/poe/kernel
	${MAKE} -C ./fox/hal/poe/kernel
	${MAKE} -C ./fox/hal/usb/kernel
	${MAKE} -C ./fox/cmd/usb/kernel
#	${MAKE} -C ./fox/cmd/nand/kernel
	${MAKE} -C ./fox/hal/init/kernel
	${MAKE} -C ./fox/cmd/temp/kernel
	${MAKE} -C ./fox/cmd/voltage/kernel
	${MAKE} -C ./fox/hal/led/kernel
	${MAKE} -C ./fox/hal/mcu/kernel
	${MAKE} -C ./fox/cmd/mcu/kernel
	${MAKE} -C ./fox/hal/rtc/kernel
	${MAKE} -C ./fox/hal/pcamap/kernel
	${MAKE} -C ./fox/cmd/rtc/kernel
	${MAKE} -C ./fox/cmd/intr/kernel
	${MAKE} -C ./fox/hal/intr/kernel		
	${MAKE} -C ./fox/hal/net/kernel
	@echo Restore generic objects
	@-$(foreach f,$(GENERIC_OBJS),rm -f ./obj/$(f);)
	@echo Build libary
	${MAKE} $(LIBS)
	@echo Save generic objects
	@-$(foreach f,$(ALL_OBJS),mv -f $(f) ./obj/;)

clean:
	${MAKE} -C ./fox/cmd/common/kernel clean
	${MAKE} -C ./fox/cmd/i2c/kernel clean
	${MAKE} -C ./fox/cmd/mem/kernel clean
#	${MAKE} -C ./fox/cmd/sflash/kernel clean
	${MAKE} -C ./fox/cmd/switch/kernel clean
	${MAKE} -C ./fox/hal/port/kernel clean
	${MAKE} -C ./fox/hal/sys/kernel clean
	${MAKE} -C ./fox/hal/i2c/kernel clean
	${MAKE} -C ./fox/hal/poe/kernel clean
	${MAKE} -C ./fox/hal/usb/kernel clean
#	${MAKE} -C ./fox/cmd/nand/kernel clean
	${MAKE} -C ./fox/cmd/poe/kernel clean
	${MAKE} -C ./fox/hal/poe/kernel clean
#	${MAKE} -C ./fox/cmd/ioexp/kernel clean
	${MAKE} -C ./fox/cmd/eeprom/kernel clean
	${MAKE} -C ./fox/cmd/usb/kernel clean
	${MAKE} -C ./fox/cmd/gpio/kernel clean
	${MAKE} -C ./fox/cmd/fan/kernel clean
	${MAKE} -C ./fox/cmd/led/kernel clean
	${MAKE} -C ./fox/cmd/temp/kernel clean
	${MAKE} -C ./fox/cmd/voltage/kernel clean
	${MAKE} -C ./fox/hal/init/kernel clean
	${MAKE} -C ./fox/hal/led/kernel clean
	${MAKE} -C ./fox/hal/mcu/kernel clean
	${MAKE} -C ./fox/cmd/mcu/kernel clean
	${MAKE} -C ./fox/hal/rtc/kernel clean
	${MAKE} -C ./fox/hal/pcamap/kernel clean
	${MAKE} -C ./fox/cmd/rtc/kernel clean
	${MAKE} -C ./fox/cmd/intr/kernel clean
	${MAKE} -C ./fox/hal/intr/kernel clean	
	${MAKE} -C ./fox/hal/net/kernel clean	
	-rm -f $(ALL_OBJS) *.a *.o *.map
	-rm -rf ./obj
	rm -f $(DIAG_VER_HEADER_FILE)
	
$(LIBS): $(ALL_OBJS)
	-rm -f $(LIBS)
	#$(AR) crus $(LIBS) $(ALL_OBJS) $(wildcard ./obj/*.o) $(INC_LIB_NAME)
	$(LD) $(FOX_CPU_ENDIAN) -r -X -N -Map foxcmd.map -o $(LIBS) $(ALL_OBJS) $(wildcard ./obj/*.o) $(INC_LIB_NAME)
	
RestoreGenericObjects:
	@-mv ./obj/* ./
	
$(foreach f,$(GENERIC_OBJS),./$(f)): %.o : ./%.c
	@-mkdir ./$(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(CMD_OBJS),./cmd/$(f)): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(HAL_OBJS),./hal/$(f)): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
