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

namespace OHOS {
namespace Media {
using namespace std;

static const mode_t MODE_RWX_USR_GRP = 02771;
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
    RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE        + " INT      DEFAULT 0  " + ")";


static const vector<string> g_initSqls = {
    CREATE_RINGTONE_TABLE,
};

RingtoneDataCallBack::RingtoneDataCallBack(void)
{
}

RingtoneDataCallBack::~RingtoneDataCallBack(void)
{
}


int32_t RingtoneDataCallBack::MkdirRecursive(const string &path, size_t start)
{
    RINGTONE_DEBUG_LOG("start pos %{public}zu", start);
    size_t end = path.find("/", start + 1);

    string subDir = "";
    if (end == std::string::npos) {
        if (start + 1 == path.size()) {
            RINGTONE_DEBUG_LOG("path size=%zu", path.size());
        } else {
            subDir = path.substr(start + 1, path.size() - start - 1);
        }
    } else {
        subDir = path.substr(start + 1, end - start - 1);
    }

    if (subDir.size() == 0) {
        return NativeRdb::E_OK;
    } else {
        string real = path.substr(0, start + subDir.size() + 1);
        mode_t mask = umask(0);
        int result = mkdir(real.c_str(), MODE_RWX_USR_GRP);
        if (result == 0) {
            RINGTONE_INFO_LOG("mkdir %{public}s successfully", real.c_str());
        } else {
            RINGTONE_INFO_LOG("mkdir %{public}s failed, errno is %{public}d", real.c_str(), errno);
        }
        umask(mask);
    }
    if (end == std::string::npos) {
        return NativeRdb::E_OK;
    }

    return MkdirRecursive(path, end);
}

int32_t RingtoneDataCallBack::CreatePreloadFolder(const string &path)
{
    RINGTONE_DEBUG_LOG("start");
    if (access(path.c_str(), F_OK) == 0) {
        RINGTONE_INFO_LOG("dir is existing");
        return NativeRdb::E_OK;
    }

    auto start = path.find(RINGTONE_CUSTOMIZED_BASE_PATH);
    if (start == string::npos) {
        RINGTONE_ERR_LOG("base dir is wrong");
        return NativeRdb::E_ERROR;
    }

    return MkdirRecursive(path, start + RINGTONE_CUSTOMIZED_BASE_PATH.size());
}

int32_t RingtoneDataCallBack::PrepareDir()
{
    static const vector<string> userPreloadDirs = {
        { RINGTONE_CUSTOMIZED_ALARM_PATH }, { RINGTONE_CUSTOMIZED_RINGTONE_PATH },
        { RINGTONE_CUSTOMIZED_NOTIFICATIONS_PATH }
    };

    for (const auto &dir: userPreloadDirs) {
        if (CreatePreloadFolder(dir) != NativeRdb::E_OK) {
            RINGTONE_INFO_LOG("scan failed on dir %{public}s", dir.c_str());
            continue;
        }
    }
    return NativeRdb::E_OK;
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

    if (PrepareDir() != NativeRdb::E_OK) {
        RINGTONE_DEBUG_LOG("Prepare dir error");
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

int32_t RingtoneDataCallBack::OnUpgrade(NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    RINGTONE_DEBUG_LOG("OnUpgrade old:%d, new:%d", oldVersion, newVersion);

    return NativeRdb::E_OK;
}
} // namespace Media
} // namespace OHOS