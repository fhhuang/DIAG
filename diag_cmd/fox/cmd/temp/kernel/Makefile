####################################################################
# Tools
#####################################################################
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip

ifeq ($(INDEPENDENCE_PROC), y)
CFLAGS += -DINDEPENDENCE_PROC
endif

CFLAGS += $(FOX_KERNEL_EXT_FLAGS) $(BOARD_VER_DEF)
CFLAGS += $(FOX_DEBUG_FLAGS)
#CFLAGS += -g -O2

####################################################################
# include path
#####################################################################
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_GENERIC_DIR)/port
INC_PATH += -I$(FOX_HAL_DIR)/init
INC_PATH += -I$(FOX_HAL_DIR)/temp
INC_PATH += -I$(FOX_HAL_DIR)/mcu -I$(FOX_HAL_DIR)/switch
INC_PATH += -I$(FOX_HAL_DIR)/sys -I$(FOX_HAL_DIR)/port
INC_PATH += -I$(FOX_CMD_ROOT_DIR)

## Include path
CFLAGS += $(INC_PATH)

INC_LIB_NAME = -l$(LIB_NAME) -li2c -lsys_utils
INC_LIB = -L../../i2c/kernel -L$(FOX_HAL_DIR)/sys/kernel
####################################################################
# ALL
#####################################################################
LIB_NAME = temp

SHARED_OBJS	= 

GENERIC_OBJS = 
HAL_OBJS	= temp/tempmonitor.o

CMD_OBJS	= cmd_tempmonitor.o

ALLOBJS 	= $(CMD_OBJS) $(foreach f,$(HAL_OBJS),./$(f))

all: $(ALLOBJS)
	$(foreach f,$(ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(CMD_OBJS): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@
	
$(foreach f,$(HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
$(SHARED_OBJS): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	-rm -f *.o *.so $(BINARYOBJ)
	-rm -rf temp/
	-rm -f $(DIAG_CMD_DIR)/$(BINARYOBJ)
	-rm -f $(FS_DIR)/lib/lib$(LIB_NAME).so

