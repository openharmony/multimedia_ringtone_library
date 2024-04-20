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

#ifndef RINGTONE_SETTINGS_H
#define RINGTONE_SETTINGS_H

#include <string>
#include <unordered_set>

#include "result_set.h"
#include "ringtone_metadata.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class RingtoneSettings {
public:
    static void ShotToneDefaultSettings();
    static void NotificationToneDefaultSettings();
    static void RingToneDefaultSettings();
private:
    static int32_t GetMetaDataFromResultSet(std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::vector<std::shared_ptr<RingtoneMetadata>> &metas);

    static void QuerySettingsFromRdb(int32_t settingType, std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::shared_ptr<NativeRdb::ResultSet> resultSet2, std::shared_ptr<NativeRdb::ResultSet> resultSetBoth);
    static int32_t UpdateSettingsToRdb(std::string whereClause, std::vector<std::string> &whereArgs,
        const std::string &sourceTypeColumn, int32_t sourceVal, const std::string &toneTypeColumn, int32_t toneVal);

    static int32_t SetDefaultSettingsWithDisplayName(int32_t settingType, std::string &displayName, int32_t toneType,
        int32_t sourceType);
    static int32_t SetDefaultSettingsWithToneId(int32_t settingType, int32_t toneId, int32_t toneType,
        int32_t sourceType);

    static int32_t CleanDefaultSettingRow(int32_t settingType, int32_t toneType, int32_t sourceType);
    static void CleanDefaultSettings(int32_t settingType, std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::shared_ptr<NativeRdb::ResultSet> resultSet2, std::shared_ptr<NativeRdb::ResultSet> resultSetBoth,
        std::string &nameValue1, std::string &nameValue2);
    static int32_t GetResultSetSize(std::shared_ptr<NativeRdb::ResultSet> &resultSet);
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_SETTINGS_H
