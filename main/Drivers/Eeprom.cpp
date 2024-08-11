#include "Eeprom.hpp"
#include <cstring>
#include "esp_log.h"
const char NAMESPACE[] = "storage";

const char EEPROM_SSID_KEY[] = "ssidKey";
const char EEPROM_PASSWORD_KEY[] = "passwordKey";
const char EEPROM_TIMEZONE_KEY[] = "timezone";

void Eeprom::Init()
{
	esp_err_t espResult = nvs_flash_init();
	if(espResult == ESP_ERR_NVS_NO_FREE_PAGES || espResult == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		espResult = nvs_flash_erase();
		if(espResult != ESP_OK)
		{
			return;
		}

		espResult = nvs_flash_init();
		if(espResult != ESP_OK)
		{
			return;
		}
	}

	CheckWifiCredentials();
	CheckTimezone();
}

void Eeprom::ReadWifiCredentials(char* ssid, char* password)
{
	ReadString(ssid, EEPROM_SSID_KEY);
	ReadString(password, EEPROM_PASSWORD_KEY);
}

void Eeprom::WriteWifiCredentials(const char* ssid, const char* password)
{
	WriteString(ssid, EEPROM_SSID_KEY);
	WriteString(password, EEPROM_PASSWORD_KEY);
}

void Eeprom::ReadTimezone(int& tz)
{
	int8_t val;
	ReadI8(val, EEPROM_TIMEZONE_KEY);
	tz = val;
}

void Eeprom::WriteTimezone(int tz)
{
	ESP_LOGI("EEPROM", "write tz %d", tz);
	WriteI8(tz, EEPROM_TIMEZONE_KEY);
}

void Eeprom::CheckWifiCredentials()
{
	char value[32];
	esp_err_t res = ReadString(value, EEPROM_SSID_KEY);
	if(res != ESP_OK)
	{
		WriteString("", EEPROM_SSID_KEY);
	}

	//memset(value, 0, 32);
	res = ReadString(value, EEPROM_PASSWORD_KEY);
	if(res != ESP_OK)
	{
		WriteString("", EEPROM_PASSWORD_KEY);
	}
}

void Eeprom::CheckTimezone()
{
	int8_t tz=0;
	esp_err_t res = ReadI8(tz, EEPROM_TIMEZONE_KEY);
	if(res != ESP_OK)
	{
		WriteI8(0, EEPROM_TIMEZONE_KEY);
	}
}

esp_err_t Eeprom::ReadString(char* value, const char* key)
{
	nvs_handle_t nvsHandle;
	esp_err_t res = nvs_open(NAMESPACE, NVS_READWRITE, &nvsHandle);
	if(res != ESP_OK)
	{
		return res;
	}

	size_t size = DEFAULT_REC_SIZE;
	res = nvs_get_str(nvsHandle, key, value, &size);
	nvs_close(nvsHandle);
	return res;
}

esp_err_t Eeprom::WriteString(const char* value, const char* key)
{
	nvs_handle_t nvsHandle;
	esp_err_t res = nvs_open(NAMESPACE, NVS_READWRITE, &nvsHandle);
	if(res != ESP_OK)
	{
		return res;
	}

	res = nvs_set_str(nvsHandle, key, value);
	if(res != ESP_OK)
	{
		nvs_close(nvsHandle);
		return res;
	}

	res = nvs_commit(nvsHandle);
	nvs_close(nvsHandle);
	return res;
}

esp_err_t Eeprom::ReadI8(int8_t& value, const char* key)
{
	nvs_handle_t nvsHandle;
	esp_err_t res = nvs_open(NAMESPACE, NVS_READWRITE, &nvsHandle);
	if(res != ESP_OK)
	{
		return res;
	}

	res = nvs_get_i8(nvsHandle, key, &value);
	ESP_LOGI("EEPROM", "read %d", value);
	nvs_close(nvsHandle);
	return res;
}

esp_err_t Eeprom::WriteI8(int8_t value, const char* key)
{
	ESP_LOGI("EEPROM", "write i8 %d", value);
	nvs_handle_t nvsHandle;
	esp_err_t res = nvs_open(NAMESPACE, NVS_READWRITE, &nvsHandle);
	if(res != ESP_OK)
	{
		return res;
	}

	res = nvs_set_i8(nvsHandle, key, value);
	if(res != ESP_OK)
	{
		nvs_close(nvsHandle);
		return res;
	}
	res = nvs_commit(nvsHandle);
	nvs_close(nvsHandle);
	return res;
}
