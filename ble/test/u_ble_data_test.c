/*
 * Copyright 2020 u-blox Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Only #includes of u_* and the C standard library are allowed here,
 * no platform stuff and no OS stuff.  Anything required from
 * the platform/OS must be brought in through u_port* to maintain
 * portability.
 */

/** @file
 * @brief Tests for the ble "general" API: these should pass on all
 * platforms where one UART is available. No short range module is
 * actually used in this set of tests.
 */

#ifdef U_CFG_OVERRIDE
# include "u_cfg_override.h" // For a customer's configuration override
#endif

#include "stddef.h"    // NULL, size_t etc.
#include "stdint.h"    // int32_t etc.
#include "stdbool.h"

#include "u_cfg_sw.h"
#include "u_cfg_app_platform_specific.h"
#include "u_cfg_test_platform_specific.h"

#include "u_error_common.h"

#include "u_port.h"
#include "u_port_debug.h"
#include "u_port_os.h"
#include "u_port_uart.h"

#include "u_at_client.h"

#include "u_short_range_module_type.h"
#include "u_short_range_edm_stream.h"
#include "u_ble_module_type.h"
#include "u_ble.h"
#ifdef U_CFG_TEST_SHORT_RANGE_MODULE_TYPE
#include "u_ble_data.h"
#endif
#include "u_ble_test_private.h"

/* ----------------------------------------------------------------
 * COMPILE-TIME MACROS
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * TYPES
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * VARIABLES
 * -------------------------------------------------------------- */

static uBleTestPrivate_t gHandles = { -1, -1, NULL, -1 };

/* ----------------------------------------------------------------
 * STATIC FUNCTIONS
 * -------------------------------------------------------------- */

/* ----------------------------------------------------------------
 * PUBLIC FUNCTIONS
 * -------------------------------------------------------------- */

#ifdef U_CFG_TEST_SHORT_RANGE_MODULE_TYPE

//lint -e{818} Suppress 'pData' could be declared as const:
// need to follow function signature
static void dataCallback(int32_t channel, size_t length,
                         char *pData, void *pParameters)
{
    (void) channel;
    (void) length;
    (void) pData;
    (void) pParameters;
}

static void connectionCallback(int32_t connHandle, char *address, int32_t type,
                               int32_t channel, int32_t mtu, void *pParameters)
{
    (void) connHandle;
    (void) address;
    (void) type;
    (void) channel;
    (void) mtu;
    (void) pParameters;
}


U_PORT_TEST_FUNCTION("[bleData]", "bleData")
{
    int32_t heapUsed;
    heapUsed = uPortGetHeapFree();

    U_PORT_TEST_ASSERT(uBleTestPrivatePreamble(U_CFG_TEST_SHORT_RANGE_MODULE_TYPE,
                                               &gHandles) == 0);


    U_PORT_TEST_ASSERT(uBleDataSetCallbackConnectionStatus(gHandles.bleHandle,
                                                           connectionCallback,
                                                           NULL) == 0);

    U_PORT_TEST_ASSERT(uBleDataSetCallbackConnectionStatus(gHandles.bleHandle,
                                                           connectionCallback,
                                                           NULL) != 0);

    U_PORT_TEST_ASSERT(uBleDataSetCallbackConnectionStatus(gHandles.bleHandle,
                                                           NULL, NULL) == 0);

    U_PORT_TEST_ASSERT(uBleDataSetCallbackConnectionStatus(gHandles.bleHandle,
                                                           connectionCallback,
                                                           NULL) == 0);

    U_PORT_TEST_ASSERT(uBleDataSetCallbackConnectionStatus(gHandles.bleHandle,
                                                           NULL, NULL) == 0);


    U_PORT_TEST_ASSERT(uBleDataSetCallbackData(gHandles.bleHandle, dataCallback, NULL) == 0);
    U_PORT_TEST_ASSERT(uBleDataSetCallbackData(gHandles.bleHandle, dataCallback, NULL) != 0);
    U_PORT_TEST_ASSERT(uBleDataSetCallbackData(gHandles.bleHandle, NULL, NULL) == 0);
    U_PORT_TEST_ASSERT(uBleDataSetCallbackData(gHandles.bleHandle, dataCallback, NULL) == 0);
    U_PORT_TEST_ASSERT(uBleDataSetCallbackData(gHandles.bleHandle, NULL, NULL) == 0);

    uBleTestPrivatePostamble(&gHandles);

#ifndef __XTENSA__
    // Check for memory leaks
    // TODO: this if'ed out for ESP32 (xtensa compiler) at
    // the moment as there is an issue with ESP32 hanging
    // on to memory in the UART drivers that can't easily be
    // accounted for.
    heapUsed -= uPortGetHeapFree();
    uPortLog("U_BLE_TEST: we have leaked %d byte(s).\n", heapUsed);
    // heapUsed < 0 for the Zephyr case where the heap can look
    // like it increases (negative leak)
    U_PORT_TEST_ASSERT(heapUsed <= 0);
#else
    (void) heapUsed;
#endif
}

#endif

/** Clean-up to be run at the end of this round of tests, just
 * in case there were test failures which would have resulted
 * in the deinitialisation being skipped.
 */
U_PORT_TEST_FUNCTION("[bleData]", "bleDataCleanUp")
{
    int32_t x;

    uBleDeinit();
    uAtClientDeinit();
    if (gHandles.uartHandle >= 0) {
        uPortUartClose(gHandles.uartHandle);
    }
    if (gHandles.edmStreamHandle >= 0) {
        uShortRangeEdmStreamClose(gHandles.edmStreamHandle);
    }

    x = uPortTaskStackMinFree(NULL);
    uPortLog("U_BLE_TEST: main task stack had a minimum of %d"
             " byte(s) free at the end of these tests.\n", x);
    U_PORT_TEST_ASSERT(x >= U_CFG_TEST_OS_MAIN_TASK_MIN_FREE_STACK_BYTES);

    uPortDeinit();

    x = uPortGetHeapMinFree();
    if (x >= 0) {
        uPortLog("U_BLE_TEST: heap had a minimum of %d"
                 " byte(s) free at the end of these tests.\n", x);
        U_PORT_TEST_ASSERT(x >= U_CFG_TEST_HEAP_MIN_FREE_BYTES);
    }
}

// End of file
