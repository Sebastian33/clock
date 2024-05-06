#include "config.hpp"

#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include <time.h>

#include "Drivers/RTCDriver.hpp"
#include "Drivers/Eeprom.hpp"
#include "Tasks/TaskNet.hpp"

#include "esp_log.h"

const gpio_num_t GPIO_SRCLK = GPIO_NUM_12;
const gpio_num_t GPIO_SER = GPIO_NUM_15;
const gpio_num_t GPIO_PWM = GPIO_NUM_16;
const gpio_num_t GPIO_RCLK = GPIO_NUM_17;

u8 digitEncoded[] = {0xe7, 0x42, 0xd5, 0xd6, 0x72, 0xb6, 0xb7, 0xc2, 0xf7, 0xf6};

const int NTP_RETRY_COUNTER = 60;

extern const u8 index_page_start[] asm("_binary_index_html_start");
extern const u8 index_page_end[]   asm("_binary_index_html_end");
const u8* getIndexPageStart()
{
	return index_page_start;
}
const u8* getIndexPageEnd()
{
	return index_page_end;
}

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
	Eeprom eeprom;
	eeprom.Init();

	TaskNet taskNet;
	eeprom.ReadWifiCredentials(taskNet.GetPointerSsid(),taskNet.GetPointerPassword());

	InitPwm();
	InitOutput();
	RTCDriver rtc;
	rtc.Init();

	EventGroupHandle_t eventGroup = xEventGroupCreate();

	taskNet.Init(&eventGroup);

	u8 dots=0;
	u8 buf[4];
	unsigned bits;
	tm dt;
	int timezone = 0;
	eeprom.ReadTimezone(timezone);
	int sync = 0;
	int retries = 0;

	while(true)
	{
		bits = xEventGroupWaitBits(eventGroup,
				MAIN_UPDATE_TIME|
				MAIN_CP_TIME|
				MAIN_WRITE_WIFI_CRED|
				MAIN_NTP_SYNC|
				MAIN_SET_TZ,
				true, false, 500 / portTICK_PERIOD_MS);
		if(bits == 0)
		{
			dt = rtc.ReadDateTime();

			if(dt.tm_hour == 0 && sync != 0)
			{
				sync = 0;
				retries = 0;
			}
			else if(sync == 0 &&
					((timezone>=0 && dt.tm_hour+timezone==1) ||
					(timezone<0 && dt.tm_hour-timezone==23)))
			{
				xEventGroupSetBits(eventGroup, MAIN_NTP_SYNC);
			}
			else if(sync<0)
			{
				sync++;
			}

			buf[0]=dt.tm_min%10;
			buf[1]=dt.tm_min/10;
			buf[2]=dt.tm_hour%10;
			buf[3]=dt.tm_hour/10;

			sendTime(buf, dots);

			dots=dots^1;
			//vTaskDelay(500 / portTICK_PERIOD_MS);
			continue;
		}

		if((bits & MAIN_UPDATE_TIME) != 0)
		{
			dt = taskNet.GetTime();
			rtc.WriteDateTime(dt);
		}
		if((bits & MAIN_CP_TIME) != 0)
		{
			taskNet.SetTime(dt);
		}
		if((bits & MAIN_WRITE_WIFI_CRED) != 0)
		{
			eeprom.WriteWifiCredentials(taskNet.GetPointerSsid(), taskNet.GetPointerPassword());
		}
		if((bits & MAIN_NTP_SYNC) != 0)
		{
			esp_err_t res = taskNet.NtpSync(dt);
			if(res != ESP_OK)
			{
				if(sync==0  && retries<5)
				{
					sync = -NTP_RETRY_COUNTER;
					retries++;
				}
				else if(sync==0 && retries>=5)
					sync = 1;
				continue;
			}

			taskNet.addTimezone(dt, timezone);
			taskNet.SetTime(dt);
			//ESP_LOGI("MAIN", "%d-%d-%d %d %d %d", dt.tm_year, dt.tm_mon, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
			xEventGroupSetBits(eventGroup, MAIN_UPDATE_TIME);
			sync = 1;
		}
		if((bits & MAIN_SET_TZ) > 0)
		{
			timezone = taskNet.GetTimezone();
			eeprom.WriteTimezone(timezone);
		}
	}
}
