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

#include "ringtone_restore_db_utils.h"

#include <unistd.h>
#include "ringtone_db_const.h"
#include "ringtone_log.h"
#include "ringtone_errno.h"
#include "result_set_utils.h"
#include "ringtone_rdb_callbacks.h"
#include "os_account_manager.h"

namespace OHOS {
namespace Media {
const static int32_t CONNECT_SIZE = 10;
const std::string CUSTOM_COUNT = "count";
const std::string PRAGMA_TABLE_NAME = "name";
const std::string PRAGMA_TABLE_TYPE = "type";
const int RDB_AREA_EL1 = 0;
const int PEER_SLOT_NUM = 4;
int32_t RingtoneRestoreDbUtils::InitDb(std::shared_ptr<NativeRdb::RdbStore> &rdbStore, const std::string &dbName,
    const std::string &dbPath, const std::string &bundleName, bool isMediaLibrary)
{
    NativeRdb::RdbStoreConfig config(dbName);
    config.SetPath(dbPath);
    config.SetBundleName(bundleName);
    config.SetReadConSize(CONNECT_SIZE);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S3);
    config.SetArea(RDB_AREA_EL1);

    int32_t err;
    RingtoneDataCallBack cb;
    rdbStore = NativeRdb::RdbHelper::GetRdbStore(config, RINGTONE_RDB_VERSION, cb, err);
    return err;
}

int32_t RingtoneRestoreDbUtils::QueryInt(std::shared_ptr<NativeRdb::RdbStore> rdbStore, const std::string &sql,
    const std::string &column)
{
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("rdb_ is nullptr, Maybe init failed.");
        return 0;
    }
    auto resultSet = rdbStore->QuerySql(sql);
    if (resultSet == nullptr || resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        return 0;
    }
    int32_t result = GetInt32Val(column, resultSet);
    return result;
}

int32_t RingtoneRestoreDbUtils::Update(std::shared_ptr<NativeRdb::RdbStore> &rdbStore, int32_t &changeRows,
    NativeRdb::ValuesBucket &valuesBucket, std::unique_ptr<NativeRdb::AbsRdbPredicates> &predicates)
{
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("rdb_ is nullptr, Maybe init failed.");
        return E_FAIL;
    }
    return rdbStore->Update(changeRows, valuesBucket, *predicates);
}

int32_t RingtoneRestoreDbUtils::QueryRingtoneCount(std::shared_ptr<NativeRdb::RdbStore> rdbStore)
{
    static std::string QUERY_TONEFILES_ALL_COUNT = "SELECT count(1) AS count FROM ToneFiles";
    return QueryInt(rdbStore, QUERY_TONEFILES_ALL_COUNT, CUSTOM_COUNT);
}

std::shared_ptr<NativeRdb::ResultSet> RingtoneRestoreDbUtils::GetQueryResultSet(
    const std::shared_ptr<NativeRdb::RdbStore> &rdbStore, const std::string &querySql)
{
    if (rdbStore == nullptr) {
        RINGTONE_ERR_LOG("rdbStore is nullptr");
        return nullptr;
    }
    return rdbStore->QuerySql(querySql);
}

std::unordered_map<std::string, std::string> RingtoneRestoreDbUtils::GetColumnInfoMap(
    const std::shared_ptr<NativeRdb::RdbStore> &rdbStore, const std::string &tableName)
{
    std::unordered_map<std::string, std::string> columnInfoMap;
    std::string querySql = "SELECT name, type FROM pragma_table_info('" + tableName + "')";
    auto resultSet = GetQueryResultSet(rdbStore, querySql);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("resultSet is nullptr");
        return columnInfoMap;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        std::string columnName = GetStringVal(PRAGMA_TABLE_NAME, resultSet);
        std::string columnType = GetStringVal(PRAGMA_TABLE_TYPE, resultSet);
        if (columnName.empty() || columnType.empty()) {
            RINGTONE_ERR_LOG("Empty column name or type: %{public}s, %{public}s", columnName.c_str(),
                columnType.c_str());
            continue;
        }
        columnInfoMap[columnName] = columnType;
    }
    return columnInfoMap;
}

bool RingtoneRestoreDbUtils::GetUserID(int &userId)
{
    std::vector<int> activeIds;
    int ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeIds);
    if (ret != 0) {
        RINGTONE_ERR_LOG("QueryActiveOsAccountIds failed ret:%{public}d", ret);
        return false;
    }
    if (activeIds.empty()) {
        RINGTONE_ERR_LOG("QueryActiveOsAccountIds activeIds empty");
        return false;
    }
    userId = activeIds[0];
    return true;
}

std::string RingtoneRestoreDbUtils::GetSourceColumnName(ToneSettingType settingType)
{
    std::string ret = "";
    switch (settingType) {
        case TONE_SETTING_TYPE_ALARM:
            ret = RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE;
            break;
        case TONE_SETTING_TYPE_SHOT:
            ret = RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE;
            break;
        case TONE_SETTING_TYPE_RINGTONE:
            ret = RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE;
            break;
        case TONE_SETTING_TYPE_NOTIFICATION:
            ret = RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE;
            break;
        default:
            break;
    }
    return ret;
}

std::string RingtoneRestoreDbUtils::GetModeColumnName(ToneSettingType settingType)
{
    std::string ret = "";
    switch (settingType) {
        case TONE_SETTING_TYPE_ALARM:
            ret = RINGTONE_COLUMN_ALARM_TONE_TYPE;
            break;
        case TONE_SETTING_TYPE_SHOT:
            ret = RINGTONE_COLUMN_SHOT_TONE_TYPE;
            break;
        case TONE_SETTING_TYPE_RINGTONE:
            ret = RINGTONE_COLUMN_RING_TONE_TYPE;
            break;
        case TONE_SETTING_TYPE_NOTIFICATION:
            ret = RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE;
            break;
        default:
            break;
    }
    return ret;
}

static std::string BuildUnusedCustomWhere()
{
    return RINGTONE_COLUMN_SOURCE_TYPE + " = " + std::to_string(SOURCE_TYPE_CUSTOMISED) +
        " AND " + RINGTONE_COLUMN_MEDIA_TYPE + " != " + std::to_string(RINGTONE_MEDIA_TYPE_VIDEO) +
        " AND NOT (" +
        "(" + RINGTONE_COLUMN_SHOT_TONE_TYPE + " != 0 AND " +
        RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = " + std::to_string(SOURCE_TYPE_CUSTOMISED) + ")" +
        " OR (" + RINGTONE_COLUMN_RING_TONE_TYPE + " != 0 AND " +
        RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + " = " + std::to_string(SOURCE_TYPE_CUSTOMISED) + ")" +
        " OR (" + RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " != 0 AND " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = " + std::to_string(SOURCE_TYPE_CUSTOMISED) + ")" +
        " OR (" + RINGTONE_COLUMN_ALARM_TONE_TYPE + " != 0 AND " +
        RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " = " + std::to_string(SOURCE_TYPE_CUSTOMISED) + ")" +
        ")";
}

static void ClearESimSlotBits(const std::shared_ptr<NativeRdb::RdbStore> &rdbStore, int32_t peerSlotMask)
{
    // Mask tone_type bits to keep only peer slots
    std::vector<std::pair<std::string, std::string>> typeSourcePairs = {
        {RINGTONE_COLUMN_SHOT_TONE_TYPE, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE},
        {RINGTONE_COLUMN_RING_TONE_TYPE, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE},
    };
    for (const auto &[typeCol, sourceCol] : typeSourcePairs) {
        std::string maskSql = "UPDATE " + RINGTONE_TABLE + " SET " + typeCol +
            " = " + typeCol + " & " + std::to_string(peerSlotMask) +
            " WHERE " + typeCol + " != 0;";
        int32_t err = rdbStore->ExecuteSql(maskSql);
        if (err != NativeRdb::E_OK) {
            RINGTONE_ERR_LOG("Mask %{public}s failed, err: %{public}d", typeCol.c_str(), err);
        }
        std::string clearSql = "UPDATE " + RINGTONE_TABLE + " SET " + sourceCol +
            " = -1 WHERE " + typeCol + " = 0 AND " + sourceCol + " > 0;";
        err = rdbStore->ExecuteSql(clearSql);
        if (err != NativeRdb::E_OK) {
            RINGTONE_ERR_LOG("Clear %{public}s failed, err: %{public}d", sourceCol.c_str(), err);
        }
    }

    // Delete eSIM SimCardSetting and PreloadConfig records
    std::string deleteSettingSql = "DELETE FROM " + SIMCARD_SETTING_TABLE +
        " WHERE " + SIMCARD_SETTING_COLUMN_MODE + " > 3;";
    int32_t err = rdbStore->ExecuteSql(deleteSettingSql);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Delete eSIM setting failed, err: %{public}d", err);
    }
    std::string deletePreloadSql = "DELETE FROM " + PRELOAD_CONFIG_TABLE +
        " WHERE " + PRELOAD_CONFIG_COLUMN_RING_TONE_TYPE + " > 6;";
    err = rdbStore->ExecuteSql(deletePreloadSql);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Delete eSIM preload failed, err: %{public}d", err);
    }
}

static std::vector<std::string> QueryUnusedCustomPaths(const std::shared_ptr<NativeRdb::RdbStore> &rdbStore)
{
    std::string whereClause = BuildUnusedCustomWhere();
    std::string querySql = "SELECT " + RINGTONE_COLUMN_DATA + " FROM " + RINGTONE_TABLE +
        " WHERE " + whereClause + ";";
    auto resultSet = rdbStore->QuerySql(querySql);
    std::vector<std::string> paths;
    if (resultSet != nullptr) {
        while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
            std::string dataPath;
            resultSet->GetString(0, dataPath);
            if (!dataPath.empty()) {
                paths.push_back(dataPath);
            }
        }
    }
    RINGTONE_INFO_LOG("Found %{public}zu unused custom non-video tones", paths.size());
    return paths;
}

static void DeleteUnusedCustomTones(const std::shared_ptr<NativeRdb::RdbStore> &rdbStore)
{
    std::string whereClause = BuildUnusedCustomWhere();
    std::string deleteSql = "DELETE FROM " + RINGTONE_TABLE + " WHERE " + whereClause + ";";
    int32_t err = rdbStore->ExecuteSql(deleteSql);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Delete unused custom tones failed, err: %{public}d", err);
    }
}

static void DeleteUnusedFiles(const std::vector<std::string> &paths, const std::string &ringtoneBasePath)
{
    if (ringtoneBasePath.empty()) {
        return;
    }
    const std::string ringtonePrefix = "/data/storage/el2/base/files/Ringtone/";
    for (const auto &dataPath : paths) {
        if (dataPath.find(ringtonePrefix) == 0) {
            std::string relativePath = dataPath.substr(ringtonePrefix.length());
            std::string fileToDelete = ringtoneBasePath + relativePath;
            if (unlink(fileToDelete.c_str()) == 0) {
                RINGTONE_INFO_LOG("Deleted unused tone file: %{private}s", fileToDelete.c_str());
            } else {
                RINGTONE_WARN_LOG("Failed to delete unused tone file: %{private}s, errno=%{public}d",
                    fileToDelete.c_str(), errno);
            }
        }
    }
}

int32_t RingtoneRestoreDbUtils::CleanESimData(const std::string &dbPath, int32_t peerSlotNum,
    const std::string &ringtoneBasePath)
{
    RINGTONE_INFO_LOG("CleanESimData enter, dbPath: %{private}s, peerSlotNum: %{public}d, "
        "ringtoneBasePath: %{private}s", dbPath.c_str(), peerSlotNum, ringtoneBasePath.c_str());
    if (peerSlotNum <= 0 || peerSlotNum > PEER_SLOT_NUM) {
        RINGTONE_ERR_LOG("Invalid peerSlotNum: %{public}d", peerSlotNum);
        return E_FAIL;
    }

    std::shared_ptr<NativeRdb::RdbStore> rdbStore = nullptr;
    int32_t err = InitDb(rdbStore, RINGTONE_LIBRARY_DB_NAME, dbPath, RINGTONE_BUNDLE_NAME, false);
    if (err != NativeRdb::E_OK || rdbStore == nullptr) {
        RINGTONE_ERR_LOG("InitDb failed, err: %{public}d", err);
        return E_HAS_DB_ERROR;
    }

    int32_t peerSlotMask = (1 << peerSlotNum) - 1;
    RINGTONE_INFO_LOG("peerSlotMask: %{public}d", peerSlotMask);

    ClearESimSlotBits(rdbStore, peerSlotMask);
    auto unusedPaths = QueryUnusedCustomPaths(rdbStore);
    DeleteUnusedCustomTones(rdbStore);
    DeleteUnusedFiles(unusedPaths, ringtoneBasePath);

    RINGTONE_INFO_LOG("CleanESimData completed");
    return E_OK;
}

} // namespace Media
} // namespace OHOS
