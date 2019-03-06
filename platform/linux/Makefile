COPTS := -Os -Wall

INCLUDES := \
	-I. \
	-I$(EGSIPSDK_DIR)/include


CFLAGS = $(COPTS) $(DEFINES) $(INCLUDES)

LDFLAGS := -lpthread -ldl -lm -lresolv 

CC = gcc
AR = ar
RM = rm -f

platform_lib:
