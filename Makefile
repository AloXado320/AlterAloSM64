# Makefile to rebuild SM64 split image

include util.mk

# Default target
default: all

# Preprocessor definitions
DEFINES :=

#==============================================================================#
# Build Options                                                                #
#==============================================================================#

# These options can either be set by building with 'make SETTING=value'.
# 'make clean' may be required first.

# Build for the N64 (turn this off for ports)
TARGET_N64 ?= 1

# Various workarounds for weird and/or old toolchains
CPP_ASSEMBLY ?= 0

# COMPILER - selects the C compiler to use
#   gcc - uses the GNU C Compiler
#   clang - uses clang C/C++ frontend for LLVM
COMPILER ?= gcc
$(eval $(call validate-option,COMPILER,gcc clang))

# VERSION - selects the version of the game to build
#   jp - builds the 1996 Japanese version
#   us - builds the 1996 North American version
#   eu - builds the 1997 PAL version
#   sh - builds the 1997 Japanese Shindou version, with rumble pak support
#   cn - builds the 2003 Chinese iQue version
VERSION ?= us
$(eval $(call validate-option,VERSION,jp us eu sh cn))

ifeq      ($(VERSION),jp)
  DEFINES   += VERSION_JP=1
  AUDIO_SRC_DIR  ?= src/audio/us_jp
else ifeq ($(VERSION),us)
  DEFINES   += VERSION_US=1
  AUDIO_SRC_DIR  ?= src/audio/us_jp
else ifeq ($(VERSION),eu)
  DEFINES   += VERSION_EU=1
  AUDIO_SRC_DIR  ?= src/audio/eu
else ifeq ($(VERSION),sh)
  DEFINES   += VERSION_SH=1
  AUDIO_SRC_DIR  ?= src/audio/sh
else ifeq ($(VERSION),cn)
  DEFINES   += VERSION_CN=1
  AUDIO_SRC_DIR  ?= src/audio/sh
endif

# GRUCODE - selects which RSP microcode to use.
#   f3dex   -
#   f3dex2  -
#   f3dzex  - newer, experimental microcode used in Animal Crossing
GRUCODE ?= f3dex
$(eval $(call validate-option,GRUCODE, f3dex f3dex2 f3dzex))

ifeq ($(GRUCODE),f3dex) # Fast3DEX
  DEFINES += F3DEX_GBI=1 F3DEX_GBI_SHARED=1
else ifeq ($(GRUCODE), f3dex2) # Fast3DEX2
  DEFINES += F3DEX_GBI_2=1 F3DEX_GBI_SHARED=1
else ifeq ($(GRUCODE),f3dzex) # Fast3DZEX (2.0J / Animal Forest - Dōbutsu no Mori)
  $(warning Fast3DZEX is experimental. Try at your own risk.)
  DEFINES += F3DZEX_GBI_2=1 F3DEX_GBI_2=1 F3DEX_GBI_SHARED=1
endif

TARGET := sm64.$(VERSION).$(GRUCODE)

# Mandatory defines to ensture correct compilation
DEFINES += NON_MATCHING=1 AVOID_UB=1

ifeq ($(COMPILER),gcc)
  MIPSISET     := -mips3
  OPT_FLAGS    := -O2
else ifeq ($(COMPILER),clang)
# clang doesn't support ABI 'o32' for 'mips3'
  MIPSISET     := -mips2
  OPT_FLAGS    ?= -O2
endif

# Whether to hide commands or not
VERBOSE ?= 0
ifeq ($(VERBOSE),0)
  V := @
endif

DEFINES += _FINALROM=1

ifeq ($(TARGET_N64),1)
  DEFINES += TARGET_N64=1
endif

#==============================================================================#
# Universal Dependencies                                                       #
#==============================================================================#

TOOLS_DIR := tools
PYTHON := python3

ifeq ($(filter all default,$(MAKECMDGOALS)),$(MAKECMDGOALS))
  ifeq ($(MAKE_RESTARTS),)
    # Extract Assets
    NOEXTRACT ?= 0
    ifeq ($(NOEXTRACT),0)
      $(eval $(call run_tool, $(PYTHON) extract_assets.py $(VERSION), Failed to extract assets))
    endif

    # Build General Tools
    $(info Building general tools...)
    $(eval $(call run_tool, $(MAKE) -s -C $(TOOLS_DIR), Failed to build tools))

    # Build sm64tools
    $(info Building sm64tools...)
    $(eval $(call run_tool, $(MAKE) -s -C $(TOOLS_DIR)/sm64tools, Failed to build sm64tools))

    $(info Building ROM...)
  endif
endif

#==============================================================================#
# Target Executable and Sources                                                #
#==============================================================================#

BUILD_DIR_BASE := build
# BUILD_DIR is the location where all build artifacts are placed
BUILD_DIR      := $(BUILD_DIR_BASE)/$(VERSION)
ROM            := $(BUILD_DIR)/$(TARGET).z64
ELF            := $(BUILD_DIR)/$(TARGET).elf
LD_SCRIPT      := sm64.ld
CHARMAP        := charmap.txt
CHARMAP_DEBUG  := charmap.debug.txt
MIO0_DIR       := $(BUILD_DIR)/bin
SOUND_BIN_DIR  := $(BUILD_DIR)/sound
TEXTURE_DIR    := textures
ACTOR_DIR      := actors
LEVEL_DIRS     := $(patsubst levels/%,%,$(dir $(wildcard levels/*/header.h)))

# Directories containing source files
SRC_DIRS := src src/boot src/engine src/game src/game/ui src/menu src/buffers src/audio $(AUDIO_SRC_DIR) actors levels bin data assets asm lib sound
BIN_DIRS := bin bin/$(VERSION)

# File dependencies and variables for specific files
include Makefile.split

# Source code files
LEVEL_C_FILES     := $(wildcard levels/*/leveldata.c) $(wildcard levels/*/script.c) $(wildcard levels/*/geo.c)
C_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c)) $(LEVEL_C_FILES)
S_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))
GENERATED_C_FILES := $(BUILD_DIR)/assets/mario_anim_data.c $(BUILD_DIR)/assets/demo_data.c

# Sound files
SOUND_BANK_FILES    := $(wildcard sound/sound_banks/*.json)
SOUND_SAMPLE_DIRS   := $(wildcard sound/samples/*)
SOUND_SAMPLE_AIFFS  := $(foreach dir,$(SOUND_SAMPLE_DIRS),$(wildcard $(dir)/*.aiff))
SOUND_SAMPLE_TABLES := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.table))
SOUND_SAMPLE_AIFCS  := $(foreach file,$(SOUND_SAMPLE_AIFFS),$(BUILD_DIR)/$(file:.aiff=.aifc))
ifeq ($(VERSION),cn)
  SOUND_SEQUENCE_DIRS := sound/sequences sound/sequences/sh
else
  SOUND_SEQUENCE_DIRS := sound/sequences sound/sequences/$(VERSION)
endif
# all .m64 files in SOUND_SEQUENCE_DIRS, plus all .m64 files that are generated from .s files in SOUND_SEQUENCE_DIRS
SOUND_SEQUENCE_FILES := \
  $(foreach dir,$(SOUND_SEQUENCE_DIRS),\
    $(wildcard $(dir)/*.m64) \
    $(foreach file,$(wildcard $(dir)/*.s),$(BUILD_DIR)/$(file:.s=.m64)) \
  )

# Object files
O_FILES := $(foreach file,$(C_FILES),$(BUILD_DIR)/$(file:.c=.o)) \
           $(foreach file,$(S_FILES),$(BUILD_DIR)/$(file:.s=.o)) \
           $(foreach file,$(GENERATED_C_FILES),$(file:.c=.o))

# Automatic dependency files
DEP_FILES := $(O_FILES:.o=.d) $(BUILD_DIR)/$(LD_SCRIPT).d

#==============================================================================#
# Compiler Options                                                             #
#==============================================================================#

# Detect prefix for MIPS toolchain
CROSS := $(call find-mips-toolchain)

ifeq ($(COMPILER),gcc)
  CC      := $(CROSS)gcc
else ifeq ($(COMPILER),clang)
  CC      := clang
endif
AS            := $(CROSS)as
LD            := $(CROSS)ld
AR            := $(CROSS)ar
OBJDUMP       := $(CROSS)objdump
OBJCOPY       := $(CROSS)objcopy

INCLUDE_DIRS := include $(BUILD_DIR) $(BUILD_DIR)/include src .
ifeq ($(TARGET_N64),1)
  INCLUDE_DIRS += include/gcc
endif

ifeq ($(TARGET_N64),1)
  TARGET_CFLAGS := -nostdinc -D_LANGUAGE_C
endif

# Process libraries
LIBS :=
AR_LIBS :=
include mk/lib/libgoddard.mk
include mk/lib/libultra.mk
include mk/lib/libgcc.mk
LINK_LIBS := $(foreach i,$(LIBS),-l$(i))

# C defines
C_DEFINES := $(foreach d,$(DEFINES),-D$(d))
DEF_INC_CFLAGS := $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(C_DEFINES)

# Prefer clang as C preprocessor if installed on the system
ifneq (,$(call find-command,clang))
  CPP      := clang
  CPPFLAGS := -E -P -x c -Wno-trigraphs $(DEF_INC_CFLAGS)
else
  CPP      := cpp
  CPPFLAGS := -P -Wno-trigraphs $(DEF_INC_CFLAGS)
endif

# C compiler options
CFLAGS = -G 0 $(TARGET_CFLAGS) $(DEF_INC_CFLAGS)
ifeq ($(COMPILER),gcc)
  CFLAGS += -mno-shared -march=vr4300 -mfix4300 -mabi=32 -mhard-float -mdivide-breaks -fno-stack-protector -fno-common -fno-zero-initialized-in-bss -fno-PIC -mno-abicalls -fno-strict-aliasing -fno-inline-functions -ffreestanding -fwrapv -Wall -Wextra
else ifeq ($(COMPILER),clang)
  CFLAGS += -target mips -mabi=32 -mhard-float -fomit-frame-pointer -fno-stack-protector -fno-common -I include -I src/ -I $(BUILD_DIR)/include -fno-PIC -mno-abicalls -fno-strict-aliasing -fno-inline-functions -ffreestanding -fwrapv -Wall -Wextra
endif

AS_DEFINES := $(foreach d,$(DEFINES), $(if $(call is_numeric,$(lastword $(subst =, ,$(d)))),$(d)))

ifeq ($(CPP_ASSEMBLY),1)
  ASFLAGS  := -march=vr4300 -mabi=32 $(foreach i,$(INCLUDE_DIRS),-I$(i)) $(foreach d,$(AS_DEFINES),--defsym $(d))
else
  ASMFLAGS := -G 0 $(DEF_INC_CFLAGS) -w -nostdinc -c -march=vr4300 -mfix4300 -mno-abicalls -DMIPSEB -D_MIPS_SIM=1 -D_MIPS_SZLONG=32 $(foreach i,$(INCLUDE_DIRS),-Wa,-I$(i))
endif

RSPASMFLAGS := $(foreach d,$(AS_DEFINES),-definelabel $(subst =, ,$(d)))

SYMLINKFLAGS := --no-check-sections $(addprefix -R ,$(SEG_FILES))
LDFLAGS := -T $(BUILD_DIR)/$(LD_SCRIPT) -Map $(BUILD_DIR)/sm64.$(VERSION).map $(SYMLINKFLAGS)

#==============================================================================#
# Miscellaneous Tools                                                          #
#==============================================================================#

# N64 tools
MIO0TOOL              := $(TOOLS_DIR)/sm64tools/mio0
N64CKSUM              := $(TOOLS_DIR)/sm64tools/n64cksum
N64GRAPHICS           := $(TOOLS_DIR)/sm64tools/n64graphics
N64GRAPHICS_CI        := $(TOOLS_DIR)/sm64tools/n64graphics_ci
TEXTCONV              := $(TOOLS_DIR)/textconv
TABLEDESIGN           := $(TOOLS_DIR)/tabledesign
VADPCM_ENC            := $(TOOLS_DIR)/vadpcm_enc
SKYCONV               := $(TOOLS_DIR)/skyconv
# Use the system installed armips if available. Otherwise use the one provided with this repository.
ifneq (,$(call find-command,armips))
  RSPASM              := armips
else
  RSPASM              := $(TOOLS_DIR)/armips
endif
ENDIAN_BITWIDTH       := $(BUILD_DIR)/endian-and-bitwidth

EMULATOR = mupen64plus
EMU_FLAGS = --noosd
LOADER = loader64
LOADER_FLAGS = -vwf
SHA1SUM = sha1sum

#==============================================================================#
# Main Targets                                                                 #
#==============================================================================#

all: $(ROM)
	@$(SHA1SUM) $(ROM)
	@$(PRINT) "${REVERSE}Build succeeded.$(NO_COL)\n"
	@$(PRINT) "${B_WHITE}==== Build Options ====$(NO_COL)\n"
	@$(PRINT) "${B_GREEN}Version:        $(B_CYAN)$(VERSION)$(NO_COL)\n"
	@$(PRINT) "${B_GREEN}Microcode:      $(B_CYAN)$(GRUCODE)$(NO_COL)\n"

clean:
	$(RM) -r $(BUILD_DIR_BASE)

distclean: clean
	$(PYTHON) extract_assets.py --clean
	$(MAKE) -C $(TOOLS_DIR) clean
	$(MAKE) -C $(TOOLS_DIR)/sm64tools clean

test: $(ROM)
	$(EMULATOR) $(EMU_FLAGS) $<

load: $(ROM)
	$(LOADER) $(LOADER_FLAGS) $<

# Extra object file dependencies
$(BUILD_DIR)/asm/ipl3_font.o:         $(IPL3_RAW_FILES)
$(BUILD_DIR)/src/boot/crash_screen.o: $(CRASH_TEXTURE_C_FILES)
$(BUILD_DIR)/lib/rsp.o:               $(BUILD_DIR)/rsp/rspboot.bin $(BUILD_DIR)/rsp/audio.bin
$(SOUND_BIN_DIR)/sound_data.o:        $(SOUND_BIN_DIR)/sound_data.ctl.inc.c $(SOUND_BIN_DIR)/sound_data.tbl.inc.c $(SOUND_BIN_DIR)/sequences.bin.inc.c $(SOUND_BIN_DIR)/bank_sets.inc.c
$(BUILD_DIR)/levels/scripts.o:        $(BUILD_DIR)/include/level_headers.h

$(BUILD_DIR)/src/audio/sh/load.o: $(SOUND_BIN_DIR)/bank_sets.inc.c $(SOUND_BIN_DIR)/sequences_header.inc.c $(SOUND_BIN_DIR)/ctl_header.inc.c $(SOUND_BIN_DIR)/tbl_header.inc.c

$(CRASH_TEXTURE_C_FILES): TEXTURE_ENCODING := u32

ifeq ($(VERSION),eu)
  TEXT_DIRS := text/de text/us text/fr

  # EU encoded text inserted into individual segment 0x19 files,
  # and course data also duplicated in leveldata.c
  $(BUILD_DIR)/bin/eu/translation_en.o: $(BUILD_DIR)/text/us/define_text.inc.c
  $(BUILD_DIR)/bin/eu/translation_de.o: $(BUILD_DIR)/text/de/define_text.inc.c
  $(BUILD_DIR)/bin/eu/translation_fr.o: $(BUILD_DIR)/text/fr/define_text.inc.c
  $(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/include/text_strings.h
  $(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/us/define_courses.inc.c
  $(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/de/define_courses.inc.c
  $(BUILD_DIR)/levels/menu/leveldata.o: $(BUILD_DIR)/text/fr/define_courses.inc.c
else
  ifeq ($(VERSION),sh)
    TEXT_DIRS := text/jp
    $(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/jp/define_text.inc.c
  else
    TEXT_DIRS := text/$(VERSION)
    # non-EU encoded text inserted into segment 0x02
    $(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/$(VERSION)/define_text.inc.c
  endif
endif
$(BUILD_DIR)/bin/segment2.o: $(BUILD_DIR)/text/debug_text.raw.inc.c

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS) $(GODDARD_SRC_DIRS) $(ULTRA_SRC_DIRS) $(LIBGCC_SRC_DIRS) $(BIN_DIRS) $(TEXTURE_DIRS) $(VERSION_ASSET_DIRS) $(TEXT_DIRS) $(SOUND_SAMPLE_DIRS) $(addprefix levels/,$(LEVEL_DIRS)) rsp include) $(MIO0_DIR) $(addprefix $(MIO0_DIR)/,$(VERSION)) $(SOUND_BIN_DIR) $(SOUND_BIN_DIR)/sequences/$(VERSION)

# Make sure build directory exists before compiling anything
DUMMY != mkdir -p $(ALL_DIRS)

$(BUILD_DIR)/include/text_strings.h: $(BUILD_DIR)/include/text_menu_strings.h
$(BUILD_DIR)/src/menu/file_select.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/menu/star_select.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/game/ui/pause_menu.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/game/ui/course_complete.o: $(BUILD_DIR)/include/text_strings.h
$(BUILD_DIR)/src/game/ui/cutscene_text.o: $(BUILD_DIR)/include/text_strings.h


#==============================================================================#
# Texture Generation                                                           #
#==============================================================================#
TEXTURE_ENCODING := u8

# Convert PNGs to RGBA32, RGBA16, IA16, IA8, IA4, IA1, I8, I4 binary files
$(BUILD_DIR)/%: %.png
	$(call print,Converting:,$<,$@)
	$(V)$(N64GRAPHICS) -s raw -i $@ -g $< -f $(lastword $(subst ., ,$@))

$(BUILD_DIR)/%.inc.c: %.png
	$(call print,Converting:,$<,$@)
	$(V)$(N64GRAPHICS) -s $(TEXTURE_ENCODING) -i $@ -g $< -f $(lastword ,$(subst ., ,$(basename $<)))

# Color Index CI8
$(BUILD_DIR)/%.ci8: %.ci8.png
	$(call print,Converting:,$<,$@)
	$(V)$(N64GRAPHICS_CI) -i $@ -g $< -f ci8

# Color Index CI4
$(BUILD_DIR)/%.ci4: %.ci4.png
	$(call print,Converting:,$<,$@)
	$(V)$(N64GRAPHICS_CI) -i $@ -g $< -f ci4


#==============================================================================#
# Compressed Segment Generation                                                #
#==============================================================================#

# Link segment file to resolve external labels
# TODO: ideally this would be `-Trodata-segment=0x07000000` but that doesn't set the address
$(BUILD_DIR)/%.elf: $(BUILD_DIR)/%.o
	$(call print,Linking ELF file:,$<,$@)
	$(V)$(LD) -e 0 -Ttext=$(SEGMENT_ADDRESS) -Map $@.map -o $@ $<
# Override for leveldata.elf, which otherwise matches the above pattern.
# Has to be a static pattern rule for make-4.4 and above to trigger the second
# expansion.
.SECONDEXPANSION:
$(LEVEL_ELF_FILES): $(BUILD_DIR)/levels/%/leveldata.elf: $(BUILD_DIR)/levels/%/leveldata.o $(BUILD_DIR)/bin/$$(TEXTURE_BIN).elf
	$(call print,Linking ELF file:,$<,$@)
	$(V)$(LD) -e 0 -Ttext=$(SEGMENT_ADDRESS) -Map $@.map --just-symbols=$(BUILD_DIR)/bin/$(TEXTURE_BIN).elf -o $@ $<

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(call print,Extracting compressible data from:,$<,$@)
	$(V)$(OBJCOPY) -O binary --only-section=.data $< $@

$(BUILD_DIR)/levels/%/leveldata.bin: $(BUILD_DIR)/levels/%/leveldata.elf
	$(call print,Extracting compressible data from:,$<,$@)
	$(V)$(OBJCOPY) -O binary --only-section=.data $< $@

# Compress binary file
$(BUILD_DIR)/%.mio0: $(BUILD_DIR)/%.bin
	$(call print,Compressing:,$<,$@)
	$(V)$(MIO0TOOL) $< $@

# convert binary mio0 to object file
$(BUILD_DIR)/%.mio0.o: $(BUILD_DIR)/%.mio0
	$(call print,Converting MIO0 to ELF:,$<,$@)
	$(V)$(LD) -r -b binary $< -o $@


#==============================================================================#
# Sound File Generation                                                        #
#==============================================================================#

$(BUILD_DIR)/%.table: %.aiff
	$(call print,Generating ADPCM table:,$<,$@)
	$(V)$(TABLEDESIGN) -s 1 $< >$@

$(BUILD_DIR)/%.aifc: $(BUILD_DIR)/%.table %.aiff
	$(call print,Encoding ADPCM:,$(word 2,$^),$@)
	$(V)$(VADPCM_ENC) -c $^ $@

$(ENDIAN_BITWIDTH): $(TOOLS_DIR)/determine-endian-bitwidth.c
	@$(PRINT) "$(GREEN)Generating endian-bitwidth $(NO_COL)\n"
	$(V)$(CC) -c $(CFLAGS) -o $@.dummy2 $< 2>$@.dummy1; true
	$(V)grep -o 'msgbegin --endian .* --bitwidth .* msgend' $@.dummy1 > $@.dummy2
	$(V)head -n1 <$@.dummy2 | cut -d' ' -f2-5 > $@
	$(V)$(RM) $@.dummy1
	$(V)$(RM) $@.dummy2

$(SOUND_BIN_DIR)/sound_data.ctl: sound/sound_banks/ $(SOUND_BANK_FILES) $(SOUND_SAMPLE_AIFCS) $(ENDIAN_BITWIDTH)
	@$(PRINT) "$(GREEN)Generating:  $(CYAN)$@ $(NO_COL)\n"
	$(V)$(PYTHON) $(TOOLS_DIR)/assemble_sound.py $(BUILD_DIR)/sound/samples/ sound/sound_banks/ $(SOUND_BIN_DIR)/sound_data.ctl $(SOUND_BIN_DIR)/ctl_header $(SOUND_BIN_DIR)/sound_data.tbl $(SOUND_BIN_DIR)/tbl_header $(C_DEFINES) $$(cat $(ENDIAN_BITWIDTH))

$(SOUND_BIN_DIR)/sound_data.tbl: $(SOUND_BIN_DIR)/sound_data.ctl
	@true

$(SOUND_BIN_DIR)/ctl_header: $(SOUND_BIN_DIR)/sound_data.ctl
	@true

$(SOUND_BIN_DIR)/tbl_header: $(SOUND_BIN_DIR)/sound_data.ctl
	@true

$(SOUND_BIN_DIR)/sequences.bin: $(SOUND_BANK_FILES) sound/sequences.json $(SOUND_SEQUENCE_DIRS) $(SOUND_SEQUENCE_FILES) $(ENDIAN_BITWIDTH)
	@$(PRINT) "$(GREEN)Generating:  $(CYAN)$@ $(NO_COL)\n"
	$(V)$(PYTHON) $(TOOLS_DIR)/assemble_sound.py --sequences $@ $(SOUND_BIN_DIR)/sequences_header $(SOUND_BIN_DIR)/bank_sets sound/sound_banks/ sound/sequences.json $(SOUND_SEQUENCE_FILES) $(C_DEFINES) $$(cat $(ENDIAN_BITWIDTH))

$(SOUND_BIN_DIR)/bank_sets: $(SOUND_BIN_DIR)/sequences.bin
	@true

$(SOUND_BIN_DIR)/sequences_header: $(SOUND_BIN_DIR)/sequences.bin
	@true

$(SOUND_BIN_DIR)/%.m64: $(SOUND_BIN_DIR)/%.o
	$(call print,Converting to M64:,$<,$@)
	$(V)$(OBJCOPY) -j .rodata $< -O binary $@


#==============================================================================#
# Generated Source Code Files                                                  #
#==============================================================================#

# Convert binary file to a comma-separated list of byte values for inclusion in C code
$(BUILD_DIR)/%.inc.c: $(BUILD_DIR)/%
	$(call print,Converting to C:,$<,$@)
	$(V)hexdump -v -e '1/1 "0x%X,"' $< > $@
	$(V)echo >> $@

# Generate animation data
$(BUILD_DIR)/assets/mario_anim_data.c: $(wildcard assets/anims/*.inc.c)
	@$(PRINT) "$(GREEN)Generating animation data $(NO_COL)\n"
	$(V)$(PYTHON) $(TOOLS_DIR)/mario_anims_converter.py > $@

# Generate demo input data
$(BUILD_DIR)/assets/demo_data.c: assets/demo_data.json $(wildcard assets/demos/*.bin)
	@$(PRINT) "$(GREEN)Generating demo data $(NO_COL)\n"
	$(V)$(PYTHON) $(TOOLS_DIR)/demo_data_converter.py assets/demo_data.json $(DEF_INC_CFLAGS) > $@

# Encode in-game text strings
$(BUILD_DIR)/$(CHARMAP): $(CHARMAP)
	$(call print,Preprocessing charmap:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DBUILD_DIR=$(BUILD_DIR) -MMD -MP -MT $@ -MF $@.d -o $@ $<
$(BUILD_DIR)/$(CHARMAP_DEBUG): $(CHARMAP)
	$(call print,Preprocessing charmap:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DCHARMAP_DEBUG -DBUILD_DIR=$(BUILD_DIR) -MMD -MP -MT $@ -MF $@.d -o $@ $<
$(BUILD_DIR)/include/text_strings.h: include/text_strings.h.in $(BUILD_DIR)/$(CHARMAP)
	$(call print,Encoding:,$<,$@)
	$(V)$(TEXTCONV) $(BUILD_DIR)/$(CHARMAP) $< $@
$(BUILD_DIR)/include/text_menu_strings.h: include/text_menu_strings.h.in
	$(call print,Encoding:,$<,$@)
	$(V)$(TEXTCONV) charmap_menu.txt $< $@
$(BUILD_DIR)/text/%/define_courses.inc.c: text/define_courses.inc.c text/%/courses.h
	@$(PRINT) "$(GREEN)Preprocessing: $(CYAN)$@ $(NO_COL)\n"
	$(V)$(CPP) $(CPPFLAGS) $< -o - -I text/$*/ | $(TEXTCONV) $(BUILD_DIR)/$(CHARMAP) - $@
$(BUILD_DIR)/text/%/define_text.inc.c: text/define_text.inc.c text/%/courses.h text/%/dialogs.h
	@$(PRINT) "$(GREEN)Preprocessing: $(CYAN)$@ $(NO_COL)\n"
	$(V)$(CPP) $(CPPFLAGS) $< -o - -I text/$*/ | $(TEXTCONV) $(BUILD_DIR)/$(CHARMAP) - $@
$(BUILD_DIR)/text/debug_text.raw.inc.c: text/debug_text.inc.c $(BUILD_DIR)/$(CHARMAP_DEBUG)
	@$(PRINT) "$(GREEN)Preprocessing: $(CYAN)$@ $(NO_COL)\n"
	$(V)$(CPP) $(CPPFLAGS) $< -o - -I text/$*/ | $(TEXTCONV) $(BUILD_DIR)/$(CHARMAP_DEBUG) - $@

# Level headers
$(BUILD_DIR)/include/level_headers.h: levels/level_headers.h.in
	$(call print,Preprocessing level headers:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -I . $< | sed -E 's|(.+)|#include "\1"|' > $@

#==============================================================================#
# Compilation Recipes                                                          #
#==============================================================================#

# Compile C code
$(BUILD_DIR)/%.o: %.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC) -c $(CFLAGS) $(OPT_FLAGS) $(MIPSISET) -o $@ $<

# Assemble assembly code
$(BUILD_DIR)/%.o: %.s
	$(call print,Assembling:,$<,$@)
ifeq ($(CPP_ASSEMBLY),1)
	$(V)$(CPP) $(CPPFLAGS) -D_LANGUAGE_ASSEMBLY $< | $(AS) $(ASFLAGS) -MD $(BUILD_DIR)/$*.d -o $@
else
	$(V)$(CROSS)gcc -c $(ASMFLAGS) -D_LANGUAGE_ASSEMBLY -x assembler-with-cpp -MMD -MF $(BUILD_DIR)/$*.d -o $@ $<
endif

# Assemble RSP assembly code
$(BUILD_DIR)/rsp/%.bin $(BUILD_DIR)/rsp/%_data.bin: rsp/%.s
	$(call print,Assembling:,$<,$@)
	$(V)$(RSPASM) -sym $@.sym $(RSPASMFLAGS) -strequ CODE_FILE $(BUILD_DIR)/rsp/$*.bin -strequ DATA_FILE $(BUILD_DIR)/rsp/$*_data.bin $<

# Run linker script through the C preprocessor
$(BUILD_DIR)/$(LD_SCRIPT): $(LD_SCRIPT) $(BUILD_DIR)/goddard.txt
	$(call print,Preprocessing linker script:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DBUILD_DIR=$(BUILD_DIR) -MMD -MP -MT $@ -MF $@.d -o $@ $<

# SS2: Goddard rules to get size
$(BUILD_DIR)/sm64_prelim.ld: $(LD_SCRIPT) $(O_FILES) $(MIO0_OBJ_FILES) $(SEG_FILES) $(AR_LIBS)
	$(call print,Preprocessing preliminary linker script:,$<,$@)
	$(V)$(CPP) $(CPPFLAGS) -DPRELIMINARY=1 -DBUILD_DIR=$(BUILD_DIR) -MMD -MP -MT $@ -MF $@.d -o $@ $<

$(BUILD_DIR)/sm64_prelim.elf: $(BUILD_DIR)/sm64_prelim.ld
	@$(PRINT) "$(GREEN)Linking Preliminary ELF file: $(CYAN)$@ $(NO_COL)\n"
    # Slightly edited version of LDFLAGS
	$(V)$(LD) -L $(BUILD_DIR) -T $< -Map $(BUILD_DIR)/sm64_prelim.map $(SYMLINKFLAGS) -o $@ $(O_FILES) $(LINK_LIBS)

$(BUILD_DIR)/goddard.txt: $(BUILD_DIR)/sm64_prelim.elf
	$(call print,Getting Goddard size...)
	$(V)$(PYTHON) $(TOOLS_DIR)/get_goddard_size.py $(BUILD_DIR)/sm64_prelim.map $(BUILD_DIR)

# Link SM64 ELF file
$(ELF): $(BUILD_DIR)/sm64_prelim.elf $(O_FILES) $(MIO0_OBJ_FILES) $(SEG_FILES) $(BUILD_DIR)/$(LD_SCRIPT) $(AR_LIBS)
	@$(PRINT) "$(GREEN)Linking ELF file:  $(CYAN)$@ $(NO_COL)\n"
	$(V)$(LD) -L $(BUILD_DIR) $(LDFLAGS) $(BUILD_DIR)/goddard.txt -o $@ $(O_FILES) $(LINK_LIBS)

# Build ROM
$(ROM): $(ELF)
	$(call print,Building ROM:,$<,$@)
	$(V)$(OBJCOPY) --pad-to=0x800000 --gap-fill=0xFF $< $@ -O binary
	$(V)$(N64CKSUM) $@

.PHONY: all clean distclean default test load libultra libgoddard libgcc
# with no prerequisites, .SECONDARY causes no intermediate target to be removed
.SECONDARY:

# Remove built-in rules, to improve performance
MAKEFLAGS += --no-builtin-rules

-include $(DEP_FILES)

print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
