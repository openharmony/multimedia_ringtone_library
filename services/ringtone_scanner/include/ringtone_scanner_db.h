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

#ifndef RINGTONE_SCANNER_DB_H
#define RINGTONE_SCANNER_DB_H

#include "result_set.h"
#include "ringtone_data_command.h"
#include "ringtone_metadata.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class RingtoneScannerDb {
public:
    EXPORT RingtoneScannerDb() = default;
    EXPORT RingtoneScannerDb(RingtoneScannerDb &other) = delete;
    EXPORT void operator=(const RingtoneScannerDb &) = delete;
    EXPORT ~RingtoneScannerDb() = default;

    EXPORT static int32_t InsertMetadata(const RingtoneMetadata &metadata, std::string &tableName);
    EXPORT static int32_t UpdateMetadata(const RingtoneMetadata &metadata, std::string &tableName);
    EXPORT static int32_t GetFileBasicInfo(const std::string &path, std::unique_ptr<RingtoneMetadata> &ptr);
    EXPORT static int32_t UpdateRingtoneRdb(NativeRdb::ValuesBucket &values, std::string &whereClause,
         std::vector<std::string> &whereArgs);
    EXPORT static int32_t QueryRingtoneRdb(std::string &whereClause, std::vector<std::string> &whereArgs,
        const std::vector<std::string> &columns, std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    EXPORT static int32_t FillMetadata(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        std::unique_ptr<RingtoneMetadata> &ptr);
private:
    static bool InsertData(const NativeRdb::ValuesBucket values, const std::string &tableName, int64_t &rowNum);
    static int32_t GetFileSet(RingtoneDataCommand &cmd, const std::vector<std::string> &columns,
        std::shared_ptr<NativeRdb::ResultSet> &resultSet);
    static void ExtractMetaFromColumn(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        std::unique_ptr<RingtoneMetadata> &metadata, const std::string &col);
    static int32_t BuildToneSettingTypeQuery(ToneSettingType type, RingtoneDataCommand &cmd);
    static int32_t GetToneSettingType(ToneSettingType type, std::vector<std::shared_ptr<RingtoneMetadata>> &metas);
};
} // namespace Media
} // namespace OHOS
#endif // RINGTONE_SCANNER_DB_H
