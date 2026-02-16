/******************************************************************************
 *
 *  Copyright 2018-2023 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
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
#ifndef ANDROID_HARDWARE_HAL_NXPUWB_V1_0_H
#define ANDROID_HARDWARE_HAL_NXPUWB_V1_0_H
#include <aidl/android/hardware/uwb/UwbEvent.h>
#include <aidl/android/hardware/uwb/UwbStatus.h>
#include <string>
#include <vector>

using ::aidl::android::hardware::uwb::UwbEvent;
using ::aidl::android::hardware::uwb::UwbStatus;

enum {
  HAL_UWB_STATUS_OK = (int32_t)UwbStatus::OK,
  HAL_UWB_STATUS_ERR_TRANSPORT = (int32_t)UwbStatus::ERR_TRANSPORT,
  HAL_UWB_STATUS_ERR_CMD_TIMEOUT = (int32_t)UwbStatus::ERR_CMD_TIMEOUT
};

enum {
  HAL_UWB_OPEN_CPLT_EVT = (int32_t)UwbEvent::OPEN_CPLT,
  HAL_UWB_CLOSE_CPLT_EVT = (int32_t)UwbEvent::CLOSE_CPLT,
  HAL_UWB_INIT_CPLT_EVT = (int32_t)UwbEvent::POST_INIT_CPLT,
  HAL_UWB_ERROR_EVT = (int32_t)UwbEvent::ERROR
};

#endif  // ANDROID_HARDWARE_HAL_NXPUWB_V1_0_H
