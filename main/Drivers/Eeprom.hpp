#include "..\config.hpp"
#include "nvs_flash.h"

#ifndef MAIN_DRIVERS_EEPROM_HPP_
#define MAIN_DRIVERS_EEPROM_HPP_
class Eeprom
{
public:
	void Init();
	void ReadWifiCredentials(char* ssid, char* password);
	void WriteWifiCredentials(const char* ssid, const char* password);

private:
	esp_err_t ReadString(char* value, const char* key);
	esp_err_t WriteString(const char* value, const char* key);

	void CheckWifiCredentials();
};

#endif /* MAIN_DRIVERS_EEPROM_HPP_ */
