V=1
SOURCE_DIR=src
BUILD_DIR=build
PROG_NAME=Chip-8

include $(N64_INST)/include/n64.mk

all: chip-8.z64
.PHONY: all

OBJS = $(BUILD_DIR)/main.o \
$(BUILD_DIR)/file_utils.o \
$(BUILD_DIR)/chip8.o \
$(BUILD_DIR)/rom.o \
$(BUILD_DIR)/input.o \
$(BUILD_DIR)/screen_rom_select.o \
$(BUILD_DIR)/screen_controller_config.o

chip-8.z64: N64_ROM_TITLE="Chip-8"
chip-8.z64: $(BUILD_DIR)/chip-8.dfs
chip-8.z64: N64_ED64ROMCONFIGFLAGS=-w eeprom16k

$(BUILD_DIR)/chip-8.elf: $(OBJS)
$(BUILD_DIR)/chip-8.dfs: $(wildcard filesystem/*)

clean:
	rm -f $(BUILD_DIR)/* *.z64
.PHONY: clean

-include $(wildcard $(BUILD_DIR)/*.d)