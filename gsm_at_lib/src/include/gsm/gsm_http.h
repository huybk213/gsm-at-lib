/**
 * \file            gsm_http.h
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
#ifndef GSM_HDR_HTTP_H
#define GSM_HDR_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gsm/gsm.h"
#include "stdbool.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_HTTP Hyper Text Transfer Protocol
 * \brief           Hyper Text Transfer Protocol (HTTP) manager
 * \{
 *
 * Currently it is under development
 *
 */

/**
 * \}
 */

#define DEFAULT_HTTP_BUF_SIZE (512)

typedef struct esp_http_client* esp_http_client_handle_t;
typedef struct esp_http_client_event* esp_http_client_event_handle_t;

typedef enum {
    HTTP_AUTH_TYPE_NONE = 0,    /*!< No authention */
    HTTP_AUTH_TYPE_BASIC,       /*!< HTTP Basic authentication */
    HTTP_AUTH_TYPE_DIGEST,      /*!< HTTP Disgest authentication */
} esp_http_client_auth_type_t;

/**
    * @brief HTTP method
    */
typedef enum {
    HTTP_METHOD_GET = 0,    /*!< HTTP GET Method */
    HTTP_METHOD_POST,       /*!< HTTP POST Method */
    HTTP_METHOD_PUT,        /*!< HTTP PUT Method */
    HTTP_METHOD_PATCH,      /*!< HTTP PATCH Method */
    HTTP_METHOD_DELETE,     /*!< HTTP DELETE Method */
    HTTP_METHOD_HEAD,       /*!< HTTP HEAD Method */
    HTTP_METHOD_NOTIFY,     /*!< HTTP NOTIFY Method */
    HTTP_METHOD_SUBSCRIBE,  /*!< HTTP SUBSCRIBE Method */
    HTTP_METHOD_UNSUBSCRIBE,/*!< HTTP UNSUBSCRIBE Method */
    HTTP_METHOD_OPTIONS,    /*!< HTTP OPTIONS Method */
    HTTP_METHOD_MAX,
} esp_http_client_method_t;


/**
    * @brief   HTTP Client events id
    */
typedef enum {
    HTTP_EVENT_ERROR = 0,       /*!< This event occurs when there are any errors during execution */
    HTTP_EVENT_ON_CONNECTED,    /*!< Once the HTTP has been connected to the server, no data exchange has been performed */
    HTTP_EVENT_HEADER_SENT,     /*!< After sending all the headers to the server */
    HTTP_EVENT_ON_HEADER,       /*!< Occurs when receiving each header sent from the server */
    HTTP_EVENT_ON_DATA,         /*!< Occurs when receiving data from the server, possibly multiple portions of the packet */
    HTTP_EVENT_ON_FINISH,       /*!< Occurs when finish a HTTP session */
    HTTP_EVENT_DISCONNECTED,    /*!< The connection has been disconnected */
} esp_http_client_event_id_t;

/**
    * @brief      HTTP Client events data
    */
typedef struct esp_http_client_event {
    esp_http_client_event_id_t event_id;    /*!< event_id, to know the cause of the event */
    void* data;                             /*!< data of the event */
    int data_len;                           /*!< data length of data */
    void* user_data;                        /*!< user_data context, from esp_http_client_config_t user_data */
    char* header_key;                       /*!< For HTTP_EVENT_ON_HEADER event_id, it's store current http header key */
    char* header_value;                     /*!< For HTTP_EVENT_ON_HEADER event_id, it's store current http header value */
} esp_http_client_event_t;

/**
    * @brief      HTTP Client transport
    */
typedef enum {
    HTTP_TRANSPORT_UNKNOWN = 0x0,   /*!< Unknown */
    HTTP_TRANSPORT_OVER_HTTP,        /*!< Transport over tcp */
    HTTP_TRANSPORT_OVER_HTTPS,        /*!< Transport over ssl */
} esp_http_client_transport_t;

typedef bool (*http_event_handle_cb)(esp_http_client_event_t* evt);

typedef struct {
    const char* url;                /*!< HTTP URL, the information on the URL is most important, it overrides the other fields below, if any */
    const char* host;               /*!< Domain or IP as string */
    int port;                /*!< Port to connect, default depend on esp_http_client_transport_t (80 or 443) */
    const char* username;           /*!< Using for Http authentication */
    const char* password;           /*!< Using for Http authentication */
    //esp_http_client_auth_type_t auth_type;           /*!< Http authentication type, see `esp_http_client_auth_type_t` */

    const char* cert_pem;           /*!< SSL server certification, PEM format as string, if the client requires to verify server */
    const char* client_cert_pem;    /*!< SSL client certification, PEM format as string, if the server requires to verify client */
    const char* client_key_pem;     /*!< SSL client key, PEM format as string, if the server requires to verify client */
    esp_http_client_method_t    method;                   /*!< HTTP Method */
    int                         timeout_ms;               /*!< Network timeout in milliseconds */
    //bool                        disable_auto_redirect;    /*!< Disable HTTP automatic redirects */
    http_event_handle_cb        event_handler;             /*!< HTTP Event Handle */
    //esp_http_client_transport_t transport_type;           /*!< HTTP transport type, see `esp_http_client_transport_t` */
    //int                         buffer_size;              /*!< HTTP buffer size (both send and receive) */
    void* user_data;               /*!< HTTP user_data context */
} esp_http_client_config_t;

/**
    * Enum for the HTTP status codes.
    */
typedef enum {
    /* 3xx - Redirection */
    HttpStatus_MovedPermanently = 301,
    HttpStatus_Found = 302,

    /* 4xx - Client Error */
    HttpStatus_Unauthorized = 401
} HttpStatus_Code;

#define ESP_ERR_HTTP_BASE               (0x7000)                    /*!< Starting number of HTTP error codes */
#define ESP_ERR_HTTP_MAX_REDIRECT       (ESP_ERR_HTTP_BASE + 1)     /*!< The error exceeds the number of HTTP redirects */
#define ESP_ERR_HTTP_CONNECT            (ESP_ERR_HTTP_BASE + 2)     /*!< Error open the HTTP connection */
#define ESP_ERR_HTTP_WRITE_DATA         (ESP_ERR_HTTP_BASE + 3)     /*!< Error write HTTP data */
#define ESP_ERR_HTTP_FETCH_HEADER       (ESP_ERR_HTTP_BASE + 4)     /*!< Error read HTTP header from server */
#define ESP_ERR_HTTP_INVALID_TRANSPORT  (ESP_ERR_HTTP_BASE + 5)     /*!< There are no transport support for the input scheme */
#define ESP_ERR_HTTP_CONNECTING         (ESP_ERR_HTTP_BASE + 6)     /*!< HTTP connection hasn't been established yet */
#define ESP_ERR_HTTP_EAGAIN             (ESP_ERR_HTTP_BASE + 7)     /*!< Mapping of errno EAGAIN to esp_err_t */

gsmr_t esp_http_client_connect(esp_http_client_config_t* config);

#ifdef __cplusplus
}
#endif

#endif /* GSM_HDR_HTTP_H */
