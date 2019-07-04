#include "uart.h"

#include "os_queue.h"
#include "os_config.h"

#include <assert.h>

osQueue_t uartRxQueue;
osQueue_t uartTxQueue;

uint32_t DEBUG_uartRxQueueSize = 0;
uint32_t DEBUG_uartTxQueueSize = 0;

void UART_Init(void)
{
    bool result;

    result = Queue_Create(&uartRxQueue, uartRX_BUFFER_SIZE, sizeof(uint8_t));
    assert(result);
    
    result = Queue_Create(&uartTxQueue, uartTX_BUFFER_SIZE, sizeof(uint8_t));
    assert(result);

    GPIO_InitTypeDef gpioStruct;
    USART_InitTypeDef usartStruct;

    USART_RCC_INIT;

    gpioStruct.GPIO_Pin = USART_Pin_Tx;
    gpioStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USART_GPIO, &gpioStruct);

    //GPIO_RX
    gpioStruct.GPIO_Pin = USART_Pin_Rx;
    gpioStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpioStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USART_GPIO, &gpioStruct);

    //USART_Settings
    usartStruct.USART_BaudRate = UART_BOUD_RATE;
    usartStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usartStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usartStruct.USART_Parity = USART_Parity_No;
    usartStruct.USART_StopBits = USART_StopBits_1;
    usartStruct.USART_WordLength = USART_WordLength_8b;
    USART_Init(USARTx, &usartStruct);

    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
    USART_Cmd(USARTx, ENABLE);

    NVIC_SetPriority(USARTx_IRQn, 14);
    NVIC_EnableIRQ(USARTx_IRQn);
}

void UART_SendChar(uint8_t data)
{
    bool result = Queue_Send(&uartTxQueue, &data, OS__MAX_DELAY);
    assert(result);
    DEBUG_uartTxQueueSize = Queue_MessagesWaiting(&uartTxQueue);

    USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

void UART_SendBuffer(const uint8_t *buffer, size_t size)
{
    for (int i = 0; i < size; ++i)
    {
        UART_SendChar(buffer[i]);
    }
}

void UART_SendString(const uint8_t *str)
{
    int i = 0;

    while (str[i] != 0)
    {
        UART_SendChar(str[i]);
        i++;
    }
}

bool UART_GetChar(uint8_t *data, uint32_t timeout)
{
    uint8_t receivedByte = 0;
    bool result = Queue_Receive(&uartRxQueue, &receivedByte, timeout);
    DEBUG_uartRxQueueSize = Queue_MessagesWaiting(&uartRxQueue);

    if (result != true)
    {
        return false;
    }
    *data = receivedByte;
    return true;
}

void USARTx_IRQHandler(void)
{
    if (USART_GetITStatus(USARTx, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USARTx, USART_IT_RXNE);

        uint8_t data = USART_ReceiveData(USARTx);

        Queue_SendFromIsr(&uartRxQueue, (void*)&data);
        DEBUG_uartRxQueueSize = Queue_MessagesWaitingFromIsr(&uartRxQueue);

    }
    else if (USART_GetITStatus(USARTx, USART_IT_TXE) != RESET)
    {
        if (Queue_IsEmptyFromIsr(&uartTxQueue) == true)
        {
            USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
            return;
        }

        uint8_t data;

        bool result = Queue_ReceiveFromIsr(&uartTxQueue, &data);
        DEBUG_uartTxQueueSize = Queue_MessagesWaitingFromIsr(&uartTxQueue);
        if (result == true)
        {
            USART_SendData(USARTx, data);
        }
    }
}
