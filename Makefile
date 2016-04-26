include config.mk

TARGET := netman
SRCS   := $(wildcard *.c)
OBJS   := $(SRCS:.c=.o)

.PHONY: all install clean distclean options

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LINK.c) $(OBJS) -o $(TARGET)

clean:
	@echo "Cleaning"
	@- $(RM) $(TARGET)
	@- $(RM) $(OBJS)

distclean: clean

install:
	@echo "Install"

make_tests:
	@echo "Build tests"

test:
	@echo "Run tests"
