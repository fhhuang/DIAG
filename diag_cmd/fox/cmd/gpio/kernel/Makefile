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
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/gpio
INC_PATH += -I$(FOX_HAL_DIR)/init -I$(FOX_HAL_DIR)/i2c
INC_PATH += -I$(FOX_HAL_DIR)/port -I$(KERNEL_DIR)/include -I$(FOX_HAL_DIR)/switch
INC_PATH += -I$(FOX_CMD_ROOT_DIR) -I$(FOX_HAL_DIR)/sys

## Include path
CFLAGS += $(INC_PATH)

####################################################################
# ALL
#####################################################################

I2C_HAL_OBJS	= gpio/gpio_hal.o 
I2C_CMD_OBJS = cmd_gpiotest.o

I2C_ALLOBJS  = $(foreach f,$(I2C_CMD_OBJS),./$(f)) $(foreach f,$(I2C_HAL_OBJS),./$(f))

#--------------------------


ALLOBJS = $(I2C_ALLOBJS) 

all: $(ALLOBJS)
	
	@#$(foreach f,$(I2C_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(I2C_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(foreach f,$(I2C_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(I2C_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o
	-rm -rf gpio/


