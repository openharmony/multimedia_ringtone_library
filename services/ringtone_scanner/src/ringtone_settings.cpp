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
#define MLOG_TAG "RingtoneSettings"

#include "ringtone_settings.h"

#include "parameter.h"
#include "rdb_errno.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_type.h"
#include "ringtone_scanner_db.h"

namespace OHOS {
namespace Media {
using namespace std;
static const char PARAM_RINGTONE_SETTING_SHOT[] = "persist.ringtone.setting.shot";
static const char PARAM_RINGTONE_SETTING_SHOT2[] = "persist.ringtone.setting.shot2";
static const char PARAM_RINGTONE_SETTING_NOTIFICATIONTONE[] = "persist.ringtone.setting.notification";
static const char PARAM_RINGTONE_SETTING_RINGTONE[] = "persist.ringtone.setting.ringtone";
static const char PARAM_RINGTONE_SETTING_RINGTONE2[] = "persist.ringtone.setting.ringtone2";
static const int32_t SYSPARA_SIZE = 128;

static const vector<string> setingColumns = {
    RINGTONE_COLUMN_TONE_ID, RINGTONE_COLUMN_DISPLAY_NAME, RINGTONE_COLUMN_SHOT_TONE_TYPE,
    RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE,
    RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, RINGTONE_COLUMN_RING_TONE_TYPE,
    RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, RINGTONE_COLUMN_ALARM_TONE_TYPE, RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE
};

int32_t RingtoneSettings::GetMetaDataFromResultSet(shared_ptr<NativeRdb::ResultSet> resultSet,
    vector<shared_ptr<RingtoneMetadata>> &metas)
{
    if (resultSet == nullptr) {
        return E_INVALID_ARGUMENTS;
    }
    auto ret = resultSet->GoToFirstRow();
    while (ret == NativeRdb::E_OK) {
        auto metaData = make_unique<RingtoneMetadata>();
        if (RingtoneScannerDb::FillMetadata(resultSet, metaData) != E_OK) {
            resultSet->Close();
            return E_DB_FAIL;
        }
        metas.push_back(std::move(metaData));
        ret = resultSet->GoToNextRow();
    };

    resultSet->Close();
    return E_OK;
}

void RingtoneSettings::QuerySettingsFromRdb(int32_t settingType, shared_ptr<NativeRdb::ResultSet> resultSet,
    shared_ptr<NativeRdb::ResultSet> resultSet2, shared_ptr<NativeRdb::ResultSet> resultSetBoth)
{
    string whereClause = {};
    vector<string> whereArgs = {};
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        whereClause = RINGTONE_COLUMN_SHOT_TONE_TYPE + " = ?";
        whereArgs = {to_string(SHOT_TONE_TYPE_SIM_CARD_1)};
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        whereClause = RINGTONE_COLUMN_RING_TONE_TYPE + " = ?";
        whereArgs = {to_string(RING_TONE_TYPE_SIM_CARD_1)};
    } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        whereClause = RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " = ?";
        whereArgs = {to_string(NOTIFICATION_TONE_TYPE)};
    } else {
        return;
    }
    if (RingtoneScannerDb::QueryRingtoneRdb(whereClause, whereArgs, setingColumns, resultSet) != E_OK) {
        RINGTONE_INFO_LOG("QueryRingtoneRdb failed");
        resultSet = nullptr;
    }
    if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        return;
    }
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        whereClause = RINGTONE_COLUMN_SHOT_TONE_TYPE + " = ?";
        whereArgs = {to_string(SHOT_TONE_TYPE_SIM_CARD_2)};
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        whereClause = RINGTONE_COLUMN_RING_TONE_TYPE + " = ?";
        whereArgs = {to_string(RING_TONE_TYPE_SIM_CARD_2)};
    } else {
        return;
    }
    if (RingtoneScannerDb::QueryRingtoneRdb(whereClause, whereArgs, setingColumns, resultSet2) != E_OK) {
        RINGTONE_INFO_LOG("QueryRingtoneRdb failed");
        resultSet2 = nullptr;
    }
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        whereClause = RINGTONE_COLUMN_SHOT_TONE_TYPE + " = ?";
        whereArgs = {to_string(SHOT_TONE_TYPE_SIM_CARD_BOTH)};
    } else {
        whereClause = RINGTONE_COLUMN_RING_TONE_TYPE + " = ?";
        whereArgs = {to_string(RING_TONE_TYPE_SIM_CARD_BOTH)};
    }
    if (RingtoneScannerDb::QueryRingtoneRdb(whereClause, whereArgs, setingColumns, resultSetBoth) != E_OK) {
        RINGTONE_INFO_LOG("QueryRingtoneRdb failed");
        resultSetBoth = nullptr;
    }
}

int32_t RingtoneSettings::UpdateSettingsToRdb(string whereClause, vector<string> &whereArgs,
    const string &sourceTypeColumn, int32_t sourceVal, const string &toneTypeColumn, int32_t toneVal)
{
    NativeRdb::ValuesBucket values = {};

    values.PutInt(sourceTypeColumn, sourceVal);
    values.PutInt(toneTypeColumn, toneVal);

    int32_t ret = RingtoneScannerDb::UpdateRingtoneRdb(values, whereClause, whereArgs);
    if (ret < 0) {
        RINGTONE_INFO_LOG("set ringtone default tone-type failed");
    }
    return ret;
}

int32_t RingtoneSettings::SetDefaultSettingsWithDisplayName(int32_t settingType, string &displayName, int32_t toneType,
    int32_t sourceType)
{
    int32_t ret = 0;
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        string whereClause = RINGTONE_COLUMN_DISPLAY_NAME + " = ? AND " + RINGTONE_COLUMN_TONE_TYPE + " = ?";
        vector<string> whereArgs = {displayName, to_string(TONE_TYPE_NOTIFICATION)};

        ret = UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, sourceType,
            RINGTONE_COLUMN_SHOT_TONE_TYPE, toneType);
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        string whereClause = RINGTONE_COLUMN_DISPLAY_NAME + " = ? AND " + RINGTONE_COLUMN_TONE_TYPE + " = ?";
        vector<string> whereArgs = {displayName, to_string(TONE_TYPE_RINGTONE)};

        ret = UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, sourceType,
            RINGTONE_COLUMN_RING_TONE_TYPE, toneType);
    } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        string whereClause = RINGTONE_COLUMN_DISPLAY_NAME + " = ? AND " + RINGTONE_COLUMN_TONE_TYPE + " = ?";
        vector<string> whereArgs = {displayName, to_string(TONE_TYPE_NOTIFICATION)};

        ret = UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, sourceType,
            RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, toneType);
    } else if (settingType == TONE_SETTING_TYPE_ALARM) {
        string whereClause = RINGTONE_COLUMN_DISPLAY_NAME + " = ? AND " + RINGTONE_COLUMN_TONE_TYPE + " = ?";
        vector<string> whereArgs = {displayName, to_string(TONE_TYPE_ALARM)};

        ret = UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, sourceType,
            RINGTONE_COLUMN_ALARM_TONE_TYPE, toneType);
    } else {
        RINGTONE_INFO_LOG("invalid tone-setting-type");
        ret = E_INVALID_ARGUMENTS;
    }
    return ret;
}

int32_t RingtoneSettings::SetDefaultSettingsWithToneId(int32_t settingType, int32_t toneId, int32_t toneType,
    int32_t sourceType)
{
    int32_t ret = 0;
    if (settingType == TONE_SETTING_TYPE_SHOT) {
        string whereClause = RINGTONE_COLUMN_TONE_ID + " = ?";
        vector<string> whereArgs = {to_string(toneId)};

        ret = UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, sourceType,
            RINGTONE_COLUMN_SHOT_TONE_TYPE, toneType);
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        string whereClause = RINGTONE_COLUMN_TONE_ID + " = ?";
        vector<string> whereArgs = {to_string(toneId)};

        ret = UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, sourceType,
            RINGTONE_COLUMN_RING_TONE_TYPE, toneType);
    } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        string whereClause = RINGTONE_COLUMN_TONE_ID + " = ?";
        vector<string> whereArgs = {to_string(toneId)};

        ret = UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, sourceType,
            RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, toneType);
    } else if (settingType == TONE_SETTING_TYPE_ALARM) {
        string whereClause = RINGTONE_COLUMN_TONE_ID + " = ?";
        vector<string> whereArgs = {to_string(toneId)};

        ret = UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, sourceType,
            RINGTONE_COLUMN_ALARM_TONE_TYPE, toneType);
    } else {
        RINGTONE_INFO_LOG("invalid tone-setting-type");
        ret = E_INVALID_ARGUMENTS;
    }
    return ret;
}

int32_t RingtoneSettings::CleanDefaultSettingRow(int32_t settingType, int32_t toneType, int32_t sourceType)
{
    int32_t ret = E_OK;

    if (settingType == TONE_SETTING_TYPE_SHOT) {
        // cleanup shot-tone default settings
        string whereClause = RINGTONE_COLUMN_SHOT_TONE_TYPE + " =? AND " +
            RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = ?";
        vector<string> whereArgs = {to_string(toneType), to_string(sourceType)};

        UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, SOURCE_TYPE_CUSTOMISED,
            RINGTONE_COLUMN_SHOT_TONE_TYPE, SHOT_TONE_TYPE_NOT);
    } else if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        // cleanup ring-tone default settings
        string whereClause = RINGTONE_COLUMN_RING_TONE_TYPE + " =? AND " +
            RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + " = ?";
        vector<string> whereArgs = {to_string(toneType), to_string(sourceType)};

        UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, SOURCE_TYPE_CUSTOMISED,
            RINGTONE_COLUMN_RING_TONE_TYPE, RING_TONE_TYPE_NOT);
    } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        // cleanup notification-tone default settings
        string whereClause = RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE + " =? AND " +
            RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " = ?";
        vector<string> whereArgs = {to_string(toneType), to_string(sourceType)};

        UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE,
            SOURCE_TYPE_CUSTOMISED, RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, NOTIFICATION_TONE_TYPE_NOT);
    } else if (settingType == TONE_SETTING_TYPE_ALARM) {
        // cleanup alarm-tone default settings
        string whereClause = RINGTONE_COLUMN_ALARM_TONE_TYPE + " =? AND " +
            RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE + " = ?";
        vector<string> whereArgs = {to_string(toneType), to_string(sourceType)};

        UpdateSettingsToRdb(whereClause, whereArgs, RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, SOURCE_TYPE_CUSTOMISED,
            RINGTONE_COLUMN_ALARM_TONE_TYPE, ALARM_TONE_TYPE_NOT);
    } else {
        RINGTONE_INFO_LOG("invalid tone-setting-type");
        ret = E_INVALID_ARGUMENTS;
    }

    return ret;
}

int32_t RingtoneSettings::GetResultSetSize(std::shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    int32_t rowCount = 0;
    if (resultSet == nullptr) {
        return rowCount;
    }
    int32_t ret = resultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get row count from resultset");
        rowCount = 0;
    }

    return rowCount;
}

void RingtoneSettings::CleanDefaultSettings(int32_t settingType, shared_ptr<NativeRdb::ResultSet> resultSet,
    shared_ptr<NativeRdb::ResultSet> resultSet2, shared_ptr<NativeRdb::ResultSet> resultSetBoth, string &nameCard1,
    string &nameCard2)
{
    int32_t toneType1 = 0;
    int32_t toneType2 = 0;
    int32_t toneTypeBoth = 0;
    int32_t rowCount = GetResultSetSize(resultSet);
    int32_t rowCount2 = GetResultSetSize(resultSet2);
    int32_t rowCountBoth = GetResultSetSize(resultSetBoth);
    if (settingType == TONE_SETTING_TYPE_RINGTONE) {
        toneType1 = RING_TONE_TYPE_SIM_CARD_1;
        toneType2 = RING_TONE_TYPE_SIM_CARD_2;
        toneTypeBoth = RING_TONE_TYPE_SIM_CARD_BOTH;
    } else if (settingType == TONE_SETTING_TYPE_SHOT) {
        toneType1 = SHOT_TONE_TYPE_SIM_CARD_1;
        toneType2 = SHOT_TONE_TYPE_SIM_CARD_2;
        toneTypeBoth = SHOT_TONE_TYPE_SIM_CARD_BOTH;
    } else if (settingType == TONE_SETTING_TYPE_NOTIFICATION) {
        toneType1 = NOTIFICATION_TONE_TYPE;
    } else if (settingType == TONE_SETTING_TYPE_ALARM) {
        toneType1 = ALARM_TONE_TYPE;
    } else {
        RINGTONE_INFO_LOG("tone type arguement error");
    }
    if (rowCount > 0 && nameCard1 != "") {
        CleanDefaultSettingRow(settingType, toneType1, SOURCE_TYPE_PRESET);
    }
    if (rowCount2 > 0 && nameCard2 != "") {
        CleanDefaultSettingRow(settingType, toneType2, SOURCE_TYPE_PRESET);
    }
    if (rowCountBoth > 0) {
        vector<shared_ptr<RingtoneMetadata>> metas = {};
        GetMetaDataFromResultSet(resultSetBoth, metas);
        for (auto meta : metas) {
            if (meta->GetRingToneSourceType() != SOURCE_TYPE_PRESET) {
                continue;
            }
            if (nameCard1 != "" && nameCard2 != "") {
                CleanDefaultSettingRow(settingType, toneTypeBoth, SOURCE_TYPE_PRESET);
            } else if (nameCard1 != "") {
                SetDefaultSettingsWithToneId(settingType, meta->GetToneId(), toneType2, SOURCE_TYPE_PRESET);
            } else if (nameCard2 != "") {
                SetDefaultSettingsWithToneId(settingType, meta->GetToneId(), toneType1, SOURCE_TYPE_PRESET);
            } else {
                RINGTONE_INFO_LOG("tone type arguement error");
            }
        }
    }
}

void RingtoneSettings::ShotToneDefaultSettings()
{
    char paramValue1[SYSPARA_SIZE] = {0};
    char paramValue2[SYSPARA_SIZE] = {0};

    GetParameter(PARAM_RINGTONE_SETTING_SHOT, "", paramValue1, SYSPARA_SIZE);
    GetParameter(PARAM_RINGTONE_SETTING_SHOT2, "", paramValue2, SYSPARA_SIZE);

    string nameCard1(paramValue1);
    string nameCard2(paramValue2);

    if ((nameCard1 == "") && (nameCard2 == "")) {
        RINGTONE_INFO_LOG("shot param settings is null");
        return;
    }

    ToneSettingType settingType = TONE_SETTING_TYPE_SHOT;
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    shared_ptr<NativeRdb::ResultSet> resultSet2 = nullptr;
    shared_ptr<NativeRdb::ResultSet> resultSetBoth = nullptr;

    QuerySettingsFromRdb(settingType, resultSet, resultSet2, resultSetBoth);

    CleanDefaultSettings(settingType, resultSet, resultSet2, resultSetBoth, nameCard1, nameCard2);

    if (nameCard1 == nameCard2 && nameCard1 != "") {
        SetDefaultSettingsWithDisplayName(settingType, nameCard1, SHOT_TONE_TYPE_SIM_CARD_BOTH, SOURCE_TYPE_PRESET);
    }
    if (nameCard1 != "") {
        SetDefaultSettingsWithDisplayName(settingType, nameCard1, SHOT_TONE_TYPE_SIM_CARD_1, SOURCE_TYPE_PRESET);
    }
    if (nameCard2 != "") {
        SetDefaultSettingsWithDisplayName(settingType, nameCard2, SHOT_TONE_TYPE_SIM_CARD_2, SOURCE_TYPE_PRESET);
    }
}

void RingtoneSettings::NotificationToneDefaultSettings()
{
    char paramValue[SYSPARA_SIZE] = {0};
    GetParameter(PARAM_RINGTONE_SETTING_NOTIFICATIONTONE, "", paramValue, SYSPARA_SIZE);

    string displayName(paramValue);
    if (displayName == "") {
        RINGTONE_INFO_LOG("notification param settings is null");
        return;
    }

    ToneSettingType settingType = TONE_SETTING_TYPE_NOTIFICATION;
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    QuerySettingsFromRdb(settingType, resultSet, nullptr, nullptr);

    CleanDefaultSettingRow(TONE_SETTING_TYPE_NOTIFICATION, NOTIFICATION_TONE_TYPE, SOURCE_TYPE_PRESET);

    SetDefaultSettingsWithDisplayName(TONE_SETTING_TYPE_NOTIFICATION, displayName, NOTIFICATION_TONE_TYPE,
        SOURCE_TYPE_PRESET);
}

void RingtoneSettings::RingToneDefaultSettings()
{
    char paramValue1[SYSPARA_SIZE] = {0};
    char paramValue2[SYSPARA_SIZE] = {0};

    GetParameter(PARAM_RINGTONE_SETTING_RINGTONE, "", paramValue1, SYSPARA_SIZE);
    GetParameter(PARAM_RINGTONE_SETTING_RINGTONE2, "", paramValue2, SYSPARA_SIZE);

    string nameCard1(paramValue1);
    string nameCard2(paramValue2);

    if ((nameCard1 == "") && (nameCard2 == "")) {
        RINGTONE_INFO_LOG("ringtone param settings is null");
        return;
    }

    ToneSettingType settingType = TONE_SETTING_TYPE_RINGTONE;
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    shared_ptr<NativeRdb::ResultSet> resultSet2 = nullptr;
    shared_ptr<NativeRdb::ResultSet> resultSetBoth = nullptr;

    QuerySettingsFromRdb(settingType, resultSet, resultSet2, resultSetBoth);

    CleanDefaultSettings(settingType, resultSet, resultSet2, resultSetBoth, nameCard1, nameCard2);

    if (nameCard1 == nameCard2 && nameCard1 != "") {
        SetDefaultSettingsWithDisplayName(settingType, nameCard1, RING_TONE_TYPE_SIM_CARD_BOTH, SOURCE_TYPE_PRESET);
    }
    if (nameCard1 != "") {
        SetDefaultSettingsWithDisplayName(settingType, nameCard1, RING_TONE_TYPE_SIM_CARD_1, SOURCE_TYPE_PRESET);
    }
    if (nameCard2 != "") {
        SetDefaultSettingsWithDisplayName(settingType, nameCard2, RING_TONE_TYPE_SIM_CARD_2, SOURCE_TYPE_PRESET);
    }
}
} // namespace Media
} // namespace OHOS
