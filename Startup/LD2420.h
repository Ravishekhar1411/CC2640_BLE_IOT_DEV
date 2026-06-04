#ifndef APPLICATION_LD2420_CC2640_H
#define APPLICATION_LD2420_CC2640_H

#include <stdbool.h>
#include <stdint.h>

#include <ti/drivers/UART.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LD2420_DEFAULT_BAUD_RATE     115200U
#define LD2420_BUFFER_SIZE           128U
#define LD2420_RX_RING_SIZE          256U
#define LD2420_MAX_DISTANCE_CM       600
#define LD2420_MIN_DISTANCE_CM       0
#define LD2420_DEFAULT_INTERVAL_MS   10U

typedef enum
{
    LD2420_NO_DETECTION = 0,
    LD2420_DETECTION_ACTIVE = 1,
    LD2420_DETECTION_LOST = 2
} LD2420_DetectionState;

typedef struct
{
    int32_t distance_cm;
    LD2420_DetectionState state;
    uint32_t timestamp_ms;
    bool is_valid;
} LD2420_Data;

typedef void (*LD2420_DetectionCallback)(int32_t distance_cm);
typedef void (*LD2420_StateChangeCallback)(LD2420_DetectionState old_state,
                                           LD2420_DetectionState new_state);
typedef void (*LD2420_DataCallback)(const LD2420_Data *data);

typedef struct
{
    uint_least8_t uart_index;
    uint32_t baud_rate;
    UART_Handle uart;

    volatile uint16_t rx_head;
    volatile uint16_t rx_tail;
    volatile bool rx_overflow;
    uint8_t rx_ring[LD2420_RX_RING_SIZE];
    uint8_t rx_byte;

    char line[LD2420_BUFFER_SIZE];
    uint16_t line_index;

    bool initialized;
    LD2420_Data current_data;
    LD2420_DetectionState last_state;

    int32_t min_distance_cm;
    int32_t max_distance_cm;
    uint32_t update_interval_ms;
    uint32_t last_update_ms;

    LD2420_DetectionCallback on_detection;
    LD2420_StateChangeCallback on_state_change;
    LD2420_DataCallback on_data_update;
} LD2420;

void LD2420_InitStruct(LD2420 *dev, uint_least8_t uart_index);
bool LD2420_Begin(LD2420 *dev);
bool LD2420_BeginWithBaud(LD2420 *dev, uint32_t baud_rate);
void LD2420_End(LD2420 *dev);

void LD2420_SetDistanceRange(LD2420 *dev, int32_t min_cm, int32_t max_cm);
void LD2420_SetUpdateInterval(LD2420 *dev, uint32_t interval_ms);

bool LD2420_SendInitCommand(LD2420 *dev);
bool LD2420_Restart(LD2420 *dev);
bool LD2420_FactoryReset(LD2420 *dev);

void LD2420_Update(LD2420 *dev);

LD2420_Data LD2420_GetCurrentData(const LD2420 *dev);
int32_t LD2420_GetDistance(const LD2420 *dev);
LD2420_DetectionState LD2420_GetState(const LD2420 *dev);
bool LD2420_IsDetecting(const LD2420 *dev);
bool LD2420_IsDataValid(const LD2420 *dev);
uint32_t LD2420_GetLastUpdateTime(const LD2420 *dev);
bool LD2420_IsInitialized(const LD2420 *dev);
bool LD2420_HadRxOverflow(LD2420 *dev, bool clear_flag);

void LD2420_OnDetection(LD2420 *dev, LD2420_DetectionCallback callback);
void LD2420_OnStateChange(LD2420 *dev, LD2420_StateChangeCallback callback);
void LD2420_OnDataUpdate(LD2420 *dev, LD2420_DataCallback callback);

const char *LD2420_GetVersionInfo(void);

#ifdef __cplusplus
}
#endif

#endif
