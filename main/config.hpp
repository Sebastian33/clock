#ifndef MAIN_CONFIG_HPP_
#define MAIN_CONFIG_HPP_
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

using u8 = unsigned char;
using u32 = unsigned;
using u64 = unsigned long long;

const int SSID_SIZE = 32;
const int PASSWORD_SIZE = 32;

const unsigned MAIN_UPDATE_TIME (1<<0);
const unsigned MAIN_CP_TIME (1<<1);
const unsigned MAIN_WRITE_WIFI_CRED (1<<2);
const unsigned MAIN_NTP_SYNC (1<<3);
const unsigned MAIN_SET_TZ (1<<4);

#endif /* MAIN_CONFIG_HPP_ */
