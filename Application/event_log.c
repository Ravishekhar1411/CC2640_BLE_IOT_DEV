#include <stdio.h>
#include <string.h>

#include <xdc/std.h>
#include <uartlog/UartLog.h>

#include "event_log.h"

#define EVENT_LOG_CRC_DATA_LENGTH              9
#define EVENT_LOG_CRC_INITIAL_VALUE        0xFFFF
#define EVENT_LOG_CRC_POLYNOMIAL           0x1021
#define EVENT_LOG_PRINT_LINE_LENGTH            64

event_log_record_t gEventLog[EVENT_LOG_CAPACITY];
uint8_t gEventLogWriteIndex = 0;
uint8_t gEventLogCount = 0;

static char eventLogPrintLines[EVENT_LOG_CAPACITY]
                              [EVENT_LOG_PRINT_LINE_LENGTH];

static uint16_t EventLog_calculateCrc(const uint8_t *pData, uint8_t length)
{
    uint16_t crc = EVENT_LOG_CRC_INITIAL_VALUE;
    uint8_t byteIndex;

    for (byteIndex = 0; byteIndex < length; byteIndex++)
    {
        uint8_t bitIndex;

        crc ^= ((uint16_t) pData[byteIndex] << 8);
        for (bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            crc = (crc & 0x8000) ?
                  (uint16_t) ((crc << 1) ^ EVENT_LOG_CRC_POLYNOMIAL) :
                  (uint16_t) (crc << 1);
        }
    }

    return crc;
}

void EventLog_store(app_msg_types_t type, const char_data_t *pCharData)
{
    event_log_record_t *pRecord = &gEventLog[gEventLogWriteIndex];
    uint16_t crc;

    memset(pRecord, 0, sizeof(*pRecord));
    pRecord->serviceType = (uint8_t) type;
    pRecord->serviceConfig[0] = (uint8_t) pCharData->svcUUID;
    pRecord->serviceConfig[1] = (uint8_t) (pCharData->svcUUID >> 8);
    pRecord->paramID = pCharData->paramID;
    pRecord->dataLength[0] = (uint8_t) pCharData->dataLen;
    pRecord->dataLength[1] = (uint8_t) (pCharData->dataLen >> 8);

    crc = EventLog_calculateCrc((const uint8_t *) pRecord,
                                EVENT_LOG_CRC_DATA_LENGTH);
    pRecord->crc[0] = (uint8_t) crc;
    pRecord->crc[1] = (uint8_t) (crc >> 8);

    gEventLogWriteIndex = (uint8_t) ((gEventLogWriteIndex + 1) %
                                     EVENT_LOG_CAPACITY);
    if (gEventLogCount < EVENT_LOG_CAPACITY)
    {
        gEventLogCount++;
    }
}

void EventLog_printAll(void)
{
    uint8_t startIndex;
    uint8_t eventIndex;

    startIndex = (gEventLogCount == EVENT_LOG_CAPACITY) ?
                 gEventLogWriteIndex : 0;

    Log_info0("EVENT_LOG_BEGIN");
    Log_info0("INDEX,SPARE,SPARE2,TYPE,SERVICE_UUID,PARAM_ID,DATA_LEN,CRC");

    for (eventIndex = 0; eventIndex < gEventLogCount; eventIndex++)
    {
        uint8_t recordIndex = (uint8_t) ((startIndex + eventIndex) %
                                         EVENT_LOG_CAPACITY);
        const event_log_record_t *pRecord = &gEventLog[recordIndex];

 /*       snprintf(eventLogPrintLines[eventIndex],
                 EVENT_LOG_PRINT_LINE_LENGTH,
                 "%u,%02X%02X,%02X,%02X,%02X%02X,%02X,%02X%02X,%02X%02X",
                 eventIndex,
                 pRecord->spare[1],
                 pRecord->spare[0],
                 pRecord->spare2,
                 pRecord->serviceType,
                 pRecord->serviceConfig[1],
                 pRecord->serviceConfig[0],
                 pRecord->paramID,
                 pRecord->dataLength[1],
                 pRecord->dataLength[0],
                 pRecord->crc[1],
                 pRecord->crc[0]);

        Log_info3("Event Log: %u %X %X",
                  eventIndex,
                  pRecord->spare[1],
                  pRecord->spare[0]);

        Log_info3("%X %X %X",
                  pRecord->spare2,
                  pRecord->serviceType,
                  pRecord->serviceConfig[1]);

        Log_info3("%X %X %X",
                  pRecord->serviceConfig[0],
                  pRecord->paramID,
                  pRecord->dataLength[1]);

        Log_info2("%X %X",
                  pRecord->dataLength[0],
                  pRecord->crc[1]);

        Log_info1("%X",
                  pRecord->crc[0]);
        Log_info1("%s", (IArg) eventLogPrintLines[eventIndex]);
 */

        Log_info4("Event Log : %d|(0x%02x)(0x%02x)|(0x%02x)|",
                  (IArg)eventIndex,
                  (IArg)pRecord->spare[1],
                  (IArg)pRecord->spare[0],
                  (IArg)pRecord->spare2);

        Log_info4("|(0x%02x)|(0x%02x)(0x%02x)|(0x%02x)",
                  (IArg)pRecord->serviceType,
                  (IArg)pRecord->serviceConfig[1],
                  (IArg)pRecord->serviceConfig[0],
                  (IArg)pRecord->paramID);

        Log_info4("|(0x%02x)(0x%02x)|(0x%02x)(0x%02x)",
                  (IArg)pRecord->dataLength[1],
                  (IArg)pRecord->dataLength[0],
                  (IArg)pRecord->crc[1],
                  (IArg)pRecord->crc[0]);
    }

    Log_info0("EVENT_LOG_END");
}
