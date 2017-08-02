ifeq ($(FOX_KERNEL_FLAGS), )

include $(TOPDIR)/config.mk

VER_HEADER_FILE = fox_ver.h
BC_VERSION = $(shell cat $(FOXCONN_DIR)/../version.txt)

####################################################################
# Tools
#####################################################################
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
AR=$(CROSS_COMPILE)ar
RANLIB=$(CROSS_COMPILE)ranlib
STRIP=$(CROSS_COMPILE)strip

####################################################################
# Objects
#####################################################################

LIBS = libfox.a

#common/gnu_getopt.o

GENERIC_OBJS = \
			mem/memtest.o \
			common/log.o \
			common/utils.o \
			mgtPort/mgmt.o \
			rtc/rtctest.o
			
CMD_OBJS = \
			init/cmd_foxinit.o \
			mem/cmd_memtest.o \
			sflash/cmd_sflashtest.o	\
			i2c/cmd_i2ctest.o \
			cpld/cmd_cpldtest.o \
			intr/cmd_intrtest.o \
			ioexp/cmd_ioexp.o \
			fan/cmd_fantest.o \
			pcie/cmd_pcietest.o \
			gpio/cmd_gpiotest.o \
			poe/cmd_poetest.o \
			mgtPort/cmd_mgmttest.o \
			rtc/cmd_rtctest.o \
			mcu/cmd_mcutest.o

HAL_OBJS = \
			init/fox_init.o \
			mem/mem_hal.o \
			sflash/sflash_hal.o \
			sflash/sflashtest.o \
			i2c/i2ctest.o i2c/i2c_hal.o i2c/i2c_utils.o \
			ioexp/ioexp_hal.o \
			cpld/cpldtest.o cpld/cpld_hal.o \
			intr/intrtest.o intr/intr_hal.o \
			sys/sys_utils.o \
			gpio/gpio_hal.o \
			hwmonitor/hwmonitor.o \
			fan/fan_hal.o \
			pcie/pcie_hal.o \
			port/port_utils.o \
			poe/poe_diag.o \
			poe/poe_hal.o \
			mgtPort/mgmt_hal.o \
			mcu/mcu_hal.o \
			rtc/rtc_hal.o	

####################################################################
# include path
#####################################################################
INC_PATH += -I./ 
INC_PATH += -I./hal/mem
INC_PATH += -I./hal/sys/ 
INC_PATH += -I./hal/init 
INC_PATH += -I./hal/common/
INC_PATH += -I./hal/sflash/
INC_PATH += -I./hal/i2c/
INC_PATH += -I./hal/ioexp/
INC_PATH += -I./hal/cpld/
INC_PATH += -I./hal/hwmonitor/
INC_PATH += -I./hal/intr/
INC_PATH += -I./hal/gpio/
INC_PATH += -I./hal/fan/
INC_PATH += -I./hal/pcie/
INC_PATH += -I./hal/switch/
INC_PATH += -I./hal/port/
INC_PATH += -I./hal/poe/
INC_PATH += -I./hal/mgtPort/
INC_PATH += -I./hal/mcu/
INC_PATH += -I./hal/rtc/


INC_PATH += -I./generic/common/ -I./generic/com/ -I./generic/flash/ -I./generic/nand/ -I./generic/mem/ -I./generic/rtc/
INC_PATH += -I./generic/switch/ -I./generic/pcie/ -I./generic/port/

# bootloader
INC_PATH += -I$(BL_DIR)/drivers/
####################################################################
# ALL
#####################################################################

#CFLAGS += -Wall -Werror -fstrict-aliasing $(INC_PATH)
CFLAGS += -DFOX_SFLASH_SUPPORT -Wall -fstrict-aliasing $(INC_PATH)

ALL_OBJS = $(foreach f,$(CMD_OBJS),./cmd/$(f)) $(foreach f,$(HAL_OBJS),./hal/$(f)) $(foreach f,$(GENERIC_OBJS),./$(f))
#ALL_OBJS = $(foreach f,$(GENERIC_OBJS),./$(f))

all: GenerateFoxVer RestoreGenericObjects BuildLibary SaveGenericObjects
	
GenerateFoxVer:
	@rm -f $(VER_HEADER_FILE)
	@echo "****** Auto-Generate $(VER_HEADER_FILE) ******"
	@echo -n "#define FOX_BC_VERSION \"" > $(VER_HEADER_FILE); \
		echo -n "$(BC_VERSION)" >> $(VER_HEADER_FILE); \
		echo "\"" >> $(VER_HEADER_FILE);
	@rm -f hal/init/fox_init.o
	
RestoreGenericObjects:
	@echo Restore generic objects
	@-mv -f ./obj/* ./

BuildLibary:
	@echo Build libary
	@make $(LIBS)

SaveGenericObjects:
	@echo Save generic objects
	@-mkdir ./obj
	@-$(foreach f,$(GENERIC_OBJS),mv -f ./$(dir $(f)) ./obj/;)

clean:
	-rm -f $(ALL_OBJS) *.a
	-rm -rf ./obj
	
$(LIBS): $(ALL_OBJS)
	-rm -f $(LIBS)
	$(AR) cr $(LIBS) $(ALL_OBJS)
	
$(foreach f,$(GENERIC_OBJS),./$(f)): %.o : ./generic/%.c
	@-mkdir ./$(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(CMD_OBJS),./cmd/$(f)): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(HAL_OBJS),./hal/$(f)): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

else

#FOX_KERNEL_EXT_FLAGS = -DFOX_KERNEL
FOX_KERNEL_EXT_FLAGS = -DFOX_KERNEL -Wall -Werror

export FOX_KERNEL_EXT_FLAGS

export FOX_CMD_DIR=$(shell pwd)/cmd
export FOX_HAL_DIR=$(shell pwd)/hal
export FOX_GENERIC_DIR=$(shell pwd)/generic
export INDEPENDENCE_PROC=y
export FOX_CMD_ROOT_DIR=./

cmd_dir_list = flash mem pcie cpld intr i2c ioexp switch temp nand sflash mgtPort
generic_dir_list = common
hal_dir_list = port sys

all:
	@for dir in $(hal_dir_list); do make -C $(FOX_HAL_DIR)/$$dir/kernel || exit 1 ; done
	
	@for dir in $(generic_dir_list); do make -C $(FOX_GENERIC_DIR)/$$dir/kernel || exit 1 ; done
	@#$(foreach n,$(cmd_dir_list),make -C $(n)/kernel || exit 1 ;)
	@for dir in $(cmd_dir_list); do make -C $(FOX_CMD_DIR)/$$dir/kernel || exit 1 ; done
	
clean:
	@for dir in $(hal_dir_list); do make clean -C $(FOX_HAL_DIR)/$$dir/kernel || exit 1 ; done
	
	@for dir in $(generic_dir_list); do make clean -C $(FOX_GENERIC_DIR)/$$dir/kernel || exit 1 ; done
	@#$(foreach n,$(cmd_dir_list),make clean -C $(n) || exit 1 ;)
	@for dir in $(cmd_dir_list); do make clean -C $(FOX_CMD_DIR)/$$dir/kernel || exit 1 ; done

endif
