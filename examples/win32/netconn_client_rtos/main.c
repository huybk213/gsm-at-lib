/**
 * \file            main.c
 * \brief           Main file
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *  
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of GSM-AT library.
 *
 * Before you start using WIN32 implementation with USB and VCP,
 * check gsm_ll_win32.c implementation and choose your COM port!
 */
#include "gsm/gsm.h"
#include "gsm/gsm_private.h"
#include "sim_manager.h"
#include "network_utils.h"
#include "netconn_client.h"
#include "network_apn_settings.h"
#include "stdafx.h"
#include "call_sms.h"
#include "gsm/gsm_ussd.h"

static gsmr_t gsm_callback_func(gsm_evt_t* evt);
/* HuyTV */

static gsmr_t gsm_call_callback(gsm_evt_t* evt)
{
    printf("HuyTV : %s\r\n", __func__);
}



static gsm_api_cmd_evt_fn gsm_finish_cmd_ussd_callback(gsmr_t res, void* arg)
{
    printf("HuyTV : %s\r\n", __func__);
    gsm_msg_t * evt = (gsm_msg_t*)arg;
    if (res == gsmOK)
    {
        printf("Query money success. Money %s\r\n", evt->msg.ussd.resp_len, evt->msg.ussd.resp);
    }
    else
    {
        printf("Query money failed\r\n");
    }
}

/* End */
/**
 * \brief           Program entry point
 */
int
main(void) {
    printf("Starting GSM application!\r\n");

    /* Initialize GSM with default callback function */
    if (gsm_init(gsm_callback_func, 1) != gsmOK) {
        printf("Cannot initialize GSM-AT Library\r\n");
    }

    /* Configure device by unlocking SIM card */
    if (configure_sim_card()) {
        printf("SIM card configured. Adding delay to stabilize SIM card.\r\n");
        gsm_delay(10000);
    } else {
        printf("Cannot configure SIM card! Is it inserted, pin valid and not under PUK? Closing down...\r\n");
        while (1) { gsm_delay(1000); }
    }

    /* Set APN credentials */
    gsm_network_set_credentials(NETWORK_APN, NETWORK_APN_USER, NETWORK_APN_PASS);

    /* Start netconn thread */
    gsm_sys_thread_create(NULL, "netconn_thread", (gsm_sys_thread_t)netconn_client_thread, NULL, GSM_SYS_THREAD_SS, GSM_SYS_THREAD_PRIO);


    // HuyTV
    /* Start call and sms func */
    printf("Start call and sms function\r\n");
    call_sms_start();

    /* Get money */
    char money_response[128];
    gsm_msg_t ussd_msg;
    gsm_ussd_run("*101#", money_response, sizeof(money_response), gsm_finish_cmd_ussd_callback, &ussd_msg, 1);

    /* Call to master every device reboot */
    const char* master_number = "0942018895";
    printf("Calling to %s\r\n", master_number);
    gsm_call_start(master_number, gsm_call_callback, NULL, 1);

    // End

    /*
     * Do not stop program here.
     * New threads were created for GSM processing
     */
    while (1) {
        gsm_delay(1000);
    }

    return 0;
}

/**
 * \brief           Event callback function for GSM stack
 * \param[in]       evt: Event information with data
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
static gsmr_t
gsm_callback_func(gsm_evt_t* evt) {
    switch (gsm_evt_get_type(evt)) {
        case GSM_EVT_INIT_FINISH: printf("Library initialized!\r\n"); break;
        /* Process and print registration change */
        case GSM_EVT_NETWORK_REG_CHANGED: 
        {
            printf("Network registration changed\r\n");
            network_utils_process_reg_change(evt); 
            break;
        }
        /* Process current network operator */
        case GSM_EVT_NETWORK_OPERATOR_CURRENT: 
        {
            printf("GSM_EVT_NETWORK_OPERATOR_CURRENT\r\n");
            network_utils_process_curr_operator(evt);
            break;
        }
        /* Process signal strength */
        case GSM_EVT_SIGNAL_STRENGTH: 
        {
            printf("GSM_EVT_SIGNAL_STRENGTH\r\n");
            network_utils_process_rssi(evt);
            break;
        }
        /* Other user events here... */

        default: break;
    }
    return gsmOK;
}
