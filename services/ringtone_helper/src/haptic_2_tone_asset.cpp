/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define MLOG_TAG "Haptic2ToneAsset"

#include "haptic_2_tone_asset.h"

#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
using namespace std;

Haptic2ToneAsset::Haptic2ToneAsset()
{
}

int32_t Haptic2ToneAsset::GetId() const
{
    return GetInt32Member(HAPTIC_2_TONE_COLUMN_ID);
}

void Haptic2ToneAsset::SetId(int32_t id)
{
    member_[HAPTIC_2_TONE_COLUMN_ID] = id;
}

int64_t Haptic2ToneAsset::GetSize() const
{
    return GetInt64Member(HAPTIC_2_TONE_COLUMN_SIZE);
}

void Haptic2ToneAsset::SetSize(int64_t size)
{
    member_[HAPTIC_2_TONE_COLUMN_SIZE] = size;
}

const string &Haptic2ToneAsset::GetPath() const
{
    return GetStrMember(HAPTIC_2_TONE_COLUMN_DATA);
}

void Haptic2ToneAsset::SetPath(const string &path)
{
    member_[HAPTIC_2_TONE_COLUMN_DATA] = path;
}

const string &Haptic2ToneAsset::GetDisplayName() const
{
    return GetStrMember(HAPTIC_2_TONE_COLUMN_DISPLAY_NAME);
}

void Haptic2ToneAsset::SetDisplayName(const string &displayName)
{
    member_[HAPTIC_2_TONE_COLUMN_DISPLAY_NAME] = displayName;
}

const string &Haptic2ToneAsset::GetTitle() const
{
    return GetStrMember(HAPTIC_2_TONE_COLUMN_TITLE);
}

void Haptic2ToneAsset::SetTitle(const string &title)
{
    member_[HAPTIC_2_TONE_COLUMN_TITLE] = title;
}

int32_t Haptic2ToneAsset::GetHaptic2ToneType() const
{
    return GetInt32Member(HAPTIC_2_TONE_COLUMN_HAPTIC_2_TONE_TYPE);
}

void Haptic2ToneAsset::SetHaptic2ToneType(int32_t type)
{
    member_[HAPTIC_2_TONE_COLUMN_HAPTIC_2_TONE_TYPE] = type;
}

int32_t Haptic2ToneAsset::GetSourceType() const
{
    return GetInt32Member(HAPTIC_2_TONE_COLUMN_SOURCE_TYPE);
}

void Haptic2ToneAsset::SetSourceType(int32_t type)
{
    member_[HAPTIC_2_TONE_COLUMN_SOURCE_TYPE] = type;
}

int64_t Haptic2ToneAsset::GetDateAdded() const
{
    return GetInt64Member(HAPTIC_2_TONE_COLUMN_DATE_ADDED);
}

void Haptic2ToneAsset::SetDateAdded(int64_t dateAdded)
{
    member_[HAPTIC_2_TONE_COLUMN_DATE_ADDED] = dateAdded;
}

int64_t Haptic2ToneAsset::GetDateModified() const
{
    return GetInt64Member(HAPTIC_2_TONE_COLUMN_DATE_MODIFIED);
}

void Haptic2ToneAsset::SetDateModified(int64_t dateModified)
{
    member_[HAPTIC_2_TONE_COLUMN_DATE_MODIFIED] = dateModified;
}

int32_t Haptic2ToneAsset::GetPlayMode() const
{
    return GetInt32Member(HAPTIC_2_TONE_COLUMN_PLAY_MODE);
}

void Haptic2ToneAsset::SetPlayMode(int32_t playMode)
{
    member_[HAPTIC_2_TONE_COLUMN_PLAY_MODE] = playMode;
}

int32_t Haptic2ToneAsset::GetScannerFlag() const
{
    return GetInt32Member(HAPTIC_2_TONE_COLUMN_SCANNER_FLAG);
}

void Haptic2ToneAsset::SetScannerFlag(int32_t scannerFlag)
{
    member_[HAPTIC_2_TONE_COLUMN_SCANNER_FLAG] = scannerFlag;
}

unordered_map<string, variant<int32_t, int64_t, string, double>> &Haptic2ToneAsset::GetMemberMap()
{
    return member_;
}

variant<int32_t, int64_t, string, double> &Haptic2ToneAsset::GetMemberValue(const string &name)
{
    return member_[name];
}

const string &Haptic2ToneAsset::GetStrMember(const string &name) const
{
    return (member_.count(name) > 0) ? get<string>(member_.at(name)) : RINGTONE_DEFAULT_STR;
}

int32_t Haptic2ToneAsset::GetInt32Member(const string &name) const
{
    return (member_.count(name) > 0) ? get<int32_t>(member_.at(name)) : RINGTONE_DEFAULT_INT32;
}

int64_t Haptic2ToneAsset::GetInt64Member(const string &name) const
{
    return (member_.count(name) > 0) ? get<int64_t>(member_.at(name)) : RINGTONE_DEFAULT_INT64;
}
} // namespace Media
} // namespace OHOS
