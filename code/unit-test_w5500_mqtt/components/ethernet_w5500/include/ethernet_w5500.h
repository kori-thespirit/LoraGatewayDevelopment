#ifndef ETHERNET_W5500_H
#define ETHERNET_W5500_H
#include <stddef.h>
#include <stdint.h>

#define SPI2_SCK 4
#define SPI2_MISO 5
#define SPI2_MOSI 6
#define SPI2_CS 7

void spi_bus_init();
void w5500_transmit(uint8_t *data, size_t datasize);
void w5500_init();
void w5500_test_mosi();
int initialize_ping();

#endif // !ETHERNET_W5500_H
