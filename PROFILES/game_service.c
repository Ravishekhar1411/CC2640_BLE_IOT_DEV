/******************************************************************************
 * Filename:       game_service.c
 *
 * Description:    This file contains the configuration
 *              definitions and prototypes for the iOS Workshop
 *

 TODO

 Need to introduce left right up and down read characterstic for
 game operation and press button.



// Commit on 07 JUNE 2026
/*********************************************************************
 * INCLUDES
 */

#include <string.h>

//#define xdc_runtime_Log_DISABLE_ALL 1  // Add to disable logs from this file
#include <xdc/runtime/Diags.h>
#include <uartlog/UartLog.h>

#include <icall.h>
#include "util.h"
/* This Header file contains all BLE API and ICall structure definition */
#include "icall_ble_api.h"

#include "labs.h"
#include "common.h"
#include "game_service.h"

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// gameService Service UUID
CONST uint8_t gameServiceUUID[ATT_UUID_SIZE] =
{
  game_BASE128_UUID(game_SERVICE_SERV_UUID)
};

// LUMINANCE
CONST uint8_t game_LUMINUUID[ATT_UUID_SIZE] =
{
  game_BASE128_UUID(game_player_right_UUID)
};

// THRESH
CONST uint8_t game_THRESHUUID[ATT_UUID_SIZE] =
{
    game_BASE128_UUID(game_player_left_UUID)
};

// HYST
CONST uint8_t game_HYSTUUID[ATT_UUID_SIZE] =
{
    game_BASE128_UUID(game_player_up_button_UUID)
};

// LMOFFON
CONST uint8_t game_LMOFFONUUID[ATT_UUID_SIZE] =
{
     game_BASE128_UUID(game_player_down_button_UUID)
};

/*********************************************************************
 * LOCAL VARIABLES
 */

static gameServiceCBs_t *pAppCBs = NULL;
static uint8_t game_icall_rsp_task_id = INVALID_TASK_ID;

/*********************************************************************
* Profile Attributes - variables
*/

// Service declaration
static CONST gattAttrType_t gameServiceDecl = { ATT_UUID_SIZE, gameServiceUUID };

// Characteristic "LUMINANCE" Properties (for declaration)
static uint8_t game_LUMINProps = GATT_PROP_READ | GATT_PROP_NOTIFY;

// Characteristic "LUMINANCE" Value variable
static uint8_t game_LUMINVal[game_LUMIN_LEN] = {0, 0};

// Characteristic "LUMINANCE" Client Characteristic Configuration Descriptor
static gattCharCfg_t *game_LUMINConfig;

// Length of data in characteristic "LUMINANCE" Value Length variable, initialised to minimum size.
static uint16_t game_LUMINValLen = game_LUMIN_LEN_MIN;

// Characteristic "LUMINANCE" User Description
static uint8_t game_LUMINUserDesc[10] = {0};

// Characteristic "THRESH" Properties (for declaration)
static uint8_t game_THRESHProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic "THRESH" Value variable
static uint8_t game_THRESHVal[game_THRESH_LEN] = { game_DEFAULT_ENEMY };

// Length of data in characteristic "THRESH" Value variable, initialised to minimum size
static uint16_t game_THRESHValLen = game_THRESH_LEN_MIN;

// Characteristic "THRESH" User Description
static uint8_t game_THRESHUserDesc[24] = "Game Monitor Threshold\0";


// Characteristic "HYST" Properties (for declaration)
static uint8_t game_HYSTProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic "HYST" Value variable
static uint8_t game_HYSTVal[ALS_HYST_LEN] = { ALS_DEFAULT_HYST };

// Length of data in characteristic "HYST" Value variable, initialised to minimum size
static uint16_t game_HYSTValLen = game_HYST_LEN_MIN;

// Characteristic "HYST" User Description
static uint8_t game_HYSTUserDesc[25] = "Game Monitor Hysteresis\0";


// Characteristic "LMOFFON" Properties (for declaration)
static uint8_t game_LMOFFONProps = GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_WRITE_NO_RSP;

// Characteristic "LMOFFON" Value variable
static uint8_t game_LMOFFONVal[game_LMOFFON_LEN] = { game_DEFAULT_LIFELINE };

// Length of data in characteristic "LMOFFON" Value variable, initialised to minimum size
static uint16_t game_LMOFFONValLen = game_LMOFFON_LEN_MIN;

// Characteristic "LMOFFON" User Description
static uint8_t game_LMOFFONUserDesc[21] = "Game Monitor Off/On\0";

/*********************************************************************
* Profile Attributes - Table
*/

static gattAttribute_t gameServiceAttrTbl[] = {

    // gameService Service Declaration
    {
        { ATT_BT_UUID_SIZE, primaryServiceUUID },
        GATT_PERMIT_READ,
        0,
        (uint8_t *)&gameServiceDecl
        },
    //
    // LUMINANCE Characteristic Declaration
    //
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &game_LUMINProps
    },
    // LUMINANCE Characteristic Value
    {
        { ATT_UUID_SIZE, game_LUMINUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        game_LUMINVal
    },
    // LUMINANCE Client Characteristic Configuration Descriptor (CCCD)
    {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8_t *)&game_LUMINConfig
    },
    // LUMINANCE Characteristic User Description
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        game_LUMINUserDesc
    },

    // THRESH Characteristic declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &game_THRESHProps
    },
    // THRESH Characteristic Value
    {
        { ATT_UUID_SIZE, game_THRESHUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        game_THRESHVal
    },
    // THRESH Characteristic User Description
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        game_THRESHUserDesc
    },

    //
    // HYST Characteristic declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &game_HYSTProps
    },
    // HYST Characteristic Value
    {
        { ATT_UUID_SIZE, game_HYSTUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        game_HYSTVal
    },
    // HYST Characteristic User Description
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        game_HYSTUserDesc
    },

    //
    // LMOFFON Characteristic declaration
    {
        { ATT_BT_UUID_SIZE, characterUUID },
        GATT_PERMIT_READ,
        0,
        &game_LMOFFONProps
    },
    // LMOFFON Characteristic Value
    {
        { ATT_UUID_SIZE, game_LMOFFONUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        game_LMOFFONVal
    },
    // LMOFFON Characteristic User Description
    {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        game_LMOFFONUserDesc
    }

};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static bStatus_t gameService_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                           uint16_t maxLen, uint8_t method );
static bStatus_t gameService_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                            uint8_t *pValue, uint16_t len, uint16_t offset,
                                            uint8_t method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// game Service Callbacks
CONST gattServiceCBs_t gameServiceCBs =
{
    gameService_ReadAttrCB,  // Read callback function pointer
    gameService_WriteAttrCB, // Write callback function pointer
    NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * EXTERN FUNCTIONS
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*
 * gameService_AddService- Initializes the gameService service by registering
 *          GATT attributes with the GATT server.
 *
 *    rspTaskId - The ICall Task Id that should receive responses for Indications.
 */
extern bStatus_t gameService_AddService( uint8_t rspTaskId )
{

    uint8_t status;

    // Allocate Client Characteristic Configuration table
    game_LUMINConfig = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) * linkDBNumConns );
    if ( game_LUMINConfig == NULL )
    {
        return ( bleMemAllocError );
    }

    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, game_LUMINConfig );

    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( gameServiceAttrTbl,
                                          GATT_NUM_ATTRS( gameServiceAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &gameServiceCBs );
    Log_info1("Registered service, %d attributes", (IArg)GATT_NUM_ATTRS( gameServiceAttrTbl ));
    return ( status );
}

/*
 * gameService_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
bStatus_t gameService_RegisterAppCBs( gameServiceCBs_t *appCallbacks )
{

    if ( appCallbacks )
    {
        pAppCBs = appCallbacks;
        Log_info1("Registered callbacks to application. Struct %p", (IArg)appCallbacks);
        return ( SUCCESS );
    }
    else
    {
        Log_warning0("Null pointer given for app callbacks.");
        return ( FAILURE );
    }
}

/*
 * gameService_SetParameter - Set an gameService parameter.
 *
 *    param - Profile parameter ID
 *    len   - length of data to write
 *    value - pointer to data to write.  This is dependent on
 *            the parameter ID and may be cast to the appropriate
 *            data type (example: data type of uint16_t will be cast to
 *            uint16_t pointer).
 */
bStatus_t gameService_SetParameter( uint8_t param, uint16_t len, void *value )
{

    bStatus_t ret = SUCCESS;
    uint8_t  *pAttrVal;
    uint8_t sendNotiInd = false;
    uint8_t needAuth;
    uint16_t *pValLen;
    uint16_t valMinLen;
    uint16_t valMaxLen;
    gattCharCfg_t *attrConfig;

    switch ( param )
    {

    case game_player_right:
        pAttrVal  =  game_LUMINVal;
        pValLen   = &game_LUMINValLen;
        valMinLen =  game_LUMIN_LEN_MIN;
        valMaxLen =  game_LUMIN_LEN;
        sendNotiInd = true;
        attrConfig = game_LUMINConfig;
        needAuth    = FALSE; // Change if authenticated link is required for sending.
        Log_info2("SetParameter : %s len: %d", (IArg)"LUMINANCE", (IArg)len);
        break;

    case game_player_left:
        pAttrVal = game_THRESHVal;
        pValLen = &game_THRESHValLen;
        valMinLen = game_THRESH_LEN_MIN;
        valMaxLen = game_THRESH_LEN;
        Log_info2("SetParameter : %s len: %d", (IArg)"THRESH", (IArg)len);
        break;

    case game_player_up_button_press:
        pAttrVal = game_HYSTVal;
        pValLen = &game_HYSTValLen;
        valMinLen = game_HYST_LEN_MIN;
        valMaxLen = game_HYST_LEN;
        Log_info2("SetParameter : %s len: %d", (IArg)"HYST", (IArg)len);
        break;

    case game_player_down_button_press:
        pAttrVal = game_LMOFFONVal;
        pValLen = &game_LMOFFONValLen;
        valMinLen = game_LMOFFON_LEN_MIN;
        valMaxLen = game_LMOFFON_LEN;
        Log_info2("SetParameter : %s len: %d", (IArg)"LMOFFON", (IArg)len);
        break;

    default:
        Log_error1("SetParameter: Parameter #%d not valid.", (IArg)param);
        return INVALIDPARAMETER;
    }


    return ret;

}


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
bStatus_t gameService_GetParameter( uint8_t param, uint16_t *len, void *value )
{

    bStatus_t ret = SUCCESS;

    switch ( param )
    {

    case game_player_right:
        *len = MIN(*len, game_LUMINValLen);
        memcpy(value, game_LUMINVal, *len);
        Log_info2("GetParameter : %s returning %d bytes", (IArg)"LUMINANCE", (IArg)*len);
        break;

    case game_player_left:
        *len = MIN(*len, game_THRESHValLen);
        memcpy(value, game_THRESHVal, *len);
        Log_info2("GetParameter : %s returning %d bytes", (IArg)"THRESH", (IArg)*len);
        break;

    case game_player_up_button_press:
        *len = MIN(*len, game_HYSTValLen);
        memcpy(value, game_HYSTVal, *len);
        Log_info2("GetParameter : %s returning %d bytes", (IArg)"HYST", (IArg)*len);
        break;

    case game_player_down_button_press:
        *len = MIN(*len, game_LMOFFONValLen);
        memcpy(value, game_LMOFFONVal, *len);
        Log_info2("GetParameter : %s returning %d bytes", (IArg)"LMOFFON", (IArg)*len);
        break;

    default:
        Log_error1("GetParameter: Parameter #%d not valid.", (IArg)param);
        ret = INVALIDPARAMETER;
        break;
    }

    return ret;

}

/*********************************************************************
 * @internal
 * @fn          gameService_findCharParamId
 *
 * @brief       Find the logical param id of an attribute in the service's attr table.
 *
 *              Works only for Characteristic Value attributes and
 *              Client Characteristic Configuration Descriptor attributes.
 *
 * @param       pAttr - pointer to attribute
 *
 * @return      uint8_t paramID (ref game_service.h) or 0xFF if not found.
 */
static uint8_t gameService_findCharParamId(gattAttribute_t *pAttr)
{

    // Is this a Client Characteristic Configuration Descriptor?
    if (ATT_BT_UUID_SIZE == pAttr->type.len && GATT_CLIENT_CHAR_CFG_UUID == *(uint16_t *)pAttr->type.uuid)
        return gameService_findCharParamId(pAttr - 1); // Assume the value attribute precedes CCCD and recurse

    // Is this attribute in "LUMINANCE"?
    else if ( ATT_UUID_SIZE == pAttr->type.len && !memcmp(pAttr->type.uuid, game_LUMINUUID, pAttr->type.len))
        return game_player_right;

    // Is this attribute in "THRESH"?
    else if ( ATT_UUID_SIZE == pAttr->type.len && !memcmp(pAttr->type.uuid, game_THRESHUUID, pAttr->type.len))
        return game_player_left;

    // Is this attribute in "HYST"?
    else if ( ATT_UUID_SIZE == pAttr->type.len && !memcmp(pAttr->type.uuid, game_HYSTUUID, pAttr->type.len))
        return game_player_up_button_press;

    // Is this attribute in "LMOFFON"?
    else if ( ATT_UUID_SIZE == pAttr->type.len && !memcmp(pAttr->type.uuid, game_LMOFFONUUID, pAttr->type.len))
        return game_player_down_button_press;

    else
        return 0xFF; // Not found. Return invalid.
}

/*********************************************************************
 * @fn          gameService_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t gameService_ReadAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                       uint8_t *pValue, uint16_t *pLen, uint16_t offset,
                                       uint16_t maxLen, uint8_t method )
{

    bStatus_t status = SUCCESS;
    uint16_t valueLen;
    uint8_t paramID = 0xFF;

    // Find settings for the characteristic to be read.
    paramID = gameService_findCharParamId( pAttr );
    switch ( paramID )
    {

    case game_player_right:
        valueLen = game_LUMINValLen;
        /* Other considerations for LUMINANCE can be inserted here */
        break;

    case game_player_left:
        valueLen = game_THRESHValLen;
        break;

    case game_player_up_button_press:
        valueLen = game_HYSTValLen;
        break;

    case game_player_down_button_press:
        valueLen = game_LMOFFONValLen;
        break;

    default:
        Log_error0("Attribute was not found.");
        return ATT_ERR_ATTR_NOT_FOUND;
    }

    // Check bounds and return the value
    if ( offset > valueLen )  // Prevent malicious ATT ReadBlob offsets.
    {
        Log_error0("An invalid offset was requested.");
        status = ATT_ERR_INVALID_OFFSET;
    }
    else
    {
        *pLen = MIN(maxLen, valueLen - offset);  // Transmit as much as possible
        memcpy(pValue, pAttr->pValue + offset, *pLen);
    }

    return status;

}

/*********************************************************************
 * @fn      gameService_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t gameService_WriteAttrCB( uint16_t connHandle, gattAttribute_t *pAttr,
                                        uint8_t *pValue, uint16_t len, uint16_t offset,
                                        uint8_t method )
{

    bStatus_t status  = SUCCESS;
    uint8_t   paramID = 0xFF;
    uint8_t   changeParamID = 0xFF;
    uint16_t  writeLenMin;
    uint16_t  writeLenMax;
    uint16_t  *pValueLenVar;

    // See if request is regarding a Client Characterisic Configuration
    if (ATT_BT_UUID_SIZE == pAttr->type.len && GATT_CLIENT_CHAR_CFG_UUID == *(uint16_t *)pAttr->type.uuid)
    {
        Log_info3("WriteAttrCB (CCCD): param: %d connHandle: %d %s",
                  (IArg)gameService_findCharParamId(pAttr),
                  (IArg)connHandle,
                  (IArg)(method == GATT_LOCAL_WRITE?"- restoring bonded state":"- OTA write"));

        // Allow notification and indication, but do not check if really allowed per CCCD.
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY |
                                                 GATT_CLIENT_CFG_INDICATE );

        if (SUCCESS == status && pAppCBs && pAppCBs->pfnCfgChangeCb)
            pAppCBs->pfnCfgChangeCb( connHandle, game_SERVICE_SERV_UUID,
                                     gameService_findCharParamId(pAttr), pValue, len );

        return status;

    }

    // Find settings for the characteristic to be written.
    paramID = gameService_findCharParamId( pAttr );
    switch ( paramID )
    {

    case game_player_left:
        writeLenMin  = game_THRESH_LEN_MIN;
        writeLenMax  = game_THRESH_LEN;
        pValueLenVar = &game_THRESHValLen;
        break;

    case game_player_up_button_press:
        writeLenMin  = game_HYST_LEN_MIN;
        writeLenMax  = game_HYST_LEN;
        pValueLenVar = &game_HYSTValLen;
        break;

    case game_player_down_button_press:
        writeLenMin  = game_LMOFFON_LEN_MIN;
        writeLenMax  = game_LMOFFON_LEN;
        pValueLenVar = &game_LMOFFONValLen;
        break;

    default:
        Log_error0("Attribute was not found.");
        return ATT_ERR_ATTR_NOT_FOUND;
    }

    // Check whether the length is within bounds.
    if ( offset >= writeLenMax )
    {
        Log_error0("An invalid offset was requested.");
        status = ATT_ERR_INVALID_OFFSET;
    }
    else if ( offset + len > writeLenMax )
    {
        Log_error0("Invalid value length was received.");
        status = ATT_ERR_INVALID_VALUE_SIZE;
    }
    else if ( offset + len < writeLenMin && ( method == ATT_EXECUTE_WRITE_REQ || method == ATT_WRITE_REQ ) )
    {
        // Refuse writes that are lower than minimum.
        // Note: Cannot determine if a Reliable Write (to several chars) is finished, so those will
        //       only be refused if this attribute is the last in the queue (method is execute).
        //       Otherwise, reliable writes are accepted and parsed piecemeal.
        Log_error0("Invalid value length was received.");
        status = ATT_ERR_INVALID_VALUE_SIZE;
    }
    else
    {
        // Copy pValue into the variable we point to from the attribute table.
        memcpy(pAttr->pValue + offset, pValue, len);

        // Only notify application and update length if enough data is written.
        //
        // Note: If reliable writes are used (meaning several attributes are written to using ATT PrepareWrite),
        //       the application will get a callback for every write with an offset + len larger than _LEN_MIN.
        // Note: For Long Writes (ATT Prepare + Execute towards only one attribute) only one callback will be issued,
        //       because the write fragments are concatenated before being sent here.
        if ( offset + len >= writeLenMin )
        {
            changeParamID = paramID;
            *pValueLenVar = offset + len; // Update data length.
        }
    }

    // Let the application know something changed (if it did) by using the
    // callback it registered earlier (if it did).
    if (changeParamID != 0xFF)
        if ( pAppCBs && pAppCBs->pfnChangeCb )
            pAppCBs->pfnChangeCb( connHandle, game_SERVICE_SERV_UUID, paramID, pValue, len+offset ); // Call app function from stack task context.

    return status;

}

/*********************************************************************
*********************************************************************/



