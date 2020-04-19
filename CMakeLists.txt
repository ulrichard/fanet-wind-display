SET(atmega328.build.f_cpu 16000000) # running on external 8MHz crystal

set(CMAKE_TOOLCHAIN_FILE ../GpsClock/arduino-cmake/cmake/ArduinoToolchain.cmake)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(FanetWindDisplay)

#print_board_list()

SET(AVR_DEVICE     atmega328)
SET(AVRDUDE_DEVICE atmega328p)

SET(IS_SOLDERED True)
IF(${IS_SOLDERED})
    SET(AVRDUDE_PORT   /dev/ttyACM0)   # diamex AVR von www.reichelt.de
    SET(AVRDUDE_PROGRAMMER   stk500v2) # diamex AVR von www.reichelt.de
ELSE()
    SET(AVRDUDE_PORT   /dev/ttyUSB0)    # Arduino
    SET(AVRDUDE_PROGRAMMER    arduino)  # Arduino
ENDIF()

ADD_DEFINITIONS(-std=c++14)

generate_arduino_library(fanet-base
    BOARD ${AVR_DEVICE}
	SRCS  
		fanet-base/fanet/fframe.cpp
		fanet-base/fanet/fneighbor.cpp
		fanet-base/fanet/frame/fname.cpp
		fanet-base/fanet/frame/fservice.cpp
		fanet-base/phy/coordinate.cpp
	HDRS  
		fanet-base/fanet/fframe.h
		fanet-base/fanet/fmac.h
		fanet-base/fanet/fmacaddr.h
		fanet-base/fanet/fneighbor.h
		fanet-base/fanet/frame/fname.h
		fanet-base/fanet/frame/fservice.h
		fanet-base/phy/coordinate.h
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/fanet-base/fanet)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/fanet-base)


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(/usr/share/arduino/libraries/SPI)

generate_arduino_firmware(fanet_wind_display
    BOARD ${AVR_DEVICE}
    SKETCH ../fanet-wind-display
	LIBS
		fanet-base
	PROGRAMMER stk500v2
    PORT  ${AVRDUDE_PORT}
)


IF(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")
	ADD_CUSTOM_TARGET(flash
				avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c ${AVRDUDE_PROGRAMMER} -b 57600 -F -u -Uflash:w:fanet_wind_display.hex
		DEPENDS fanet_wind_display
	)
ELSE(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")
	ADD_CUSTOM_TARGET(flash
				avrdude -P ${AVRDUDE_PORT} -p ${AVRDUDE_DEVICE} -c ${AVRDUDE_PROGRAMMER} -Uflash:w:fanet_wind_display.hex
		DEPENDS fanet_wind_display
	)
ENDIF(${AVRDUDE_PROGRAMMER} STREQUAL "arduino")



