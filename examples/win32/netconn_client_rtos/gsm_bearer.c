#include "gsm_bearer.h"
#include "gsm/gsm_private.h"
#include "gsm/gsm_mem.h"
#include "stdafx.h"

gsmr_t gsm_bearer_open(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_SAPBR;
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SAPBR_OPEN;
    GSM_MSG_VAR_REF(msg).msg.bearer_param.cmd_type = 1;
    GSM_MSG_VAR_REF(msg).msg.bearer_param.cid = 1;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

gsmr_t gsm_bearer_close(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{

}