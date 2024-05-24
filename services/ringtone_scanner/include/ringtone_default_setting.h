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

#ifndef RINGTONE_DEFAULT_SETTING_H
#define RINGTONE_DEFAULT_SETTING_H

#include <memory>
#include <string>

#include "rdb_store.h"
#include "result_set.h"
#include "ringtone_metadata.h"
#include "ringtone_setting_manager.h"

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))
class RingtoneDefaultSetting {
public:
    EXPORT static std::unique_ptr<RingtoneDefaultSetting> GetObj(std::shared_ptr<NativeRdb::RdbStore> &rdb);
    EXPORT RingtoneDefaultSetting(std::shared_ptr<NativeRdb::RdbStore> &rdb);
    ~RingtoneDefaultSetting() = default;
    EXPORT void Update();
private:
    EXPORT void ShotToneDefaultSettings();
    EXPORT void NotificationToneDefaultSettings();
    EXPORT void RingToneDefaultSettings();
    EXPORT std::string GetTonePathByDisplayName(const std::string &name);

    std::unique_ptr<RingtoneSettingManager> settingMgr_ = nullptr;
};
} // namespace Media
} // namespace OHOS

#endif // RINGTONE_DEFAULT_SETTING_H
