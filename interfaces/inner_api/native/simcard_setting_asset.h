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

#ifndef SIMCARD_SETTING_ASSET_H
#define SIMCARD_SETTING_ASSET_H

#include <mutex>
#include <string>

namespace OHOS {
namespace Media {
#define EXPORT __attribute__ ((visibility ("default")))

class SimcardSettingAsset {
public:
    EXPORT SimcardSettingAsset();
    EXPORT virtual ~SimcardSettingAsset() = default;

    EXPORT int32_t GetMode() const;
    EXPORT void SetMode(int32_t mode);

    EXPORT int32_t GetVibrateMode() const;
    EXPORT void SetVibrateMode(int32_t mode);

    EXPORT const std::string &GetToneFile() const;
    EXPORT void SetToneFile(const std::string &path);

    EXPORT const std::string &GetVibrateFile() const;
    EXPORT void SetVibrateFile(const std::string &path);

    EXPORT int32_t GetRingtoneType() const;
    EXPORT void SetRingtoneType(int32_t mode);

    EXPORT int32_t GetRingMode() const;
    EXPORT void SetRingMode(int32_t mode);

    EXPORT std::unordered_map<std::string, std::variant<int32_t, int64_t, std::string, double>> &GetMemberMap();

private:
    int32_t GetInt32Member(const std::string &name) const;
    const std::string &GetStrMember(const std::string &name) const;

    std::unordered_map<std::string, std::variant<int32_t, int64_t, std::string, double>> member_;
};
} // namespace Media
} // namespace OHOS

#endif  // SIMCARD_SETTING_ASSET_H
