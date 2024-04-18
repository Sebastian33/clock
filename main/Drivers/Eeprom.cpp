#include "Eeprom.hpp"

const char NAMESPACE[] = "storage";

const char EEPROM_SSID_KEY[] = "ssidKey";
const char EEPROM_PASSWORD_KEY[] = "passwordKey";

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

void Eeprom::CheckWifiCredentials()
{
	char value[32];
	esp_err_t res = ReadString(value, EEPROM_SSID_KEY);
	if(res != ESP_OK)
	{
		WriteString("", EEPROM_SSID_KEY);
	}

	res = ReadString(value, EEPROM_PASSWORD_KEY);
	if(res != ESP_OK)
	{
		WriteString("", EEPROM_PASSWORD_KEY);
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

	size_t size;
	res = nvs_get_str(nvsHandle, key, (char*)value, &size);
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
