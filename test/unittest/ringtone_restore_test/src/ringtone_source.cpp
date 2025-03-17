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

#include "ringtone_source.h"

#include "ringtone_db_const.h"

using namespace std;

namespace OHOS {
namespace Media {

const string RingtoneRestoreRdbOpenCb::CREATE_RINGTONE_TABLE = "CREATE TABLE IF NOT EXISTS " + RINGTONE_TABLE + "(" +
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
    RINGTONE_COLUMN_SCANNER_FLAG                  + " INT      DEFAULT 0  " + ")";

int RingtoneRestoreRdbOpenCb::OnCreate(NativeRdb::RdbStore &store)
{
    return store.ExecuteSql(CREATE_RINGTONE_TABLE);
}

int RingtoneRestoreRdbOpenCb::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    return 0;
}

int RingtoneLocalRdbOpenCb::OnCreate(NativeRdb::RdbStore &store)
{
    return 0;
}

int RingtoneLocalRdbOpenCb::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    return 0;
}

void RingtoneSource::Init(const std::string &restoreDbPath, const std::string &localDbPath)
{
    int errCode = 0;
    NativeRdb::RdbStoreConfig localRdbConfig(localDbPath);
    RingtoneLocalRdbOpenCb localDbHelper;
    localRdbPtr_ = NativeRdb::RdbHelper::GetRdbStore(localRdbConfig, 1, localDbHelper, errCode);
    NativeRdb::RdbStoreConfig restoreRdbConfig(restoreDbPath);
    RingtoneRestoreRdbOpenCb restoreDbHelper;
    restoreRdbPtr_ = NativeRdb::RdbHelper::GetRdbStore(restoreRdbConfig, 1, restoreDbHelper, errCode);
    InitRingtoneDb();
}

void RingtoneSource::InitRingtoneDb()
{
    restoreRdbPtr_->ExecuteSql("INSERT INTO " + RINGTONE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/alarms/Adara.ogg'," +
        " 10414, 'Adara.ogg', 'Adara', 2, 0, 'audio/ogg', 2, 1505707241000, 1505707241846, 1505707241," +
        " 600, 0, -1, 0, -1, 0, -1, 1, 2)");
    restoreRdbPtr_->ExecuteSql("INSERT INTO " + RINGTONE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/ringtones/Carme.ogg'," +
        " 26177, 'Carme.ogg', 'Carme', 2, 1, 'audio/ogg', 2, 1505707241000, 1505707241846, 1505707241," +
        " 1242, 0, -1, 0, -1, 3, 2, 0, -1)");
    restoreRdbPtr_->ExecuteSql("INSERT INTO " + RINGTONE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/notifications/Radon.ogg'," +
        " 25356, 'Radon.ogg', 'Radon', 2, 2, 'audio/ogg', 2, 1505707241000, 1505707241846, 1505707241," +
        " 1800, 0, -1, 1, 2, 0, -1, 0, -1)");
    restoreRdbPtr_->ExecuteSql("INSERT INTO " + RINGTONE_TABLE +
        " VALUES (last_insert_rowid()+1, '/data/storage/el2/base/files/Ringtone/notifications/Titan.ogg'," +
        " 30984, 'Titan.ogg', 'Titan', 2, 2, 'audio/ogg', 2, 1505707241000, 1505707241846, 1505707241," +
        " 1947, 3, 2, 0, -1, 0, -1, 0, -1)");
}
} // namespace Media
} // namespace OHOS