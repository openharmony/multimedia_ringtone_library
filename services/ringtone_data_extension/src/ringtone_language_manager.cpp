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
const int32_t UNKNOWN_INDEX = -1;
const int32_t FIELD_LENGTH = 2;
const string RINGTONE_MULTILINGUAL_FILE_PATH = "/system/etc/resource/media/audio/ringtone_list_language.xml";

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
    if (!ReadMultilingualResources(RINGTONE_MULTILINGUAL_FILE_PATH, RINGTONE_FILE)) {
        return;
    }
    ChangeLanguageDataToRingtone(rowCount, resultSet);
    RINGTONE_INFO_LOG("UpdateRingtonLanguage end.");
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

    vector<pair<string, int>> fieldIndex = {
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
        if (SetValuesFromResultSet(resultSet, fieldIndex, values, ringtoneId) == E_OK) {
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

int32_t RingtoneLanguageManager::GetFieldIndex(const shared_ptr<ResultSet> &resultSet,
    vector<pair<string, int>> &fieldIndex)
{
    for (auto& field : fieldIndex) {
        if (resultSet->GetColumnIndex(field.first, field.second) != E_OK) {
            RINGTONE_ERR_LOG("failed to get field index");
            return E_RDB;
        }
    }
    return E_OK;
}

int32_t RingtoneLanguageManager::SetValuesFromResultSet(const shared_ptr<ResultSet> &resultSet,
    const vector<pair<string, int>> &fieldIndex, ValuesBucket &values, int32_t &ringtoneId)
{
    string data;
    if (fieldIndex.size() < FIELD_LENGTH) {
        return E_RDB;
    }
    if (resultSet->GetInt(fieldIndex[0].second, ringtoneId) != E_OK) {
        RINGTONE_ERR_LOG("failed to get tone_id value");
        return E_RDB;
    }
    if (resultSet->GetString(fieldIndex[1].second, data) != E_OK) {
        RINGTONE_ERR_LOG("failed to get data value");
        return E_RDB;
    }
    values.PutString(RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE, systemLanguage_);
    string realName = RingtoneFileUtils::GetBaseNameFromPath(data);
    auto item = ringtoneTranslate_[systemLanguage_].find(realName);
    if (item == ringtoneTranslate_[systemLanguage_].end()) {
        return E_OK;
    }
    string titleName = item->second;
    values.PutString(RINGTONE_COLUMN_TITLE, titleName);
    return E_OK;
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
                RINGTONE_ERR_LOG("failed to child node name is not matched");
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
            }
        }
    }
    return true;
}

} // namespace Media
} // namespace OHOS
