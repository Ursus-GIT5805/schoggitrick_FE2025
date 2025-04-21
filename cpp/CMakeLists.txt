cmake_minimum_required(VERSION 3.12)

set(NAME_SRC src/main.cpp)
set(NAME "SchoggiTrick")
set(EXECUTABLE schoggitrick)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # used for clangd
list(APPEND CMAKE_MODULE_PATH "/usr/local/lib/cmake")

project(${NAME})

add_library(adafruit_hat "")
target_sources(adafruit_hat
  PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/pwm.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/i2cdevice.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/util.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/adafruitmotorhat.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/adafruitdcmotor.h"

  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/pwm.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/i2cdevice.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/adafruitmotorhat.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/adafruitdcmotor.cpp"
)

add_executable( ${EXECUTABLE} ${NAME_SRC} )

# External packages
find_package(OpenCV REQUIRED)
find_package(Curses REQUIRED)
find_package(raspicam REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/adafruit-motor-hat-cpp-library/source/
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CURSES_INCLUDE_DIRS}
  /usr/include
)

target_link_libraries( ${EXECUTABLE}
  adafruit_hat
  /usr/lib/libwiringPi.so
  ${OpenCV_LIBS}
  ${CURSES_LIBRARIES}
  ${raspicam_LIBS}
  ${raspicam_CV_LIBS}
)

link_directories( ${CMAKE_BINARY_DIR}/bin )
