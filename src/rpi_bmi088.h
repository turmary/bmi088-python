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
#ifndef __RPI_BMI088_H__
#define __RPI_BMI088_H__

#include "bmi08x.h"

#define BMI088_I2C_ADDR		0x19

typedef struct {
	struct bmi08x_dev bmi;
	struct bmi08x_sensor_data acc;
	struct bmi08x_sensor_data gyr;
	double accel_range;
	double gyro_range;
} rpi_bmi088_t;

void* rpi_bmi088_alloc(void);
int rpi_bmi088_free(rpi_bmi088_t* dev);

extern int rpi_bmi088_init(
	rpi_bmi088_t* dev,
	const char* i2c_dev,
	int accel_addr,
	int gyro_addr,
	const struct bmi08x_cfg* accel,
	const struct bmi08x_cfg* gyro
);

extern int rpi_bmi088_get_accel(
	rpi_bmi088_t* dev,
	double* x, double* y, double* z
);

extern uint32_t rpi_bmi088_get_sensor_time(
	rpi_bmi088_t* dev
);

extern int rpi_bmi088_get_gyro(
	rpi_bmi088_t* dev,
	double* x, double* y, double* z
);

#endif//__RPI_BMI088_H__
