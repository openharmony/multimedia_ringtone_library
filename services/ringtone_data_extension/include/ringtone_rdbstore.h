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

#ifndef RINGTONE_RDBSTORE_H
#define RINGTONE_RDBSTORE_H

#include "ringtone_unistore.h"
#include "ringtone_rdb_callbacks.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))

class RingtoneRdbStore final : public RingtoneUnistore {
public:
    EXPORT static std::shared_ptr<RingtoneUnistore> GetInstance(
        const std::shared_ptr<OHOS::AbilityRuntime::Context> &context = nullptr);
    EXPORT virtual ~RingtoneRdbStore();

    explicit RingtoneRdbStore(const std::shared_ptr<OHOS::AbilityRuntime::Context> &context);

    EXPORT virtual int32_t Init() override;
    EXPORT virtual void Stop() override;

    EXPORT virtual int32_t Insert(RingtoneDataCommand &cmd, int64_t &rowId) override;
    EXPORT virtual int32_t Delete(RingtoneDataCommand &cmd, int32_t &deletedRows) override;
    EXPORT virtual int32_t Update(RingtoneDataCommand &cmd, int32_t &changedRows) override;
    EXPORT std::shared_ptr<NativeRdb::ResultSet> Query(RingtoneDataCommand &cmd,
        const std::vector<std::string> &columns) override;

    EXPORT int32_t ExecuteSql(const std::string &sql) override;
    EXPORT std::shared_ptr<NativeRdb::ResultSet> QuerySql(const std::string &sql,
        const std::vector<std::string> &selectionArgs = std::vector<std::string>()) override;
    EXPORT std::shared_ptr<NativeRdb::RdbStore> GetRaw() override;
private:
    std::string bundleName_ {RINGTONE_BUNDLE_NAME};
    NativeRdb::RdbStoreConfig config_ {""};
    static std::shared_ptr<NativeRdb::RdbStore> rdbStore_;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_RDBSTORE_H
