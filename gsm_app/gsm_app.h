#pragma once
#include "stdafx.h"
#include "netconn_client.h"
#include "gsm/gsm.h"
#include "gsm/gsm_network_api.h"
#include "gsm/gsm_private.h"
#include "stdafx.h"

gsmr_t gsm_app_connect_to_http_server(char* url);
gsmr_t gsm_app_get_time(gsm_datetime_t* time);