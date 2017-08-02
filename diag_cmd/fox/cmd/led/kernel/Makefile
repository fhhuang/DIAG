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
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/led -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_GENERIC_DIR)/port -I$(FOX_HAL_DIR)/port -I$(FOX_HAL_DIR)/mcu
INC_PATH += -I$(FOX_CMD_ROOT_DIR) -I$(FOX_HAL_DIR)/sys  -I$(FOX_HAL_DIR)/init

## Include path
CFLAGS += $(INC_PATH)

####################################################################
# ALL
#####################################################################

LED_CMD_OBJS = cmd_ledtest.o

LED_ALLOBJS  = $(foreach f,$(LED_CMD_OBJS),./$(f))

#--------------------------


ALLOBJS = $(LED_ALLOBJS) 

all: $(ALLOBJS)
	
	@#$(foreach f,$(LED_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(LED_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(foreach f,$(LED_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	-rm -f *.o
	-rm -rf led/
