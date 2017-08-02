####################################################################
# Tools
#####################################################################
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip

CFLAGS += $(FOX_KERNEL_EXT_FLAGS) $(BOARD_VER_DEF)
CFLAGS += $(FOX_DEBUG_FLAGS)
#CFLAGS += -g -O2

####################################################################
# include path
#####################################################################
INC_PATH  = -I$(FOX_GENERIC_DIR)/common
INC_PATH += -I$(FOX_GENERIC_DIR)/port
INC_PATH += -I$(FOX_HAL_DIR)/common -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_HAL_DIR)/port
INC_PATH += -I$(FOX_HAL_DIR)/i2c
INC_PATH += -I$(FOX_HAL_DIR)/mcu
INC_PATH += -I$(FOX_HAL_DIR)/switch
INC_PATH += -I$(FOX_HAL_DIR)/mem
INC_PATH += -I$(FOX_HAL_DIR)/gpio
INC_PATH += -I$(FOX_HAL_DIR)/switch
INC_PATH += -I$(FOX_HAL_DIR)/sys
INC_PATH += -I$(FOX_HAL_DIR)/init
INC_PATH += -I$(FOX_CMD_ROOT_DIR)
INC_PATH += -I$(FOX_HAL_DIR)/i2c/act2lib

## Include path
CFLAGS += $(INC_PATH)

INC_LIB_NAME = 
INC_LIB = 

####################################################################
# ALL
#####################################################################
I2C_HAL_OBJS	= i2c/i2c_hal.o i2c/i2ctest.o i2c/i2c_fpga.o i2c/act2test.o i2c/mpp_i2c.o

ALLOBJS = $(I2C_HAL_OBJS)

all: $(ALLOBJS)
	@#$(foreach f,$(ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)

$(I2C_HAL_OBJS): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) $(LIBS) -c $< -o $@
	
clean:
	-rm -f *.o
	-rm -rf i2c/
