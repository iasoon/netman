include config.mk

TARGET := netman
SRCS   := $(wildcard *.c)
OBJS   := $(SRCS:.c=.o)

.PHONY: all clean distclean options

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LINK.c) $(OBJS) -o $(TARGET)

clean:
	@echo "Cleaning"
	@- $(RM) $(TARGET)
	@- $(RM) $(OBJS)

distclean: clean
