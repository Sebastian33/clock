set(SOURCE_FILES "RTCDriver.cpp" "Eeprom.cpp")
add_library(Drivers STATIC ${SOURCE_FILES})

include_directories(${IDF_PATH}/components/nvs_flash/include/)
include_directories(${IDF_PATH}/components/spi_flash/include/)