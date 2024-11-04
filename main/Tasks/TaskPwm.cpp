#include "TaskPwm.hpp"
#include "driver/adc.h"

#include "esp_log.h"

const ledc_mode_t PWM_SPEED_MODE = LEDC_LOW_SPEED_MODE;
const ledc_channel_t PWM_CHANNEL = LEDC_CHANNEL_0;

const adc1_channel_t ADC_CHANNEL = ADC1_CHANNEL_4;

const u32 TASK_PWM_PRIORITY = configMAX_PRIORITIES-5;

const int MEAS_NUM = 10;
const int ADC_BRIGHT_LEVEL = 1000;
const int ADC_DIM_LEVEL = 3000;

const int PWM_DUTY_BRIGHT = 7168;
const int PWM_DUTY_HALF_BRIGHT = 2048;
const int PWM_DUTY_DIM = 1024;

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
	int adc = 0;
	int duty = 0;
	int newDuty = 0;
	while(true)
	{
		for(int i=0;i<MEAS_NUM;i++)
		{
			adc += adc1_get_raw(ADC_CHANNEL);
			//ESP_LOGI("MAIN", "ADC: %d", adc);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}

		adc /= MEAS_NUM;
		if(adc>(ADC_DIM_LEVEL+50))
			newDuty = PWM_DUTY_DIM;
		else if( adc>(ADC_BRIGHT_LEVEL+50) && adc<=(ADC_DIM_LEVEL-50))
			newDuty = PWM_DUTY_HALF_BRIGHT;
		else if (adc<=(ADC_BRIGHT_LEVEL-50))
			newDuty = PWM_DUTY_BRIGHT;

		if(duty != newDuty)
		{
			duty = newDuty;
			ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
			ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
		}

		adc = 0;
		vTaskDelay(20000 / portTICK_PERIOD_MS);
	}
}
