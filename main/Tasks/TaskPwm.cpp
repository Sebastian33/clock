#include "TaskPwm.hpp"
#include "driver/adc.h"

const ledc_mode_t PWM_SPEED_MODE = LEDC_LOW_SPEED_MODE;
const ledc_channel_t PWM_CHANNEL = LEDC_CHANNEL_0;

const adc1_channel_t ADC_CHANNEL = ADC1_CHANNEL_4;

const u32 TASK_PWM_PRIORITY = configMAX_PRIORITIES-5;

TaskPwm::TaskPwm(gpio_num_t gpioPwm0): gpioPwm(gpioPwm0), mainEventGroup(nullptr)
{}

void TaskPwm::Init(EventGroupHandle_t* eventGroup)
{
	mainEventGroup = eventGroup;

	ledc_timer_config_t timer;
	timer.speed_mode = PWM_SPEED_MODE;
	timer.timer_num = LEDC_TIMER_0;
	timer.duty_resolution = LEDC_TIMER_13_BIT;
	timer.freq_hz = 5000;
	timer.clk_cfg = LEDC_AUTO_CLK;
	ledc_timer_config(&timer);

	ledc_channel_config_t ledc_channel;
	ledc_channel.speed_mode = PWM_SPEED_MODE;
	ledc_channel.channel = LEDC_CHANNEL_0;
	ledc_channel.timer_sel = LEDC_TIMER_0;
	ledc_channel.intr_type =  LEDC_INTR_FADE_END;
	ledc_channel.gpio_num = gpioPwm;
	ledc_channel.duty = 4096;
	ledc_channel.hpoint = 0;
	ledc_channel_config(&ledc_channel);

	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_6);
}

void TaskPwm::Start()
{
	xTaskCreate(Run, "TASK PWM", 2*1024, nullptr, TASK_PWM_PRIORITY, &handle);
}

void TaskPwm::Run(void* args)
{
	int counter=0;
	while(true)
	{
		int adc=adc1_get_raw(ADC_CHANNEL);
		//ESP_LOGI("MAIN", "ADC: %d", adc);
		counter++;
		if(counter>10)
		{
			counter=0;
			if(adc>3000)
				ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 7200);
			else
				ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4096);
			ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
		}

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
