/*
 * Copyright 2012-2020, 2022-2023 NXP
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
#ifndef _PHNXPUCIHAL_EXT_H_
#define _PHNXPUCIHAL_EXT_H_

#include <phNxpUciHal.h>
#include <string.h>

#define UCI_GID_NXP_PROPRIETARY 0x0D /* 1101b Proprietary Group */

/* Extended device core configirations */
#define EXTENDED_DEVICE_CONFIG_ID              0xE4

#define UCI_EXT_PARAM_DELAY_CALIBRATION_VALUE  0x00
#define UCI_EXT_PARAM_AOA_CALIBRATION_CTRL     0x01
#define UCI_EXT_PARAM_DPD_WAKEUP_SRC           0x02
#define UCI_EXT_PARAM_WTX_COUNT_CONFIG         0x03
#define UCI_EXT_PARAM_DPD_ENTRY_TIMEOUT        0x04
#define UCI_EXT_PARAM_WIFI_COEX_FEATURE        0x05
#define UCI_EXT_PARAM_TX_BASE_BAND_CONFIG      0x26
#define UCI_EXT_PARAM_DDFS_TONE_CONFIG         0x27
#define UCI_EXT_PARAM_TX_PULSE_SHAPE_CONFIG    0x28
#define UCI_EXT_PARAM_CLK_CONFIG_CTRL          0x30
#define UCI_EXT_PARAM_DBG_RFRAME_LOG_NTF 0x22
#define UCI_PARAM_ID_LOW_POWER_MODE            0x01
/* customer specific calib params */
#define VENDOR_CALIB_PARAM_TX_POWER_PER_ANTENNA 0x04

/* Proprietary GID */
#define UCI_GID_PROPRIETARY_0x0F 0x0F

/* Customer Specific  OID */
#define SET_VENDOR_SET_CALIBRATION 0x21

#define RMS_TX_POWER_SHIFT 8
#define UCI_RMS_TX_POWER_INDEX 7
#define UCI_RMS_TX_POWER_LEN 2

/* Extended Uci status Reason code */
#define UCI_EXT_STATUS_SE_RECOVERY_FAILURE  0x72
#define UCI_EXT_STATUS_SE_RECOVERY_SUCCESS  0x73
#define UCI_EXT_STATUS_SE_APDU_CMD_FAIL     0x74
#define UCI_EXT_STATUS_SE_AUTH_FAIL         0x75

typedef struct {
  pthread_attr_t attr_thread;
  pthread_t hal_thread_handling; /* fw crash thread handle */
  pthread_cond_t mCondVar;
  pthread_mutex_t lock;
  bool_t isThermalRecoveryOngoing;
} phNxpUciHalProp_Control_t;

tHAL_UWB_STATUS phNxpUciHal_send_ext_cmd(uint16_t cmd_len, const uint8_t* p_cmd);
tHAL_UWB_STATUS phNxpUciHal_process_ext_rsp(uint16_t cmd_len, uint8_t* p_buff);
tHAL_UWB_STATUS phNxpUciHal_set_board_config();
void phNxpUciHal_processCalibParamTxPowerPerAntenna(const uint8_t *p_data, uint16_t data_len);
void phNxpUciHal_extcal_handle_coreinit(void);
void phNxpUciHal_process_response();
void phNxpUciHal_handle_set_country_code(const char country_code[2]);
#endif /* _PHNXPNICHAL_EXT_H_ */
