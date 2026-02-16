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
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include <phUwbStatus.h>
#include <phNxpLog.h>
#include <phTmlUwb_spi.h>
#include <string.h>
#include "phNxpUciHal_utils.h"
#include "phNxpUciHal.h"
/*********************** Global Variables *************************************/
/* UCI HAL Control structure */
extern phNxpUciHal_Control_t nxpucihal_ctrl;

/*******************************************************************************
**
** Function         phTmlUwb_spi_open_and_configure
**
** Description      Open and configure SR100
**
** Parameters       pConfig     - hardware information
**                  pLinkHandle - device handle
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - open_and_configure operation success
**                  UWBSTATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
tHAL_UWB_STATUS phTmlUwb_spi_open_and_configure(pphTmlUwb_Config_t pConfig,
                                          void** pLinkHandle) {
  int nHandle;

  NXPLOG_TML_D("Opening port=%s\n", pConfig->pDevName);
  /* open port */
  nHandle = open(pConfig->pDevName, O_RDWR);
  if (nHandle < 0) {
    NXPLOG_TML_E("_spi_open() Failed: retval %x", nHandle);
    *pLinkHandle = NULL;
    return UWBSTATUS_INVALID_DEVICE;
  }

  *pLinkHandle = (void*)((intptr_t)nHandle);

  /*Reset SR100 */
  phTmlUwb_Spi_Ioctl((void*)((intptr_t)nHandle), phTmlUwb_SetPower, 0);
  usleep(1000);
  phTmlUwb_Spi_Ioctl((void*)((intptr_t)nHandle), phTmlUwb_SetPower, 1);
  usleep(10000);

  return UWBSTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         phTmlUwb_spi_write
**
** Description      Writes requested number of bytes from given buffer into
**                  SR100
**
** Parameters       pDevHandle       - valid device handle
**                  pBuffer          - buffer for read data
**                  nNbBytesToWrite  - number of bytes requested to be written
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - write operation failure
**
*******************************************************************************/
int phTmlUwb_spi_write(void* pDevHandle, uint8_t* pBuffer,
                       int nNbBytesToWrite) {
  int ret;
  int numWrote = 0;
  int numByteWrite = 0;

  if (NULL == pDevHandle) {
    NXPLOG_TML_E("_spi_write() device is null");
    return -1;
  }
  numByteWrite  = NORMAL_MODE_HEADER_LEN;

  ret = write((intptr_t)pDevHandle, pBuffer, nNbBytesToWrite);
  if (ret > 0) {
    NXPLOG_TML_D("_spi_write()_1 ret : %x", ret);
    numWrote = ret;
  } else {
    NXPLOG_TML_D("_spi_write()_1 failed : %d", ret);
    return -1;
  }
  return numWrote;
}

/*******************************************************************************
**
** Function         phTmlUwb_spi_read
**
** Description      Reads requested number of bytes from SR100 device into
**                  given buffer
**
** Parameters       pDevHandle       - valid device handle
**                  pBuffer          - buffer for read data
**                  nNbBytesToRead   - number of bytes requested to be read
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*******************************************************************************/
int phTmlUwb_spi_read(void* pDevHandle, uint8_t* pBuffer, int nNbBytesToRead) {
  int ret_Read;
  uint16_t totalBtyesToRead = 0;

  UNUSED(nNbBytesToRead);
  if (NULL == pDevHandle) {
    NXPLOG_TML_E("_spi_read() error handle");
    return -1;
  }
  totalBtyesToRead = NORMAL_MODE_HEADER_LEN;
  /*Just requested 3 bytes header here but driver will get the header + payload and returns*/
  ret_Read = read((intptr_t)pDevHandle, pBuffer, totalBtyesToRead);
  if (ret_Read < 0) {
     NXPLOG_TML_E("_spi_read() error: %d", ret_Read);
    return -1;
  } else if((nxpucihal_ctrl.fw_dwnld_mode) && ((0xFF == pBuffer[0]) || ((0x00 == pBuffer[0]) && (0x00 == pBuffer[3])))) {
      NXPLOG_TML_E("_spi_read() error: Invalid UCI packet");
      /* To Avoid spurious interrupt after FW download */
      return 0;
  }
  //nxpucihal_ctrl.cir_dump_len = ret_Read - NORMAL_MODE_HEADER_LEN;
  return ret_Read;
}

/*******************************************************************************
**
** Function         phTmlUwb_Spi_Ioctl
**
** Description      Reset SR100, using VEN pin
**
** Parameters       pDevHandle     - valid device handle
**                  level          - reset level
**
** Returns           0   - reset operation success
**                  -1   - reset operation failure
**
*******************************************************************************/
int phTmlUwb_Spi_Ioctl(void* pDevHandle, phTmlUwb_ControlCode_t eControlCode , long arg) {
  NXPLOG_TML_D("phTmlUwb_Spi_Ioctl(), cmd %d,  arg %ld", eControlCode, arg);
  int ret = 1;
  if (NULL == pDevHandle) {
    return -1;
  }
  switch(eControlCode){
    case phTmlUwb_SetPower:
      ioctl((intptr_t)pDevHandle, SRXXX_SET_PWR, arg);
      break;
    case phTmlUwb_EnableFwdMode:
      ioctl((intptr_t)pDevHandle, SRXXX_SET_FWD, arg);
      break;
    case phTmlUwb_EnableThroughPut:
      //ioctl((intptr_t)pDevHandle, SRXXX_GET_THROUGHPUT, arg);
      break;
    case phTmlUwb_EseReset:
      ioctl((intptr_t)pDevHandle, SRXXX_ESE_RESET, arg);
      break;
    default:
      NXPLOG_TML_D("phTmlUwb_Spi_Ioctl(), Invalid command");
      ret = -1;
  }
  return ret;
}

/*******************************************************************************
**
** Function         phTmlUwb_spi_close
**
** Description      Closes SR100 device
**
** Parameters       pDevHandle - device handle
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_spi_close(void* pDevHandle) {
  if (NULL != pDevHandle) {
    close((intptr_t)pDevHandle);
  }

  return;
}
