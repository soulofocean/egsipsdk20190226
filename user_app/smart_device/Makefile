default: all

USER_APP_OUT = $(APP_BUILD_DIR)/$(EGSIPSDK_LIB_NAME)_$(USER_APP).out

CSRCS_APP := $(wildcard ./*.c)
OBJS_APP := $(CSRCS_APP:%.c=%.o)

include $(PLATFORM_BUILD_DIR)/Makefile

LIBS:=$(LIB_OUT_DIR)/lib$(EGSIPSDK_LIB_NAME).a

all: $(USER_APP_OUT)

$(USER_APP_OUT): $(LIBS) $(OBJS_APP)
	$(CC) -o $@  $(OBJS_APP) $(LIBS) $(LDFLAGS)
	mkdir -p $(BIN_OUT_DIR)
	cp -f $(USER_APP_OUT) $(BIN_OUT_DIR)/

