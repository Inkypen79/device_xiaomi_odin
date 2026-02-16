/*
 * Copyright 2012-2019 NXP
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
 * UWB Status Values - Function Return Codes
 */

#ifndef PHUWBSTATUS_H
#define PHUWBSTATUS_H

#include <phUwbTypes.h>
#include <uci_defs.h>

/* Internally required by PHUWBSTVAL. */
#define PHUWBSTSHL8 (8U)
/* Required by PHUWBSTVAL. */
#define PHUWBSTBLOWER ((tHAL_UWB_STATUS)(0x00FFU))

/*
 *  UWB Status Composition Macro
 *
 *  This is the macro which must be used to compose status values.
 *
 *  phUwbCompID Component ID, as defined in phUwbCompId.h .
 *  phUwbStatus Status values, as defined in phUwbStatus.h .
 *
 *  The macro is not required for the UWBSTATUS_SUCCESS value.
 *  This is the only return value to be used directly.
 *  For all other values it shall be used in assignment and conditional
 * statements, e.g.:
 *     tHAL_UWB_STATUS status = PHUWBSTVAL(phUwbCompID, phUwbStatus); ...
 *     if (status == PHUWBSTVAL(phUwbCompID, phUwbStatus)) ...
 */
#define PHUWBSTVAL(phUwbCompID, phUwbStatus)               \
  (((phUwbStatus) == (UWBSTATUS_SUCCESS))                  \
       ? (UWBSTATUS_SUCCESS)                               \
       : ((((tHAL_UWB_STATUS)(phUwbStatus)) & (PHUWBSTBLOWER)) | \
          (((uint16_t)(phUwbCompID)) << (PHUWBSTSHL8))))

/*
 * PHUWBSTATUS
 * Get grp_retval from Status Code
 */
#define PHUWBSTATUS(phUwbStatus) ((phUwbStatus)&0x00FFU)
#define PHUWBCID(phUwbStatus) (((phUwbStatus)&0xFF00U) >> 8)

/*
 *  Status Codes
 *
 *  Generic Status codes for the UWB components. Combined with the Component ID
 *  they build the value (status) returned by each function.
 *  Example:
 *      grp_comp_id "Component ID" -  e.g. 0x10, plus
 *      status code as listed in this file - e.g. 0x03
 *      result in a status value of 0x0003.
 */

/*
 * The function indicates successful completion
 */
#define UWBSTATUS_SUCCESS UCI_STATUS_OK

/*
 *  The function indicates successful completion
 */
#define UWBSTATUS_OK (UCI_STATUS_OK)


/*
 * Device specifier/handle value is invalid for the operation
 */
#define UWBSTATUS_INVALID_DEVICE (0x0001)

/*
 * A non-blocking function returns this immediately to indicate
 * that an internal operation is in progress
 */
#define UWBSTATUS_PENDING (0x0002)

/*
 * A board communication error occurred
 * (e.g. Configuration went wrong)
 */
#define UWBSTATUS_BOARD_COMMUNICATION_ERROR (0x0003)

/*
 * At least one parameter could not be properly interpreted
 */
#define UWBSTATUS_INVALID_PARAMETER (UCI_STATUS_INVALID_PARAM)

/*
 * Not enough resources Memory, Timer etc(e.g. allocation failed.)
 */
#define UWBSTATUS_INSUFFICIENT_RESOURCES (0x0005)

/*
 * Invalid State of the particular state machine
 */
#define UWBSTATUS_INVALID_STATE (0x0006)

/*
 * This Layer is Not initialized, hence initialization required.
 */
#define UWBSTATUS_NOT_INITIALISED (0x0007)

/*
 * The Layer is already initialized, hence initialization repeated.
 */
#define UWBSTATUS_ALREADY_INITIALISED (0x0008)

/*
 * The operation is currently not possible or not allowed
 */
#define UWBSTATUS_NOT_ALLOWED (0x0009)

/*
 * FW version error while performing FW download,
 * FW major version mismatch (cannot downgrade FW major version) or FW version
 * already upto date
 * User may be trying to flash Mobile FW on top of Infra FW, which is not
 * allowed
 * Download appropriate version of FW
 */
#define UWBSTATUS_FW_VERSION_ERROR (0x000A)

/*
 *  The system is busy with the previous operation.
 */
#define UWBSTATUS_BUSY (0x000B)

/* NDEF Mapping error codes */

/* Read operation failed */
#define UWBSTATUS_READ_FAILED (0x000C)

/*
 * Write operation failed
 */
#define UWBSTATUS_WRITE_FAILED (0x000D)

/*
 * Response Time out for the control message(UWBC not responded)
 */
#define UWBSTATUS_RESPONSE_TIMEOUT (0x000E)

/*
 * The function/command has been aborted
 */
#define UWBSTATUS_CMD_ABORTED (0x000F)

/*
 * Shutdown in progress, cannot handle the request at this time.
 */
#define UWBSTATUS_SHUTDOWN (0x0010)

/*
 * Invalid handle for the operation
 */
#define UWBSTATUS_INVALID_HANDLE (0x0011)

/*
 * Requested command is not supported
 */
#define UWBSTATUS_COMMAND_NOT_SUPPORTED (0x0012)

/*
 * Requested Retransmit command
 */
#define UWBSTATUS_COMMAND_RETRANSMIT (0x0013)

/*
 * File Not Found error
 */
#define UWBSTATUS_FILE_NOT_FOUND (0x0014)

/*
 * Invalid Command Length
 */
#define UWBSTATUS_INVALID_COMMAND_LENGTH (0x0015)

/*
 * Status code for failure
 */
#define UWBSTATUS_FAILED (0x00FF)

#endif /* PHUWBSTATUS_H */
