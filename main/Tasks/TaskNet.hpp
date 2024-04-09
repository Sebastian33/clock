#include "../config.hpp"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"

#ifndef MAIN_TASKS_TASKNET_HPP_
#define MAIN_TASKS_TASKNET_HPP_

class TaskNet
{
public:
	void Init(EventGroupHandle_t* eventGroup);

	static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
private:
	esp_netif_t* accessPointIF;
	esp_netif_t* stationIF;
};



#endif /* MAIN_TASKS_TASKNET_HPP_ */
