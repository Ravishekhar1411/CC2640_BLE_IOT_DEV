/******************************************************************************
 * Filename:       game_service.h
 *
 * Description:    This file contains the configuration
 *              definitions and prototypes for the iOS Workshop
 *
 * Copyright (c) 2018, Ekko Tech Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materigame provided with the distribution.
 *
 * *  Neither the name of Ekko Tech Limited nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
// Commit on 07 JUNE 2026
#ifndef PROFILES_game_SERVICE_H_
#define PROFILES_game_SERVICE_H_


/*********************************************************************
 * INCLUDES
 */
#include "labs.h"
#include "common.h"

#include <bcomdef.h>

/*********************************************************************
 * TYPEDEFS
 */
//
// Memory layout of characteristic data
//
// Ambient Light Service
// LMLUMIN characteristic
typedef uint16_t lumin_char_t;

// LMTHRESH characteristic
typedef uint16_t thresh_char_t;

// LMHYST characteristic
typedef uint8_t hyst_char_t;

// LMOFFON characteristics
typedef uint8_t lmoffon_char_t;

/*********************************************************************
 * CONSTANTS
 */

//
// Default values for characteristics
//
#define game_DEFAULT_STATUS         50           // Status
#define game_DEFAULT_ENEMY           5           // No of enemies
#define game_DEFAULT_LIFELINE        1           // Life Line

//
// Ambient Light Service UUIDs
// BASE UUID:

#define game_BASE128_UUID(uuid)  0x70, 0xe4, 0x24, 0x03, 0x77, 0xea, 0x37, 0xb0, 0xf6, 0x40, 0xa0, 0x8a, \
                        LO_UINT16(uuid), HI_UINT16(uuid), 0x5e, 0x77

// Service UUID
#define game_SERVICE_SERV_UUID       0x0500

// game Luminance Characteristic defines
#define game_player_right                0
#define game_player_right_UUID              0x0501
#define game_LUMIN_LEN               sizeof(lumin_char_t)
#define game_LUMIN_LEN_MIN           sizeof(lumin_char_t)

// Light Monitor Threshold
#define game_player_left               1
#define game_player_left_UUID             0x0502
#define game_THRESH_LEN              sizeof(thresh_char_t)
#define game_THRESH_LEN_MIN          sizeof(thresh_char_t)

// Light Monitor Hysteresis
#define game_player_up_button_press  2
#define game_player_up_button_UUID               0x0503
#define game_HYST_LEN                sizeof(hyst_char_t)
#define game_HYST_LEN_MIN            sizeof(hyst_char_t)

// Light Monitor Off/On
#define game_player_down_button_press 3
#define game_player_down_button_UUID             0x0504
#define game_LMOFFON_LEN              sizeof(lmoffon_char_t)
#define game_LMOFFON_LEN_MIN          sizeof(lmoffon_char_t)

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*gameServiceChange_t)( uint16_t connHandle, uint16_t svcUuid, uint8_t paramID,
                                                        uint8_t *pValue, uint16_t len );

typedef struct
{
  gameServiceChange_t        pfnChangeCb;     // Called when characteristic value changes
  gameServiceChange_t        pfnCfgChangeCb;  // Called when characteristic CCCD changes
} gameServiceCBs_t;


/*********************************************************************
 * API FUNCTIONS
 */

/*
 * gameService_AddService- Initializes the gameService service by registering
 *          GATT attributes with the GATT server.
 *
 *    rspTaskId - The ICall Task Id that should receive responses for Indications.
 */
extern bStatus_t gameService_AddService( uint8_t rspTaskId );

/*
 * gameService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t gameService_RegisterAppCBs( gameServiceCBs_t *appCallbacks );

/*
 * gameService_SetParameter - Set a gameService parameter.
 *
 *    param - Profile parameter ID
 *    len   - length of data to write
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
extern bStatus_t gameService_SetParameter( uint8_t param, uint16_t len, void *value );

/*
 * gameService_GetParameter - Get a gameService parameter.
 *
 *    param - Profile parameter ID
 *    len   - pointer to a variable that contains the maximum length that can be written to *value.
              After the call, this value will contain the actual returned length.
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
extern bStatus_t gameService_GetParameter( uint8_t param, uint16_t *len, void *value );


/*********************************************************************
*********************************************************************/


#endif /* PROFILES_game_SERVICE_H_ */
