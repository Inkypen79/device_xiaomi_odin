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

#if !defined(NXPLOG__H_INCLUDED)
#define NXPLOG__H_INCLUDED
#include <log/log.h>

typedef struct uci_log_level {
  uint8_t global_log_level;
  uint8_t extns_log_level;
  uint8_t hal_log_level;
  uint8_t dnld_log_level;
  uint8_t tml_log_level;
  uint8_t ucix_log_level;
  uint8_t ucir_log_level;
} uci_log_level_t;

/* global log level Ref */
extern uci_log_level_t gLog_level;
/* define log module included when compile */
#define ENABLE_EXTNS_TRACES TRUE
#define ENABLE_HAL_TRACES TRUE
#define ENABLE_TML_TRACES TRUE
#define ENABLE_FWDNLD_TRACES TRUE
#define ENABLE_UCIX_TRACES TRUE
#define ENABLE_UCIR_TRACES TRUE

#define ENABLE_HCPX_TRACES FALSE
#define ENABLE_HCPR_TRACES FALSE

/* ####################### Set the log module name in .conf file
 * ########################## */
#define NAME_NXPLOG_EXTNS_LOGLEVEL "NXP_LOG_EXTNS_LOGLEVEL"
#define NAME_NXPLOG_HAL_LOGLEVEL "NXP_LOG_UCIHAL_LOGLEVEL"
#define NAME_NXPLOG_UCIX_LOGLEVEL "NXP_LOG_UCIX_LOGLEVEL"
#define NAME_NXPLOG_UCIR_LOGLEVEL "NXP_LOG_UCIR_LOGLEVEL"
#define NAME_NXPLOG_FWDNLD_LOGLEVEL "NXP_LOG_FWDNLD_LOGLEVEL"
#define NAME_NXPLOG_TML_LOGLEVEL "NXP_LOG_TML_LOGLEVEL"

/* ####################### Set the log module name by Android property
 * ########################## */
#define PROP_NAME_NXPLOG_GLOBAL_LOGLEVEL  "persist.vendor.uwb.nxp_log_level_global"
#define PROP_NAME_NXPLOG_EXTNS_LOGLEVEL   "persist.vendor.uwb.nxp_log_level_extns"
#define PROP_NAME_NXPLOG_HAL_LOGLEVEL     "persist.vendor.uwb.nxp_log_level_hal"
#define PROP_NAME_NXPLOG_UCI_LOGLEVEL     "persist.vendor.uwb.nxp_log_level_uci"
#define PROP_NAME_NXPLOG_FWDNLD_LOGLEVEL  "persist.vendor.uwb.nxp_log_level_dnld"
#define PROP_NAME_NXPLOG_TML_LOGLEVEL     "persist.vendor.uwb.nxp_log_level_tml"

/* ####################### Set the logging level for EVERY COMPONENT here
 * ######################## :START: */
#define NXPLOG_LOG_SILENT_LOGLEVEL 0x00
#define NXPLOG_LOG_ERROR_LOGLEVEL 0x01
#define NXPLOG_LOG_WARN_LOGLEVEL 0x02
#define NXPLOG_LOG_DEBUG_LOGLEVEL 0x03
/* ####################### Set the default logging level for EVERY COMPONENT
 * here ########################## :END: */

/* The Default log level for all the modules. */
#define NXPLOG_DEFAULT_LOGLEVEL NXPLOG_LOG_ERROR_LOGLEVEL

/* ################################################################################################################
 */
/* ############################################### Component Names
 * ################################################ */
/* ################################################################################################################
 */

extern const char* NXPLOG_ITEM_EXTNS;  /* Android logging tag for NxpExtns  */
extern const char* NXPLOG_ITEM_UCIHAL; /* Android logging tag for NxpUciHal */
extern const char* NXPLOG_ITEM_UCIX;   /* Android logging tag for NxpUciX   */
extern const char* NXPLOG_ITEM_UCIR;   /* Android logging tag for NxpUciR   */
extern const char* NXPLOG_ITEM_FWDNLD; /* Android logging tag for NxpFwDnld */
extern const char* NXPLOG_ITEM_TML;    /* Android logging tag for NxpTml    */

#ifdef NXP_HCI_REQ
extern const char* NXPLOG_ITEM_HCPX; /* Android logging tag for NxpHcpX   */
extern const char* NXPLOG_ITEM_HCPR; /* Android logging tag for NxpHcpR   */
#endif                               /*NXP_HCI_REQ*/

/* ######################################## Defines used for Logging data
 * ######################################### */
#ifdef NXP_VRBS_REQ
#define NXPLOG_FUNC_ENTRY(COMP) \
  LOG_PRI(ANDROID_LOG_VERBOSE, (COMP), "+:%s", (__func__))
#define NXPLOG_FUNC_EXIT(COMP) \
  LOG_PRI(ANDROID_LOG_VERBOSE, (COMP), "-:%s", (__func__))
#endif /*NXP_VRBS_REQ*/

/* ################################################################################################################
 */
/* ######################################## Logging APIs of actual modules
 * ######################################## */
/* ################################################################################################################
 */
/* Logging APIs used by NxpExtns module */
#if (ENABLE_EXTNS_TRACES == TRUE)
#define NXPLOG_EXTNS_D(...)                                        \
  {                                                                \
    if ((gLog_level.extns_log_level >= NXPLOG_LOG_DEBUG_LOGLEVEL)) \
      LOG_PRI(ANDROID_LOG_DEBUG, NXPLOG_ITEM_EXTNS, __VA_ARGS__);  \
  }
#define NXPLOG_EXTNS_W(...)                                       \
  {                                                               \
    if ((gLog_level.extns_log_level >= NXPLOG_LOG_WARN_LOGLEVEL)) \
      LOG_PRI(ANDROID_LOG_WARN, NXPLOG_ITEM_EXTNS, __VA_ARGS__);  \
  }
#define NXPLOG_EXTNS_E(...)                                       \
  {                                                               \
    if (gLog_level.extns_log_level >= NXPLOG_LOG_ERROR_LOGLEVEL)  \
      LOG_PRI(ANDROID_LOG_ERROR, NXPLOG_ITEM_EXTNS, __VA_ARGS__); \
  }
#else
#define NXPLOG_EXTNS_D(...)
#define NXPLOG_EXTNS_W(...)
#define NXPLOG_EXTNS_E(...)
#endif /* Logging APIs used by NxpExtns module */

/* Logging APIs used by NxpUciHal module */
#if (ENABLE_HAL_TRACES == TRUE)
#define NXPLOG_UCIHAL_D(...)                                       \
  {                                                                \
    if ((gLog_level.hal_log_level >= NXPLOG_LOG_DEBUG_LOGLEVEL))   \
      LOG_PRI(ANDROID_LOG_DEBUG, NXPLOG_ITEM_UCIHAL, __VA_ARGS__); \
  }
#define NXPLOG_UCIHAL_W(...)                                      \
  {                                                               \
    if ((gLog_level.hal_log_level >= NXPLOG_LOG_WARN_LOGLEVEL))   \
      LOG_PRI(ANDROID_LOG_WARN, NXPLOG_ITEM_UCIHAL, __VA_ARGS__); \
  }
#define NXPLOG_UCIHAL_E(...)                                       \
  {                                                                \
    if (gLog_level.hal_log_level >= NXPLOG_LOG_ERROR_LOGLEVEL)     \
      LOG_PRI(ANDROID_LOG_ERROR, NXPLOG_ITEM_UCIHAL, __VA_ARGS__); \
  }
#else
#define NXPLOG_UCIHAL_D(...)
#define NXPLOG_UCIHAL_W(...)
#define NXPLOG_UCIHAL_E(...)
#endif /* Logging APIs used by HAL module */

/* Logging APIs used by NxpUciX module */
#if (ENABLE_UCIX_TRACES == TRUE)
#define NXPLOG_UCIX_D(...)                                        \
  {                                                               \
    if ((gLog_level.ucix_log_level >= NXPLOG_LOG_DEBUG_LOGLEVEL)) \
      LOG_PRI(ANDROID_LOG_DEBUG, NXPLOG_ITEM_UCIX, __VA_ARGS__);  \
  }
#define NXPLOG_UCIX_W(...)                                       \
  {                                                              \
    if ((gLog_level.ucix_log_level >= NXPLOG_LOG_WARN_LOGLEVEL)) \
      LOG_PRI(ANDROID_LOG_WARN, NXPLOG_ITEM_UCIX, __VA_ARGS__);  \
  }
#define NXPLOG_UCIX_E(...)                                       \
  {                                                              \
    if (gLog_level.ucix_log_level >= NXPLOG_LOG_ERROR_LOGLEVEL)  \
      LOG_PRI(ANDROID_LOG_ERROR, NXPLOG_ITEM_UCIX, __VA_ARGS__); \
  }
#else
#define NXPLOG_UCIX_D(...)
#define NXPLOG_UCIX_W(...)
#define NXPLOG_UCIX_E(...)
#endif /* Logging APIs used by UCIx module */

/* Logging APIs used by NxpUciR module */
#if (ENABLE_UCIR_TRACES == TRUE)
#define NXPLOG_UCIR_D(...)                                        \
  {                                                               \
    if ((gLog_level.ucir_log_level >= NXPLOG_LOG_DEBUG_LOGLEVEL)) \
      LOG_PRI(ANDROID_LOG_DEBUG, NXPLOG_ITEM_UCIR, __VA_ARGS__);  \
  }
#define NXPLOG_UCIR_W(...)                                       \
  {                                                              \
    if ((gLog_level.ucir_log_level >= NXPLOG_LOG_WARN_LOGLEVEL)) \
      LOG_PRI(ANDROID_LOG_WARN, NXPLOG_ITEM_UCIR, __VA_ARGS__);  \
  }
#define NXPLOG_UCIR_E(...)                                       \
  {                                                              \
    if (gLog_level.ucir_log_level >= NXPLOG_LOG_ERROR_LOGLEVEL)  \
      LOG_PRI(ANDROID_LOG_ERROR, NXPLOG_ITEM_UCIR, __VA_ARGS__); \
  }
#else
#define NXPLOG_UCIR_D(...)
#define NXPLOG_UCIR_W(...)
#define NXPLOG_UCIR_E(...)
#endif /* Logging APIs used by UCIR module */

/* Logging APIs used by NxpFwDnld module */
#if (ENABLE_FWDNLD_TRACES == TRUE)
#define NXPLOG_FWDNLD_D(...)                                       \
  {                                                                \
    if ((gLog_level.dnld_log_level >= NXPLOG_LOG_DEBUG_LOGLEVEL))  \
      LOG_PRI(ANDROID_LOG_DEBUG, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#define NXPLOG_FWDNLD_W(...)                                      \
  {                                                               \
    if ((gLog_level.dnld_log_level >= NXPLOG_LOG_WARN_LOGLEVEL))  \
      LOG_PRI(ANDROID_LOG_WARN, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#define NXPLOG_FWDNLD_E(...)                                       \
  {                                                                \
    if (gLog_level.dnld_log_level >= NXPLOG_LOG_ERROR_LOGLEVEL)    \
      LOG_PRI(ANDROID_LOG_ERROR, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#else
#define NXPLOG_FWDNLD_D(...)
#define NXPLOG_FWDNLD_W(...)
#define NXPLOG_FWDNLD_E(...)
#endif /* Logging APIs used by NxpFwDnld module */

/* Logging APIs used by NxpTml module */
#if (ENABLE_TML_TRACES == TRUE)
#define NXPLOG_TML_D(...)                                        \
  {                                                              \
    if ((gLog_level.tml_log_level >= NXPLOG_LOG_DEBUG_LOGLEVEL)) \
      LOG_PRI(ANDROID_LOG_DEBUG, NXPLOG_ITEM_TML, __VA_ARGS__);  \
  }
#define NXPLOG_TML_W(...)                                       \
  {                                                             \
    if ((gLog_level.tml_log_level >= NXPLOG_LOG_WARN_LOGLEVEL)) \
      LOG_PRI(ANDROID_LOG_WARN, NXPLOG_ITEM_TML, __VA_ARGS__);  \
  }
#define NXPLOG_TML_E(...)                                       \
  {                                                             \
    if (gLog_level.tml_log_level >= NXPLOG_LOG_ERROR_LOGLEVEL)  \
      LOG_PRI(ANDROID_LOG_ERROR, NXPLOG_ITEM_TML, __VA_ARGS__); \
  }
#else
#define NXPLOG_TML_D(...)
#define NXPLOG_TML_W(...)
#define NXPLOG_TML_E(...)
#endif /* Logging APIs used by NxpTml module */

#ifdef NXP_HCI_REQ
/* Logging APIs used by NxpHcpX module */
#if (ENABLE_HCPX_TRACES == TRUE)
#define NXPLOG_HCPX_D(...)                                         \
  {                                                                \
    if ((gLog_level.dnld_log_level >= NXPLOG_LOG_DEBUG_LOGLEVEL))  \
      LOG_PRI(ANDROID_LOG_DEBUG, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#define NXPLOG_HCPX_W(...)                                        \
  {                                                               \
    if ((gLog_level.dnld_log_level >= NXPLOG_LOG_WARN_LOGLEVEL))  \
      LOG_PRI(ANDROID_LOG_WARN, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#define NXPLOG_HCPX_E(...)                                         \
  {                                                                \
    if (gLog_level.dnld_log_level >= NXPLOG_LOG_ERROR_LOGLEVEL)    \
      LOG_PRI(ANDROID_LOG_ERROR, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#else
#define NXPLOG_HCPX_D(...)
#define NXPLOG_HCPX_W(...)
#define NXPLOG_HCPX_E(...)
#endif /* Logging APIs used by NxpHcpX module */

/* Logging APIs used by NxpHcpR module */
#if (ENABLE_HCPR_TRACES == TRUE)
#define NXPLOG_HCPR_D(...)                                         \
  {                                                                \
    if ((gLog_level.dnld_log_level >= NXPLOG_LOG_DEBUG_LOGLEVEL))  \
      LOG_PRI(ANDROID_LOG_DEBUG, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#define NXPLOG_HCPR_W(...)                                        \
  {                                                               \
    if ((gLog_level.dnld_log_level >= NXPLOG_LOG_WARN_LOGLEVEL))  \
      LOG_PRI(ANDROID_LOG_WARN, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#define NXPLOG_HCPR_E(...)                                         \
  {                                                                \
    if (gLog_level.dnld_log_level >= NXPLOG_LOG_ERROR_LOGLEVEL)    \
      LOG_PRI(ANDROID_LOG_ERROR, NXPLOG_ITEM_FWDNLD, __VA_ARGS__); \
  }
#else
#define NXPLOG_HCPR_D(...)
#define NXPLOG_HCPR_W(...)
#define NXPLOG_HCPR_E(...)
#endif /* Logging APIs used by NxpHcpR module */
#endif /* NXP_HCI_REQ */

#ifdef NXP_VRBS_REQ
#if (ENABLE_EXTNS_TRACES == TRUE)
#define NXPLOG_EXTNS_ENTRY() NXPLOG_FUNC_ENTRY(NXPLOG_ITEM_EXTNS)
#define NXPLOG_EXTNS_EXIT() NXPLOG_FUNC_EXIT(NXPLOG_ITEM_EXTNS)
#else
#define NXPLOG_EXTNS_ENTRY()
#define NXPLOG_EXTNS_EXIT()
#endif

#if (ENABLE_HAL_TRACES == TRUE)
#define NXPLOG_UCIHAL_ENTRY() NXPLOG_FUNC_ENTRY(NXPLOG_ITEM_UCIHAL)
#define NXPLOG_UCIHAL_EXIT() NXPLOG_FUNC_EXIT(NXPLOG_ITEM_UCIHAL)
#else
#define NXPLOG_UCIHAL_ENTRY()
#define NXPLOG_UCIHAL_EXIT()
#endif

#if (ENABLE_UCIX_TRACES == TRUE)
#define NXPLOG_UCIX_ENTRY() NXPLOG_FUNC_ENTRY(NXPLOG_ITEM_UCIX)
#define NXPLOG_UCIX_EXIT() NXPLOG_FUNC_EXIT(NXPLOG_ITEM_UCIX)
#else
#define NXPLOG_UCIX_ENTRY()
#define NXPLOG_UCIX_EXIT()
#endif

#if (ENABLE_UCIR_TRACES == TRUE)
#define NXPLOG_UCIR_ENTRY() NXPLOG_FUNC_ENTRY(NXPLOG_ITEM_UCIR)
#define NXPLOG_UCIR_EXIT() NXPLOG_FUNC_EXIT(NXPLOG_ITEM_UCIR)
#else
#define NXPLOG_UCIR_ENTRY()
#define NXPLOG_UCIR_EXIT()
#endif

#ifdef NXP_HCI_REQ

#if (ENABLE_HCPX_TRACES == TRUE)
#define NXPLOG_HCPX_ENTRY() NXPLOG_FUNC_ENTRY(NXPLOG_ITEM_HCPX)
#define NXPLOG_HCPX_EXIT() NXPLOG_FUNC_EXIT(NXPLOG_ITEM_HCPX)
#else
#define NXPLOG_HCPX_ENTRY()
#define NXPLOG_HCPX_EXIT()
#endif

#if (ENABLE_HCPR_TRACES == TRUE)
#define NXPLOG_HCPR_ENTRY() NXPLOG_FUNC_ENTRY(NXPLOG_ITEM_HCPR)
#define NXPLOG_HCPR_EXIT() NXPLOG_FUNC_EXIT(NXPLOG_ITEM_HCPR)
#else
#define NXPLOG_HCPR_ENTRY()
#define NXPLOG_HCPR_EXIT()
#endif
#endif /* NXP_HCI_REQ */

#endif /* NXP_VRBS_REQ */

void phNxpLog_InitializeLogLevel(void);

#endif /* NXPLOG__H_INCLUDED */
