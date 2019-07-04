#include "packet_parser.h"
#include <stdlib.h>     //for strtoul()
#include <string.h>
#include <ctype.h>      //for isxdigit()
#include <stdio.h>

#include "strtouint8.h" //для быстрой конвертации строки hex в uint8_t
#include "crc8.h"

#include <assert.h>

#define PARAMETER_NUMBER_BASE16     (16u)
#define MIN_PACKET_SIZE             (4u)
#define COMMAND_SIZE                (2u)
#define CRC_SIZE                    (2u)
#define UINT32_PARAM_SIZE           (8u)

#define PARAM_SEPARATOR             (0x1F)      // Байт разделителя параметров

static bool checkPacketLenght(packet_t *packet);
static void convetAsciiToBinBuffer(uint8_t *buffer, uint16_t bufferSize);
static parserError_t getParam(packet_t *packet, uint16_t *bufferIndex, uint16_t *paramSize);
parserError_t stringToLong(uint8_t *buffer, uint16_t bufferSize, uint32_t *ptrValue);
static bool stringIsHexNumber(uint8_t *str, size_t size);

void Packet_Init(packet_t *packet, uint8_t *workBuffer, uint16_t sizeOfBuffer)
{
    assert(packet != NULL);
    assert(workBuffer != NULL);
    assert(sizeOfBuffer != 0);

    packet->ptrBuffer = workBuffer;
    packet->maxNumOfElements = sizeOfBuffer;

    Packet_Clear(packet);
}

uint8_t *Packet_GetBufferPtr(packet_t *packet)
{
    assert(packet != NULL);

    return packet->ptrBuffer;
}

uint16_t Packet_GetMaxNumOfElements(packet_t *packet)
{
    assert(packet != NULL);

    return packet->maxNumOfElements;
}

bool Packet_InsertByte(packet_t *packet, uint8_t byte)
{
    assert(packet != NULL);

    if (packet->numOfElements < packet->maxNumOfElements)
    {
        packet->ptrBuffer[packet->numOfElements] = byte;
        packet->numOfElements++;
        return true;
    }
    return false;
}

uint16_t Packet_GetNumOfElements(packet_t *packet)
{
    assert(packet != NULL);

    return packet->numOfElements;
}

void Packet_SetNumOfElements(packet_t *packet, uint16_t numOfElements)
{
    assert(numOfElements <= packet->maxNumOfElements);

    packet->numOfElements = numOfElements;
}

void Packet_Clear(packet_t *packet)
{
    assert(packet != NULL);

    memset(packet->ptrBuffer, 0, packet->maxNumOfElements);
    packet->numOfElements = 0;
    packet->searchParamIndex = COMMAND_SIZE;
}

bool Packet_CheckCrc(packet_t *packet)
{
    if (checkPacketLenght(packet) == false)
    {
        return false;
    }

    uint32_t receivedCrc = 0;
    parserError_t result = stringToLong(&packet->ptrBuffer[packet->numOfElements - CRC_SIZE], CRC_SIZE, &receivedCrc);
    if (result != PARSER_ERROR_OK)
    {
        return false;
    }

    uint8_t calculatedCrc = Crc8(packet->ptrBuffer, packet->numOfElements - CRC_SIZE);

    if (receivedCrc != calculatedCrc)
    {
        return false;
    }
    return true;
}

bool Packet_GetCommandCode(packet_t *packet, uint8_t *ptrCommand)
{
    assert(packet != NULL);
    assert(ptrCommand != NULL);

    if (checkPacketLenght(packet) == false)
    {
        return false;
    }
    uint32_t command = 0;
    parserError_t result = stringToLong(packet->ptrBuffer, COMMAND_SIZE, &command);
    if (result != PARSER_ERROR_OK)
    {
        return false;
    }

    *ptrCommand = command;
    return true;
}

parserError_t Packet_GetParamUint32(packet_t *packet, uint32_t *ptrData)
{
    assert(packet != NULL);
    assert(ptrData != NULL);

    uint16_t index = 0;
    uint16_t size = 0;

    parserError_t result = getParam(packet, &index, &size);
    if (result != PARSER_ERROR_OK)
    {
        return result;
    }

    result = stringToLong(&packet->ptrBuffer[index], size, ptrData);
    return result;
}

parserError_t Packet_GetParamString(packet_t *packet, uint8_t **ptrPtrString, uint16_t *ptrSize)
{
    assert(packet != NULL);
    assert(ptrPtrString != NULL);
    assert(ptrSize != NULL);

    uint16_t index = 0;
    uint16_t size = 0;

    parserError_t result = getParam(packet, &index, &size);
    if (result != PARSER_ERROR_OK)
    {
        return result;
    }

    *ptrPtrString = &packet->ptrBuffer[index];
    *ptrSize = size;
    return PARSER_ERROR_OK;
}

parserError_t Packet_GetParamBuffer(packet_t *packet, uint8_t **ptrPtrBuffer, uint16_t *ptrSize)
{
    assert(packet != NULL);
    assert(ptrPtrBuffer != NULL);
    assert(ptrSize != NULL);

    uint16_t index = 0;
    uint16_t size = 0;

    parserError_t result = getParam(packet, &index, &size);
    if (result != PARSER_ERROR_OK)
    {
        return result;
    }
    // Длина массива должна быть четная
    if ((size % 2) != 0)
    {
        return PARSER_ERROR_WRONG_PARAMETER;
    }

    *ptrPtrBuffer = &packet->ptrBuffer[index];
    convetAsciiToBinBuffer(*ptrPtrBuffer, size);
    *ptrSize = size / 2;
    return PARSER_ERROR_OK;
}

static void convetAsciiToBinBuffer(uint8_t *buffer, uint16_t bufferSize)
{
    uint16_t resultBufferSize = bufferSize / 2;

    for (int i = 0; i < resultBufferSize; ++i) {
        uint32_t convertedByte = 0;
        stringToLong(&buffer[i * 2], 2, &convertedByte);
        buffer[i] = (uint8_t)convertedByte;
    }
}

static parserError_t getParam(packet_t *packet, uint16_t *bufferIndex, uint16_t *paramSize)
{
    assert(packet != NULL);
    assert(bufferIndex != NULL);
    assert(paramSize != NULL);

    uint16_t startIndex = packet->searchParamIndex;
    uint16_t endIndex = startIndex;
    uint16_t maxIndex = packet->numOfElements - CRC_SIZE;
    bool endBufferReached = false;

    while(1)
    {
        uint8_t byte = packet->ptrBuffer[endIndex];
        if (byte == PARAM_SEPARATOR)
        {
            break;
        }
        if (endIndex >= maxIndex)
        {
            endBufferReached = true;
            break;
        }
        endIndex++;
    }
    *bufferIndex = startIndex;
    *paramSize = endIndex - startIndex;
    packet->searchParamIndex = endIndex + 1;

    if (*paramSize != 0)
    {
        return PARSER_ERROR_OK;
    }
    else if (endBufferReached)
    {
        return PARSER_ERROR_OUT_OF_PARAMETERS;
    }
    else
    {
        return PARSER_ERROR_EMPTY_PARAMETER;
    }
}

static bool checkPacketLenght(packet_t *packet)
{
    assert(packet != NULL);

    if (packet->numOfElements < MIN_PACKET_SIZE)
    {
        return false;
    }
    return true;
}

parserError_t stringToLong(uint8_t *buffer, uint16_t bufferSize, uint32_t *ptrValue)
{
    if (bufferSize > UINT32_PARAM_SIZE)
    {
        return PARSER_ERROR_WRONG_PARAMETER;
    }
    if (stringIsHexNumber(buffer, bufferSize) != true)
    {
        return PARSER_ERROR_WRONG_PARAMETER;
    }

    // Если размер строки 2 символа, то используем быструю функцию преобразования строки в число
    if (bufferSize == 2)
    {
        *ptrValue = strHexToUint8(buffer);
    }
    else    // Иначе используем стандартную
    {
        uint8_t buf[UINT32_PARAM_SIZE + 1] = {0};
        strncpy((char *)buf, (const char *)buffer, bufferSize);
        *ptrValue = (uint32_t)strtoul((const char *)buf, NULL, PARAMETER_NUMBER_BASE16);
    }
    return PARSER_ERROR_OK;
}

static bool stringIsHexNumber(uint8_t *str, size_t size)
{
    assert(str != NULL);

    for (size_t i = 0; i < size; ++i)
    {
        if (isxdigit(str[i]) == 0)
        {
            return false;
        }
    }
    return true;
}

void Packet_InsertCommandCode(packet_t *packet, uint8_t commandCode)
{
    uint8_t *buffer = packet->ptrBuffer;
    sprintf((char*)buffer, "%02X", commandCode);
    packet->numOfElements = COMMAND_SIZE;
}

bool Packet_InsertParamUint32(packet_t *packet, uint32_t data)
{
    uint8_t buffer[9];

    sprintf((char*)buffer, "%X", data);
    size_t bufferLength = strlen((char*)buffer);

    if ((packet->numOfElements + bufferLength + 1) > (packet->maxNumOfElements - CRC_SIZE))
    {
        return false;
    }

    memcpy(&packet->ptrBuffer[packet->numOfElements], buffer, bufferLength);
    packet->numOfElements += bufferLength;

    packet->ptrBuffer[packet->numOfElements] = PARAM_SEPARATOR;
    packet->numOfElements++;

    return true;
}

bool Packet_InsertParamString(packet_t *packet, const char *string, uint16_t size)
{
    if ((packet->numOfElements + size + 1) > (packet->maxNumOfElements - CRC_SIZE))
    {
        return false;
    }
    memcpy(&packet->ptrBuffer[packet->numOfElements], string, size);
    packet->numOfElements += size;

    packet->ptrBuffer[packet->numOfElements] = PARAM_SEPARATOR;
    packet->numOfElements++;

    return true;
}

bool Packet_InsertParamBuffer(packet_t *packet, uint8_t *buffer, uint16_t size)
{
    if ((packet->numOfElements + size * 2 + 1) > (packet->maxNumOfElements - CRC_SIZE))
    {
        return false;
    }
    for (int i = 0; i < size; ++i)
    {
        uint8_t *ptrDestination = &packet->ptrBuffer[packet->numOfElements];
        sprintf((char*)ptrDestination, "%02X", buffer[i]);
        packet->numOfElements += 2;
    }

    packet->ptrBuffer[packet->numOfElements] = PARAM_SEPARATOR;
    packet->numOfElements++;

    return true;
}

void Packet_InsertCrc(packet_t *packet)
{
    assert(packet->numOfElements + CRC_SIZE <= packet->maxNumOfElements);

    // Если есть лишний сепаратор параметров - удаляем его
    if (packet->numOfElements > COMMAND_SIZE)
    {
        if (packet->ptrBuffer[packet->numOfElements - 1] == PARAM_SEPARATOR)
        {
            packet->numOfElements--;
        }
    }

    uint8_t crc = Crc8(packet->ptrBuffer, packet->numOfElements);
    uint8_t crcString[CRC_SIZE + 1];

    sprintf((char*)crcString, "%02X", crc);
    memcpy(&packet->ptrBuffer[packet->numOfElements], crcString, CRC_SIZE);
    packet->numOfElements += CRC_SIZE;
}
