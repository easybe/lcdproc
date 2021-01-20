/** \file server/drivers/hd44780-drivers.h
 * Interface to low-level driver types, headers and names.
 *
 * To add support for a new driver in this file:
 * -# include your header file
 * -# Add a new connectionType
 * -# Add an entry in the \c ConnectionMapping structure
 */

#ifndef SEVENSEG_DRIVERS_H
#define SEVENSEG_DRIVERS_H

/* sevenseg specific header files */
#ifdef HAVE_I2C
# include "sevenseg-i2c.h"
#endif
#ifdef HAVE_SPI
# include "sevenseg-spi.h"
#endif


/** connectionType mapping table:
 * - string to identify connection in config file
 * - connection type identifier
 * - interface type
 * - initialisation function
 */
static const ConnectionMapping connectionMapping[] = {
#ifdef HAVE_I2C
	{"i2c", SEVENSEG_CT_I2C, IF_TYPE_I2C, sevenseg_init_i2c},
#endif
#ifdef HAVE_SPI
	{"spi", SEVENSEG_CT_SPI, IF_TYPE_SPI, sevenseg_init_spi},
#endif
	/* default, end of structure element (do not delete) */
	{NULL, SEVENSEG_CT_UNKNOWN, IF_TYPE_UNKNOWN, NULL}
};

#endif
