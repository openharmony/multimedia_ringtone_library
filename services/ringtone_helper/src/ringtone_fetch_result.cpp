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
#define MLOG_TAG "RingtoneFetchResult"

#include "ringtone_fetch_result.h"

#include "ringtone_log.h"

namespace OHOS {
namespace Media {
using namespace std;
using ResultTypeMap = unordered_map<string, RingtoneResultSetDataType>;

static const ResultTypeMap &GetResultTypeMap()
{
    static const ResultTypeMap RESULT_TYPE_MAP = {
        { RINGTONE_COLUMN_TONE_ID, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_DATA, DATA_TYPE_STRING },
        { RINGTONE_COLUMN_SIZE, DATA_TYPE_INT64 },
        { RINGTONE_COLUMN_DISPLAY_NAME, DATA_TYPE_STRING },
        { RINGTONE_COLUMN_TITLE, DATA_TYPE_STRING },
        { RINGTONE_COLUMN_MEDIA_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_TONE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_MIME_TYPE, DATA_TYPE_STRING },
        { RINGTONE_COLUMN_SOURCE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_DATE_ADDED, DATA_TYPE_INT64 },
        { RINGTONE_COLUMN_DATE_MODIFIED, DATA_TYPE_INT64 },
        { RINGTONE_COLUMN_DATE_TAKEN, DATA_TYPE_INT64 },
        { RINGTONE_COLUMN_DURATION, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_SHOT_TONE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_RING_TONE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_ALARM_TONE_TYPE, DATA_TYPE_INT32 },
        { RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE, DATA_TYPE_INT32 },
    };
    return RESULT_TYPE_MAP;
}

template <class T>
RingtoneFetchResult<T>::RingtoneFetchResult(const shared_ptr<DataShare::DataShareResultSet> &resultset)
{
    resultset_ = resultset;
    GetCount();
}

template <class T>
// empty constructor napi
RingtoneFetchResult<T>::RingtoneFetchResult() : resultset_(nullptr)
{
}

template <class T>
RingtoneFetchResult<T>::~RingtoneFetchResult()
{
    resultset_.reset();
}

template <class T>
void RingtoneFetchResult<T>::Close()
{
    if (resultset_ != nullptr) {
        resultset_->Close();
        resultset_ = nullptr;
    }
}

template <class T>
int32_t RingtoneFetchResult<T>::GetCount()
{
    int32_t count = 0;
    if (resultset_ == nullptr || resultset_->GetRowCount(count) != NativeRdb::E_OK) {
        return 0;
    }
    return count < 0 ? 0 : count;
}

template<class T>
shared_ptr<DataShare::DataShareResultSet> &RingtoneFetchResult<T>::GetDataShareResultSet()
{
    return resultset_;
}

template <class T>
unique_ptr<T> RingtoneFetchResult<T>::GetObjectAtPosition(int32_t index)
{
    if (resultset_ == nullptr) {
        RINGTONE_ERR_LOG("rs is null");
        return nullptr;
    }

    int32_t count = GetCount();
    if ((index < 0) || (index > (count - 1))) {
        RINGTONE_ERR_LOG("index not proper");
        return nullptr;
    }

    if (resultset_->GoToRow(index) != 0) {
        RINGTONE_ERR_LOG("failed to go to row at index pos");
        return nullptr;
    }

    return GetObject();
}

template <class T>
unique_ptr<T> RingtoneFetchResult<T>::GetFirstObject()
{
    if ((resultset_ == nullptr) || (resultset_->GoToFirstRow() != 0)) {
        RINGTONE_DEBUG_LOG("resultset is null|first row failed");
        return nullptr;
    }

    return GetObject();
}

template <class T>
unique_ptr<T> RingtoneFetchResult<T>::GetNextObject()
{
    if ((resultset_ == nullptr) || (resultset_->GoToNextRow() != 0)) {
        RINGTONE_DEBUG_LOG("resultset is null|go to next row failed");
        return nullptr;
    }

    return GetObject();
}

template <class T>
unique_ptr<T> RingtoneFetchResult<T>::GetLastObject()
{
    if ((resultset_ == nullptr) || (resultset_->GoToLastRow() != 0)) {
        RINGTONE_ERR_LOG("resultset is null|go to last row failed");
        return nullptr;
    }

    return GetObject();
}

template <class T>
bool RingtoneFetchResult<T>::IsAtLastRow()
{
    if (resultset_ == nullptr) {
        RINGTONE_ERR_LOG("resultset null");
        return false;
    }

    bool retVal = false;
    resultset_->IsAtLastRow(retVal);
    return retVal;
}

variant<int32_t, int64_t, string, double> ReturnDefaultOnError(string errMsg, RingtoneResultSetDataType dataType)
{
    if (dataType == DATA_TYPE_STRING) {
        return "";
    } else if (dataType == DATA_TYPE_INT64) {
        return static_cast<int64_t>(0);
    } else {
        return 0;
    }
}

template <class T>
variant<int32_t, int64_t, string, double> RingtoneFetchResult<T>::GetRowValFromColumn(const string &columnName,
    RingtoneResultSetDataType dataType, shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    if ((resultset_ == nullptr) && (resultSet == nullptr)) {
        return ReturnDefaultOnError("Resultset is null", dataType);
    }
    int index;
    int status;
    if (resultSet) {
        status = resultSet->GetColumnIndex(columnName, index);
    } else {
        status = resultset_->GetColumnIndex(columnName, index);
    }
    if (status != NativeRdb::E_OK) {
        return ReturnDefaultOnError("failed to obtain the index", dataType);
    }
    return GetValByIndex(index, dataType, resultSet);
}

template <class T>
variant<int32_t, int64_t, string, double> RingtoneFetchResult<T>::GetValByIndex(int32_t index,
    RingtoneResultSetDataType dataType, shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    if ((resultset_ == nullptr) && (resultSet == nullptr)) {
        return ReturnDefaultOnError("Resultset is null", dataType);
    }
    variant<int32_t, int64_t, string, double> cellValue;
    int integerVal = 0;
    string stringVal = "";
    int64_t longVal = 0;
    int status;
    double doubleVal = 0.0;
    switch (dataType) {
        case DATA_TYPE_STRING:
            if (resultSet) {
                status = resultSet->GetString(index, stringVal);
            } else {
                status = resultset_->GetString(index, stringVal);
            }
            cellValue = move(stringVal);
            break;
        case DATA_TYPE_INT32:
            if (resultSet) {
                status = resultSet->GetInt(index, integerVal);
            } else {
                status = resultset_->GetInt(index, integerVal);
            }
            cellValue = integerVal;
            break;
        case DATA_TYPE_INT64:
            if (resultSet) {
                status = resultSet->GetLong(index, longVal);
            } else {
                status = resultset_->GetLong(index, longVal);
            }
            cellValue = longVal;
            break;
        case DATA_TYPE_DOUBLE:
            if (resultSet) {
                status = resultSet->GetDouble(index, doubleVal);
            } else {
                status = resultset_->GetDouble(index, doubleVal);
            }
            cellValue = doubleVal;
            break;
        default:
            break;
    }
    return cellValue;
}

template<class T>
void RingtoneFetchResult<T>::SetRingtoneAsset(RingtoneAsset *asset, shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    if ((resultset_ == nullptr) && (resultSet == nullptr)) {
        RINGTONE_ERR_LOG("SetRingtoneAsset fail, result is nullptr");
        return;
    }
    vector<string> columnNames;
    if (resultSet != nullptr) {
        resultSet->GetAllColumnNames(columnNames);
    } else {
        resultset_->GetAllColumnNames(columnNames);
    }
    int32_t index = -1;
    auto &map = asset->GetMemberMap();
    for (const auto &name : columnNames) {
        index++;
        if (GetResultTypeMap().count(name) == 0) {
            continue;
        }
        auto memberType = GetResultTypeMap().at(name);
        map.emplace(move(name), move(GetValByIndex(index, memberType, resultSet)));
    }
}

template<class T>
void RingtoneFetchResult<T>::GetObjectFromResultSet(RingtoneAsset *asset, shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    SetRingtoneAsset(asset, resultSet);
}

template<class T>
unique_ptr<T> RingtoneFetchResult<T>::GetObject(shared_ptr<NativeRdb::ResultSet> &resultSet)
{
    unique_ptr<T> asset = make_unique<T>();
    GetObjectFromResultSet(asset.get(), resultSet);
    return asset;
}

template <class T>
unique_ptr<T> RingtoneFetchResult<T>::GetObject()
{
    shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    return GetObject(resultSet);
}

template <class T>
unique_ptr<T> RingtoneFetchResult<T>::GetObjectFromRdb(shared_ptr<NativeRdb::ResultSet> &resultSet, int idx)
{
    if ((resultSet == nullptr) || (resultSet->GoToFirstRow() != 0) || (resultSet->GoTo(idx))) {
        RINGTONE_ERR_LOG("resultset is null|first row failed");
        return nullptr;
    }

    return GetObject(resultSet);
}

template class RingtoneFetchResult<RingtoneAsset>;
}  // namespace Media
}  // namespace OHOS
