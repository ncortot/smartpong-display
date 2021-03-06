# This file is a makefile included from the top level makefile which
# defines the sources built for the target.

# Define the prefix to this directory. 
# Note: The name must be unique within this build and should be
#       based on the root of the project
TARGET_SRC_PATH = src
CORE_SRC_PATH = $(CORE_FIRMWARE_PATH)/src

# Add include to all objects built for this target
INCLUDE_DIRS += inc
INCLUDE_DIRS += resources
INCLUDE_DIRS += $(CORE_FIRMWARE_PATH)/inc

# C source files included in this build.
CSRC +=

# C++ source files included in this build.
CPPSRC += $(TARGET_SRC_PATH)/application.cpp
CPPSRC += $(TARGET_SRC_PATH)/audio_player.cpp
CPPSRC += $(TARGET_SRC_PATH)/flash_player.cpp
CPPSRC += $(TARGET_SRC_PATH)/ht1632c.cpp
CPPSRC += $(TARGET_SRC_PATH)/main.cpp

# C++ source files from the core-firmware
CPPSRC += $(CORE_SRC_PATH)/newlib_stubs.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_utilities.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_eeprom.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_i2c.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_interrupts.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_ipaddress.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_network.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_print.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_servo.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_spi.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_stream.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_string.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_tcpclient.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_tcpserver.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_time.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_tone.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_udp.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_usartserial.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_usbserial.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wiring_wifi.cpp
CPPSRC += $(CORE_SRC_PATH)/spark_wlan.cpp
CPPSRC += $(CORE_SRC_PATH)/stm32_it.cpp
CPPSRC += $(CORE_SRC_PATH)/usb_desc.cpp
CPPSRC += $(CORE_SRC_PATH)/usb_endp.cpp
CPPSRC += $(CORE_SRC_PATH)/usb_istr.cpp
CPPSRC += $(CORE_SRC_PATH)/usb_prop.cpp
CPPSRC += $(CORE_SRC_PATH)/wifi_credentials_reader.cpp

# ASM source files included in this build.
ASRC +=

