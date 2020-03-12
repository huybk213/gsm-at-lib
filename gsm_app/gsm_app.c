#include "stdafx.h"
#include "gsm_app.h"
#include "stdbool.h"
#include "vsm_debug.h"
#include "gsm/gsm_bearer.h"
#include "gsm/gsm_location_time.h"

#define HEADER_BAK_LEN 32
#define HTTP_READ_BUFFER_SIZE 1024

typedef enum
{
    HTTP_PARSE_ERROR = -1,
    HTTP_ONLY_GOT_STATUS_CODE = 0x01,
    HTTP_ONLY_GOT_STATUS_CODE_AND_CONTENT_LENGTH_BUT_NOT_GET_FULL_HEADER = 0x03,
    HTTP_PARSE_SUCESS = 0x04
} http_parse_state_t;

typedef struct {
    uint32_t	status_code;
    uint32_t	header_len;
    uint8_t* body;
    uint32_t	body_len;
    uint8_t* header_bak;
    uint32_t	parse_status;
} http_response_result_t;

#define HEADER_BAK_LEN 32
#define HTTP_PARSE_ERROR (-1)
#define HTTP_ONLY_GOT_STATUS_CODE 1
#define HTTP_ONLY_GOT_STATUS_CODE_AND_CONTENT_LENGTH_BUT_NOT_GET_FULL_HEADER 3
#define HTTP_PARSE_SUCESS 4

void* http_malloc(unsigned int size)
{
    return malloc(size);
}

void http_free(void* buf)
{
    free(buf);
}

#if 0		//not support redirect
static char* redirect = NULL;
static int redirect_len;
static uint16_t redirect_server_port;
static char* redirect_server_host = NULL;
static char* redirect_resource = NULL;
#endif

/******************************************************************************************************************
** Function Name  : _parse_http_response
** Description    : Parse the http response to get some useful parameters
** Input          : response	: The http response got from server
**					response_len: The length of http response
**					result		: The struct that store the usful infor about the http response
** Return         : Parse OK:	1 -> Only got the status code
**								3 -> Got the status code and content_length, but didn't get the full header
**								4 -> Got all the information needed
**					Failed:		-1
*******************************************************************************************************************/

static int _parse_http_response(uint8_t* response, uint32_t response_len, http_response_result_t* result, uint8_t** remain_data, uint32_t* remain_size)
{
    uint32_t i, p, q, m;
    uint32_t header_end = 0;

    //Get status code
    if (0 == result->parse_status)
    {//didn't get the http response
        uint8_t status[4] = { 0 };
        i = p = q = m = 0;
        for (; i < response_len; ++i)
        {
            if (' ' == response[i])
            {
                ++m;
                if (1 == m)
                {//after HTTP/1.1
                    p = i;
                }
                else if (2 == m)
                {//after status code
                    q = i;
                    break;
                }
            }
        }
        if (!p || !q || q - p != 4)
        {//Didn't get the status code
            return HTTP_PARSE_ERROR;
        }

        memcpy(status, response + p + 1, 3);//get the status code
        result->status_code = atoi((char const*)status);
        if (result->status_code == 200)
            result->parse_status = HTTP_ONLY_GOT_STATUS_CODE;
        else if (result->status_code == 302)
        {
#if 0       // not support redirect
            char* tmp = NULL;
            const uint8_t* location1 = (uint8_t*)"LOCATION";
            const uint8_t* location2 = (uint8_t*)"Location";
            printf("response 302:%s \r\n", response);

            if ((tmp = strstr((char const*)response, (char const*)location1)) || (tmp = strstr((char const*)response, (char const*)location2)))
            {
                redirect_len = strlen(tmp + 10);
                printf("Location len = %d\r\n", redirect_len);
                if (redirect == NULL)
                {
                    redirect = http_malloc(redirect_len);
                    if (redirect == NULL)
                    {
                        return HTTP_PARSE_ERROR;
                    }
                }
                memset(redirect, 0, redirect_len);
                memcpy(redirect, tmp + 10, strlen(tmp + 10));
            }

            if (redirect_server_host == NULL)
            {
                redirect_server_host = http_malloc(redirect_len);
                if (redirect_server_host == NULL)
                {
                    return HTTP_PARSE_ERROR;
                }
            }

            if (redirect_resource == NULL)
            {
                redirect_resource = http_malloc(redirect_len);
                if (redirect_resource == NULL)
                {
                    return HTTP_PARSE_ERROR;
                }
            }

            memset(redirect_server_host, 0, redirect_len);
            memset(redirect_resource, 0, redirect_len);
            if (parser_url(redirect, redirect_server_host, &redirect_server_port, redirect_resource) < 0)
            {
                return HTTP_PARSE_ERROR;
            }
            return HTTP_PARSE_ERROR;
#else
            printf("Not support redirect url\r\n");
            return HTTP_PARSE_ERROR;
#endif
        }
        else
        {
            printf("\r\n[%s] The http response status code is %d", __func__, result->status_code);
            return HTTP_PARSE_ERROR;
        }
    }

    //if didn't receive the full http header
    if (HTTP_ONLY_GOT_STATUS_CODE_AND_CONTENT_LENGTH_BUT_NOT_GET_FULL_HEADER == result->parse_status)
    {//didn't get the http response
        p = q = 0;
        for (i = 0; i < response_len; ++i) {
            if (response[i] == '\r' && response[i + 1] == '\n' &&
                response[i + 2] == '\r' && response[i + 3] == '\n')
            {//the end of header
                header_end = i + 4;
                result->parse_status = HTTP_PARSE_SUCESS;
                result->header_len = header_end;
                result->body = response + header_end;
                break;
            }
        }
        if (HTTP_ONLY_GOT_STATUS_CODE_AND_CONTENT_LENGTH_BUT_NOT_GET_FULL_HEADER == result->parse_status)
        {//Still didn't receive the full header	
            result->header_bak = http_malloc(HEADER_BAK_LEN + 1);
            memset(result->header_bak, 0, strlen((char const*)result->header_bak));
            memcpy(result->header_bak, response + response_len - HEADER_BAK_LEN, HEADER_BAK_LEN);
        }
    }

    //Get Content-Length
    if (HTTP_ONLY_GOT_STATUS_CODE == result->parse_status)
    {//didn't get the content length
        const uint8_t* content_length_buf1 = (uint8_t*)"CONTENT-LENGTH";
        const uint8_t* content_length_buf2 = (uint8_t*)"Content-Length";
        const uint32_t content_length_buf_len = strlen((char const*)content_length_buf1);
        p = q = 0;

        for (i = 0; i < response_len; ++i)
        {
            if (response[i] == '\r' && response[i + 1] == '\n')
            {
                q = i;//the end of the line
                if (!memcmp(response + p, content_length_buf1, content_length_buf_len) ||
                    !memcmp(response + p, content_length_buf2, content_length_buf_len))
                {//get the content length
                    int j1 = p + content_length_buf_len, j2 = q - 1;
                    while (j1 < q && (*(response + j1) == ':' || *(response + j1) == ' ')) ++j1;
                    while (j2 > j1&&* (response + j2) == ' ') --j2;
                    uint8_t len_buf[12] = { 0 };
                    memcpy(len_buf, response + j1, j2 - j1 + 1);
                    result->body_len = atoi((char const*)len_buf);
                    result->parse_status = 2;
                }
                p = i + 2;
            }
            if (response[i] == '\r' && response[i + 1] == '\n' &&
                response[i + 2] == '\r' && response[i + 3] == '\n')
            {//Get the end of header
                header_end = i + 4;//p is the start of the body
                if (result->parse_status == 2)
                {//get the full header and the content length
                    result->parse_status = HTTP_PARSE_SUCESS;
                    result->header_len = header_end;
                    result->body = response + header_end;
                    *remain_size = response_len - i - 4;
                    // memcpy(remain_data, response + i + 4, *remain_size);
                    *remain_data = response + i + 4;
                }
                else
                {//there are no content length in header	
                    printf("\r\n[%s] No Content-Length in header", __func__);
                    return HTTP_PARSE_ERROR;
                }
                break;
            }
        }

        if (HTTP_ONLY_GOT_STATUS_CODE == result->parse_status)
        {//didn't get the content length and the full header
            result->header_bak = http_malloc(HEADER_BAK_LEN + 1);
            memset(result->header_bak, 0, strlen((char const*)result->header_bak));
            memcpy(result->header_bak, response + response_len - HEADER_BAK_LEN, HEADER_BAK_LEN);
        }
        else if (2 == result->parse_status)
        {//didn't get the full header but get the content length
            result->parse_status = HTTP_ONLY_GOT_STATUS_CODE_AND_CONTENT_LENGTH_BUT_NOT_GET_FULL_HEADER;
            result->header_bak = http_malloc(HEADER_BAK_LEN + 1);
            memset(result->header_bak, 0, strlen((char const*)result->header_bak));
            memcpy(result->header_bak, response + response_len - HEADER_BAK_LEN, HEADER_BAK_LEN);
        }
    }

    return result->parse_status;
}

/* Convert
    url : http://nguyentrongbang9x.s3-ap-southeast-1.amazonaws.com:80/cmd.txt

    To : server : nguyentrongbang9x.s3-ap-southeast-1,
    port 80 (if has),
    resource /cmd.txt
*/

static bool gsm_parse_http_url(char* url, char* server, char* resource, uint32_t* port)
{
#define VSM_HTTPCLIENT_MAX_CONNECT_URL 128      // only for dev
#define VSM_HTTPCLIENT_MAX_FILE_LEN 128 // only for dev

    if (url == NULL || server == NULL || resource == NULL || strlen(url) == 0)
    {
        return 0;
    }

    if (strstr(url, "https://"))
    {
        printf("Secure https\r\n");
        url += 8;       // Not verifed https server
    }
    else if (strstr(url, "http://"))
    {
        printf("Normal http\r\n");
        url += 7;
    }

    uint32_t len_url = strlen(url);
    uint32_t i, count = 0;;
    for (i = 0; i < len_url; i++)
    {
        if (*(url + i) == '/') count++;
        if (count == 1)
        {
            if (i > VSM_HTTPCLIENT_MAX_CONNECT_URL)
            {
                printf("Connect url too long, only support %d byte\r\n", VSM_HTTPCLIENT_MAX_CONNECT_URL);
                return 0;
            }

            uint32_t len_file = len_url - i;

            if (len_file > VSM_HTTPCLIENT_MAX_FILE_LEN)
            {
                printf("File path too long, only support %d byte\r\n", VSM_HTTPCLIENT_MAX_FILE_LEN);
                return 0;
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

            printf("Connect url %s, file %s, port(%s) %d\r\n", server, resource, p, *port);
            return 1;
        }
    }
    return 0;
}

/**
 * \brief           Downlading file via HTTP
 * \param[in]       url: file url
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t gsm_app_connect_to_http_server(char* url)  // ssl not supported 
{
    if (url == NULL || strlen(url) == 0)
    {
        printf("Invalid parameter\r\n");
        return gsmERR;
    }

    if (strstr(url, "https"))
    {
        printf("Currently not supported https\r\n");
        return gsmERR;
    }
    http_response_result_t rsp_result = { 0 };
    uint32_t port = 80;     // default http port is 80
    char domain[128], resource[128];
    char* buf = NULL;
    uint32_t total_recv = 0;

    gsmr_t res = gsmERR;
    gsm_pbuf_p pbuf;

    gsm_netconn_p client = NULL;
    //gsm_sys_sem_t* sem = (void*)arg; 
    gsm_sys_sem_t* sem = NULL;

    if (gsm_parse_http_url(url, domain, resource, &port) == 0)
    {
        printf("Parse connect url %s err\r\n", url);
        return gsmERR;
    }

    char* request = (unsigned char*)malloc(HTTP_READ_BUFFER_SIZE);      // pvPortMalloc
    char* cp = (char*)request;
    if (cp == NULL)
    {
        printf("No memory\r\n");
        return gsmERRMEM;
    }
    //if (!buf)
    //{
    //    printf("\r\n[%s] Alloc buffer failed", __func__);
    //    goto http_exit;
    //}

    cp += sprintf(cp, "GET %s HTTP/1.1\r\n", resource);
    cp += sprintf(cp, "Host: %s\r\n", domain);
    cp += sprintf(cp, "Accept: text/html, */*\r\n");
    cp += sprintf(cp, "User-Agent: GSM Client\r\n");
    cp += sprintf(cp, "Connection: keep-alive\r\n");
    cp += sprintf(cp, "Cache-control: no-cache\r\n");
    cp += sprintf(cp, "\r\n");

    printf("HTTP header %s\r\n", request);

    /* Request attach to network */
    printf("%s : Request attach to network\r\n", __func__);
    while (gsm_network_request_attach() != gsmOK) {
        //printf(".");
        gsm_delay(1000);
    }
    printf("Network attack success\r\n");
    char* remain_buf = NULL;
    uint32_t body_len = 0;
    /*
     * First create a new instance of netconn
     * connection and initialize system message boxes
     * to accept received packet buffers
     */
    client = gsm_netconn_new(GSM_NETCONN_TYPE_TCP);
    if (client != NULL) {
        /*
         * Connect to external server as client
         * with custom NETCONN_CONN_HOST and CONN_PORT values
         *
         * Function will block thread until we are successfully connected (or not) to server
         */
        res = gsm_netconn_connect(client, domain, port);
        if (res == gsmOK)
        {                     /* Are we successfully connected? */
            printf("Connected to %s, port %d\r\n", domain, port);

            res = gsm_netconn_write(client, request, strlen(request));    /* Send data to server */
            if (request)
            {
                http_free(request);
                request = NULL;
            }

            if (res == gsmOK)
            {
                res = gsm_netconn_flush(client);/* Flush data to output */
            }
            if (res == gsmOK)
            {                 /* Were data sent? */
                printf("Data were successfully sent to server\r\n");

                buf = http_malloc(HTTP_READ_BUFFER_SIZE);
                if (!buf)
                {
                    printf("\r\n[%s] Alloc buffer failed", __func__);
                    goto http_exit;
                }

                // remain_buf = malloc(HTTP_READ_BUFFER_SIZE);
                // if (!remain_buf) 
                // {
                //     printf("\r\n[%s] Alloc buffer failed", __FUNCTION__);
                //     goto http_exit;
                // }

                /*
                 * Since we sent HTTP request,
                 * we are expecting some data from server
                 * or at least forced connection close from remote side
                 */
                uint32_t remain_byte_after_parse_header = 0;
                printf("Connected\r\n");
                uint32_t idx = 0;
                int data_len = 0;
                int read_bytes = 0;
                bool exit = 0;
                while (HTTP_ONLY_GOT_STATUS_CODE_AND_CONTENT_LENGTH_BUT_NOT_GET_FULL_HEADER >= rsp_result.parse_status)
                {//still read header
                    if (0 == rsp_result.parse_status)
                    {
                        //didn't get the http response
                        memset(buf, 0, HTTP_READ_BUFFER_SIZE);
                        exit = 0;
                        res = gsm_netconn_receive(client, &pbuf);
                        if (res == gsmCLOSED)
                        {     /* Was the connection closed? This can be checked by return status of receive function */
                            printf("Connection closed by remote side...\r\n");
                            exit = 1;
                        }
                        else if (res == gsmTIMEOUT)
                        {
                            printf("Netconn timeout while receiving data. You may try multiple readings before deciding to close manually\r\n");
                            exit = 1;
                        }
                        if (exit == 1)
                        {
                            goto http_exit;
                        }

                        read_bytes = (int)gsm_pbuf_length(pbuf, 1);
                        char* p_data = (char*)gsm_pbuf_data(pbuf);
                        memcpy(buf, p_data, read_bytes);
                        gsm_pbuf_free(pbuf);    /* Free the memory after usage */


                        idx = read_bytes;
                        memset(&rsp_result, 0, sizeof(rsp_result));

                        if (_parse_http_response(buf, idx, &rsp_result, &remain_buf, &remain_byte_after_parse_header) == HTTP_PARSE_ERROR)
                        {
                            goto http_exit;
                        }
                    }
                    else if ((HTTP_ONLY_GOT_STATUS_CODE == rsp_result.parse_status) || (HTTP_ONLY_GOT_STATUS_CODE_AND_CONTENT_LENGTH_BUT_NOT_GET_FULL_HEADER == rsp_result.parse_status))
                    {//just get the status code
                        memset(buf, 0, HTTP_READ_BUFFER_SIZE);
                        memcpy(buf, rsp_result.header_bak, HEADER_BAK_LEN);
                        http_free(rsp_result.header_bak);
                        rsp_result.header_bak = NULL;

                        exit = 0;
                        res = gsm_netconn_receive(client, &pbuf);
                        if (res == gsmCLOSED)
                        {     /* Was the connection closed? This can be checked by return status of receive function */
                            printf("Connection closed by remote side...\r\n");
                            exit = 1;
                        }
                        else if (res == gsmTIMEOUT)
                        {
                            printf("Netconn timeout while receiving data. You may try multiple readings before deciding to close manually\r\n");
                            exit = 1;
                        }

                        if (exit)
                        {
                            printf("\r\n[%s] Read socket failed", __func__);
                            goto http_exit;
                        }

                        read_bytes = (int)gsm_pbuf_length(pbuf, 1);
                        char* p_data = (char*)gsm_pbuf_data(pbuf);
                        memcpy(buf + HEADER_BAK_LEN, p_data, read_bytes);
                        gsm_pbuf_free(pbuf);    /* Free the memory after usage */

                        idx = read_bytes + HEADER_BAK_LEN;

                        if (_parse_http_response(buf, read_bytes + HEADER_BAK_LEN, &rsp_result, &remain_buf, &remain_byte_after_parse_header) == HTTP_PARSE_ERROR)
                        {
                            printf("Parse http response error\r\n");
                            goto http_exit;
                        }
                    }
                }

                if (0 == rsp_result.body_len)
                {
                    printf("\r\n[%s] Http body size = 0 !\r\n", __func__);
                    goto http_exit;
                }

                body_len = rsp_result.body_len;
                printf("\r\n[%s] File size %d\r\n", __func__, body_len);

                // if (remain_byte_after_parse_header)
                // {
                //     printf("Fw binary header ");
                //     for (uint32_t i = 0; i < 4; i++)     // debug
                //     {
                //         printf("0x%02X ", remain_buf[i]);
                //     }
                //     printf("\r\n");
                // }
                total_recv = remain_byte_after_parse_header;

                if (total_recv)
                {
                    printf("Data %s\r\n", remain_buf);
                    // http_free(remain_buf);
                    // remain_buf = NULL;
                }

                do {
                    /*
                     * Receive single packet of data
                     *
                     * Function will block thread until new packet
                     * is ready to be read from remote side
                     *
                     * After function returns, don't forgot the check value.
                     * Returned status will give you info in case connection
                     * was closed too early from remote side
                     */
                    res = gsm_netconn_receive(client, &pbuf);
                    if (res == gsmCLOSED)
                    {     /* Was the connection closed? This can be checked by return status of receive function */
                        printf("Connection closed by remote side...\r\n");
                        break;
                    }
                    else if (res == gsmTIMEOUT)
                    {
                        printf("Netconn timeout while receiving data. You may try multiple readings before deciding to close manually\r\n");
                        break;
                    }

                    if (res == gsmOK && pbuf != NULL)
                    { /* Make sure we have valid packet buffer */
                        /*
                         * At this point read and manipulate
                         * with received buffer and check if you expect more data
                         *
                         * After you are done using it, it is important
                         * you free the memory otherwise memory leaks will appear
                         */

                        total_recv += (int)gsm_pbuf_length(pbuf, 1);
                        printf("\r\n\r\nTotal byte received %d bytes\r\n\r\n%s\r\n\r\n", total_recv, (char*)gsm_pbuf_data(pbuf));
                        gsm_pbuf_free(pbuf);    /* Free the memory after usage */
                        pbuf = NULL;
                    }
                    gsm_delay(10);  // Release time for another task process
                } while (total_recv < body_len);
            }
            else
            {
                printf("Error writing data to remote host!\r\n");
            }
            if (total_recv == body_len)
            {
                printf("HTTP download success\r\n");
                res = gsmOK;
            }
        }
        else
        {
            printf("Cannot connect to remote host %s:%d\r\n", domain, port);
        }
    }
http_exit:
    if (client) gsm_netconn_close(client);
    client = NULL;

    // if (remain_buf) http_free(remain_buf);
    if (buf) http_free(buf);
    if (request) free(request);

    gsm_network_request_detach();

    if (gsm_sys_sem_isvalid(sem))
    {
        gsm_sys_sem_release(sem);
    }
    return res;
}

//static void _location_and_time_evt(gsmr_t res, void* arg)
//{
//    gsm_msg_t* evt = (gsm_msg_t*)arg;
//    if (evt->res == gsmOK)
//    {
//        printf("Time %d/%02d/%02d %d:%02d:%02d\r\nLongitude %s\r\nLatiude %s\r\n",
//            evt->msg.location_and_time.date_time.year,
//            evt->msg.location_and_time.date_time.month,
//            evt->msg.location_and_time.date_time.date,
//            evt->msg.location_and_time.date_time.hours,
//            evt->msg.location_and_time.date_time.minutes,
//            evt->msg.location_and_time.date_time.seconds,
//            evt->msg.location_and_time.location.longitude,
//            evt->msg.location_and_time.location.latitude
//        );
//    }
//    else
//    {
//        printf("Get location and time error %d\r\n", evt->msg.location_and_time.err_code);
//    }
//}

/**
 * \brief           Get current date and time over gsm
 * \param[in]       time: time variable
 * \return          \ref gsmOK on success, member of \ref gsmr_t otherwise
 */
gsmr_t gsm_app_get_time(gsm_datetime_t* time)
{
    if (time == NULL)
    {
        return 0;
    }

    gsmr_t res = gsmERR;
    res = gsm_bearer_open(NULL, NULL, 1, 1);
    
    //if(res != gsmOK)
    //{
    //    printf("gsm_bearer_open error\r\n");
    //    goto exit;
    //}

    gsm_msg_t gsm_msg;
    memset(&gsm_msg, 0, sizeof(gsm_msg_t));
    res = gsm_location_time_get(NULL, &gsm_msg, 1);
    if (res != gsmOK || gsm_msg.msg.location_and_time.err_code != 0)
    {
        printf("gsm_location_time_get error code %d\r\n", gsm_msg.msg.location_and_time.err_code);
        goto exit;
    }

    memcpy(time, &gsm_msg.msg.location_and_time.date_time, sizeof(gsm_datetime_t));
    res = gsmOK;

    exit:
    return res;
}
