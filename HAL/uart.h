#ifndef UART_H
#define UART_H

#include "stm32f10x.h"
#include <stdbool.h>
#include <stddef.h>

#include "hardware_config.h"


#define UART_BOUD_RATE         		115200L
#define uartTX_BUFFER_SIZE  		256
#define uartRX_BUFFER_SIZE          256

#if (UART_MODULE == 1)
#define USARTx						USART1
#define USART_Pin_Tx				GPIO_Pin_9
#define USART_Pin_Rx				GPIO_Pin_10
#define USART_GPIO					GPIOA
#define USARTx_IRQHandler			USART1_IRQHandler
#define USARTx_IRQn					USART1_IRQn
#define USART_RCC_INIT				do { 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);	\
											RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 ,ENABLE); }\
									while(0)
#elif (UART_MODULE == 2)
#define USARTx						USART2
#define USART_Pin_Tx				GPIO_Pin_2
#define USART_Pin_Rx				GPIO_Pin_3
#define USART_GPIO					GPIOA
#define USARTx_IRQHandler			USART2_IRQHandler
#define USARTx_IRQn					USART2_IRQn
#define USART_RCC_INIT				do {	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);	\
											RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 ,ENABLE); }\
									while(0)
#elif (UART_MODULE == 3)
#define USARTx						USART3
#define USART_Pin_Tx				GPIO_Pin_10
#define USART_Pin_Rx				GPIO_Pin_11
#define USART_GPIO					GPIOB
#define USARTx_IRQHandler			USART3_IRQHandler
#define USARTx_IRQn					USART3_IRQn
#define USART_RCC_INIT				do {	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);	\
											RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 ,ENABLE); }\
									while(0)
#endif

void UART_Init();
void UART_SendChar(uint8_t data);
void UART_SendBuffer(const uint8_t *buffer, size_t size);
void UART_SendString(const uint8_t *str);
bool UART_GetChar(uint8_t *data, uint32_t timeout);

#endif
