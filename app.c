/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "sl_bluetooth.h"
#include "app_assert.h"
#include "gatt_db.h"
#include "app.h"
//#include "app_log.h"

#include "custom_adv.h"

CustomAdv_t sData; // Our custom advertising data stored here

uint8_t num_presses = 0;
uint8_t last_press = 0xFF;

// Macros.
#define UINT16_TO_BYTES(n)            ((uint8_t) (n)), ((uint8_t)((n) >> 8))
#define UINT16_TO_BYTE0(n)            ((uint8_t) (n))
#define UINT16_TO_BYTE1(n)            ((uint8_t) ((n) >> 8))

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
static uint8_t custom_ADV = 0xaa;

/**************************************************************************//**
 * Set up a custom advertisement package according to iBeacon specifications.
 * The advertisement package is 30 bytes long.
 * See the iBeacon specification for further details.
 *****************************************************************************/
static void bcn_setup_adv_beaconing(void);
static void custom_adv_beaconing(void);

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/

volatile uint16_t Adv_counter = 0;
uint16_t TX_counter = 0;
uint16_t RX_counter = 0;
uint16_t CRCERR_counter = 0;
uint16_t FAIL_counter = 0;
#define MAX_ADV_EVTS 1

void sl_bt_on_event(sl_bt_msg_t *evt)
{

//  app_log("test");
  sl_status_t sc;
  int16_t ret_power_min, ret_power_max;
  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Set 0 dBm maximum Transmit Power.
      sc = sl_bt_system_set_tx_power(SL_BT_CONFIG_MIN_TX_POWER, 0,
                                     &ret_power_min, &ret_power_max);
      app_assert_status(sc);
      (void)ret_power_min;
      (void)ret_power_max;
      // Initialize iBeacon ADV data.
      //bcn_setup_adv_beaconing();

      sl_bt_system_get_counters(1, &TX_counter,  &RX_counter, &CRCERR_counter, &FAIL_counter );
      app_assert_status(sc);

      sc = sl_bt_advertiser_create_set(&custom_ADV);
             app_assert(sc == SL_STATUS_OK,
                           "[E: 0x%04x] Failed to create advertising set\n",
                           (int)sc);

      custom_adv_beaconing();
      break;

    case sl_bt_evt_advertiser_timeout_id:

      Adv_counter += MAX_ADV_EVTS;
      sl_bt_system_get_counters(0, &TX_counter,  &RX_counter, &CRCERR_counter, &FAIL_counter );
      custom_adv_beaconing();


//      app_log("Adv_counter = %d \n\r", Adv_counter);
//      app_log("Counters TX = %d, RX = %d, CRC = %d, Failures = %d \n\r", TX_counter, RX_counter, CRCERR_counter, FAIL_counter );
//      app_log("%s\n\r", sData);

      break;
    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

static void custom_adv_beaconing(void)
{
  sl_status_t sc;


        // Set advertising interval to 100ms.
        sc = sl_bt_advertiser_set_timing(
            custom_ADV,
          160, // min. adv. interval (milliseconds * 1.6)
          160, // max. adv. interval (milliseconds * 1.6)
          0,   // adv. duration
          MAX_ADV_EVTS);  // max. num. adv. events

        sl_bt_advertiser_set_channel_map(custom_ADV, 7);

        fill_adv_packet(&sData, 0x06, 0xAAAA, (uint8_t )(Adv_counter>>8), (uint8_t )Adv_counter, "CustomAdvDemo");
        //start_adv(&sData, custom_ADV);

        //app_log("data size %d \n\r", sData.data_size);

#ifndef NONLEGACY

        // Set custom advertising data.
       sc =  sl_bt_legacy_advertiser_set_data(custom_ADV,
                                                     0,
                                                     sData.data_size,
                                                     (const uint8_t *)(&sData));

       app_assert_status(sc);

   //     Start advertising in user mode and disable connections.

               sc = sl_bt_legacy_advertiser_start(
                 custom_ADV,
                 sl_bt_advertiser_non_connectable);

       app_assert_status(sc);



        app_assert_status(sc);

#else

//        sc = sl_bt_advertiser_set_data(custom_ADV,
//                                       0,
//                                       sizeof(sData),
//                                       (uint8_t *)(&sData));
//        app_assert_status(sc);


//        sc = sl_bt_legacy_advertiser_start(
//          custom_ADV,
//          sl_bt_advertiser_non_connectable);

        sc = sl_bt_advertiser_start(
          custom_ADV,
          sl_bt_advertiser_user_data,
          sl_bt_advertiser_non_connectable);


#endif

}
