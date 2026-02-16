/******************************************************************************
 *
 *  Copyright (C) 2011-2012 Broadcom Corporation
 *  Copyright 2018-2019, 2023 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
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
//#define LOG_NDEBUG 0
#define LOG_TAG "NxpUwbConf"

#include <sys/stat.h>

#include <iomanip>
#include <memory>
#include <sstream>
#include <limits.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <android-base/logging.h>
#include <cutils/properties.h>
#include <log/log.h>

#include "phNxpConfig.h"
#include "phNxpUciHal.h"
#include "phNxpUciHal_ext.h"
#include "phNxpUciHal_utils.h"
#include "phNxpLog.h"

static const char default_nxp_config_path[] = "/vendor/etc/libuwb-nxp.conf";
static const char country_code_config_name[] = "libuwb-countrycode.conf";
static const char nxp_uci_config_file[] = "libuwb-uci.conf";
static const char default_uci_config_path[] = "/vendor/etc/";

static const char country_code_specifier[] = "<country>";
static const char sku_specifier[] = "<sku>";

static const char prop_name_calsku[] = "persist.vendor.uwb.cal.sku";
static const char prop_default_calsku[] = "defaultsku";

using namespace::std;

class uwbParam
{
public:
    enum class type { STRING, NUMBER, BYTEARRAY, STRINGARRAY };
    uwbParam();
    uwbParam(const uwbParam& param);
    uwbParam(uwbParam&& param);

    uwbParam(const string& value);
    uwbParam(vector<uint8_t>&& value);
    uwbParam(unsigned long value);
    uwbParam(vector<string>&& value);

    virtual ~uwbParam();

    type getType() const { return m_type; }
    unsigned long numValue() const {return m_numValue;}
    const char*   str_value() const {return m_str_value.c_str();}
    size_t        str_len() const   {return m_str_value.length();}
    const uint8_t* arr_value() const { return m_arrValue.data(); }
    size_t arr_len() const { return m_arrValue.size(); }

    size_t str_arr_len() const { return m_arrStrValue.size(); }
    const char* str_arr_elem(const int index) const { return m_arrStrValue[index].c_str(); }
    size_t str_arr_elem_len(const int index) const { return m_arrStrValue[index].length(); }

    void dump(const string &tag) const;
private:
    unsigned long   m_numValue;
    string          m_str_value;
    vector<uint8_t>  m_arrValue;
    vector<string>  m_arrStrValue;
    type m_type;
};

class CUwbNxpConfig
{
public:
    CUwbNxpConfig();
    CUwbNxpConfig(CUwbNxpConfig&& config);
    CUwbNxpConfig(const char *filepath);
    virtual ~CUwbNxpConfig();
    CUwbNxpConfig& operator=(CUwbNxpConfig&& config);

    bool isValid() const { return mValidFile; }
    bool isCountrySpecific() const { return mCountrySpecific; }
    void reset() {
        m_map.clear();
        mValidFile = false;
    }

    const uwbParam*    find(const char* p_name) const;
    void    setCountry(const string& strCountry);

    void    dump() const;

    const unordered_map<string, uwbParam>& get_data() const {
        return m_map;
    }
private:
    bool    readConfig();

    unordered_map<string, uwbParam> m_map;
    bool    mValidFile;
    string  mFilePath;
    string  mCurrentFile;
    bool    mCountrySpecific;
};

/*******************************************************************************
**
** Function:    isPrintable()
**
** Description: determine if 'c' is printable
**
** Returns:     1, if printable, otherwise 0
**
*******************************************************************************/
static inline bool isPrintable(char c)
{
    return  (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '/' || c == '_' || c == '-' || c == '.' || c == ',';
}

/*******************************************************************************
**
** Function:    isDigit()
**
** Description: determine if 'c' is numeral digit
**
** Returns:     true, if numerical digit
**
*******************************************************************************/
static inline bool isDigit(char c, int base)
{
    if (base == 10) {
        return isdigit(c);
    } else if (base == 16) {
        return isxdigit(c);
    } else {
        return false;
    }
}

static inline bool isArrayDelimeter(char c)
{
    return (isspace(c) || c== ',' || c == ':' || c == '-' || c == '}');
}

/*******************************************************************************
**
** Function:    getDigitValue()
**
** Description: return numerical value of a decimal or hex char
**
** Returns:     numerical value if decimal or hex char, otherwise 0
**
*******************************************************************************/
inline int getDigitValue(char c, int base)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    if (base == 16)
    {
        if ('A' <= c && c <= 'F')
            return c - 'A' + 10;
        else if ('a' <= c && c <= 'f')
            return c - 'a' + 10;
    }
    return 0;
}

/*******************************************************************************
**
** Function:    CUwbNxpConfig::readConfig()
**
** Description: read Config settings and parse them into a linked list
**              move the element from linked list to a array at the end
**
** Returns:     1, if there are any config data, 0 otherwise
**
*******************************************************************************/
bool CUwbNxpConfig::readConfig()
{
    enum {
        BEGIN_LINE = 1,
        TOKEN,
        STR_VALUE,
        NUM_VALUE,
        ARR_SPACE,
        ARR_STR,
        ARR_STR_SPACE,
        ARR_NUM,
        BEGIN_HEX,
        BEGIN_QUOTE,
        END_LINE
    };

    FILE*   fd;
    string  token;
    string  strValue;
    unsigned long    numValue = 0;
    vector<uint8_t> arrValue;
    vector<string> arrStr;
    int     base = 0;
    int     c;
    const char *name = mCurrentFile.c_str();
    unsigned long state = BEGIN_LINE;

    mValidFile = false;
    m_map.clear();

    /* open config file, read it into a buffer */
    if ((fd = fopen(name, "r")) == NULL)
    {
        ALOGD("%s Cannot open config file %s\n", __func__, name);
        return false;
    }
    ALOGV("%s Opened config %s\n", __func__, name);

    for (;;) {
        c = fgetc(fd);

        switch (state) {
        case BEGIN_LINE:
            if (isPrintable(c)) {
                token.clear();
                numValue = 0;
                strValue.clear();
                arrValue.clear();
                arrStr.clear();
                state = TOKEN;
                token.push_back(c);
            } else {
                state = END_LINE;
            }
            break;
        case TOKEN:
            if (c == '=') {
                state = BEGIN_QUOTE;
            } else if (isPrintable(c)) {
                token.push_back(c);
            } else {
                state = END_LINE;
            }
            break;
        case BEGIN_QUOTE:
            if (c == '"') {
                state = STR_VALUE;
                base = 0;
            } else if (c == '0') {
                state = BEGIN_HEX;
            } else if (isDigit(c, 10)) {
                state = NUM_VALUE;
                base = 10;
                numValue = getDigitValue(c, base);
            } else if (c == '{') {
                state = ARR_SPACE;
                base = 16;
            } else {
                state = END_LINE;
            }
            break;
        case BEGIN_HEX:
            if (c == 'x' || c == 'X') {
                state = NUM_VALUE;
                base = 16;
                numValue = 0;
            } else if (isDigit(c, 10)) {
                state = NUM_VALUE;
                base = 10;
                numValue = getDigitValue(c, base);
            } else {
                m_map.try_emplace(token, move(uwbParam(numValue)));
                state = END_LINE;
            }
            break;
        case NUM_VALUE:
            if (isDigit(c, base)) {
                numValue *= base;
                numValue += getDigitValue(c, base);
            } else {m_map.try_emplace(token, move(uwbParam(numValue)));
                state = END_LINE;
            }
            break;
        case ARR_SPACE:
            if (isDigit(c, 16)) {
                numValue = getDigitValue(c, base);
                state = ARR_NUM;
            } else if (c == '}') {
                m_map.try_emplace(token, move(uwbParam(move(arrValue))));
                state = END_LINE;
            } else if (c == '"') {
                state = ARR_STR;
            } else if (c == EOF) {
                state = END_LINE;
            }
            break;
        case ARR_STR:
            if (c == '"') {
                arrStr.emplace_back(move(strValue));
                strValue.clear();
                state = ARR_STR_SPACE;
            } else {
                strValue.push_back(c);
            }
            break;
        case ARR_STR_SPACE:
            if (c == '}') {
                m_map.try_emplace(token, move(uwbParam(move(arrStr))));
                state = END_LINE;
            } else if (c == '"') {
                state = ARR_STR;
            }
            break;
        case ARR_NUM:
            if (isDigit(c, 16)) {
                numValue *= 16;
                numValue += getDigitValue(c, base);
            } else if (isArrayDelimeter(c)) {
                arrValue.push_back(numValue & 0xff);
                state = ARR_SPACE;
            } else {
                state = END_LINE;
            }
            if (c == '}') {
                m_map.try_emplace(token, move(uwbParam(move(arrValue))));
                state = END_LINE;
            }
            break;
        case STR_VALUE:
            if (c == '"') {
                state = END_LINE;
                m_map.try_emplace(token, move(uwbParam(strValue)));
            } else {
                strValue.push_back(c);
            }
            break;
        case END_LINE:
            // do nothing
        default:
            break;
        }
        if (c == EOF)
            break;
        else if (state == END_LINE && (c == '\n' || c == '\r'))
            state = BEGIN_LINE;
        else if (c == '#')
            state = END_LINE;
    }

    fclose(fd);

    if (m_map.size() > 0) {
        mValidFile = true;
    }

    return mValidFile;
}

/*******************************************************************************
**
** Function:    CUwbNxpConfig::CUwbNxpConfig()
**
** Description: class constructor
**
** Returns:     none
**
*******************************************************************************/
CUwbNxpConfig::CUwbNxpConfig() :
    mValidFile(false),
    mCountrySpecific(false)
{
}

/*******************************************************************************
**
** Function:    CUwbNxpConfig::~CUwbNxpConfig()
**
** Description: class destructor
**
** Returns:     none
**
*******************************************************************************/
CUwbNxpConfig::~CUwbNxpConfig()
{
}

CUwbNxpConfig::CUwbNxpConfig(const char *filepath) :
    mValidFile(false),
    mFilePath(filepath),
    mCountrySpecific(false)
{
    auto pos = mFilePath.find(sku_specifier);
    if (pos != string::npos) {
        char prop_str[PROPERTY_VALUE_MAX];
        property_get(prop_name_calsku, prop_str,prop_default_calsku);
        mFilePath.replace(pos, strlen(sku_specifier), prop_str);
    }

    // country specifier will be evaluated later in setCountry() path
    pos = mFilePath.find(country_code_specifier);
    if (pos == string::npos) {
        mCurrentFile = mFilePath;
        readConfig();
    } else {
        mCountrySpecific = true;
    }
}

CUwbNxpConfig::CUwbNxpConfig(CUwbNxpConfig&& config)
{
    m_map = move(config.m_map);
    mValidFile = config.mValidFile;
    mFilePath = move(config.mFilePath);
    mCurrentFile = move(config.mCurrentFile);
    mCountrySpecific = config.mCountrySpecific;

    config.mValidFile = false;
}

CUwbNxpConfig& CUwbNxpConfig::operator=(CUwbNxpConfig&& config)
{
    m_map = move(config.m_map);
    mValidFile = config.mValidFile;
    mFilePath = move(config.mFilePath);
    mCurrentFile = move(config.mCurrentFile);
    mCountrySpecific = config.mCountrySpecific;

    config.mValidFile = false;
    return *this;
}

void CUwbNxpConfig::setCountry(const string& strCountry)
{
    if (!isCountrySpecific())
        return;

    mCurrentFile = mFilePath;
    auto pos = mCurrentFile.find(country_code_specifier);
    if (pos == string::npos) {
        return;
    }

    mCurrentFile.replace(pos, strlen(country_code_specifier), strCountry);
    readConfig();
}

/*******************************************************************************
**
** Function:    CUwbNxpConfig::find()
**
** Description: search if a setting exist in the setting array
**
** Returns:     pointer to the setting object
**
*******************************************************************************/
const uwbParam* CUwbNxpConfig::find(const char* p_name) const
{
    const auto it = m_map.find(p_name);

    if (it == m_map.cend()) {
        return NULL;
    }
    return &it->second;
}

/*******************************************************************************
**
** Function:    CUwbNxpConfig::dump()
**
** Description: prints all elements in the list
**
** Returns:     none
**
*******************************************************************************/
void CUwbNxpConfig::dump() const
{
    ALOGV("Dump configuration file %s : %s, %zu entries", mCurrentFile.c_str(),
        mValidFile ? "valid" : "invalid", m_map.size());

    for (auto &it : m_map) {
        auto &key = it.first;
        auto &param = it.second;
        param.dump(key);
    }
}

/*******************************************************************************/
uwbParam::uwbParam() :
    m_numValue(0),
    m_type(type::NUMBER)
{
}

uwbParam::~uwbParam()
{
}

uwbParam::uwbParam(const uwbParam &param) :
    m_numValue(param.m_numValue),
    m_str_value(param.m_str_value),
    m_arrValue(param.m_arrValue),
    m_arrStrValue(param.m_arrStrValue),
    m_type(param.m_type)
{
}

uwbParam::uwbParam(uwbParam &&param) :
    m_numValue(param.m_numValue),
    m_str_value(move(param.m_str_value)),
    m_arrValue(move(param.m_arrValue)),
    m_arrStrValue(move(param.m_arrStrValue)),
    m_type(param.m_type)
{
}

uwbParam::uwbParam(const string& value) :
    m_numValue(0),
    m_str_value(value),
    m_type(type::STRING)
{
}

uwbParam::uwbParam(unsigned long value) :
    m_numValue(value),
    m_type(type::NUMBER)
{
}

uwbParam::uwbParam(vector<uint8_t> &&value) :
    m_arrValue(move(value)),
    m_type(type::BYTEARRAY)
{
}

uwbParam::uwbParam(vector<string> &&value) :
    m_arrStrValue(move(value)),
    m_type(type::STRINGARRAY)
{
}


void uwbParam::dump(const string &tag) const
{
    if (m_type == type::NUMBER) {
        ALOGV(" - %s = 0x%lx", tag.c_str(), m_numValue);
    } else if (m_type == type::STRING) {
        ALOGV(" - %s = %s", tag.c_str(), m_str_value.c_str());
    } else if (m_type == type::BYTEARRAY) {
        stringstream ss_hex;
        ss_hex.fill('0');
        for (auto b : m_arrValue) {
            ss_hex << setw(2) << hex << (int)b << " ";
        }
        ALOGV(" - %s = { %s}", tag.c_str(), ss_hex.str().c_str());
    } else if (m_type == type::STRINGARRAY) {
        stringstream ss;
        for (auto s : m_arrStrValue) {
            ss << "\"" << s << "\", ";
        }
        ALOGV(" - %s = { %s}", tag.c_str(), ss.str().c_str());
    }
}
/*******************************************************************************/
class RegionCodeMap {
public:
    void loadMapping(const char *filepath) {
        CUwbNxpConfig config(filepath);
        if (!config.isValid()) {
            ALOGW("Region mapping was not provided.");
            return;
        }

        ALOGI("Region mapping was provided by %s", filepath);
        auto &all_params = config.get_data();
        for (auto &it : all_params) {
            const auto &region_str = it.first;
            const uwbParam *param = &it.second;

            // split space-separated strings into set
            stringstream ss(param->str_value());
            string cc;
            unordered_set<string> cc_set;
            while (ss >> cc) {
              if (cc.length() == 2 && isupper(cc[0]) && isupper(cc[1])) {
                cc_set.emplace(move(cc));
              }
            }
            auto result = m_map.try_emplace(region_str, move(cc_set));
            if (!result.second) {
              // region conlifct : merge
              result.first->second.merge(move(cc_set));
            }
        }
        m_config = move(config);
    }
    string xlateCountryCode(const char country_code[2]) {
        string code{country_code[0], country_code[1]};
        if (m_config.isValid()) {
            for (auto &it : m_map) {
                const auto &region_str = it.first;
                const auto &cc_set = it.second;
                if (cc_set.find(code) != cc_set.end()) {
                    ALOGV("map country code %c%c --> %s",
                            country_code[0], country_code[1], region_str.c_str());
                    return region_str;
                }
            }
        }
        return code;
    }
    void reset() {
        m_config.reset();
        m_map.clear();
    }
    void dump() {
        ALOGV("Region mapping dump:");
        for (auto &entry : m_map) {
            const auto &region_str = entry.first;
            const auto &cc_set = entry.second;
            stringstream ss;
            for (const auto s : cc_set) {
                ss << "\"" << s << "\", ";
            }
            ALOGV("- %s = { %s}", region_str.c_str(), ss.str().c_str());
        }
    }
private:
    CUwbNxpConfig m_config;
    unordered_map<string, unordered_set<string>> m_map;
};

/*******************************************************************************/
class CascadeConfig {
public:
    CascadeConfig();

    void init(const char *main_config);
    void deinit();
    bool setCountryCode(const char country_code[2]);

    const uwbParam* find(const char *name)  const;
    bool    getValue(const char* name, char* pValue, size_t len) const;
    bool    getValue(const char* name, unsigned long& rValue) const;
    bool    getValue(const char* name, uint8_t* pValue, long len, long* readlen) const;
private:
    // default_nxp_config_path
    CUwbNxpConfig mMainConfig;

    // uci config
    CUwbNxpConfig mUciConfig;

    // EXTRA_CONF_PATH[N]
    vector<CUwbNxpConfig> mExtraConfig;

    // [COUNTRY_CODE_CAP_FILE_LOCATION]/country_code_config_name
    CUwbNxpConfig mCapsConfig;

    // Region Code mapping
    RegionCodeMap mRegionMap;

    // Current region code
    string mCurRegionCode;

    void dump() {
        mMainConfig.dump();
        mUciConfig.dump();

        for (const auto &config : mExtraConfig)
            config.dump();

        mCapsConfig.dump();
        mRegionMap.dump();
    }
};

CascadeConfig::CascadeConfig()
{
}

void CascadeConfig::init(const char *main_config)
{
    ALOGV("CascadeConfig initialize with %s", main_config);

    // Main config file
    CUwbNxpConfig config(main_config);
    if (!config.isValid()) {
        ALOGW("Failed to load main config file");
        return;
    }
    mMainConfig = move(config);

    {
        // UCI config file
        std::string uciConfigFilePath = default_uci_config_path;
        uciConfigFilePath += nxp_uci_config_file;

        CUwbNxpConfig config(uciConfigFilePath.c_str());
        if (!config.isValid()) {
            ALOGW("Failed to load uci config file:%s",
                    uciConfigFilePath.c_str());
        } else {
            mUciConfig = move(config);
        }
    }

    // Read EXTRA_CONF_PATH[N]
    for (int i = 1; i <= 10; i++) {
        char key[32];
        snprintf(key, sizeof(key), "EXTRA_CONF_PATH_%d", i);
        const uwbParam *param = mMainConfig.find(key);
        if (!param)
            continue;
        CUwbNxpConfig config(param->str_value());
        ALOGD("Extra calibration file %s : %svalid", param->str_value(), config.isValid() ? "" : "in");
        if (config.isValid() || config.isCountrySpecific()) {
            mExtraConfig.emplace_back(move(config));
        }
    }

    // Pick one libuwb-countrycode.conf with the highest VERSION number
    // from multiple directories specified by COUNTRY_CODE_CAP_FILE_LOCATION
    unsigned long arrLen = 0;
    if (NxpConfig_GetStrArrayLen(NAME_COUNTRY_CODE_CAP_FILE_LOCATION, &arrLen) && arrLen > 0) {
        const long loc_max_len = 260;
        auto loc = make_unique<char[]>(loc_max_len);
        int version, max_version = -1;
        string strPickedPath;
        bool foundCapFile = false;
        CUwbNxpConfig pickedConfig;

        for (int i = 0; i < arrLen; i++) {
            if (!NxpConfig_GetStrArrayVal(NAME_COUNTRY_CODE_CAP_FILE_LOCATION, i, loc.get(), loc_max_len)) {
                continue;
            }
            string strPath(loc.get());
            strPath += country_code_config_name;

            ALOGV("Try to load %s", strPath.c_str());

            CUwbNxpConfig config(strPath.c_str());

            const uwbParam *param = config.find(NAME_NXP_COUNTRY_CODE_VERSION);
            version = param ? atoi(param->str_value()) : -2;
            if (version > max_version) {
                foundCapFile = true;
                pickedConfig = move(config);
                strPickedPath = move(strPath);
                max_version = version;
            }
        }
        if (foundCapFile) {
            mCapsConfig = move(pickedConfig);
            ALOGI("CountryCodeCaps file %s loaded with VERSION=%d", strPickedPath.c_str(), max_version);
        } else {
            ALOGI("No CountryCodeCaps specified");
        }
    } else {
        ALOGI(NAME_COUNTRY_CODE_CAP_FILE_LOCATION " was not specified, skip loading CountryCodeCaps");
    }

    // Load region mapping
    const uwbParam *param = find(NAME_REGION_MAP_PATH);
    if (param) {
        mRegionMap.loadMapping(param->str_value());
    }

    ALOGD("CascadeConfig initialized");

    dump();
}

void CascadeConfig::deinit()
{
    mMainConfig.reset();
    mExtraConfig.clear();
    mCapsConfig.reset();
    mRegionMap.reset();
    mUciConfig.reset();
    mCurRegionCode.clear();
}

bool CascadeConfig::setCountryCode(const char country_code[2])
{
    string strRegion = mRegionMap.xlateCountryCode(country_code);

    if (strRegion == mCurRegionCode) {
        ALOGI("Same region code(%c%c --> %s), per-country configuration not updated.",
              country_code[0], country_code[1], strRegion.c_str());
        return false;
    }

    ALOGI("Apply country code %c%c --> %s\n", country_code[0], country_code[1], strRegion.c_str());
    mCurRegionCode = strRegion;
    for (auto &x : mExtraConfig) {
        if (x.isCountrySpecific()) {
            x.setCountry(mCurRegionCode);
            x.dump();
        }
    }
    return true;
}

const uwbParam* CascadeConfig::find(const char *name) const
{
    const uwbParam* param = NULL;

    param = mCapsConfig.find(name);
    if (param)
      return param;

    for (auto it = mExtraConfig.rbegin(); it != mExtraConfig.rend(); it++) {
        param = it->find(name);
        if (param)
            break;
    }
    if (!param) {
        param = mMainConfig.find(name);
    }
    if (!param) {
        param = mUciConfig.find(name);
    }
    return param;
}

// TODO: move these getValue() helpers out of the class
bool CascadeConfig::getValue(const char* name, char* pValue, size_t len) const
{
    const uwbParam *param = find(name);
    if (!param)
        return false;
    if (param->getType() != uwbParam::type::STRING)
        return false;
    if (len < (param->str_len() + 1))
        return false;

    strncpy(pValue, param->str_value(), len);
    return true;
}

bool CascadeConfig::getValue(const char* name, uint8_t* pValue, long len, long* readlen) const
{
    const uwbParam *param = find(name);
    if (!param)
        return false;
    if (param->getType() != uwbParam::type::BYTEARRAY)
        return false;
    if (len < param->arr_len())
        return false;
    memcpy(pValue, param->arr_value(), param->arr_len());
    if (readlen)
        *readlen = param->arr_len();
    return true;
}

bool CascadeConfig::getValue(const char* name, unsigned long& rValue) const
{
    const uwbParam *param = find(name);
    if (!param)
        return false;
    if (param->getType() != uwbParam::type::NUMBER)
        return false;

    rValue = param->numValue();
    return true;
}

/*******************************************************************************/

static CascadeConfig gConfig;

void NxpConfig_Init(void)
{
    gConfig.init(default_nxp_config_path);
}

void NxpConfig_Deinit(void)
{
    gConfig.deinit();
}

// return true if new per-country configuration file was load.
//        false if it can stay at the current configuration.
bool NxpConfig_SetCountryCode(const char country_code[2])
{
    return gConfig.setCountryCode(country_code);
}

/*******************************************************************************
**
** Function:    NxpConfig_GetStr
**
** Description: API function for getting a string value of a setting
**
** Returns:     True if found, otherwise False.
**
*******************************************************************************/
int NxpConfig_GetStr(const char* name, char* pValue, unsigned long len)
{
    return gConfig.getValue(name, pValue, len);
}

/*******************************************************************************
**
** Function:    NxpConfig_GetByteArray()
**
** Description: Read byte array value from the config file.
**
** Parameters:
**              name    - name of the config param to read.
**              pValue  - pointer to input buffer.
**              bufflen - input buffer length.
**              len     - out parameter to return the number of bytes read from config file,
**                        return -1 in case bufflen is not enough.
**
** Returns:     TRUE[1] if config param name is found in the config file, else FALSE[0]
**
*******************************************************************************/
int NxpConfig_GetByteArray(const char* name, uint8_t* pValue, long bufflen, long *len)
{
    return gConfig.getValue(name, pValue, bufflen,len);
}

/*******************************************************************************
**
** Function:    NxpConfig_GetNum
**
** Description: API function for getting a numerical value of a setting
**
** Returns:     true, if successful
**
*******************************************************************************/
int NxpConfig_GetNum(const char* name, void* pValue, unsigned long len)
{
    if (pValue == NULL){
        return false;
    }
    const uwbParam* pParam = gConfig.find(name);

    if (pParam == NULL)
        return false;
    if (pParam->getType() != uwbParam::type::NUMBER)
        return false;

    unsigned long v = pParam->numValue();
    switch (len)
    {
    case sizeof(unsigned long):
        *(static_cast<unsigned long*>(pValue)) = (unsigned long)v;
        break;
    case sizeof(unsigned short):
        *(static_cast<unsigned short*>(pValue)) = (unsigned short)v;
        break;
    case sizeof(unsigned char):
        *(static_cast<unsigned char*> (pValue)) = (unsigned char)v;
        break;
    default:
        return false;
    }
    return true;
}

// Get the length of a 'string-array' type parameter
int NxpConfig_GetStrArrayLen(const char* name, unsigned long* pLen)
{
    const uwbParam* param = gConfig.find(name);
    if (!param || param->getType() != uwbParam::type::STRINGARRAY)
        return false;

    *pLen = param->str_arr_len();
    return true;
}

// Get a string value from 'string-array' type parameters, index zero-based
int NxpConfig_GetStrArrayVal(const char* name, int index, char* pValue, unsigned long len)
{
    const uwbParam* param = gConfig.find(name);
    if (!param || param->getType() != uwbParam::type::STRINGARRAY)
        return false;
    if (index < 0 || index >= param->str_arr_len())
        return false;

    if (len < param->str_arr_elem_len(index) + 1)
        return false;
    strncpy(pValue, param->str_arr_elem(index), len);
    return true;
}
