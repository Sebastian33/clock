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
	void SetTime(struct tm& dt);

	static void startWebServer(httpd_handle_t* server);
	static void stopWebServer(httpd_handle_t* server);

	static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

	static esp_err_t IndexGet(httpd_req_t* req);
	static esp_err_t DateTimePost(httpd_req_t* req);

private:
	esp_netif_t* accessPointIF;
	esp_netif_t* stationIF;

	httpd_handle_t server = nullptr;

	struct tm dateTime;
};



#endif /* MAIN_TASKS_TASKNET_HPP_ */
