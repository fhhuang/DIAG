#ifeq ($(FOX_KERNEL_FLAGS), yes)
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
INC_PATH  += -I$(FOX_GENERIC_DIR)/common
INC_PATH  += -I$(FOX_GENERIC_DIR)/port
INC_PATH  += -I$(FOX_HAL_DIR)/port
INC_PATH  += -I$(FOX_HAL_DIR)/sys
INC_PATH  += -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/i2c -I$(FOX_HAL_DIR)/gpio
INC_PATH  += -I$(FOX_HAL_DIR)/init -I$(FOX_HAL_DIR)/usb
INC_PATH  += -I$(FOX_CMD_ROOT_DIR)

## Include path
CFLAGS += $(INC_PATH)

####################################################################
# ALL
#####################################################################
USB_HAL_OBJS	= usb_hal.o
USB_CMD_OBJS = 



#ALLOBJS = $(CMN_HAL_OBJS) $(PORT_ALLOBJS) $(SWLB_ALLOBJS) $(MAC_ALLOBJS) $(SMI_ALLOBJS) $(XSMI_ALLOBJS)
ALLOBJS = $(foreach f,$(USB_CMD_OBJS),./$(f)) $(foreach f,$(USB_HAL_OBJS),./$(f))

all: $(ALLOBJS)
	@#$(foreach f,$(ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(foreach f,$(USB_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(USB_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/usb/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	-rm -f *.o
	-rm -rf usb

#endif
