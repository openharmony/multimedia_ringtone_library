/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MLOG_TAG "ringtone_language"

#include "ringtone_language_manager.h"

#include "parameter.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"
#include "ringtone_type.h"
#include "ringtone_file_utils.h"
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif

#include <cstring>
#include <libxml/tree.h>
#include <libxml/parser.h>

namespace OHOS {
namespace Media {
using namespace OHOS::NativeRdb;
using namespace std;

const char *LANGUAGE_KEY = "persist.global.language";
const char *DEFAULT_LANGUAGE_KEY = "const.global.language";
const string CHINESE_ABBREVIATION = "zh-Hans";
const string ENGLISH_ABBREVIATION = "en-Latn-US";
const int32_t SYSPARA_SIZE = 64;
const int32_t SYSINIT_TYPE = 1;
const int32_t STANDARDVIBRATION = 1;
const int32_t UNKNOWN_INDEX = -1;

#ifdef USE_CONFIG_POLICY
static constexpr char RINGTONE_MULTILINGUAL_FILE_PATH[] =
    "etc/resource/media/audio/ringtone_list_language.xml";
static constexpr char VIBRATION_MULTILINGUAL_FILE_PATH[] =
    "etc/resource/media/haptics/vibration_list_language.xml";
#else
static constexpr char RINGTONE_MULTILINGUAL_FILE_PATH[] =
    "/system/variant/phone/base/etc/resource/media/audio/ringtone_list_language.xml";
static constexpr char VIBRATION_MULTILINGUAL_FILE_PATH[] =
    "/system/variant/phone/base/etc/resource/media/haptics/vibration_list_language.xml";
#endif

shared_ptr<RingtoneLanguageManager> RingtoneLanguageManager::instance_ = nullptr;
mutex RingtoneLanguageManager::mutex_;

RingtoneLanguageManager::RingtoneLanguageManager(void)
{
}

RingtoneLanguageManager::~RingtoneLanguageManager(void)
{
}

shared_ptr<RingtoneLanguageManager> RingtoneLanguageManager::GetInstance()
{
    if (instance_ == nullptr) {
        lock_guard<mutex> lock(mutex_);

        if (instance_ == nullptr) {
            instance_ = make_shared<RingtoneLanguageManager>();
        }
    }
    return instance_;
}

void RingtoneLanguageManager::SyncAssetLanguage()
{
    RINGTONE_INFO_LOG("SyncAssetLanguage start.");
    systemLanguage_ = GetSystemLanguage();
    if (systemLanguage_.empty()) {
        RINGTONE_ERR_LOG("Failed to get system language");
        return;
    }
    RINGTONE_INFO_LOG("system language is %{public}s", systemLanguage_.c_str());
    if (strncmp(systemLanguage_.c_str(), CHINESE_ABBREVIATION.c_str(), CHINESE_ABBREVIATION.size()) == 0) {
        systemLanguage_ = CHINESE_ABBREVIATION;
    } else {
        systemLanguage_ = ENGLISH_ABBREVIATION;
    }
    UpdateRingtoneLanguage();
    UpdateVibrationLanguage();
    RINGTONE_INFO_LOG("SyncAssetLanguage end.");
}

string RingtoneLanguageManager::GetSystemLanguage()
{
    char param[SYSPARA_SIZE] = {0};
    int status = GetParameter(LANGUAGE_KEY, "", param, SYSPARA_SIZE);
    if (status > 0) {
        return param;
    }
    status = GetParameter(DEFAULT_LANGUAGE_KEY, "", param, SYSPARA_SIZE);
    if (status > 0) {
        return param;
    }
    return "";
}

void RingtoneLanguageManager::UpdateRingtoneLanguage()
{
    RINGTONE_INFO_LOG("UpdateRingtonLanguage start.");
    int32_t rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    if (CheckLanguageTypeByRingtone(rowCount, resultSet) != E_OK) {
        return;
    }
    RINGTONE_INFO_LOG("%{public}d ring tones need to be sync", rowCount);
    if (rowCount == 0) {
        return;
    }
#ifdef USE_CONFIG_POLICY
    char buf[MAX_PATH_LEN] = {0};
    char *path = GetOneCfgFile(RINGTONE_MULTILINGUAL_FILE_PATH, buf, MAX_PATH_LEN);
    if (path == nullptr || *path == '\0') {
        RINGTONE_ERR_LOG("GetOneCfgFile for %{public}s failed.", RINGTONE_MULTILINGUAL_FILE_PATH);
        return;
    }
#else
    const char *path = RINGTONE_MULTILINGUAL_FILE_PATH;
#endif

    if (!ReadMultilingualResources(path, RINGTONE_FILE)) {
        return;
    }
    ChangeLanguageDataToRingtone(rowCount, resultSet);
    RINGTONE_INFO_LOG("UpdateRingtonLanguage end.");
}

void RingtoneLanguageManager::UpdateVibrationLanguage()
{
    RINGTONE_INFO_LOG("UpdateVibrationLanguage start.");
    int32_t rowCount = 0;
    std::shared_ptr<NativeRdb::ResultSet> resultSet;
    if (CheckLanguageTypeByVibration(rowCount, resultSet) != E_OK) {
        return;
    }
    RINGTONE_INFO_LOG("%{public}d vibration need to be sync", rowCount);
    if (rowCount == 0) {
        return;
    }
#ifdef USE_CONFIG_POLICY
    char buf[MAX_PATH_LEN] = {0};
    char *path = GetOneCfgFile(VIBRATION_MULTILINGUAL_FILE_PATH, buf, MAX_PATH_LEN);
    if (path == nullptr || *path == '\0') {
        RINGTONE_ERR_LOG("GetOneCfgFile for %{public}s failed.", VIBRATION_MULTILINGUAL_FILE_PATH);
        return;
    }
#else
    const char *path = VIBRATION_MULTILINGUAL_FILE_PATH;
#endif

    if (!ReadMultilingualResources(path, VIBRATION_FILE)) {
        return;
    }
    ChangeLanguageDataToVibration(rowCount, resultSet);
    RINGTONE_INFO_LOG("UpdateVibrationLanguage end.");
}

int32_t RingtoneLanguageManager::CheckLanguageTypeByRingtone(int32_t &rowCount,
    shared_ptr<ResultSet> &resultSet)
{
    vector<string> columns = {
        RINGTONE_COLUMN_TONE_ID,
        RINGTONE_COLUMN_DATA
    };

    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("failed to get raw rdb");
        return E_RDB;
    }

    AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
    absRdbPredicates.EqualTo(RINGTONE_COLUMN_SOURCE_TYPE, SYSINIT_TYPE);
    absRdbPredicates.And();
    absRdbPredicates.BeginWrap();
    absRdbPredicates.NotEqualTo(RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE, systemLanguage_);
    absRdbPredicates.Or();
    absRdbPredicates.IsNull(RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE);
    absRdbPredicates.EndWrap();
    resultSet = rawRdb->Query(absRdbPredicates, columns);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("failed to query rdb");
        return E_RDB;
    }

    int32_t ret = resultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get resultSet row count");
        return E_RDB;
    }
    return E_OK;
}

void RingtoneLanguageManager::ChangeLanguageDataToRingtone(int32_t rowCount,
    const std::shared_ptr<ResultSet> &resultSet)
{
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("failed to get raw rdb");
        return;
    }

    map<string, int> fieldIndex = {
        { RINGTONE_COLUMN_TONE_ID, UNKNOWN_INDEX },
        { RINGTONE_COLUMN_DATA, UNKNOWN_INDEX }
    };
    if (GetFieldIndex(resultSet, fieldIndex) != E_OK) {
        return;
    }

    for (int i = 0; i < rowCount; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            RINGTONE_ERR_LOG("failed to goto row : %{public}d", i);
            return;
        }

        ValuesBucket values;
        int ringtoneId;
        if (SetValuesFromResultSet(resultSet, fieldIndex, values, ringtoneId, RINGTONE_FILE) == E_OK) {
            AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
            absRdbPredicates.EqualTo(RINGTONE_COLUMN_TONE_ID, ringtoneId);
            int32_t changedRows;
            int32_t result = rawRdb->Update(changedRows, values, absRdbPredicates);
            if (result != E_OK || changedRows <= 0) {
                RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
                return;
            }
        }
    }
}

int32_t RingtoneLanguageManager::GetFieldIndex(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
    std::map<std::string, int> &fieldIndex)
{
    for (auto& field : fieldIndex) {
        if (resultSet->GetColumnIndex(field.first, field.second) != E_OK) {
            RINGTONE_ERR_LOG("failed to get field index");
            return E_RDB;
        }
    }
    return E_OK;
}

int32_t RingtoneLanguageManager::SetValuesFromResultSet(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
    const std::map<std::string, int> &fieldIndex, NativeRdb::ValuesBucket &values, int32_t &indexId,
    ResourceFileType resourceFileType)
{
    string data;
    string idIndexField = resourceFileType == RINGTONE_FILE ? RINGTONE_COLUMN_TONE_ID : VIBRATE_COLUMN_VIBRATE_ID;
    string dataIndexField = resourceFileType == RINGTONE_FILE ? RINGTONE_COLUMN_DATA : VIBRATE_COLUMN_DATA;
    string titleIndexField = resourceFileType == RINGTONE_FILE ? RINGTONE_COLUMN_TITLE : VIBRATE_COLUMN_TITLE;
    string languageIndexField = resourceFileType == RINGTONE_FILE ?
        RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE : VIBRATE_COLUMN_DISPLAY_LANGUAGE;
    auto& translation = resourceFileType == RINGTONE_FILE ? ringtoneTranslate_ : vibrationTranslate_;

    auto idItem = fieldIndex.find(idIndexField);
    if (idItem == fieldIndex.end()) {
        RINGTONE_ERR_LOG("failed to get %{public}s index", idIndexField.c_str());
        return E_RDB;
    }
    if (resultSet->GetInt(idItem->second, indexId) != E_OK) {
        RINGTONE_ERR_LOG("failed to get tone_id value");
        return E_RDB;
    }

    auto dataItem = fieldIndex.find(dataIndexField);
    if (dataItem == fieldIndex.end()) {
        RINGTONE_ERR_LOG("failed to get %{public}s index", dataIndexField.c_str());
        return E_RDB;
    }
    if (resultSet->GetString(dataItem->second, data) != E_OK) {
        RINGTONE_ERR_LOG("failed to get tone_id value");
        return E_RDB;
    }

    values.PutString(languageIndexField, systemLanguage_);
    string realName = RingtoneFileUtils::GetBaseNameFromPath(data);
    auto item = translation[systemLanguage_].find(realName);
    if (item == translation[systemLanguage_].end()) {
        return E_OK;
    }
    string titleName = item->second;
    values.PutString(titleIndexField, titleName);
    return E_OK;
}

int32_t RingtoneLanguageManager::CheckLanguageTypeByVibration(int32_t &rowCount,
    std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    vector<string> columns = {
        VIBRATE_COLUMN_VIBRATE_ID,
        VIBRATE_COLUMN_DATA
    };

    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("failed to get raw rdb");
        return E_RDB;
    }

    AbsRdbPredicates absRdbPredicates(VIBRATE_TABLE);
    absRdbPredicates.EqualTo(VIBRATE_COLUMN_VIBRATE_TYPE, STANDARDVIBRATION);
    absRdbPredicates.And();
    absRdbPredicates.BeginWrap();
    absRdbPredicates.NotEqualTo(VIBRATE_COLUMN_DISPLAY_LANGUAGE, systemLanguage_);
    absRdbPredicates.Or();
    absRdbPredicates.IsNull(VIBRATE_COLUMN_DISPLAY_LANGUAGE);
    absRdbPredicates.EndWrap();
    resultSet = rawRdb->Query(absRdbPredicates, columns);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("failed to query rdb");
        return E_RDB;
    }

    int32_t ret = resultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get resultSet row count");
        return E_RDB;
    }
    return E_OK;
}

void RingtoneLanguageManager::ChangeLanguageDataToVibration(int32_t rowCount,
    const std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("failed to get raw rdb");
        return;
    }

    map<string, int> fieldIndex = {
        { VIBRATE_COLUMN_VIBRATE_ID, UNKNOWN_INDEX },
        { VIBRATE_COLUMN_DATA, UNKNOWN_INDEX }
    };
    if (GetFieldIndex(resultSet, fieldIndex) != E_OK) {
        return;
    }

    for (int i = 0; i < rowCount; i++) {
        if (resultSet->GoToRow(i) != E_OK) {
            RINGTONE_ERR_LOG("failed to goto row : %{public}d", i);
            return;
        }

        ValuesBucket values;
        int vibrateId;
        if (SetValuesFromResultSet(resultSet, fieldIndex, values, vibrateId, VIBRATION_FILE) == E_OK) {
            AbsRdbPredicates absRdbPredicates(VIBRATE_TABLE);
            absRdbPredicates.EqualTo(VIBRATE_COLUMN_VIBRATE_ID, vibrateId);
            int32_t changedRows;
            int32_t result = rawRdb->Update(changedRows, values, absRdbPredicates);
            if (result != E_OK || changedRows <= 0) {
                RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
                return;
            }
        }
    }
}

bool RingtoneLanguageManager::ReadMultilingualResources(const string &filePath, ResourceFileType resourceFileType)
{
    std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)> docPtr(
        xmlReadFile(filePath.c_str(), nullptr, XML_PARSE_NOBLANKS), xmlFreeDoc);
    if (docPtr == nullptr) {
        RINGTONE_ERR_LOG("failed to read xml file [%{public}s]", filePath.c_str());
        xmlErrorPtr error = xmlGetLastError();
        if (error != nullptr) {
            RINGTONE_ERR_LOG("Error: %{public}s (line %{public}d): %{public}s",
                error->file, error->line, error->message);
            xmlResetLastError();
        }
        return false;
    }

    xmlNodePtr rootNode = xmlDocGetRootElement(docPtr.get());
    if (rootNode == nullptr) {
        RINGTONE_ERR_LOG("failed to read root node");
        return false;
    }
    if (resourceFileType == RINGTONE_FILE) {
        if (xmlStrcmp(rootNode->name, BAD_CAST "RingtoneList") != 0) {
            RINGTONE_ERR_LOG("failed to root node name is not matched");
            return false;
        }
        ringtoneTranslate_.clear();
    } else if (resourceFileType == VIBRATION_FILE) {
        if (xmlStrcmp(rootNode->name, BAD_CAST "VibrationList") != 0) {
            RINGTONE_ERR_LOG("failed to root node name is not matched");
            return false;
        }
        vibrationTranslate_.clear();
    }
    return ParseMultilingualXml(rootNode, resourceFileType);
}

bool RingtoneLanguageManager::ParseMultilingualXml(xmlNodePtr &rootNode, ResourceFileType resourceFileType)
{
    for (xmlNodePtr itemNode = rootNode->children; itemNode; itemNode = itemNode->next) {
        if (xmlStrcmp(itemNode->name, BAD_CAST "Language") != 0) {
            continue;
        }

        string language;
        auto xmlLanguage = reinterpret_cast<char*>(xmlGetProp(itemNode, BAD_CAST "type"));
        if (xmlLanguage != nullptr) {
            language = string(xmlLanguage);
            xmlFree(xmlLanguage);
        }

        for (xmlNodePtr childNode = itemNode->children; childNode; childNode = childNode->next) {
            if (resourceFileType == RINGTONE_FILE && xmlStrcmp(childNode->name, BAD_CAST "Ring") != 0) {
                RINGTONE_ERR_LOG("failed to ringtone child node name is not matched");
                return false;
            } else if (resourceFileType == VIBRATION_FILE && xmlStrcmp(childNode->name, BAD_CAST "vibrtion") != 0) {
                RINGTONE_ERR_LOG("failed to vibrate child node name is not matched");
                return false;
            }

            string resourceName, displayName;
            auto xmlResourceName = reinterpret_cast<char*>(xmlGetProp(childNode, BAD_CAST "resource_name"));
            if (xmlResourceName) {
                resourceName = string(xmlResourceName);
                xmlFree(xmlResourceName);
            }

            auto xmlDisplayName = reinterpret_cast<char*>(xmlGetProp(childNode, BAD_CAST "title"));
            if (xmlDisplayName) {
                displayName = string(xmlDisplayName);
                xmlFree(xmlDisplayName);
            }

            if (resourceFileType == RINGTONE_FILE && !resourceName.empty() && !displayName.empty()) {
                ringtoneTranslate_[language][resourceName] = displayName;
            } else if (resourceFileType == VIBRATION_FILE && !resourceName.empty() && !displayName.empty()) {
                vibrationTranslate_[language][resourceName] = displayName;
            }
        }
    }
    return true;
}

} // namespace Media
} // namespace OHOS
