/** \file server/drivers/hd44780-i2c.c
 * \c i2c connection type of \c hd44780 driver for Hitachi HD44780 based LCD displays.
 *
 * The LCD is operated in its 4 bit-mode to be connected to the 8 bit-port
 * of a single PCF8574(A) or PCA9554(A) that is accessed by the server via the i2c bus.
 */

/* Copyright (c) 2020 Ezra Buehler <ezra@easyb.ch>
 *
 * This file is released under the GNU General Public License. Refer to the
 * COPYING file distributed with this package.
 */

#include "sevenseg-i2c.h"

#include "shared/report.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "i2c.h"

void i2c_sevenseg_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch);
void i2c_sevenseg_backlight(PrivateData *p, unsigned char state);
void i2c_sevenseg_close(PrivateData *p);

#define I2C_ADDR_MASK 0x7f

static void
i2c_out(PrivateData *p, unsigned char val)
{
	unsigned char data[2];
	int datalen;
	static int no_more_errormsgs = 0;

	data[0] = val;
	datalen = 1;

	if (i2c_write_no_ack(p->i2c, data, datalen) < 0) {
		p->sevenseg_functions->drv_report(no_more_errormsgs ? RPT_DEBUG:RPT_ERR,
										  "sevenseg: I2C: i2c write data %u failed: %s",
										  val, strerror(errno));
		no_more_errormsgs = 1;
	}
}


/**
 * Initialize the driver.
 * \param drvthis  Pointer to driver structure.
 * \retval 0       Success.
 * \retval -1      Error.
 */
int
sevenseg_init_i2c(Driver *drvthis)
{
	PrivateData *p = (PrivateData*) drvthis->private_data;
	sevenseg_functions *sevenseg_functions = p->sevenseg_functions;
	char device[256] = I2C_DEFAULT_DEVICE;

	/* READ CONFIG FILE */

	/* Get serial device to use */
	strncpy(device, drvthis->config_get_string(drvthis->name, "Device", 0, I2C_DEFAULT_DEVICE), sizeof(device));
	device[sizeof(device) - 1] = '\0';

	report(RPT_INFO,"sevenseg: I2C: Using device '%s' and address 0x%02X for a %s",
		device, p->port & I2C_ADDR_MASK, (p->port & I2C_PCAX_MASK) ? "PCA9554(A)" : "PCF8574(A)");

	p->i2c = i2c_open(device, p->port & I2C_ADDR_MASK);
	if (!p->i2c) {
		report(RPT_ERR, "sevenseg: I2C: connecting to device '%s' slave 0x%02X failed:", device, p->port & I2C_ADDR_MASK, strerror(errno));
		return(-1);
	}

	sevenseg_functions->senddata = i2c_sevenseg_senddata;
	sevenseg_functions->backlight = i2c_sevenseg_backlight;
	sevenseg_functions->close = i2c_sevenseg_close;

	// powerup the lcd now
	/* We'll now send 0x03 a couple of times,
	 * which is in fact (FUNCSET | IF_8BIT) >> 4
	 */
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		sevenseg_functions->uPause(p, 1);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		sevenseg_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	sevenseg_functions->uPause(p, 15000);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		sevenseg_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	sevenseg_functions->uPause(p, 5000);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		sevenseg_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	sevenseg_functions->uPause(p, 100);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D4 | p->i2c_line_D5);
	if (p->delayBus)
		sevenseg_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D4 | p->i2c_line_D5);
	sevenseg_functions->uPause(p, 100);

	// now in 8-bit mode...  set 4-bit mode
	/*
	OLD   (FUNCSET | IF_4BIT) >> 4 0x02
	ALT   (FUNCSET | IF_4BIT)      0x20
	*/
	i2c_out(p, p->i2c_line_D5);
	if (p->delayBus)
		sevenseg_functions->uPause(p, 1);

	i2c_out(p, p->i2c_line_EN | p->i2c_line_D5);
	if (p->delayBus)
		sevenseg_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_D5);
	sevenseg_functions->uPause(p, 100);

	// Set up two-line, small character (5x8) mode
	//sevenseg_functions->senddata(p, 0, RS_INSTR, FUNCSET | IF_4BIT | TWOLINE | SMALLCHAR);
	//sevenseg_functions->uPause(p, 40);

	common_init(p, IF_4BIT);

	return 0;
}

void
i2c_sevenseg_close(PrivateData *p) {
	if (p->i2c >= 0)
		i2c_close(p->i2c);
}


/**
 * Send data or commands to the display.
 * \param p          Pointer to driver's private data structure.
 * \param displayID  ID of the display (or 0 for all) to send data to.
 * \param flags      Defines whether to end a command or data.
 * \param ch         The value to send.
 */
void
i2c_sevenseg_senddata(PrivateData *p, unsigned char displayID, unsigned char flags, unsigned char ch)
{
	unsigned char portControl = 0;
	unsigned char h=0;
	unsigned char l=0;
	if( ch & 0x80 ) h |= p->i2c_line_D7;
	if( ch & 0x40 ) h |= p->i2c_line_D6;
	if( ch & 0x20 ) h |= p->i2c_line_D5;
	if( ch & 0x10 ) h |= p->i2c_line_D4;
	if( ch & 0x08 ) l |= p->i2c_line_D7;
	if( ch & 0x04 ) l |= p->i2c_line_D6;
	if( ch & 0x02 ) l |= p->i2c_line_D5;
	if( ch & 0x01 ) l |= p->i2c_line_D4;
	if (flags == RS_INSTR)
		portControl = 0;
	else //if (flags == RS_DATA)
		portControl = p->i2c_line_RS;

	portControl |= p->backlight_bit;

	i2c_out(p, portControl | h);
	if (p->delayBus)
		p->sevenseg_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_EN | portControl | h);
	if (p->delayBus)
		p->sevenseg_functions->uPause(p, 1);
	i2c_out(p, portControl | h);

	i2c_out(p, portControl | l);
	if (p->delayBus)
		p->sevenseg_functions->uPause(p, 1);
	i2c_out(p, p->i2c_line_EN | portControl | l);
	if (p->delayBus)
		p->sevenseg_functions->uPause(p, 1);
	i2c_out(p, portControl | l);
}


/**
 * Turn display backlight on or off.
 * \param p      Pointer to driver's private data structure.
 * \param state  New backlight status.
 */
void i2c_sevenseg_backlight(PrivateData *p, unsigned char state)
{
	if ( p->i2c_backlight_invert == 0 )
		p->backlight_bit = ((!have_backlight_pin(p)||state) ? 0 : p->i2c_line_BL);
	else // Inverted backlight - npn transistor
		p->backlight_bit = ((have_backlight_pin(p) && state) ? p->i2c_line_BL : 0);
	i2c_out(p, p->backlight_bit);
}
