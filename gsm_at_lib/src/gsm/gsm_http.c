/**
 * \file            gsm_http.c
 * \brief           HTTP API
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
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         $_version_$
 */
#include "gsm/gsm_private.h"
#include "gsm/gsm_http.h"
#include "gsm/gsm_mem.h"
#include "stdafx.h"
#include "stdbool.h"

#if GSM_CFG_HTTP || __DOXYGEN__

#define GSM_DEFAULT_HTTP_PORT 80
#define GSM_DEFAULT_HTTPS_PORT 443
#define GSM_HTTPCLIENT_MAX_CONNECT_URL 128      // only for dev
#define GSM_HTTPCLIENT_MAX_FILE_LEN 128 // only for dev
/**
 * @brief      Start a HTTP session
 *             This function must be the first function to call,
 *             and it returns a esp_http_client_handle_t that you must use as input to other functions in the interface.
 *             This call MUST have a corresponding call to esp_http_client_cleanup when the operation is complete.
 *
 * @param[in]  config   The configurations, see `http_client_config_t`
 *
 * @return
 *     - `esp_http_client_handle_t`
 *     - NULL if any errors
 */

 /* Convert
     url : http://nguyentrongbang9x.s3-ap-southeast-1.amazonaws.com:80/cmd.txt

     To : server : nguyentrongbang9x.s3-ap-southeast-1,
     port 80 (if has),
     resource /cmd.txt
 */


 /**
  * \brief           Parse http url to server, resouce file/path and port
  * \param[in]       url: file url
  * \param[in]       server: buffer for server address or domain
  * \param[in]       resource: buffer for file path
  * \param[in]       port: server port address
  * \param[in]       is_https : http type
  * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
  */

static gsmr_t gsm_parse_http_url(char* url, char* server, char* resource, uint32_t* port, bool * is_https)
{

    if (url == NULL || server == NULL || resource == NULL || strlen(url) == 0)
    {
        return gsmERR;
    }

    if (strstr(url, "https://"))
    {
        printf("Secure https\r\n");
        url += 8;       // Not verifed https server
        *is_https = 1;
    }
    else if (strstr(url, "http://"))
    {
        printf("Normal http\r\n");
        url += 7;
        *is_https = 0;
    }

    uint32_t len_url = strlen(url);
    uint32_t i, count = 0;;
    for (i = 0; i < len_url; i++)
    {
        if (*(url + i) == '/') count++;
        if (count == 1)
        {
            if (i > GSM_HTTPCLIENT_MAX_CONNECT_URL)
            {
                printf("Connect url too long, only support %d byte\r\n", GSM_HTTPCLIENT_MAX_CONNECT_URL);
                return gsmERR;
            }

            uint32_t len_file = len_url - i;

            if (len_file > GSM_HTTPCLIENT_MAX_FILE_LEN)
            {
                printf("File path too long, only support %d byte\r\n", GSM_HTTPCLIENT_MAX_FILE_LEN);
                return gsmERR;
            }

            memcpy(server, url, i);
            server[i] = '\0';
            memcpy(resource, url + i, len_file);
            resource[len_file] = '\0';

            /* Get port */
            char* p = server;
            while (*p != ':' && *p) p++;
            if (*p)
            {
                server[strlen(server) - strlen(p)] = '\0';
                p++;
                *port = (int)atoi(p);
            }
            else
            {
                if (*is_https == 0) port = GSM_DEFAULT_HTTP_PORT;
                else  port = GSM_DEFAULT_HTTPS_PORT;
            }

            return gsmOK;
        }
    }
    return gsmERR;
}

gsmr_t esp_http_client_connect(esp_http_client_config_t* config)
{
#ifdef MODULE_SIM_SUPPORT_NATIVE_HTTP
    if (config == NULL || config->url == NULL || strlen(config->url) == 0)
    {
        printf("Invalid param");
        return gsmERR;
    }

    char server[GSM_HTTPCLIENT_MAX_CONNECT_URL];
    char resource[GSM_HTTPCLIENT_MAX_FILE_LEN];
    bool is_https = 0;
    if (gsmOK != gsm_parse_http_url(config->url, server, resource, config->port, &is_https))
    {
        printf("Parse url error\r\n");
        return gsmERR;
    }

    printf("Connect url %s, file %s, port %d\r\n", server, resource, config->port);

    switch (config->method)
    {
    case HTTP_METHOD_GET:
    {
        break;
    }

    default:
        printf("Invalid method, implement later\r\n");
        break;
    }

#else
    printf("Module SIM8(9)00 does not support native HTTP(s). You must build http client base on TCP/UDP socket\r\n");
    return gsmERR;
#endif
}

#endif /* GSM_CFG_HTTP || __DOXYGEN__ */
