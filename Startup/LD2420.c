#include "LD2420.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

static const uint8_t ld2420_cmd_init[] = {
    0xFD, 0xFC, 0xFB, 0xFA, 0x08, 0x00, 0x12, 0x00,
    0x00, 0x00, 0x64, 0x00, 0x00, 0x00, 0x04, 0x03,
    0x02, 0x01
};

static const uint8_t ld2420_cmd_restart[] = {
    0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xA1, 0x00,
    0x00, 0x00, 0x04, 0x03, 0x02, 0x01
};

static const uint8_t ld2420_cmd_factory_reset[] = {
    0xFD, 0xFC, 0xFB, 0xFA, 0x04, 0x00, 0xA2, 0x00,
    0x00, 0x00, 0x04, 0x03, 0x02, 0x01
};

static LD2420 *active_dev;

static uint32_t now_ms(void)
{
    uint64_t usec = (uint64_t)Clock_getTicks() * (uint64_t)Clock_tickPeriod;
    return (uint32_t)(usec / 1000U);
}

static void sleep_ms(uint32_t ms)
{
    uint32_t ticks;

    if (ms == 0U) {
        return;
    }

    ticks = ((ms * 1000U) + Clock_tickPeriod - 1U) / Clock_tickPeriod;
    if (ticks == 0U) {
        ticks = 1U;
    }

    Task_sleep((UInt)ticks);
}

static void start_rx(LD2420 *dev)
{
    if (dev != NULL && dev->uart != NULL) {
        (void)UART_read(dev->uart, &dev->rx_byte, 1U);
    }
}

static void uart_read_callback(UART_Handle handle, void *buffer, size_t count)
{
    LD2420 *dev = active_dev;
    uint16_t next_head;

    (void)handle;
    (void)buffer;

    if (dev == NULL || dev->uart == NULL || count == 0U) {
        return;
    }

    next_head = (uint16_t)((dev->rx_head + 1U) % LD2420_RX_RING_SIZE);
    if (next_head == dev->rx_tail) {
        dev->rx_overflow = true;
    } else {
        dev->rx_ring[dev->rx_head] = dev->rx_byte;
        dev->rx_head = next_head;
    }

    start_rx(dev);
}

static bool pop_rx(LD2420 *dev, uint8_t *byte)
{
    if (dev == NULL || byte == NULL || dev->rx_tail == dev->rx_head) {
        return false;
    }

    *byte = dev->rx_ring[dev->rx_tail];
    dev->rx_tail = (uint16_t)((dev->rx_tail + 1U) % LD2420_RX_RING_SIZE);
    return true;
}

static void flush_rx(LD2420 *dev)
{
    if (dev == NULL) {
        return;
    }

    dev->rx_head = 0U;
    dev->rx_tail = 0U;
    dev->rx_overflow = false;
    dev->line_index = 0U;
}

static bool send_bytes(LD2420 *dev, const uint8_t *data, size_t len)
{
    if (dev == NULL || dev->uart == NULL || data == NULL || len == 0U) {
        return false;
    }

    return UART_write(dev->uart, data, len) == (int)len;
}

static void update_state(LD2420 *dev, LD2420_DetectionState new_state)
{
    LD2420_DetectionState old_state;

    if (dev == NULL) {
        return;
    }

    old_state = dev->last_state;
    dev->last_state = new_state;

    if (dev->on_state_change != NULL && old_state != new_state) {
        dev->on_state_change(old_state, new_state);
    }
}

static bool parse_line(LD2420 *dev, const char *line)
{
    int32_t distance;
    LD2420_DetectionState new_state;

    if (dev == NULL || line == NULL) {
        return false;
    }

    if (strncmp(line, "Range ", 6U) != 0) {
        return false;
    }

    distance = (int32_t)strtol(&line[6], NULL, 10);
    if (distance < dev->min_distance_cm || distance > dev->max_distance_cm) {
        return false;
    }

    new_state = (distance > 0) ? LD2420_DETECTION_ACTIVE : LD2420_NO_DETECTION;

    dev->current_data.distance_cm = distance;
    dev->current_data.timestamp_ms = now_ms();
    dev->current_data.is_valid = true;

    if (new_state != dev->current_data.state) {
        update_state(dev, new_state);
    }

    dev->current_data.state = new_state;

    if (dev->on_detection != NULL && new_state == LD2420_DETECTION_ACTIVE) {
        dev->on_detection(distance);
    }

    if (dev->on_data_update != NULL) {
        dev->on_data_update(&dev->current_data);
    }

    return true;
}

static void process_byte(LD2420 *dev, uint8_t byte)
{
    if (dev == NULL) {
        return;
    }

    if (byte == '\r') {
        return;
    }

    if (byte == '\n') {
        dev->line[dev->line_index] = '\0';
        if (dev->line_index > 0U) {
            (void)parse_line(dev, dev->line);
        }
        dev->line_index = 0U;
        return;
    }

    if (dev->line_index < (LD2420_BUFFER_SIZE - 1U)) {
        dev->line[dev->line_index++] = (char)byte;
    } else {
        dev->line_index = 0U;
    }
}

static void process_rx(LD2420 *dev)
{
    uint8_t byte;

    while (pop_rx(dev, &byte)) {
        process_byte(dev, byte);
    }
}

void LD2420_InitStruct(LD2420 *dev, uint_least8_t uart_index)
{
    if (dev == NULL) {
        return;
    }

    memset(dev, 0, sizeof(*dev));
    dev->uart_index = uart_index;
    dev->baud_rate = LD2420_DEFAULT_BAUD_RATE;
    dev->min_distance_cm = LD2420_MIN_DISTANCE_CM;
    dev->max_distance_cm = LD2420_MAX_DISTANCE_CM;
    dev->update_interval_ms = LD2420_DEFAULT_INTERVAL_MS;
    dev->current_data.state = LD2420_NO_DETECTION;
    dev->last_state = LD2420_NO_DETECTION;
}

bool LD2420_Begin(LD2420 *dev)
{
    return LD2420_BeginWithBaud(dev, LD2420_DEFAULT_BAUD_RATE);
}

bool LD2420_BeginWithBaud(LD2420 *dev, uint32_t baud_rate)
{
    UART_Params params;

    if (dev == NULL || active_dev != NULL) {
        return false;
    }

    UART_Params_init(&params);
    params.baudRate = baud_rate;
    params.readMode = UART_MODE_CALLBACK;
    params.writeMode = UART_MODE_BLOCKING;
    params.readDataMode = UART_DATA_BINARY;
    params.writeDataMode = UART_DATA_BINARY;
    params.readReturnMode = UART_RETURN_FULL;
    params.readEcho = UART_ECHO_OFF;
    params.readCallback = uart_read_callback;

    dev->uart = UART_open(dev->uart_index, &params);
    if (dev->uart == NULL) {
        return false;
    }

    active_dev = dev;
    dev->baud_rate = baud_rate;
    flush_rx(dev);
    start_rx(dev);

    sleep_ms(100U);
    if (!LD2420_SendInitCommand(dev)) {
        LD2420_End(dev);
        return false;
    }

    dev->initialized = true;
    dev->last_update_ms = now_ms();
    flush_rx(dev);
    return true;
}

void LD2420_End(LD2420 *dev)
{
    if (dev == NULL) {
        return;
    }

    if (dev->uart != NULL) {
        UART_readCancel(dev->uart);
        UART_close(dev->uart);
        dev->uart = NULL;
    }

    if (active_dev == dev) {
        active_dev = NULL;
    }

    dev->initialized = false;
    flush_rx(dev);
}

void LD2420_SetDistanceRange(LD2420 *dev, int32_t min_cm, int32_t max_cm)
{
    if (dev != NULL) {
        dev->min_distance_cm = min_cm;
        dev->max_distance_cm = max_cm;
    }
}

void LD2420_SetUpdateInterval(LD2420 *dev, uint32_t interval_ms)
{
    if (dev != NULL) {
        dev->update_interval_ms = interval_ms;
    }
}

bool LD2420_SendInitCommand(LD2420 *dev)
{
    if (!send_bytes(dev, ld2420_cmd_init, sizeof(ld2420_cmd_init))) {
        return false;
    }

    sleep_ms(100U);
    return true;
}

bool LD2420_Restart(LD2420 *dev)
{
    if (!send_bytes(dev, ld2420_cmd_restart, sizeof(ld2420_cmd_restart))) {
        return false;
    }

    sleep_ms(500U);
    return LD2420_SendInitCommand(dev);
}

bool LD2420_FactoryReset(LD2420 *dev)
{
    if (!send_bytes(dev, ld2420_cmd_factory_reset, sizeof(ld2420_cmd_factory_reset))) {
        return false;
    }

    sleep_ms(1000U);
    return LD2420_SendInitCommand(dev);
}

void LD2420_Update(LD2420 *dev)
{
    uint32_t current_ms;

    if (dev == NULL || !dev->initialized) {
        return;
    }

    current_ms = now_ms();
    if ((uint32_t)(current_ms - dev->last_update_ms) < dev->update_interval_ms) {
        return;
    }

    dev->last_update_ms = current_ms;
    process_rx(dev);
}

LD2420_Data LD2420_GetCurrentData(const LD2420 *dev)
{
    LD2420_Data empty_data;

    if (dev != NULL) {
        return dev->current_data;
    }

    memset(&empty_data, 0, sizeof(empty_data));
    empty_data.state = LD2420_NO_DETECTION;
    return empty_data;
}

int32_t LD2420_GetDistance(const LD2420 *dev)
{
    return (dev != NULL) ? dev->current_data.distance_cm : 0;
}

LD2420_DetectionState LD2420_GetState(const LD2420 *dev)
{
    return (dev != NULL) ? dev->current_data.state : LD2420_NO_DETECTION;
}

bool LD2420_IsDetecting(const LD2420 *dev)
{
    return LD2420_GetState(dev) == LD2420_DETECTION_ACTIVE;
}

bool LD2420_IsDataValid(const LD2420 *dev)
{
    return (dev != NULL) ? dev->current_data.is_valid : false;
}

uint32_t LD2420_GetLastUpdateTime(const LD2420 *dev)
{
    return (dev != NULL) ? dev->current_data.timestamp_ms : 0U;
}

bool LD2420_IsInitialized(const LD2420 *dev)
{
    return (dev != NULL) ? dev->initialized : false;
}

bool LD2420_HadRxOverflow(LD2420 *dev, bool clear_flag)
{
    bool overflow;

    if (dev == NULL) {
        return false;
    }

    overflow = dev->rx_overflow;
    if (clear_flag) {
        dev->rx_overflow = false;
    }

    return overflow;
}

void LD2420_OnDetection(LD2420 *dev, LD2420_DetectionCallback callback)
{
    if (dev != NULL) {
        dev->on_detection = callback;
    }
}

void LD2420_OnStateChange(LD2420 *dev, LD2420_StateChangeCallback callback)
{
    if (dev != NULL) {
        dev->on_state_change = callback;
    }
}

void LD2420_OnDataUpdate(LD2420 *dev, LD2420_DataCallback callback)
{
    if (dev != NULL) {
        dev->on_data_update = callback;
    }
}

const char *LD2420_GetVersionInfo(void)
{
    return "LD2420 CC2640 TI-Drivers UART Driver v1.0.0";
}
