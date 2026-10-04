#ifndef __SPI_TASK_H
#define __SPI_TASK_H

typedef enum {
    spi_init = 0x00,
    spi_tx   = 0x01
} spi_data_struct;

typedef enum {
    spi_master = 0x00,
    spi_slave  = 0x01
} spi_mode;

typedef enum {
    spi_data_8b  = 0x00,
    spi_data_16b = 0x01
} spi_data_size;

typedef enum {
    spi_msb_first = 0x00,
    spi_lsb_first = 0x01
} spi_first_bit;

typedef enum {
    spi_prescaler2   = 0x00,
    spi_prescaler4   = 0x01,
    spi_prescaler8   = 0x02,
    spi_prescaler16  = 0x03,
    spi_prescaler32  = 0x04,
    spi_prescaler64  = 0x05,
    spi_prescaler128 = 0x06,
    spi_prescaler256 = 0x07
} spi_prescaler;

typedef enum {
    spi_clock_low  = 0x00,
    spi_clock_high = 0x01
} spi_clock_polarity;

typedef enum {
    spi_clock_1edge = 0x00,
    spi_clock_2edge = 0x01
} spi_clock_phase;

void spiTask(void);

#endif
