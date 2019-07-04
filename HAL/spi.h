#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void Spi_Init();
void Spi_StartTransfer(uint8_t* buffer, size_t size);
bool Spi_IsTransferComplete();

//uint8_t SpiDriver_BlockingTransfer(spi_t* this, uint8_t data);
