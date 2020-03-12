#include "gsm/gsm_bearer.h"
#include "gsm/gsm_private.h"
#include "gsm/gsm_mem.h"
#include "stdafx.h"

gsmr_t gsm_bearer_open(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking, uint8_t cid)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_SAPBR;
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SAPBR_OPEN;
    GSM_MSG_VAR_REF(msg).msg.bearer_param.cid = cid;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

gsmr_t gsm_bearer_close(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking, uint8_t cid)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_SAPBR;
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SAPBR_CLOSE;
    GSM_MSG_VAR_REF(msg).msg.bearer_param.cid = cid;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}

gsmr_t gsm_bearer_query(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking, uint8_t cid)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd = GSM_CMD_SAPBR;
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SAPBR_QUERY;
    GSM_MSG_VAR_REF(msg).msg.bearer_param.cid = cid;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 10000);
}
