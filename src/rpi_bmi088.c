/*    
 * BMI088 program
 *
 * Tested with Seeed Grove - 6-Axis Accelerometer&Gyroscope (BMI088)
 * Author      : Peter Yang
 * Create Time : Dec 2018
 * Change Log  :
 *     11:09 2018/12/20 Initial version
 *
 * The MIT License (MIT)
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
#define _DEBUG	0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include "rpi_bmi088.h"
#include "rpi_i2c.h"
#include "bmi088.h"

#define RAW_MAX		0x8000

void* rpi_bmi088_alloc(void) {
	return malloc(sizeof(rpi_bmi088_t));
}

int rpi_bmi088_free(rpi_bmi088_t* dev) {
	free(dev);
	return 0;
}

static double accel_range_map[] = {
	3000.0, 6000.0, 12000.0, 24000.0
};
static double gyro_range_map[] = {
	2000.0, 1000.0, 500.0, 250.0, 125.0
};

int rpi_bmi088_init(
	rpi_bmi088_t* dev,
	const char* i2c_dev,
	int accel_addr,
	int gyro_addr,
	const struct bmi08x_cfg* accel,
	const struct bmi08x_cfg* gyro
) {
	/* Declare an instance of the BMI088 device */
	int rt = BMI08X_OK;
	int pwr_mode, range;

	#if _DEBUG
	printf("%s() +++\n", __func__);
	#endif

	if ((rt = rpi_i2c_init(i2c_dev)) < 0) {
		return rt;
	}

	/* fill device parameters */
	dev->bmi.accel_id 	= accel_addr;
	dev->bmi.gyro_id	= gyro_addr;
	dev->bmi.intf 		= BMI08X_I2C_INTF;
	dev->bmi.read 		= rpi_i2c_read;
	dev->bmi.write 		= rpi_i2c_write;
	dev->bmi.delay_ms 	= rpi_delay_ms;
	dev->bmi.read_write_len = 31;

	/* init device. */
	rt = bmi088_init(&dev->bmi);

	/* Read Chip ID from the accel */
	if (rt == BMI08X_OK) {
		uint8_t data = 0;
		rt = bmi08a_get_regs(BMI08X_ACCEL_CHIP_ID_REG, &data, 1, &dev->bmi);
		if (rt != BMI08X_OK) {
			return BMI08X_E_COM_FAIL;
		}
		#if _DEBUG
		printf("%s() L%d ACCEL ID = 0x%02X\n", __func__, __LINE__, data);
		#endif
		
		/* Read gyro chip id */
		rt = bmi08g_get_regs(BMI08X_GYRO_CHIP_ID_REG, &data, 1, &dev->bmi);
		if (rt != BMI08X_OK) {
			return BMI08X_E_COM_FAIL;
		}
		#if _DEBUG
		printf("%s() L%d GYRO  ID = 0x%02X\n", __func__, __LINE__, data);
		#endif
	} else {
		#if _DEBUG
		printf("%s() L%d error = %d\n", __func__, __LINE__, rt);
		#endif
		return rt;
	}

	/* Configuring the accelerometer */
	dev->bmi.accel_cfg = *accel;
	rt = bmi08a_set_power_mode(&dev->bmi);

	/*
	 *  Wait for 10ms to switch between the power modes
	 *  delay taken care inside the function
	 *
	 *  rpi_bmi->bmi.delay_ms(10);
	 */
	rt = bmi08a_set_meas_conf(&dev->bmi);

	/* Read the accel power mode */
	pwr_mode = bmi08a_get_power_mode(&dev->bmi);
	/* prevent warning */
	pwr_mode = pwr_mode;

	range = (accel->range > BMI088_ACCEL_RANGE_24G)?
		BMI088_ACCEL_RANGE_24G: accel->range;
	dev->accel_range = accel_range_map[range];

	/* Configuring the gyro */
	dev->bmi.gyro_cfg = *gyro;

	rt = bmi08g_set_power_mode(&dev->bmi);
	/*
	 * Wait for 30ms to switch between the power modes -
	 * delay taken care inside the function
	 */
	rt = bmi08g_set_meas_conf(&dev->bmi);

	range = (gyro->range > BMI08X_GYRO_RANGE_125_DPS)?
		BMI08X_GYRO_RANGE_125_DPS: gyro->range;
	dev->gyro_range = gyro_range_map[range];

	#if _DEBUG
	printf("%s() ---\n", __func__);
	#endif
	return BMI08X_OK;
}

int rpi_bmi088_get_accel(
	rpi_bmi088_t* dev,
	double* x, double* y, double* z
) {
	int rt;

	rt = bmi08a_get_data(&dev->acc, &dev->bmi);
	if (rt != BMI08X_OK) {
		return rt;
	}

	*x = dev->acc.x * dev->accel_range / RAW_MAX;
	*y = dev->acc.y * dev->accel_range / RAW_MAX;
	*z = dev->acc.z * dev->accel_range / RAW_MAX;
	return BMI08X_OK;
}

int rpi_bmi088_get_gyro(
	rpi_bmi088_t* dev,
	double* x, double* y, double* z
) {
	int rt;

	rt = bmi08g_get_data(&dev->gyr, &dev->bmi);
	if (rt != BMI08X_OK) {
		return rt;
	}

	*x = dev->gyr.x * dev->gyro_range / RAW_MAX;
	*y = dev->gyr.y * dev->gyro_range / RAW_MAX;
	*z = dev->gyr.z * dev->gyro_range / RAW_MAX;
	return BMI08X_OK;
}

uint32_t rpi_bmi088_get_sensor_time(
	rpi_bmi088_t* dev
) {
	uint32_t snr_tm = 0;

	/* Read the sensor time */
	bmi08a_get_sensor_time(&dev->bmi, &snr_tm);

	return snr_tm;
}

#ifdef _HAS_MAIN
#include "main.c"
#endif
