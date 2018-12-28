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
#ifndef __RPI_ICM20600_H__
#define __RPI_ICM20600_H__

#include <stdint.h>

#define ICM20600_I2C_ADDR0              0x68
#define ICM20600_I2C_ADDR1              0x69

// Gyroscope scale range
typedef enum {
	RANGE_250_DPS = 0,
	RANGE_500_DPS,
	RANGE_1K_DPS,
	RANGE_2K_DPS,
} gyro_scale_type_t;

// Accelerometer scale range
typedef enum {
	RANGE_2G = 0,
	RANGE_4G,
	RANGE_8G,
	RANGE_16G,
} acc_scale_type_t;

// Gyroscope output data rate
typedef enum {
	GYRO_RATE_8K_BW_3281 = 0,
	GYRO_RATE_8K_BW_250,
	GYRO_RATE_1K_BW_176,
	GYRO_RATE_1K_BW_92,
	GYRO_RATE_1K_BW_41,
	GYRO_RATE_1K_BW_20,
	GYRO_RATE_1K_BW_10,
	GYRO_RATE_1K_BW_5,
} gyro_lownoise_odr_type_t;

// Accelerometer output data rate
typedef enum {
	ACC_RATE_4K_BW_1046 = 0,
	ACC_RATE_1K_BW_420,
	ACC_RATE_1K_BW_218,
	ACC_RATE_1K_BW_99,
	ACC_RATE_1K_BW_44,
	ACC_RATE_1K_BW_21,
	ACC_RATE_1K_BW_10,
	ACC_RATE_1K_BW_5,
} acc_lownoise_odr_type_t ;

// Averaging filter settings for Low Power Accelerometer mode
typedef enum {
	ACC_AVERAGE_4 = 0,
	ACC_AVERAGE_8,
	ACC_AVERAGE_16,
	ACC_AVERAGE_32,
} acc_averaging_sample_type_t;

// Averaging filter configuration for low-power gyroscope mode
typedef enum {
	GYRO_AVERAGE_1 = 0,
	GYRO_AVERAGE_2,
	GYRO_AVERAGE_4,
	GYRO_AVERAGE_8,
	GYRO_AVERAGE_16,
	GYRO_AVERAGE_32,
	GYRO_AVERAGE_64,
	GYRO_AVERAGE_128,
} gyro_averaging_sample_type_t;

// ICM20600 power mode
typedef enum {
	ICM_SLEEP_MODE = 0,
	ICM_STANDYBY_MODE,
	ICM_ACC_LOW_POWER,
	ICM_ACC_LOW_NOISE,
	ICM_GYRO_LOW_POWER,
	ICM_GYRO_LOW_NOISE,
	ICM_6AXIS_LOW_POWER,
	ICM_6AXIS_LOW_NOISE,
} icm20600_power_type_t;

typedef struct {
	uint8_t addr;
	double acc_scale;
	double gyro_scale;
} rpi_icm20600_t;

typedef struct icm20600_cfg {
	uint16_t gyro_range;
	uint16_t gyro_rate;
	// Averaging filter configuration for low-power gyroscope mode
	uint16_t gyro_aver;

	uint16_t acc_range;
	uint16_t acc_rate;
	// Averaging filter settings for Low Power Accelerometer mode
	uint16_t acc_aver;

	// Set power mode, the default mode is
	// ICM_6AXIS_LOW_POWER(set in XXX_init() function)
	uint16_t power;

	// Divides the internal sample rate to generate
	// the sensor data output rate. Only works at mode:
	//     ICM_GYRO_LOW_POWER
	//     ICM_ACC_LOW_POWER
	//     ICM_ACC_LOW_NOISE
	// SAMPLE_RATE = 1KHz / (1 + divider)
	uint16_t divider;
} icm20600_cfg_t;

void* rpi_icm20600_alloc(void);
int rpi_icm20600_free(rpi_icm20600_t* dev);

// return >=0: device ID
//         <0: error  #
int rpi_icm20600_init(
	rpi_icm20600_t* dev,
	const char* i2c_dev,
	int i2c_addr,
	const icm20600_cfg_t* conf
);

int rpi_icm20600_get_accel(
	rpi_icm20600_t* dev,
	double* x, double* y, double* z
);

int rpi_icm20600_get_gyro(
	rpi_icm20600_t* dev,
	double* x, double* y, double* z
);

// Yes there is a digital-output temperature sensor in ICM20600,
// return a integer centigrade degree
int rpi_icm20600_get_temperature(
	rpi_icm20600_t* dev,
	double* temperature
);

#endif//__RPI_ICM20600_H__
