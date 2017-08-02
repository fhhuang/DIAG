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
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/switch -I$(FOX_HAL_DIR)/common
INC_PATH += -I$(FOX_GENERIC_DIR)/port -I$(FOX_GENERIC_DIR)/switch -I$(FOX_HAL_DIR)/port -I$(FOX_HAL_DIR)/mcu
INC_PATH += -I$(FOX_CMD_ROOT_DIR) -I$(FOX_HAL_DIR)/sys  -I$(FOX_HAL_DIR)/init -I$(FOX_HAL_DIR)/i2c
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

####################################################################
# ALL
#####################################################################
CMN_HAL_OBJS = switch/switch_hal.o switch/switch_lb.o

PORT_HAL_OBJS = switch/switch_port.o
PORT_CMD_OBJS = cmd_port.o

PORT_ALLOBJS  = $(foreach f,$(PORT_CMD_OBJS),./$(f)) $(foreach f,$(PORT_HAL_OBJS),./$(f))

#--------------------------
TUNE_HAL_OBJS = switch/switch_tune.o
TUNE_CMD_OBJS = cmd_tune.o

TUNE_ALLOBJS  = $(foreach f,$(TUNE_CMD_OBJS),./$(f)) $(foreach f,$(TUNE_HAL_OBJS),./$(f))

#--------------------------
SWLB_HAL_OBJS = 
SWLB_CMD_OBJS = cmd_swlbtest.o

SWLB_ALLOBJS  = $(foreach f,$(SWLB_CMD_OBJS),./$(f)) $(foreach f,$(SWLB_HAL_OBJS),./$(f))

#--------------------------
#MAC_GENERIC_OBJS = switch/mac_test.o
MAC_GENERIC_OBJS =
MAC_HAL_OBJS = 
MAC_CMD_OBJS = cmd_mac.o

MAC_ALLOBJS  = $(foreach f,$(MAC_CMD_OBJS),./$(f)) $(foreach f,$(MAC_GENERIC_OBJS),./$(f)) $(foreach f,$(MAC_HAL_OBJS),./$(f))

#--------------------------
SMI_GENERIC_OBJS = switch/smi_test.o
#SMI_GENERIC_OBJS =
SMI_HAL_OBJS = 
SMI_CMD_OBJS = cmd_smi.o cmd_smitest.o

SMI_ALLOBJS 	= $(foreach f,$(SMI_CMD_OBJS),./$(f)) $(foreach f,$(SMI_GENERIC_OBJS),./$(f)) $(foreach f,$(SMI_HAL_OBJS),./$(f))

#--------------------------
XSMI_GENERIC_OBJS = 
XSMI_HAL_OBJS =  
#XSMI_CMD_OBJS = cmd_xsmitest.o

XSMI_ALLOBJS 	= $(foreach f,$(XSMI_CMD_OBJS),./$(f)) $(foreach f,$(XSMI_GENERIC_OBJS),./$(f)) $(foreach f,$(XSMI_HAL_OBJS),./$(f))

ALLOBJS = $(CMN_HAL_OBJS) $(PORT_ALLOBJS) $(TUNE_ALLOBJS) $(SWLB_ALLOBJS) $(MAC_ALLOBJS) $(SMI_ALLOBJS) $(XSMI_ALLOBJS)

all: $(ALLOBJS)
	@#$(foreach f,$(CMN_HAL_OBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(CMN_HAL_OBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
	@#$(foreach f,$(PORT_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(PORT_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)

	@#$(foreach f,$(TUNE_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(TUNE_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
	@#$(foreach f,$(SWLB_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(SWLB_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
	@#$(foreach f,$(MAC_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(MAC_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
	@#$(foreach f,$(SMI_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(SMI_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)

	@#$(foreach f,$(XSMI_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(XSMI_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)

$(foreach f,$(CMN_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
$(foreach f,$(PORT_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(PORT_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(TUNE_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(TUNE_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
$(foreach f,$(SWLB_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(SWLB_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(SWLB_GENERIC_OBJS),./$(f)): %.o : $(FOX_GENERIC_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
$(foreach f,$(MAC_CMD_OBJS),./$(f)): %.o : ../%.c
	@$(CC) $(CFLAGS) -c $< -o $@
	
$(foreach f,$(MAC_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(MAC_GENERIC_OBJS),./$(f)): %.o : $(FOX_GENERIC_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(SMI_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(SMI_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
$(foreach f,$(SMI_GENERIC_OBJS),./$(f)): %.o : $(FOX_GENERIC_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(XSMI_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(XSMI_HAL_OBJS),./$(f)): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
$(foreach f,$(XSMI_GENERIC_OBJS),./$(f)): %.o : $(FOX_GENERIC_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o
	-rm -rf switch/
