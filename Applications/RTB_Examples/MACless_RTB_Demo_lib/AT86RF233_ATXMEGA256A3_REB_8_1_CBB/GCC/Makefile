##############################################################################
# Makefile for the project MACless_RTB_Demo using library
###############################################################################
# $Id: Makefile 33883 2012-11-20 14:14:07Z sschneid $

# Build specific properties
_TAL_TYPE = AT86RF233
_PAL_TYPE = ATXMEGA256A3
_PAL_GENERIC_TYPE = XMEGA
_BOARD_TYPE = REB_8_1_CBB
_RTB_TYPE = RTB_PMU_233R
_HIGHEST_STACK_LAYER = RTB
_RADIO_CHANNEL = 26
_LIB_TYPE = rtb_pmu_233r_macless_noantdiv

# Path variables
## Path to main project directory
MAIN_DIR = ../../../../..
APP_DIR = ../..
PATH_TAL = $(MAIN_DIR)/TAL
PATH_MAC = $(MAIN_DIR)/MAC
PATH_PAL = $(MAIN_DIR)/PAL
PATH_RTB = $(MAIN_DIR)/RTB
PATH_RES = $(MAIN_DIR)/Resources
PATH_GLOB_INC = $(MAIN_DIR)/Includes
PATH_SIO_SUPPORT = $(MAIN_DIR)/Applications/Helper_Files/SIO_Support

## General Flags
PROJECT = MACless_RTB_Demo

MCU = atxmega256a3
TARGET_DIR = .
TARGET = $(TARGET_DIR)/$(PROJECT).elf
CC = avr-gcc

## Options common to compile, link and assembly rules
COMMON = -mmcu=$(MCU)

## Compile options common for all C compilation units.
CFLAGS = $(COMMON)
CFLAGS += -Wall -Werror -g -Wundef -std=c99 -DSIO_HUB -DUART0 -Os
CFLAGS += -DDEBUG=0
CFLAGS += -DBAUD_RATE=38400
CFLAGS += -DENABLE_RTB
CFLAGS += -DRTB_TYPE=$(_RTB_TYPE)
CFLAGS += -DF_CPU=32000000UL
CFLAGS += -DTAL_TYPE=$(_TAL_TYPE)
CFLAGS += -DPAL_GENERIC_TYPE=$(_PAL_GENERIC_TYPE)
CFLAGS += -DPAL_TYPE=$(_PAL_TYPE)
CFLAGS += -DBOARD_TYPE=$(_BOARD_TYPE)
CFLAGS += -DHIGHEST_STACK_LAYER=$(_HIGHEST_STACK_LAYER)
CFLAGS += -DDISABLE_TSTAMP_IRQ=1
CFLAGS += -DANTENNA_DIVERSITY=0
CFLAGS += -DRADIO_CHANNEL=$(_RADIO_CHANNEL)
CFLAGS += -MD -MP -MT $(*F).o -MF dep/$(@F).d

## Assembly specific flags
ASMFLAGS = $(COMMON)
ASMFLAGS += $(CFLAGS)
ASMFLAGS += -x assembler-with-cpp -Wa,-g

## Linker flags
LDFLAGS = $(COMMON) -Wl,-Map=$(PROJECT).map
## Intel Hex file production flags
HEX_FLASH_FLAGS = -R .eeprom

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings

## Include directories for application
INCLUDES = -I $(APP_DIR)/Inc
## Include directories for SIO support
INCLUDES += -I $(PATH_SIO_SUPPORT)/Inc
## Include directories for general includes
INCLUDES += -I $(MAIN_DIR)/Include
## Include directories for resources
INCLUDES += -I $(MAIN_DIR)/Resources/Buffer_Management/Inc/
INCLUDES += -I $(MAIN_DIR)/Resources/Queue_Management/Inc/
## Include directories for MAC
INCLUDES += -I $(MAIN_DIR)/MAC/Inc/
## Include directories for TAL
INCLUDES += -I $(MAIN_DIR)/TAL/Inc/
INCLUDES += -I $(MAIN_DIR)/TAL/$(_TAL_TYPE)/Inc/
## Include directories for PAL
INCLUDES += -I $(MAIN_DIR)/PAL/Inc/
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/Generic/Inc
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Inc/
## Include directories for specific boards type
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/
INCLUDES += -I $(MAIN_DIR)/PAL/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/$(_BOARD_TYPE)
## Include directories for RTB
INCLUDES += -I $(MAIN_DIR)/RTB/Inc/

## Library Directories
LIBDIRS = -L $(MAIN_DIR)/RTB/RTB_Lib/$(_TAL_TYPE)_$(_PAL_TYPE)_$(_BOARD_TYPE)/GCC

## Libraries
LIBS = -l_$(_LIB_TYPE)
LIBS += -lm

## Objects that must be built in order to link
OBJECTS = $(TARGET_DIR)/main.o\
	$(TARGET_DIR)/sio_handler.o\
	$(TARGET_DIR)/pal_uart.o\
	$(TARGET_DIR)/pal_sio_hub.o\
	$(TARGET_DIR)/pal_irq.o\
	$(TARGET_DIR)/pal.o\
	$(TARGET_DIR)/pal_timer.o\
	$(TARGET_DIR)/pal_board.o\
	$(TARGET_DIR)/pal_utils.o\
	$(TARGET_DIR)/pal_trx_access.o\
	$(TARGET_DIR)/bmm.o\
	$(TARGET_DIR)/qmm.o\
	$(TARGET_DIR)/tal.o\
	$(TARGET_DIR)/tal_rx.o\
	$(TARGET_DIR)/tal_tx.o\
	$(TARGET_DIR)/tal_ed.o\
	$(TARGET_DIR)/tal_slotted_csma.o\
	$(TARGET_DIR)/tal_pib.o\
	$(TARGET_DIR)/tal_init.o\
	$(TARGET_DIR)/tal_irq_handler.o\
	$(TARGET_DIR)/tal_pwr_mgmt.o\
	$(TARGET_DIR)/tal_rx_enable.o\
	$(TARGET_DIR)/mac_data_extract_mhr.o\
	$(TARGET_DIR)/rtb.o\
	$(TARGET_DIR)/rtb_api.o\
	$(TARGET_DIR)/rtb_callback_wrapper.o\
	$(TARGET_DIR)/rtb_dispatcher.o\
	$(TARGET_DIR)/rtb_hw_233r_xmega.o\
	$(TARGET_DIR)/rtb_pib.o\
	$(TARGET_DIR)/rtb_rx.o\
	$(TARGET_DIR)/rtb_tx.o\
	$(TARGET_DIR)/usr_rtb_pmu_validity_ind.o

## Objects explicitly added by the user
LINKONLYOBJECTS =

## Build
all: $(TARGET) $(TARGET_DIR)/$(PROJECT).hex $(TARGET_DIR)/$(PROJECT).eep $(TARGET_DIR)/$(PROJECT).lss size

## Compile
$(TARGET_DIR)/main.o: $(APP_DIR)/Src/main.c
	$(CC) $(INCLUDES) $(CFLAGS) -c  -o $@ $<
$(TARGET_DIR)/sio_handler.o: $(PATH_SIO_SUPPORT)/Src/sio_handler.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_uart.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src/pal_uart.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_sio_hub.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Src/pal_sio_hub.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_irq.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/$(_BOARD_TYPE)/pal_irq.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src/pal.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_timer.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src/pal_timer.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_board.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/$(_PAL_TYPE)/Boards/$(_BOARD_TYPE)/pal_board.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_utils.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src/pal_utils.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/pal_trx_access.o: $(PATH_PAL)/$(_PAL_GENERIC_TYPE)/Generic/Src/pal_trx_access.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/bmm.o: $(PATH_RES)/Buffer_Management/Src/bmm.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/qmm.o: $(PATH_RES)/Queue_Management/Src/qmm.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_rx.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_rx.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_tx.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_tx.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_ed.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_ed.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_slotted_csma.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_slotted_csma.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_pib.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_pib.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_init.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_init.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_irq_handler.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_irq_handler.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_pwr_mgmt.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_pwr_mgmt.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/tal_rx_enable.o: $(PATH_TAL)/$(_TAL_TYPE)/Src/tal_rx_enable.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/mac_data_extract_mhr.o: $(PATH_MAC)/Src/mac_data_extract_mhr.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/rtb.o: $(PATH_RTB)/Src/rtb.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/rtb_api.o: $(PATH_RTB)/Src/rtb_api.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/rtb_callback_wrapper.o: $(PATH_RTB)/Src/rtb_callback_wrapper.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/rtb_dispatcher.o: $(PATH_RTB)/Src/rtb_dispatcher.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/rtb_hw_233r_xmega.o: $(PATH_RTB)/Src/rtb_hw_233r_xmega.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/rtb_pib.o: $(PATH_RTB)/Src/rtb_pib.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/rtb_rx.o: $(PATH_RTB)/Src/rtb_rx.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/rtb_tx.o: $(PATH_RTB)/Src/rtb_tx.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/usr_rtb_pmu_validity_ind.o: $(PATH_RTB)/Src/usr_rtb_pmu_validity_ind.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/usr_rtb_range_conf.o: $(PATH_RTB)/Src/usr_rtb_range_conf.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/usr_rtb_reset_conf.o: $(PATH_RTB)/Src/usr_rtb_reset_conf.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<
$(TARGET_DIR)/usr_rtb_set_conf.o: $(PATH_RTB)/Src/usr_rtb_set_conf.c
	$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $<

##Link
$(TARGET): $(OBJECTS)
	 $(CC) $(LDFLAGS) $(OBJECTS) $(LINKONLYOBJECTS) $(LIBDIRS) $(LIBS) -o $(TARGET)

%.hex: $(TARGET)
	avr-objcopy -O ihex $(HEX_FLASH_FLAGS)  $< $@

%.eep: $(TARGET)
	avr-objcopy $(HEX_EEPROM_FLAGS) -O ihex $< $@ || exit 0

%.lss: $(TARGET)
	avr-objdump -h -S $< > $@

## avr-size options
ifdef windir
SIZEFLAGS = -C --mcu=${MCU}
else
ifdef WINDIR
SIZEFLAGS = -C --mcu=${MCU}
else
SIZEFLAGS = -B
endif
endif

size: ${TARGET}
	@echo
	@avr-size $(SIZEFLAGS) ${TARGET}

## Clean target
.PHONY: clean
clean:
	-rm -rf $(TARGET_DIR)/*.o $(TARGET_DIR)/$(PROJECT).elf dep/* $(TARGET_DIR)/$(PROJECT).hex $(TARGET_DIR)/$(PROJECT).eep $(TARGET_DIR)/$(PROJECT).lss $(TARGET_DIR)/$(PROJECT).map

##Options for null device
ifdef windir
NULLDEV = NUL:
else
ifdef WINDIR
NULLDEV = NUL:
else
NULLDEV = /dev/null
endif
endif
## Other dependencies
-include $(shell mkdir dep 2>$(NULLDEV)) $(wildcard dep/*)

