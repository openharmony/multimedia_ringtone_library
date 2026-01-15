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

} // namespace Media
} // namespace OHOS
