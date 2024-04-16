#include "../config.hpp"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_server.h"

#ifndef MAIN_TASKS_TASKNET_HPP_
#define MAIN_TASKS_TASKNET_HPP_

class TaskNet
{
public:
	void Init(EventGroupHandle_t* eventGroup);
	void SetTime(tm& dt);
	tm GetTime();
	void SetWifiCredentials(char* ssid, char* password);
	void GetWifiCredentials(char* ssid, char* password);
	void SetMainEvent(unsigned event);

	static void startWebServer(httpd_handle_t* server);
	static void stopWebServer(httpd_handle_t* server);

	static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

	static esp_err_t IndexGet(httpd_req_t* req);
	static esp_err_t DateTimePost(httpd_req_t* req);
	static esp_err_t WifiCredPost(httpd_req_t* req);

private:
	EventGroupHandle_t* mainEventGroup;

	esp_netif_t* accessPointIF;
	esp_netif_t* stationIF;

	httpd_handle_t server = nullptr;

	tm dateTime;
	char ssid[SSID_SIZE];
	char password[PASSWORD_SIZE];
};



#endif /* MAIN_TASKS_TASKNET_HPP_ */
