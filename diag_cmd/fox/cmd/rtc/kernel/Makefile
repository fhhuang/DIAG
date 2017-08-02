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
INC_PATH = -I$(FOX_GENERIC_DIR)/common -I$(FOX_HAL_DIR)/rtc -I$(FOX_GENERIC_DIR)/rtc -I$(FOX_HAL_DIR)/mcu -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_GENERIC_DIR)/port -I$(FOX_HAL_DIR)/port
INC_PATH += -I$(FOX_CMD_ROOT_DIR) -I$(FOX_HAL_DIR)/sys -I$(FOX_HAL_DIR)/init

## Include path
CFLAGS += $(INC_PATH)

####################################################################
# ALL
#####################################################################

RTC_CMD_OBJS = cmd_rtc.o

GENERIC_OBJS = rtc/rtctest.o

RTC_ALLOBJS	= $(GENERIC_OBJS) $(foreach f,$(RTC_CMD_OBJS),./$(f))

#--------------------------


ALLOBJS = $(RTC_ALLOBJS) 

all: $(ALLOBJS)
	
	@#$(foreach f,$(RTC_ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(RTC_ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)
	
$(foreach f,$(RTC_CMD_OBJS),./$(f)): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(GENERIC_OBJS),./$(f)): %.o : $(FOX_GENERIC_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o
	-rm -rf rtc/

