#include "CppUTest/TestHarness.h"

#include "packet_parser.h"

#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 20

packet_t packet;
static uint8_t buffer[BUFFER_SIZE];

TEST_GROUP(PacketParserTestGroup)
{
    void setup()
    {
        Packet_Init(&packet, buffer, BUFFER_SIZE);
    }
};

TEST(PacketParserTestGroup, CheckInitStructureTest)
{
    POINTERS_EQUAL(buffer, packet.ptrBuffer);

    for (int i = 0; i < BUFFER_SIZE; ++i)
    {
        BYTES_EQUAL(0, packet.ptrBuffer[i]);
    }

    CHECK_EQUAL(BUFFER_SIZE, packet.maxNumOfElements);
    CHECK_EQUAL(2, packet.searchParamIndex);
    CHECK_EQUAL(0, packet.numOfElements);
}

TEST(PacketParserTestGroup, CheckGetBufferPtrTest)
{
    POINTERS_EQUAL(buffer, Packet_GetBufferPtr(&packet));
}

TEST(PacketParserTestGroup, CheckGetMaxNumOfElementsTest)
{
    CHECK_EQUAL(BUFFER_SIZE, Packet_GetMaxNumOfElements(&packet));
}

TEST(PacketParserTestGroup, ClearTest)
{
    packet.numOfElements = 6;
    packet.searchParamIndex = 7;

    Packet_Clear(&packet);

    CHECK_EQUAL(2, packet.searchParamIndex);
    CHECK_EQUAL(0, packet.numOfElements);
}

TEST(PacketParserTestGroup, InsertByteTest)
{
    CHECK(Packet_InsertByte(&packet, 0xA));
    CHECK_EQUAL(0xA, buffer[0]);
    CHECK(Packet_InsertByte(&packet, 0xB));
    CHECK_EQUAL(0xA, buffer[0]);
    CHECK_EQUAL(0xB, buffer[1]);

    for (int i = 0; i < BUFFER_SIZE - 2; ++i)
    {
        CHECK(Packet_InsertByte(&packet, i));
    }

    CHECK_FALSE(Packet_InsertByte(&packet, 0xF));
    CHECK_EQUAL(BUFFER_SIZE, Packet_GetNumOfElements(&packet));
}

TEST(PacketParserTestGroup, GetNumOfElementsTest)
{
    for (int i = 0; i < BUFFER_SIZE; ++i)
    {
        CHECK(Packet_InsertByte(&packet, i));
        CHECK_EQUAL(i + 1, Packet_GetNumOfElements(&packet));
    }

    Packet_Clear(&packet);
    CHECK_EQUAL(0, Packet_GetNumOfElements(&packet));
}

TEST(PacketParserTestGroup, SetNumOfElementsTest)
{
    CHECK_EQUAL(0, Packet_GetNumOfElements(&packet));

    Packet_SetNumOfElements(&packet, 10);
    CHECK_EQUAL(10, Packet_GetNumOfElements(&packet));
}

TEST(PacketParserTestGroup, GetCommandCodeInvalidBufferSizeTest)
{
    CHECK(Packet_InsertByte(&packet, 0));
    CHECK(Packet_InsertByte(&packet, 0));
    CHECK(Packet_InsertByte(&packet, 0));

    uint8_t command = 0xFF;
    CHECK_FALSE(Packet_GetCommandCode(&packet, &command));
}

TEST(PacketParserTestGroup, GetCommandCodeInvalidCodeTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'R'));
    CHECK(Packet_InsertByte(&packet, 0));
    CHECK(Packet_InsertByte(&packet, 0));
    CHECK(Packet_InsertByte(&packet, 0));
    CHECK(Packet_InsertByte(&packet, 0));
    CHECK(Packet_InsertByte(&packet, 0));
    CHECK(Packet_InsertByte(&packet, 0));
    CHECK(Packet_InsertByte(&packet, 0));

    uint8_t command = 0xFF;
    CHECK_FALSE(Packet_GetCommandCode(&packet, &command));
}

TEST(PacketParserTestGroup, CheckCrcInvalidParamTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '1'));

    CHECK_FALSE(Packet_CheckCrc(&packet));
}

TEST(PacketParserTestGroup, CheckCrcInvalidCrcTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, '4'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'R'));

    CHECK_FALSE(Packet_CheckCrc(&packet));
}

TEST(PacketParserTestGroup, CheckCrcTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, '4'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'B'));

    CHECK(Packet_CheckCrc(&packet));
}

TEST(PacketParserTestGroup, GetCommandTest)
{
    uint8_t command = 0xFF;

    Packet_Clear(&packet);
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, '4'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    command = 0xFF;
    CHECK(Packet_GetCommandCode(&packet, &command));
    CHECK_EQUAL(0, command);

    Packet_Clear(&packet);
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, '4'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    command = 0xFF;
    CHECK(Packet_GetCommandCode(&packet, &command));
    CHECK_EQUAL(1, command);

    Packet_Clear(&packet);
    CHECK(Packet_InsertByte(&packet, 'F'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, '4'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    command = 0xFF;
    CHECK(Packet_GetCommandCode(&packet, &command));
    CHECK_EQUAL(0xF1, command);

    Packet_Clear(&packet);
    CHECK(Packet_InsertByte(&packet, 'F'));
    CHECK(Packet_InsertByte(&packet, 'F'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, '4'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    command = 0xFF;
    CHECK(Packet_GetCommandCode(&packet, &command));
    CHECK_EQUAL(0xFF, command);
}

TEST(PacketParserTestGroup, GetParamUint32Test)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, 'C'));

    CHECK(Packet_InsertByte(&packet, 0x1F));

    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, 'F'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, 'E'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, 'C'));
    CHECK(Packet_InsertByte(&packet, 'D'));
    CHECK(Packet_InsertByte(&packet, '9'));

    CHECK(Packet_InsertByte(&packet, 0x1F));

    CHECK(Packet_InsertByte(&packet, 'A'));

    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));

    uint32_t value = 0;

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamUint32(&packet, &value));
    CHECK_EQUAL(0x1b1c, value);

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamUint32(&packet, &value));
    CHECK_EQUAL(0x1f2e3cd9, value);

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamUint32(&packet, &value));
    CHECK_EQUAL(0xA, value);

    CHECK_EQUAL(PARSER_ERROR_OUT_OF_PARAMETERS, Packet_GetParamUint32(&packet, &value));
}

TEST(PacketParserTestGroup, GetParamUint32EmptyParamTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, 'F'));
    CHECK(Packet_InsertByte(&packet, 0x1F));
    CHECK(Packet_InsertByte(&packet, 0x1F));
    CHECK(Packet_InsertByte(&packet, 'C'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, 'F'));
    CHECK(Packet_InsertByte(&packet, 0x1F));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));

    uint32_t value = 0;

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamUint32(&packet, &value));
    CHECK_EQUAL(0xF, value);

    CHECK_EQUAL(PARSER_ERROR_EMPTY_PARAMETER, Packet_GetParamUint32(&packet, &value));

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamUint32(&packet, &value));
    CHECK_EQUAL(0xc11f, value);
}

TEST(PacketParserTestGroup, GetParamUint32InvalidNumberTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));

    CHECK(Packet_InsertByte(&packet, 'A'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    CHECK(Packet_InsertByte(&packet, 'r'));

    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));

    uint32_t value = 0;
    CHECK_EQUAL(PARSER_ERROR_WRONG_PARAMETER, Packet_GetParamUint32(&packet, &value));
}

TEST(PacketParserTestGroup, GetParamUint32TooLongNumberTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));

    CHECK(Packet_InsertByte(&packet, 'A'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    CHECK(Packet_InsertByte(&packet, 'A'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    CHECK(Packet_InsertByte(&packet, 'A'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    CHECK(Packet_InsertByte(&packet, 'A'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    CHECK(Packet_InsertByte(&packet, '1'));

    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));

    uint32_t value = 0;
    CHECK_EQUAL(PARSER_ERROR_WRONG_PARAMETER, Packet_GetParamUint32(&packet, &value));
}

TEST(PacketParserTestGroup, GetParamStringTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));

    CHECK(Packet_InsertByte(&packet, 'D'));
    CHECK(Packet_InsertByte(&packet, 'R'));
    CHECK(Packet_InsertByte(&packet, 'i'));
    CHECK(Packet_InsertByte(&packet, 'm'));
    CHECK(Packet_InsertByte(&packet, 0x1F));

    CHECK(Packet_InsertByte(&packet, 'H'));
    CHECK(Packet_InsertByte(&packet, 'e'));
    CHECK(Packet_InsertByte(&packet, 'l'));
    CHECK(Packet_InsertByte(&packet, 'l'));
    CHECK(Packet_InsertByte(&packet, 'o'));
    CHECK(Packet_InsertByte(&packet, ' '));
    CHECK(Packet_InsertByte(&packet, 'w'));
    CHECK(Packet_InsertByte(&packet, 'o'));
    CHECK(Packet_InsertByte(&packet, 0x1F));

    CHECK(Packet_InsertByte(&packet, 'A'));

    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));

    uint8_t *str;
    uint16_t size;

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamString(&packet, &str, &size));
    CHECK(strncmp("DRim", (const char *)str, size) == 0);

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamString(&packet, &str, &size));
    CHECK(strncmp("Hello wo", (const char *)str, size) == 0);

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamString(&packet, &str, &size));
    CHECK(strncmp("A", (const char *)str, size) == 0);

    CHECK_EQUAL(PARSER_ERROR_OUT_OF_PARAMETERS, Packet_GetParamString(&packet, &str, &size));
}

TEST(PacketParserTestGroup, GetParamStringEmptyParamTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));

    CHECK(Packet_InsertByte(&packet, 'S'));
    CHECK(Packet_InsertByte(&packet, 0x1F));
    CHECK(Packet_InsertByte(&packet, 0x1F));
    CHECK(Packet_InsertByte(&packet, 'D'));
    CHECK(Packet_InsertByte(&packet, 'R'));
    CHECK(Packet_InsertByte(&packet, 'i'));
    CHECK(Packet_InsertByte(&packet, 'm'));

    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));

    uint8_t *buffer;
    uint16_t size;

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamString(&packet, &buffer, &size));
    CHECK(strncmp((const char *)"S", (const char *)buffer, size) == 0);

    CHECK_EQUAL(PARSER_ERROR_EMPTY_PARAMETER, Packet_GetParamString(&packet, &buffer, &size));

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamString(&packet, &buffer, &size));
    CHECK(strncmp((const char *)"DRim", (const char *)buffer, size) == 0);
}

TEST(PacketParserTestGroup, GetParamBufferTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));

    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, 'A'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'B'));
    CHECK(Packet_InsertByte(&packet, '0'));

    CHECK(Packet_InsertByte(&packet, 0x1F));

    CHECK(Packet_InsertByte(&packet, 'F'));
    CHECK(Packet_InsertByte(&packet, 'E'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'A'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));

    uint8_t *resultBuffer;
    uint16_t size;
    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamBuffer(&packet, &resultBuffer, &size));
    CHECK_EQUAL(5, size);
    uint8_t buffer1[20] = {1, 2, 3, 0xA0, 0xB0};
    MEMCMP_EQUAL(buffer1, resultBuffer, 5);

    CHECK_EQUAL(PARSER_ERROR_OK, Packet_GetParamBuffer(&packet, &resultBuffer, &size));
    CHECK_EQUAL(2, size);
    uint8_t buffer2[20] = {0xFE, 0x0A};
    MEMCMP_EQUAL(buffer2, resultBuffer, 2);

    CHECK_EQUAL(PARSER_ERROR_OUT_OF_PARAMETERS, Packet_GetParamBuffer(&packet, &resultBuffer, &size));
}

TEST(PacketParserTestGroup, GetParamBufferNotOddNumOfCharsTest)
{
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));

    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '1'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '2'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '3'));
    CHECK(Packet_InsertByte(&packet, 'A'));
    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, 'B'));

    CHECK(Packet_InsertByte(&packet, '0'));
    CHECK(Packet_InsertByte(&packet, '0'));

    uint8_t *resultBuffer;
    uint16_t size;
    CHECK_EQUAL(PARSER_ERROR_WRONG_PARAMETER, Packet_GetParamBuffer(&packet, &resultBuffer, &size));
}

TEST(PacketParserTestGroup, InsertCommandCodeTest)
{
    Packet_InsertCommandCode(&packet, 0);
    CHECK_EQUAL(2, packet.numOfElements);
    CHECK_EQUAL('0', buffer[0]);
    CHECK_EQUAL('0', buffer[1]);

    Packet_Clear(&packet);
    Packet_InsertCommandCode(&packet, 1);
    CHECK_EQUAL(2, packet.numOfElements);
    CHECK_EQUAL('0', buffer[0]);
    CHECK_EQUAL('1', buffer[1]);

    Packet_Clear(&packet);
    Packet_InsertCommandCode(&packet, 0xf);
    CHECK_EQUAL(2, packet.numOfElements);
    CHECK_EQUAL('0', buffer[0]);
    CHECK_EQUAL('F', buffer[1]);

    Packet_Clear(&packet);
    Packet_InsertCommandCode(&packet, 0xe0);
    CHECK_EQUAL(2, packet.numOfElements);
    CHECK_EQUAL('E', buffer[0]);
    CHECK_EQUAL('0', buffer[1]);

    Packet_Clear(&packet);
    Packet_InsertCommandCode(&packet, 0xff);
    CHECK_EQUAL(2, packet.numOfElements);
    CHECK_EQUAL('F', buffer[0]);
    CHECK_EQUAL('F', buffer[1]);
}

TEST(PacketParserTestGroup, InsertParamUint32Test)
{
    Packet_InsertCommandCode(&packet, 2);
    CHECK(Packet_InsertParamUint32(&packet, 0xFA));
    CHECK(Packet_InsertParamUint32(&packet, 0x1234));
    CHECK(Packet_InsertParamUint32(&packet, 0x5678));
    CHECK_FALSE(Packet_InsertParamUint32(&packet, 0xABCD));

    CHECK_EQUAL(15, packet.numOfElements);

    CHECK_EQUAL('0', buffer[0]);
    CHECK_EQUAL('2', buffer[1]);

    CHECK_EQUAL('F', buffer[2]);
    CHECK_EQUAL('A', buffer[3]);
    CHECK_EQUAL(0x1F, buffer[4]);

    CHECK_EQUAL('1', buffer[5]);
    CHECK_EQUAL('2', buffer[6]);
    CHECK_EQUAL('3', buffer[7]);
    CHECK_EQUAL('4', buffer[8]);
    CHECK_EQUAL(0x1F, buffer[9]);

    CHECK_EQUAL('5', buffer[10]);
    CHECK_EQUAL('6', buffer[11]);
    CHECK_EQUAL('7', buffer[12]);
    CHECK_EQUAL('8', buffer[13]);
    CHECK_EQUAL(0x1F, buffer[14]);
}

TEST(PacketParserTestGroup, InsertParamStringTest)
{
    char string1[20] = "HELLO WORLD";
    char string2[20] = "BY";

    Packet_InsertCommandCode(&packet, 3);
    CHECK(Packet_InsertParamString(&packet, string1, 11));
    CHECK(Packet_InsertParamString(&packet, string2, 2));
    CHECK_FALSE(Packet_InsertParamString(&packet, string2, 2));

    CHECK_EQUAL(17, packet.numOfElements);

    CHECK_EQUAL('0', buffer[0]);
    CHECK_EQUAL('3', buffer[1]);

    CHECK_EQUAL('H', buffer[2]);
    CHECK_EQUAL('E', buffer[3]);
    CHECK_EQUAL('L', buffer[4]);
    CHECK_EQUAL('L', buffer[5]);
    CHECK_EQUAL('O', buffer[6]);
    CHECK_EQUAL(' ', buffer[7]);
    CHECK_EQUAL('W', buffer[8]);
    CHECK_EQUAL('O', buffer[9]);
    CHECK_EQUAL('R', buffer[10]);
    CHECK_EQUAL('L', buffer[11]);
    CHECK_EQUAL('D', buffer[12]);
    CHECK_EQUAL(0x1F, buffer[13]);

    CHECK_EQUAL('B', buffer[14]);
    CHECK_EQUAL('Y', buffer[15]);
    CHECK_EQUAL(0x1F, buffer[16]);
}

TEST(PacketParserTestGroup, InsertParamBufferTest)
{
    uint8_t buffer1[20] = {1, 2, 3, 0xf0, 0xee};
    uint8_t buffer2[20] = {0xa, 0xb};

    Packet_InsertCommandCode(&packet, 0x20);
    CHECK(Packet_InsertParamBuffer(&packet, buffer1, 5));
    CHECK(Packet_InsertParamBuffer(&packet, buffer2, 2));
    CHECK_FALSE(Packet_InsertParamBuffer(&packet, buffer2, 2));

    CHECK_EQUAL(18, packet.numOfElements);

    CHECK_EQUAL('2', buffer[0]);
    CHECK_EQUAL('0', buffer[1]);

    CHECK_EQUAL('0', buffer[2]);
    CHECK_EQUAL('1', buffer[3]);
    CHECK_EQUAL('0', buffer[4]);
    CHECK_EQUAL('2', buffer[5]);
    CHECK_EQUAL('0', buffer[6]);
    CHECK_EQUAL('3', buffer[7]);
    CHECK_EQUAL('F', buffer[8]);
    CHECK_EQUAL('0', buffer[9]);
    CHECK_EQUAL('E', buffer[10]);
    CHECK_EQUAL('E', buffer[11]);
    CHECK_EQUAL(0x1F, buffer[12]);

    CHECK_EQUAL('0', buffer[13]);
    CHECK_EQUAL('A', buffer[14]);
    CHECK_EQUAL('0', buffer[15]);
    CHECK_EQUAL('B', buffer[16]);
    CHECK_EQUAL(0x1F, buffer[17]);
}

TEST(PacketParserTestGroup, CalculateCrcTest)
{
    char string1[20] = "HELLO WORLD";
    char string2[20] = "BY";

    Packet_InsertCommandCode(&packet, 3);
    CHECK(Packet_InsertParamString(&packet, string1, 11));
    CHECK(Packet_InsertParamString(&packet, string2, 2));
    Packet_InsertCrc(&packet);

    CHECK(Packet_CheckCrc(&packet));
}
