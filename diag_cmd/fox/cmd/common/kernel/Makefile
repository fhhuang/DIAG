#ifeq ($(FOX_KERNEL_FLAGS), yes)
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

CFLAGS += $(FOX_KERNEL_EXT_FLAGS)
CFLAGS += $(FOX_DEBUG_FLAGS)

####################################################################
# include path
#####################################################################
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_GENERIC_DIR)/mem -I$(FOX_HAL_DIR)/mem -I$(FOX_HAL_DIR)/init
INC_PATH += -I$(FOX_CMD_ROOT_DIR)

## Include path
CFLAGS += $(INC_PATH)

####################################################################
# ALL
#####################################################################

GENERIC_OBJS = common/log.o common/utils.o 

ALLOBJS 	= $(CMD_OBJS) $(foreach f,$(GENERIC_OBJS),./$(f)) $(foreach f,$(HAL_OBJS),./$(f))

all: $(ALLOBJS)
	$(foreach f,$(GENERIC_OBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(CMD_OBJS): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(GENERIC_OBJS),./$(f)): %.o : $(FOX_GENERIC_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f common/*.o
	-rm -f $(DIAG_CMD_DIR)/$(BINARYOBJ)

#endif
