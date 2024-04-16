#ifndef MAIN_CONFIG_HPP_
#define MAIN_CONFIG_HPP_
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

using u8 = unsigned char;

const int SSID_SIZE = 32;
const int PASSWORD_SIZE = 32;

const unsigned MAIN_SET_TIME (1<<0);
const unsigned MAIN_WRITE_WIFI_CRED (1<<1);


#endif /* MAIN_CONFIG_HPP_ */
