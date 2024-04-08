#include <RTCDriver.hpp>
#include "driver/i2c.h"

const int GPIO_SDA = 13;
const int GPIO_SCL = 14;

const unsigned I2C_ADDRESS = 0xa2;
const unsigned I2C_OFFSET = 0x02;
const unsigned I2C_DATA_SIZE = 7;

u8 bcd2dec(u8 a)
{
	return (a&0xf)+10*(a>>4);
}

u8 dec2bcd(u8 a)
{
	return ((a/10)<<4)|(a%10);
}

void RTCDriver::Init()
{
	i2c_config_t conf={};
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = GPIO_SDA;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = GPIO_SCL;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 400000;
	i2c_param_config(I2C_NUM_0, &conf);
	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}

void RTCDriver::WriteDateTime(const tm& datetime)
{
	u8 buf[I2C_DATA_SIZE];
	buf[0] = dec2bcd(datetime.tm_sec);
	buf[1] = dec2bcd(datetime.tm_min);
	buf[2] = dec2bcd(datetime.tm_hour);
	buf[3] = dec2bcd(datetime.tm_yday);
	buf[4] = dec2bcd(datetime.tm_wday);
	buf[5] = dec2bcd(datetime.tm_mon) | (1<<7);// century bit
	buf[6] = dec2bcd(datetime.tm_year-100);//years since 1900
	Write(I2C_ADDRESS, I2C_OFFSET, buf, I2C_DATA_SIZE);
}

tm RTCDriver::ReadDateTime()
{
	u8 buf[I2C_DATA_SIZE];
	Read(I2C_ADDRESS, I2C_OFFSET, buf, I2C_DATA_SIZE);
	tm datetime;
	datetime.tm_sec = bcd2dec(buf[0] & 0x7f);
	datetime.tm_min = bcd2dec(buf[1] & 0x7f);
	datetime.tm_hour = bcd2dec(buf[2] & 0x3f);
	datetime.tm_yday = bcd2dec(buf[3] & 0x3f);
	datetime.tm_wday = bcd2dec(buf[4] & 0x07);
	u8 century = (buf[5]>>7)&1;
	datetime.tm_mon = bcd2dec(buf[5] & 0x1f);
	datetime.tm_year = bcd2dec(buf[6]) + (century ? 100 : 0);
	return datetime;
}

void RTCDriver::Write(unsigned address, unsigned offset, u8* buf, unsigned size)
{
	i2c_cmd_handle_t handle = i2c_cmd_link_create();
	i2c_master_start(handle);
	i2c_master_write_byte(handle, address, true);
	i2c_master_write_byte(handle, offset, true);
	i2c_master_write(handle, buf, size, true);
	i2c_master_stop(handle);
	i2c_master_cmd_begin(I2C_NUM_0, handle, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(handle);
}

void RTCDriver::Read(unsigned address, unsigned offset, u8* buf, unsigned size)
{
	i2c_cmd_handle_t handle = i2c_cmd_link_create();
	i2c_master_start(handle);
	i2c_master_write_byte(handle, address, true);
	i2c_master_write_byte(handle, offset, true);
	i2c_master_start(handle);
	i2c_master_write_byte(handle, address|1, true);
	i2c_master_read(handle, buf, size, I2C_MASTER_LAST_NACK);
	i2c_master_stop(handle);
	i2c_master_cmd_begin(I2C_NUM_0, handle, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(handle);
}
