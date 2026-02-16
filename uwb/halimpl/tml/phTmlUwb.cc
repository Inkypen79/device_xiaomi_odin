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

#include <phDal4Uwb_messageQueueLib.h>
#include <phNxpLog.h>
#include <phNxpUciHal_utils.h>
#include <phOsalUwb_Timer.h>
#include <phTmlUwb.h>
#include <phTmlUwb_spi.h>
#include <phNxpUciHal.h>
#include <errno.h>

extern phNxpUciHal_Control_t nxpucihal_ctrl;

/*
 * Duration of Timer to wait after sending an Uci packet
 */
#define PHTMLUWB_MAXTIME_RETRANSMIT (200U)
#define MAX_WRITE_RETRY_COUNT 0x03
/* Value to reset variables of TML  */
#define PH_TMLUWB_RESET_VALUE (0x00)

/* Indicates a Initial or offset value */
#define PH_TMLUWB_VALUE_ONE (0x01)

/* Initialize Context structure pointer used to access context structure */
phTmlUwb_Context_t* gpphTmlUwb_Context = NULL;

/* Local Function prototypes */
static tHAL_UWB_STATUS phTmlUwb_StartThread(void);
static void phTmlUwb_CleanUp(void);
static void phTmlUwb_ReadDeferredCb(void* pParams);
static void phTmlUwb_WriteDeferredCb(void* pParams);
static void* phTmlUwb_TmlReaderThread(void* pParam);
static void* phTmlUwb_TmlWriterThread(void* pParam);

extern void setDeviceHandle(void* pDevHandle);

static void phTmlUwb_WaitWriteComplete(void);
static void phTmlUwb_SignalWriteComplete(void);
static int phTmlUwb_WaitReadInit(void);
static int phTmlUwb_ReadAbortInit(void);

/* Function definitions */

/*******************************************************************************
**
** Function         phTmlUwb_Init
**
** Description      Provides initialization of TML layer and hardware interface
**                  Configures given hardware interface and sends handle to the
**                  caller
**
** Parameters       pConfig - TML configuration details as provided by the upper
**                            layer
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - initialization successful
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_FAILED - initialization failed (for example,
**                                     unable to open hardware interface)
**                  UWBSTATUS_INVALID_DEVICE - device has not been opened or has
**                                             been disconnected
**
*******************************************************************************/
tHAL_UWB_STATUS phTmlUwb_Init(pphTmlUwb_Config_t pConfig) {
  tHAL_UWB_STATUS wInitStatus = UWBSTATUS_SUCCESS;

  /* Check if TML layer is already Initialized */
  if (NULL != gpphTmlUwb_Context) {
    /* TML initialization is already completed */
    wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_ALREADY_INITIALISED);
  }
  /* Validate Input parameters */
  else if ((NULL == pConfig) ||
           (PH_TMLUWB_RESET_VALUE == pConfig->dwGetMsgThreadId)) {
    /*Parameters passed to TML init are wrong */
    wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
  } else {
    /* Allocate memory for TML context */
    gpphTmlUwb_Context =
        (phTmlUwb_Context_t*)malloc(sizeof(phTmlUwb_Context_t));

    if (NULL == gpphTmlUwb_Context) {
      wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
    } else {
      /* Initialise all the internal TML variables */
      memset(gpphTmlUwb_Context, PH_TMLUWB_RESET_VALUE,
             sizeof(phTmlUwb_Context_t));
      /* Make sure that the thread runs once it is created */
      gpphTmlUwb_Context->bThreadDone = 1;

      /* Open the device file to which data is read/written */
      wInitStatus = phTmlUwb_spi_open_and_configure(
          pConfig, &(gpphTmlUwb_Context->pDevHandle));

      if (UWBSTATUS_SUCCESS != wInitStatus) {
        wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_DEVICE);
        gpphTmlUwb_Context->pDevHandle = NULL;
      } else {
        gpphTmlUwb_Context->tReadInfo.bEnable = 0;
        gpphTmlUwb_Context->tWriteInfo.bEnable = 0;
        gpphTmlUwb_Context->tReadInfo.bThreadBusy = false;
        gpphTmlUwb_Context->tWriteInfo.bThreadBusy = false;

        setDeviceHandle(gpphTmlUwb_Context->pDevHandle);  // To set device handle for FW download usecase

        if (0 != sem_init(&gpphTmlUwb_Context->rxSemaphore, 0, 0)) {
          wInitStatus = UWBSTATUS_FAILED;
        } else if (0 != sem_init(&gpphTmlUwb_Context->txSemaphore, 0, 0)) {
          wInitStatus = UWBSTATUS_FAILED;
        } else if(0 != phTmlUwb_WaitReadInit()) {
           wInitStatus = UWBSTATUS_FAILED;
        } else if(0 != phTmlUwb_ReadAbortInit()) {
           wInitStatus = UWBSTATUS_FAILED;
        } else if (0 != sem_init(&gpphTmlUwb_Context->postMsgSemaphore, 0, 0)) {
          wInitStatus = UWBSTATUS_FAILED;
        } else {
          sem_post(&gpphTmlUwb_Context->postMsgSemaphore);
          /* Start TML thread (to handle write and read operations) */
          if (UWBSTATUS_SUCCESS != phTmlUwb_StartThread()) {
            wInitStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
          } else {
            /* Store the Thread Identifier to which Message is to be posted */
            gpphTmlUwb_Context->dwCallbackThreadId =
                  pConfig->dwGetMsgThreadId;
            wInitStatus = UWBSTATUS_SUCCESS;
          }
        }
      }
    }
  }
  /* Clean up all the TML resources if any error */
  if (UWBSTATUS_SUCCESS != wInitStatus) {
    /* Clear all handles and memory locations initialized during init */
    phTmlUwb_CleanUp();
  }

  return wInitStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_StartThread
**
** Description      Initializes comport, reader and writer threads
**
** Parameters       None
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - threads initialized successfully
**                  UWBSTATUS_FAILED - initialization failed due to system error
**
*******************************************************************************/
static tHAL_UWB_STATUS phTmlUwb_StartThread(void) {
  tHAL_UWB_STATUS wStartStatus = UWBSTATUS_SUCCESS;
  void* h_threadsEvent = 0x00;
  int pthread_create_status = 0;

  /* Create Reader and Writer threads */
  pthread_create_status =
      pthread_create(&gpphTmlUwb_Context->readerThread, NULL,
                     &phTmlUwb_TmlReaderThread, (void*)h_threadsEvent);
  if (0 != pthread_create_status) {
    wStartStatus = UWBSTATUS_FAILED;
  } else {
    /*Start Writer Thread*/
    pthread_create_status =
        pthread_create(&gpphTmlUwb_Context->writerThread, NULL,
                       &phTmlUwb_TmlWriterThread, (void*)h_threadsEvent);
    if (0 != pthread_create_status) {
      wStartStatus = UWBSTATUS_FAILED;
    }
  }

  return wStartStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_TmlReaderThread
**
** Description      Read the data from the lower layer driver
**
** Parameters       pParam  - parameters for Writer thread function
**
** Returns          None
**
*******************************************************************************/
static void* phTmlUwb_TmlReaderThread(void* pParam) {
  tHAL_UWB_STATUS wStatus = UWBSTATUS_SUCCESS;
  int32_t dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;
  uint8_t temp[UCI_MAX_DATA_LEN];
  /* Transaction info buffer to be passed to Callback Thread */
  static phTmlUwb_TransactInfo_t tTransactionInfo;
  /* Structure containing Tml callback function and parameters to be invoked
     by the callback thread */
  static phLibUwb_DeferredCall_t tDeferredInfo;
  /* Initialize Message structure to post message onto Callback Thread */
  static phLibUwb_Message_t tMsg;
  UNUSED(pParam);
  NXPLOG_TML_D("SRxxx - Tml Reader Thread Started................\n");

  /* Writer thread loop shall be running till shutdown is invoked */
  while (gpphTmlUwb_Context->bThreadDone) {
    /* If Tml write is requested */
    /* Set the variable to success initially */
    wStatus = UWBSTATUS_SUCCESS;
    if(sem_wait(&gpphTmlUwb_Context->rxSemaphore)!=0){
        NXPLOG_TML_E("Failed to wait semaphore");
    }
    /* If Tml read is requested */
    if (1 == gpphTmlUwb_Context->tReadInfo.bEnable) {
      NXPLOG_TML_D("SRxxx - Read requested.....\n");
      /* Set the variable to success initially */
      wStatus = UWBSTATUS_SUCCESS;

      /* Variable to fetch the actual number of bytes read */
      dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;

      /* Read the data from the file onto the buffer */
      if (NULL != gpphTmlUwb_Context->pDevHandle) {
        NXPLOG_TML_D("SRxxx - Invoking SPI Read.....\n");
        dwNoBytesWrRd =
            phTmlUwb_spi_read(gpphTmlUwb_Context->pDevHandle, temp, UCI_MAX_DATA_LEN);
        if(gpphTmlUwb_Context->bThreadDone == 0) {
          return NULL;
        }

        if (-1 == dwNoBytesWrRd) {
          NXPLOG_TML_E("SRxxx - Error in SPI Read.....\n");
          pthread_mutex_lock(&gpphTmlUwb_Context->read_abort_lock);
          if(gpphTmlUwb_Context->is_read_abort) {
            pthread_cond_signal(&gpphTmlUwb_Context->read_abort_condition);
            gpphTmlUwb_Context->is_read_abort = false;
          }
          else {
            sem_post(&gpphTmlUwb_Context->rxSemaphore);
          }
          pthread_mutex_unlock(&gpphTmlUwb_Context->read_abort_lock);
        } else if (dwNoBytesWrRd > UCI_MAX_DATA_LEN) {
          NXPLOG_TML_E("Numer of bytes read exceeds the limit .....\n");
          sem_post(&gpphTmlUwb_Context->rxSemaphore);
        } else if(0 == dwNoBytesWrRd) {
          NXPLOG_TML_E("SRxxx - Empty packet Read, Ignore read and try new read...\n");
          pthread_mutex_lock(&gpphTmlUwb_Context->read_abort_lock);
          if(gpphTmlUwb_Context->is_read_abort) {
            pthread_cond_signal(&gpphTmlUwb_Context->read_abort_condition);
            gpphTmlUwb_Context->is_read_abort = false;
          }
          else {
            sem_post(&gpphTmlUwb_Context->rxSemaphore);
          }
          pthread_mutex_unlock(&gpphTmlUwb_Context->read_abort_lock);
        } else {
          memcpy(gpphTmlUwb_Context->tReadInfo.pBuffer, temp, dwNoBytesWrRd);

          NXPLOG_TML_D("SRxxx - SPI Read successful.....\n");
          gpphTmlUwb_Context->tReadInfo.bEnable = 0;
          if (gpphTmlUwb_Context->tWriteInfo.bThreadBusy) {
            NXPLOG_TML_D("Delay Read if write thread is busy");
          }
          /* Update the actual number of bytes read including header */
          gpphTmlUwb_Context->tReadInfo.wLength = (uint16_t)(dwNoBytesWrRd);
          phNxpUciHal_print_packet("RECV",
                                   gpphTmlUwb_Context->tReadInfo.pBuffer,
                                   gpphTmlUwb_Context->tReadInfo.wLength);

          dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;

          /* Fill the Transaction info structure to be passed to Callback
           * Function */
          tTransactionInfo.wStatus = wStatus;
          tTransactionInfo.pBuff = gpphTmlUwb_Context->tReadInfo.pBuffer;
          /* Actual number of bytes read is filled in the structure */
          tTransactionInfo.wLength = gpphTmlUwb_Context->tReadInfo.wLength;

          /* Read operation completed successfully. Post a Message onto Callback
           * Thread*/
          /* Prepare the message to be posted on User thread */
          tDeferredInfo.pCallback = &phTmlUwb_ReadDeferredCb;
          tDeferredInfo.pParameter = &tTransactionInfo;
          tMsg.eMsgType = PH_LIBUWB_DEFERREDCALL_MSG;
          tMsg.pMsgData = &tDeferredInfo;
          tMsg.Size = sizeof(tDeferredInfo);
          NXPLOG_TML_D("SRxxx - Posting read message.....\n");
          /* TML reader writer callback synchronization mutex lock --- START */
          pthread_mutex_lock(&gpphTmlUwb_Context->wait_busy_lock);
          if ((gpphTmlUwb_Context->gWriterCbflag == false) &&
            ((gpphTmlUwb_Context->tReadInfo.pBuffer[0] & 0x60) != 0x60)) {
            phTmlUwb_WaitWriteComplete();
          }
          /* TML reader writer callback synchronization mutex lock --- END */
          pthread_mutex_unlock(&gpphTmlUwb_Context->wait_busy_lock);
          phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, &tMsg);
        }
      } else {
        NXPLOG_TML_D("SRxxx -gpphTmlUwb_Context->pDevHandle is NULL");
      }
    } else {
      NXPLOG_TML_D("SRxxx - read request NOT enabled");
      usleep(10 * 1000);
    }
  } /* End of While loop */

  return NULL;
}

/*******************************************************************************
**
** Function         phTmlUwb_TmlWriterThread
**
** Description      Writes the requested data onto the lower layer driver
**
** Parameters       pParam  - context provided by upper layer
**
** Returns          None
**
*******************************************************************************/
static void* phTmlUwb_TmlWriterThread(void* pParam) {
  tHAL_UWB_STATUS wStatus = UWBSTATUS_SUCCESS;
  int32_t dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;
  /* Transaction info buffer to be passed to Callback Thread */
  static phTmlUwb_TransactInfo_t tTransactionInfo;
  /* Structure containing Tml callback function and parameters to be invoked
     by the callback thread */
  static phLibUwb_DeferredCall_t tDeferredInfo;
  /* Initialize Message structure to post message onto Callback Thread */
  static phLibUwb_Message_t tMsg;
  UNUSED(pParam);
  NXPLOG_TML_D("SRxxx - Tml Writer Thread Started................\n");

  /* Writer thread loop shall be running till shutdown is invoked */
  while (gpphTmlUwb_Context->bThreadDone) {
    NXPLOG_TML_D("SRxxx - Tml Writer Thread Running................\n");
    if (sem_wait(&gpphTmlUwb_Context->txSemaphore) != 0) {
      NXPLOG_TML_E("Failed to wait semaphore ");
    }
    /* If Tml write is requested */
    if (1 == gpphTmlUwb_Context->tWriteInfo.bEnable) {
      NXPLOG_TML_D("SRxxx - Write requested.....\n");
      /* Set the variable to success initially */
      wStatus = UWBSTATUS_SUCCESS;
      if (NULL != gpphTmlUwb_Context->pDevHandle) {
        gpphTmlUwb_Context->tWriteInfo.bEnable = 0;
        /* Variable to fetch the actual number of bytes written */
        dwNoBytesWrRd = PH_TMLUWB_RESET_VALUE;
        /* Write the data in the buffer onto the file */
        NXPLOG_TML_D("SRxxx - Invoking SPI Write.....\n");

        /* TML reader writer callback synchronization mutex lock --- START
         */
        pthread_mutex_lock(&gpphTmlUwb_Context->wait_busy_lock);
        gpphTmlUwb_Context->gWriterCbflag = false;
        dwNoBytesWrRd =
            phTmlUwb_spi_write(gpphTmlUwb_Context->pDevHandle,
                               gpphTmlUwb_Context->tWriteInfo.pBuffer,
                               gpphTmlUwb_Context->tWriteInfo.wLength);
        /* TML reader writer callback synchronization mutex lock --- END */
        pthread_mutex_unlock(&gpphTmlUwb_Context->wait_busy_lock);

        /* Try SPI Write Five Times, if it fails :*/
        if (-1 == dwNoBytesWrRd) {
          NXPLOG_TML_E("SRxxx - Error in SPI Write.....\n");
          wStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_FAILED);
        } else {
          phNxpUciHal_print_packet("SEND",
                                   gpphTmlUwb_Context->tWriteInfo.pBuffer,
                                   gpphTmlUwb_Context->tWriteInfo.wLength);
        }
        if (UWBSTATUS_SUCCESS == wStatus) {
          NXPLOG_TML_D("SRxxx - SPI Write successful.....\n");
          dwNoBytesWrRd = PH_TMLUWB_VALUE_ONE;
        }
        /* Fill the Transaction info structure to be passed to Callback Function
         */
        tTransactionInfo.wStatus = wStatus;
        tTransactionInfo.pBuff = gpphTmlUwb_Context->tWriteInfo.pBuffer;
        /* Actual number of bytes written is filled in the structure */
        tTransactionInfo.wLength = (uint16_t)dwNoBytesWrRd;

        /* Prepare the message to be posted on the User thread */
        tDeferredInfo.pCallback = &phTmlUwb_WriteDeferredCb;
        tDeferredInfo.pParameter = &tTransactionInfo;
        /* Write operation completed successfully. Post a Message onto Callback
         * Thread*/
        tMsg.eMsgType = PH_LIBUWB_DEFERREDCALL_MSG;
        tMsg.pMsgData = &tDeferredInfo;
        tMsg.Size = sizeof(tDeferredInfo);


        NXPLOG_TML_D("SRxxx - Posting Fresh Write message.....\n");
        phTmlUwb_DeferredCall(gpphTmlUwb_Context->dwCallbackThreadId, &tMsg);
        if (UWBSTATUS_SUCCESS == wStatus) {
          /* TML reader writer callback synchronization mutex lock --- START
             */
          pthread_mutex_lock(&gpphTmlUwb_Context->wait_busy_lock);
          gpphTmlUwb_Context->gWriterCbflag = true;
          phTmlUwb_SignalWriteComplete();
            /* TML reader writer callback synchronization mutex lock --- END */
          pthread_mutex_unlock(&gpphTmlUwb_Context->wait_busy_lock);
          }
      } else {
        NXPLOG_TML_D("SRxxx - gpphTmlUwb_Context->pDevHandle is NULL");
      }
    } else {
      NXPLOG_TML_D("SRxxx - Write request NOT enabled");
      usleep(10000);
    }

  } /* End of While loop */

  return NULL;
}

/*******************************************************************************
**
** Function         phTmlUwb_CleanUp
**
** Description      Clears all handles opened during TML initialization
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_CleanUp(void) {
  if (NULL == gpphTmlUwb_Context) {
    return;
  }
  if (NULL != gpphTmlUwb_Context->pDevHandle) {
    (void)phTmlUwb_Spi_Ioctl(gpphTmlUwb_Context->pDevHandle, phTmlUwb_SetPower, 0);
    gpphTmlUwb_Context->bThreadDone = 0;
  }
  sem_destroy(&gpphTmlUwb_Context->rxSemaphore);
  sem_destroy(&gpphTmlUwb_Context->txSemaphore);
  sem_destroy(&gpphTmlUwb_Context->postMsgSemaphore);
  pthread_mutex_destroy(&gpphTmlUwb_Context->wait_busy_lock);
  pthread_cond_destroy(&gpphTmlUwb_Context->wait_busy_condition);
  phTmlUwb_spi_close(gpphTmlUwb_Context->pDevHandle);
  gpphTmlUwb_Context->pDevHandle = NULL;
  /* Clear memory allocated for storing Context variables */
  free((void*)gpphTmlUwb_Context);
  /* Set the pointer to NULL to indicate De-Initialization */
  gpphTmlUwb_Context = NULL;

  return;
}

/*******************************************************************************
**
** Function         phTmlUwb_eSE_Reset
**
** Description      Invoke this API to reset Secure Element
**
** Parameters       None
**
** Returns          void
**
*******************************************************************************/
void phTmlUwb_eSE_Reset(void) {
  int status;
  if (NULL != gpphTmlUwb_Context->pDevHandle) {
    status = phTmlUwb_Spi_Ioctl(gpphTmlUwb_Context->pDevHandle, phTmlUwb_EseReset, 0);
    NXPLOG_TML_E("se reset status received %d",status);
  }
}
/*******************************************************************************
**
** Function         phTmlUwb_Shutdown
**
** Description      Uninitializes TML layer and hardware interface
**
** Parameters       None
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - TML configuration released successfully
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_FAILED - un-initialization failed (example: unable
**                                     to close interface)
**
*******************************************************************************/
tHAL_UWB_STATUS phTmlUwb_Shutdown(void) {
  tHAL_UWB_STATUS wShutdownStatus = UWBSTATUS_SUCCESS;

  /* Check whether TML is Initialized */
  if (NULL != gpphTmlUwb_Context) {
    /* Reset thread variable to terminate the thread */
    gpphTmlUwb_Context->bThreadDone = 0;
    /* Clear All the resources allocated during initialization */
    phTmlUwb_Spi_Ioctl(gpphTmlUwb_Context->pDevHandle, phTmlUwb_SetPower, ABORT_READ_PENDING);
    sem_post(&gpphTmlUwb_Context->rxSemaphore);
    usleep(1000);
    sem_post(&gpphTmlUwb_Context->txSemaphore);
    usleep(1000);
    sem_post(&gpphTmlUwb_Context->postMsgSemaphore);
    usleep(1000);
    if (0 != pthread_join(gpphTmlUwb_Context->readerThread, (void**)NULL)) {
      NXPLOG_TML_E("Fail to kill reader thread!");
    }
    if (0 != pthread_join(gpphTmlUwb_Context->writerThread, (void**)NULL)) {
      NXPLOG_TML_E("Fail to kill writer thread!");
    }
    NXPLOG_TML_D("bThreadDone == 0");

    phTmlUwb_CleanUp();
  } else {
    wShutdownStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
  }

  return wShutdownStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_Write
**
** Description      Asynchronously writes given data block to hardware
**                  interface/driver. Enables writer thread if there are no
**                  write requests pending. Returns successfully once writer
**                  thread completes write operation. Notifies upper layer using
**                  callback mechanism.
**
**                  NOTE:
**                  * it is important to post a message with id
**                    PH_TMLUWB_WRITE_MESSAGE to IntegrationThread after data
**                    has been written to SRxxx
**                  * if CRC needs to be computed, then input buffer should be
**                    capable to store two more bytes apart from length of
**                    packet
**
** Parameters       pBuffer - data to be sent
**                  wLength - length of data buffer
**                  pTmlWriteComplete - pointer to the function to be invoked
**                                      upon completion
**                  pContext - context provided by upper layer
**
** Returns          UWB status:
**                  UWBSTATUS_PENDING - command is yet to be processed
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_BUSY - write request is already in progress
**
*******************************************************************************/
tHAL_UWB_STATUS phTmlUwb_Write(uint8_t* pBuffer, uint16_t wLength,
                         pphTmlUwb_TransactCompletionCb_t pTmlWriteComplete,
                         void* pContext) {
  tHAL_UWB_STATUS wWriteStatus;

  /* Check whether TML is Initialized */

  if (NULL != gpphTmlUwb_Context) {
    if ((NULL != gpphTmlUwb_Context->pDevHandle) && (NULL != pBuffer) &&
        (PH_TMLUWB_RESET_VALUE != wLength) && (NULL != pTmlWriteComplete)) {
      if (!gpphTmlUwb_Context->tWriteInfo.bThreadBusy) {
        /* Setting the flag marks beginning of a Write Operation */
        gpphTmlUwb_Context->tWriteInfo.bThreadBusy = true;
        /* Copy the buffer, length and Callback function,
           This shall be utilized while invoking the Callback function in thread
           */
        gpphTmlUwb_Context->tWriteInfo.pBuffer = pBuffer;
        gpphTmlUwb_Context->tWriteInfo.wLength = wLength;
        gpphTmlUwb_Context->tWriteInfo.pThread_Callback = pTmlWriteComplete;
        gpphTmlUwb_Context->tWriteInfo.pContext = pContext;

        wWriteStatus = UWBSTATUS_PENDING;
        /* Set event to invoke Writer Thread */
        gpphTmlUwb_Context->tWriteInfo.bEnable = 1;
        sem_post(&gpphTmlUwb_Context->txSemaphore);
      } else {
        wWriteStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_BUSY);
      }
    } else {
      wWriteStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
    }
  } else {
    wWriteStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
  }

  return wWriteStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_Read
**
** Description      Asynchronously reads data from the driver
**                  Number of bytes to be read and buffer are passed by upper
**                  layer.
**                  Enables reader thread if there are no read requests pending
**                  Returns successfully once read operation is completed
**                  Notifies upper layer using callback mechanism
**
** Parameters       pBuffer - location to send read data to the upper layer via
**                            callback
**                  wLength - length of read data buffer passed by upper layer
**                  pTmlReadComplete - pointer to the function to be invoked
**                                     upon completion of read operation
**                  pContext - context provided by upper layer
**
** Returns          UWB status:
**                  UWBSTATUS_PENDING - command is yet to be processed
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_BUSY - read request is already in progress
**
*******************************************************************************/
tHAL_UWB_STATUS phTmlUwb_Read(uint8_t* pBuffer, uint16_t wLength,
                        pphTmlUwb_TransactCompletionCb_t pTmlReadComplete,
                        void* pContext) {
  tHAL_UWB_STATUS wReadStatus;

  /* Check whether TML is Initialized */
  if (NULL != gpphTmlUwb_Context) {
    if ((gpphTmlUwb_Context->pDevHandle != NULL) && (NULL != pBuffer) &&
        (PH_TMLUWB_RESET_VALUE != wLength) && (NULL != pTmlReadComplete)) {
      if (!gpphTmlUwb_Context->tReadInfo.bThreadBusy) {
        /* Setting the flag marks beginning of a Read Operation */
        gpphTmlUwb_Context->tReadInfo.bThreadBusy = true;
        /* Copy the buffer, length and Callback function,
           This shall be utilized while invoking the Callback function in thread
           */
        gpphTmlUwb_Context->tReadInfo.pBuffer = pBuffer;
        gpphTmlUwb_Context->tReadInfo.wLength = wLength;
        gpphTmlUwb_Context->tReadInfo.pThread_Callback = pTmlReadComplete;
        gpphTmlUwb_Context->tReadInfo.pContext = pContext;
        wReadStatus = UWBSTATUS_PENDING;

        /* Set event to invoke Reader Thread */
        if(gpphTmlUwb_Context->is_read_abort != true) {
          gpphTmlUwb_Context->tReadInfo.bEnable = 1; // To be enabled later
          sem_post(&gpphTmlUwb_Context->rxSemaphore); // To be enabled later
        }
      } else {
        wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_BUSY);
      }
    } else {
      wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_INVALID_PARAMETER);
    }
  } else {
    wReadStatus = PHUWBSTVAL(CID_UWB_TML, UWBSTATUS_NOT_INITIALISED);
  }

  return wReadStatus;
}

/*******************************************************************************
**
** Function         phTmlUwb_ReadAbort
**
** Description      Aborts pending read request (if any)
**
** Parameters       None
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - ongoing read operation aborted
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_NOT_INITIALIZED - TML layer is not initialized
**                  UWBSTATUS_BOARD_COMMUNICATION_ERROR - unable to cancel read
**                                                        operation
**
*******************************************************************************/
tHAL_UWB_STATUS phTmlUwb_ReadAbort(void) {
  gpphTmlUwb_Context->tReadInfo.bEnable = 0;

  /*Reset the flag to accept another Read Request */
  gpphTmlUwb_Context->tReadInfo.bThreadBusy = false;
  return UWBSTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         phTmlUwb_WriteAbort
**
** Description      Aborts pending write request (if any)
**
** Parameters       None
**
** Returns          UWB status:
**                  UWBSTATUS_SUCCESS - ongoing write operation aborted
**                  UWBSTATUS_INVALID_PARAMETER - at least one parameter is
**                                                invalid
**                  UWBSTATUS_NOT_INITIALIZED - TML layer is not initialized
**                  UWBSTATUS_BOARD_COMMUNICATION_ERROR - unable to cancel write
**                                                        operation
**
*******************************************************************************/
tHAL_UWB_STATUS phTmlUwb_WriteAbort(void) {

  gpphTmlUwb_Context->tWriteInfo.bEnable = 0;

  /* Reset the flag to accept another Write Request */
  gpphTmlUwb_Context->tWriteInfo.bThreadBusy = false;
  return UWBSTATUS_SUCCESS;
}

/*******************************************************************************
**
** Function         phTmlUwb_DeferredCall
**
** Description      Posts message on upper layer thread
**                  upon successful read or write operation
**
** Parameters       dwThreadId  - id of the thread posting message
**                  ptWorkerMsg - message to be posted
**
** Returns          None
**
*******************************************************************************/
void phTmlUwb_DeferredCall(uintptr_t dwThreadId,
                           phLibUwb_Message_t* ptWorkerMsg) {
  intptr_t bPostStatus;
  UNUSED(dwThreadId);
  /* Post message on the user thread to invoke the callback function */
  if (sem_wait(&gpphTmlUwb_Context->postMsgSemaphore) != 0) {
    NXPLOG_TML_E("phTmlUwb_DeferredCall: semaphore error");
  }
  bPostStatus =
      phDal4Uwb_msgsnd(gpphTmlUwb_Context->dwCallbackThreadId, ptWorkerMsg, 0);
  sem_post(&gpphTmlUwb_Context->postMsgSemaphore);
}

/*******************************************************************************
**
** Function         phTmlUwb_ReadDeferredCb
**
** Description      Read thread call back function
**
** Parameters       pParams - context provided by upper layer
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_ReadDeferredCb(void* pParams) {
  /* Transaction info buffer to be passed to Callback Function */
  phTmlUwb_TransactInfo_t* pTransactionInfo = (phTmlUwb_TransactInfo_t*)pParams;

  /* Reset the flag to accept another Read Request */
  gpphTmlUwb_Context->tReadInfo.bThreadBusy = false;
  gpphTmlUwb_Context->tReadInfo.pThread_Callback(
      gpphTmlUwb_Context->tReadInfo.pContext, pTransactionInfo);

  return;
}

/*******************************************************************************
**
** Function         phTmlUwb_WriteDeferredCb
**
** Description      Write thread call back function
**
** Parameters       pParams - context provided by upper layer
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_WriteDeferredCb(void* pParams) {
  /* Transaction info buffer to be passed to Callback Function */
  phTmlUwb_TransactInfo_t* pTransactionInfo = (phTmlUwb_TransactInfo_t*)pParams;

  /* Reset the flag to accept another Write Request */
  gpphTmlUwb_Context->tWriteInfo.bThreadBusy = false;
  gpphTmlUwb_Context->tWriteInfo.pThread_Callback(
      gpphTmlUwb_Context->tWriteInfo.pContext, pTransactionInfo);

  return;
}

/*******************************************************************************
**
** Function         phTmlUwb_WaitWriteComplete
**
** Description      wait function for reader thread
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_WaitWriteComplete(void) {
  int ret;
  struct timespec absTimeout;
  if (clock_gettime(CLOCK_MONOTONIC, &absTimeout) == -1) {
    NXPLOG_TML_E("Reader Thread clock_gettime failed");
  } else {
    absTimeout.tv_sec += 1; /*1 second timeout*/
    gpphTmlUwb_Context->wait_busy_flag = true;
    NXPLOG_TML_D("phTmlUwb_WaitWriteComplete - enter");
    ret = pthread_cond_timedwait(&gpphTmlUwb_Context->wait_busy_condition,
                                 &gpphTmlUwb_Context->wait_busy_lock,
                                 &absTimeout);
    if ((ret != 0) && (ret != ETIMEDOUT)) {
      NXPLOG_TML_E("Reader Thread wait failed");
    }
    NXPLOG_TML_D("phTmlUwb_WaitWriteComplete - exit");
  }
}

/*******************************************************************************
**
** Function         phTmlUwb_SignalWriteComplete
**
** Description      function to invoke reader thread
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
static void phTmlUwb_SignalWriteComplete(void) {
  int ret;
  if (gpphTmlUwb_Context->wait_busy_flag == true) {
    NXPLOG_TML_D("phTmlUwb_SignalWriteComplete - enter");
    gpphTmlUwb_Context->wait_busy_flag = false;
    ret = pthread_cond_signal(&gpphTmlUwb_Context->wait_busy_condition);
    if (ret) {
      NXPLOG_TML_E(" phTmlUwb_SignalWriteComplete failed, error = 0x%X", ret);
    }
    NXPLOG_TML_D("phTmlUwb_SignalWriteComplete - exit");
  }
}

/*******************************************************************************
**
** Function         phTmlUwb_WaitReadInit
**
** Description      init function for reader thread
**
** Parameters       None
**
** Returns          int
**
*******************************************************************************/
static int phTmlUwb_WaitReadInit(void) {
  int ret;
  pthread_condattr_t attr;
  pthread_condattr_init(&attr);
  pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  memset(&gpphTmlUwb_Context->wait_busy_condition, 0,
         sizeof(gpphTmlUwb_Context->wait_busy_condition));
  pthread_mutex_init(&gpphTmlUwb_Context->wait_busy_lock, NULL);
  ret = pthread_cond_init(&gpphTmlUwb_Context->wait_busy_condition, &attr);
  if (ret) {
    NXPLOG_TML_E(" phTmlUwb_WaitReadInit failed, error = 0x%X", ret);
  }
  return ret;
}

/*******************************************************************************
**
** Function         phTmlUwb_ReadAbortInit
**
** Description      init function read abort context
**
** Parameters       None
**
** Returns          int
**
*******************************************************************************/
static int phTmlUwb_ReadAbortInit(void) {
  int ret;
  pthread_condattr_t attr;
  pthread_condattr_init(&attr);
  pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  memset(&gpphTmlUwb_Context->read_abort_condition, 0,
         sizeof(gpphTmlUwb_Context->read_abort_condition));
  pthread_mutex_init(&gpphTmlUwb_Context->read_abort_lock, NULL);
  ret = pthread_cond_init(&gpphTmlUwb_Context->read_abort_condition, &attr);
  if (ret) {
    NXPLOG_TML_E(" phTmlUwb_ReadAbortInit failed, error = 0x%X", ret);
  }
  return ret;
}

/*******************************************************************************
**
** Function         phTmlUwb_Chip_Reset
**
** Description      Invoke this API to Chip enable/Disable
**
** Parameters       None
**
** Returns          void
**
*******************************************************************************/
void phTmlUwb_Chip_Reset(void){
  if (NULL != gpphTmlUwb_Context->pDevHandle) {
    phTmlUwb_Spi_Ioctl(gpphTmlUwb_Context->pDevHandle, phTmlUwb_SetPower, 0);
    usleep(1000);
    phTmlUwb_Spi_Ioctl(gpphTmlUwb_Context->pDevHandle, phTmlUwb_SetPower, 1);
  }
}

/*******************************************************************************
**
** Function         phTmlUwb_Spi_Reset
**
** Description      Invoke this API to reset spi
**
** Parameters       None
**
** Returns          void
**
*******************************************************************************/
void phTmlUwb_Spi_Reset(void) {
  int ret;
  struct timespec absTimeout;
  phTmlUwb_ReadAbort();
  if (clock_gettime(CLOCK_MONOTONIC, &absTimeout) == -1) {
    NXPLOG_TML_E("Reader Thread clock_gettime failed");
  }
  absTimeout.tv_sec += 1; /*1 second timeout*/
  pthread_mutex_lock(&gpphTmlUwb_Context->read_abort_lock);
  gpphTmlUwb_Context->is_read_abort = true;
  phTmlUwb_Spi_Ioctl(gpphTmlUwb_Context->pDevHandle, phTmlUwb_SetPower, ABORT_READ_PENDING);
  phTmlUwb_Chip_Reset();
  ret = pthread_cond_timedwait(&gpphTmlUwb_Context->read_abort_condition,
                                 &gpphTmlUwb_Context->read_abort_lock,
                                 &absTimeout);
  if ((ret != 0) && (ret != ETIMEDOUT)) {
    NXPLOG_TML_E("Reader Thread wait failed");
  }
  usleep(5000);    //wait for helios bootROM mode
  gpphTmlUwb_Context->is_read_abort = false;
  pthread_mutex_unlock(&gpphTmlUwb_Context->read_abort_lock);
  /*Abort the reader thread if client thread shall enable read again in case if valid packet received and notified to upper layer*/
  phTmlUwb_ReadAbort();
}
