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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include "rpi_bmi088.h"

int main(int argc, const char* argv[]) {
	rpi_bmi088_t rpi_bmi[1];
	uint32_t tm;
	double x, y, z;

	struct bmi08x_cfg accel_cfg[1] = {
		{
		BMI08X_ACCEL_PM_ACTIVE,
		BMI088_ACCEL_RANGE_6G,
		BMI08X_ACCEL_BW_NORMAL,
		BMI08X_ACCEL_ODR_100_HZ
		}
	};
	struct bmi08x_cfg gyro_cfg[1] = {
		{
		BMI08X_GYRO_PM_NORMAL,
		BMI08X_GYRO_RANGE_1000_DPS,
		BMI08X_GYRO_BW_23_ODR_200_HZ,
		BMI08X_GYRO_BW_23_ODR_200_HZ
		}
	};

	/* prevent warning */
	argc = argc;
	argv[0] = argv[0];

	rpi_bmi088_init(rpi_bmi,
			"/dev/i2c-1",
			BMI08X_ACCEL_I2C_ADDR_SECONDARY,
			BMI08X_GYRO_I2C_ADDR_SECONDARY,
			accel_cfg,
			gyro_cfg
			);

	for (;;) {
		tm = rpi_bmi088_get_sensor_time(rpi_bmi);
		printf("Sensor time: %5u\n",    tm);

		rpi_bmi088_get_accel(rpi_bmi, &x, &y, &z);
		printf("ACCEL X = %7.2lf mg Y = %7.2lf mg Z = %7.2lf mg\n", x, y, z);

		rpi_bmi088_get_gyro(rpi_bmi, &x, &y, &z);
		printf("GYRO  X = %7.2lfdps Y = %7.2lfdps Z = %7.2lfdps\n", x, y, z);

		rpi_bmi->bmi.delay_ms(1000);
	}
	return 0;
}
