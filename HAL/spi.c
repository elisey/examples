#include "spi.h"
#include "stm32f10x.h"

volatile bool isTransferComplete = true;
static DMA_InitTypeDef dma;

static void rccInit();
static void gpioInit();
static void spiInit();
static void dmaInit();
static void dmaStart(uint8_t* buffer, size_t size);

void Spi_Init()
{
	rccInit();
	gpioInit();
	spiInit();
    dmaInit();
}

void Spi_StartTransfer(uint8_t* buffer, size_t size)
{
	dmaStart(buffer, size);
}

bool Spi_IsTransferComplete()
{
	return isTransferComplete;
}

/*uint8_t SpiDriver_BlockingTransfer(spi_t* this, uint8_t data)
{
	this->SPIx->DR = (uint16_t) data;
	volatile uint16_t i = 0;
	while ( ! (this->SPIx->SR & SPI_I2S_FLAG_RXNE) && (++i != 0) )
	{
	}
	uint8_t receivedData = (uint8_t)(this->SPIx->DR);
	return receivedData;
}*/

static void rccInit()
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
}

static void gpioInit()
{
	/*GPIO_InitTypeDef gpioIn;
	gpioIn.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpioIn.GPIO_Speed = GPIO_Speed_50MHz;
	gpioIn.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOB, &gpioIn);*/

	GPIO_InitTypeDef gpioOut;
	gpioOut.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioOut.GPIO_Speed = GPIO_Speed_50MHz;
	gpioOut.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_Init(GPIOB, &gpioOut);
}

static void spiInit()
{
	SPI_InitTypeDef spi;
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	spi.SPI_CPOL = SPI_CPOL_Low;
	spi.SPI_CPHA = SPI_CPHA_1Edge;
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	spi.SPI_NSS = SPI_NSS_Soft;
	spi.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &spi);
	SPI_Cmd(SPI2, ENABLE);
}

static void dmaInit()
{
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	dma.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
	dma.DMA_MemoryBaseAddr = (uint32_t)NULL;
	dma.DMA_DIR = DMA_DIR_PeripheralDST;
	dma.DMA_BufferSize = 1;
	dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	dma.DMA_Mode = DMA_Mode_Normal;
	dma.DMA_Priority = DMA_Priority_Medium;
	dma.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel5, &dma);
	DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
	NVIC_SetPriority(DMA1_Channel5_IRQn, 13);
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

static void dmaStart(uint8_t* buffer, size_t size)
{
	DMA_Cmd(DMA1_Channel5, DISABLE);
	dma.DMA_MemoryBaseAddr = (uint32_t)buffer;
	dma.DMA_BufferSize = size;
	DMA_Init(DMA1_Channel5, &dma);
	DMA_Cmd(DMA1_Channel5, ENABLE);
	isTransferComplete = false;
}

void DMA1_Channel5_IRQHandler()
{
	if (DMA_GetITStatus(DMA1_IT_TC5) == SET)
	{
		DMA_ClearITPendingBit(DMA1_IT_TC5);
		isTransferComplete = true;
	}
}
