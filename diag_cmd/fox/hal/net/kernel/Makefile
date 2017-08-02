####################################################################
# Tools
#####################################################################
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
STRIP = $(CROSS_COMPILE)strip

CFLAGS += $(FOX_KERNEL_EXT_FLAGS) $(BOARD_VER_DEF)
CFLAGS += -g -O2

####################################################################
# include path
#####################################################################
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/switch  -I$(FOX_HAL_DIR)/common
INC_PATH += -I$(FOX_GENERIC_DIR)/port -I$(FOX_HAL_DIR)/port -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_HAL_DIR)/sys -I$(FOX_CMD_ROOT_DIR) -I$(FOX_HAL_DIR)/init
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/cpssEnabler/mainOs/h/
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/cpssEnabler/mainSysConfig/h/appDemo/userExit
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/common/h
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/mainPpDrv/h/
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/mainPpDrv/h/cpss/generic/smi
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/mainPpDrv/h/cpssdriver/pp/hw
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/mainExtUtils/h/
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/mainUT/utfTool/h/
INC_PATH += -I$(BUILD_ROOT_DIR)/cpss/cpssEnabler/mainSysConfig/h/


## Include path
CFLAGS += $(INC_PATH)

INC_LIB_NAME = 
INC_LIB = 

####################################################################
# ALL
#####################################################################
NET_HAL_OBJS	= net/net.o net/tftp.o

ALLOBJS = $(NET_HAL_OBJS)

all: $(ALLOBJS)
	@#$(foreach f,$(ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)

$(NET_HAL_OBJS): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	-rm -f *.o
	-rm -rf net/

