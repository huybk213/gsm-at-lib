/*
 * debug.h
 *
 *  Created on: Oct 16, 2017
 *      Author: manhbt
 */

#ifndef VSM_DEBUG_H_
#define VSM_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdint.h"
#include "string.h"

#define VSM_DEBUG_DISABLE                0
#define VSM_DEBUG_ERROR_DISABLE          0
#define VSM_DEBUG_RAW_DISABLE            0
#define VSM_DEBUG_WARN_DISABLE           0
#define VSM_DEBUG_INFO_DISABLE           0


#undef VSM_DEBUG    
#undef VSM_DEBUG_ERROR
#undef VSM_DEBUG_WARNING

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void vsm_debug_nothing(const char* sz, ...);
// void vsm_debug_print(const char* string, ...);
#define vsm_debug_print         qcom_printf
// void vsm_debug_print_raw(const char* string, ...);
void vsm_debug_dump(const void* data, int len, const char* string, ...);


#define xTaskGetTickCount() 0

#define VSM_DEBUG_DUMP                           vsm_debug_dump

#define VSM_DEBUG_RAW(s,...)                printf(s)
#define VSM_DEBUG(s,...)                    printf(KWHT "%d [D] %s::%d " s "\r\n" KNRM, xTaskGetTickCount(), __FILE__, __LINE__)
#define VSM_DEBUG_INFO(s,...)               printf(KGRN "%d [I] %s " s "\r\n" KNRM, xTaskGetTickCount(), __FILE__)
#define VSM_DEBUG_ERROR(s,...)              printf(KRED "%d [E] %s " s "\r\n" KNRM, xTaskGetTickCount(), __FILE__)
#define VSM_DEBUG_WARN(s,...)               printf(KYEL "%d [W] %s " s "\r\n" KNRM, xTaskGetTickCount(), __FILE__)
#define VSM_DEBUG_COLOR(color, s,...)       printf(color s "\n" KNRM, ##args)



#if (VSM_DEBUG_DISABLE)
#undef VSM_DEBUG
#define VSM_DEBUG            vsm_debug_nothing
#endif
#if (VSM_DEBUG_ERROR_DISABLE)
#undef VSM_DEBUG_ERROR
#define VSM_DEBUG_ERROR      vsm_debug_nothing
#endif
#if (VSM_DEBUG_WARN_DISABLE)
#undef VSM_DEBUG_WARN
#define VSM_DEBUG_WARN    vsm_debug_nothing
#endif
#if (VSM_DEBUG_INFO_DISABLE)
#undef VSM_DEBUG_INFO
#define VSM_DEBUG_INFO       vsm_debug_nothing
#endif
#if (VSM_DEBUG_RAW_DISABLE)
#undef VSM_DEBUG_RAW
#define VSM_DEBUG_RAW        vsm_debug_nothing
#endif

// #define VSM_DEBUG            VSM_DEBUG            ///< Print debug string with format (*str, arg1, arg2,...) terminate by new line
// #define VSM_DEBUG_RAW        VSM_DEBUG_RAW        ///< Print debug string with format (*str, arg1, arg2,...)
// #define VSM_DEBUG_ERROR      VSM_DEBUG_ERROR      ///< Print debug string with format (*str, arg1, arg2,...) with 'error' begin
// #define VSM_DEBUG_WARN       VSM_DEBUG_WARN       ///< Print debug string with format (*str, arg1, arg2,...) with  'warning' begin
// #define VSM_DEBUG_DUMP       VSM_DEBUG_DUMP       ///< Dump memory to debug channel, format (pointer, length, debug_string)
// #define VSM_DEBUG_COLOR      VSM_DEBUG_COLOR

/* Define ASSERT MACROS */
#ifndef ASSERT_NONVOID
    #define ASSERT_NONVOID(con,ret)             {if(!(con)) {VSM_DEBUG_ERROR("\r\nASSERT in file %s, line %d\r\n", __FILE__, __LINE__); return ret;} }
#endif

#ifndef ASSERT_VOID
    #define ASSERT_VOID(con)                    {if(!(con)) {VSM_DEBUG_ERROR("\r\nASSERT in file %s, line %d\r\n", __FILE__, __LINE__); return;   }  }
#endif

#ifndef ASSERT
    #define ASSERT(con)                         {if(!(con)) {VSM_DEBUG_ERROR("\r\nASSERT in file %s, line %d\r\n", __FILE__, __LINE__);           }  }
#endif





#ifdef __cplusplus
}
#endif

#endif /* VSM_DEBUG_H_ */
