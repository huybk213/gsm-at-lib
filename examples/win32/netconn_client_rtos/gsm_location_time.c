#include "gsm_location_time.h"
#include "gsm/gsm_private.h"
#include "gsm/gsm_mem.h"
#include "stdafx.h"

gsmr_t gsm_location_time_get(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{
    /* You must call gsm_bearer_open */
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_LOCATION_TIME_GET;
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_IDLE;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}
