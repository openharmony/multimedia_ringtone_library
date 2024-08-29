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

#include "simcard_setting_asset.h"

#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
using namespace std;

SimcardSettingAsset::SimcardSettingAsset()
{
}

int32_t SimcardSettingAsset::GetMode() const
{
    return GetInt32Member(SIMCARD_SETTING_COLUMN_MODE);
}

void SimcardSettingAsset::SetMode(int32_t mode)
{
    member_[SIMCARD_SETTING_COLUMN_MODE] = mode;
}

int32_t SimcardSettingAsset::GetVibrateMode() const
{
    return GetInt32Member(SIMCARD_SETTING_COLUMN_VIBRATE_MODE);
}

void SimcardSettingAsset::SetVibrateMode(int32_t mode)
{
    member_[SIMCARD_SETTING_COLUMN_VIBRATE_MODE] = mode;
}

const string &SimcardSettingAsset::GetToneFile() const
{
    return GetStrMember(SIMCARD_SETTING_COLUMN_TONE_FILE);
}

void SimcardSettingAsset::SetToneFile(const string &path)
{
    member_[SIMCARD_SETTING_COLUMN_TONE_FILE] = path;
}

const string &SimcardSettingAsset::GetVibrateFile() const
{
    return GetStrMember(SIMCARD_SETTING_COLUMN_VIBRATE_FILE);
}

void SimcardSettingAsset::SetVibrateFile(const string &path)
{
    member_[SIMCARD_SETTING_COLUMN_VIBRATE_FILE] = path;
}

int32_t SimcardSettingAsset::GetRingtoneType() const
{
    return GetInt32Member(SIMCARD_SETTING_COLUMN_RINGTONE_TYPE);
}

void SimcardSettingAsset::SetRingtoneType(int32_t mode)
{
    member_[SIMCARD_SETTING_COLUMN_RINGTONE_TYPE] = mode;
}

int32_t SimcardSettingAsset::GetRingMode() const
{
    return GetInt32Member(SIMCARD_SETTING_COLUMN_RING_MODE);
}

void SimcardSettingAsset::SetRingMode(int32_t mode)
{
    member_[SIMCARD_SETTING_COLUMN_RING_MODE] = mode;
}

int32_t SimcardSettingAsset::GetInt32Member(const string &name) const
{
    return (member_.count(name) > 0) ? get<int32_t>(member_.at(name)) : RINGTONE_DEFAULT_INT32;
}

const string &SimcardSettingAsset::GetStrMember(const string &name) const
{
    return (member_.count(name) > 0) ? get<string>(member_.at(name)) : RINGTONE_DEFAULT_STR;
}

unordered_map<string, variant<int32_t, int64_t, string, double>> &SimcardSettingAsset::GetMemberMap()
{
    return member_;
}
} // namespace Media
} // namespace OHOS
