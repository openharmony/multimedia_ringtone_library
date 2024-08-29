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
#define MLOG_TAG "Scanner"

#include "ringtone_scanner_db.h"

#include "result_set_utils.h"
#include "rdb_errno.h"
#include "ringtone_errno.h"
#include "ringtone_file_utils.h"
#include "ringtone_log.h"
#include "ringtone_rdbstore.h"

namespace OHOS {
namespace Media {
using namespace std;
using namespace OHOS::NativeRdb;
using namespace OHOS::DataShare;
int32_t RingtoneScannerDb::GetFileBasicInfo(const string &path, unique_ptr<RingtoneMetadata> &ptr)
{
    vector<string> columns = {
        RINGTONE_COLUMN_TONE_ID, RINGTONE_COLUMN_SIZE, RINGTONE_COLUMN_DATE_MODIFIED, RINGTONE_COLUMN_TITLE,
        RINGTONE_COLUMN_DATE_ADDED
    };
    string whereClause = RINGTONE_COLUMN_DATA + " = ?";
    vector<string> args = { path };

    Uri uri("");
    RingtoneDataCommand cmd(uri, RINGTONE_TABLE, RingtoneOperationType::QUERY);
    cmd.GetAbsRdbPredicates()->SetWhereClause(whereClause);
    cmd.GetAbsRdbPredicates()->SetWhereArgs(args);

    shared_ptr<NativeRdb::ResultSet> resultSet;
    int32_t ret = GetFileSet(cmd, columns, resultSet);
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("Update operation failed. ret=%{public}d", ret);
        return E_DB_FAIL;
    }

    return FillMetadata(resultSet, ptr);
}

int32_t RingtoneScannerDb::GetVibrateFileBasicInfo(const string &path, unique_ptr<VibrateMetadata> &ptr)
{
    vector<string> columns = {
        VIBRATE_COLUMN_VIBRATE_ID, VIBRATE_COLUMN_SIZE, VIBRATE_COLUMN_DATE_MODIFIED, VIBRATE_COLUMN_TITLE,
        VIBRATE_COLUMN_DATE_ADDED
    };
    string whereClause = VIBRATE_COLUMN_DATA + " = ?";
    vector<string> args = { path };

    Uri uri("");
    RingtoneDataCommand cmd(uri, VIBRATE_TABLE, RingtoneOperationType::QUERY);
    cmd.GetAbsRdbPredicates()->SetWhereClause(whereClause);
    cmd.GetAbsRdbPredicates()->SetWhereArgs(args);

    shared_ptr<NativeRdb::ResultSet> resultSet;
    int32_t ret = GetFileSet(cmd, columns, resultSet);
    if (ret != E_OK) {
        RINGTONE_ERR_LOG("Update operation failed. ret=%{public}d", ret);
        return E_DB_FAIL;
    }

    return FillVibrateMetadata(resultSet, ptr);
}

int32_t RingtoneScannerDb::QueryRingtoneRdb(const string &whereClause, vector<string> &whereArgs,
    const vector<string> &columns, shared_ptr<NativeRdb::ResultSet> &resultSet, const string &tableName)
{
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("get raw rdb failed");
        return E_RDB;
    }

    AbsRdbPredicates absRdbPredicates(tableName);
    absRdbPredicates.SetWhereClause(whereClause);
    absRdbPredicates.SetWhereArgs(whereArgs);
    resultSet = rawRdb->Query(absRdbPredicates, columns);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("return nullptr when query rdb");
        return E_RDB;
    }

    return E_OK;
}

int32_t RingtoneScannerDb::UpdateRingtoneRdb(ValuesBucket &values, const string &whereClause,
    vector<string> &whereArgs, const string &tableName)
{
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("get raw rdb failed");
        return E_RDB;
    }
    int32_t updateCount = 0;
    int32_t result = rawRdb->Update(updateCount, tableName, values, whereClause, whereArgs);
    if (result != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, updateCount);
        return E_DB_FAIL;
    }
    return updateCount;
}

int32_t RingtoneScannerDb::FillMetadata(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &ptr)
{
    std::vector<std::string> columnNames;
    int32_t err = resultSet->GetAllColumnNames(columnNames);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get all column names");
        return E_RDB;
    }

    for (const auto &col : columnNames) {
        ExtractMetaFromColumn(resultSet, ptr, col);
    }

    return E_OK;
}

int32_t RingtoneScannerDb::FillVibrateMetadata(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<VibrateMetadata> &ptr)
{
    std::vector<std::string> columnNames;
    int32_t err = resultSet->GetAllColumnNames(columnNames);
    if (err != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get all column names");
        return E_RDB;
    }

    for (const auto &col : columnNames) {
        ExtractVibrateMetaFromColumn(resultSet, ptr, col);
    }

    return E_OK;
}

void RingtoneScannerDb::ExtractMetaFromColumn(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<RingtoneMetadata> &metadata, const std::string &col)
{
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_NULL;
    RingtoneMetadata::RingtoneMetadataFnPtr requestFunc = nullptr;
    auto itr = metadata->memberFuncMap_.find(col);
    if (itr != metadata->memberFuncMap_.end()) {
        dataType = itr->second.first;
        requestFunc = itr->second.second;
    } else {
        RINGTONE_ERR_LOG("invalid column name %{private}s", col.c_str());
        return;
    }

    std::variant<int32_t, std::string, int64_t, double> data =
        ResultSetUtils::GetValFromColumn<const shared_ptr<NativeRdb::ResultSet>>(col, resultSet, dataType);

    // Use the function pointer from map and pass data to fn ptr
    if (requestFunc != nullptr) {
        (metadata.get()->*requestFunc)(data);
    }
}

void RingtoneScannerDb::ExtractVibrateMetaFromColumn(const shared_ptr<NativeRdb::ResultSet> &resultSet,
    unique_ptr<VibrateMetadata> &metadata, const std::string &col)
{
    RingtoneResultSetDataType dataType = RingtoneResultSetDataType::DATA_TYPE_NULL;
    VibrateMetadata::VibrateMetadataFnPtr requestFunc = nullptr;
    auto itr = metadata->memberFuncMap_.find(col);
    if (itr != metadata->memberFuncMap_.end()) {
        dataType = itr->second.first;
        requestFunc = itr->second.second;
    } else {
        RINGTONE_ERR_LOG("invalid column name %{private}s", col.c_str());
        return;
    }

    std::variant<int32_t, std::string, int64_t, double> data =
        ResultSetUtils::GetValFromColumn<const shared_ptr<NativeRdb::ResultSet>>(col, resultSet, dataType);

    // Use the function pointer from map and pass data to fn ptr
    if (requestFunc != nullptr) {
        (metadata.get()->*requestFunc)(data);
    }
}

int32_t RingtoneScannerDb::GetFileSet(RingtoneDataCommand &cmd, const vector<string> &columns,
    shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("get raw rdb failed");
        return E_RDB;
    }
    resultSet = rawRdb->Query(*cmd.GetAbsRdbPredicates(), columns);
    if (resultSet == nullptr) {
        RINGTONE_ERR_LOG("return nullptr when query rdb");
        return E_RDB;
    }

    int32_t rowCount = 0;
    int32_t ret = resultSet->GetRowCount(rowCount);
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to get row count");
        return E_RDB;
    }
    if (rowCount == 0) {
        return E_OK;
    }

    ret = resultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("failed to go to first row");
        return E_RDB;
    }
    return E_OK;
}

static void InsertDateAdded(const RingtoneMetadata &metadata, ValuesBucket &outValues)
{
    int64_t dateAdded = metadata.GetDateAdded();
    if (dateAdded == 0) {
        int64_t dateTaken = metadata.GetDateTaken();
        if (dateTaken == 0) {
            int64_t dateModified = metadata.GetDateModified();
            if (dateModified == 0) {
                dateAdded = RingtoneFileUtils::UTCTimeMilliSeconds();
                RINGTONE_WARN_LOG("Invalid dateAdded time, use current time instead: %{public}" PRId64, dateAdded);
            } else {
                dateAdded = dateModified;
                RINGTONE_WARN_LOG("Invalid dateAdded time, use dateModified instead: %{public}" PRId64, dateAdded);
            }
        } else {
            dateAdded = dateTaken * MSEC_TO_SEC;
            RINGTONE_WARN_LOG("Invalid dateAdded time, use dateTaken instead: %{public}" PRId64, dateAdded);
        }
    }
    outValues.PutLong(RINGTONE_COLUMN_DATE_ADDED, dateAdded);
}

static void InsertVibrateDateAdded(const VibrateMetadata &metadata, ValuesBucket &outValues)
{
    int64_t dateAdded = metadata.GetDateAdded();
    if (dateAdded == 0) {
        int64_t dateTaken = metadata.GetDateTaken();
        if (dateTaken == 0) {
            int64_t dateModified = metadata.GetDateModified();
            if (dateModified == 0) {
                dateAdded = RingtoneFileUtils::UTCTimeMilliSeconds();
                RINGTONE_WARN_LOG("Invalid dateAdded time, use current time instead: %{public}" PRId64, dateAdded);
            } else {
                dateAdded = dateModified;
                RINGTONE_WARN_LOG("Invalid dateAdded time, use dateModified instead: %{public}" PRId64, dateAdded);
            }
        } else {
            dateAdded = dateTaken * MSEC_TO_SEC;
            RINGTONE_WARN_LOG("Invalid dateAdded time, use dateTaken instead: %{public}" PRId64, dateAdded);
        }
    }
    outValues.PutLong(VIBRATE_COLUMN_DATE_ADDED, dateAdded);
}

static void SetValuesFromMetaData(const RingtoneMetadata &metadata, ValuesBucket &values, bool isInsert)
{
    values.PutString(RINGTONE_COLUMN_DATA, metadata.GetData());
    values.PutLong(RINGTONE_COLUMN_SIZE, metadata.GetSize());
    values.PutString(RINGTONE_COLUMN_DISPLAY_NAME, metadata.GetDisplayName());
    values.PutString(RINGTONE_COLUMN_TITLE, metadata.GetTitle());
    values.PutInt(RINGTONE_COLUMN_MEDIA_TYPE, metadata.GetMediaType());
    values.PutInt(RINGTONE_COLUMN_TONE_TYPE, metadata.GetToneType());
    values.PutString(RINGTONE_COLUMN_MIME_TYPE, metadata.GetMimeType());
    values.PutInt(RINGTONE_COLUMN_SOURCE_TYPE, metadata.GetSourceType());
    values.PutLong(RINGTONE_COLUMN_DATE_MODIFIED, metadata.GetDateModified());
    values.PutLong(RINGTONE_COLUMN_DATE_TAKEN, metadata.GetDateTaken());
    values.PutInt(RINGTONE_COLUMN_DURATION, metadata.GetDuration());
    values.PutInt(RINGTONE_COLUMN_SHOT_TONE_TYPE, metadata.GetShotToneType());
    values.PutInt(RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, metadata.GetShotToneSourceType());
    values.PutInt(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, metadata.GetNotificationToneType());
    values.PutInt(RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, metadata.GetNotificationToneSourceType());
    values.PutInt(RINGTONE_COLUMN_RING_TONE_TYPE, metadata.GetRingToneType());
    values.PutInt(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, metadata.GetRingToneSourceType());
    values.PutInt(RINGTONE_COLUMN_ALARM_TONE_TYPE, metadata.GetAlarmToneType());
    values.PutInt(RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, metadata.GetAlarmToneSourceType());

    if (isInsert) {
        InsertDateAdded(metadata, values);
        return;
    }
}

static void SetValuesFromVibrateMetaData(const VibrateMetadata &metadata, ValuesBucket &values, bool isInsert)
{
    values.PutString(VIBRATE_COLUMN_DATA, metadata.GetData());
    values.PutLong(VIBRATE_COLUMN_SIZE, metadata.GetSize());
    values.PutString(VIBRATE_COLUMN_DISPLAY_NAME, metadata.GetDisplayName());
    values.PutString(VIBRATE_COLUMN_TITLE, metadata.GetTitle());
    values.PutString(VIBRATE_COLUMN_DISPLAY_LANGUAGE, metadata.GetDisplayLanguage());
    values.PutInt(VIBRATE_COLUMN_VIBRATE_TYPE, metadata.GetVibrateType());
    values.PutInt(VIBRATE_COLUMN_SOURCE_TYPE, metadata.GetSourceType());
    values.PutLong(VIBRATE_COLUMN_DATE_MODIFIED, metadata.GetDateModified());
    values.PutLong(VIBRATE_COLUMN_DATE_TAKEN, metadata.GetDateTaken());
    values.PutInt(VIBRATE_COLUMN_PLAY_MODE, metadata.GetPlayMode());

    if (isInsert) {
        InsertVibrateDateAdded(metadata, values);
    }
}

int32_t RingtoneScannerDb::UpdateMetadata(const RingtoneMetadata &metadata, string &tableName)
{
    int32_t updateCount = 0;
    ValuesBucket values;
    string whereClause = RINGTONE_COLUMN_TONE_ID + " = ?";
    vector<string> whereArgs = { to_string(metadata.GetToneId()) };

    SetValuesFromMetaData(metadata, values, false);

    tableName = RINGTONE_TABLE;
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("get raw rdb failed");
        return E_RDB;
    }
    int32_t result = rawRdb->Update(updateCount, tableName, values, whereClause, whereArgs);
    if (result != NativeRdb::E_OK || updateCount <= 0) {
        RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, updateCount);
        return E_DB_FAIL;
    }
    return updateCount;
}

int32_t RingtoneScannerDb::UpdateVibrateMetadata(const VibrateMetadata &metadata, string &tableName)
{
    int32_t updateCount = 0;
    ValuesBucket values;
    string whereClause = VIBRATE_COLUMN_VIBRATE_ID + " = ?";
    vector<string> whereArgs = { to_string(metadata.GetVibrateId()) };

    SetValuesFromVibrateMetaData(metadata, values, false);

    tableName = VIBRATE_TABLE;
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("get raw rdb failed");
        return E_RDB;
    }
    int32_t result = rawRdb->Update(updateCount, tableName, values, whereClause, whereArgs);
    if (result != NativeRdb::E_OK || updateCount <= 0) {
        RINGTONE_ERR_LOG("Update operation failed. Result %{public}d. Updated %{public}d", result, updateCount);
        return E_DB_FAIL;
    }
    return updateCount;
}

int32_t RingtoneScannerDb::InsertMetadata(const RingtoneMetadata &metadata, string &tableName)
{
    ValuesBucket values;
    int32_t rowNum = 0;
    tableName = RINGTONE_TABLE;

    SetValuesFromMetaData(metadata, values, true);
    if (!InsertData(values, tableName, rowNum)) {
        return E_DB_FAIL;
    }

    return rowNum;
}

int32_t RingtoneScannerDb::InsertVibrateMetadata(const VibrateMetadata &metadata, string &tableName)
{
    ValuesBucket values;
    int32_t rowNum = 0;
    tableName = VIBRATE_TABLE;

    SetValuesFromVibrateMetaData(metadata, values, true);
    if (!InsertData(values, tableName, rowNum)) {
        return E_DB_FAIL;
    }

    return rowNum;
}

bool RingtoneScannerDb::InsertData(const ValuesBucket values, const string &tableName, int32_t &rowNum)
{
    auto rawRdb = RingtoneRdbStore::GetInstance()->GetRaw();
    if (rawRdb == nullptr) {
        RINGTONE_ERR_LOG("get raw rdb failed");
        return E_RDB;
    }

    int64_t nRow = static_cast<int64_t>(rowNum);
    int32_t result = rawRdb->Insert(nRow, tableName, values);
    if (nRow <= 0) {
        RINGTONE_ERR_LOG("Ringtone library Insert functionality is failed, rowNum %{public}ld", (long) nRow);
        return false;
    }

    if (result != NativeRdb::E_OK) {
        RINGTONE_ERR_LOG("Ringtone library Insert functionality is failed, return %{public}d", result);
        return false;
    }

    if (nRow < std::numeric_limits<int32_t>::min() || nRow > std::numeric_limits<int32_t>::max()) {
        return false;
    }
    rowNum = static_cast<int32_t>(nRow);

    return true;
}
} // namespace Media
} // namespace OHOS
