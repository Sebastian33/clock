#include "../config.hpp"

#include "esp_system.h"
#include "esp_event.h"
#include "driver/ledc.h"

#ifndef MAIN_TASKS_TASKPWM_HPP_
#define MAIN_TASKS_TASKPWM_HPP_

class TaskPwm
{
public:
	TaskPwm(gpio_num_t gpioPwm0);

	void Init(EventGroupHandle_t* eventGroup);
	void Start();

	static void Run(void* args);

private:
	gpio_num_t gpioPwm;

	EventGroupHandle_t* mainEventGroup;
	TaskHandle_t handle;
};



#endif /* MAIN_TASKS_TASKPWM_HPP_ */
