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

#ifndef DUALFWK_SOUND_SETTING_H
#define DUALFWK_SOUND_SETTING_H

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "dualfwk_conf_parser.h"
#include "dualfwk_conf_loader.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
struct DualFwkSettingItem {
    std::string toneFileName {};
    int32_t settingType {TONE_SETTING_TYPE_INVALID};
    int32_t toneType {0};
    bool defaultSysSet {false};
    bool setFlag {false};
    bool isTitle {false};
};

class DualFwkSoundSetting {
public:
    DualFwkSoundSetting();
    ~DualFwkSoundSetting() = default;
    int32_t ProcessConfRow(std::unique_ptr<DualFwkConfRow> &conf);
    void SettingsTraval(std::function<void (DualFwkSettingItem &)> func);
    void ProcessConf(const DualFwkConf &conf);
    std::vector<std::string> GetFileNames() const;
    std::vector<std::string> GetDisplayNames() const;
    std::vector<DualFwkSettingItem> GetSettings() const;
private:
    std::unordered_map<int32_t, DualFwkSettingItem> settings_;
};

} // namespace Media
} // namespace OHOS
#endif // DUALFWK_SOUND_SETTING_H
