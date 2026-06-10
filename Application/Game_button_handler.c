/******************************************************************************
 * Filename:       Game_bitton_handler.c
 *
 * Description:    This file contains the configuration
 *                 for future game development of andoroid game buttons i.e. Left, right, up and down
 *

 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "labs.h"
#include "common.h"

#include "icall_ble_api.h"
#include <icall.h>

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/devices/cc26x0r2/driverlib/vims.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <driverlib/ioc.h>
#include <driverlib/prcm.h>
#include <driverlib/ssi.h>
#include <driverlib/trng.h>
#include <driverlib/udma.h>
#include <driverlib/aux_adc.h>
#include <driverlib/aux_wuc.h>
#include <Game_button_handler.h>

#include <xdc/runtime/Diags.h>
#include <uartlog/UartLog.h>
#include <osal_snv.h>

#include "project_zero.h"
#include "lss_handler.h"
#include "lss_service.h"
#include "als_service.h"

/*********************************************************************
 * CONSTANTS
 */



