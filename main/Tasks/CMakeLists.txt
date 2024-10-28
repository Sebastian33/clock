set(SOURCE_FILES "TaskNet.cpp" "TaskPwm.cpp")
add_library(Tasks STATIC ${SOURCE_FILES})

include_directories(${IDF_PATH}/components/esp_http_server/include/)
include_directories(${IDF_PATH}/components/nghttp/port/include/)