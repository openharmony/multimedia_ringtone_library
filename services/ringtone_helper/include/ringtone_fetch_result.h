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

#ifndef RINGTONE_FETCH_RESULT_H
#define RINGTONE_FETCH_RESULT_H

#include "abs_shared_result_set.h"
#include "datashare_result_set.h"
#include "rdb_errno.h"
#include "ringtone_asset.h"
#include "ringtone_db_const.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
/**
 * @brief Class for returning the data cursor to application.
 *
 * @since 1.0
 * @version 1.0
 */
template <class T>
class RingtoneFetchResult {
public:
    EXPORT explicit RingtoneFetchResult(const std::shared_ptr<DataShare::DataShareResultSet> &resultset);
    EXPORT RingtoneFetchResult();
    EXPORT virtual ~RingtoneFetchResult();

    EXPORT void Close();
    EXPORT int32_t GetCount();
    EXPORT bool IsAtLastRow();

    EXPORT std::shared_ptr<DataShare::DataShareResultSet> &GetDataShareResultSet();

    EXPORT std::unique_ptr<T> GetObjectAtPosition(int32_t index);
    EXPORT std::unique_ptr<T> GetFirstObject();
    EXPORT std::unique_ptr<T> GetObjectFromRdb(std::shared_ptr<NativeRdb::ResultSet> &resultSet, int idx);
    EXPORT std::unique_ptr<T> GetNextObject();
    EXPORT std::unique_ptr<T> GetLastObject();
    EXPORT std::unique_ptr<T> GetObject();

private:
    EXPORT std::unique_ptr<T> GetObject(std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    EXPORT std::variant<int32_t, int64_t, std::string, double> GetRowValFromColumn(std::string columnName,
        RingtoneResultSetDataType dataType, std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    std::variant<int32_t, int64_t, std::string, double> GetValByIndex(int32_t index, RingtoneResultSetDataType dataType,
        std::shared_ptr<NativeRdb::ResultSet> &resultSet);

    void SetRingtoneAsset(RingtoneAsset *asset, std::shared_ptr<NativeRdb::ResultSet> &resultSet);

    void GetObjectFromResultSet(RingtoneAsset *asset, std::shared_ptr<NativeRdb::ResultSet> &resultSet);

    std::shared_ptr<DataShare::DataShareResultSet> resultset_ = nullptr;
};
} // namespace Media
} // namespace OHOS

#endif  // INTERFACES_INNERKITS_NATIVE_INCLUDE_FETCH_RESULT_H
