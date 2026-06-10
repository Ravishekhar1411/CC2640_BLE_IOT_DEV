/******************************************************************************
 * Filename:       Game_button_service.h
 *
 *
 *****************************************************************************/

#ifndef PROFILES_GAME_BUTTON_SERVICE_H_
#define PROFILES_GAME_BUTTON_SERVICE_H_


/*********************************************************************
 * INCLUDES
 */
#include <bcomdef.h>

#include "labs.h"
#include "common.h"

/*********************************************************************
 * TYPEDEFS
 */
//
// Memory layout of characteristic data
//
// LED String Service
// PROGRAM characteristic
typedef uint8_t program_char_t;

// OFFON characteristic
typedef uint8_t offon_char_t;

// RGB characteristic
typedef struct button_char {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} button_char_t;

/*********************************************************************
 * CONSTANTS
 */

//
// LAB_6 - Pairing and Bonding
// Add define for "pairing not started"

//
// Default values for characteristics
// TODO - Below ones would not be use
#define GAME_BUTTON_LSS_DEFAULT_OFFON           0x02       // On
#define GAME_BUTTON_LSS_DEFAULT_RED             0x07        // Low-level red
#define GAME_BUTTON_LSS_DEFAULT_GREEN           0x00        // Green off
#define GAME_BUTTON_LSS_DEFAULT_BLUE            0x00        // Blue off

//
// LED String Service UUIDs
// BASE UUID:
#define GAME_BUTTON_LSS_BASE128_UUID(uuid)  0x68, 0x66, 0x27, 0x33, 0x77, 0xea, 0x37, 0xb0, 0xf6, 0x40, 0xa0, 0x8a, \
                        LO_UINT16(uuid), HI_UINT16(uuid), 0x5e, 0x77

// Service UUID
#define GAME_BUTTON_LSS_SERVICE_SERV_UUID       0x0300

// LED String Switch Characteristic defines
#define GAME_BUTTON_LSS_OFFON_ID                0
#define GAME_BUTTON_LSS_OFFON_UUID              0x0301
#define GAME_BUTTON_LSS_OFFON_LEN               sizeof(offon_char_t)
#define GAME_BUTTON_LSS_OFFON_LEN_MIN           sizeof(offon_char_t)

// LED String RGB Characteristic defines
#define GAME_BUTTON_LSS_RGB_ID                  1
#define GAME_BUTTON_LSS_RGB_UUID                0x0302
#define GAME_BUTTON_LSS_RGB_LEN                 sizeof(button_char_t)
#define GAME_BUTTON_LSS_RGB_LEN_MIN             sizeof(button_char_t)

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*GameButtonServiceChange_t)( uint16_t connHandle, uint16_t svcUuid, uint8_t paramID, uint8_t *pValue, uint16_t len );

typedef struct
{
    GameButtonServiceChange_t        pfnChangeCb;     // Called when characteristic value changes
    GameButtonServiceChange_t        pfnCfgChangeCb;  // Called when characteristic CCCD changes
} GameButtonLssServiceCBs_t;


/*********************************************************************
 * API FUNCTIONS
 */

/*
 * LssService_AddService- Initialises the LssService service by registering
 *          GATT attributes with the GATT server.
 *
 *    rspTaskId - The ICall Task Id that should receive responses for Indications.
 */
extern bStatus_t Game_Button_LssService_AddService( uint8_t rspTaskId );

/*
 * LssService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t Game_Button_LssService_RegisterAppCBs( GameButtonLssServiceCBs_t *appCallbacks );

/*
 * LssService_SetParameter - Set a LssService parameter.
 *
 *    param - Profile parameter ID
 *    len   - length of data to write
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
extern bStatus_t Game_Button_LssService_SetParameter( uint8_t param, uint16_t len, void *value );

/*
 * LssService_GetParameter - Get a LssService parameter.
 *
 *    param - Profile parameter ID
 *    len   - pointer to a variable that contains the maximum length that can be written to *value.
              After the call, this value will contain the actual returned length.
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
extern bStatus_t Game_Button_LssService_GetParameter( uint8_t param, uint16_t *len, void *value );


/*********************************************************************
// BASE UUID: 775ed4a2-8aa0-40f6-b037-ea770326e665 : 77 5e d4 a2 8a a0 40 f6 b0 37 ea 77 03 26 e6 65
#define LSS_BASE128_UUID(uuid)  0x65, 0xe6, 0x26, 0x03, 0x77, 0xea, 0x37, 0xb0, 0xf6, 0x40, 0xa0, 0x8a, \
                        LO_UINT16(uuid), HI_UINT16(uuid), 0x5e, 0x77

*********************************************************************/


#endif /* PROFILES_LSS_SERVICE_H_ */
