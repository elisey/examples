#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    PARSER_ERROR_OK = 0,
    PARSER_ERROR_EMPTY_PARAMETER = 1,
    PARSER_ERROR_WRONG_PARAMETER = 2,
    PARSER_ERROR_OUT_OF_PARAMETERS = 3,
} parserError_t;

typedef struct
{
    uint8_t *ptrBuffer;
    uint16_t maxNumOfElements;
    uint16_t numOfElements;
    uint16_t searchParamIndex;
} packet_t;

void Packet_Init(packet_t *packet, uint8_t *workBuffer, uint16_t sizeOfBuffer);
uint8_t *Packet_GetBufferPtr(packet_t *packet);
uint16_t Packet_GetMaxNumOfElements(packet_t *packet);

bool Packet_InsertByte(packet_t *packet, uint8_t byte);
uint16_t Packet_GetNumOfElements(packet_t *packet);
void Packet_SetNumOfElements(packet_t *packet, uint16_t numOfElements);
void Packet_Clear(packet_t *packet);
bool Packet_CheckCrc(packet_t *packet);

bool Packet_GetCommandCode(packet_t *packet, uint8_t *ptrCommand);
parserError_t Packet_GetParamUint32(packet_t *packet, uint32_t *ptrData);
parserError_t Packet_GetParamString(packet_t *packet, uint8_t **ptrPtrString, uint16_t *ptrSize);
parserError_t Packet_GetParamBuffer(packet_t *packet, uint8_t **ptrPtrBuffer, uint16_t *ptrSize);

void Packet_InsertCommandCode(packet_t *packet, uint8_t commandCode);
bool Packet_InsertParamUint32(packet_t *packet, uint32_t Data);
bool Packet_InsertParamString(packet_t *packet, const char *string, uint16_t size);
bool Packet_InsertParamBuffer(packet_t *packet, uint8_t *buffer, uint16_t size);
void Packet_InsertCrc(packet_t *packet);

#ifdef __cplusplus
}
#endif
