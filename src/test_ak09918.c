/*	
 * AK09918 device driver
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
#include "rpi_ak09918.h"
#include "rpi_i2c.h"

int main(int argc, char* argv[]) {
	rpi_ak09918_t ak[1];
	double x, y, z;
	int id, rt;
	int delay;

	id = rpi_ak09918_init(
		ak,
		"/dev/i2c-1",
		AK09918_I2C_ADDR,
		AK09918_NORMAL
		);
	printf("Device ID: 0x%02X\n", id);

	#if 0
	fprintf(stderr, "Waiting sensor ready ");
	do {
		rt = rpi_ak09918_is_ready(ak);
		rpi_delay_ms(1);
		fprintf(stderr, ".");
	} while (rt != AK09918_ERR_OK);
	fprintf(stderr, "OK\n");
	rt = rpi_ak09918_read(ak, &x, &y, &z);
	#endif

	rt = rpi_ak09918_self_test(ak);
	printf("SELF TEST RESULT --- %s\n", rpi_ak09918_err_string(rt));

	rpi_ak09918_set_mode(ak, AK09918_POWER_DOWN);
	rpi_ak09918_set_mode(ak, AK09918_CONTINUOUS_100HZ);

	/*
	 * To compute a proper delay, match between
	 * data-output-rate and reading rate
	 */
	delay = 8000; // 8ms

	for (;;) {
		rt = rpi_ak09918_is_ready(ak);
		if (rt != AK09918_ERR_OK) {
			delay++;
			printf("%s usleep = %d us\n", rpi_ak09918_err_string(rt), delay);
			usleep(delay);
			continue;
		}

		rt = rpi_ak09918_is_skip(ak);
		if (rt == AK09918_ERR_DOR) {
			if (delay > 0) delay--;
			printf("%s usleep = %d us\n", rpi_ak09918_err_string(rt), delay);
		}

		rt = rpi_ak09918_read(ak, &x, &y, &z);
		if (rt != AK09918_ERR_OK) {
			printf("%s\n", rpi_ak09918_err_string(rt));
			rpi_delay_ms(1000);
			continue;	
		}
		printf("COMPASS X = %7.2lf uT Y = %7.2lf uT Z = %7.2lf uT\n", x, y, z);

		usleep(delay);
	}
	return 0;
}
