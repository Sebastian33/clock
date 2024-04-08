#include <RTCDriver.hpp>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include <time.h>

const gpio_num_t GPIO_SRCLK = GPIO_NUM_12;
const gpio_num_t GPIO_SER = GPIO_NUM_15;
const gpio_num_t GPIO_PWM = GPIO_NUM_16;
const gpio_num_t GPIO_RCLK = GPIO_NUM_17;

extern void app_main();

typedef unsigned char u8;

u8 digitEncoded[] = {0xe7, 0x42, 0xd5, 0xd6, 0x72, 0xb6, 0xb7, 0xc2, 0xf7, 0xf6};



void sendBit(unsigned bit)
{
	gpio_set_level(GPIO_SER, bit);
	//vTaskDelay(10 / portTICK_PERIOD_MS);
	gpio_set_level(GPIO_SRCLK, 1);
	//vTaskDelay(10 / portTICK_PERIOD_MS);
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
	//vTaskDelay(10 / portTICK_PERIOD_MS);
	gpio_set_level(GPIO_RCLK, 0);
}

void app_main(void)
{
	//PWM
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

	//GPIO
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

	//i2c
	RTCDriver rtc;
	rtc.Init();

	u8 buf[7];
//	buf[0]=dec2bcd(0);
//	buf[1]=dec2bcd(33);
//	buf[2]=dec2bcd(11);
//	buf[3]=dec2bcd(31);
//	buf[4]=dec2bcd(6); //weekday
//	buf[5]=dec2bcd(3) | (1<<7); //month
//	buf[6]=dec2bcd(24);
//
//    i2c_cmd_handle_t handle = i2c_cmd_link_create();
//    i2c_master_start(handle);
//    i2c_master_write_byte(handle, 0xa2, true);
//    i2c_master_write_byte(handle, 0x02, true);
//    i2c_master_write(handle, buf, 7, true);
//    i2c_master_stop(handle);
//    i2c_master_cmd_begin(I2C_NUM_0, handle, 1000/portTICK_PERIOD_MS);
//    i2c_cmd_link_delete(handle);
//
//    vTaskDelay(500 / portTICK_PERIOD_MS);

	u8 dots=0;
	while(true)
	{
//		i2c_cmd_handle_t handle = i2c_cmd_link_create();
//		i2c_master_start(handle);
//		i2c_master_write_byte(handle, 0xa2, true);
//		i2c_master_write_byte(handle, 0x02, true);
//		i2c_master_start(handle);
//		i2c_master_write_byte(handle, 0xa3, true);
//		i2c_master_read(handle, buf, 7, I2C_MASTER_LAST_NACK);
//		i2c_master_stop(handle);
//		i2c_master_cmd_begin(I2C_NUM_0, handle, 1000/portTICK_PERIOD_MS);
//		i2c_cmd_link_delete(handle);
//

		tm dt = rtc.ReadDateTime();
		buf[0]=dt.tm_min%10;
		buf[1]=dt.tm_min/10;
		buf[2]=dt.tm_hour%10;
		buf[3]=dt.tm_hour/10;

		sendTime(buf, dots);

		dots=dots^1;
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}

	/*//I2C and RTC
	u8 buf[7];
    printf("Hello world!\n");
    i2c_config_t conf={};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 13;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = 14;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

    buf[0]=dec2bcd(0);
    buf[1]=dec2bcd(10);
    buf[2]=dec2bcd(20);
    buf[3]=dec2bcd(2);
    buf[4]=dec2bcd(5); //weekday
    buf[5]=dec2bcd(3) | (1<<7); //month
    buf[6]=dec2bcd(24);


    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, 0xa2, true);
    i2c_master_write_byte(handle, 0x02, true);
    i2c_master_write(handle, buf, 7, true);
    i2c_master_stop(handle);
    i2c_master_cmd_begin(I2C_NUM_0, handle, 1000/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(handle);

    for (int i = 10; i >= 0; i--)
    {
    	i2c_cmd_handle_t handle = i2c_cmd_link_create();
   	    i2c_master_start(handle);
   	    i2c_master_write_byte(handle, 0xa2, true);
   	    i2c_master_write_byte(handle, 0x02, true);
   	    i2c_master_start(handle);
   	    i2c_master_write_byte(handle, 0xa3, true);
   	    i2c_master_read(handle, buf, 7, I2C_MASTER_LAST_NACK);
   	    i2c_master_stop(handle);
   	    i2c_master_cmd_begin(I2C_NUM_0, handle, 1000/portTICK_PERIOD_MS);
   	    i2c_cmd_link_delete(handle);

    	printf("%d:%d:%d %d-%d-%d\n", bcd2dec(buf[0]&0x7f), bcd2dec(buf[1]&0x7f), bcd2dec(buf[2]&0x3f), bcd2dec(buf[3]&0x3f), bcd2dec(buf[5]&0x1f), bcd2dec(buf[6]));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }*/

}
