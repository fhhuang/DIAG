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
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/poe
INC_PATH += -I$(FOX_GENERIC_DIR)/port -I$(FOX_HAL_DIR)/port -I$(FOX_HAL_DIR)/cpld
INC_PATH += -I$(FOX_CMD_ROOT_DIR) -I$(FOX_HAL_DIR)/sys  -I$(FOX_HAL_DIR)/init

## Include path
CFLAGS += $(INC_PATH)

####################################################################
# ALL
#####################################################################

POE_CMD_OBJS = cmd_poetest.o

POE_ALLOBJS  = $(foreach f,$(POE_CMD_OBJS),./$(f))

#--------------------------


ALLOBJS = $(POE_ALLOBJS) 

all: $(ALLOBJS)
	
	@#$(foreach f,$(POE_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(POE_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(foreach f,$(POE_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	-rm -f *.o
	-rm -rf poe/
