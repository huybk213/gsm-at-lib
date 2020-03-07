/******************************************************************************
 * @file:    vsm_cli.c
 * @brief:
 * @version: V0.0.0
 * @date:    2019/11/12
 * @author:
 * @email:
 *
 * THE SOURCE CODE AND ITS RELATED DOCUMENTATION IS PROVIDED "AS IS". VINSMART
 * JSC MAKES NO OTHER WARRANTY OF ANY KIND, WHETHER EXPRESS, IMPLIED OR,
 * STATUTORY AND DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * SATISFACTORY QUALITY, NON INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * THE SOURCE CODE AND DOCUMENTATION MAY INCLUDE ERRORS. VINSMART JSC
 * RESERVES THE RIGHT TO INCORPORATE MODIFICATIONS TO THE SOURCE CODE IN LATER
 * REVISIONS OF IT, AND TO MAKE IMPROVEMENTS OR CHANGES IN THE DOCUMENTATION OR
 * THE PRODUCTS OR TECHNOLOGIES DESCRIBED THEREIN AT ANY TIME.
 *
 * VINSMART JSC SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGE OR LIABILITY ARISING FROM YOUR USE OF THE SOURCE CODE OR
 * ANY DOCUMENTATION, INCLUDING BUT NOT LIMITED TO, LOST REVENUES, DATA OR
 * PROFITS, DAMAGES OF ANY SPECIAL, INCIDENTAL OR CONSEQUENTIAL NATURE, PUNITIVE
 * DAMAGES, LOSS OF PROPERTY OR LOSS OF PROFITS ARISING OUT OF OR IN CONNECTION
 * WITH THIS AGREEMENT, OR BEING UNUSABLE, EVEN IF ADVISED OF THE POSSIBILITY OR
 * PROBABILITY OF SUCH DAMAGES AND WHETHER A CLAIM FOR SUCH DAMAGE IS BASED UPON
 * WARRANTY, CONTRACT, TORT, NEGLIGENCE OR OTHERWISE.
 *
 * (C)Copyright VINSMART JSC 2019 All rights reserved
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "app_cli.h"
#include "vsm_shell.h"
#include "stdafx.h"
#include "gsm_sys_port.h"
#include "gsm/gsm.h"
#include "stdafx.h"
#include "gsm/gsm_ussd.h"
#include "gsm/gsm.h"
#include "gsm/gsm_private.h"
#include "sim_manager.h"
#include "network_utils.h"
#include "gsm_app.h"
#include "call_sms.h"

static void vsm_cli_main_loop(void* pvParameters);
static int32_t cli_connect_to_server(p_shell_context_t context, int32_t argc, char **argv);
static int32_t cli_factory_reset(p_shell_context_t context, int32_t argc, char **argv);
static int32_t cli_ussd_money(p_shell_context_t context, int32_t argc, char **argv);
static int32_t cli_trace_task_stack(p_shell_context_t context, int32_t argc, char **argv);
static int32_t cli_get_rssi(p_shell_context_t context, int32_t argc, char** argv);
static int32_t cli_send_sms(p_shell_context_t context, int32_t argc, char** argv);
static int32_t cli_call(p_shell_context_t context, int32_t argc, char** argv);
static int32_t cli_config_sim(p_shell_context_t context, int32_t argc, char** argv);

static const shell_command_context_t cli_command_table[] = 
{
    {"connect",       "\tconnect: Connect to http server\r\n",                cli_connect_to_server,       1},
    {"ussd",        "\tussd: ussd function\r\n",                 cli_ussd_money,     1},
    {"factory",     "\tFactory: Factory reset\r\n",             cli_factory_reset,      0},
    {"stack",       "\tStack: Trace task stack size\r\n",       cli_trace_task_stack,   0},
    {"rssi",       "\tRSSI: Get signal strength\r\n",       cli_get_rssi,   0},
    {"sms",       "\tsms: Send sms\r\n",       cli_send_sms,   2},
    {"call",       "\tcall: Call to specific number\r\n",       cli_call,   1},
    {"configsim",       "\tconfigsim: Config sim card\r\n",       cli_config_sim,   0}

};



shell_context_struct user_context;


void user_put_char(uint8_t *buf, uint32_t len)
{
    printf("%s", (char*)buf);
    //fflush(stdout);
}

void user_get_char(uint8_t *buf, uint32_t len)
{
    // xSerialGetChar ((char*)buf, portMAX_DELAY) ;

    *buf = (uint8_t)(getchar());
}


void vsm_cli_start(void)
{

    gsm_sys_thread_create(NULL, "gsm_cli", (gsm_sys_thread_t)vsm_cli_main_loop, NULL, 4096, 0);
}

/*! \brief  VSM CLI Task
*/
void vsm_cli_main_loop(void* pvParameters)
{
    setbuf(stdout, NULL);
    SHELL_Init(&user_context,
            user_put_char,
            user_get_char,
            printf,
            ">");

    /* Register CLI commands */
    for(int i = 0; i < sizeof(cli_command_table)/sizeof(shell_command_context_t); i ++) {
        SHELL_RegisterCommand(&cli_command_table[i]);
    }

    /* Run CLI task */
    SHELL_Main(&user_context);
}

/* Reset System */
static int32_t cli_connect_to_server(p_shell_context_t context, int32_t argc, char **argv)
{
    gsm_app_connect_to_http_server(argv[1]);
    return 0;
}

int32_t  cli_ussd_money                (p_shell_context_t context, int32_t argc, char **argv)
{
    char money_response[128];
    gsm_ussd_run(argv[1], money_response, sizeof(money_response), NULL, NULL, 1);
    return 0;
}

static int32_t cli_trace_task_stack          (p_shell_context_t context, int32_t argc, char **argv)
{
    return 1;
}


int32_t cli_get_wlan_mac_addr       (p_shell_context_t context, int32_t argc, char **argv)
{
    return 1;
}


static int32_t  cli_factory_reset               (p_shell_context_t context, int32_t argc, char **argv)
{
    return 1;
}


static int32_t cli_get_rssi(p_shell_context_t context, int32_t argc, char** argv)
{
    int16_t rssi;
    gsm_network_rssi(&rssi, NULL, NULL, 1);
    printf("RSSI %d\r\n", rssi);
    return 1;
}

static int32_t cli_send_sms(p_shell_context_t context, int32_t argc, char** argv)
{
    gsm_sms_send(argv[1], argv[2], NULL, NULL, 1);
    return 1;
}

static int32_t cli_call(p_shell_context_t context, int32_t argc, char** argv)
{
    call_sms_start();
    gsm_call_start(argv[1], NULL, NULL, 1);
    return 0;
}

static int32_t cli_config_sim(p_shell_context_t context, int32_t argc, char** argv)
{
    if (configure_sim_card() == 0)
    {
        printf("Config simcard error\r\n");
    }
    return 1;
}