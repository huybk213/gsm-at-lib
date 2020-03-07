/*
 * debug.c
 *
 *  Created on: Oct 16, 2017
 *      Author: manhbt
 */


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#include "vsm_debug.h"
#include "stdafx.h"

void vsm_debug_nothing(const char* sz, ...){

}


void vsm_debug_dump(const void* data, int len, const char* string, ...)
{
    uint8_t* p = (uint8_t*)data;
    uint8_t  buffer[16];
    int iLen;
    int i;

    printf("%s %u bytes\n", string, len);
    while (len > 0)
    {
        iLen = (len > 16) ? 16 : len;
        memset(buffer, 0, 16);
        memcpy(buffer, p, iLen);
        for (i = 0; i < 16; i++)
        {
            if (i < iLen)
                printf("%02X ", buffer[i]);
            else
                printf("   ");
        }
        printf("\t");
        for (i = 0; i < 16; i++)
        {
            if (i < iLen)
            {
                if (isprint(buffer[i]))
                    printf("%c", (char)buffer[i]);
                else
                    printf(".");
            }
            else
                printf(" ");
        }
        printf("\n");
        len -= iLen;
        p += iLen;
    }
}






