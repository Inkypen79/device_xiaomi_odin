/*
 * Copyright 2018-2022 NXP
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

#ifndef _PHNXPUCIHAL_FW_H
#define _PHNXPUCIHAL_FW_H

/****************************************************************************************/
/*   GLOBAL FUNCTION PROTOTYPES */
/****************************************************************************************/
#define PHHBCI_GPIO_TIMEOUT_MS (10000U)
#define PHHBCI_GPIO_QBTIMEOUT_MS (5000U)

/*FW Versions*/
#define PHHBCI_FW_B0_VERSION 0xB0
#define PHHBCI_FW_B1_VERSION 0xB1
#define PHHBCI_FW_B2_VERSION 0xB2

/* Place configuration switches, which overrides values from phDefaultConfig.h,
 * here. */
#if defined(PHFL_BUILD_TYPE_DEBUG)
#define PHFL_CONFIG_DEBUG_PRINT 1
//#define PHFL_LOG_HOST 1
#endif
#ifndef PHFL_CONFIG_ENABLE_ASSERTIONS
#define PHFL_CONFIG_ENABLE_ASSERTIONS 1
#endif
#ifndef PHFL_CONFIG_ENABLE_ASSERTIONS_DEBUG
#ifndef __QAC__
#define PHFL_CONFIG_ENABLE_ASSERTIONS_DEBUG 0
#else
#define PHFL_CONFIG_ENABLE_ASSERTIONS_DEBUG 1
#endif
#endif

#ifndef PHFL_CONFIG_NORTOS
#define PHFL_CONFIG_NORTOS 1
#endif

#if (PHFL_CONFIG_TARGET_PLATFORM == PHFL_CONFIG_TARGET_PLATFORM_ARM)
#include <stdint.h>
#else

#ifndef __uint8_t_defined
#define __uint8_t_defined
/**
* \brief 8 bit unsigned integer
*/
typedef unsigned char uint8_t;
#endif

#ifndef __uint16_t_defined
#define __uint16_t_defined
/**
* \brief 16 bit unsigned integer
*/
typedef unsigned short uint16_t;
#endif

#ifndef __uint32_t_defined
#define __uint32_t_defined
/**
* \brief 32 bit unsigned integer
*/
typedef unsigned long uint32_t;
#endif

#ifndef __int8_t_defined
#define __int8_t_defined
/**
* \brief 8 bit signed integer
*/
typedef signed char int8_t;
#endif

#ifndef __int16_t_defined
#define __int16_t_defined
/**
* \brief 16 bit signed integer
*/
typedef signed short int16_t;
#endif

#ifndef __int32_t_defined
#define __int32_t_defined
/**
* \brief 32 bit signed integer
*/
typedef signed int int32_t;
#endif
#endif

#if !defined(__cplusplus) || defined(__arm__)
#ifndef __BOOL_DEFINED
#define __BOOL_DEFINED 1
/**
* \brief Boolean type
*/
#ifndef false
#define false 0
#endif
#ifndef true
#define true (!false)
#endif
// typedef uint8_t bool;
#endif
#endif

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#ifndef NULL
#define NULL ((void*)0) /**< Value used for NULL pointers */
#endif

#ifdef PHFL_ENABLE_STACK_CHECK
#define PH_CALL_DIRECT_FUNCTION(fct) (fct)
#else
#define PH_CALL_DIRECT_FUNCTION(fct)
#endif

#if (PHFL_CONFIG_TARGET_PLATFORM == PHFL_CONFIG_TARGET_PLATFORM_ARM) || \
    defined(__DOXYGEN)
/**
 * \brief Macro used to place given variable as ZI-data to specified section.
 * \note The section need to be defined in the scatter file.
 * Example:
 * \code
static uint32_t aBmaBuffer[PH_BOOT_BMA_BUFFER_SIZE]
PH_PLACE_ZI_DATA_TO_SECTION("BMA");
\endcode
 */
#define PH_PLACE_ZI_DATA_TO_SECTION(SECTIONNAME) \
  __attribute__((section(SECTIONNAME), zero_init))

/**
 * \brief Macro used to place constant data to specified section.
 * \note The section need to be defined in the scatter file.
 * Example:
 * \code
static const uint32_t dwFoo PH_PLACE_RO_DATA_TO_SECTION("FOO");
\endcode
 */
#define PH_PLACE_RO_DATA_TO_SECTION(SECTIONNAME) \
  __attribute__((section(SECTIONNAME)))
#define PH_PLACE_FUNCTION_TO_SECTION(SECTIONNAME) \
  __attribute__((section(SECTIONNAME)))

#define STRINGIFY(a) #a

#define PH_ZI_DATA_SECTION(SECTIONNAME)                  \
  _Pragma(STRINGIFY(arm section rwdata = #SECTIONNAME)); \
  _Pragma(STRINGIFY(arm section zidata = #SECTIONNAME));

#define PH_RO_DATA_SECTION(SECTIONNAME) \
  _Pragma(STRINGIFY(arm section rodata = #SECTIONNAME));

#define PH_CODE_SECTION(SECTIONNAME) \
  _Pragma(STRINGIFY(arm section code = #SECTIONNAME));

#define PH_PACK_STRUCT_BEGIN __packed
#define PH_PACK_STRUCT_END
#define PH_ALIGN __align
#define PH_USED __attribute__((used))
#else
#ifdef __QAC__
#define PH_ZI_DATA_SECTION(SECTIONNAME) static void(SECTIONNAME)()
#define PH_RO_DATA_SECTION(SECTIONNAME) static void(SECTIONNAME)()
#define PH_CODE_SECTION(SECTIONNAME) static void(SECTIONNAME)()
#else
#define PH_ZI_DATA_SECTION(SECTIONNAME)
#define PH_RO_DATA_SECTION(SECTIONNAME)
#define PH_CODE_SECTION(SECTIONNAME)
#endif
#define PH_PLACE_ZI_DATA_TO_SECTION(SECTIONNAME)
#define PH_PLACE_RO_DATA_TO_SECTION(SECTIONNAME)
#define PH_PLACE_FUNCTION_TO_SECTION(SECTIONNAME)
#define PH_ALIGN(a)
#define PH_USED
#define PH_PACK_STRUCT_BEGIN
#define PH_PACK_STRUCT_END __attribute__((__packed__))
#endif
/*********************************************************************************************************************/
/*   GLOBAL DEFINES */
/*********************************************************************************************************************/
#define PHHBCI_LEN_HDR (4U)
#define PHHBCI_LEN_LRC (1U)
#define PHHBCI_MAX_LEN_DATA_MOSI (2048U)
#define PHHBCI_MAX_LEN_PAYLOAD_MOSI (PHHBCI_MAX_LEN_DATA_MOSI + PHHBCI_LEN_LRC)
#define PHHBCI_MAX_LEN_DATA_MISO (256U)
#define PHHBCI_MAX_LEN_PAYLOAD_MISO (PHHBCI_MAX_LEN_DATA_MISO + PHHBCI_LEN_LRC)

#define PHHBCI_CLASS_MASK (0xF0U)
#define PHHBCI_SUBCLASS_MASK (0x0FU)
#define PHHBCI_ERROR_STATUS_MASK (0x80U)
#define PHHBCI_APDU_SEG_FLAG (0x8000U)

#define PHHBCI_HELIOS_CHIP_ID_SZ (16U)
#define PHHBCI_HELIOS_ID_SZ (4U)
#define PHHBCI_HELIOS_CA_ROOT_PUB_KEY_SZ (64U)
#define PHHBCI_HELIOS_NXP_PUB_KEY_SZ (64U)
#define PHHBCI_HELIOS_ROM_VERSION_SZ (1U)
#define PHHBCI_HELIOS_OTP_AUTOLOAD_INFO_SZ (44U)

#define PHHBCI_MODE_LEN_MSB_OFFSET (3U)
#define PHHBCI_MODE_LEN_LSB_OFFSET (2U)
#define PHHBCI_MODE_CHIP_ID_OFFSET (8U)
#define PHHBCI_MODE_DEV_LIFE_CYCLE_INFO_OFFSET (39U)

#define PHHBCI_HELIOS_DEV_KEY      0x00
#define PHHBCI_HELIOS_PROD_KEY_1   0x0F
#define PHHBCI_HELIOS_PROD_KEY_2   0x3F

#define HELIOS_MAX_SRAM_SZ (256 * 1024U)


/* Patch ROM */
#define PHHBCI_PATCHROM_SINGLE_ENTRY_SZ (sizeof(uint32_t) * 3U)
#define PHHBCI_PATCHROM_MAX_PATCH_TABLE_ENTRIES (48 + 1U)
#define PHHBCI_PATCHROM_MAX_PATCH_CODE_SZ (64 * 1024U)
#define PHHBCI_PATCHROM_SIGNATURE_SZ (3072U >> 3)
#define PHHBCI_PATCHROM_MAX_PATCH_TABLE_SZ \
  (PHHBCI_PATCHROM_SINGLE_ENTRY_SZ * PHHBCI_PATCHROM_MAX_PATCH_TABLE_ENTRIES)

/*#define PHHBCI_PATCHROM_MAX_IMAGE_SZ                \
    ( sizeof(phHbci_PatchROMInfo_t)                 \
    + PHHBCI_PATCHROM_MAX_PATCH_TABLE_SZ            \
    + PHHBCI_PATCHROM_MAX_PATCH_CODE_SZ             \
    + PHHBCI_PATCHROM_SIGNATURE_SZ )*/
#define PHHBCI_PATCHROM_MAX_IMAGE_SZ HELIOS_MAX_SRAM_SZ

typedef enum phHbci_Status {
  phHbci_Success = 0x00,
  phHbci_Failure = 0x01,
  phHbci_File_Not_found = 0x14

} phHbci_Status_t;
/*********************************************************************************************************************/
/*   GLOBAL DATATYPES */
/*********************************************************************************************************************/
constexpr uint8_t phHbci_Class_General = 0x00;
constexpr uint8_t phHbci_Class_Patch_ROM = 0x20;
constexpr uint8_t phHbci_Class_HIF_Image = 0x50;
constexpr uint8_t phHbci_Class_IM4_Image = 0x60;

constexpr uint8_t phHbci_SubClass_Query = 0x01;
constexpr uint8_t phHbci_SubClass_Answer = 0x02;
constexpr uint8_t phHbci_SubClass_Command = 0x03;
constexpr uint8_t phHbci_SubClass_Ack = 0x04;

typedef enum phHbci_Ack {
  phHbci_Valid_APDU = 0x01,

  phHbci_Invalid_LRC = 0x81,
  phHbci_Invlaid_Class = 0x82,
  phHbci_Invalid_Instruction = 0x83,
  phHbci_Invalid_Segment_Length = 0x84

} phHbci_Ack_t;
typedef struct phHbci_MosiApdu {
  uint8_t cls;
  uint8_t ins;
  uint16_t len;
  uint8_t payload[PHHBCI_MAX_LEN_PAYLOAD_MOSI];

} phHbci_MosiApdu_t;

typedef struct phHbci_MisoApdu {
  uint8_t cls;
  uint8_t ins;
  uint16_t len;
  uint8_t payload[PHHBCI_MAX_LEN_PAYLOAD_MISO];

} phHbci_MisoApdu_t;

typedef struct phPalSr100_Config {
  int8_t* pDevName;

  uint32_t dwBaudRate;

  void* pDevHandle;
  /*!< Device handle output */
} phPalSr100_Config_t, *pphPalSr100_Config_t;
typedef enum Link {
  Link_SPI,
  Link_I3C,

  Link_Default = Link_SPI

} Link_t;

typedef enum Mode {
  Mode_Patch_ROM = 1,
  Mode_HIF_Image,

  Mode_Default = Mode_HIF_Image

} Mode_t;

typedef enum Capture {
  Capture_Off,
  Capture_Apdu,
  Capture_Apdu_With_Dummy_Miso,

  Capture_Default = Capture_Off

} Capture_t;

typedef struct Options {
  char* imgFile;
  FILE* fImg;
  char* mosiFile;
  FILE* fMosi;
  char* misoFile;
  FILE* fMiso;
  Link_t link;
  Mode_t mode;
  Capture_t capture;

} Options_t;

/***** eClass_General *****/

typedef enum phHbci_General_Query {
  phHbci_General_Qry_Status = 0x21,

  phHbci_General_Qry_Chip_ID = 0x31,
  phHbci_General_Qry_Helios_ID = 0x32,
  //phHbci_General_Qry_CA_Root_Pub_Key = 0x33,
  phHbci_General_Qry_NXP_Pub_Key = 0x33,
  phHbci_General_Qry_ROM_Version = 0x34,
  phHbci_General_Qry_Fw_Dwnld_Root_Key = 0x35,
  phHbci_General_Qry_OTP_AutoLoad_Info = 0x36,

} phHbci_General_Query_t;

typedef enum phHbci_General_Answer {
  phHbci_General_Ans_HBCI_Ready = phHbci_General_Qry_Status,
  phHbci_General_Ans_Mode_Patch_ROM_Ready = 0x23,
  phHbci_General_Ans_Mode_HIF_Image_Ready = 0x24,
  phHbci_General_Ans_Mode_IM4_Image_Ready = 0x25,

  phHbci_General_Ans_Chip_ID = phHbci_General_Qry_Chip_ID,
  phHbci_General_Ans_Helios_ID = phHbci_General_Qry_Helios_ID,
  //phHbci_General_Ans_CA_Root_Pub_Key = phHbci_General_Qry_CA_Root_Pub_Key,
  phHbci_General_Ans_NXP_Pub_Key = phHbci_General_Qry_NXP_Pub_Key,
  phHbci_General_Ans_ROM_Version = phHbci_General_Qry_ROM_Version,
  phHbci_General_Ans_Fw_Dwnld_Root_Key = phHbci_General_Qry_Fw_Dwnld_Root_Key,
  phHbci_General_Ans_OTP_AutoLoad_Info = phHbci_General_Qry_OTP_AutoLoad_Info,

  phHbci_General_Ans_Boot_Success = 0x41,

  phHbci_General_Ans_Boot_Autoload_Fail = 0xD1,
  phHbci_General_Ans_Boot_GPIOConf_CRC_Fail = 0xD2,
  phHbci_General_Ans_Boot_TRIM_CRC_Fail = 0xD3,
  phHbci_General_Ans_Boot_GPIOTRIM_CRC_Fail = 0xD4,

  phHbci_General_Ans_HBCI_Fail = 0xE1,
  phHbci_General_Ans_Mode_Patch_ROM_Fail = 0xE3,
  phHbci_General_Ans_Mode_HIF_Image_Fail = 0xE4,
  phHbci_General_Ans_Mode_IM4_Image_Fail = 0xE5

} phHbci_General_Answer_t;

typedef enum phHbci_General_Command {
  phHbci_General_Cmd_Mode_Patch_ROM = phHbci_General_Ans_Mode_Patch_ROM_Ready,
  phHbci_General_Cmd_Mode_HIF_Image = phHbci_General_Ans_Mode_HIF_Image_Ready,
  phHbci_General_Cmd_Mode_IM4_Image = phHbci_General_Ans_Mode_IM4_Image_Ready

} phHbci_General_Command_t;

phHbci_MosiApdu_t gphHbci_MosiApdu;
phHbci_MisoApdu_t gphHbci_MisoApdu;
phPalSr100_Config_t tPalConfig;
Options_t gOpts;

uint8_t gDummyMiso[PHHBCI_MAX_LEN_PAYLOAD_MISO];

phHbci_Status_t phHbci_GetStatus(void);
phHbci_Status_t phHbci_GeneralStatus(phHbci_General_Command_t mode);
phHbci_Status_t phHbci_QueryInfo(uint8_t* pInfo, uint32_t* pInfoSz,
                                 uint32_t maxSz, bool matchMaxSz);
phHbci_Status_t phHbci_GetGeneralInfo(uint8_t* pInfo, uint32_t* pInfoSz);
phHbci_Status_t phHbci_GetInfo(uint8_t* pInfo, uint32_t* pInfoSz);
phHbci_Status_t phHbci_PutCommand(uint8_t* pImg, uint32_t imgSz);

uint8_t phHbci_CalcLrc(uint8_t* pBuf, uint16_t bufSz);
int printUsage(char* pProg);
int cMain(int argc, char* argv[]);
int cppGetApdu(uint8_t* pApdu, uint16_t sz);
int cppPutApdu(uint8_t* pApdu, uint16_t sz);
int cppWaitForGPIOEvent(uint32_t timeoutMs);
void cppResetGPIOEvent(void);
phHbci_Status_t phHbci_GetApdu(uint8_t* pApdu, uint16_t sz);
phHbci_Status_t phHbci_PutApdu(uint8_t* pApdu, uint16_t sz);
phHbci_Status_t phHbci_Master(phHbci_General_Command_t mode, uint8_t* pImg,
                              uint32_t imgSz);

typedef struct phHbci_PatchROMInfo {
  uint32_t patchFileMarker;
  uint32_t offsetRAM;
  uint32_t szPatchCode;
  uint16_t numPatchTableEntries;
  uint16_t szSignatureBuf;

} phHbci_PatchROMInfo_t;

/* HIF Image */
//#define PHHIF_MAX_IMAGE_SZ                  ((256-32)*1024U)
#define PHHIF_MAX_IMAGE_SZ HELIOS_MAX_SRAM_SZ

/*********************************************************************************************************************/
/*   GLOBAL MACROS */
/*********************************************************************************************************************/
#define PHHBCI_PEEK32(w32, w8)                                    \
  do {                                                            \
    w32 = ((uint32_t)(w8[3]) << 24) | ((uint32_t)(w8[2]) << 16) | \
          ((uint32_t)(w8[1]) << 8) | w8[0];                       \
  } while (0)

#define PHHBCI_READ32(w32, w8) \
  do {                         \
    PHHBCI_PEEK32(w32, w8);    \
    w8 += sizeof(w32);         \
  } while (0)

/***** eClass_Patch_ROM *****/

typedef enum phHbci_Patch_ROM_Query {
  phHbci_Patch_ROM_Qry_Patch_Status = 0x01

} phHbci_Patch_ROM_Query_t;

typedef enum phHbci_Patch_ROM_Answer {
  phHbci_Patch_ROM_Ans_Patch_Success = phHbci_Patch_ROM_Qry_Patch_Status,

  phHbci_Patch_ROM_Ans_File_Too_Large = 0x81,
  phHbci_Patch_ROM_Ans_Invalid_Patch_File_Marker = 0x82,
  phHbci_Patch_ROM_Ans_Too_Many_Patch_Table_Entries = 0x83,
  phHbci_Patch_ROM_Ans_Invalid_Patch_Code_Size = 0x84,
  phHbci_Patch_ROM_Ans_Invalid_Global_Patch_Marker = 0x85,
  phHbci_Patch_ROM_Ans_Invalid_Signature_Size = 0x86,
  phHbci_Patch_ROM_Ans_Invalid_Signature = 0x87

} phHbci_Patch_ROM_Answer_t;

typedef enum phHbci_Patch_ROM_Command {
  phHbci_Patch_ROM_Cmd_Download_Patch = phHbci_Patch_ROM_Qry_Patch_Status

} phHbci_Patch_ROM_Command_t;

/***** eClass_HIF_Image *****/

typedef enum phHbci_HIF_Image_Query {
  phHbci_HIF_Image_Qry_Image_Status = 0x01

} phHbci_HIF_Image_Query_t;

typedef enum phHbci_HIF_Image_Answer {
  phHbci_HIF_Image_Ans_Image_Success = phHbci_HIF_Image_Qry_Image_Status,
  phHbci_HIF_Image_Ans_Header_Success = 0x04,
  phHbci_HIF_Image_Ans_Quickboot_Settings_Success = 0x05,
  phHbci_HIF_Image_Ans_Execution_Settings_Success = 0x06,

  phHbci_HIF_Image_Ans_Header_Too_Large = 0x81,
  phHbci_HIF_Image_Ans_Header_Parse_Error = 0x82,
  phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Crypto = 0x83,
  phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Mode = 0x84,
  phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Hash = 0x85,
  phHbci_HIF_Image_Ans_Invalid_Cipher_Type_Curve = 0x86,
  phHbci_HIF_Image_Ans_Invalid_ECC_Key_Length = 0x87,
  phHbci_HIF_Image_Ans_Invalid_Payload_Description = 0x88,
  phHbci_HIF_Image_Ans_Invalid_Firmware_Version = 0x89,
  phHbci_HIF_Image_Ans_Invalid_ECID_Mask = 0x8A,
  phHbci_HIF_Image_Ans_Invalid_ECID_Value = 0x8B,
  phHbci_HIF_Image_Ans_Invalid_Encrypted_Payload_Hash = 0x8C,
  phHbci_HIF_Image_Ans_Invalid_Header_Signature = 0x8D,
  phHbci_HIF_Image_Ans_Install_Settings_Too_Large = 0x8E,
  phHbci_HIF_Image_Ans_Install_Settings_Parse_Error = 0x8F,
  phHbci_HIF_Image_Ans_Payload_Too_Large = 0x90,
  phHbci_HIF_Image_Ans_Quickboot_Settings_Parse_Error = 0x91,
  phHbci_HIF_Image_Ans_Invalid_Static_Hash = 0x92,
  phHbci_HIF_Image_Ans_Invalid_Dynamic_Hash = 0x93,
  phHbci_HIF_Image_Ans_Execution_Settings_Parse_Error = 0x94,
  phHbci_HIF_Image_Ans_Key_Read_Error = 0x95

} phHbci_HIF_Image_Answer_t;

typedef enum phHbci_HIF_Image_Command {
  phHbci_HIF_Image_Cmd_Download_Image = phHbci_HIF_Image_Qry_Image_Status

} phHbci_HIF_Image_Command_t;

/***** eClass_IM4_Image *****/

typedef enum phHbci_IM4_Image_Query {
  phHbci_IM4_Image_Qry_IM4_Status = 0x01,
  phHbci_IM4_Image_Qry_IM4M_Status = 0x02,
  phHbci_IM4_Image_Qry_IM4P_Status = 0x03,
  phHbci_IM4_Image_Qry_File_Descriptor_Status = 0x04,
  phHbci_IM4_Image_Qry_Payload_Status = 0x05

} phHbci_IM4_Image_Query_t;

typedef enum phHbci_IM4_Image_Answer {
  phHbci_IM4_Image_Ans_IM4_Success = phHbci_IM4_Image_Qry_IM4_Status,
  phHbci_IM4_Image_Ans_IM4M_Success = phHbci_IM4_Image_Qry_IM4M_Status,
  phHbci_IM4_Image_Ans_IM4P_Success = phHbci_IM4_Image_Qry_IM4P_Status,
  phHbci_IM4_Image_Ans_File_Descriptor_Success =
      phHbci_IM4_Image_Qry_File_Descriptor_Status,
  phHbci_IM4_Image_Ans_Payload_Success = phHbci_IM4_Image_Qry_Payload_Status,

  phHbci_IM4_Image_Ans_IM4M_Too_Large = 0x81,
  phHbci_IM4_Image_Ans_IM4M_Parse_Error = 0x82,
  phHbci_IM4_Image_Ans_Invalid_Chip_ID = 0x83,
  phHbci_IM4_Image_Ans_Invalid_Helios_ID = 0x84,
  phHbci_IM4_Image_Ans_Invalid_IM4M_Leaf_Certificate = 0x85,
  phHbci_IM4_Image_Ans_Invalid_IM4M_Manifest_Signature = 0x86,
  phHbci_IM4_Image_Ans_IM4P_Too_Large = 0x87,
  phHbci_IM4_Image_Ans_Invalid_IM4P_Hash = 0x88,
  phHbci_IM4_Image_Ans_IM4P_Parse_Error = 0x89,
  phHbci_IM4_Image_Ans_Invalid_IM4P_Signature = 0x8A,
  phHbci_IM4_Image_Ans_File_Descriptor_Too_Large = 0x8B,
  phHbci_IM4_Image_Ans_Invalid_File_Descriptor = 0x8C,
  phHbci_IM4_Image_Ans_Payload_Too_Large = 0x8D,
  phHbci_IM4_Image_Ans_Invalid_Encrypted_Payload_Hash = 0x8E,
  phHbci_IM4_Image_Ans_Invalid_Download_Settings = 0x8F

} phHbci_IM4_Image_Answer_t;

typedef enum phHbci_IM4_Image_Command {
  phHbci_IM4_Image_Cmd_Download_IM4 = phHbci_IM4_Image_Qry_IM4_Status,
  phHbci_IM4_Image_Cmd_Download_IM4M = phHbci_IM4_Image_Qry_IM4M_Status,
  phHbci_IM4_Image_Cmd_Download_IM4P = phHbci_IM4_Image_Qry_IM4P_Status,
  phHbci_IM4_Image_Cmd_Download_File_Descriptor =
      phHbci_IM4_Image_Qry_File_Descriptor_Status,
  phHbci_IM4_Image_Cmd_Download_Payload = phHbci_IM4_Image_Qry_Payload_Status

} phHbci_IM4_Image_Command_t;

//#pragma pack(push, 1)

//#pragma pack(pop)
#endif /* _PHNXPUCIHAL_FW_H */
