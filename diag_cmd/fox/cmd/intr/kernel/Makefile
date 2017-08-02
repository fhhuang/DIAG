####################################################################
# Tools
#####################################################################
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip

CFLAGS += $(FOX_KERNEL_EXT_FLAGS)
CFLAGS += -g -O2

####################################################################
# include path
#####################################################################
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/intr
INC_PATH += -I$(FOX_HAL_DIR)/init 
INC_PATH += -I$(FOX_GENERIC_DIR)/port -I$(FOX_HAL_DIR)/port -I$(FOX_HAL_DIR)/cpld -I$(FOX_HAL_DIR)/gpio
INC_PATH += -I$(FOX_CMD_ROOT_DIR) -I$(FOX_HAL_DIR)/sys

## Include path
CFLAGS += $(INC_PATH)

####################################################################
# ALL
#####################################################################

INTR_CMD_OBJS = cmd_intrtest.o

INTR_ALLOBJS  = $(foreach f,$(INTR_CMD_OBJS),./$(f))

#--------------------------


ALLOBJS = $(INTR_ALLOBJS) 

all: $(ALLOBJS)
	
	@#$(foreach f,$(INTR_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(INTR_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(foreach f,$(INTR_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	-rm -f *.o
	-rm -rf intr/
