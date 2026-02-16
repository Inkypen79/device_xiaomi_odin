/*
 * Copyright 2012-2018, 2023 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "NxpUwbHal"
#include "phNxpLog.h"
#include "phNxpConfig.h"
#include <cutils/properties.h>
#include <log/log.h>
#include <stdio.h>
#include <string.h>

const char* NXPLOG_ITEM_EXTNS = "NxpExtns";
const char* NXPLOG_ITEM_UCIHAL = "NxpUwbHal";
const char* NXPLOG_ITEM_UCIX = "NxpUciX";
const char* NXPLOG_ITEM_UCIR = "NxpUciR";
const char* NXPLOG_ITEM_FWDNLD = "NxpFwDnld";
const char* NXPLOG_ITEM_TML = "NxpUwbTml";

#ifdef NXP_HCI_REQ
const char* NXPLOG_ITEM_HCPX = "NxpHcpX";
const char* NXPLOG_ITEM_HCPR = "NxpHcpR";
#endif /*NXP_HCI_REQ*/

/* global log level structure */
uci_log_level_t gLog_level;

/*******************************************************************************
 *
 * Function         phNxpLog_SetGlobalLogLevel
 *
 * Description      Sets the global log level for all modules.
 *                  This value is set by Android property
 *uwb.nxp_log_level_global.
 *                  If value can be overridden by module log level.
 *
 * Returns          The value of global log level
 *
 ******************************************************************************/
static uint8_t phNxpLog_SetGlobalLogLevel(void) {
  uint8_t level = NXPLOG_DEFAULT_LOGLEVEL;
  unsigned long num = 0;
  char valueStr[PROPERTY_VALUE_MAX] = {0};

  int len = property_get(PROP_NAME_NXPLOG_GLOBAL_LOGLEVEL, valueStr, "");
  if (len > 0) {
    // let Android property override .conf variable
    sscanf(valueStr, "%lu", &num);
    level = (unsigned char)num;
  }
  memset(&gLog_level, level, sizeof(uci_log_level_t));
  return level;
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetHALLogLevel
 *
 * Description      Sets the HAL layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void phNxpLog_SetHALLogLevel(uint8_t level) {
  unsigned long num = 0;
  int len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};

  if (NxpConfig_GetNum(NAME_NXPLOG_HAL_LOGLEVEL, &num, sizeof(num))) {
    gLog_level.hal_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
    ;
  }

  len = property_get(PROP_NAME_NXPLOG_HAL_LOGLEVEL, valueStr, "");
  if (len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);

    gLog_level.hal_log_level = (unsigned char)num;
  }
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetExtnsLogLevel
 *
 * Description      Sets the Extensions layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void phNxpLog_SetExtnsLogLevel(uint8_t level) {
  unsigned long num = 0;
  int len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  if (NxpConfig_GetNum(NAME_NXPLOG_EXTNS_LOGLEVEL, &num, sizeof(num))) {
    gLog_level.extns_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
    ;
  }

  len = property_get(PROP_NAME_NXPLOG_EXTNS_LOGLEVEL, valueStr, "");
  if (len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    gLog_level.extns_log_level = (unsigned char)num;
  }
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetTmlLogLevel
 *
 * Description      Sets the Tml layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void phNxpLog_SetTmlLogLevel(uint8_t level) {
  unsigned long num = 0;
  int len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  if (NxpConfig_GetNum(NAME_NXPLOG_TML_LOGLEVEL, &num, sizeof(num))) {
    gLog_level.tml_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
    ;
  }

  len = property_get(PROP_NAME_NXPLOG_TML_LOGLEVEL, valueStr, "");
  if (len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    gLog_level.tml_log_level = (unsigned char)num;
  }
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetDnldLogLevel
 *
 * Description      Sets the FW download layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void phNxpLog_SetDnldLogLevel(uint8_t level) {
  unsigned long num = 0;
  int len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  if (NxpConfig_GetNum(NAME_NXPLOG_FWDNLD_LOGLEVEL, &num, sizeof(num))) {
    gLog_level.dnld_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
    ;
  }

  len = property_get(PROP_NAME_NXPLOG_FWDNLD_LOGLEVEL, valueStr, "");
  if (len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    gLog_level.dnld_log_level = (unsigned char)num;
  }
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetUciTxLogLevel
 *
 * Description      Sets the UCI transaction layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void phNxpLog_SetUciTxLogLevel(uint8_t level) {
  unsigned long num = 0;
  int len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  if (NxpConfig_GetNum(NAME_NXPLOG_UCIX_LOGLEVEL, &num, sizeof(num))) {
    gLog_level.ucix_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
  }
  if (NxpConfig_GetNum(NAME_NXPLOG_UCIR_LOGLEVEL, &num, sizeof(num))) {
    gLog_level.ucir_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
    ;
  }

  len = property_get(PROP_NAME_NXPLOG_UCI_LOGLEVEL, valueStr, "");
  if (len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    gLog_level.ucix_log_level = (unsigned char)num;
    gLog_level.ucir_log_level = (unsigned char)num;
  }
}

/******************************************************************************
 * Function         phNxpLog_InitializeLogLevel
 *
 * Description      Initialize and get log level of module from libuwb-nxp.conf
 *or
 *                  Android runtime properties.
 *                  The Android property uwb.nxp_global_log_level is to
 *                  define log level for all modules. Modules log level will
 *overwide global level.
 *                  The Android property will overwide the level
 *                  in libuwb-nxp.conf
 *
 *                  Android property names:
 *                      uwb.nxp_log_level_global    * defines log level for all
 *modules
 *                      uwb.nxp_log_level_extns     * extensions module log
 *                      uwb.nxp_log_level_hal       * Hal module log
 *                      uwb.nxp_log_level_dnld      * firmware download module
 *log
 *                      uwb.nxp_log_level_tml       * TML module log
 *                      uwb.nxp_log_level_uci       * UCI transaction log
 *
 *                  Log Level values:
 *                      NXPLOG_LOG_SILENT_LOGLEVEL  0        * No trace to show
 *                      NXPLOG_LOG_ERROR_LOGLEVEL   1        * Show Error trace
 *only
 *                      NXPLOG_LOG_WARN_LOGLEVEL    2        * Show Warning
 *trace and Error trace
 *                      NXPLOG_LOG_DEBUG_LOGLEVEL   3        * Show all traces
 *
 * Returns          void
 *
 ******************************************************************************/
void phNxpLog_InitializeLogLevel(void) {
  uint8_t level = phNxpLog_SetGlobalLogLevel();
  phNxpLog_SetHALLogLevel(level);
  phNxpLog_SetExtnsLogLevel(level);
  phNxpLog_SetTmlLogLevel(level);
  phNxpLog_SetDnldLogLevel(level);
  phNxpLog_SetUciTxLogLevel(level);

  ALOGV("%s: global =%u, Fwdnld =%u, extns =%u, \
                hal =%u, tml =%u, ucir =%u, \
                ucix =%u",
           __func__, gLog_level.global_log_level, gLog_level.dnld_log_level,
           gLog_level.extns_log_level, gLog_level.hal_log_level,
           gLog_level.tml_log_level, gLog_level.ucir_log_level,
           gLog_level.ucix_log_level);
}
