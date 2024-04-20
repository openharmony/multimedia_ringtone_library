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

#ifndef RINGTONE_COLUMN_DATA_COMMAND_H
#define RINGTONE_COLUMN_DATA_COMMAND_H

#include "abs_rdb_predicates.h"
#include "datashare_predicates.h"
#include "ringtone_db_const.h"
#include "uri.h"
#include "values_bucket.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
enum class EXPORT RingtoneOperationType : uint32_t {
    UNKNOWN_TYPE = 0,
    OPEN,
    INSERT,
    DELETE,
    UPDATE,
    QUERY,
    SCAN,
};

class RingtoneDataCommand {
public:
    EXPORT explicit RingtoneDataCommand(const Uri &uri, const std::string &table, const RingtoneOperationType type);
    EXPORT RingtoneDataCommand() = delete;
    EXPORT ~RingtoneDataCommand();
    EXPORT RingtoneDataCommand(const RingtoneDataCommand &) = delete;
    EXPORT RingtoneDataCommand &operator=(const RingtoneDataCommand &) = delete;
    EXPORT RingtoneDataCommand(RingtoneDataCommand &&) = delete;
    EXPORT RingtoneDataCommand &operator=(RingtoneDataCommand &&) = delete;

    const Uri &GetUri() const;
    const std::string &GetTableName();
    NativeRdb::ValuesBucket &GetValueBucket();
    EXPORT NativeRdb::AbsRdbPredicates *GetAbsRdbPredicates();
    const std::string &GetBundleName();
    const std::string &GetResult();

    EXPORT void SetDataSharePred(const DataShare::DataSharePredicates &pred);
    EXPORT void SetValueBucket(const NativeRdb::ValuesBucket &value);
    void SetBundleName(const std::string &bundleName);
    void SetResult(const std::string &result);
    RingtoneOperationType GetOprnType() const;
private:
    static int32_t GetToneIdFromUri(Uri &uri);

    Uri uri_ {""};
    std::string tableName_;
    RingtoneOperationType oprnType_ {RingtoneOperationType::UNKNOWN_TYPE};
    int32_t toneId_;
    NativeRdb::ValuesBucket insertValue_;
    std::unique_ptr<NativeRdb::AbsRdbPredicates> absRdbPredicates_;
    std::unique_ptr<const DataShare::DataSharePredicates> datasharePred_;
    std::string bundleName_;
    std::string result_;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_COLUMN_DATA_COMMAND_H
