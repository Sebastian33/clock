#include "TaskNet.hpp"
#include <string.h>

#include "esp_log.h"

const char AP_SSID[] = "clock";
const char AP_PASSWORD[] = "12345678";

TaskNet* taskNet = nullptr;

const u8* getIndexPageStart();
const u8* getIndexPageEnd();


void TaskNet::Init(EventGroupHandle_t* eventGroup)
{
	taskNet = this;

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	accessPointIF = esp_netif_create_default_wifi_ap();
	stationIF = esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t wifiEvents;
	esp_event_handler_instance_t getIpEvents;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
										ESP_EVENT_ANY_ID,
										&wifiEventHandler,
										&server,
										&wifiEvents));



	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
	                                    IP_EVENT_STA_GOT_IP,
	                                    &wifiEventHandler,
	                                    nullptr,
	                                    &getIpEvents));
	wifi_config_t APConfig = {};
	memcpy(APConfig.ap.ssid, AP_SSID, strlen(AP_SSID));
	memcpy(APConfig.ap.password, AP_PASSWORD, strlen(AP_PASSWORD));
	APConfig.ap.ssid_len = strlen((char*)APConfig.ap.ssid);
	APConfig.ap.channel = 4;
	APConfig.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	APConfig.ap.max_connection = 4;

	wifi_config_t SConfig = {};
	strcpy((char*)SConfig.sta.ssid, "");
	strcpy((char*)SConfig.sta.password, "");
	SConfig.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
	SConfig.sta.pmf_cfg = {
		true,
		false
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &APConfig));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &SConfig));
	ESP_ERROR_CHECK(esp_wifi_start());
}

void TaskNet::wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_id == WIFI_EVENT_AP_START)
	{
	   	TaskNet::startWebServer((httpd_handle_t*)arg);
	   	ESP_LOGI("NET", "starterd");
	}
	else if (event_id == WIFI_EVENT_AP_STOP)
	{
		TaskNet::stopWebServer((httpd_handle_t*)arg);
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		//esp_wifi_connect();
	}
	else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
	{
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
	}
	else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_START)
	{
	}
}

static const httpd_uri_t indexGetD = {
	"/",
	HTTP_GET,
	TaskNet::indexGet,
	nullptr
};

esp_err_t TaskNet::indexGet(httpd_req_t* req)
{
	ESP_LOGI("NET", "index get");
	httpd_resp_send(req, (char *)getIndexPageStart(), getIndexPageEnd()-getIndexPageStart());
	return ESP_OK;
}

void TaskNet::startWebServer(httpd_handle_t* server)
{
	if (*server != nullptr)
	{
		return;
	}

	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.lru_purge_enable = true;
	if(httpd_start(server, &config) == ESP_OK)
	{
	   	httpd_register_uri_handler(*server, &indexGetD);
	}
}

void TaskNet::stopWebServer(httpd_handle_t* server)
{
	if(*server != nullptr)
	{
		httpd_stop(*server);
		*server = nullptr;
	}
}
