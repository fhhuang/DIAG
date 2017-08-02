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
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_HAL_DIR)/common -I$(FOX_HAL_DIR)/port -I$(FOX_GENERIC_DIR)/port
INC_PATH += -I$(FOX_HAL_DIR)/sys  -I$(FOX_HAL_DIR)/init -I$(FOX_HAL_DIR)/i2c

## Include path
CFLAGS += $(INC_PATH)

INC_LIB_NAME = 
INC_LIB = 

####################################################################
# ALL
#####################################################################
SYS_HAL_OBJS	= sys/sys_utils.o

ALLOBJS = $(SYS_HAL_OBJS)

all: $(ALLOBJS)
	@#$(foreach f,$(ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)

$(SYS_HAL_OBJS): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	-rm -f *.o
	-rm -rf sys/
