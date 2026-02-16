/*
 * Copyright 2012-2020,2022 NXP
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

#ifndef _PHNXPUCIHAL_ADAPTATION_H_
#define _PHNXPUCIHAL_ADAPTATION_H_

//#include <android/hardware/uwb/1.0/IUwb.h>
//#include <android/hardware/uwb/1.0/types.h>

typedef uint8_t uwb_event_t;
typedef uint8_t uwb_status_t;

/*`
 * The callback passed in from the UWB stack that the HAL
 * can use to pass events back to the stack.
 */
typedef void(uwb_stack_callback_t)(uwb_event_t event,
                                   uwb_status_t event_status);

/*
 * The callback passed in from the UWB stack that the HAL
 * can use to pass incomming data to the stack.
 */
typedef void(uwb_stack_data_callback_t)(uint16_t data_len, uint8_t* p_data);

/* NXP HAL functions */
uint16_t phNxpUciHal_open(uwb_stack_callback_t* p_cback,
                     uwb_stack_data_callback_t* p_data_cback);
uint16_t phNxpUciHal_write(uint16_t data_len, const uint8_t* p_data);
uint16_t phNxpUciHal_close();
uint16_t phNxpUciHal_coreInitialization();
uint16_t phNxpUciHal_sessionInitialization(uint32_t sessionId);

#endif /* _PHNXPUCIHAL_ADAPTATION_H_ */
