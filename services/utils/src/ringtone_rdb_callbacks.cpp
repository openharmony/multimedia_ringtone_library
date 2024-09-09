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

namespace OHOS {
namespace Media {
using namespace std;

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
    RINGTONE_COLUMN_DISPLAY_LANGUAGE_TYPE         + " TEXT                " + ")";

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
    SIMCARD_SETTING_COLUMN_RINGTONE_TYPE          + ") VALUES (1, 0), (1, 1), (1, 2), (1, 3), \
                                                        (2, 0), (2, 1), (2, 2), (2, 3),        \
                                                        (3, 0), (3, 1), (3, 2), (3, 3);";

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
    VIBRATE_COLUMN_PLAY_MODE                      + " INT      DEFAULT 0  " + ")";


static const vector<string> g_initSqls = {
    CREATE_RINGTONE_TABLE,
    CREATE_VIBRATE_TABLE,
    CREATE_SIMCARD_SETTING_TABLE,
    INIT_SIMCARD_SETTING_TABLE,
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

static void AddVibrateTable(NativeRdb::RdbStore &store)
{
    const vector<string> sqls = {
        CREATE_VIBRATE_TABLE,
        CREATE_SIMCARD_SETTING_TABLE,
        INIT_SIMCARD_SETTING_TABLE,
    };
    RINGTONE_INFO_LOG("Add vibrate table");
    ExecSqls(sqls, store);
}

static void UpgradeExtension(NativeRdb::RdbStore &store, int32_t oldVersion)
{
    if (oldVersion < VERSION_ADD_DISPLAY_LANGUAGE_COLUMN) {
        AddDisplayLanguageColumn(store);
    }
    if (oldVersion < VERSION_ADD_VIBRATE_TABLE) {
        AddVibrateTable(store);
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
