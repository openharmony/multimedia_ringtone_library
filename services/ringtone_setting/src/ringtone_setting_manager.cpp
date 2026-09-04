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
    // sourceType只接受PRESET(1)和CUSTOMISED(2)，INVALID(-1)等值会被拒绝
    if ((sourceType != SOURCE_TYPE_PRESET) && (sourceType != SOURCE_TYPE_CUSTOMISED)) {
        return E_INVALID_ARGUMENTS;
    }
    if ((settingType < TONE_SETTING_TYPE_ALARM) || (settingType >= TONE_SETTING_TYPE_MAX)) {
        return E_INVALID_ARGUMENTS;
    }
    // 通知和闹钟的toneType是固定值（非卡位图），必须匹配
    if (((settingType == TONE_SETTING_TYPE_NOTIFICATION) && (toneType != NOTIFICATION_TONE_TYPE)) ||
        ((settingType == TONE_SETTING_TYPE_ALARM) && (toneType != ALARM_TONE_TYPE))) {
        return E_INVALID_ARGUMENTS;
    }
    // 短信和来电的toneType是卡位图，必须至少有一个卡位被设置
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        if (toneType <= SHOT_TONE_TYPE_NOT || !HasAnyCardSet(toneType)) {
            return E_INVALID_ARGUMENTS;
        }
    }
    if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        if (toneType <= RING_TONE_TYPE_NOT || !HasAnyCardSet(toneType)) {
            return E_INVALID_ARGUMENTS;
        }
    }
    return E_OK;
}

int32_t RingtoneSettingManager::TryMergeExistingSetting(const string &tonePath, int32_t settingType,
    int32_t toneType, int32_t sourceType)
{
    for (auto it = settings_.find(tonePath); it != settings_.end(); it++) {
        // 完全重复：settingType + toneType + sourceType 三者均相同
        if ((settingType == it->second.settingType) && (toneType == it->second.toneType) &&
            (sourceType == it->second.sourceType)) {
            RINGTONE_INFO_LOG("warning: this setting is existing, tone file:%{public}s", tonePath.c_str());
            return E_FAIL;
        } else if ((settingType == it->second.settingType) && (sourceType == it->second.sourceType)) {
            // 同设置类型同来源，但卡位不同：合并卡位图
            if (settingType == TONE_SETTING_TYPE_SHOT || settingType == TONE_SETTING_TYPE_RINGTONE) {
                return MergeCardToneType(it->second, toneType);
            }
        }
    }
    return E_ERR;
}

int32_t RingtoneSettingManager::MergeCardToneType(SettingItem &item, int32_t toneType)
{
    // 检查新卡位是否已存在于已有项中
    if (static_cast<uint32_t>(item.toneType) & static_cast<uint32_t>(toneType)) {
        RINGTONE_INFO_LOG("card already merged, toneType=%{public}d", item.toneType);
        return E_OK;
    }
    // 位或运算合并卡位图，如 0b0001 | 0b0010 = 0b0011
    item.toneType = SetCardMask(item.toneType, toneType);
    RINGTONE_INFO_LOG("merged card mask=%{public}d", item.toneType);
    return E_OK;
}

int32_t RingtoneSettingManager::CommitSetting(int32_t toneId, string &tonePath, int32_t settingType, int32_t toneType,
    int32_t sourceType)
{
    RINGTONE_INFO_LOG("toneId=%{public}d, tonePath=%{public}s, settingType=%{public}d, toneType=%{public}d,"
        "sourceType=%{public}d", toneId, tonePath.c_str(), settingType, toneType, sourceType);
    // 步骤1: 参数合法性校验
    auto ret = CommitSettingCompare(settingType, toneType, sourceType);
    if (ret != E_OK) {
        return ret;
    }
    // 步骤2: 尝试合并到已有的同路径缓存项（如合并卡位图）
    ret = TryMergeExistingSetting(tonePath, settingType, toneType, sourceType);
    if (ret != E_ERR) {
        return ret;
    }
    // 步骤3: 无可合并项，新增缓存项
    SettingItem item = {toneId, settingType, toneType, sourceType};
    settings_.emplace(tonePath, item);
    return E_OK;
}

void RingtoneSettingManager::TravelSettings(function<int32_t (string &, SettingItem &)> func)
{
    for (auto it = settings_.cbegin(); it != settings_.cend(); ++it) {
        string first = it->first;
        SettingItem item = it->second;
        func(first, item);
    }
}

void RingtoneSettingManager::SetForceFlush(bool forceFlush)
{
    RINGTONE_INFO_LOG("flag = %{public}d", forceFlush ? 1 : 0);
    forceFlush_ = forceFlush;
}

void RingtoneSettingManager::FlushSettings()
{
    TravelSettings([this](string &tonePath, SettingItem &item) -> int32_t {
        int32_t ret = CleanupSetting(item.settingType, item.toneType, item.sourceType);
        if (ret != E_OK) {
            RINGTONE_ERR_LOG("error: cleanup settings failed, tonePath=%{public}s", tonePath.c_str());
        }
        return ret;
    });

    TravelSettings([this](string &tonePath, SettingItem &item) -> int32_t {
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
        RINGTONE_ERR_LOG("column name invalid %{public}s", col.c_str());
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

int32_t RingtoneSettingManager::UpdateCardToneSetting(const std::string &toneTypeColumn,
    const std::string &sourceTypeColumn, int32_t currentVal, int32_t notValue, int32_t toneType,
    int32_t sourceType, int32_t toneId)
{
    int32_t val = (currentVal == notValue) ? toneType : SetCardMask(currentVal, toneType);
    string updateSql = "UPDATE ToneFiles SET " +
        toneTypeColumn + " = " + to_string(val) + ", " +
        sourceTypeColumn + " = " + to_string(sourceType) +
        " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);

    if (!forceFlush_) {
        updateSql += " AND " + sourceTypeColumn + " NOT IN (1, 2)";
    }

    int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
    if (rdbRet < 0) {
        RINGTONE_ERR_LOG("execute update failed");
        return E_DB_FAIL;
    }
    return E_OK;
}

int32_t RingtoneSettingManager::UpdateShotSetting(shared_ptr<RingtoneMetadata> &meta, int32_t toneType,
    int32_t sourceType)
{
    return UpdateCardToneSetting(RINGTONE_COLUMN_SHOT_TONE_TYPE, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE,
        meta->GetShotToneType(), SHOT_TONE_TYPE_NOT, toneType, sourceType, meta->GetToneId());
}

int32_t RingtoneSettingManager::UpdateRingtoneSetting(shared_ptr<RingtoneMetadata> &meta, int32_t toneType,
    int32_t sourceType)
{
    return UpdateCardToneSetting(RINGTONE_COLUMN_RING_TONE_TYPE, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE,
        meta->GetRingToneType(), RING_TONE_TYPE_NOT, toneType, sourceType, meta->GetToneId());
}

int32_t RingtoneSettingManager::UpdateNotificationSetting(shared_ptr<RingtoneMetadata> &meta, int32_t toneType,
    int32_t sourceType)
{
    string updateSql = "UPDATE ToneFiles SET " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " = " + to_string(toneType) + ", " +
        RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = " + to_string(sourceType) +
        " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(meta->GetToneId());

    if (!forceFlush_) {
        updateSql += " AND " + RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " NOT IN (1, 2)";
    }
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

    if (!forceFlush_) {
        updateSql += " AND " + RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " NOT IN (1, 2)";
    }
        
    int32_t rdbRet = ringtoneRdb_->ExecuteSql(updateSql);
    if (rdbRet < 0) {
        RINGTONE_ERR_LOG("execute update failed");
        return E_DB_FAIL;
    }

    return E_OK;
}

int32_t RingtoneSettingManager::UpdateSettingsByPath(string &tonePath, int32_t settingType, int32_t toneType,
    int32_t sourceType)
{
    string querySql = QUERY_SETTINGS_BY_PATH + "\"" + tonePath + "\"";
    auto ret = TravelQueryResultSet(querySql, [&](shared_ptr<RingtoneMetadata> &meta) -> bool {
        string updateSql = {};
        if (settingType == TONE_SETTING_TYPE_SHOT) {
            // update shot-tone settings
            if (UpdateShotSetting(meta, toneType, sourceType) != E_OK) {
                return false;
            }
        } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
            // update ring-tone settings
            if (UpdateRingtoneSetting(meta, toneType, sourceType) != E_OK) {
                return false;
            }
        } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
            // update notification-tone settings
            if (UpdateNotificationSetting(meta, toneType, sourceType) != E_OK) {
                return false;
            }
        } else if (settingType == TONE_SETTING_TYPE_ALARM) {
            // update alarm-tone settings
            if (UpdateAlarmSetting(meta, toneType, sourceType) != E_OK) {
                return false;
            }
        } else {
            RINGTONE_INFO_LOG("invalid tone-setting-type");
            return false;
        }
        return true;
    });

    return ret;
}

int32_t RingtoneSettingManager::UpdateSettingsWithToneId(int32_t settingType, int32_t toneId, int32_t toneType)
{
    int32_t ret = E_OK;

    string updateSql = {};
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        // update shot-tone settings
        updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " +
            to_string(toneType) + " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        // update ring-tone settings
        updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_RING_TONE_TYPE + " = " +
            to_string(toneType) + " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
    } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        // update notification-tone settings
        updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " = " +
            to_string(toneType) + " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
    } else if (settingType == TONE_SETTING_TYPE_ALARM) {
        // update alarm-tone settings
        updateSql = "UPDATE ToneFiles SET " + RINGTONE_COLUMN_ALARM_TONE_TYPE + " = " +
            to_string(toneType) + " WHERE " + RINGTONE_COLUMN_TONE_ID + " = " + to_string(toneId);
    } else {
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
        if (GetSimCardCount(toneType) > 1) {
            updateSql = SHOT_SETTING_CLEANUP_CLAUSE + " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " <> " +
                to_string(SHOT_TONE_TYPE_DEFAULT) + " AND " + RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = " +
                to_string(sourceType);
        } else {
            updateSql = SHOT_SETTING_CLEANUP_CLAUSE + " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " +
                to_string(toneType) + " AND " + RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = " + to_string(sourceType);
        }
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        if (GetSimCardCount(toneType) > 1) {
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
        if ((settingType == TONE_SETTING_TYPE_SHOT) && HasAnyCardSet(toneType) &&
            (static_cast<uint32_t>(meta->GetShotToneType()) & static_cast<uint32_t>(toneType)) &&
            (GetSimCardCount(meta->GetShotToneType()) > 1)) {
            int32_t cleanType = ClearCardMask(meta->GetShotToneType(), toneType);
            UpdateSettingsWithToneId(settingType, meta->GetToneId(), cleanType);
        } else if ((settingType == TONE_SETTING_TYPE_RINGTONE) && HasAnyCardSet(toneType) &&
            (static_cast<uint32_t>(meta->GetRingToneType()) & static_cast<uint32_t>(toneType)) &&
            (GetSimCardCount(meta->GetRingToneType()) > 1)) {
            int32_t cleanType = ClearCardMask(meta->GetRingToneType(), toneType);
            UpdateSettingsWithToneId(settingType, meta->GetToneId(), cleanType);
        } else {
            ret = false;
        }

        return ret;
    });

    return CleanupSettingFromRdb(settingType, toneType, sourceType);
}

int32_t RingtoneSettingManager::TravelQueryResultSet(string querySql,
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
        if (func(meta) == true) {
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
