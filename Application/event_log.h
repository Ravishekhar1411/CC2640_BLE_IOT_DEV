#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "common.h"

#define EVENT_LOG_CAPACITY                    16
#define EVENT_LOG_RECORD_SIZE                 11

/*
 * Event record byte layout:
 * [0..1] spare, [2] spare2, [3] service type, [4..5] service UUID,
 * [6] parameter ID, [7..8] data length, [9..10] CRC-16/CCITT-FALSE.
 * Multi-byte values are stored least-significant byte first.
 */
typedef struct
{
    uint8_t spare[2];
    uint8_t spare2;
    uint8_t serviceType;
    uint8_t serviceConfig[2];
    uint8_t paramID;
    uint8_t dataLength[2];
    uint8_t crc[2];
} event_log_record_t;

extern event_log_record_t gEventLog[EVENT_LOG_CAPACITY];
extern uint8_t gEventLogWriteIndex;
extern uint8_t gEventLogCount;

void EventLog_store(app_msg_types_t type, const char_data_t *pCharData);
void EventLog_printAll(void);

#ifdef __cplusplus
}
#endif

#endif /* EVENT_LOG_H */
