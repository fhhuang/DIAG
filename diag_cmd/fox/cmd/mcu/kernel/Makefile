####################################################################
# Tools
#####################################################################
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip

CFLAGS += $(FOX_KERNEL_EXT_FLAGS)
CFLAGS += $(FOX_DEBUG_FLAGS)
#CFLAGS += -g -O2

####################################################################
# include path
#####################################################################
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/mcu -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_GENERIC_DIR)/port -I$(FOX_HAL_DIR)/port -I$(FOX_HAL_DIR)/i2c
INC_PATH += -I$(FOX_CMD_ROOT_DIR) -I$(FOX_HAL_DIR)/sys  -I$(FOX_HAL_DIR)/init

## Include path
CFLAGS += $(INC_PATH)

####################################################################
# ALL
#####################################################################

MCU_CMD_OBJS = cmd_mcutest.o

MCU_ALLOBJS  = $(foreach f,$(MCU_CMD_OBJS),./$(f))

#--------------------------


ALLOBJS = $(MCU_ALLOBJS)

all: $(ALLOBJS)
	
	@#$(foreach f,$(MCU_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(MCU_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(foreach f,$(MCU_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	-rm -f *.o
	-rm -rf mcu/
