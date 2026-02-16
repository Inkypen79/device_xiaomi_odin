/*
 * Copyright 2012-2020 NXP
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

/*
 * UWB Component ID Values - Used for Function Return Codes
 */

#ifndef PHUWBCOMPID_H
#define PHUWBCOMPID_H

/*
 *  Component IDs
 *
 *  IDs for all UWB components. Combined with the Status Code they build the
 * value (status)
 *  returned by each function.
 *
 *  ID Number Spaces:
 *  - 01..1F: HAL
 *  - 20..3F: UWB-MW (Local Device)
 *  - 40..5F: UWB-MW (Remote Device)
 *  .
 *
 *         The value CID_UWB_NONE does not exist for Component IDs. Do not use
 * this value except
 *         for UWBSTATUS_SUCCESS. The enumeration function uses CID_UWB_NONE
 *         to mark unassigned "References".
 */
/* Unassigned or doesn't apply (see #UWBSTATUS_SUCCESS) */
#define CID_UWB_NONE 0x00
#define CID_UWB_TML 0x01 /* Transport Mapping Layer */
#define CID_UWB_LLC 0x07 /* Logical Link Control Layer */
/* UWB Controller(UWBC) Interface Layer */
#define CID_UWB_UCI 0x08
/* Firmware Download Management Layer */
#define CID_UWB_DNLD 0x09
#define CID_UWB_HAL 0x10 /* Hardware Abstraction Layer */
/* Operating System Abstraction Layer*/
#define CID_UWB_OSAL CID_UWB_NONE
#define CID_FRI_UWB_OVR_HAL 0x20       /* UWB-Device, HAL-based */

#endif /* PHUWBCOMPID_H */
