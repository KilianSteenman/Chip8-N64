V=1
SOURCE_DIR=src
BUILD_DIR=build
PROG_NAME=Chip-8

include $(N64_INST)/include/n64.mk

all: chip-8.z64
.PHONY: all

OBJS = $(BUILD_DIR)/main.o

chip-8.z64: N64_ROM_TITLE="Chip-8"

$(BUILD_DIR)/chip-8.elf: $(OBJS)
$(BUILD_DIR)/chip-8.dfs: $(wildcard filesystem/*)

clean:
	rm -f $(BUILD_DIR)/* *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d)