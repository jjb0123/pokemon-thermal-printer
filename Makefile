PROJECT_NAME = pokemon-thermal-printer

NRF_IC = nrf52833
SDK_VERSION = 16
SOFTDEVICE_MODEL = blank

APP_HEADER_PATHS += source
APP_HEADER_PATHS += source/printer

ifdef TEST_BUTTON
  APP_SOURCE_PATHS += .
  APP_SOURCES      += test_button_print.c
else ifdef TEST_BAUD
  APP_SOURCE_PATHS += .
  APP_SOURCES      += test_baud_sweep.c
else ifdef TEST_PRINTER
  # Minimal build: just test_breadboard.c + dfr0503 driver, no sensors/pokemon/speaker
  APP_SOURCE_PATHS += .
  APP_SOURCE_PATHS += source/printer
  APP_SOURCES      += test_breadboard.c
  APP_SOURCES      += $(notdir $(wildcard source/printer/*.c))
else
  APP_HEADER_PATHS += source/sensors
  APP_HEADER_PATHS += source/pokemon
  APP_HEADER_PATHS += source/speaker

  APP_SOURCE_PATHS += source
  APP_SOURCE_PATHS += source/printer
  APP_SOURCE_PATHS += source/sensors
  APP_SOURCE_PATHS += source/pokemon
  APP_SOURCE_PATHS += source/speaker

  APP_SOURCES += $(notdir $(wildcard source/*.c))
  APP_SOURCES += $(notdir $(wildcard source/printer/*.c))
  APP_SOURCES += $(notdir $(wildcard source/sensors/*.c))
  APP_SOURCES += $(notdir $(wildcard source/pokemon/*.c))
  APP_SOURCES += $(notdir $(wildcard source/speaker/*.c))
endif

NRF_BASE_DIR = ../GitHub/nu-microbit-base/s26-lab2-jandy/software/nrf52x-base/

include ../GitHub/nu-microbit-base/s26-lab2-jandy/software/boards/microbit_v2/Board.mk
include $(NRF_BASE_DIR)/make/AppMakefile.mk
