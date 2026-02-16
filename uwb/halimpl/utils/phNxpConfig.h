/******************************************************************************
 *
 *  Copyright (C) 2011-2012 Broadcom Corporation
 *  Copyright 2018-2019, 2023 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdint.h>

void NxpConfig_Init(void);
void NxpConfig_Deinit(void);
bool NxpConfig_SetCountryCode(const char country_code[2]);

int NxpConfig_GetStr(const char* name, char* p_value, unsigned long len);
int NxpConfig_GetNum(const char* name, void* p_value, unsigned long len);
int NxpConfig_GetByteArray(const char* name, uint8_t* pValue, long bufflen, long *len);

int NxpConfig_GetStrArrayLen(const char* name, unsigned long *pLen);
int NxpConfig_GetStrArrayVal(const char* name, int index, char* pValue, unsigned long len);

/* libuwb-nxp.conf parameters */
#define NAME_UWB_BOARD_VARIANT_CONFIG "UWB_BOARD_VARIANT_CONFIG"
#define NAME_UWB_BOARD_VARIANT_VERSION "UWB_BOARD_VARIANT_VERSION"
#define NAME_UWB_CORE_EXT_DEVICE_DEFAULT_CONFIG "UWB_CORE_EXT_DEVICE_DEFAULT_CONFIG"
#define NAME_UWB_DEBUG_DEFAULT_CONFIG "UWB_DEBUG_DEFAULT_CONFIG"
#define NAME_ANTENNA_PAIR_SELECTION_CONFIG1 "ANTENNA_PAIR_SELECTION_CONFIG1"
#define NAME_ANTENNA_PAIR_SELECTION_CONFIG2 "ANTENNA_PAIR_SELECTION_CONFIG2"
#define NAME_ANTENNA_PAIR_SELECTION_CONFIG3 "ANTENNA_PAIR_SELECTION_CONFIG3"
#define NAME_ANTENNA_PAIR_SELECTION_CONFIG4 "ANTENNA_PAIR_SELECTION_CONFIG4"
#define NAME_ANTENNA_PAIR_SELECTION_CONFIG5 "ANTENNA_PAIR_SELECTION_CONFIG5"
#define NAME_NXP_CORE_CONF_BLK "NXP_CORE_CONF_BLK_"
#define NAME_UWB_FW_DOWNLOAD_LOG "UWB_FW_DOWNLOAD_LOG"
#define NAME_NXP_UWB_FLASH_CONFIG "NXP_UWB_FLASH_CONFIG"
#define NAME_NXP_UWB_XTAL_38MHZ_CONFIG "NXP_UWB_XTAL_38MHZ_CONFIG"
#define NAME_NXP_UWB_EXTENDED_NTF_CONFIG "NXP_UWB_EXTENDED_NTF_CONFIG"
#define NAME_UWB_CORE_EXT_DEVICE_SR1XX_T_CONFIG "UWB_CORE_EXT_DEVICE_SR1XX_T_CONFIG"
#define NAME_UWB_CORE_EXT_DEVICE_SR1XX_S_CONFIG "UWB_CORE_EXT_DEVICE_SR1XX_S_CONFIG"
#define NAME_COUNTRY_CODE_CAP_FILE_LOCATION "COUNTRY_CODE_CAP_FILE_LOCATION"
#define NAME_UWB_VENDOR_CAPABILITY "UWB_VENDOR_CAPABILITY"

#define NAME_UWB_BINDING_LOCKING_ALLOWED "UWB_BINDING_LOCKING_ALLOWED"
#define NAME_NXP_UWB_PROD_FW_FILENAME "NXP_UWB_PROD_FW_FILENAME"
#define NAME_NXP_UWB_DEV_FW_FILENAME "NXP_UWB_DEV_FW_FILENAME"
#define NAME_NXP_UWB_FW_FILENAME "NXP_UWB_FW_FILENAME"
#define NAME_NXP_UWB_EXT_APP_DEFAULT_CONFIG "NXP_UWB_EXT_APP_DEFAULT_CONFIG"
#define NAME_NXP_UWB_EXT_APP_SR1XX_T_CONFIG "NXP_UWB_EXT_APP_SR1XX_T_CONFIG"
#define NAME_NXP_UWB_EXT_APP_SR1XX_S_CONFIG "NXP_UWB_EXT_APP_SR1XX_S_CONFIG"
#define NAME_UWB_USER_FW_BOOT_MODE_CONFIG "UWB_USER_FW_BOOT_MODE_CONFIG"
#define NAME_NXP_COUNTRY_CODE_CONFIG "NXP_COUNTRY_CODE_CONFIG"
#define NAME_NXP_UWB_COUNTRY_CODE_CAPS "UWB_COUNTRY_CODE_CAPS"

#define NAME_NXP_SECURE_CONFIG_BLK "NXP_SECURE_CONFIG_BLK_"
#define NAME_PLATFORM_ID "PLATFORM_ID"

#define NAME_REGION_MAP_PATH "REGION_MAP_PATH"

#define NAME_NXP_UCI_CONFIG_PATH "NXP_UCI_CONFIG_PATH"

/* libuwb-uci.conf parameters */
#define NAME_NXP_UWB_LOW_POWER_MODE "UWB_LOW_POWER_MODE"

/* libuwb-countrycode.conf parameters */
#define NAME_NXP_COUNTRY_CODE_VERSION "VERSION"

/* default configuration */
#define default_storage_location "/data/vendor/uwb"

#endif
