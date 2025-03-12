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
#define MLOG_TAG "VibrateAsset"

#include "vibrate_asset.h"

#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_type.h"
#include "vibrate_type.h"

namespace OHOS {
namespace Media {
using namespace std;

VibrateAsset::VibrateAsset()
{
}

int32_t VibrateAsset::GetId() const
{
    return GetInt32Member(VIBRATE_COLUMN_VIBRATE_ID);
}

void VibrateAsset::SetId(int32_t id)
{
    member_[VIBRATE_COLUMN_VIBRATE_ID] = id;
}

int64_t VibrateAsset::GetSize() const
{
    return GetInt64Member(VIBRATE_COLUMN_SIZE);
}

void VibrateAsset::SetSize(int64_t size)
{
    member_[VIBRATE_COLUMN_SIZE] = size;
}

const string &VibrateAsset::GetPath() const
{
    return GetStrMember(VIBRATE_COLUMN_DATA);
}

void VibrateAsset::SetPath(const string &path)
{
    member_[VIBRATE_COLUMN_DATA] = path;
}

const string &VibrateAsset::GetDisplayName() const
{
    return GetStrMember(VIBRATE_COLUMN_DISPLAY_NAME);
}

void VibrateAsset::SetDisplayName(const string &displayName)
{
    member_[VIBRATE_COLUMN_DISPLAY_NAME] = displayName;
}

const string &VibrateAsset::GetTitle() const
{
    return GetStrMember(VIBRATE_COLUMN_TITLE);
}

void VibrateAsset::SetTitle(const string &title)
{
    member_[VIBRATE_COLUMN_TITLE] = title;
}

const std::string &VibrateAsset::GetDisplayLanguage() const
{
    return GetStrMember(VIBRATE_COLUMN_DISPLAY_LANGUAGE);
}

void VibrateAsset::SetDisplayLanguage(const std::string &displayLanguage)
{
    member_[VIBRATE_COLUMN_DISPLAY_LANGUAGE] = displayLanguage;
}

int32_t VibrateAsset::GetVibrateType() const
{
    return GetInt32Member(VIBRATE_COLUMN_VIBRATE_TYPE);
}

void VibrateAsset::SetVibrateType(int32_t type)
{
    member_[VIBRATE_COLUMN_VIBRATE_TYPE] = type;
}

int32_t VibrateAsset::GetSourceType() const
{
    return GetInt32Member(VIBRATE_COLUMN_SOURCE_TYPE);
}

void VibrateAsset::SetSourceType(int32_t type)
{
    member_[VIBRATE_COLUMN_SOURCE_TYPE] = type;
}

int64_t VibrateAsset::GetDateAdded() const
{
    return GetInt64Member(VIBRATE_COLUMN_DATE_ADDED);
}

void VibrateAsset::SetDateAdded(int64_t dateAdded)
{
    member_[VIBRATE_COLUMN_DATE_ADDED] = dateAdded;
}

int64_t VibrateAsset::GetDateModified() const
{
    return GetInt64Member(VIBRATE_COLUMN_DATE_MODIFIED);
}

void VibrateAsset::SetDateModified(int64_t dateModified)
{
    member_[VIBRATE_COLUMN_DATE_MODIFIED] = dateModified;
}

int64_t VibrateAsset::GetDateTaken() const
{
    return GetInt64Member(VIBRATE_COLUMN_DATE_TAKEN);
}

void VibrateAsset::SetDateTaken(int64_t dataTaken)
{
    member_[VIBRATE_COLUMN_DATE_TAKEN] = dataTaken;
}

int32_t VibrateAsset::GetPlayMode() const
{
    return GetInt32Member(VIBRATE_COLUMN_PLAY_MODE);
}

void VibrateAsset::SetPlayMode(int32_t playMode)
{
    member_[VIBRATE_COLUMN_PLAY_MODE] = playMode;
}

int32_t VibrateAsset::GetScannerFlag() const
{
    return GetInt32Member(VIBRATE_COLUMN_SCANNER_FLAG);
}

void VibrateAsset::SetScannerFlag(int32_t flag)
{
    member_[VIBRATE_COLUMN_SCANNER_FLAG] = flag;
}

unordered_map<string, variant<int32_t, int64_t, string, double>> &VibrateAsset::GetMemberMap()
{
    return member_;
}

variant<int32_t, int64_t, string, double> &VibrateAsset::GetMemberValue(const string &name)
{
    return member_[name];
}

const string &VibrateAsset::GetStrMember(const string &name) const
{
    return (member_.count(name) > 0) ? get<string>(member_.at(name)) : RINGTONE_DEFAULT_STR;
}

int32_t VibrateAsset::GetInt32Member(const string &name) const
{
    return (member_.count(name) > 0) ? get<int32_t>(member_.at(name)) : RINGTONE_DEFAULT_INT32;
}

int64_t VibrateAsset::GetInt64Member(const string &name) const
{
    return (member_.count(name) > 0) ? get<int64_t>(member_.at(name)) : RINGTONE_DEFAULT_INT64;
}
} // namespace Media
} // namespace OHOS
