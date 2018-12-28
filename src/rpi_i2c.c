/*
 * i2c interface to access i2c-dev
 *
 * Author      : Peter Yang
 * Create Time : Dec 2018
 * Change Log  :
 *     11:09 2018/12/26 Initial version
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
#include "rpi_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RAW_MAX		0x8000
static int rpi_i2c_fd = -1;

int rpi_i2c_init(const char* dev_path) {
	int fd;

	if ((fd = rpi_i2c_fd) >= 0) {
		return fd;
	}

	if ((fd = open(dev_path, O_RDWR)) < 0) {
		printf("Failed to open i2c bus %s, error = %d\n",
		       dev_path, fd);
	} else {
		rpi_i2c_fd = fd;
	}
	return fd;
}

// return none-zero = FAIL
//        zero      = OK
int8_t rpi_i2c_write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	uint8_t* buf = (uint8_t*)malloc(len + 1);
	int rt;

	if (buf == NULL || rpi_i2c_fd < 0) {
		return RPI_I2C_FAIL;
	}

	if ((rt = ioctl(rpi_i2c_fd, I2C_SLAVE, dev_addr) < 0)) {
		printf("Failed to talk to slave %02X, error = %d.\n", dev_addr, rt);
		return RPI_I2C_FAIL;
	}

	buf[0] = reg_addr;
	memcpy(buf + 1, data, len);

	if ((rt = write(rpi_i2c_fd, buf, len + 1)) != len + 1) {
		printf("Failed to write i2c bus %u bytes with error = %d.\n",
		       len + 1, rt);
		rt = RPI_I2C_FAIL;
	} else {
		rt = RPI_I2C_OK;
	}

	free(buf);
	return rt;
}

// return none-zero = FAIL
//        zero      = OK
int8_t rpi_i2c_read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	int rt;

	if (rpi_i2c_fd < 0) {
		return RPI_I2C_FAIL;
	}

	if ((rt = ioctl(rpi_i2c_fd, I2C_SLAVE, dev_addr) < 0)) {
		printf("Failed to talk to slave %02X, error = %d.\n", dev_addr, rt);
		return RPI_I2C_FAIL;
	}

	#if _DEBUG
	/*
	 * fprintf(stderr, "dev: 0x%02X  reg: 0x%02X\n", dev_addr, reg_addr);
	 */
	#endif

	if ((rt = write(rpi_i2c_fd, &reg_addr, 1)) != 1) {
		printf("Failed to write (then read) i2c reg = 0x%02X, error = %d.\n",
		       reg_addr, rt);
		return RPI_I2C_FAIL;
	}

	if ((rt = read(rpi_i2c_fd, data, len)) != len) {
		printf("Failed to read from i2c bus with error = %d.\n", rt);
		return RPI_I2C_FAIL;
	}
	return RPI_I2C_OK;
}

void rpi_delay_ms(uint32_t millis) {
	usleep(millis * 1000UL);
	return;
}

int i2c_read_byte(uint8_t dev, uint8_t reg) {
	uint8_t data;

	if (rpi_i2c_read(dev, reg, &data, 1)) {
		return RPI_I2C_FAIL;
	}
	return data;
}

int i2c_read_word(uint8_t dev, uint8_t reg) {
	uint8_t data[2];

	if (rpi_i2c_read(dev, reg, data, 2)) {
		return RPI_I2C_FAIL;
	}
	return ((unsigned)data[0] << 8) | data[1];
}

int i2c_write_byte(uint8_t dev, uint8_t reg, uint8_t data) {
	return rpi_i2c_write(dev, reg, &data, 1);
}

int i2c_write_word(uint8_t dev, uint8_t reg, uint16_t data) {
	uint8_t buffer[2];

	buffer[0] = data >> 8;
	buffer[1] = data & 0xFF;
	if (rpi_i2c_write(dev, reg, buffer, 2)) {
		return RPI_I2C_FAIL;
	}
	return RPI_I2C_OK;
}

#ifdef __cplusplus
}
#endif
