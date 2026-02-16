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

#ifndef PHTMLUWB_H
#define PHTMLUWB_H

#include <phUwbCommon.h>

/*
 * Message posted by Reader thread uponl
 * completion of requested operation
 */
#define PH_TMLUWB_READ_MESSAGE (0xAA)

/*
 * Message posted by Writer thread upon
 * completion of requested operation
 */
#define PH_TMLUWB_WRITE_MESSAGE (0x55)

/*
 * Value indicates to reset device
 */
#define PH_TMLUWB_RESETDEVICE (0x00008001)

/*
***************************Globals,Structure and Enumeration ******************
*/

/*
 * Transaction (Tx/Rx) completion information structure of TML
 *
 * This structure holds the completion callback information of the
 * transaction passed from the TML layer to the Upper layer
 * along with the completion callback.
 *
 * The value of field wStatus can be interpreted as:
 *
 *     - UWBSTATUS_SUCCESS                    Transaction performed
 * successfully.
 *     - UWBSTATUS_FAILED                     Failed to wait on Read/Write
 * operation.
 *     - UWBSTATUS_INSUFFICIENT_STORAGE       Not enough memory to store data in
 * case of read.
 *     - UWBSTATUS_BOARD_COMMUNICATION_ERROR  Failure to Read/Write from the
 * file or timeout.
 */

typedef struct phTmlUwb_TransactInfo {
  tHAL_UWB_STATUS wStatus;       /* Status of the Transaction Completion*/
  uint8_t* pBuff;          /* Response Data of the Transaction*/
  uint16_t wLength;        /* Data size of the Transaction*/
} phTmlUwb_TransactInfo_t; /* Instance of Transaction structure */

/*
 * TML transreceive completion callback to Upper Layer
 *
 * pContext - Context provided by upper layer
 * pInfo    - Transaction info. See phTmlUwb_TransactInfo
 */
typedef void (*pphTmlUwb_TransactCompletionCb_t)(
    void* pContext, phTmlUwb_TransactInfo_t* pInfo);

/*
 * TML Deferred callback interface structure invoked by upper layer
 *
 * This could be used for read/write operations
 *
 * dwMsgPostedThread Message source identifier
 * pParams Parameters for the deferred call processing
 */
typedef void (*pphTmlUwb_DeferFuncPointer_t)(uint32_t dwMsgPostedThread,
                                             void* pParams);
/*
 * Structure containing details related to read and write operations
 *
 */
typedef struct phTmlUwb_ReadWriteInfo {
  volatile uint8_t bEnable; /*This flag shall decide whether to perform
                               Write/Read operation */
  uint8_t
      bThreadBusy; /*Flag to indicate thread is busy on respective operation */
  /* Transaction completion Callback function */
  pphTmlUwb_TransactCompletionCb_t pThread_Callback;
  void* pContext;        /*Context passed while invocation of operation */
  uint8_t* pBuffer;      /*Buffer passed while invocation of operation */
  uint16_t wLength;      /*Length of data read/written */
  tHAL_UWB_STATUS wWorkStatus; /*Status of the transaction performed */
} phTmlUwb_ReadWriteInfo_t;

/*
 *Base Context Structure containing members required for entire session
 */
typedef struct phTmlUwb_Context {
  pthread_t readerThread; /*Handle to the thread which handles write and read
                             operations */
  pthread_t writerThread;
  volatile uint8_t
      bThreadDone; /*Flag to decide whether to run or abort the thread */
  phTmlUwb_ReadWriteInfo_t tReadInfo;  /*Pointer to Reader Thread Structure */
  phTmlUwb_ReadWriteInfo_t tWriteInfo; /*Pointer to Writer Thread Structure */
  void* pDevHandle;                    /* Pointer to Device Handle */
  uintptr_t dwCallbackThreadId; /* Thread ID to which message to be posted */
  uint8_t bEnableCrc;           /*Flag to validate/not CRC for input buffer */
  sem_t rxSemaphore;
  sem_t txSemaphore;      /* Lock/Acquire txRx Semaphore */
  sem_t postMsgSemaphore; /* Semaphore to post message atomically by Reader &
                             writer thread */
  pthread_cond_t wait_busy_condition; /*Condition to wait reader thread*/
  pthread_mutex_t wait_busy_lock;     /*Condition lock to wait reader thread*/
  pthread_mutex_t read_abort_lock;    /*Condition lock to wait read abort*/
  pthread_cond_t read_abort_condition;  /*Condition to wait read abort*/
  volatile uint8_t wait_busy_flag;    /*Condition flag to wait reader thread*/
  volatile uint8_t is_read_abort;    /*Condition flag for read abort*/
  volatile uint8_t gWriterCbflag;    /* flag to indicate write callback message is pushed to
                           queue*/
} phTmlUwb_Context_t;

/*
 * Enum definition contains  supported ioctl control codes.
 *
 * phTmlUwb_Spi_IoCtl
 */
typedef enum {
  phTmlUwb_Invalid = 0,
  phTmlUwb_SetPower,
  phTmlUwb_EnableFwdMode,
  phTmlUwb_EnableThroughPut,
  phTmlUwb_EseReset
} phTmlUwb_ControlCode_t;     /* Control code for IOCTL call */

/*
 * TML Configuration exposed to upper layer.
 */
typedef struct phTmlUwb_Config {
  /* Port name connected to SR100
   *
   * Platform specific canonical device name to which SR100 is connected.
   *
   * e.g. On Linux based systems this would be /dev/SR100
   */
  const char* pDevName;
  /* Callback Thread ID
   *
   * This is the thread ID on which the Reader & Writer thread posts message. */
  uintptr_t dwGetMsgThreadId;
  /* Communication speed between DH and SR100
   *
   * This is the baudrate of the bus for communication between DH and SR100 */
  uint32_t dwBaudRate;
} phTmlUwb_Config_t, *pphTmlUwb_Config_t; /* pointer to phTmlUwb_Config_t */

/*
 * TML Deferred Callback structure used to invoke Upper layer Callback function.
 */
typedef struct {
  /* Deferred callback function to be invoked */
  pphTmlUwb_DeferFuncPointer_t pDef_call;
  /* Source identifier
   *
   * Identifier of the source which posted the message
   */
  uint32_t dwMsgPostedThread;
  /** Actual Message
   *
   * This is passed as a parameter passed to the deferred callback function
   * pDef_call. */
  void* pParams;
} phTmlUwb_DeferMsg_t; /* DeferMsg structure passed to User Thread */

/* Function declarations */
tHAL_UWB_STATUS phTmlUwb_Init(pphTmlUwb_Config_t pConfig);
tHAL_UWB_STATUS phTmlUwb_Shutdown(void);
tHAL_UWB_STATUS phTmlUwb_Write(uint8_t* pBuffer, uint16_t wLength,
                         pphTmlUwb_TransactCompletionCb_t pTmlWriteComplete,
                         void* pContext);
tHAL_UWB_STATUS phTmlUwb_Read(uint8_t* pBuffer, uint16_t wLength,
                        pphTmlUwb_TransactCompletionCb_t pTmlReadComplete,
                        void* pContext);
tHAL_UWB_STATUS phTmlUwb_WriteAbort(void);
tHAL_UWB_STATUS phTmlUwb_ReadAbort(void);
void phTmlUwb_eSE_Reset(void);
void phTmlUwb_Spi_Reset(void);
void phTmlUwb_Chip_Reset(void);
void phTmlUwb_DeferredCall(uintptr_t dwThreadId,
                           phLibUwb_Message_t* ptWorkerMsg);
#endif /*  PHTMLUWB_H  */
