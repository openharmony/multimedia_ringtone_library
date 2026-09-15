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

#include <set>
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

struct SimCardConfigInfo {
    bool isEsim = false;
    int32_t simLabelIndex = 0;
    int32_t slotIndex = 0;
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

// 拷贝 SimCardSetting 表数据：从 fromMode 列拷贝到 toMode 列
// 拷贝字段：tone_file, vibrate_file, vibrate_mode, ring_mode
// 限定 ringtone_type 为 1(短信), 2(来电), 101(振动短信), 102(振动来电)
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

// 给包含 fromCardMask 比特位的行追加 toCardMask 比特位（OR 操作）
// 同时处理 shot_tone_type 和 ring_tone_type 两列
static void AddESimToneTypeBit(NativeRdb::RdbStore &store, int32_t fromCardMask, int32_t toCardMask)
{
    RINGTONE_INFO_LOG("AddESimToneTypeBit: fromCardMask=%{public}d, toCardMask=%{public}d",
        fromCardMask, toCardMask);
    // 短信铃声：给包含 fromCardMask 的行追加 toCardMask
    const string updateShotSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " | ?" +
        " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ? != 0";
    int32_t ret = store.ExecuteSql(updateShotSql,
        { NativeRdb::ValueObject(toCardMask), NativeRdb::ValueObject(fromCardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("AddESimToneTypeBit shot_tone_type failed, ret=%{public}d", ret);
    }

    // 来电铃声：给包含 fromCardMask 的行追加 toCardMask
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


// 消除指定卡位掩码的系统预置默认铃声（source_type=1 的行中清除对应比特位）
static void RemovePresetToneTypeBit(NativeRdb::RdbStore &store, int32_t cardMask)
{
    RINGTONE_INFO_LOG("RemovePresetToneTypeBit: cardMask=%{public}d", cardMask);
    // 消除来电铃声的系统预置默认值
    const string updateRingSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_RING_TONE_TYPE + " = " +
        RINGTONE_COLUMN_RING_TONE_TYPE + " & ~?" +
        " WHERE " + RINGTONE_COLUMN_RING_TONE_TYPE + " & ? != 0" +
        " AND " + RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE + " = ?";
    int32_t ret = store.ExecuteSql(updateRingSql,
        { NativeRdb::ValueObject(cardMask), NativeRdb::ValueObject(cardMask),
          NativeRdb::ValueObject(SOURCE_TYPE_PRESET) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("RemovePresetToneTypeBit ring_tone_type failed, ret=%{public}d", ret);
    }

    // 消除短信铃声的系统预置默认值
    const string updateShotSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ~?" +
        " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ? != 0" +
        " AND " + RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE + " = ?";
    ret = store.ExecuteSql(updateShotSql,
        { NativeRdb::ValueObject(cardMask), NativeRdb::ValueObject(cardMask),
          NativeRdb::ValueObject(SOURCE_TYPE_PRESET) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("RemovePresetToneTypeBit shot_tone_type failed, ret=%{public}d", ret);
    }
    RINGTONE_INFO_LOG("RemovePresetToneTypeBit complete, cardMask=%{public}d", cardMask);
}

// 清除所有行中指定卡位掩码的比特位（不限 source_type）
static void ClearToneTypeBit(NativeRdb::RdbStore &store, int32_t cardMask)
{
    RINGTONE_INFO_LOG("ClearToneTypeBit: cardMask=%{public}d", cardMask);
    // 清除来电铃声的卡位比特
    const string updateRingSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_RING_TONE_TYPE + " = " +
        RINGTONE_COLUMN_RING_TONE_TYPE + " & ~?" +
        " WHERE " + RINGTONE_COLUMN_RING_TONE_TYPE + " & ? != 0";
    int32_t ret = store.ExecuteSql(updateRingSql,
        { NativeRdb::ValueObject(cardMask), NativeRdb::ValueObject(cardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("ClearToneTypeBit ring_tone_type failed, ret=%{public}d", ret);
    }

    // 清除短信铃声的卡位比特
    const string updateShotSql =
        "UPDATE " + RINGTONE_TABLE + " SET " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " = " +
        RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ~?" +
        " WHERE " + RINGTONE_COLUMN_SHOT_TONE_TYPE + " & ? != 0";
    ret = store.ExecuteSql(updateShotSql,
        { NativeRdb::ValueObject(cardMask), NativeRdb::ValueObject(cardMask) });
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("ClearToneTypeBit shot_tone_type failed, ret=%{public}d", ret);
    }
    RINGTONE_INFO_LOG("ClearToneTypeBit complete, cardMask=%{public}d", cardMask);
}

// DetectActiveCards: GetActiveSimAccountInfoList ret=0, listSize=2 
// [{"simId":1,"isEsim":false,"slotIndex":0,"simLabelIndex":1,"isActive":true,"iccId":"8986****1924****6716"}]
// card[0]: isEsim=0, simLabelIndex=2
static std::vector<SimCardConfigInfo> DetectActiveCards()
{
    std::vector<SimCardConfigInfo> config;
#ifdef CORE_SERVICE_ENABLE
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
        SimCardConfigInfo simInfo;
        simInfo.isEsim = telInfo.isEsim;
        simInfo.simLabelIndex = telInfo.simLabelIndex;
        simInfo.slotIndex = telInfo.slotIndex;
        config.push_back(simInfo);
    }
#else
    RINGTONE_WARN_LOG("CORE_SERVICE_ENABLE not defined, skip eSIM detection");
#endif
    return config;
}

// ==================== eSIM 迁移处理函数（对应 req.md 11 种场景） ====================

// 场景1：0 张卡 — 不做任何处理
static void ESimMigration1(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration1: 0 cards, 不做任何处理");
}

// 场景2：SIM1 — 不做任何处理
static void ESimMigration2(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration2: SIM1 only, 不做任何处理");
}

// 场景3：SIM2 — 不做任何处理
static void ESimMigration3(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration3: SIM2 only, 不做任何处理");
}

// 场景4：eSIM1
// SimCardSetting: mode=2(原eSIM1) → mode=4(eSIM1) 拷贝 tone_file/vibrate_file/vibrate_mode/ring_mode
// ToneFiles: 消除eSIM1预置默认铃声，将卡二铃声继承到eSIM1
static void ESimMigration4(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration4: eSIM1 only");
    // SimCardSetting: 卡槽2原eSIM1(mode=2) → eSIM1(mode=4)
    MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_1);
    // ToneFiles 步骤1-2: 消除eSIM1系统预置的默认铃声和短信音
    RemovePresetToneTypeBit(store, ESIM_CARD_1_MASK);
    // ToneFiles 步骤3-4: 包含卡二(SIM_CARD_2_MASK)的铃声追加eSIM1比特位(ESIM_CARD_1_MASK)
    AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_1_MASK);
}

// 场景5：eSIM2
// SimCardSetting: mode=2(原eSIM2) → mode=5(eSIM2) 拷贝 tone_file/vibrate_file/vibrate_mode/ring_mode
// ToneFiles: 消除eSIM2预置默认铃声，将卡二铃声继承到eSIM2
static void ESimMigration5(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration5: eSIM2 only");
    // SimCardSetting: 卡槽2原eSIM2(mode=2) → eSIM2(mode=5)
    MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_2);
    // ToneFiles 步骤1-2: 消除eSIM2系统预置的默认铃声和短信音
    RemovePresetToneTypeBit(store, ESIM_CARD_2_MASK);
    // ToneFiles 步骤3-4: 包含卡二(SIM_CARD_2_MASK)的铃声追加eSIM2比特位(ESIM_CARD_2_MASK)
    AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_2_MASK);
}

// 场景6：SIM1+SIM2 — 不做任何处理
static void ESimMigration6(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration6: SIM1+SIM2, 不做任何处理");
}

// 场景7：SIM1+eSIM1
// SimCardSetting: mode=2(原eSIM1) → mode=4(eSIM1)
// ToneFiles: 消除eSIM1预置默认铃声，将卡二铃声继承到eSIM1
static void ESimMigration7(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration7: SIM1+eSIM1");
    // SimCardSetting: 卡槽2原eSIM1(mode=2) → eSIM1(mode=4)
    MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_1);
    // ToneFiles 步骤1-2: 消除eSIM1系统预置的默认铃声和短信音
    RemovePresetToneTypeBit(store, ESIM_CARD_1_MASK);
    // ToneFiles 步骤3-4: 包含卡二(SIM_CARD_2_MASK)的铃声追加eSIM1比特位(ESIM_CARD_1_MASK)
    AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_1_MASK);
}

// 场景8：SIM1+eSIM2
// SimCardSetting: mode=2(原eSIM2) → mode=5(eSIM2)
// ToneFiles: 消除eSIM2预置默认铃声，将卡二铃声继承到eSIM2
static void ESimMigration8(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration8: SIM1+eSIM2");
    // SimCardSetting: 卡槽2原eSIM2(mode=2) → eSIM2(mode=5)
    MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_2);
    // ToneFiles 步骤1-2: 消除eSIM2系统预置的默认铃声和短信音
    RemovePresetToneTypeBit(store, ESIM_CARD_2_MASK);
    // ToneFiles 步骤3-4: 包含卡二(SIM_CARD_2_MASK)的铃声追加eSIM2比特位(ESIM_CARD_2_MASK)
    AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_2_MASK);
}

// 场景9：SIM2+eSIM1
// SimCardSetting: mode=2(原eSIM1) → mode=4(eSIM1), mode=1(原SIM2) → mode=2(覆盖卡1卡2)
// ToneFiles: 消除eSIM1预置 → 卡二继承到eSIM1 → 清除卡二 → 卡一继承到卡二
static void ESimMigration9(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration9: SIM2+eSIM1");
    // ---- SimCardSetting 表 ----
    // 步骤1: 卡槽2原eSIM1(mode=2) → eSIM1(mode=4)，必须先迁移eSIM避免被覆盖
    MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_1);
    // 步骤2: 卡槽1原SIM2(mode=1) → 卡1卡2(mode=2)，覆盖写入
    MigrateESimSimCardSetting(store, SIMCARD_MODE_1, SIMCARD_MODE_2);
    // ---- ToneFiles 表 ----
    // 步骤1-2: 消除eSIM1系统预置的默认铃声和短信音
    RemovePresetToneTypeBit(store, ESIM_CARD_1_MASK);
    // 步骤3-4: 包含卡二(SIM_CARD_2_MASK)的铃声追加eSIM1比特位(ESIM_CARD_1_MASK)
    AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_1_MASK);
    // 步骤5-6: 清除原SIM2设置的铃声和短信音（清除卡二比特位）
    ClearToneTypeBit(store, SIM_CARD_2_MASK);
    // 步骤7-8: 包含卡一(SIM_CARD_1_MASK)的铃声追加卡二比特位(SIM_CARD_2_MASK)
    AddESimToneTypeBit(store, SIM_CARD_1_MASK, SIM_CARD_2_MASK);
}

// 场景10：SIM2+eSIM2
// SimCardSetting: mode=2(原eSIM2) → mode=5(eSIM2), mode=1(原SIM2) → mode=2(覆盖卡1卡2)
// ToneFiles: 消除eSIM2预置 → 卡二继承到eSIM2 → 清除卡二 → 卡一继承到卡二
static void ESimMigration10(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration10: SIM2+eSIM2");
    // ---- SimCardSetting 表 ----
    // 步骤1: 卡槽2原eSIM2(mode=2) → eSIM2(mode=5)，必须先迁移eSIM避免被覆盖
    MigrateESimSimCardSetting(store, SIMCARD_MODE_2, SIMCARD_MODE_ESIM_2);
    // 步骤2: 卡槽1原SIM2(mode=1) → 卡1卡2(mode=2)，覆盖写入
    MigrateESimSimCardSetting(store, SIMCARD_MODE_1, SIMCARD_MODE_2);
    // ---- ToneFiles 表 ----
    // 步骤1-2: 消除eSIM2系统预置的默认铃声和短信音
    RemovePresetToneTypeBit(store, ESIM_CARD_2_MASK);
    // 步骤3-4: 包含卡二(SIM_CARD_2_MASK)的铃声追加eSIM2比特位(ESIM_CARD_2_MASK)
    AddESimToneTypeBit(store, SIM_CARD_2_MASK, ESIM_CARD_2_MASK);
    // 步骤5-6: 清除原SIM2设置的铃声和短信音（清除卡二比特位）
    ClearToneTypeBit(store, SIM_CARD_2_MASK);
    // 步骤7-8: 包含卡一(SIM_CARD_1_MASK)的铃声追加卡二比特位(SIM_CARD_2_MASK)
    AddESimToneTypeBit(store, SIM_CARD_1_MASK, SIM_CARD_2_MASK);
}

// 场景11：eSIM1+eSIM2 — 不做任何处理
static void ESimMigration11(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("ESimMigration11: eSIM1+eSIM2, 不做任何处理");
}

// ==================== 配置签名（用于顺序无关的场景匹配） ====================
// 使用 (isEsim, simLabelIndex, slotIndex) 三元组标识每张卡，
// 与设计表 configs 列完全对应，确保匹配精确无误。

struct ConfigSig {
    bool isEsim;
    int32_t simLabelIndex;
    int32_t slotIndex;

    bool operator<(const ConfigSig &o) const
    {
        if (isEsim != o.isEsim) {
            return isEsim < o.isEsim;
        }
        if (simLabelIndex != o.simLabelIndex) {
            return simLabelIndex < o.simLabelIndex;
        }
        return slotIndex < o.slotIndex;
    }
    bool operator==(const ConfigSig &o) const
    {
        return isEsim == o.isEsim && simLabelIndex == o.simLabelIndex && slotIndex == o.slotIndex;
    }
};

// 从 SimCardConfigInfo 提取配置签名
static ConfigSig MakeSig(const SimCardConfigInfo &c)
{
    return { c.isEsim, c.simLabelIndex, c.slotIndex };
}

// 将配置列表转换为有序签名集合，用于顺序无关的比较
static std::set<ConfigSig> MakeSigSet(const std::vector<SimCardConfigInfo> &configs)
{
    std::set<ConfigSig> sigs;
    for (const auto &c : configs) {
        sigs.insert(MakeSig(c));
    }
    return sigs;
}

// 单卡场景分发：根据 (isEsim, simLabelIndex, slotIndex) 匹配场景2~5
static void DispatchSingleCard(NativeRdb::RdbStore &store, const SimCardConfigInfo &card)
{
    ConfigSig sig = MakeSig(card);
    // 场景2：SIM1 — (isEsim=false, simLabelIndex=1, slotIndex=0)
    if (sig == ConfigSig{false, 1, 0}) {
        ESimMigration2(store);
        return;
    }
    // 场景3：SIM2 — (isEsim=false, simLabelIndex=2, slotIndex=1)
    if (sig == ConfigSig{false, 2, 1}) {
        ESimMigration3(store);
        return;
    }
    // 场景4：eSIM1 — (isEsim=true, simLabelIndex=1, slotIndex=1)
    if (sig == ConfigSig{true, 1, 1}) {
        ESimMigration4(store);
        return;
    }
    // 场景5：eSIM2 — (isEsim=true, simLabelIndex=2, slotIndex=1)
    if (sig == ConfigSig{true, 2, 1}) {
        ESimMigration5(store);
        return;
    }
    RINGTONE_ERR_LOG("DispatchSingleCard: 无法识别的单卡配置 "
        "isEsim=%{public}d, simLabelIndex=%{public}d, slotIndex=%{public}d",
        card.isEsim, card.simLabelIndex, card.slotIndex);
}

// 双卡场景分发：使用有序签名集合做顺序无关匹配，匹配场景6~11
static void DispatchDualCard(NativeRdb::RdbStore &store, const std::vector<SimCardConfigInfo> &configs)
{
    std::set<ConfigSig> sigs = MakeSigSet(configs);
    // 场景6：SIM1+SIM2 — (false,1,0) + (false,2,1)
    if (sigs == std::set<ConfigSig>{{false, 1, 0}, {false, 2, 1}}) {
        ESimMigration6(store);
        return;
    }
    // 场景7：SIM1+eSIM1 — (false,1,0) + (true,1,1)
    if (sigs == std::set<ConfigSig>{{false, 1, 0}, {true, 1, 1}}) {
        ESimMigration7(store);
        return;
    }
    // 场景8：SIM1+eSIM2 — (false,1,0) + (true,2,1)
    if (sigs == std::set<ConfigSig>{{false, 1, 0}, {true, 2, 1}}) {
        ESimMigration8(store);
        return;
    }
    // 场景9：SIM2+eSIM1 — (false,2,0) + (true,1,1)
    if (sigs == std::set<ConfigSig>{{false, 2, 0}, {true, 1, 1}}) {
        ESimMigration9(store);
        return;
    }
    // 场景10：SIM2+eSIM2 — (false,2,0) + (true,2,1)
    if (sigs == std::set<ConfigSig>{{false, 2, 0}, {true, 2, 1}}) {
        ESimMigration10(store);
        return;
    }
    // 场景11：eSIM1+eSIM2 — (true,1,0) + (true,2,1)
    if (sigs == std::set<ConfigSig>{{true, 1, 0}, {true, 2, 1}}) {
        ESimMigration11(store);
        return;
    }
    RINGTONE_ERR_LOG("DispatchDualCard: 无法识别的双卡配置组合");
    for (const auto &c : configs) {
        RINGTONE_ERR_LOG("  config: isEsim=%{public}d, simLabelIndex=%{public}d, slotIndex=%{public}d",
            c.isEsim, c.simLabelIndex, c.slotIndex);
    }
}

// ==================== ApplyESimMigration 分发函数 ====================
// 根据 configs 的卡数量和每张卡的 (isEsim, simLabelIndex, slotIndex) 组合，
// 分发到对应的 ESimMigration1~11 处理函数。
// configs 列表顺序可能不同，使用有序签名集合做顺序无关匹配。
static void ApplyESimMigration(NativeRdb::RdbStore &store, const std::vector<SimCardConfigInfo> &configs)
{
    RINGTONE_INFO_LOG("ApplyESimMigration: configs.size=%{public}zu", configs.size());
    if (configs.empty()) {
        // 场景1：0 张卡
        ESimMigration1(store);
        return;
    }
    if (configs.size() == 1) {
        // 单卡场景：分发到场景2~5
        DispatchSingleCard(store, configs[0]);
        return;
    }
    // 双卡场景：分发到场景6~11
    DispatchDualCard(store, configs);
}

static void AddESimRecords(NativeRdb::RdbStore &store)
{
    RINGTONE_INFO_LOG("AddESimRecords start");

    // Step 1: Insert eSIM SimCardSetting rows (mode 4/5, ringtone_type 1/2/101/102)
    ExecSqls({INIT_SIMCARD_SETTING_TABLE}, store);
    RINGTONE_INFO_LOG("Step1: eSIM SimCardSetting rows inserted");

    // Step 2: Detect active SIM configuration and apply data migration
    std::vector<SimCardConfigInfo> configs = DetectActiveCards();
    ApplyESimMigration(store, configs);

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

static bool VibrateModeIsNull(NativeRdb::RdbStore &store,
    const string &mode, const string &ringtoneType)
{
    const string sqlCountPreloadConf = "SELECT vibrate_mode FROM SimCardSetting WHERE mode = ? AND ringtone_type = ?";
    vector<string> selectionArgs = {mode, ringtoneType};
    auto resultSet = store.QuerySql(sqlCountPreloadConf, selectionArgs);
    if (resultSet == nullptr || resultSet->GoToFirstRow() != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("query error");
        return false;
    }
    int32_t vibrateMode = GetInt32Val(SIMCARD_SETTING_COLUMN_VIBRATE_MODE, resultSet);
    if (vibrateMode == 1) {
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
    if (VibrateModeIsNull(store, "1", "101")) {
        UpdateSimCardSettingESIMValue(store, "1", "101", "1", "1");
    }
    //判断 mod = 1,ringtonetype = 102 是否存在数据，不存在继承 1，2 //注意ring_mode = 0无震动不继承
    if (VibrateModeIsNull(store, "1", "102") && RingModeHasValue(store, "1", "2")) {
        UpdateSimCardSettingESIMValue(store, "1", "102", "1", "2");
    }
    //判断 mod = 2,ringtonetype = 101 是否存在数据，不存在继承 2，1
    if (VibrateModeIsNull(store, "2", "101")) {
        UpdateSimCardSettingESIMValue(store, "2", "101", "2", "1");
    }
    //判断 mod = 2,ringtonetype = 102 是否存在数据，不存在继承 2，2 //注意ring_mode = 0无震动不继承
    if (VibrateModeIsNull(store, "2", "102") && RingModeHasValue(store, "2", "2")) {
        UpdateSimCardSettingESIMValue(store, "2", "102", "2", "2");
    }
    //判断 mod = 3,ringtonetype = 103 是否存在数据，不存在继承 3，3
    if (VibrateModeIsNull(store, "3", "103")) {
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
