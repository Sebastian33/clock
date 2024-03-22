#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/gpio.h"


typedef unsigned char u8;

u8 bcd2dec(u8 a)
{
	return (a&0xf)+10*(a>>4);
}

u8 dec2bcd(u8 a)
{
	return ((a/10)<<4)|(a%10);
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
	ledc_channel.gpio_num = 16;
	ledc_channel.duty = 8192;
	ledc_channel.hpoint = 0;
	ledc_channel_config(&ledc_channel);

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4096);
	ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

	//GPIO
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	//12-srclk, 15-ser, 17-rclk
	io_conf.pin_bit_mask = (1<<12)|(1<<15)|(1<<17);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
	gpio_set_level(12, 0);
	gpio_set_level(15, 0);
	gpio_set_level(17, 0);

	while(true)
	{
		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);


		gpio_set_level(17, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(17, 0);

		vTaskDelay(500 / portTICK_PERIOD_MS);


		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);

		gpio_set_level(15, 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(12, 0);
		gpio_set_level(15, 0);


		gpio_set_level(17, 1);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		gpio_set_level(17, 0);

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
