#include "../config.hpp"
#include "nvs_flash.h"

#ifndef MAIN_DRIVERS_EEPROM_HPP_
#define MAIN_DRIVERS_EEPROM_HPP_
class Eeprom
{
public:
	void Init();
	void ReadWifiCredentials(char* ssid, char* password);
	void WriteWifiCredentials(const char* ssid, const char* password);
	void ReadTimezone(int& tz);
	void WriteTimezone(int tz);

private:
	esp_err_t ReadString(char* value, const char* key);
	esp_err_t WriteString(const char* value, const char* key);
	esp_err_t ReadI8(int8_t& value, const char* key);
	esp_err_t WriteI8(int8_t value, const char* key);

	void CheckWifiCredentials();
	void CheckTimezone();
};

#endif /* MAIN_DRIVERS_EEPROM_HPP_ */
