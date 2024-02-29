#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/i2c.h"

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

	u8 buf[7];
    printf("Hello world!\n");
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 13;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = 14;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

    buf[0]=dec2bcd(1);
    buf[1]=dec2bcd(20);
    buf[2]=dec2bcd(14);
    buf[3]=dec2bcd(29);
    buf[4]=dec2bcd(3);
    buf[5]=dec2bcd(2) | (1<<7);
    buf[6]=dec2bcd(24);

    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, 0xa2, true);
    i2c_master_write_byte(handle, 0x02, true);
    i2c_master_start(handle);
    i2c_master_write_byte(handle, 0xa2, true);
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

    	printf("%d:%d:%d %d-%d-%d", bcd2dec(buf[0]&0x7f), bcd2dec(buf[1]&0x7f), bcd2dec(buf[2]&0x3f), bcd2dec(buf[3]&0x3f), bcd2dec(buf[5]&0x1f), bcd2dec(buf[6]));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
