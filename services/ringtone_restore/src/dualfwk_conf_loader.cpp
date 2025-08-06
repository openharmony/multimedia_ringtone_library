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

#include <fstream>
#include <vector>

#include "dualfwk_conf_loader.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_restore_db_utils.h"
#include "ringtone_utils.h"

namespace OHOS {
namespace Media {

static const std::string SETTINGS_DATA_URI_BASE =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/USER_SETTINGSDATA_";
static const std::string SETTINGS_DATA_FIELD_KEY = "KEYWORD";
static const std::string SETTINGS_DATA_FIELD_VAL = "VALUE";
static const std::string ERR_CONFIG_VALUE = "";
static std::vector<std::string> SETTINGS_COLUMNS = {SETTINGS_DATA_FIELD_VAL};
static const int DEFAULT_USERID = 100;

const int CHAR_LOWER_CASE_A = 97;
const int CHAR_LOWER_CASE_D = 100;
const int CHAR_LOWER_CASE_I = 105;
const int CHAR_LOWER_CASE_N = 110;
const int CHAR_LOWER_CASE_O = 111;
const int CHAR_LOWER_CASE_R = 114;
const std::string DUAL_NAME = std::string() + static_cast<char>(CHAR_LOWER_CASE_A) +
    static_cast<char>(CHAR_LOWER_CASE_N) + static_cast<char>(CHAR_LOWER_CASE_D) +
    static_cast<char>(CHAR_LOWER_CASE_R) + static_cast<char>(CHAR_LOWER_CASE_O) +
    static_cast<char>(CHAR_LOWER_CASE_I) + static_cast<char>(CHAR_LOWER_CASE_D);
static const std::string EXTERNAL_DB_NAME = "external.db";
static const std::string EXTERNAL_DB_RESTORE_PATH =
    "/data/storage/el2/base/.backup/restore/com." + DUAL_NAME + ".providers.media.module/ce/databases/external.db";
static const std::string INTERNAL_DB_NAME = "internal.db";
static const std::string INTERNAL_DB_RESTORE_PATH =
    "/data/storage/el2/base/.backup/restore/com." + DUAL_NAME + ".providers.media.module/ce/databases/internal.db";

DualFwkConfLoader::DualFwkConfLoader() {}

int32_t DualFwkConfLoader::Init()
{
    DataShare::CreateOptions options;

    int userId = 0;
    if (!RingtoneRestoreDbUtils::GetUserID(userId)) {
        RINGTONE_DEBUG_LOG("Failed to get userId, using DEFAULT_USERID=%{public}d", DEFAULT_USERID);
        userId = DEFAULT_USERID;
    }
    settingsDataUri_ = SETTINGS_DATA_URI_BASE + std::to_string(userId);
    RINGTONE_DEBUG_LOG("Getting data share helper with SETTINGS_DATA_URI = %{public}s", settingsDataUri_.c_str());

    dataShareHelper_ = DataShare::DataShareHelper::Creator(settingsDataUri_, options);
    if (dataShareHelper_ == nullptr) {
        RINGTONE_WARN_LOG("dataShareHelper_ is null, failed to get data share helper");
        return E_ERR;
    }

    RINGTONE_INFO_LOG("Successfully initialized.");
    return E_OK;
}

static std::string GetSound(const std::string &line)
{
    std::string tag = "sound=\"";
    auto lenTag = tag.size();
    auto positionOfTag = line.find(tag);
    if (positionOfTag == std::string::npos) {
        return "";
    }
    auto positionOfQuote = line.substr(positionOfTag + lenTag).find("\"");
    if (positionOfQuote == std::string::npos) {
        return "";
    }
    return line.substr(positionOfTag + lenTag, positionOfQuote);
}

static int32_t ParseBackupFile(const std::string &backupFile, const std::vector<std::string> &keys,
    std::map<std::string, std::string> &results)
{
    RINGTONE_INFO_LOG("parse backupfile %{public}s uid=%{public}d", backupFile.c_str(), getuid());
    if (RingtoneFileUtils::IsFileExists(backupFile)) {
        RINGTONE_ERR_LOG("the file exists path: %{private}s", backupFile.c_str());
        return E_FILE_EXIST;
    }
    std::ifstream fs(backupFile);
    if (!fs.good()) {
        RINGTONE_ERR_LOG("failed to open file %{private}s", backupFile.c_str());
        return E_ERR;
    }
    
    std::string line;
    while (std::getline(fs, line)) {
        for (const auto &key : keys) {
            auto pos = line.find(key);
            if (pos == std::string::npos) {
                continue;
            }
            std::string value = GetSound(line);
            if (value.size() > 0) {
                results.emplace(key, value);
            }
        }
    }
    return E_SUCCESS;
}

int32_t DualFwkConfLoader::Load(DualFwkConf &conf, const RestoreSceneType &type, const std::string &backupFile)
{
    if (dataShareHelper_ == nullptr) {
        RINGTONE_ERR_LOG("DualFwkConfLoader is not initialized successfully");
        return E_ERR;
    }
    std::map<std::string, std::string> backupConfigs;
    if (ParseBackupFile(backupFile, {"mms_sim1_channel", "mms_sim2_channel"}, backupConfigs) != E_SUCCESS) {
        RINGTONE_WARN_LOG("Failed to parse backup file %{public}s", backupFile.c_str());
    }

    if (type == RestoreSceneType::RESTORE_SCENE_TYPE_DUAL_CLONE) {
        RINGTONE_INFO_LOG("Load configurations for RestoreSceneType::RESTORE_SCENE_TYPE_DUAL_CLONE");
        conf.notificationSoundPath = GetConf("notification_sound_path");
        conf.ringtonePath = GetConf("ringtone_path");
        conf.ringtone2Path = GetConf("ringtone2_path");
        conf.alarmAlertPath = GetConf("alarm_alert_path");
        conf.messagePath = GetConf("message_path");
        conf.messageSub1 = GetConfPath("messageSub1");
    } else {
        RINGTONE_INFO_LOG("Load configurations for RestoreSceneType::RESTORE_SCENE_TYPE_DUAL_UPGRADE");
        conf.notificationSoundPath = GetConf("notification_sound_path");
        conf.ringtonePath = GetConf("ringtone_path");
        conf.ringtone2Path = GetConf("ringtone2_path");
        conf.alarmAlertPath = GetConf("alarm_alert_path");
        conf.messagePath = GetConf("message_path");
        conf.messageSub1 = GetConfPath("messageSub1");
    }
    
    return E_OK;
}

void DualFwkConfLoader::ShowConf(const DualFwkConf &conf)
{
    RINGTONE_DEBUG_LOG("===================================================");
    RINGTONE_DEBUG_LOG("conf.notificationSoundPath  = %{public}s", conf.notificationSoundPath.c_str());
    RINGTONE_DEBUG_LOG("conf.ringtonePath = %{public}s", conf.ringtonePath.c_str());
    RINGTONE_DEBUG_LOG("conf.ringtone2Path = %{public}s", conf.ringtone2Path.c_str());
    RINGTONE_DEBUG_LOG("conf.alarmAlertPath = %{public}s", conf.alarmAlertPath.c_str());
    RINGTONE_DEBUG_LOG("conf.messagePath = %{public}s", conf.messagePath.c_str());
    RINGTONE_DEBUG_LOG("conf.messageSub1 = %{public}s", conf.messageSub1.c_str());
    RINGTONE_DEBUG_LOG("===================================================");
}

std::string DualFwkConfLoader::GetConf(const std::string &key)
{
    DataShare::DataSharePredicates dataSharePredicates;
    dataSharePredicates.EqualTo(SETTINGS_DATA_FIELD_KEY, key);
    Uri uri(settingsDataUri_);
    auto resultSet = dataShareHelper_->Query(uri, dataSharePredicates, SETTINGS_COLUMNS);
    if (resultSet == nullptr) {
        RINGTONE_DEBUG_LOG("resultSet is null, failed to get value of key = %{public}s", key.c_str());
        return "";
    }

    int32_t numRows = 0;
    resultSet->GetRowCount(numRows);
    RINGTONE_INFO_LOG("%{public}d records found with key = %{public}s", numRows, key.c_str());
    if (numRows == 0) {
        RINGTONE_DEBUG_LOG("no record at key = %{public}s, returning an empty string.", key.c_str());
        return "";
    }
    int32_t columnIndex = 0;
    int32_t rowNumber = 0;
    resultSet->GoToRow(rowNumber);
    std::string valueResult = "";
    int32_t ret = resultSet->GetString(columnIndex, valueResult);
    if (ret != 0) {
        RINGTONE_DEBUG_LOG("failed to get string, returning an empty string.");
    } else {
        RINGTONE_DEBUG_LOG("valueResult = %{public}s", valueResult.c_str());
    }
    return valueResult;
}

int32_t DualFwkConfLoader::ValueToDBFileID(const std::string &value, std::string &dbName, std::string &dbPath,
    std::string &fileId)
{
    size_t posExternal = value.find("external");
    if (posExternal != std::string::npos) {
        dbName = EXTERNAL_DB_NAME;
        dbPath = EXTERNAL_DB_RESTORE_PATH;
    }

    size_t posInternal = value.find("internal");
    if (posInternal != std::string::npos) {
        dbName = INTERNAL_DB_NAME;
        dbPath = INTERNAL_DB_RESTORE_PATH;
    }

    size_t pos = value.rfind("/");
    if (pos == std::string::npos || (posExternal == std::string::npos && posInternal == std::string::npos)) {
        RINGTONE_ERR_LOG("Invalid value");
        return E_ERR;
    }

    fileId = value.substr(pos + 1);
    if (!RingtoneUtils::IsNumber(fileId)) {
        RINGTONE_INFO_LOG("Invalid file id %{public}s", fileId.c_str());
        return E_ERR;
    }
    return E_OK;
}

std::string DualFwkConfLoader::GetConfPath(const std::string &key)
{
    std::string dbName;
    std::string dbPath;
    std::string fileId;
    std::string value = GetConf(key);
    size_t pos = value.find("content://media");
    if (pos != 0) {
        RINGTONE_INFO_LOG("key: %{public}s value: %{public}s", key.c_str(), value.c_str());
        return value;
    }

    int32_t ret = ValueToDBFileID(value, dbName, dbPath, fileId);
    if (ret != E_OK) {
        return ERR_CONFIG_VALUE;
    }

    if (!RingtoneFileUtils::IsFileExists(dbPath)) {
        RINGTONE_ERR_LOG("%{public}s is not exist", dbName.c_str());
        return ERR_CONFIG_VALUE;
    }

    std::shared_ptr<NativeRdb::RdbStore> rdbStore = nullptr;
    int32_t err = RingtoneRestoreDbUtils::InitDb(rdbStore, dbName, dbPath, RINGTONE_BUNDLE_NAME, true);
    if (err != E_OK || rdbStore == nullptr) {
        RINGTONE_ERR_LOG("Init rdbStore fail. err: %{public}d", err);
        return ERR_CONFIG_VALUE;
    }

    std::string querySql = "SELECT _data FROM files WHERE _id = ?";
    std::vector<std::string> args = { fileId };
    auto resultSet = rdbStore->QuerySql(querySql, args);
    if (resultSet == nullptr || resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Query fail.");
        return ERR_CONFIG_VALUE;
    }

    std::string data;
    if (resultSet->GetString(0, data) != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Get data from resultSet fail.");
        return ERR_CONFIG_VALUE;
    }

    RINGTONE_INFO_LOG("Get conf path success, data: %{public}s", data.c_str());
    return data;
}
} // namespace Media
} // namespace OHOS