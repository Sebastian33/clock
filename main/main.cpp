#include "config.hpp"

#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include <time.h>
#include "nvs_flash.h"

#include "Drivers/RTCDriver.hpp"
#include "Tasks/TaskNet.hpp"

const gpio_num_t GPIO_SRCLK = GPIO_NUM_12;
const gpio_num_t GPIO_SER = GPIO_NUM_15;
const gpio_num_t GPIO_PWM = GPIO_NUM_16;
const gpio_num_t GPIO_RCLK = GPIO_NUM_17;


u8 digitEncoded[] = {0xe7, 0x42, 0xd5, 0xd6, 0x72, 0xb6, 0xb7, 0xc2, 0xf7, 0xf6};

void InitPwm()
{
	ledc_timer_config_t timer;
	timer.speed_mode = LEDC_LOW_SPEED_MODE;
	timer.timer_num = LEDC_TIMER_0;
	timer.duty_resolution = LEDC_TIMER_13_BIT;
	timer.freq_hz = 5000;
	timer.clk_cfg = LEDC_AUTO_CLK;
	ledc_timer_config(&timer);

	ledc_channel_config_t ledc_channel;
	ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
	ledc_channel.channel = LEDC_CHANNEL_0;
	ledc_channel.timer_sel = LEDC_TIMER_0;
	ledc_channel.intr_type =  LEDC_INTR_FADE_END;
	ledc_channel.gpio_num = GPIO_PWM;
	ledc_channel.duty = 4096;
	ledc_channel.hpoint = 0;
	ledc_channel_config(&ledc_channel);
}

void InitOutput()
{
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1<<GPIO_SRCLK)|(1<<GPIO_SER)|(1<<GPIO_RCLK);
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);
	gpio_set_level(GPIO_SRCLK, 0);
	gpio_set_level(GPIO_SER, 0);
	gpio_set_level(GPIO_RCLK, 0);
}

void sendBit(unsigned bit)
{
	gpio_set_level(GPIO_SER, bit);
	gpio_set_level(GPIO_SRCLK, 1);
	gpio_set_level(GPIO_SRCLK, 0);
	gpio_set_level(GPIO_SER, 0);
}

void sendTime(u8* digits, u8 dots)
{
	u8 digit;
	for(int i=0;i<4;i++)
	{
		digit = digitEncoded[digits[i]];
		if(i==1 || i==2)
		{
			digit|=dots<<3;
		}
		for(int j=7;j>=0;j--)
		{
			sendBit((digit>>j)&1);
		}
	}

	gpio_set_level(GPIO_RCLK, 1);
	gpio_set_level(GPIO_RCLK, 0);
}

extern "C" void app_main(void)
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

	InitPwm();
	InitOutput();

	RTCDriver rtc;
	rtc.Init();

	EventGroupHandle_t eventGroup = xEventGroupCreate();

	TaskNet taskNet;
	taskNet.Init(&eventGroup);

	u8 dots=0;
	u8 buf[4];
	while(true)
	{
		//if(xEventGroupWaitBits(eventGroup, 0, true, false, 500/ portTICK_PERIOD_MS))

		tm dt = rtc.ReadDateTime();
		buf[0]=dt.tm_min%10;
		buf[1]=dt.tm_min/10;
		buf[2]=dt.tm_hour%10;
		buf[3]=dt.tm_hour/10;

		sendTime(buf, dots);

		dots=dots^1;
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
