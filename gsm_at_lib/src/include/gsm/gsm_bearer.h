#pragma once
#include "gsm/gsm.h"

gsmr_t gsm_bearer_open(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking, uint8_t cid);