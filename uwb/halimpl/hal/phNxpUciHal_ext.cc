/*
 * Copyright 2012-2019, 2022-2023 NXP
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
#include <string.h>
#include <sys/stat.h>

#include <atomic>
#include <bitset>

#include <cutils/properties.h>

#include "phDal4Uwb_messageQueueLib.h"
#include "phNxpConfig.h"
#include "phNxpLog.h"
#include "phNxpUciHal_ext.h"
#include "phNxpUciHal.h"
#include "phTmlUwb.h"
#include "phUwbCommon.h"

/* Timeout value to wait for response from DEVICE_TYPE_SR1xx */
#define HAL_EXTNS_WRITE_RSP_TIMEOUT (100)
#define HAL_HW_RESET_NTF_TIMEOUT 10000 /* 10 sec wait */

/******************* Global variables *****************************************/
extern phNxpUciHal_Control_t nxpucihal_ctrl;

extern uint32_t cleanup_timer;
extern uint32_t timeoutTimerId;
extern short conf_tx_power;

static std::vector<uint8_t> gtx_power;
static std::vector<uint8_t> gRMS_tx_power;

phNxpUciHalProp_Control_t extNxpucihal_ctrl;
uint32_t hwResetTimer;

/************** HAL extension functions ***************************************/
static void hal_extns_write_rsp_timeout_cb(uint32_t TimerId, void *pContext);
static void phNxpUciHal_send_dev_status_ntf();
static bool phNxpUciHal_is_retry_required(uint8_t uci_octet0);
static void phNxpUciHal_clear_thermal_error_status();
static void phNxpUciHal_hw_reset_ntf_timeout_cb(uint32_t timerId,
                                                void *pContext);
tHAL_UWB_STATUS phNxpUciHal_handle_thermal_error_status();

/******************************************************************************
 * Function         phNxpUciHal_process_ext_cmd_rsp
 *
 * Description      This function process the extension command response. It
 *                  also checks the received response to expected response.
 *
 * Returns          returns UWBSTATUS_SUCCESS if response is as expected else
 *                  returns failure.
 *
 ******************************************************************************/
tHAL_UWB_STATUS phNxpUciHal_process_ext_cmd_rsp(uint16_t cmd_len,
                                                const uint8_t *p_cmd,
                                                uint16_t *data_written) {
  tHAL_UWB_STATUS status = UWBSTATUS_FAILED;
  uint8_t ext_cmd_retry_cnt = 0, invalid_len_retry_cnt = 0;
  bool exit_loop = 0, isRetryRequired = false;
  /* Create the local semaphore */
  if (phNxpUciHal_init_cb_data(&nxpucihal_ctrl.ext_cb_data, NULL) !=
      UWBSTATUS_SUCCESS) {
    NXPLOG_UCIHAL_D("Create ext_cb_data failed");
    return UWBSTATUS_FAILED;
  }

  isRetryRequired = phNxpUciHal_is_retry_required(p_cmd[0]);

  /* Send ext command */
  do {
    NXPLOG_UCIHAL_D("Entered do while loop");

    nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_SUCCESS;
    nxpucihal_ctrl.ext_cb_waiting = true;

    *data_written = phNxpUciHal_write_unlocked(cmd_len, p_cmd);

    if (*data_written != cmd_len) {
      NXPLOG_UCIHAL_D("phNxpUciHal_write failed for hal ext");
      goto clean_and_return;
    }
    if (nxpucihal_ctrl.hal_parse_enabled) {
      goto clean_and_return;
    }

    NXPLOG_UCIHAL_D("ext_cmd_retry_cnt is %d",ext_cmd_retry_cnt);

    if (isRetryRequired) {
      NXPLOG_UCIHAL_D("Received chained command or data command, no need to "
                      "wait for response");
      exit_loop = 1;
    } else {
      /* Start timer */
      status =
          phOsalUwb_Timer_Start(timeoutTimerId, HAL_EXTNS_WRITE_RSP_TIMEOUT,
                                &hal_extns_write_rsp_timeout_cb, NULL);
      if (UWBSTATUS_SUCCESS == status) {
        NXPLOG_UCIHAL_D("Response timer started");
      } else {
        NXPLOG_UCIHAL_E("Response timer not started!!!");
        status = UWBSTATUS_FAILED;
        goto clean_and_return;
      }

      /* Wait for rsp */
      NXPLOG_UCIHAL_D("Waiting after ext cmd sent");
      if (SEM_WAIT(nxpucihal_ctrl.ext_cb_data)) {
        NXPLOG_UCIHAL_E("p_hal_ext->ext_cb_data.sem semaphore error");
        goto clean_and_return;
      }
      nxpucihal_ctrl.ext_cb_waiting = false;

      switch (nxpucihal_ctrl.ext_cb_data.status) {
      case UWBSTATUS_RESPONSE_TIMEOUT:
      case UWBSTATUS_COMMAND_RETRANSMIT:
        ext_cmd_retry_cnt++;
        break;
      case UWBSTATUS_INVALID_COMMAND_LENGTH:
        // XXX: Why retrying here?
        invalid_len_retry_cnt++;
        break;
      default:
        exit_loop = 1;
        break;
      }
      if ((ext_cmd_retry_cnt >= MAX_COMMAND_RETRY_COUNT) ||
          (invalid_len_retry_cnt >= 0x03)) {
        exit_loop = 1;
        phNxpUciHal_send_dev_status_ntf();
      }
    }
  } while(exit_loop == 0);

  if (!isRetryRequired) {
    /* Stop Timer */
    status = phOsalUwb_Timer_Stop(timeoutTimerId);

    if (UWBSTATUS_SUCCESS == status) {
      NXPLOG_UCIHAL_D("Response timer stopped");
    } else {
      NXPLOG_UCIHAL_E("Response timer stop ERROR!!!");
      status = UWBSTATUS_FAILED;
      goto clean_and_return;
    }

    if (nxpucihal_ctrl.ext_cb_data.status != UWBSTATUS_SUCCESS) {
      NXPLOG_UCIHAL_E("Response Status = 0x%x",
                      nxpucihal_ctrl.ext_cb_data.status);
      status = UWBSTATUS_FAILED;
      goto clean_and_return;
    }
    NXPLOG_UCIHAL_D("Checking response");
    status = UWBSTATUS_SUCCESS;
  }
clean_and_return:
  phNxpUciHal_cleanup_cb_data(&nxpucihal_ctrl.ext_cb_data);

  return status;
}

/******************************************************************************
 * Function         phNxpUciHal_send_ext_cmd
 *
 * Description      This function send the extension command to UWBC. No
 *                  response is checked by this function but it waits for
 *                  the response to come.
 *
 * Returns          Returns UWBSTATUS_SUCCESS if sending cmd is successful and
 *                  response is received.
 *
 ******************************************************************************/
tHAL_UWB_STATUS phNxpUciHal_send_ext_cmd(uint16_t cmd_len, const uint8_t* p_cmd) {
  tHAL_UWB_STATUS status;

  if (cmd_len >= UCI_MAX_DATA_LEN) {
    status = UWBSTATUS_FAILED;
    return status;
  }
  uint16_t data_written = 0;
  HAL_ENABLE_EXT();
  nxpucihal_ctrl.cmd_len = cmd_len;
  memcpy(nxpucihal_ctrl.p_cmd_data, p_cmd, cmd_len);
  status = phNxpUciHal_process_ext_cmd_rsp(
      nxpucihal_ctrl.cmd_len, nxpucihal_ctrl.p_cmd_data, &data_written);
  HAL_DISABLE_EXT();

  return status;
}

/******************************************************************************
 * Function         hal_extns_write_rsp_timeout_cb
 *
 * Description      Timer call back function
 *
 * Returns          None
 *
 ******************************************************************************/
static void hal_extns_write_rsp_timeout_cb(uint32_t timerId, void* pContext) {
  UNUSED(timerId);
  UNUSED(pContext);
  NXPLOG_UCIHAL_E("hal_extns_write_rsp_timeout_cb - write timeout!!!");
  nxpucihal_ctrl.ext_cb_data.status = UWBSTATUS_RESPONSE_TIMEOUT;
  usleep(1);
  SEM_POST(&(nxpucihal_ctrl.ext_cb_data));

  return;
}

/******************************************************************************
 * Function         phNxpUciHal_set_board_config
 *
 * Description      This function is called to set the board varaint config
 * Returns          return 0 on success and -1 on fail, On success
 *                  update the acutual state of operation in arg pointer
 *
 ******************************************************************************/
tHAL_UWB_STATUS phNxpUciHal_set_board_config(){
  tHAL_UWB_STATUS status;
  uint8_t buffer[] = {0x2E,0x00,0x00,0x02,0x01,0x01};
  /* Set the board variant configurations */
  unsigned long num = 0;
  NXPLOG_UCIHAL_D("%s: enter; ", __func__);
  uint8_t boardConfig = 0, boardVersion = 0;

  if(NxpConfig_GetNum(NAME_UWB_BOARD_VARIANT_CONFIG, &num, sizeof(num))){
    boardConfig = (uint8_t)num;
    NXPLOG_UCIHAL_D("%s: NAME_UWB_BOARD_VARIANT_CONFIG: %x", __func__,boardConfig);
  } else {
    NXPLOG_UCIHAL_D("%s: NAME_UWB_BOARD_VARIANT_CONFIG: failed %x", __func__,boardConfig);
  }
  if(NxpConfig_GetNum(NAME_UWB_BOARD_VARIANT_VERSION, &num, sizeof(num))){
    boardVersion = (uint8_t)num;
    NXPLOG_UCIHAL_D("%s: NAME_UWB_BOARD_VARIANT_VERSION: %x", __func__,boardVersion);
  } else{
    NXPLOG_UCIHAL_D("%s: NAME_UWB_BOARD_VARIANT_VERSION: failed %lx", __func__,num);
  }
  buffer[4] = boardConfig;
  buffer[5] = boardVersion;

  status = phNxpUciHal_send_ext_cmd(sizeof(buffer), buffer);

  return status;
}

/*******************************************************************************
**
** Function         phNxpUciHal_process_ext_rsp
**
** Description      Process extension function response
**
** Returns          UWBSTATUS_SUCCESS if success
**
*******************************************************************************/
tHAL_UWB_STATUS phNxpUciHal_process_ext_rsp(uint16_t rsp_len, uint8_t* p_buff){
  tHAL_UWB_STATUS status;
  int NumOfTlv, index;
  uint8_t paramId, extParamId, IdStatus;
  index = UCI_NTF_PAYLOAD_OFFSET; // index for payload start
  status = p_buff[index++];
  if(status  == UCI_STATUS_OK){
    NXPLOG_UCIHAL_D("%s: status success %d", __func__, status);
    return UWBSTATUS_SUCCESS;
  }
  NumOfTlv = p_buff[index++];
  while (index < rsp_len) {
      paramId = p_buff[index++];
      if(paramId == EXTENDED_DEVICE_CONFIG_ID) {
        extParamId = p_buff[index++];
        IdStatus = p_buff[index++];

        switch(extParamId) {
          case UCI_EXT_PARAM_DELAY_CALIBRATION_VALUE:
          case UCI_EXT_PARAM_AOA_CALIBRATION_CTRL:
          case UCI_EXT_PARAM_DPD_WAKEUP_SRC:
          case UCI_EXT_PARAM_WTX_COUNT_CONFIG:
          case UCI_EXT_PARAM_DPD_ENTRY_TIMEOUT:
          case UCI_EXT_PARAM_WIFI_COEX_FEATURE:
          case UCI_EXT_PARAM_TX_BASE_BAND_CONFIG:
          case UCI_EXT_PARAM_DDFS_TONE_CONFIG:
          case UCI_EXT_PARAM_TX_PULSE_SHAPE_CONFIG:
          case UCI_EXT_PARAM_CLK_CONFIG_CTRL:
            if(IdStatus == UCI_STATUS_FEATURE_NOT_SUPPORTED){
              NXPLOG_UCIHAL_E("%s: Vendor config param: %x %x is Not Supported", __func__, paramId, extParamId);
              status = UWBSTATUS_SUCCESS;
            } else {
              status = UWBSTATUS_FAILED;
              return status;
            }
            break;
          default:
            NXPLOG_UCIHAL_D("%s: Vendor param ID: %x", __func__, extParamId);
            break;
        }
      } else {
        IdStatus = p_buff[index++];
        switch(paramId) {
          case UCI_PARAM_ID_LOW_POWER_MODE:
            if(IdStatus == UCI_STATUS_FEATURE_NOT_SUPPORTED){
              NXPLOG_UCIHAL_E("%s: Generic config param: %x is Not Supported", __func__, paramId);
              status = UWBSTATUS_SUCCESS;
            } else {
              status = UWBSTATUS_FAILED;
              return status;
            }
            break;
          default:
            NXPLOG_UCIHAL_D("%s: Generic param ID: %x", __func__, paramId);
            break;
        }
      }
    }
 NXPLOG_UCIHAL_D("%s: exit %d", __func__, status);
 return status;
}

static bool phNxpUciHal_setCalibParamTxPower(void);

/*******************************************************************************
 * Function      phNxpUciHal_resetRuntimeSettings
 *
 * Description   reset per-country code settigs to default
 *
 *******************************************************************************/
static void phNxpUciHal_resetRuntimeSettings(void)
{
  phNxpUciHal_Runtime_Settings_t *rt_set = &nxpucihal_ctrl.rt_settings;
  rt_set->uwb_enable = true;
  rt_set->restricted_channel_mask = 0;
  rt_set->tx_power_offset = 0;

}

/*******************************************************************************
 * Function      phNxpUciHal_applyCountryCaps
 *
 * Description   Creates supported channel's and Tx power TLV format for
 *specific country code  and updates map.
 *
 * Returns       void
 *
 *******************************************************************************/
static void phNxpUciHal_applyCountryCaps(const char country_code[2],
    const uint8_t *cc_resp, uint32_t cc_resp_len,
    uint8_t *cc_data, uint32_t *cc_data_len)
{
  phNxpUciHal_Runtime_Settings_t *rt_set = &nxpucihal_ctrl.rt_settings;

  phNxpUciHal_resetRuntimeSettings();

  uint16_t idx = 1; // first byte = number countries
  bool country_code_found = false;

  while (idx < cc_resp_len) {
    uint8_t tag = cc_resp[idx++];
    uint8_t len = cc_resp[idx++];

    if (country_code_found) {
      switch (tag) {
      case UWB_ENABLE_TAG:
        if (len == 1) {
          rt_set->uwb_enable = cc_resp[idx];
          NXPLOG_UCIHAL_D("CountryCaps uwb_enable = %u", cc_resp[idx]);
        }
        break;
      case CHANNEL_5_TAG:
        if (len == 1 && !cc_resp[idx]) {
          rt_set->restricted_channel_mask |= 1<< 5;
          NXPLOG_UCIHAL_D("CountryCaps channel 5 support = %u", cc_resp[idx]);
        }
        break;
      case CHANNEL_9_TAG:
        if (len == 1 && !cc_resp[idx]) {
          rt_set->restricted_channel_mask |= 1<< 9;
          NXPLOG_UCIHAL_D("CountryCaps channel 9 support = %u", cc_resp[idx]);
        }
        break;
      case TX_POWER_TAG:
        if (len == 2) {
          rt_set->tx_power_offset = (short)((cc_resp[idx + 0]) | (((cc_resp[idx + 1]) << RMS_TX_POWER_SHIFT) & 0xFF00));
          NXPLOG_UCIHAL_D("CountryCaps tx_power_offset = %d", rt_set->tx_power_offset);

          phNxpUciHal_setCalibParamTxPower();
        }
        break;
      default:
        break;
      }
    }
    if (tag == COUNTRY_CODE_TAG) {
      country_code_found = (cc_resp[idx + 0] == country_code[0]) && (cc_resp[idx + 1] == country_code[1]);
    }
    idx += len;
  }

  // Force UWB disabled even COUNTRY_CODE_CAPS provides it as enabled
  if (!is_valid_country_code(country_code) && rt_set->uwb_enable) {
    NXPLOG_UCIHAL_E("UWB is enabled by COUNTRY-CODE_CAPS with invalid country code %c%c,"
                    " forcing disabled", country_code[0], country_code[1]);
    rt_set->uwb_enable = false;
  }

  // consist up 'cc_data' TLVs
  uint8_t fira_channels = 0xff;
  if (rt_set->restricted_channel_mask & (1 << 5))
    fira_channels &= CHANNEL_5_MASK;
  if (rt_set->restricted_channel_mask & (1 << 9))
    fira_channels &= CHANNEL_9_MASK;

  uint8_t ccc_channels = 0;
  if (!(rt_set->restricted_channel_mask & (1 << 5)))
    ccc_channels |= 0x01;
  if (!(rt_set->restricted_channel_mask & (1 << 9)))
    ccc_channels |= 0x02;

  uint8_t index = 0;
  if ((index + 3) <= *cc_data_len) {
    cc_data[index++] = UWB_CHANNELS;
    cc_data[index++] = 0x01;
    cc_data[index++] = fira_channels;
  }

  if ((index + 3) <= *cc_data_len) {
    cc_data[index++] = CCC_UWB_CHANNELS;
    cc_data[index++] = 0x01;
    cc_data[index++] = ccc_channels;
  }
  *cc_data_len = index;
}

/*******************************************************************************
 * Function      phNxpUciHal_send_dev_status_ntf
 *
 * Description   send device status notification
 *
 * Returns       void
 *
 *******************************************************************************/
static void phNxpUciHal_send_dev_status_ntf() {
 NXPLOG_UCIHAL_D("phNxpUciHal_send_dev_status_ntf ");
 nxpucihal_ctrl.rx_data_len = 5;
 static uint8_t rsp_data[5] = {0x60, 0x01, 0x00, 0x01, 0xFF};
 (*nxpucihal_ctrl.p_uwb_stack_data_cback)(nxpucihal_ctrl.rx_data_len, rsp_data);
}

/*******************************************************************************
 * Function      phNxpUciHal_is_retry_required
 *
 * Description   UCI command retry check
 *
 * Returns       true/false
 *
 *******************************************************************************/
static bool phNxpUciHal_is_retry_required(uint8_t uci_octet0) {
 bool isRetryRequired = false, isChained_cmd = false, isData_Msg = false;
 isChained_cmd = (bool)((uci_octet0 & UCI_PBF_ST_CONT) >> UCI_PBF_SHIFT);
 isData_Msg = ((uci_octet0 & UCI_MT_MASK) >> UCI_MT_SHIFT) == UCI_MT_DATA;
 isRetryRequired = isChained_cmd | isData_Msg;
 return isRetryRequired;
}

/******************************************************************************
 * Function         phNxpUciHal_updateTxPower
 *
 * Description      This function updates the tx antenna power
 *
 * Returns          true/false
 *
 ******************************************************************************/
static void phNxpUciHal_updateTxPower(void)
{
  phNxpUciHal_Runtime_Settings_t *rt_set = &nxpucihal_ctrl.rt_settings;

  if (rt_set->tx_power_offset == 0)
    return;

  if (gtx_power.empty())
    return;

  uint8_t index = 2;  // channel + param ID

  if (gtx_power[index++]) { // number of entries
    uint8_t num_of_antennas = gtx_power[index++];
    while (num_of_antennas--) {
      index += 3; // antenna Id(1) + Peak Tx(2)
      long tx_power_long = gtx_power[index]  | ((gtx_power[index + 1] << RMS_TX_POWER_SHIFT) & 0xFF00);
      tx_power_long += rt_set->tx_power_offset;

      // long to 16bit little endian
      if (tx_power_long < 0)
        tx_power_long = 0;
      uint16_t tx_power_u16 = (uint16_t)tx_power_long;
      gtx_power[index++] = tx_power_u16 & 0xff;
      gtx_power[index++] = tx_power_u16 >> RMS_TX_POWER_SHIFT;
    }
  }
}

/*******************************************************************************
 * Function      phNxpUciHal_processCalibParamTxPowerPerAntenna
 *
 * Description  Stores Tx power set during calibration
 *
 * Returns      void
 *
 *******************************************************************************/
void phNxpUciHal_processCalibParamTxPowerPerAntenna(const uint8_t *p_data, uint16_t data_len)
{
  phNxpUciHal_Runtime_Settings_t *rt_set = &nxpucihal_ctrl.rt_settings;

  // RMS Tx power -> Octet [4, 5] in calib data
  NXPLOG_UCIHAL_D("phNxpUciHal_processCalibParamTxPowerPerAntenna %d", rt_set->tx_power_offset);

  gtx_power = std::move(std::vector<uint8_t> {p_data + UCI_MSG_HDR_SIZE, p_data + data_len});

  phNxpUciHal_updateTxPower();

  memcpy(&nxpucihal_ctrl.p_cmd_data[UCI_MSG_HDR_SIZE], gtx_power.data(),  gtx_power.size());
}

/******************************************************************************
 * Function         phNxpUciHal_setCalibParamTxPower
 *
 * Description      This function sets the TX power
 *
 * Returns          true/false
 *
 ******************************************************************************/
static bool phNxpUciHal_setCalibParamTxPower(void)
{
  phNxpUciHal_updateTxPower();

  // GID : 0xF / OID : 0x21
  std::vector<uint8_t> packet{0x2f, 0x21, 0x00, 0x00};
  packet.insert(packet.end(), gtx_power.begin(), gtx_power.end());
  packet[3] = gtx_power.size();

  tHAL_UWB_STATUS status = phNxpUciHal_send_ext_cmd(packet.size(), packet.data());
  if (status != UWBSTATUS_SUCCESS) {
      NXPLOG_UCIHAL_D("%s: send failed", __func__);
  }

  gtx_power.clear();
  gRMS_tx_power.clear();

  return true;
}

/******************************************************************************
 * Function         phNxpUciHal_hw_reset_ntf_timeout_cb
 *
 * Description      Timer call back function
 *
 * Returns          None
 *
 ******************************************************************************/
static void phNxpUciHal_hw_reset_ntf_timeout_cb(uint32_t timerId,
                                                void *pContext) {
 UNUSED(timerId);
 UNUSED(pContext);
 NXPLOG_UCIHAL_E("phNxpUciHal_hw_reset_ntf_timeout_cb!!!");
 tHAL_UWB_STATUS status;

 status = phOsalUwb_Timer_Stop(hwResetTimer);
 if (UWBSTATUS_SUCCESS == status) {
      NXPLOG_UCIHAL_D("Response timer stopped");
 } else {
      NXPLOG_UCIHAL_E("Response timer stop ERROR!!!");
 }
 pthread_cond_signal(&extNxpucihal_ctrl.mCondVar);
 extNxpucihal_ctrl.isThermalRecoveryOngoing = false;

 return;
}

/******************************************************************************
 * Function         phNxpUciHal_handle_thermal_error_status
 *
 * Description     This function handles the core generic error ntf with status
 *                 temperature exceeded(0x54)
 *
 * Returns          return uwb status, On success
 *                  update the acutual state of operation in arg pointer
 *
 ******************************************************************************/
tHAL_UWB_STATUS phNxpUciHal_handle_thermal_error_status() {

 tHAL_UWB_STATUS status = UWBSTATUS_FAILED;
 extNxpucihal_ctrl.isThermalRecoveryOngoing = true;

 /* Send FW crash NTF to upper layer for triggering MW recovery */
 nxpucihal_ctrl.rx_data_len = 5;
 nxpucihal_ctrl.p_rx_data[0] = 0x60;
 nxpucihal_ctrl.p_rx_data[1] = 0x01;
 nxpucihal_ctrl.p_rx_data[2] = 0x00;
 nxpucihal_ctrl.p_rx_data[3] = 0x01;
 nxpucihal_ctrl.p_rx_data[4] = 0xFF;
 (*nxpucihal_ctrl.p_uwb_stack_data_cback)(nxpucihal_ctrl.rx_data_len,
                                          nxpucihal_ctrl.p_rx_data);

 hwResetTimer = phOsalUwb_Timer_Create();

 status = phOsalUwb_Timer_Start(hwResetTimer, HAL_HW_RESET_NTF_TIMEOUT,
                                &phNxpUciHal_hw_reset_ntf_timeout_cb, NULL);

 if (UWBSTATUS_SUCCESS == status) {
      nxpucihal_ctrl.isRecoveryTimerStarted = true;
      NXPLOG_UCIHAL_E("HW Reset Ntf timer started");
 } else {
      NXPLOG_UCIHAL_E("HW Reset Ntf timer not started!!!");
      pthread_cond_signal(&extNxpucihal_ctrl.mCondVar);
      return UWBSTATUS_FAILED;
 }
 return UWBSTATUS_SUCCESS;
}

/******************************************************************************
 * Function         phNxpUciHal_clear_thermal_error_status
 *
 * Description     This function is used to clear thermal error context.
 *
 * Returns          void
 *
 ******************************************************************************/
static void phNxpUciHal_clear_thermal_error_status() {
 tHAL_UWB_STATUS status = UWBSTATUS_FAILED;
 nxpucihal_ctrl.isSkipPacket = 1;
 NXPLOG_UCIHAL_D("received hw reset ntf");
 pthread_cond_signal(&extNxpucihal_ctrl.mCondVar);
 extNxpucihal_ctrl.isThermalRecoveryOngoing = false;
 if (nxpucihal_ctrl.isRecoveryTimerStarted == true) {
      status = phOsalUwb_Timer_Stop(hwResetTimer);
      if (UWBSTATUS_SUCCESS == status) {
        NXPLOG_UCIHAL_D("Response timer stopped");
      } else {
        NXPLOG_UCIHAL_E("Response timer stop ERROR!!!");
      }
 }
}

struct ReadOtpCookie {
  ReadOtpCookie(uint8_t param_id, uint8_t *buffer, size_t len) :
    m_valid(false), m_id(param_id), m_buffer(buffer), m_len(len) {
  }
  std::atomic_bool  m_valid;
  uint8_t m_id;
  uint8_t *m_buffer;
  size_t  m_len;
};

/******************************************************************************
 * Function         otp_read_data
 *
 * Description      Read OTP calibration data
 *
 * Returns          true on success
 *
 ******************************************************************************/
static bool otp_read_data(const uint8_t channel, const uint8_t param_id, uint8_t *buffer, size_t len)
{
  ReadOtpCookie cookie(param_id, buffer, len);

  if (phNxpUciHal_init_cb_data(&nxpucihal_ctrl.calib_data_ntf_wait, &cookie) != UWBSTATUS_SUCCESS) {
    NXPLOG_UCIHAL_E("Failed to create call back data for reading otp");
    return false;
  }

  // READ_CALIB_DATA_CMD
  std::vector<uint8_t> packet{(UCI_MT_CMD << UCI_MT_SHIFT) | UCI_GID_PROPRIETARY_0X0A, UCI_MSG_READ_CALIB_DATA, 0x00, 0x03};
  packet.push_back(channel);
  packet.push_back(0x01);      // OTP read option
  packet.push_back(param_id);

  tHAL_UWB_STATUS status = phNxpUciHal_send_ext_cmd(packet.size(), packet.data());
  if (status != UWBSTATUS_SUCCESS) {
    goto fail_otp_read_data;
  }

  phNxpUciHal_sem_timed_wait_sec(&nxpucihal_ctrl.calib_data_ntf_wait, 3);
  if (!cookie.m_valid) {
    goto fail_otp_read_data;
  }

  phNxpUciHal_cleanup_cb_data(&nxpucihal_ctrl.calib_data_ntf_wait);
  return true;

fail_otp_read_data:
  phNxpUciHal_cleanup_cb_data(&nxpucihal_ctrl.calib_data_ntf_wait);
  NXPLOG_UCIHAL_E("Failed to read OTP data id=%u", param_id);
  return false;
}

/******************************************************************************
 * Function         phNxpUciHal_parseCoreDeviceInfoRsp
 *
 * Description      This function parse Core device Info response.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpUciHal_parseCoreDeviceInfoRsp(const uint8_t *p_rx_data, size_t rx_data_len)
{
  uint8_t index = 13; // Excluding the header and Versions
  uint8_t paramId = 0;
  uint8_t length = 0;

  if (nxpucihal_ctrl.isDevInfoCached) {
    return;
  }

  NXPLOG_UCIHAL_D("phNxpUciHal_parseCoreDeviceInfoRsp Enter..");

  if (rx_data_len > sizeof(nxpucihal_ctrl.dev_info_resp)) {
      NXPLOG_UCIHAL_E("FIXME: CORE_DEVICE_INFO_RSP buffer overflow!");
      return;
  }

  memcpy(nxpucihal_ctrl.dev_info_resp, nxpucihal_ctrl.p_rx_data, nxpucihal_ctrl.rx_data_len);

  uint8_t len = p_rx_data[index++];
  len = len + index;
  while (index < len) {
    paramId = p_rx_data[index++];
    length = p_rx_data[index++];
    if (paramId == DEVICE_NAME_PARAM_ID && length >= 6) {
      /* SR100T --> T */
      switch(p_rx_data[index + 5]) {
      case DEVICE_TYPE_SR1xxS:
        nxpucihal_ctrl.device_type = DEVICE_TYPE_SR1xxS;
        break;
      case DEVICE_TYPE_SR1xxT:
        nxpucihal_ctrl.device_type = DEVICE_TYPE_SR1xxT;
        break;
      default:
        nxpucihal_ctrl.device_type = DEVICE_TYPE_UNKNOWN;
        break;
      }
    } else if (paramId == FW_VERSION_PARAM_ID && length >= 3) {
      nxpucihal_ctrl.fw_version.major_version = p_rx_data[index];
      nxpucihal_ctrl.fw_version.minor_version = p_rx_data[index + 1];
      nxpucihal_ctrl.fw_version.rc_version = p_rx_data[index + 2];
    } else if (paramId == FW_BOOT_MODE_PARAM_ID && length >= 1) {
      nxpucihal_ctrl.fw_boot_mode = p_rx_data[index];
      break;
    }
    index = index + length;
  }
  NXPLOG_UCIHAL_D("phNxpUciHal_parseCoreDeviceInfoRsp: Device Info cached.");
  nxpucihal_ctrl.isDevInfoCached = true;
  return;
}


/******************************************************************************
 * Function         phNxpUciHal_process_response
 *
 * Description      This function handles all the propriotory hal
 *functionalities.
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpUciHal_process_response() {
 tHAL_UWB_STATUS status;

 uint8_t mt, gid, oid, pbf;

 mt = (nxpucihal_ctrl.p_rx_data[0]  & UCI_MT_MASK) >> UCI_MT_SHIFT;
 gid = nxpucihal_ctrl.p_rx_data[0] & UCI_GID_MASK;
 oid = nxpucihal_ctrl.p_rx_data[1] & UCI_OID_MASK;
 pbf = (nxpucihal_ctrl.p_rx_data[0] & UCI_PBF_MASK) >> UCI_PBF_SHIFT;

 if (((gid == UCI_GID_CORE) && (oid == UCI_MSG_CORE_GENERIC_ERROR_NTF)) &&
   ((nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET] == UCI_STATUS_THERMAL_RUNAWAY) ||
   (nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET] == UCI_STATUS_LOW_VBAT))) {
   nxpucihal_ctrl.isSkipPacket = 1;
   status = phNxpUciHal_handle_thermal_error_status();
   if (status != UCI_STATUS_OK) {
     NXPLOG_UCIHAL_E("phNxpUciHal_handle_thermal_error_status failed");
   }
 }

 if ((gid == UCI_GID_CORE) && (oid == UCI_MSG_CORE_DEVICE_STATUS_NTF) &&
     (nxpucihal_ctrl.p_rx_data[UCI_RESPONSE_STATUS_OFFSET] ==
      UCI_STATUS_HW_RESET)) {
      phNxpUciHal_clear_thermal_error_status();
 }

  // Remember CORE_DEVICE_INFO_RSP
  if (mt == UCI_MT_RSP && (gid == UCI_GID_CORE) && (oid == UCI_MSG_CORE_DEVICE_INFO)) {
    if (pbf) {
      // FIXME: Fix the whole logic if this really happens
      NXPLOG_UCIHAL_E("FIXME: CORE_DEVICE_INFO_RSP is fragmented!");
    } else {
      phNxpUciHal_parseCoreDeviceInfoRsp(nxpucihal_ctrl.p_rx_data, nxpucihal_ctrl.rx_data_len);
    }
  }

  //
  // Handle NXP_READ_CALIB_DATA_NTF
  //
  if ((mt == UCI_MT_NTF) && (gid == UCI_GID_PROPRIETARY_0X0A) && (oid == UCI_MSG_READ_CALIB_DATA)) {
    // READ_CALIB_DATA_NTF: status(1), length-of-payload(1), payload(N)
    const uint8_t plen = nxpucihal_ctrl.p_rx_data[3]; // payload-length
    const uint8_t *p = &nxpucihal_ctrl.p_rx_data[4];  // payload
    ReadOtpCookie *cookie = (ReadOtpCookie*)nxpucihal_ctrl.calib_data_ntf_wait.pContext;

    if (!cookie) {
      NXPLOG_UCIHAL_E("Otp read: unexpected OTP read.");
    } else if (cookie->m_valid) {
      // cookie is already valid
      NXPLOG_UCIHAL_E("Otp read: unexpected OTP read, param-id=0x%x", cookie->m_id);
    } else if (plen < 2) {
      NXPLOG_UCIHAL_E("Otp read: bad payload length %u", plen);
    } else if (p[0] != UCI_STATUS_OK) {
      NXPLOG_UCIHAL_E("Otp read: bad status=0x%x", nxpucihal_ctrl.p_rx_data[4]);
    } else if (p[1] != cookie->m_len) {
      NXPLOG_UCIHAL_E("Otp read: size mismatch %u (expected %zu for param 0x%x)",
        p[1], cookie->m_len, cookie->m_id);
    } else {
      memcpy(cookie->m_buffer, &p[2], cookie->m_len);
      cookie->m_valid = true;
      SEM_POST(&nxpucihal_ctrl.calib_data_ntf_wait);
    }
  }
}

// SW defined data structures
typedef enum {
  // 6 bytes
  // [1:0] cap1 [3:2] cap2 [5:4] gm current control
  EXTCAL_PARAM_CLK_ACCURACY   = 0x1,    // xtal

  // 3n + 1 bytes
  // [0] n, number of entries +  n * { [0] antenna-id [1:0] RX delay(Q14.2) }
  EXTCAL_PARAM_RX_ANT_DELAY   = 0x2,    // ant_delay

  // 5N + 1 bytes
  // [0]: n, number of entries + n * { [0] antenna-id [2:1] delta-peak [4:3] id-rms }
  EXTCAL_PARAM_TX_POWER       = 0x3,    // tx_power

  // channel independent
  // 1 byte
  //  b0: enable/disable DDFS tone generation (default off)
  //  b1: enable/disable DC suppression (default off)
  EXTCAL_PARAM_TX_BASE_BAND_CONTROL   = 0x101,  // ddfs_enable, dc_suppress

  // channel independent (raw data contains channel info)
  // bytes array
  EXTCAL_PARAM_DDFS_TONE_CONFIG       = 0x102,  // ddfs_tone_config

  // channel independent
  // byte array
  EXTCAL_PARAM_TX_PULSE_SHAPE         = 0x103,  // tx_pulse_shape
} extcal_param_id_t;

// Based on NXP SR1xx UCI v2.0.5
// current HAL impl only supports "xtal" read from otp
// others should be existed in .conf files

static tHAL_UWB_STATUS sr1xx_set_calibration(uint8_t channel, const std::vector<uint8_t> &tlv)
{
  // SET_CALIBRATION_CMD header: GID=0xF OID=0x21
  std::vector<uint8_t> packet({ (0x20 | UCI_GID_PROPRIETARY_0X0F), UCI_MSG_SET_DEVICE_CALIBRATION, 0x00, 0x00});

  // use 9 for channel-independent parameters
  if (!channel) {
    channel = 9;
  }
  packet.push_back(channel);
  packet.insert(packet.end(), tlv.begin(), tlv.end());
  packet[3] = packet.size() - 4;
  return phNxpUciHal_send_ext_cmd(packet.size(), packet.data());
}

static tHAL_UWB_STATUS sr1xx_set_conf(const std::vector<uint8_t> &tlv)
{
  // SET_CALIBRATION_CMD header: GID=0xF OID=0x21
  std::vector<uint8_t> packet({ (0x20 | UCI_GID_CORE), UCI_MSG_CORE_SET_CONFIG, 0x00, 0x00});
  packet.push_back(1);  // number of parameters
  packet.insert(packet.end(), tlv.begin(), tlv.end());
  packet[3] = packet.size() - 4;
  return phNxpUciHal_send_ext_cmd(packet.size(), packet.data());
}

/******************************************************************************
 * Function         phNxpUciHal_apply_calibration
 *
 * Description      Send calibration/dev-config command to UWBS
 *
 * Parameters       id        - parameter id
 *                  channel   - channel number. 0 if it's channel independentt
 *                  data      - parameter value
 *                  data_len  - length of data
 *
 * Returns          0 : success, <0 : errno
 *
 ******************************************************************************/
tHAL_UWB_STATUS sr1xx_apply_calibration(extcal_param_id_t id, const uint8_t ch, const uint8_t *data, size_t data_len)
{
  // Device Calibration
  const uint8_t UCI_PARAM_ID_RF_CLK_ACCURACY_CALIB    = 0x01;
  const uint8_t UCI_PARAM_ID_RX_ANT_DELAY_CALIB       = 0x02;
  const uint8_t UCI_PARAM_ID_TX_POWER_PER_ANTENNA     = 0x04;

  // Device Configurations
  const uint16_t UCI_PARAM_ID_TX_BASE_BAND_CONFIG     = 0xe426;
  const uint16_t UCI_PARAM_ID_DDFS_TONE_CONFIG        = 0xe427;
  const uint16_t UCI_PARAM_ID_TX_PULSE_SHAPE_CONFIG   = 0xe428;

  switch (id) {
  case EXTCAL_PARAM_CLK_ACCURACY:
    {
      if (data_len != 6) {
        return UWBSTATUS_FAILED;
      }

      std::vector<uint8_t> tlv;
      // Tag
      tlv.push_back(UCI_PARAM_ID_RF_CLK_ACCURACY_CALIB);
      // Length
      tlv.push_back((uint8_t)data_len + 1);
      // Value
      tlv.push_back(3); // number of register (must be 0x03)
      tlv.insert(tlv.end(), data, data + data_len);

      return sr1xx_set_calibration(ch, tlv);
    }
  case EXTCAL_PARAM_RX_ANT_DELAY:
    {
      if (!ch || data_len < 1 || !data[0] || (data[0] * 3) != (data_len - 1)) {
        return UWBSTATUS_FAILED;
      }

      std::vector<uint8_t> tlv;
      // Tag
      tlv.push_back(UCI_PARAM_ID_RX_ANT_DELAY_CALIB);
      // Length
      tlv.push_back((uint8_t)data_len);
      // Value
      tlv.insert(tlv.end(), data, data + data_len);

      return sr1xx_set_calibration(ch, tlv);
    }
  case EXTCAL_PARAM_TX_POWER:
    {
      if (!ch || data_len < 1 || !data[0] || (data[0] * 5) != (data_len - 1)) {
        return UWBSTATUS_FAILED;
      }

      std::vector<uint8_t> tlv;
      // Tag
      tlv.push_back(UCI_PARAM_ID_TX_POWER_PER_ANTENNA);
      // Length
      tlv.push_back((uint8_t)data_len);
      // Value
      tlv.insert(tlv.end(), data, data + data_len);

      return sr1xx_set_calibration(ch, tlv);
    }
  case EXTCAL_PARAM_TX_BASE_BAND_CONTROL:
    {
      if (data_len != 1) {
        return UWBSTATUS_FAILED;
      }

      std::vector<uint8_t> tlv;
      // Tag
      tlv.push_back(UCI_PARAM_ID_TX_BASE_BAND_CONFIG >> 8);
      tlv.push_back(UCI_PARAM_ID_TX_BASE_BAND_CONFIG & 0xff);
      // Length
      tlv.push_back(1);
      // Value
      tlv.push_back(data[0]);

      return sr1xx_set_conf(tlv);
    }
  case EXTCAL_PARAM_DDFS_TONE_CONFIG:
    {
      if (!data_len) {
        return UWBSTATUS_FAILED;
      }

      std::vector<uint8_t> tlv;
      // Tag
      tlv.push_back(UCI_PARAM_ID_DDFS_TONE_CONFIG >> 8);
      tlv.push_back(UCI_PARAM_ID_DDFS_TONE_CONFIG & 0xff);
      // Length
      tlv.push_back(data_len);
      // Value
      tlv.insert(tlv.end(), data, data + data_len);

      return sr1xx_set_conf(tlv);
    }
  case EXTCAL_PARAM_TX_PULSE_SHAPE:
    {
      if (!data_len) {
        return UWBSTATUS_FAILED;
      }

      std::vector<uint8_t> tlv;
      // Tag
      tlv.push_back(UCI_PARAM_ID_TX_PULSE_SHAPE_CONFIG >> 8);
      tlv.push_back(UCI_PARAM_ID_TX_PULSE_SHAPE_CONFIG & 0xff);
      // Length
      tlv.push_back(data_len);
      // Value
      tlv.insert(tlv.end(), data, data + data_len);

      return sr1xx_set_conf(tlv);
    }
  default:
    NXPLOG_UCIHAL_E("Unsupported parameter: 0x%x", id);
    return UWBSTATUS_FAILED;
  }
}

tHAL_UWB_STATUS sr1xx_read_otp(extcal_param_id_t id, uint8_t *data, size_t data_len, size_t *retlen)
{
  switch(id) {
  case EXTCAL_PARAM_CLK_ACCURACY:
    {
      const size_t param_len = 6;
      uint8_t otp_xtal_data[3];

      if (data_len < param_len) {
        NXPLOG_UCIHAL_E("Requested RF_CLK_ACCURACY_CALIB with %zu bytes (expected >= %zu)", data_len, param_len);
        return UWBSTATUS_FAILED;
      }
      if (!otp_read_data(0x09, OTP_ID_XTAL_CAP_GM_CTRL, otp_xtal_data, sizeof(otp_xtal_data))) {
        NXPLOG_UCIHAL_E("Failed to read OTP XTAL_CAP_GM_CTRL");
        return UWBSTATUS_FAILED;
      }
      memset(data, 0, param_len);
      // convert OTP_ID_XTAL_CAP_GM_CTRL to EXTCAL_PARAM_RX_ANT_DELAY
      data[0] = otp_xtal_data[0]; // cap1
      data[2] = otp_xtal_data[1]; // cap2
      data[4] = otp_xtal_data[2]; // gm_current_control (default: 0x30)
      *retlen = param_len;
      return UWBSTATUS_SUCCESS;
    }
    break;
  default:
    NXPLOG_UCIHAL_E("Unsupported otp parameter %d", id);
    return UWBSTATUS_FAILED;
  }
}

// Channels
const static uint8_t cal_channels[] = {5, 6, 8, 9};

static void extcal_do_xtal(void)
{
  int ret;

  // RF_CLK_ACCURACY_CALIB (otp supported)
  // parameters: cal.otp.xtal=0|1, cal.xtal=X
  uint8_t otp_xtal_flag = 0;
  uint8_t xtal_data[32];
  size_t xtal_data_len = 0;

  if (NxpConfig_GetNum("cal.otp.xtal", &otp_xtal_flag, 1) && otp_xtal_flag) {
    sr1xx_read_otp(EXTCAL_PARAM_CLK_ACCURACY, xtal_data, sizeof(xtal_data), &xtal_data_len);
  }
  if (!xtal_data_len) {
    long retlen = 0;
    if (NxpConfig_GetByteArray("cal.xtal", xtal_data, sizeof(xtal_data), &retlen)) {
      xtal_data_len = retlen;
    }
  }

  if (xtal_data_len) {
    NXPLOG_UCIHAL_D("Apply CLK_ACCURARY (len=%zu, from-otp=%c)", xtal_data_len, otp_xtal_flag ? 'y' : 'n');

    ret = sr1xx_apply_calibration(EXTCAL_PARAM_CLK_ACCURACY, 0, xtal_data, xtal_data_len);

    if (ret != UWBSTATUS_SUCCESS) {
      NXPLOG_UCIHAL_E("Failed to apply CLK_ACCURACY (len=%zu, from-otp=%c)",
          xtal_data_len, otp_xtal_flag ? 'y' : 'n');
    }
  }
}

static void extcal_do_ant_delay(void)
{
  std::bitset<8> rx_antenna_mask(nxpucihal_ctrl.cal_rx_antenna_mask);
  const uint8_t n_rx_antennas = rx_antenna_mask.size();

  // RX_ANT_DELAY_CALIB
  // parameter: cal.ant<N>.ch<N>.ant_delay=X
  // N(1) + N * {AntennaID(1), Rxdelay(Q14.2)}
  if (n_rx_antennas) {
    for (auto ch : cal_channels) {
      std::vector<uint8_t> entries;
      uint8_t n_entries = 0;

      for (auto i = 0; i < n_rx_antennas; i++) {
        if (!rx_antenna_mask[i])
          continue;

        const uint8_t ant_id = i + 1;
        uint16_t delay_value;
        char key[32];
        std::snprintf(key, sizeof(key), "cal.ant%u.ch%u.ant_delay", ant_id, ch);

        if (!NxpConfig_GetNum(key, &delay_value, 2))
          continue;

        NXPLOG_UCIHAL_D("Apply RX_ANT_DELAY_CALIB: %s = %u", key, delay_value);
        entries.push_back(ant_id);
        // Little Endian
        entries.push_back(delay_value & 0xff);
        entries.push_back(delay_value >> 8);
        n_entries++;
      }

      if (!n_entries)
        continue;

      entries.insert(entries.begin(), n_entries);
      tHAL_UWB_STATUS ret = sr1xx_apply_calibration(EXTCAL_PARAM_RX_ANT_DELAY, ch, entries.data(), entries.size());
      if (ret != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("Failed to apply RX_ANT_DELAY for channel %u", ch);
      }
    }
  }
}

static void extcal_do_tx_power(void)
{
  std::bitset<8> tx_antenna_mask(nxpucihal_ctrl.cal_tx_antenna_mask);
  const uint8_t n_tx_antennas = tx_antenna_mask.size();

  // TX_POWER
  // parameter: cal.ant<N>.ch<N>.tx_power={...}
  if (n_tx_antennas) {
    for (auto ch : cal_channels) {
      std::vector<uint8_t> entries;
      uint8_t n_entries = 0;

      for (auto i = 0; i < n_tx_antennas; i++) {
        if (!tx_antenna_mask[i])
          continue;

        char key[32];
        const uint8_t ant_id = i + 1;
        std::snprintf(key, sizeof(key), "cal.ant%u.ch%u.tx_power", ant_id, ch);

        uint8_t power_value[32];
        long retlen = 0;
        if (!NxpConfig_GetByteArray(key, power_value, sizeof(power_value), &retlen)) {
          continue;
        }

        NXPLOG_UCIHAL_D("Apply TX_POWER: %s = { %lu bytes }", key, retlen);
        entries.push_back(ant_id);
        entries.insert(entries.end(), power_value, power_value + retlen);
        n_entries++;
      }

      if (!n_entries)
        continue;

      entries.insert(entries.begin(), n_entries);
      tHAL_UWB_STATUS ret = sr1xx_apply_calibration(EXTCAL_PARAM_TX_POWER, ch, entries.data(), entries.size());
      if (ret != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("Failed to apply TX_POWER for channel %u", ch);
      }
    }
  }
}

static void extcal_do_tx_pulse_shape(void)
{
  // parameters: cal.tx_pulse_shape={...}
  long retlen = 0;
  uint8_t data[64];

  if (NxpConfig_GetByteArray("cal.tx_pulse_shape", data, sizeof(data), &retlen) && retlen) {
      NXPLOG_UCIHAL_D("Apply TX_PULSE_SHAPE: data = { %lu bytes }", retlen);

      tHAL_UWB_STATUS ret = sr1xx_apply_calibration(EXTCAL_PARAM_TX_PULSE_SHAPE, 0, data, (size_t)retlen);
      if (ret != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("Failed to apply TX_PULSE_SHAPE.");
      }
  }
}

static void extcal_do_tx_base_band(void)
{
  // TX_BASE_BAND_CONTROL, DDFS_TONE_CONFIG
  // parameters: cal.ddfs_enable=1|0, cal.dc_suppress=1|0, ddfs_tone_config={...}
  uint8_t ddfs_enable = 0, dc_suppress = 0;
  uint8_t ddfs_tone[256];
  long retlen = 0;
  tHAL_UWB_STATUS ret;

  if (NxpConfig_GetNum("cal.ddfs_enable", &ddfs_enable, 1)) {
    NXPLOG_UCIHAL_D("Apply TX_BASE_BAND_CONTROL: ddfs_enable=%u", ddfs_enable);
  }
  if (NxpConfig_GetNum("cal.dc_suppress", &dc_suppress, 1)) {
    NXPLOG_UCIHAL_D("Apply TX_BASE_BAND_CONTROL: dc_suppress=%u", dc_suppress);
  }

  // DDFS_TONE_CONFIG
  if (ddfs_enable) {
    if (!NxpConfig_GetByteArray("cal.ddfs_tone_config", ddfs_tone, sizeof(ddfs_tone), &retlen) || !retlen) {
      NXPLOG_UCIHAL_E("cal.ddfs_tone_config is not supplied while cal.ddfs_enable=1, ddfs was not enabled.");
      ddfs_enable = 0;
    } else {
      NXPLOG_UCIHAL_D("Apply DDFS_TONE_CONFIG: ddfs_tone_config = { %lu bytes }", retlen);

      ret = sr1xx_apply_calibration(EXTCAL_PARAM_DDFS_TONE_CONFIG, 0, ddfs_tone, (size_t)retlen);
      if (ret != UWBSTATUS_SUCCESS) {
        NXPLOG_UCIHAL_E("Failed to apply DDFS_TONE_CONFIG, ddfs was not enabled.");
        ddfs_enable = 0;
      }
    }
  }

  // TX_BASE_BAND_CONTROL
  {
    NXPLOG_UCIHAL_D("Apply TX_BASE_BAND_CONTROL: ddfs_enable=%u, dc_suppress=%u", ddfs_enable, dc_suppress);

    uint8_t flag = 0;
    if (ddfs_enable)
      flag |= 0x01;
    if (dc_suppress)
      flag |= 0x02;
    ret = sr1xx_apply_calibration(EXTCAL_PARAM_TX_BASE_BAND_CONTROL, 0, &flag, 1);
    if (ret) {
      NXPLOG_UCIHAL_E("Failed to apply TX_BASE_BAND_CONTROL");
    }
  }
}

static void extcal_do_restrictions(const char country_code[2])
{
  phNxpUciHal_Runtime_Settings_t *rt_set = &nxpucihal_ctrl.rt_settings;

  phNxpUciHal_resetRuntimeSettings();

  uint16_t mask= 0;
  if (NxpConfig_GetNum("cal.restricted_channels", &mask, sizeof(mask))) {
    NXPLOG_UCIHAL_D("Restriction flag, restricted channel mask=0x%x", mask);
    rt_set->restricted_channel_mask = mask;
  }

  uint8_t uwb_disable = 0;
  if (NxpConfig_GetNum("cal.uwb_disable", &uwb_disable, sizeof(uwb_disable))) {
    NXPLOG_UCIHAL_D("Restriction flag, uwb_disable=%u", uwb_disable);
    rt_set->uwb_enable = !uwb_disable;
  }

  // Force UWB disabled even ExtCal provides it as enabled
  if (!is_valid_country_code(country_code)) {
    NXPLOG_UCIHAL_E("UWB is enabled by ExtCal with invalid country code %c%c,"
                    " forcing disabled",
                    country_code[0], country_code[1]);
    rt_set->uwb_enable = false;
  }
}

/******************************************************************************
 * Function         phNxpUciHal_extcal_handle_coreinit
 *
 * Description      Apply additional core device settings
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpUciHal_extcal_handle_coreinit(void)
{
  // read rx_aantenna_mask, tx_antenna_mask
  uint8_t rx_antenna_mask_n = 0x1;
  uint8_t tx_antenna_mask_n = 0x1;
  if (!NxpConfig_GetNum("cal.rx_antenna_mask", &rx_antenna_mask_n, 1)) {
      NXPLOG_UCIHAL_E("cal.rx_antenna_mask is not specified, use default 0x%x", rx_antenna_mask_n);
  }
  if (!NxpConfig_GetNum("cal.tx_antenna_mask", &tx_antenna_mask_n, 1)) {
      NXPLOG_UCIHAL_E("cal.tx_antenna_mask is not specified, use default 0x%x", tx_antenna_mask_n);
  }
  nxpucihal_ctrl.cal_rx_antenna_mask = rx_antenna_mask_n;
  nxpucihal_ctrl.cal_tx_antenna_mask = tx_antenna_mask_n;

  extcal_do_xtal();
  extcal_do_ant_delay();
}

extern bool isCountryCodeMapCreated;

/******************************************************************************
 * Function         phNxpUciHal_handle_set_country_code
 *
 * Description      Apply per-country settings
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpUciHal_handle_set_country_code(const char country_code[2])
{
  //
  // TODO: stop all active session which are affected by new country code
  // TODO: delay per-country calibrations when there's active sessions (or juststop them?)
  //
  NXPLOG_UCIHAL_D("Apply country code %c%c", country_code[0], country_code[1]);

  phNxpUciHal_Runtime_Settings_t *rt_set = &nxpucihal_ctrl.rt_settings;

  if (!is_valid_country_code(country_code)) {
    NXPLOG_UCIHAL_D("Country code %c%c is invalid, disable UWB", country_code[0], country_code[1]);
  }

  if (NxpConfig_SetCountryCode(country_code)) {
    // Load 'COUNTRY_CODE_CAPS' and apply it to 'conf_map'
    uint8_t cc_caps[UCI_MAX_DATA_LEN];
    long retlen = 0;
    if (NxpConfig_GetByteArray(NAME_NXP_UWB_COUNTRY_CODE_CAPS, cc_caps, sizeof(cc_caps), &retlen) && retlen) {
      NXPLOG_UCIHAL_D("COUNTRY_CODE_CAPS is provided.");
      isCountryCodeMapCreated = false;

      uint32_t cc_caps_len = retlen;
      uint8_t cc_data[UCI_MAX_DATA_LEN];
      uint32_t cc_data_len = sizeof(cc_data);
      phNxpUciHal_applyCountryCaps(country_code, cc_caps, cc_caps_len, cc_data, &cc_data_len);

      if (get_conf_map(cc_data, cc_data_len)) {
        isCountryCodeMapCreated = true;
        NXPLOG_UCIHAL_D("Country code caps loaded");
      }
    } else {
      NXPLOG_UCIHAL_D("COUNTRY_CODE_CAPS was not provided.");
    }

    // per-country extra calibrations are only triggered when 'COUNTRY_CODE_CAPS' is not provided
    if (!isCountryCodeMapCreated) {
      NXPLOG_UCIHAL_D("Apply per-country extra calibrations");
      extcal_do_restrictions(country_code);

      if (rt_set->uwb_enable) {
        extcal_do_tx_power();
        extcal_do_tx_pulse_shape();
        extcal_do_tx_base_band();
      }
    }
  }

  // send country code response to upper layer
  nxpucihal_ctrl.rx_data_len = 5;
  static uint8_t rsp_data[5] = { 0x4c, 0x01, 0x00, 0x01 };
  if (rt_set->uwb_enable) {
    rsp_data[4] = UWBSTATUS_SUCCESS;
  } else {
    rsp_data[4] = UCI_STATUS_CODE_ANDROID_REGULATION_UWB_OFF;
  }
  (*nxpucihal_ctrl.p_uwb_stack_data_cback)(nxpucihal_ctrl.rx_data_len, rsp_data);
}
