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
#include <stdio.h>
#include <unistd.h>
#include "rpi_icm20600.h"
#include "rpi_i2c.h"

int main(int argc, char* argv[]) {
	rpi_icm20600_t icm[1];
	double x, y, z;
	int id;

	icm20600_cfg_t config[1] = {
		{
		RANGE_2K_DPS,
		GYRO_RATE_1K_BW_176,
		GYRO_AVERAGE_1,
		RANGE_16G,
		ACC_RATE_1K_BW_420,
		ACC_AVERAGE_4,
		ICM_6AXIS_LOW_POWER,
		0
		}
	};

	id = rpi_icm20600_init(
		icm,
		"/dev/i2c-1",
		ICM20600_I2C_ADDR1,
		config
		);

	printf("Device ID: 0x%02X\n", id);

	for (;;) {
		double t;

		rpi_icm20600_get_temperature(icm, &t);
		printf("Thermo  = %7.2lf C\n", t);

		rpi_icm20600_get_accel(icm, &x, &y, &z);
		printf("ACCEL X = %7.2lf mg Y = %7.2lf mg Z = %7.2lf mg\n", x, y, z);

		rpi_icm20600_get_gyro(icm, &x, &y, &z);
		printf("GYRO  X = %7.2lfdps Y = %7.2lfdps Z = %7.2lfdps\n", x, y, z);

		rpi_delay_ms(1000);
	}
	return 0;
}
