CFLAGS:=-mcpu=cortex-m3 -mthumb
LDFLAGS:= -mcpu=cortex-m3 -mthumb
CROSS_COMPILE_APPS=arm-uclinuxeabi-
CC=$(CROSS_COMPILE_APPS)gcc

