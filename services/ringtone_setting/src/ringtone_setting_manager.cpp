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
#define MLOG_TAG "RingtoneSettingManager"

#include "ringtone_setting_manager.h"

#include "rdb_errno.h"
#include "result_set_utils.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_metadata.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
using namespace std;
static const string RINGTONE_SETTINGS_SELECT_COLUMNS =
    RINGTONE_COLUMN_TONE_ID + ", " + RINGTONE_COLUMN_DATA + ", " + RINGTONE_COLUMN_SHOT_TONE_TYPE + ", " +
    RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + ", " + RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + ", " +
    RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + ", " + RINGTONE_COLUMN_RING_TONE_TYPE + ", " +
    RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + ", " + RINGTONE_COLUMN_ALARM_TONE_TYPE + ", " +
    RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE;

static const string RINGTONE_SETTINGS_WHERECLAUSE_SHOT = RINGTONE_COLUMN_SHOT_TONE_TYPE + " <> " +
    to_string(SHOT_TONE_TYPE_NOT);

static const string RINGTONE_SETTINGS_WHERECLAUSE_ALARM = RINGTONE_COLUMN_ALARM_TONE_TYPE + " <> " +
    to_string(ALARM_TONE_TYPE_NOT);

static const string RINGTONE_SETTINGS_WHERECLAUSE_NOTIFICATION = RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " <> " +
    to_string(NOTIFICATION_TONE_TYPE_NOT);

static const string RINGTONE_SETTINGS_WHERECLAUSE_RINGTONE = RINGTONE_COLUMN_RING_TONE_TYPE + " <> " +
    to_string(RING_TONE_TYPE_NOT);

static const string QUERY_SHOTTONE_SETTINGS_SQL = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS + " FROM "
    + RINGTONE_TABLE + " WHERE " + RINGTONE_SETTINGS_WHERECLAUSE_SHOT;

static const string QUERY_ALARMTONE_SETTINGS_SQL = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS + " FROM "
    + RINGTONE_TABLE + " WHERE " + RINGTONE_SETTINGS_WHERECLAUSE_ALARM;

static const string QUERY_NOTIFICATIONTONE_SETTINGS_SQL = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS +
    " FROM " + RINGTONE_TABLE + " WHERE " + RINGTONE_SETTINGS_WHERECLAUSE_NOTIFICATION;

static const string QUERY_RINGTONE_SETTINGS_SQL = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS + " FROM " +
    RINGTONE_TABLE + " WHERE " + RINGTONE_SETTINGS_WHERECLAUSE_RINGTONE;

static const string QUERY_SETTINGS_BY_PATH = "SELECT " + RINGTONE_SETTINGS_SELECT_COLUMNS + " FROM "
    + RINGTONE_TABLE + " WHERE " + RINGTONE_COLUMN_DATA + " = ";

RingtoneSettingManager::RingtoneSettingManager(std::shared_ptr<NativeRdb::RdbStore> rdb) : ringtoneRdb_(rdb)
{
}

int32_t RingtoneSettingManager::CommitSettingCompare(int32_t settingType, int32_t toneType, int32_t sourceType)
{
    if ((sourceType != SOURCE_TYPE_PRESET) && (sourceType != SOURCE_TYPE_CUSTOMISED)) {
        return E_INVALID_ARGUMENTS;
    }
    if ((settingType < TONE_SETTING_TYPE_ALARM) || (settingType >= TONE_SETTING_TYPE_MAX)) {
        return E_INVALID_ARGUMENTS;
    }
    if (((settingType == TONE_SETTING_TYPE_NOTIFICATION) && (toneType != NOTIFICATION_TONE_TYPE)) ||
        ((settingType == TONE_SETTING_TYPE_ALARM) && (toneType != ALARM_TONE_TYPE))) {
        return E_INVALID_ARGUMENTS;
    }
    if ((settingType == TONE_SETTING_TYPE_SHOT) && ((toneType <= SHOT_TONE_TYPE_NOT) ||
        (toneType >= TONE_SETTING_TYPE_MAX))) {
        return E_INVALID_ARGUMENTS;
    }
    if ((settingType == TONE_SETTING_TYPE_RINGTONE) && ((toneType <= RING_TONE_TYPE_NOT) ||
        (toneType >= TONE_SETTING_TYPE_MAX))) {
        return E_INVALID_ARGUMENTS;
    }
    return E_OK;
}

int32_t RingtoneSettingManager::CommitSetting(int32_t toneId, const string &tonePath, int32_t settingType,
    int32_t toneType, int32_t sourceType)
{
    RINGTONE_INFO_LOG("toneId=%{public}d, tonePath=%{public}s, settingType=%{public}d, toneType=%{public}d,"
        "sourceType=%{public}d", toneId, tonePath.c_str(), settingType, toneType, sourceType);
    auto ret = CommitSettingCompare(settingType, toneType, sourceType);
    if (ret != E_OK) {
        return ret;
    }
    SettingItem item = {toneId, settingType, toneType, sourceType};
    for (auto it = settings_.find(tonePath); it != settings_.end(); it++) {
        if ((settingType == it->second.settingType) && (toneType == it->second.toneType) &&
            (sourceType == it->second.sourceType)) {
            RINGTONE_INFO_LOG("warning: this setting is existing, tone file:%{public}s", tonePath.c_str());
            return E_FAIL;
        } else if ((settingType == it->second.settingType) && (sourceType == it->second.sourceType)) {
            if (((settingType == TONE_SETTING_TYPE_SHOT) && (it->second.toneType == SHOT_TONE_TYPE_SIM_CARD_BOTH)) ||
                ((settingType == TONE_SETTING_TYPE_RINGTONE) &&
                (it->second.toneType == SHOT_TONE_TYPE_SIM_CARD_BOTH))) {
                RINGTONE_INFO_LOG("warning: this setting is existing, tone file:%{public}s", tonePath.c_str());
                return E_FAIL;
            }
            if ((settingType == TONE_SETTING_TYPE_SHOT) && (toneType != it->second.toneType)) {
                it->second.toneType = SHOT_TONE_TYPE_SIM_CARD_BOTH;
                return E_OK;
            }
            if ((settingType == TONE_SETTING_TYPE_RINGTONE) && (toneType != it->second.toneType)) {
                it->second.toneType = RING_TONE_TYPE_SIM_CARD_BOTH;
                return E_OK;
            }
        } else {
            continue;
        }
    }
    settings_.emplace(tonePath, item);
    return E_OK;
}

void RingtoneSettingManager::TravelSettings(function<int32_t (const string &, SettingItem &)> func)
{
    for (auto it = settings_.cbegin(); it != settings_.cend(); ++it) {
        string first = it->first;
        SettingItem item = it->second;
        func(first, item);
    }
}

void RingtoneSettingManager::FlushSettings()
{
    TravelSettings([this](const string &tonePath, SettingItem &item) -> int32_t {
        int32_t ret = CleanupSetting(item.settingType, item.toneType, item.sourceType);
        if (ret != E_OK) {
            RINGTONE_ERR_LOG("error: cleanup settings failed, tonePath=%{public}s", tonePath.c_str());
        }
        return ret;
    });

    TravelSettings([this](const string &tonePath, SettingItem &item) -> int32_t {
        int32_t ret = this->UpdateSettingsByPath(tonePath, item.settingType, item.toneType, item.sourceType);
        if (ret != E_OK) {
            RINGTONE_ERR_LOG("error: update settings failed, tonePath=%{public}s", tonePath.c_str());
        }
        return ret;
    });

    settings_.clear();
}

int32_t RingtoneSettingManager::PopulateMetadata(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metaData)
{
    std::vector<std::string> columnNames;
    int32_t err = resultSet->GetAllColumnNames(columnNames);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get all column names");
        return E_RDB;
    }

    for (const auto &col : columnNames) {
        ExtractMetaFromColumn(resultSet, metaData, col);
    }

    return E_OK;
}

void RingtoneSettingManager::ExtractMetaFromColumn(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metadata, const std::string &col)
{
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_NULL;
    RingtoneMetadata::RingtoneMetadataFnPtr requestFunc = nullptr;
    auto itr = metadata->memberFuncMap_.find(col);
    if (itr != metadata->memberFuncMap_.end()) {
        dataType = itr->second.first;
        requestFunc = itr->second.second;
    } else {
        RINGTONE_ERR_LOG("column name invalid %{private}s", col.c_str());
        return;
    }

    std::variant<int32_t, std::string, int64_t, double> data =
        ResultSetUtils::GetValFromColumn<const shared_ptr<NativeRdb::ResultSet>>(col, resultSet, dataType);

    // Use the function pointer from map and pass data to fn ptr
    if (requestFunc != nullptr) {
        (metadata.get()->*requestFunc)(data);
    }
}

int32_t RingtoneSettingManager::GetMetaDataFromResultSet(shared_ptr<NativeRdb::ResultSet> resultSet,
    vector<shared_ptr<RingtoneMetadata>> &metaDatas)
{
    if (resultSet == nullptr) {
        RINGTONE_INFO_LOG("invalid argument");
        return E_INVALID_ARGUMENTS;
    }
    auto ret = resultSet->GoToFirstRow();
    while (ret == NativeRdb::E_OK) {
        auto metaData = make_unique<RingtoneMetadata>();
        if (PopulateMetadata(resultSet, metaData) != E_OK) {
            resultSet->Close();
            RINGTONE_INFO_LOG("read resultset error");
            return E_DB_FAIL;
        }
        metaDatas.push_back(std::move(metaData));
        ret = resultSet->GoToNextRow();
    };

    return E_OK;
}

int32_t RingtoneSettingManager::UpdateShotSetting(shared_ptr<RingtoneMetadata> &meta, int32_t toneType,
    int32_t sourceType)
{
    int32_t val = meta->GetShotToneType();
    if (val != SHOT_TONE_TYPE_NOT && val != toneType) {
        val = SHOT_TONE_TYPE_SIM_CARD_BOTH;
    } else {
        val = toneType;
    }
    string updateSql = "UPDATE ToneFiles SET " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " + to_string(val) + ", " +
        RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = " + to_string(sourceType) +
        " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(meta->GetToneId());
    int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
    if (rdbRet < 0) {
        RINGTONE_ERR_LOG("execute update failed");
        return E_DB_FAIL;
    }

    return E_OK;
}

int32_t RingtoneSettingManager::UpdateRingtoneSetting(shared_ptr<RingtoneMetadata> &meta, int32_t toneType,
    int32_t sourceType)
{
    int32_t val = meta->GetRingToneType();
    if (val != RING_TONE_TYPE_NOT && val != toneType) {
        val = RING_TONE_TYPE_SIM_CARD_BOTH;
    } else {
        val = toneType;
    }
    string updateSql = "UPDATE ToneFiles SET " +
        RINGTONE_COLUMN_RING_TONE_TYPE + " = " + to_string(val) + ", " +
        RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + " = " + to_string(sourceType) +
        " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(meta->GetToneId());
    int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
    if (rdbRet < 0) {
        RINGTONE_ERR_LOG("execute update failed");
        return E_DB_FAIL;
    }

    return E_OK;
}

int32_t RingtoneSettingManager::UpdateNotificationSetting(shared_ptr<RingtoneMetadata> &meta, int32_t toneType,
    int32_t sourceType)
{
    string updateSql = "UPDATE ToneFiles SET " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " = " + to_string(toneType) + ", " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = " + to_string(sourceType) +
        " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(meta->GetToneId());
    int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
    if (rdbRet < 0) {
        RINGTONE_ERR_LOG("execute update failed");
        return E_DB_FAIL;
    }

    return E_OK;
}

int32_t RingtoneSettingManager::UpdateAlarmSetting(shared_ptr<RingtoneMetadata> &meta, int32_t toneType,
    int32_t sourceType)
{
    string updateSql = "UPDATE ToneFiles SET " +
        RINGTONE_COLUMN_ALARM_TONE_TYPE + " = " + to_string(toneType) + ", " +
        RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " = " + to_string(sourceType) +
        " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(meta->GetToneId());
    int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
    if (rdbRet < 0) {
        RINGTONE_ERR_LOG("execute update failed");
        return E_DB_FAIL;
    }

    return E_OK;
}

int32_t RingtoneSettingManager::UpdateSettingsByPath(const string &tonePath, int32_t settingType, int32_t toneType,
    int32_t sourceType)
{
    string querySql = QUERY_SETTINGS_BY_PATH + "\"" + tonePath + "\"";
    auto ret = TravelQueryResultSet(querySql, [&](shared_ptr<RingtoneMetadata> &meta) -> bool {
        string updateSql = {};
        bool result = true;
        switch (settingType) {
            case TONE_SETTING_TYPE_SHOT:
                result = UpdateShotSetting(meta, toneType, sourceType) == E_OK;
                break;
            case TONE_SETTING_TYPE_RINGTONE:
                result = UpdateRingtoneSetting(meta, toneType, sourceType) == E_OK;
                break;
            case TONE_SETTING_TYPE_NOTIFICATION:
                result = UpdateNotificationSetting(meta, toneType, sourceType) == E_OK;
                break;
            case TONE_SETTING_TYPE_ALARM:
                result = UpdateAlarmSetting(meta, toneType, sourceType) == E_OK;
                break;
            default:
                RINGTONE_INFO_LOG("invalid tone-setting-type");
                result =  false;
                break;
        }
        return result;
    });

    return ret;
}

int32_t RingtoneSettingManager::UpdateSettingsWithToneId(int32_t settingType, int32_t toneId, int32_t toneType)
{
    int32_t ret = E_OK;

    string updateSql = {};
    switch (settingType) {
        case TONE_SETTING_TYPE_SHOT:
            // update shot-tone settings
            updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " +
                to_string(toneType) + " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
            break;
        case TONE_SETTING_TYPE_RINGTONE:
            // update ring-tone settings
            updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_RING_TONE_TYPE + " = " +
                to_string(toneType) + " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
            break;
        case TONE_SETTING_TYPE_NOTIFICATION:
            // update notification-tone settings
            updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " = " +
                to_string(toneType) + " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
            break;
        case TONE_SETTING_TYPE_ALARM:
            // update alarm-tone settings
            updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_ALARM_TONE_TYPE + " = " +
                to_string(toneType) + " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
            break;
        default:
            RINGTONE_INFO_LOG("invalid tone-setting-type");
            return E_INVALID_ARGUMENTS;
    }
    if (!updateSql.empty()) {
        int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
        if (rdbRet < 0) {
            RINGTONE_ERR_LOG("execute update failed");
            ret = E_DB_FAIL;
        }
    }
    return ret;
}

static const string SHOT_SETTING_CLEANUP_CLAUSE = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " +
    to_string(SHOT_TONE_TYPE_DEFAULT) + ", " + RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = " +
    to_string(SHOT_TONE_SOURCE_TYPE_DEFAULT);

static const string RINGTONE_SETTING_CLEANUP_CLAUSE = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_RING_TONE_TYPE + " = " +
    to_string(RING_TONE_TYPE_DEFAULT) + ", " + RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + "=" +
    to_string(RING_TONE_SOURCE_TYPE_DEFAULT);

int32_t RingtoneSettingManager::CleanupSettingFromRdb(int32_t settingType, int32_t toneType, int32_t sourceType)
{
    int32_t ret = E_OK;
    string updateSql = {};
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        if (toneType == SHOT_TONE_TYPE_SIM_CARD_BOTH) {
            updateSql = SHOT_SETTING_CLEANUP_CLAUSE + " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " <> " +
                to_string(SHOT_TONE_TYPE_DEFAULT) + " AND " + RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = " +
                to_string(sourceType);
        } else {
            updateSql = SHOT_SETTING_CLEANUP_CLAUSE + " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " +
                to_string(toneType) + " AND " + RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = " + to_string(sourceType);
        }
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        if (toneType == RING_TONE_TYPE_SIM_CARD_BOTH) {
            updateSql = RINGTONE_SETTING_CLEANUP_CLAUSE  + " WHERE " + RINGTONE_COLUMN_RING_TONE_TYPE + " <> " +
                to_string(RING_TONE_TYPE_DEFAULT) + " AND " + RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + " = " +
                to_string(sourceType);
        } else {
            updateSql = RINGTONE_SETTING_CLEANUP_CLAUSE  + " WHERE " + RINGTONE_COLUMN_RING_TONE_TYPE + " = " +
                to_string(toneType) + " AND " + RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + " = " +
                to_string(sourceType);
        }
    } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " = " +
            to_string(NOTIFICATION_TONE_TYPE_NOT) + ", " + RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = " +
            to_string(NOTIFICATION_TONE_SOURCE_TYPE_DEFAULT) + " WHERE " + RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE +
            " = " + to_string(toneType) + " AND " + RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = " +
            to_string(sourceType);
    } else if (settingType == TONE_SETTING_TYPE_ALARM) {
        updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_ALARM_TONE_TYPE + " = " +
            to_string(ALARM_TONE_TYPE_NOT) + ", " + RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " = " +
            to_string(ALARM_TONE_SOURCE_TYPE_DEFAULT) + " WHERE " + RINGTONE_COLUMN_ALARM_TONE_TYPE + " = " +
            to_string(toneType) + " AND " + RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " = " +
            to_string(sourceType);
    } else {
        return E_INVALID_ARGUMENTS;
    }
    if (!updateSql.empty()) {
        int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
        if (rdbRet < 0) {
            RINGTONE_ERR_LOG("execute update failed");
            ret = E_DB_FAIL;
        }
    }
    return ret;
}

int32_t RingtoneSettingManager::CleanupSetting(int32_t settingType, int32_t toneType, int32_t sourceType)
{
    if (ringtoneRdb_ == nullptr) {
        RINGTONE_ERR_LOG("ringtone rdb_ is nullptr");
        return E_DB_FAIL;
    }

    string querySql = {};
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        querySql = QUERY_SHOTTONE_SETTINGS_SQL + " AND " + RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = " +
            to_string(sourceType);
    } else if (settingType == TONE_SETTING_TYPE_ALARM) {
        querySql = QUERY_ALARMTONE_SETTINGS_SQL + " AND " + RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " = " +
            to_string(sourceType);
    } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        querySql = QUERY_NOTIFICATIONTONE_SETTINGS_SQL + " AND " +
            RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = " + to_string(sourceType);
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        querySql = QUERY_RINGTONE_SETTINGS_SQL + " AND " + RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + " = " +
            to_string(sourceType);
    } else {
        RINGTONE_ERR_LOG("setting type is not existing");
        return E_INVALID_ARGUMENTS;
    }

    TravelQueryResultSet(querySql, [&](shared_ptr<RingtoneMetadata> &meta) -> bool {
        int32_t ret = true;
        if ((settingType == TONE_SETTING_TYPE_SHOT) && (toneType != SHOT_TONE_TYPE_SIM_CARD_BOTH) &&
            (meta->GetShotToneType() == SHOT_TONE_TYPE_SIM_CARD_BOTH)) {
            int32_t cleanType = (toneType == SHOT_TONE_TYPE_SIM_CARD_1 ? SHOT_TONE_TYPE_SIM_CARD_2 :
                SHOT_TONE_TYPE_SIM_CARD_1);
            UpdateSettingsWithToneId(settingType, meta->GetToneId(), cleanType);
        } else if ((settingType == TONE_SETTING_TYPE_RINGTONE) && (toneType != RING_TONE_TYPE_SIM_CARD_BOTH) &&
            (meta->GetRingToneType() == RING_TONE_TYPE_SIM_CARD_BOTH)) {
            int32_t cleanType = (toneType == RING_TONE_TYPE_SIM_CARD_1 ? RING_TONE_TYPE_SIM_CARD_2 :
                RING_TONE_TYPE_SIM_CARD_1);
            UpdateSettingsWithToneId(settingType, meta->GetToneId(), cleanType);
        } else {
            ret = false;
        }

        return ret;
    });

    return CleanupSettingFromRdb(settingType, toneType, sourceType);
}

int32_t RingtoneSettingManager::TravelQueryResultSet(const string &querySql,
    function<bool (shared_ptr<RingtoneMetadata> &)> func)
{
    auto resultSet = ringtoneRdb_->QuerySql(querySql);
    if (resultSet == nullptr) {
        RINGTONE_INFO_LOG("result is null");
        return E_OK;
    }
    vector<shared_ptr<RingtoneMetadata>> metaDatas = {};
    int32_t ret = GetMetaDataFromResultSet(resultSet, metaDatas);
    if (ret != E_OK) {
        RINGTONE_INFO_LOG("read resultSet error");
        resultSet->Close();
        return E_DB_FAIL;
    }
    for (auto meta : metaDatas) {
        if (!func || func(meta) == true) {
            break;
        }
    }

    resultSet->Close();
    return E_OK;
}

int32_t RingtoneSettingManager::Update(int &changedRows, const NativeRdb::ValuesBucket &values,
    const NativeRdb::AbsRdbPredicates &predicates)
{
    if (ringtoneRdb_ == nullptr) {
        RINGTONE_ERR_LOG("ringtone rdb_ is nullptr");
        return E_DB_FAIL;
    }
    return ringtoneRdb_->Update(changedRows, values, predicates);
}

} // namespace Media
} // namespace OHOS
