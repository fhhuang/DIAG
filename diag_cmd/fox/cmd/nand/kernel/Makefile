ifeq ($(FOX_KERNEL_FLAGS), yes)
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
CFLAGS += -g -O2

####################################################################
# include path
#####################################################################
INC_PATH  = -I$(FOX_GENERIC_DIR)/common -I$(KERNEL_DIR)/include
INC_PATH += -I$(FOX_GENERIC_DIR)/nand -I$(FOX_HAL_DIR)/nand -I$(FOX_HAL_DIR)/init
INC_PATH += -I$(FOX_CMD_ROOT_DIR)

## Include path
CFLAGS += $(INC_PATH)

INC_LIB_NAME = -l$(LIB_NAME) -lcommon
INC_LIB = -L./ -L$(FOX_GENERIC_DIR)/common/kernel

####################################################################
# ALL
#####################################################################
LIB_NAME = nand
SHARED_OBJS	= nand/nand_hal.o

GENERIC_OBJS = nand/nandtest.o

CMD_OBJS	= cmd_nandtest.o

ALLOBJS 	= $(CMD_OBJS) $(foreach f,$(GENERIC_OBJS),./$(f))

BINARYOBJ = nandtest

BINARYOBJ_SOFT =

GENERIC_FILES = 

all: 
	$(foreach f,$(CMD_OBJS),rm -f $(f);)
	
	if [ $(INDEPENDENCE_PROC) == y ]; then \
		make process; \
	else \
		make single_proc; \
	fi

process: $(ALLOBJS) $(SHARED_OBJS)
	@# shared library
	$(CC) -fpic -shared -o./lib$(LIB_NAME).so $(SHARED_OBJS)
	
	cp -f ./lib$(LIB_NAME).so $(FS_DIR)/lib
	$(STRIP) $(FS_DIR)/lib/lib$(LIB_NAME).so
	
	$(CC) $(CFLAGS) -o $(BINARYOBJ) $(ALLOBJS) $(LFLAGS) $(INC_LIB) $(INC_LIB_NAME)
	
	for soft_bin in $(BINARYOBJ_SOFT); do rm -f $$soft_bin ; ln -s $(BINARYOBJ) $$soft_bin ; done
	
	cp -f $(BINARYOBJ) $(DIAG_CMD_DIR)/
	for soft_bin in $(BINARYOBJ_SOFT); do cp -f -P $$soft_bin $(DIAG_CMD_DIR)/ ; done
	
	$(STRIP) $(DIAG_CMD_DIR)/$(BINARYOBJ)

single_proc: $(ALLOBJS) $(SHARED_OBJS)
	@#$(foreach f,$(ALLOBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(ALLOBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)

	@#$(foreach f,$(SHARED_OBJS),mkdir $(FOX_CMD_ROOT_DIR)/obj/$(dir $(f)) -p; cp -rf ./$(f) $(FOX_CMD_ROOT_DIR)/obj/$(f);)
	$(foreach f,$(SHARED_OBJS),cp -f $(f) $(FOX_CMD_ROOT_DIR)/obj/;)

$(CMD_OBJS): %.o : ../%.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(foreach f,$(GENERIC_OBJS),./$(f)): %.o : $(FOX_GENERIC_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@

$(SHARED_OBJS): %.o : $(FOX_HAL_DIR)/%.c
	@-mkdir $(dir $@) -p
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	-rm -f *.o *.so $(BINARYOBJ)
	-rm -rf nand/
	for soft_bin in $(BINARYOBJ_SOFT); do rm -f $$soft_bin ; done
	
	-rm -f $(DIAG_CMD_DIR)/$(BINARYOBJ)
	for soft_bin in $(BINARYOBJ_SOFT); do rm -f $(DIAG_CMD_DIR)/$$soft_bin ; done

	-rm -f $(FS_DIR)/lib/lib$(LIB_NAME).so
endif
