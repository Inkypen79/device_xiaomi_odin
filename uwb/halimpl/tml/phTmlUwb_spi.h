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

/* Basic type definitions */
#include <phUwbTypes.h>
#include <phTmlUwb.h>

#define SRXXX_MAGIC 0xEA
#define SRXXX_SET_PWR _IOW(SRXXX_MAGIC, 0x01, uint32_t)
#define SRXXX_SET_FWD _IOW(SRXXX_MAGIC, 0x02, uint32_t)
#define SRXXX_ESE_RESET _IOW(SRXXX_MAGIC, 0x03, uint32_t)
#define SRXXX_GET_THROUGHPUT _IOW(SRXXX_MAGIC, 0x04, uint32_t)

#define PWR_DISABLE               0
#define PWR_ENABLE                1
#define ABORT_READ_PENDING        2

#define NORMAL_MODE_HEADER_LEN    4
#define NORMAL_MODE_LEN_OFFSET    3

#define EXTENDED_SIZE_LEN_OFFSET  1
#define UCI_EXTENDED_PKT_MASK     0xC0
#define UCI_EXTENDED_SIZE_SHIFT   6
#define UCI_NORMAL_PKT_SIZE       0
#define UCI_EXT_PKT_SIZE_512B     1
#define UCI_EXT_PKT_SIZE_1K       2
#define UCI_EXT_PKT_SIZE_2K       3

#define UCI_PKT_SIZE_512B         512
#define UCI_PKT_SIZE_1K           1024
#define UCI_PKT_SIZE_2K           2048

/* Function declarations */
void phTmlUwb_spi_close(void* pDevHandle);
tHAL_UWB_STATUS phTmlUwb_spi_open_and_configure(pphTmlUwb_Config_t pConfig,
                                          void** pLinkHandle);
int phTmlUwb_spi_read(void* pDevHandle, uint8_t* pBuffer, int nNbBytesToRead);
int phTmlUwb_spi_write(void* pDevHandle, uint8_t* pBuffer, int nNbBytesToWrite);
int phTmlUwb_Spi_Ioctl(void* pDevHandle, phTmlUwb_ControlCode_t cmd, long arg);
