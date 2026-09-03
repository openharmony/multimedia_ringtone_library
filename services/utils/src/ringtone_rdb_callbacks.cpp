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

#include "ringtone_rdb_callbacks.h"

#include <sys/stat.h>

#include "rdb_sql_utils.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_db_const.h"
#include "ringtone_file_utils.h"
#include "ringtone_mimetype_utils.h"
#include "ringtone_type.h"
#include "ringtone_utils.h"
#include "result_set_utils.h"
#include "preferences_helper.h"
#include "dfx_const.h"
#ifdef CORE_SERVICE_ENABLE
#include "core_service_client.h"
#include "telephony_errors.h"
#endif

namespace OHOS {
namespace Media {
using namespace std;

const string DEFAULT_MIME_TYPE = "application/octet-stream";
static const char RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY[] = "ringtone.scanner.completed";
static const int RINGTONE_PARAMETER_SCANNER_COMPLETED_FALSE = 0;

const int RINGTONE_PRELOAD_CONF_COUNT = 7;

const std::string CREATE_RINGTONE_TABLE = "CREATE TABLE IF NOT EXISTS " + RINGTONE_TABLE + "(" +
    RINGTONE_COLUMN_TONE_ID                       + " INTEGER  PRIMARY KEY AUTOINCREMENT, " +
    RINGTONE_COLUMN_DATA                          + " TEXT              , " +
    RINGTONE_COLUMN_SIZE                          + " BIGINT   DEFAULT 0, " +
    RINGTONE_COLUMN_DISPLAY_NAME                  + " TEXT              , " +
    RINGTONE_COLUMN_TITLE                         + " TEXT              , " +
    RINGTONE_COLUMN_MEDIA_TYPE                    + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_TONE_TYPE                     + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_MIME_TYPE                     + " TEXT              , " +
    RINGTONE_COLUMN_SOURCE_TYPE                   + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_DATE_ADDED                    + " BIGINT   DEFAULT 0, " +
    RINGTONE_COLUMN_DATE_MODIFIED                 + " BIGINT   DEFAULT 0, " +
    RINGTONE_COLUMN_DATE_TAKEN                    + " BIGINT   DEFAULT 0, " +
    RINGTONE_COLUMN_DURATION                      + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_SHOT_TONE_TYPE                + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE         + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE        + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_RING_TONE_TYPE                + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE         + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_ALARM_TONE_TYPE               + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE        + " INT      DEFAULT 0, " +
    RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE         + " TEXT              , " +
    RINGTONE_COLUMN_SCANNER_FLAG                  + " INT      DEFAULT 0  " + ")";

const std::string CREATE_PRELOAD_CONF_TABLE = "CREATE TABLE IF NOT EXISTS " + PRELOAD_CONFIG_TABLE + "(" +
    PRELOAD_CONFIG_COLUMN_RING_TONE_TYPE          + " INTEGER  PRIMARY KEY," +
    PRELOAD_CONFIG_COLUMN_TONE_ID                 + " INTEGER             ," +
    PRELOAD_CONFIG_COLUMN_DISPLAY_NAME            + " TEXT                 " + ")";

const std::string INIT_PRELOAD_CONF_TABLE = "INSERT OR IGNORE INTO " + PRELOAD_CONFIG_TABLE + " (" +
    PRELOAD_CONFIG_COLUMN_RING_TONE_TYPE + ") VALUES (1), (2), (3), (4), (5), (6), (7), (8), (9), (10);";

const std::string CREATE_SIMCARD_SETTING_TABLE = "CREATE TABLE IF NOT EXISTS " + SIMCARD_SETTING_TABLE + "(" +
    SIMCARD_SETTING_COLUMN_MODE                   + " INTEGER            ," +
    SIMCARD_SETTING_COLUMN_RINGTONE_TYPE          + " INTEGER            ," +
    SIMCARD_SETTING_COLUMN_TONE_FILE              + " TEXT               ," +
    SIMCARD_SETTING_COLUMN_VIBRATE_FILE           + " TEXT               ," +
    SIMCARD_SETTING_COLUMN_VIBRATE_MODE           + " INTEGER            ," +
    SIMCARD_SETTING_COLUMN_RING_MODE              + " INTEGER            ," +
    " PRIMARY KEY (" + SIMCARD_SETTING_COLUMN_MODE + ", " + SIMCARD_SETTING_COLUMN_RINGTONE_TYPE + "))";

const std::string INIT_SIMCARD_SETTING_TABLE = "INSERT OR IGNORE INTO " + SIMCARD_SETTING_TABLE + " (" +
    SIMCARD_SETTING_COLUMN_MODE                   + ", " +
    SIMCARD_SETTING_COLUMN_RINGTONE_TYPE          + ") VALUES " +
    // 响铃模式 (12条): mode 1/2/3, ringtone_type 0/1/2/3
    "(1, 0), (1, 1), (1, 2), (1, 3), " +
    "(2, 0), (2, 1), (2, 2), (2, 3), " +
    "(3, 0), (3, 1), (3, 2), (3, 3), " +
    // eSIM1/eSIM2 响铃模式 (8条新增): mode 4/5, ringtone_type 1/2/101/102
    "(4, 1), (4, 2), (4, 101), (4, 102), " +   // eSIM1: 短信, 来电, 振动-短信, 振动-来电
    "(5, 1), (5, 2), (5, 101), (5, 102), " +   // eSIM2: 短信, 来电, 振动-短信, 振动-来电
    // 振动模式 (6条): ringtone_type 100/101/102/103
    "(1, 101), (1, 102), " +   // 卡1: 振动-短信, 振动-来电
    "(2, 101), (2, 102), " +   // 卡2: 振动-短信, 振动-来电
    "(3, 100), (3, 103);"; // 非卡相关: 振动-闹钟, 振动-通知

const std::string CREATE_VIBRATE_TABLE = "CREATE TABLE IF NOT EXISTS " + VIBRATE_TABLE + "(" +
    VIBRATE_COLUMN_VIBRATE_ID                     + " INTEGER  PRIMARY KEY AUTOINCREMENT, " +
    VIBRATE_COLUMN_DATA                           + " TEXT              , " +
    VIBRATE_COLUMN_SIZE                           + " BIGINT   DEFAULT 0, " +
    VIBRATE_COLUMN_DISPLAY_NAME                   + " TEXT              , " +
    VIBRATE_COLUMN_TITLE                          + " TEXT              , " +
    VIBRATE_COLUMN_DISPLAY_LANGUAGE               + " TEXT              , " +
    VIBRATE_COLUMN_VIBRATE_TYPE                   + " INT      DEFAULT 0, " +
    VIBRATE_COLUMN_SOURCE_TYPE                    + " INT      DEFAULT 0, " +
    VIBRATE_COLUMN_DATE_ADDED                     + " BIGINT   DEFAULT 0, " +
    VIBRATE_COLUMN_DATE_MODIFIED                  + " BIGINT   DEFAULT 0, " +
    VIBRATE_COLUMN_DATE_TAKEN                     + " BIGINT   DEFAULT 0, " +
    VIBRATE_COLUMN_PLAY_MODE                      + " INT      DEFAULT 0, " +
    VIBRATE_COLUMN_SCANNER_FLAG                   + " INT      DEFAULT 0  " + ")";


const std::string CREATE_HAPTIC_2_TONE_TABLE = "CREATE TABLE IF NOT EXISTS " + HAPTIC_2_TONE_TABLE + "(" +
    HAPTIC_2_TONE_COLUMN_ID + " INTEGER  PRIMARY KEY AUTOINCREMENT, " +
    HAPTIC_2_TONE_COLUMN_DATA + " TEXT              , " +
    HAPTIC_2_TONE_COLUMN_SIZE + " BIGINT   DEFAULT 0, " +
    HAPTIC_2_TONE_COLUMN_DISPLAY_NAME + " TEXT              , " +
    HAPTIC_2_TONE_COLUMN_TITLE + " TEXT              , " +
    HAPTIC_2_TONE_COLUMN_HAPTIC_2_TONE_TYPE + " INT      DEFAULT 0, " +
    HAPTIC_2_TONE_COLUMN_SOURCE_TYPE + " INT      DEFAULT 0, " +
    HAPTIC_2_TONE_COLUMN_DATE_ADDED + " BIGINT   DEFAULT 0, " +
    HAPTIC_2_TONE_COLUMN_DATE_MODIFIED + " BIGINT   DEFAULT 0, " +
    HAPTIC_2_TONE_COLUMN_PLAY_MODE + " INT      DEFAULT 0, " +
    HAPTIC_2_TONE_COLUMN_SCANNER_FLAG + " INT      DEFAULT 0  " + ")";

static const vector<string> g_initSqls = {
    CREATE_RINGTONE_TABLE,
    CREATE_VIBRATE_TABLE,
    CREATE_SIMCARD_SETTING_TABLE,
    INIT_SIMCARD_SETTING_TABLE,
    CREATE_PRELOAD_CONF_TABLE,
    INIT_PRELOAD_CONF_TABLE,
    CREATE_HAPTIC_2_TONE_TABLE,
};

struct CardConfig {
    bool hasSim1 = false;
    bool hasSim2 = false;
    bool hasESim1 = false;
    bool hasESim2 = false;
};

RingtoneDataCallBack::RingtoneDataCallBack(void)
{
}

RingtoneDataCallBack::~RingtoneDataCallBack(void)
{
}

int32_t RingtoneDataCallBack::InitSql(NativeRdb::RdbStore &store)
{
    for (const string &sqlStr : g_initSqls) {
        if (store.ExecuteSql(sqlStr) != NativeRdb::E_OK) {
            RINGTONE_ERR_LOG("Failed to execute sql");
            return NativeRdb::E_ERROR;
        }
    }
    return NativeRdb::E_OK;
}

int32_t RingtoneDataCallBack::OnCreate(NativeRdb::RdbStore &store)
{
    if (InitSql(store) != NativeRdb::E_OK) {
        RINGTONE_DEBUG_LOG("Failed to init sql");
        return NativeRdb::E_ERROR;
    }

    RingtoneFileUtils::CreateRingtoneDir();
    return NativeRdb::E_OK;
}

static void ExecSqls(const vector<string> &sqls, NativeRdb::RdbStore &store)
{
    int32_t err = NativeRdb::E_OK;
    for (const auto &sql : sqls) {
        err = store.ExecuteSql(sql);
        if (err != NativeRdb::E_OK) {
            RINGTONE_ERR_LOG("Failed to exec: %{private}s", sql.c_str());
            continue;
        }
    }
}

static void AddDisplayLanguageColumn(NativeRdb::RdbStore &store)
{
    const vector<string> sqls = {
        "ALTER TABLE " + RINGTONE_TABLE + " ADD COLUMN " + RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE + " TEXT",
    };
    RINGTONE_INFO_LOG("Add display language column");
    ExecSqls(sqls, store);
}

static void AddScannerFlagColumn(NativeRdb::RdbStore &store)
{
    const vector<string> sqls = {
        "ALTER TABLE " + RINGTONE_TABLE + " ADD COLUMN " + RINGTONE_COLUMN_SCANNER_FLAG + " INT DEFAULT 0",
        "ALTER TABLE " + VIBRATE_TABLE + " ADD COLUMN " + VIBRATE_COLUMN_SCANNER_FLAG + " INT DEFAULT 0",
    };
    RINGTONE_INFO_LOG("Add scanner flag column");
    ExecSqls(sqls, store);
}

static void UpdateMimeType(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("Update MimeType Begin");
    RingtoneMimeTypeUtils::InitMimeTypeMap();
    const string sql = "SELECT * FROM " + RINGTONE_TABLE;
    auto resultSet = store.QuerySql(sql);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("error query sql %{public}s", sql.c_str());
        return;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        std::string mimeType = GetStringVal(RINGTONE_COLUMN_MIME_TYPE, resultSet);
        if (mimeType != DEFAULT_MIME_TYPE) {
            continue;
        }
        string displayName = GetStringVal(RINGTONE_COLUMN_DISPLAY_NAME, resultSet);
        int32_t toneid = GetInt32Val(RINGTONE_COLUMN_TONE_ID, resultSet);
        std::string extension = RingtoneFileUtils::GetFileExtension(displayName);
        mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(extension);
        int32_t mime = RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(mimeType);
        RINGTONE_INFO_LOG("extension: %{public}s, mimeType: %{public}s, toneid: %{public}d mime: %{public}d",
            extension.c_str(), mimeType.c_str(), toneid, mime);

        NativeRdb::ValuesBucket values;
        values.PutString(RINGTONE_COLUMN_MIME_TYPE, mimeType);
        values.PutInt(RINGTONE_COLUMN_MEDIA_TYPE, mime);
        NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
        absRdbPredicates.EqualTo(RINGTONE_COLUMN_TONE_ID, toneid);
        int32_t changedRows;
        int32_t result = store.Update(changedRows, values, absRdbPredicates);
        if (result != E_OK || changedRows <= 0) {
            RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
        }
    }
    resultSet->Close();
}

static void UpdateMediaType(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("Update MediaType Begin");
    RingtoneMimeTypeUtils::InitMimeTypeMap();
    const string sql = "SELECT * FROM " + RINGTONE_TABLE +  " WHERE " + RINGTONE_COLUMN_MEDIA_TYPE + " = 0";
    auto resultSet = store.QuerySql(sql);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("error query sql %{public}s", sql.c_str());
        return;
    }
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        string displayName = GetStringVal(RINGTONE_COLUMN_DISPLAY_NAME, resultSet);
        int32_t toneid = GetInt32Val(RINGTONE_COLUMN_TONE_ID, resultSet);
        string extension = RingtoneFileUtils::GetFileExtension(displayName);
        string mimeType = RingtoneMimeTypeUtils::GetMimeTypeFromExtension(extension);
        int32_t mediaType = RingtoneMimeTypeUtils::GetMediaTypeFromMimeType(mimeType);

        NativeRdb::ValuesBucket values;
        values.PutInt(RINGTONE_COLUMN_MEDIA_TYPE, mediaType);
        NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
        absRdbPredicates.EqualTo(RINGTONE_COLUMN_TONE_ID, toneid);
        int32_t changedRows;
        int32_t result = store.Update(changedRows, values, absRdbPredicates);
        if (result != E_OK || changedRows <= 0) {
            RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
        }
    }
    resultSet->Close();
    RINGTONE_INFO_LOG("Update MediaType End");
}

static void AddPreloadConfTable(NativeRdb::RdbStore &store)
{
    const vector<string> sqls = {
        CREATE_PRELOAD_CONF_TABLE,
        INIT_PRELOAD_CONF_TABLE
    };
    RINGTONE_INFO_LOG("Add preload config table");
    ExecSqls(sqls, store);
}

static void AddRingMockHapticAudioTable(NativeRdb::RdbStore &store)
{
    const vector<string> sqls = {
        CREATE_HAPTIC_2_TONE_TABLE,
    };
    RINGTONE_INFO_LOG("Add sim ringtone table");
    ExecSqls(sqls, store);
}

static void AddSoundModeVibrateRecords(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("Add sound mode vibrate records");
    // Step 1: 插入振动模式缺省记录
    const string insertSql =
        "INSERT OR IGNORE INTO " + SIMCARD_SETTING_TABLE + " (" +
        SIMCARD_SETTING_COLUMN_MODE + ", " +
        SIMCARD_SETTING_COLUMN_RINGTONE_TYPE + ") VALUES " +
        "(1, 101), (1, 102), " +
        "(2, 101), (2, 102), " +
        "(3, 100), (3, 103)";
    int32_t result = store.ExecuteSql(insertSql);
    CHECK_AND_RETURN_LOG(result == E_OK, "init SimCardSetting failed. Result %{public}d.", result);
}

static void UpdateDefaultSystemTone(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("setting system tone begin");
    auto infos = RingtoneUtils::GetDefaultSystemtoneInfo();
    for (auto info : infos) {
        const string querySql = "SELECT tone_id FROM ToneFiles WHERE display_name = "s + "\"" + info.second + "\"";
        auto resultSet = store.QuerySql(querySql);
        if (resultSet == nullptr || resultSet->GoToFirstRow() != NativeRdb::E_OK) {
            RINGTONE_ERR_LOG("Update operation failed. no resultSet");
            continue;
        }

        int32_t tone_id = GetInt32Val("tone_id", resultSet);
        NativeRdb::ValuesBucket values;
        values.PutString(PRELOAD_CONFIG_COLUMN_DISPLAY_NAME, info.second);
        values.PutInt(PRELOAD_CONFIG_COLUMN_TONE_ID, tone_id);
        NativeRdb::AbsRdbPredicates absRdbPredicates(PRELOAD_CONFIG_TABLE);
        absRdbPredicates.EqualTo(PRELOAD_CONFIG_COLUMN_RING_TONE_TYPE, std::to_string(info.first));
        int32_t changedRows = 0;
        int32_t result = store.Update(changedRows, values, absRdbPredicates);
        if (result != E_OK || changedRows <= 0) {
            RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
        }
    }
}

static void AddVibrateTable(NativeRdb::RdbStore &store)
{
    const vector<string> sqls = {
        CREATE_VIBRATE_TABLE,
        CREATE_SIMCARD_SETTING_TABLE,
        INIT_SIMCARD_SETTING_TABLE,
    };
    int32_t errCode;
    shared_ptr<NativePreferences::Preferences> prefs =
        NativePreferences::PreferencesHelper::GetPreferences(COMMON_XML_EL1, errCode);
    if (!prefs) {
        RINGTONE_ERR_LOG("AddVibrateTable:  update faild errCode=%{public}d", errCode);
    } else {
        prefs->PutInt(RINGTONE_PARAMETER_SCANNER_COMPLETED_KEY, RINGTONE_PARAMETER_SCANNER_COMPLETED_FALSE);
        prefs->FlushSync();
    }

    RINGTONE_INFO_LOG("Add vibrate table");
    ExecSqls(sqls, store);
}

static void MigrateESimSimCardSetting(NativeRdb::RdbStore &store, int32_t fromMode, int32_t toMode)
{
    RINGTONE_INFO_LOG("MigrateESimSimCardSetting: fromMode=%{public}d, toMode=%{public}d", fromMode, toMode);
    const string migrateSql =
        "INSERT OR REPLACE INTO " + SIMCARD_SETTING_TABLE + " (" +
        SIMCARD_SETTING_COLUMN_MODE + ", " +
        SIMCARD_SETTING_COLUMN_RINGTONE_TYPE + ", " +
        SIMCARD_SETTING_COLUMN_TONE_FILE + ", " +
        SIMCARD_SETTING_COLUMN_VIBRATE_FILE + ", " +
        SIMCARD_SETTING_COLUMN_VIBRATE_MODE + ", " +
        SIMCARD_SETTING_COLUMN_RING_MODE + ") " +
        "SELECT ?, " +
        SIMCARD_SETTING_COLUMN_RINGTONE_TYPE + ", " +
        SIMCARD_SETTING_COLUMN_TONE_FILE + ", " +
        SIMCARD_SETTING_COLUMN_VIBRATE_FILE + ", " +
        SIMCARD_SETTING_COLUMN_VIBRATE_MODE + ", " +
        SIMCARD_SETTING_COLUMN_RING_MODE + " " +
        "FROM " + SIMCARD_SETTING_TABLE + " " +
        "WHERE " + SIMCARD_SETTING_COLUMN_MODE + " = ? AND " +
        SIMCARD_SETTING_COLUMN_RINGTONE_TYPE + " IN (" +
        to_string(RINGTONE_TYPE_RING_SMS) + ", " +
        to_string(RINGTONE_TYPE_RING_RINGTONE) + ", " +
        to_string(RINGTONE_TYPE_VIBRATE_SMS) + ", " +
        to_string(RINGTONE_TYPE_VIBRATE_RINGTONE) + ")";
    int32_t ret = store.ExecuteSql(migrateSql,
        { NativeRdb::ValueObject(toMode), NativeRdb::ValueObject(fromMode) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("MigrateESimSimCardSetting failed, fromMode=%{public}d, toMode=%{public}d, ret=%{public}d",
            fromMode, toMode, ret);
    } else {
        RINGTONE_INFO_LOG("MigrateESimSimCardSetting success, fromMode=%{public}d, toMode=%{public}d",
            fromMode, toMode);
    }
}

static void AddESimToneTypeBit(NativeRdb::RdbStore &store, int32_t fromCardMask, int32_t toCardMask)
{
    RINGTONE_INFO_LOG("AddESimToneTypeBit: fromCardMask=%{public}d, toCardMask=%{public}d",
        fromCardMask, toCardMask);
    // Add toCardMask bitmask to records that have fromCardMask set (OR operation)
    const string updateShotSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " | ?" +
        " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ? != 0";
    int32_t ret = store.ExecuteSql(updateShotSql,
        { NativeRdb::ValueObject(toCardMask), NativeRdb::ValueObject(fromCardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("AddESimToneTypeBit shot_tone_type failed, ret=%{public}d", ret);
    }

    const string updateRingSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_RING_TONE_TYPE + " = " + RINGTONE_COLUMN_RING_TONE_TYPE + " | ?" +
        " WHERE " + RINGTONE_COLUMN_RING_TONE_TYPE + " & ? != 0";
    ret = store.ExecuteSql(updateRingSql,
        { NativeRdb::ValueObject(toCardMask), NativeRdb::ValueObject(fromCardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("AddESimToneTypeBit ring_tone_type failed, ret=%{public}d", ret);
    }
    RINGTONE_INFO_LOG("AddESimToneTypeBit complete, fromCardMask=%{public}d, toCardMask=%{public}d",
        fromCardMask, toCardMask);
}

static void ReplaceToneTypeBit(NativeRdb::RdbStore &store, int32_t fromCardMask, int32_t toCardMask)
{
    RINGTONE_INFO_LOG("ReplaceToneTypeBit: fromCardMask=%{public}d, toCardMask=%{public}d",
        fromCardMask, toCardMask);
    // Replace fromCardMask with toCardMask: clear fromCardMask bits, set toCardMask bits
    const string updateShotSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " = (" +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ~?) | ?" +
        " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ? != 0";
    int32_t ret = store.ExecuteSql(updateShotSql,
        { NativeRdb::ValueObject(fromCardMask), NativeRdb::ValueObject(toCardMask),
          NativeRdb::ValueObject(fromCardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("ReplaceToneTypeBit shot_tone_type failed, ret=%{public}d", ret);
    }

    const string updateRingSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_RING_TONE_TYPE + " = (" +
        RINGTONE_COLUMN_RING_TONE_TYPE + " & ~?) | ?" +
        " WHERE " + RINGTONE_COLUMN_RING_TONE_TYPE + " & ? != 0";
    ret = store.ExecuteSql(updateRingSql,
        { NativeRdb::ValueObject(fromCardMask), NativeRdb::ValueObject(toCardMask),
          NativeRdb::ValueObject(fromCardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("ReplaceToneTypeBit ring_tone_type failed, ret=%{public}d", ret);
    }
    RINGTONE_INFO_LOG("ReplaceToneTypeBit complete, fromCardMask=%{public}d, toCardMask=%{public}d",
        fromCardMask, toCardMask);
}

static CardConfig DetectActiveCards()
{
    CardConfig config;
#ifdef CORE_SERVICE_ENABLE
    static constexpr int32_t SIM_LABEL_INDEX_1 = 1;
    static constexpr int32_t SIM_LABEL_INDEX_2 = 2;
    std::vector<Telephony::IccAccountInfo> telIccAccountInfoList;
    int32_t ret = Telephony::CoreServiceClient::GetInstance().GetActiveSimAccountInfoList(telIccAccountInfoList);
    RINGTONE_INFO_LOG("DetectActiveCards: GetActiveSimAccountInfoList ret=%{public}d, listSize=%{public}zu",
        ret, telIccAccountInfoList.size());
    if ((ret != Telephony::TELEPHONY_ERR_SUCCESS) && (ret != Telephony::TELEPHONY_ERR_NO_SIM_CARD)) {
        RINGTONE_ERR_LOG("GetActiveSimAccountInfoList error, ret=%{public}d", ret);
        return config;
    }
    if (ret == Telephony::TELEPHONY_ERR_NO_SIM_CARD) {
        RINGTONE_INFO_LOG("No active SIM card");
        return config;
    }
    for (size_t i = 0; i < telIccAccountInfoList.size(); i++) {
        const auto &telInfo = telIccAccountInfoList[i];
        RINGTONE_INFO_LOG("  card[%{public}zu]: isEsim=%{public}d, simLabelIndex=%{public}d",
            i, telInfo.isEsim, telInfo.simLabelIndex);
        if (telInfo.isEsim) {
            if (telInfo.simLabelIndex == SIM_LABEL_INDEX_1) { config.hasESim1 = true; }
            else if (telInfo.simLabelIndex == SIM_LABEL_INDEX_2) { config.hasESim2 = true; }
        } else {
            if (telInfo.simLabelIndex == SIM_LABEL_INDEX_1) { config.hasSim1 = true; }
            else if (telInfo.simLabelIndex == SIM_LABEL_INDEX_2) { config.hasSim2 = true; }
        }
    }
    RINGTONE_INFO_LOG("Card detection result: hasSim1=%{public}d, hasSim2=%{public}d, "
        "hasESim1=%{public}d, hasESim2=%{public}d",
        config.hasSim1, config.hasSim2, config.hasESim1, config.hasESim2);
#else
    RINGTONE_WARN_LOG("CORE_SERVICE_ENABLE not defined, skip eSIM detection");
#endif
    return config;
}

static void ApplyESimMigration(NativeRdb::RdbStore &store, const CardConfig &config)
{
    if (config.hasSim1 && config.hasESim1) {
        // SIM1+eSIM1: 卡1 stays, old 卡2(=eSIM1) → new 卡2 + eSIM1
        RINGTONE_INFO_LOG("Scenario: SIM1+eSIM1, migrate old 卡2→new 卡2+eSIM1");
        MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_1);
        AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_1_MASK);
    } else if (config.hasSim1 && config.hasESim2) {
        // SIM1+eSIM2: 卡1 stays, old 卡2(=eSIM2) → new 卡2 + eSIM2
        RINGTONE_INFO_LOG("Scenario: SIM1+eSIM2, migrate old 卡2→new 卡2+eSIM2");
        MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_2);
        AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_2_MASK);
    } else if (config.hasSim2 && config.hasESim1) {
        // SIM2+eSIM1: 卡2 stays, old 卡1(=eSIM1) → new 卡1 + eSIM1
        RINGTONE_INFO_LOG("Scenario: SIM2+eSIM1, migrate old 卡1→new 卡1+eSIM1");
        MigrateESimSimCardSetting(store, SIMCARD_MODE_1, SIMCARD_MODE_ESIM_1);
        AddESimToneTypeBit(store, SIM_CARD_1_MASK, ESIM_CARD_1_MASK);
    } else if (config.hasSim2 && config.hasESim2) {
        // SIM2+eSIM2: old 卡2→new 卡1, old 卡1(=eSIM2)→new 卡1+eSIM2
        RINGTONE_INFO_LOG("Scenario: SIM2+eSIM2, migrate old 卡2→new 卡1, old 卡1(=eSIM2)→new 卡1+eSIM2");
        // SimCardSetting: first copy 卡1→eSIM2, then copy 卡2→卡1
        MigrateESimSimCardSetting(store, SIMCARD_MODE_1, SIMCARD_MODE_ESIM_2);
        MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_1);
        // ToneFiles: first add eSIM2 bit to 卡1 records, then replace 卡2 bit with 卡1 bit
        AddESimToneTypeBit(store, SIM_CARD_1_MASK, ESIM_CARD_2_MASK);
        ReplaceToneTypeBit(store, SIM_CARD_2_MASK, SIM_CARD_1_MASK);
    } else if (config.hasESim1 && config.hasESim2) {
        // eSIM1+eSIM2: old 卡1→eSIM1, old 卡2→eSIM2
        RINGTONE_INFO_LOG("Scenario: eSIM1+eSIM2, migrate old 卡1→eSIM1, old 卡2→eSIM2");
        MigrateESimSimCardSetting(store, SIMCARD_MODE_1, SIMCARD_MODE_ESIM_1);
        MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_2);
        AddESimToneTypeBit(store, SIM_CARD_1_MASK, ESIM_CARD_1_MASK);
        AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_2_MASK);
    } else if (config.hasESim1) {
        // eSIM1 only: old 卡2(=eSIM1) → new 卡2 + eSIM1
        RINGTONE_INFO_LOG("Scenario: eSIM1 only, migrate old 卡2→new 卡2+eSIM1");
        MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_1);
        AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_1_MASK);
    } else if (config.hasESim2) {
        // eSIM2 only: old 卡2(=eSIM2) → new 卡2 + eSIM2
        RINGTONE_INFO_LOG("Scenario: eSIM2 only, migrate old 卡2→new 卡2+eSIM2");
        MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_2);
        AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_2_MASK);
    } else {
        RINGTONE_INFO_LOG("No eSIM card active, skip eSIM data migration");
    }
}

static void AddESimRecords(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("AddESimRecords start");

    // Step 1: Insert eSIM SimCardSetting rows (mode 4/5, ringtone_type 1/2/101/102)
    ExecSqls({INIT_SIMCARD_SETTING_TABLE}, store);
    RINGTONE_INFO_LOG("Step1: eSIM SimCardSetting rows inserted");

    // Step 2: Detect active SIM configuration and apply data migration
    CardConfig config = DetectActiveCards();
    ApplyESimMigration(store, config);

    RINGTONE_INFO_LOG("AddESimRecords complete");
}

static bool CheckAndGetDataUri(const string &displayName, const string &dataUri,
    int32_t toneType, string &newDataUri)
{
    if (displayName.empty() || dataUri.empty()) {
        return true;
    }
    string ringDirName;
    if (toneType == TONE_TYPE_ALARM) {
        ringDirName = "alarms";
    } else if (toneType == TONE_TYPE_RINGTONE) {
        ringDirName = "ringtones";
    } else if (toneType == TONE_TYPE_NOTIFICATION) {
        ringDirName = "notifications";
    } else if (toneType == TONE_TYPE_CONTACTS) {
        ringDirName = "contacts";
    } else {
        RINGTONE_ERR_LOG("error tone type, displayName:%{public}s", displayName.c_str());
        return true;
    }
    auto lastPos = dataUri.find_last_of(RINGTONE_SLASH_CHAR);
    if (lastPos == std::string::npos) {
        return true;
    }
    string fileName = dataUri.substr(lastPos + 1);
    string filePath = dataUri.substr(0, lastPos);
    lastPos = filePath.find_last_of(RINGTONE_SLASH_CHAR);
    if (lastPos == std::string::npos) {
        return true;
    }
    string dirName = filePath.substr(lastPos + 1);
    if (dirName == ringDirName && fileName == displayName) {
        return true;
    } else {
        size_t start_pos = 0;
        if ((start_pos = dataUri.find(RINGTONE_CUSTOMIZED_BASE_PATH)) != std::string::npos) {
            newDataUri = RINGTONE_CUSTOMIZED_BASE_PATH + "/Ringtone/" + ringDirName + "/" + displayName;
            if (RingtoneFileUtils::IsFileExists(newDataUri)) {
                return false;
            }
        }
    }
    return true;
}

static void UpdateDataUri(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("Update Data Uri Begin");
    const string querySql = "SELECT " + RINGTONE_COLUMN_DISPLAY_NAME + " , " + RINGTONE_COLUMN_DATA +
        " , " + RINGTONE_COLUMN_TONE_ID + " , " + RINGTONE_COLUMN_TONE_TYPE + " FROM " + RINGTONE_TABLE +
        " WHERE " + RINGTONE_COLUMN_SOURCE_TYPE + " = 2";
    auto resultSet = store.QuerySql(querySql);
    CHECK_AND_RETURN_LOG(resultSet != nullptr, "error query sql %{public}s", querySql.c_str());
    while (resultSet->GoToNextRow() == NativeRdb::E_OK) {
        string displayName = GetStringVal(RINGTONE_COLUMN_DISPLAY_NAME, resultSet);
        string dataUri = GetStringVal(RINGTONE_COLUMN_DATA, resultSet);
        int32_t toneid = GetInt32Val(RINGTONE_COLUMN_TONE_ID, resultSet);
        int32_t toneType = GetInt32Val(RINGTONE_COLUMN_TONE_TYPE, resultSet);
        string newDataUri = "";
        if (CheckAndGetDataUri(displayName, dataUri, toneType, newDataUri)) {
            RINGTONE_INFO_LOG("check data path ok. toneid %{public}d", toneid);
            continue;
        }
        if (newDataUri.empty()) {
            RINGTONE_ERR_LOG("new data path err. toneid %{public}d", toneid);
            continue;
        }
        RINGTONE_INFO_LOG("need update uri toneid:%{public}d, displayName:%{public}s", toneid, displayName.c_str());
        NativeRdb::ValuesBucket values;
        values.PutString(RINGTONE_COLUMN_DATA, newDataUri);
        NativeRdb::AbsRdbPredicates absRdbPredicates(RINGTONE_TABLE);
        absRdbPredicates.EqualTo(RINGTONE_COLUMN_TONE_ID, toneid);
        int32_t changedRows;
        int32_t result = store.Update(changedRows, values, absRdbPredicates);
        if (result != E_OK || changedRows <= 0) {
            RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, changedRows);
        }
    }
    resultSet->Close();
    RINGTONE_INFO_LOG("Update Data Uri End");
}

static void UpdatePreloadConfTable(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("Update Preload Conf Begin");
    const string sqlCountPreloadConf = "SELECT count(1) AS count FROM " + PRELOAD_CONFIG_TABLE;
    auto resultSet = store.QuerySql(sqlCountPreloadConf);
    if (resultSet == nullptr || resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("query error");
        return;
    }
    int32_t count = GetInt32Val("count", resultSet);
    if (count > RINGTONE_PRELOAD_CONF_COUNT) {
        RINGTONE_INFO_LOG("no need to UpDatePreloadConf");
        return;
    }
    const vector<string> sqls = {
        INIT_PRELOAD_CONF_TABLE,
    };
    ExecSqls(sqls, store);
    UpdateDefaultSystemTone(store);
}

static bool VibrateModeHasValue(NativeRdb::RdbStore &store,
    const string &mode, const string &ringtoneType)
{
    const string sqlCountPreloadConf = "SELECT * FROM SimCardSetting WHERE mode = ? AND ringtone_type = ?";
    vector<string> selectionArgs = {mode, ringtoneType};
    auto resultSet = store.QuerySql(sqlCountPreloadConf, selectionArgs);
    if (resultSet == nullptr || resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("query error");
        return false;
    }
    int32_t count = GetInt32Val(SIMCARD_SETTING_COLUMN_VIBRATE_MODE, resultSet);
    if (count == 1) {
        return false;
    }
    return true;
}

static bool RingModeHasValue(NativeRdb::RdbStore &store,
    const string &mode, const string &ringtoneType)
{
    const string sqlCountPreloadConf = "SELECT * FROM SimCardSetting WHERE mode = ? AND ringtone_type = ?";
    vector<string> selectionArgs = {mode, ringtoneType};
    auto resultSet = store.QuerySql(sqlCountPreloadConf, selectionArgs);
    if (resultSet == nullptr || resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("query error");
        return false;
    }
    int32_t count = GetInt32Val(SIMCARD_SETTING_COLUMN_RING_MODE, resultSet);
    if (count == 0) {
        return false;
    }
    return true;
}

static void UpdateSimCardSettingESIMValue(NativeRdb::RdbStore &store,
    const string &mode, const string &ringtoneType, const string &modeOld, const string &ringtoneTypeOld)
{
    const string upDatesql = "UPDATE SimCardSetting SET (tone_file, vibrate_file)= \
        (SELECT tone_file, vibrate_file FROM SimCardSetting WHERE mode = ? AND ringtone_type = ?) \
        WHERE  mode = ? AND ringtone_type = ? ";
    vector<NativeRdb::ValueObject> values = {
        NativeRdb::ValueObject(modeOld),
        NativeRdb::ValueObject(ringtoneTypeOld),
        NativeRdb::ValueObject(mode),
        NativeRdb::ValueObject(ringtoneType)
    };
    int32_t err = store.ExecuteSql(upDatesql, values);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Failed to exec: %{private}s", upDatesql.c_str());
    }
    return;
}

static void UpdateSoundModeVibrateRecords(NativeRdb::RdbStore &store)
{
    //判断 mod = 1,ringtonetype = 101 是否存在数据，不存在继承 1，1
    if (VibrateModeHasValue(store, "1", "101")) {
        UpdateSimCardSettingESIMValue(store, "1", "101", "1", "1");
    }
    //判断 mod = 1,ringtonetype = 102 是否存在数据，不存在继承 1，2 //注意ring_mode = 0无震动不继承
    if (VibrateModeHasValue(store, "1", "102") && RingModeHasValue(store, "1", "2")) {
        UpdateSimCardSettingESIMValue(store, "1", "102", "1", "2");
    }
    //判断 mod = 2,ringtonetype = 101 是否存在数据，不存在继承 2，1
    if (VibrateModeHasValue(store, "2", "101")) {
        UpdateSimCardSettingESIMValue(store, "2", "101", "2", "1");
    }
    //判断 mod = 2,ringtonetype = 102 是否存在数据，不存在继承 2，2 //注意ring_mode = 0无震动不继承
    if (VibrateModeHasValue(store, "2", "102") && RingModeHasValue(store, "2", "2")) {
        UpdateSimCardSettingESIMValue(store, "2", "102", "2", "2");
    }
    //判断 mod = 3,ringtonetype = 103 是否存在数据，不存在继承 3，3
    if (VibrateModeHasValue(store, "3", "103")) {
        UpdateSimCardSettingESIMValue(store, "3", "103", "3", "3");
    }
}

static void UpgradeExtension(NativeRdb::RdbStore &store, int32_t oldVersion)
{
    if (oldVersion < VERSION_ADD_DISPLAY_LANGUAGE_COLUMN) {
        AddDisplayLanguageColumn(store);
    }
    if (oldVersion < VERSION_UPDATE_MIME_TYPE) {
        UpdateMimeType(store);
    }
    if (oldVersion < VERSION_ADD_PRELOAD_CONF_TABLE) {
        AddPreloadConfTable(store);
        UpdateDefaultSystemTone(store);
    }
    if (oldVersion < VERSION_ADD_VIBRATE_TABLE) {
        AddVibrateTable(store);
    }
    if (oldVersion < VERSION_UPDATE_WATCH_MIME_TYPE) {
        UpdateMimeType(store);
    }
    if (oldVersion < VERSION_ADD_SCANNER_FLAG) {
        AddScannerFlagColumn(store);
    }
    if (oldVersion < VERSION_UPDATE_MEDIA_TYPE_VIDEO) {
        UpdateMediaType(store);
    }
    if (oldVersion < VERSION_UPDATE_DATA_URI) {
        UpdateDataUri(store);
    }
    if (oldVersion < VERSION_ADD_HAPTIC_2_TONE_TABLE) {
        AddRingMockHapticAudioTable(store);
    }
    if (oldVersion < VERSION_ADD_SOUND_MODE_VIBRATE) {
        AddSoundModeVibrateRecords(store);
    }
    if (oldVersion < VERSION_UPDATE_PRELOAD_CONF_TABLE) {
        UpdatePreloadConfTable(store);
        UpdateSoundModeVibrateRecords(store);
        AddESimRecords(store);
    }
}

int32_t RingtoneDataCallBack::OnUpgrade(NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    RINGTONE_INFO_LOG("OnUpgrade old:%d, new:%d", oldVersion, newVersion);
    UpgradeExtension(store, oldVersion);
    return NativeRdb::E_OK;
}
} // namespace Media
} // namespace OHOS
