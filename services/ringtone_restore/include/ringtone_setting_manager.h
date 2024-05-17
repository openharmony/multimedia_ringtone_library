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

#ifndef RINGTONE_SETTING_MANAGER_H
#define RINGTONE_SETTING_MANAGER_H

#include <string>
#include <unordered_set>

#include "result_set.h"
#include "rdb_store.h"
#include "ringtone_metadata.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
class RingtoneSettingManager final {
public:
    RingtoneSettingManager(std::shared_ptr<NativeRdb::RdbStore> rdb);
    ~RingtoneSettingManager() = default;
    int32_t CommitSetting(int32_t toneId, std::string &tonePath, int32_t settingType,
        int32_t toneType, int32_t sourceType);
    void FlushSettings();
    int32_t TravelQueryResultSet(std::string querySql, std::function<bool (std::shared_ptr<RingtoneMetadata> &)> func);
private:
    struct SettingItem {
        int32_t toneId = TONE_ID_DEFAULT;
        int32_t settingType = TONE_SETTING_TYPE_DEFAULT;
        int32_t toneType = TONE_TYPE_DEFAULT;
        int32_t sourceType = SOURCE_TYPE_DEFAULT;
    };
    int32_t CommitSettingCompare(int32_t settingType, int32_t toneType, int32_t sourceType);
    void TravelSettings(std::function<int32_t (std::string &, SettingItem &)> func);
    int32_t PopulateMetadata(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        std::unique_ptr<RingtoneMetadata> &metaData);
    int32_t GetMetaDataFromResultSet(std::shared_ptr<NativeRdb::ResultSet> resultSet,
        std::vector<std::shared_ptr<RingtoneMetadata>> &metaDatas);
    int32_t CleanupSettingFromRdb(int32_t settingType, int32_t toneType, int32_t sourceType);
    int32_t UpdateSettingsWithTonePath(std::string &tonePath, int32_t settingType, int32_t toneType);
    int32_t UpdateSettingsWithToneId(int32_t settingType, int32_t toneId, int32_t toneType);
    int32_t CleanupSetting(int32_t settingType, int32_t toneType, int32_t sourceType);
    void ExtractMetaFromColumn(const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
        std::unique_ptr<RingtoneMetadata> &metadata, const std::string &col);
    int32_t UpdateSettingsByPath(std::string &tonePath, int32_t settingType, int32_t toneType, int32_t sourceType);
    int32_t UpdateShotSetting(std::shared_ptr<RingtoneMetadata> &meta, int32_t toneType, int32_t sourceType);
    int32_t UpdateRingtoneSetting(std::shared_ptr<RingtoneMetadata> &meta, int32_t toneType, int32_t sourceType);
    int32_t UpdateNotificationSetting(std::shared_ptr<RingtoneMetadata> &meta, int32_t toneType, int32_t sourceType);
    int32_t UpdateAlarmSetting(std::shared_ptr<RingtoneMetadata> &meta, int32_t toneType, int32_t sourceType);
private:
    std::shared_ptr<NativeRdb::RdbStore> ringtoneRdb_ = nullptr;
    std::unordered_multimap<std::string, SettingItem> settings_ = {};
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_SETTING_MANAGER_H
