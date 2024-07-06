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

#ifndef RINGTONE_UNISTORE_H
#define RINGTONE_UNISTORE_H

#include "fa_ability_context.h"
#include "rdb_helper.h"
#include "ringtone_data_command.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class RingtoneUnistore {
public:
    EXPORT RingtoneUnistore() = default;
    EXPORT virtual ~RingtoneUnistore() = default;

    EXPORT virtual int32_t Init() = 0;
    EXPORT virtual void Stop() = 0;

    EXPORT virtual int32_t Insert(RingtoneDataCommand &cmd, int64_t &rowId) = 0;
    EXPORT virtual int32_t Delete(RingtoneDataCommand &cmd, int32_t &rowId) = 0;
    EXPORT virtual int32_t Update(RingtoneDataCommand &cmd, int32_t &rowId) = 0;
    EXPORT virtual std::shared_ptr<NativeRdb::ResultSet> Query(RingtoneDataCommand &cmd,
        const std::vector<std::string> &columns)
    {
        return nullptr;
    }

    EXPORT virtual int32_t ExecuteSql(const std::string &sql)
    {
        return NativeRdb::E_NOT_SUPPORT;
    }

    EXPORT virtual std::shared_ptr<NativeRdb::ResultSet> QuerySql(const std::string &sql,
        const std::vector<std::string> &selectionArgs = std::vector<std::string>())
    {
        return nullptr;
    }
    EXPORT virtual std::shared_ptr<NativeRdb::RdbStore> GetRaw() = 0;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_UNISTORE_H
