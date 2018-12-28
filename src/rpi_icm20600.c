/*    
 * ICM20600 device driver
 *
 * log: 11:09 2018/12/26 Initial version
 *      Base on https://github.com/Seeed-Studio/Seeed_ICM20600_AK09918.git
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2018  Peter Yang <turmary@126.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software", to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdlib.h>
#include <unistd.h>
#include "rpi_icm20600.h"
#include "rpi_i2c.h"

/***************************************************************
 ICM20600 I2C Register
 ***************************************************************/
#define ICM20600_XG_OFFS_TC_H           0x04
#define ICM20600_XG_OFFS_TC_L           0x05
#define ICM20600_YG_OFFS_TC_H           0x07
#define ICM20600_YG_OFFS_TC_L           0x08
#define ICM20600_ZG_OFFS_TC_H           0x0a
#define ICM20600_ZG_OFFS_TC_L           0x0b
#define ICM20600_SELF_TEST_X_ACCEL      0x0d
#define ICM20600_SELF_TEST_Y_ACCEL      0x0e
#define ICM20600_SELF_TEST_Z_ACCEL      0x0f
#define ICM20600_XG_OFFS_USRH           0x13
#define ICM20600_XG_OFFS_USRL           0x14
#define ICM20600_YG_OFFS_USRH           0x15
#define ICM20600_YG_OFFS_USRL           0x16
#define ICM20600_ZG_OFFS_USRH           0x17
#define ICM20600_ZG_OFFS_USRL           0x18
#define ICM20600_SMPLRT_DIV             0x19
#define ICM20600_CONFIG                 0x1a
#define ICM20600_GYRO_CONFIG            0x1b
#define ICM20600_ACCEL_CONFIG           0x1c
#define ICM20600_ACCEL_CONFIG2          0x1d
#define ICM20600_GYRO_LP_MODE_CFG       0x1e
#define ICM20600_ACCEL_WOM_X_THR        0x20
#define ICM20600_ACCEL_WOM_Y_THR        0x21
#define ICM20600_ACCEL_WOM_Z_THR        0x22
#define ICM20600_FIFO_EN                0x23
#define ICM20600_FSYNC_INT              0x36
#define ICM20600_INT_PIN_CFG            0x37
#define ICM20600_INT_ENABLE             0x38
#define ICM20600_FIFO_WM_INT_STATUS     0x39
#define ICM20600_INT_STATUS             0x3a
#define ICM20600_ACCEL_XOUT_H           0x3b
#define ICM20600_ACCEL_XOUT_L           0x3c
#define ICM20600_ACCEL_YOUT_H           0x3d
#define ICM20600_ACCEL_YOUT_L           0x3e
#define ICM20600_ACCEL_ZOUT_H           0x3f
#define ICM20600_ACCEL_ZOUT_L           0x40
#define ICM20600_TEMP_OUT_H             0x41
#define ICM20600_TEMP_OUT_L             0x42
#define ICM20600_GYRO_XOUT_H            0x43
#define ICM20600_GYRO_XOUT_L            0x44
#define ICM20600_GYRO_YOUT_H            0x45
#define ICM20600_GYRO_YOUT_L            0x46
#define ICM20600_GYRO_ZOUT_H            0x47
#define ICM20600_GYRO_ZOUT_L            0x48
#define ICM20600_SELF_TEST_X_GYRO       0x50
#define ICM20600_SELF_TEST_Y_GYRO       0x51
#define ICM20600_SELF_TEST_Z_GYRO       0x52
#define ICM20600_FIFO_WM_TH1            0x60
#define ICM20600_FIFO_WM_TH2            0x61
#define ICM20600_SIGNAL_PATH_RESET      0x68
#define ICM20600_ACCEL_INTEL_CTRL       0x69
#define ICM20600_USER_CTRL              0x6A
#define ICM20600_PWR_MGMT_1             0x6b
#define ICM20600_PWR_MGMT_2             0x6c
#define ICM20600_I2C_IF                 0x70
#define ICM20600_FIFO_COUNTH            0x72
#define ICM20600_FIFO_COUNTL            0x73
#define ICM20600_FIFO_R_W               0x74
#define ICM20600_WHO_AM_I               0x75
#define ICM20600_XA_OFFSET_H            0x77
#define ICM20600_XA_OFFSET_L            0x78
#define ICM20600_YA_OFFSET_H            0x7a
#define ICM20600_YA_OFFSET_L            0x7b
#define ICM20600_ZA_OFFSET_H            0x7d
#define ICM20600_ZA_OFFSET_L            0x7e

#define ICM20600_FIFO_EN_BIT            (1 << 6)
#define ICM20600_FIFO_RST_BIT           (1 << 2)
#define ICM20600_RESET_BIT              (1 << 0)
#define ICM20600_DEVICE_RESET_BIT       (1 << 7)

#define RAW_MAX                         0x8000

int icm20600_set_power_mode(rpi_icm20600_t* dev, icm20600_power_type_t mode) {
	uint8_t pwr1, pwr2 = 0x00;
	uint8_t gyro_lp;

	pwr1 = i2c_read_byte(dev->addr, ICM20600_PWR_MGMT_1) & 0x8F;

	// When set to ‘1’ low-power gyroscope mode is enabled.
	// Default setting is 0
	gyro_lp = i2c_read_byte(dev->addr, ICM20600_GYRO_LP_MODE_CFG) & 0x7F;

	switch(mode) {
	case ICM_SLEEP_MODE:
		pwr1 |= 0x40;		// set 0b01000000
		break;

	case ICM_STANDYBY_MODE:
		pwr1 |= 0x10;		// set 0b00010000
		pwr2 = 0x38;		// 0x00111000 disable acc
		break;

	case ICM_ACC_LOW_POWER:
		pwr1 |= 0x20;		// set bit5 0b00100000
		pwr2 = 0x07;		// 0x00000111 disable gyro
		break;

	case ICM_ACC_LOW_NOISE:
		pwr1 |= 0x00;	 
		pwr2 = 0x07;		// 0x00000111 disable gyro
		break;

	case ICM_GYRO_LOW_POWER:
		pwr1 |= 0x00;		// dont set bit5 0b00000000
		pwr2 = 0x38;		// 0x00111000 disable acc
		gyro_lp |= 0x80;
		break;

	case ICM_GYRO_LOW_NOISE:
		pwr1 |= 0x00;  
		pwr2 = 0x38;		// 0x00111000 disable acc
		break;

	case ICM_6AXIS_LOW_POWER:
		pwr1 |= 0x00;		// dont set bit5 0b00100000
		gyro_lp |= 0x80;
		break;

	case ICM_6AXIS_LOW_NOISE:
	default:
		pwr1 |= 0x00;  
		break;
	}
	i2c_write_byte(dev->addr, ICM20600_PWR_MGMT_1, pwr1);
	i2c_write_byte(dev->addr, ICM20600_PWR_MGMT_2, pwr2);
	i2c_write_byte(dev->addr, ICM20600_GYRO_LP_MODE_CFG, gyro_lp);
	return 0;
}

int icm20600_set_gyro_range(rpi_icm20600_t* dev, gyro_scale_type_t range) {
	uint8_t data = 0;

	data = i2c_read_byte(dev->addr, ICM20600_GYRO_CONFIG) & 0xE7;

	switch(range){
	case RANGE_250_DPS:
		data |= 0x00;   // 0bxxx00xxx
		dev->gyro_scale = 250.0;
		break;

	case RANGE_500_DPS:
		data |= 0x08;   // 0bxxx00xxx
		dev->gyro_scale = 500.0;
		break;

	case RANGE_1K_DPS:
		data |= 0x10;   // 0bxxx10xxx
		dev->gyro_scale = 1000.0;
		break;

	default:
	case RANGE_2K_DPS:
		data |= 0x18;   // 0bxxx11xxx
		dev->gyro_scale = 2000.0;
		break;
	}
	i2c_write_byte(dev->addr, ICM20600_GYRO_CONFIG, data);
	return 0;
}

int icm20600_set_gyro_rate(rpi_icm20600_t* dev, gyro_lownoise_odr_type_t odr) {
	uint8_t data;

	// DLPF_CFG[2:0] 0b11111000
	data = i2c_read_byte(dev->addr, ICM20600_CONFIG) & 0xF8;

	switch(odr) {
	case GYRO_RATE_8K_BW_3281: data |= 0x07; break;
	case GYRO_RATE_8K_BW_250:  data |= 0x00; break;
	case GYRO_RATE_1K_BW_176:  data |= 0x01; break;
	case GYRO_RATE_1K_BW_92:   data |= 0x02; break;
	case GYRO_RATE_1K_BW_41:   data |= 0x03; break;
	case GYRO_RATE_1K_BW_20:   data |= 0x04; break;
	case GYRO_RATE_1K_BW_10:   data |= 0x05; break;
	default:
	case GYRO_RATE_1K_BW_5:    data |= 0x06; break;
	}
	i2c_write_byte(dev->addr, ICM20600_CONFIG, data);
	return 0;
}

int icm20600_set_gyro_aver(rpi_icm20600_t* dev, gyro_averaging_sample_type_t sample) {
	uint8_t data = 0;

	data = i2c_read_byte(dev->addr, ICM20600_GYRO_LP_MODE_CFG) & 0x8F;

	switch(sample){
	case GYRO_AVERAGE_1:  data |= 0x00; break;
	case GYRO_AVERAGE_2:  data |= 0x10; break;
	case GYRO_AVERAGE_4:  data |= 0x20; break;
	case GYRO_AVERAGE_8:  data |= 0x30; break;
	case GYRO_AVERAGE_16: data |= 0x40; break;
	case GYRO_AVERAGE_32: data |= 0x50; break;
	case GYRO_AVERAGE_64: data |= 0x60; break;
	default:
	case GYRO_AVERAGE_128:data |= 0x70; break;
	}
	i2c_write_byte(dev->addr, ICM20600_GYRO_LP_MODE_CFG, data);
	return 0;
}

int icm20600_set_acc_range(rpi_icm20600_t* dev, acc_scale_type_t range) {
	uint8_t data;

	data = i2c_read_byte(dev->addr, ICM20600_ACCEL_CONFIG) & 0xE7;

	switch(range) {
	case RANGE_2G:
		data |= 0x00;   // 0bxxx00xxx
		dev->acc_scale = 2000.0;
		break;
		
	case RANGE_4G:
		data |= 0x08;   // 0bxxx01xxx
		dev->acc_scale = 4000.0;
		break;

	case RANGE_8G:
		data |= 0x10;   // 0bxxx10xxx
		dev->acc_scale = 8000.0;
		break;

	case RANGE_16G:
	default:
		data |= 0x18;   // 0bxxx11xxx
		dev->acc_scale = 16000.0;
		break;
	}
	i2c_write_byte(dev->addr, ICM20600_ACCEL_CONFIG, data);
	return 0;
}

int icm20600_set_acc_rate(rpi_icm20600_t* dev, acc_lownoise_odr_type_t odr) {
	uint8_t data;

	data = i2c_read_byte(dev->addr, ICM20600_ACCEL_CONFIG2) & 0xF0;

	switch (odr) {
	case ACC_RATE_4K_BW_1046: data |= 0x08; break;
	case ACC_RATE_1K_BW_420:  data |= 0x07; break;
	case ACC_RATE_1K_BW_218:  data |= 0x01; break;
	case ACC_RATE_1K_BW_99:   data |= 0x02; break;
	case ACC_RATE_1K_BW_44:   data |= 0x03; break;
	case ACC_RATE_1K_BW_21:   data |= 0x04; break;
	case ACC_RATE_1K_BW_10:   data |= 0x05; break;
	default:
	case ACC_RATE_1K_BW_5:    data |= 0x06; break;
	}
	i2c_write_byte(dev->addr, ICM20600_ACCEL_CONFIG2, data);
}

int icm20600_set_acc_aver(rpi_icm20600_t* dev, acc_averaging_sample_type_t sample) {
	uint8_t data = 0;

	data = i2c_read_byte(dev->addr, ICM20600_ACCEL_CONFIG2) & 0xCF;

	switch(sample) {
	case ACC_AVERAGE_4:  data |= 0x00; break;
	case ACC_AVERAGE_8:  data |= 0x10; break;
	case ACC_AVERAGE_16: data |= 0x20; break;
	default:
	case ACC_AVERAGE_32: data |= 0x30; break;		
	}
	i2c_write_byte(dev->addr, ICM20600_ACCEL_CONFIG2, data);
	return 0;
}

void* rpi_icm20600_alloc(void) {
	return malloc(sizeof(rpi_icm20600_t));
}

int rpi_icm20600_free(rpi_icm20600_t* dev) {
	free(dev);
	return 0;
}

int rpi_icm20600_init(
	rpi_icm20600_t* dev,
	const char* i2c_dev,
	int i2c_addr,
	const icm20600_cfg_t* conf
) {
	uint8_t dummy;
	int rt;

	dev->addr = i2c_addr;

	if ((rt = rpi_i2c_init(i2c_dev)) < 0) {
		return rt;
	}

	// reset device
	dummy = i2c_read_byte(dev->addr, ICM20600_USER_CTRL);
	// ICM20600_USER_CTRL[0] 0b11111110
	dummy |= ICM20600_RESET_BIT;
	i2c_write_byte(dev->addr, ICM20600_USER_CTRL, dummy);

	usleep(100);

	// configuration
	i2c_write_byte(dev->addr, ICM20600_CONFIG, 0x00);
	// disable fifo
	i2c_write_byte(dev->addr, ICM20600_FIFO_EN, 0x00);

	// set default power mode
	icm20600_set_power_mode(dev, conf->power);

	// gyro config
	icm20600_set_gyro_range(dev, conf->gyro_range);
	icm20600_set_gyro_rate(dev,  conf->gyro_rate);
	// for low power mode only
	icm20600_set_gyro_aver(dev,  conf->gyro_aver);

	// accel config
	icm20600_set_acc_range(dev,  conf->acc_range);
	icm20600_set_acc_rate(dev,   conf->acc_rate);
	// for low power mode only
	icm20600_set_acc_aver(dev,   conf->acc_aver);

	// SAMPLE_RATE = 1KHz / (1 + divider)
	// work for low-power gyroscope
	//          low-power accelerometer
	//          low-noise accelerometer
	i2c_write_byte(dev->addr, ICM20600_SMPLRT_DIV, conf->divider);

	dummy = i2c_read_byte(dev->addr, ICM20600_WHO_AM_I);
	return dummy;
}

int rpi_icm20600_get_accel(
	rpi_icm20600_t* dev,
	double* x, double* y, double* z
) {
	int16_t rx, ry, rz;

	rx = i2c_read_word(dev->addr, ICM20600_ACCEL_XOUT_H);
	ry = i2c_read_word(dev->addr, ICM20600_ACCEL_YOUT_H);
	rz = i2c_read_word(dev->addr, ICM20600_ACCEL_ZOUT_H);

	*x = dev->acc_scale * rx / RAW_MAX;
	*y = dev->acc_scale * ry / RAW_MAX;
	*z = dev->acc_scale * rz / RAW_MAX;
	return 0;
}

int rpi_icm20600_get_gyro(
	rpi_icm20600_t* dev,
	double* x, double* y, double* z
) {
	int16_t rx, ry, rz;

	rx = i2c_read_word(dev->addr, ICM20600_GYRO_XOUT_H);
	ry = i2c_read_word(dev->addr, ICM20600_GYRO_YOUT_H);
	rz = i2c_read_word(dev->addr, ICM20600_GYRO_ZOUT_H);

	*x = dev->gyro_scale * rx / RAW_MAX;
	*y = dev->gyro_scale * ry / RAW_MAX;
	*z = dev->gyro_scale * rz / RAW_MAX;
	return 0;
}

// Yes there is a digital-output temperature sensor in ICM20600
// return in centigrade degree
int rpi_icm20600_get_temperature(
	rpi_icm20600_t* dev,
	double* temperature
) {
	int16_t raw;

	raw = i2c_read_word(dev->addr, ICM20600_TEMP_OUT_H);
	/* Datasheet coefficients */
	*temperature = raw / 326.8 + 25.0;
	return 0;
}
