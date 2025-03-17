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
#define MLOG_TAG "RingtoneAsset"

#include "ringtone_asset.h"

#include "ringtone_db_const.h"
#include "ringtone_errno.h"
#include "ringtone_log.h"
#include "ringtone_type.h"

namespace OHOS {
namespace Media {
using namespace std;

RingtoneAsset::RingtoneAsset()
{
}

int32_t RingtoneAsset::GetId() const
{
    return GetInt32Member(RINGTONE_COLUMN_TONE_ID);
}

void RingtoneAsset::SetId(int32_t id)
{
    member_[RINGTONE_COLUMN_TONE_ID] = id;
}

int64_t RingtoneAsset::GetSize() const
{
    return GetInt64Member(RINGTONE_COLUMN_SIZE);
}

void RingtoneAsset::SetSize(int64_t size)
{
    member_[RINGTONE_COLUMN_SIZE] = size;
}

const string &RingtoneAsset::GetPath() const
{
    return GetStrMember(RINGTONE_COLUMN_DATA);
}

void RingtoneAsset::SetPath(const string &path)
{
    member_[RINGTONE_COLUMN_DATA] = path;
}

const string &RingtoneAsset::GetDisplayName() const
{
    return GetStrMember(RINGTONE_COLUMN_DISPLAY_NAME);
}

void RingtoneAsset::SetDisplayName(const string &displayName)
{
    member_[RINGTONE_COLUMN_DISPLAY_NAME] = displayName;
}

const string &RingtoneAsset::GetTitle() const
{
    return GetStrMember(RINGTONE_COLUMN_TITLE);
}

void RingtoneAsset::SetTitle(const string &title)
{
    member_[RINGTONE_COLUMN_TITLE] = title;
}

const string &RingtoneAsset::GetMimeType() const
{
    return GetStrMember(RINGTONE_COLUMN_MIME_TYPE);
}

void RingtoneAsset::SetMimeType(const string &mimeType)
{
    member_[RINGTONE_COLUMN_MIME_TYPE] = mimeType;
}

int32_t RingtoneAsset::GetToneType() const
{
    return GetInt32Member(RINGTONE_COLUMN_TONE_TYPE);
}

void RingtoneAsset::SetToneType(int32_t toneType)
{
    member_[RINGTONE_COLUMN_TONE_TYPE] = toneType;
}

int32_t RingtoneAsset::GetSourceType() const
{
    return GetInt32Member(RINGTONE_COLUMN_SOURCE_TYPE);
}

void RingtoneAsset::SetSourceType(int32_t type)
{
    member_[RINGTONE_COLUMN_SOURCE_TYPE] = type;
}

int64_t RingtoneAsset::GetDateAdded() const
{
    return GetInt64Member(RINGTONE_COLUMN_DATE_ADDED);
}

void RingtoneAsset::SetDateAdded(int64_t dateAdded)
{
    member_[RINGTONE_COLUMN_DATE_ADDED] = dateAdded;
}

int64_t RingtoneAsset::GetDateModified() const
{
    return GetInt64Member(RINGTONE_COLUMN_DATE_MODIFIED);
}

void RingtoneAsset::SetDateModified(int64_t dateModified)
{
    member_[RINGTONE_COLUMN_DATE_MODIFIED] = dateModified;
}


int64_t RingtoneAsset::GetDateTaken() const
{
    return GetInt64Member(RINGTONE_COLUMN_DATE_TAKEN);
}

void RingtoneAsset::SetDateTaken(int64_t dataTaken)
{
    member_[RINGTONE_COLUMN_DATE_TAKEN] = dataTaken;
}

int32_t RingtoneAsset::GetDuration() const
{
    return GetInt32Member(RINGTONE_COLUMN_DURATION);
}

void RingtoneAsset::SetDuration(int32_t duration)
{
    member_[RINGTONE_COLUMN_DURATION] = duration;
}

int32_t RingtoneAsset::GetShottoneType() const
{
    return GetInt32Member(RINGTONE_COLUMN_SHOT_TONE_TYPE);
}

void RingtoneAsset::SetShottoneType(int32_t type)
{
    member_[RINGTONE_COLUMN_SHOT_TONE_TYPE] = type;
}

int32_t RingtoneAsset::GetShottoneSourceType() const
{
    return GetInt32Member(RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE);
}

void RingtoneAsset::SetShottoneSourceType(int32_t type)
{
    member_[RINGTONE_COLUMN_SHOT_TONE_SOURCE_TYPE] = type;
}

int32_t RingtoneAsset::GetNotificationtoneType() const
{
    return GetInt32Member(RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE);
}

void RingtoneAsset::SetNotificationtoneType(int32_t type)
{
    member_[RINGTONE_COLUMN_NOTIFICATION_TONE_TYPE] = type;
}

int32_t RingtoneAsset::GetNotificationtoneSourceType() const
{
    return GetInt32Member(RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE);
}

void RingtoneAsset::SetNotificationtoneSourceType(int32_t type)
{
    member_[RINGTONE_COLUMN_NOTIFICATION_TONE_SOURCE_TYPE] = type;
}

int32_t RingtoneAsset::GetRingtoneType() const
{
    return GetInt32Member(RINGTONE_COLUMN_RING_TONE_TYPE);
}

void RingtoneAsset::SetRingtoneType(int32_t type)
{
    member_[RINGTONE_COLUMN_RING_TONE_TYPE] = type;
}

int32_t RingtoneAsset::GetRingtoneSourceType() const
{
    return GetInt32Member(RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE);
}

void RingtoneAsset::SetRingtoneSourceType(int32_t type)
{
    member_[RINGTONE_COLUMN_RING_TONE_SOURCE_TYPE] = type;
}

int32_t RingtoneAsset::GetAlarmtoneType() const
{
    return GetInt32Member(RINGTONE_COLUMN_ALARM_TONE_TYPE);
}

void RingtoneAsset::SetAlarmtoneType(int32_t type)
{
    member_[RINGTONE_COLUMN_ALARM_TONE_TYPE] = type;
}

int32_t RingtoneAsset::GetAlarmtoneSourceType() const
{
    return GetInt32Member(RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE);
}

void RingtoneAsset::SetAlarmtoneSourceType(int32_t type)
{
    member_[RINGTONE_COLUMN_ALARM_TONE_SOURCE_TYPE] = type;
}

int32_t RingtoneAsset::GetScannerFlag() const
{
    return GetInt32Member(RINGTONE_COLUMN_SCANNER_FLAG);
}

void RingtoneAsset::SetScannerFlag(int32_t flag)
{
    member_[RINGTONE_COLUMN_SCANNER_FLAG] = flag;
}

void RingtoneAsset::SetOpenStatus(int32_t fd, int32_t openStatus)
{
    lock_guard<mutex> lock(openStatusMapMutex_);
    if (openStatusMap_ == nullptr) {
        openStatusMap_ = make_shared<unordered_map<int32_t, int32_t>>();
    }
    openStatusMap_->insert({fd, openStatus});
}

void RingtoneAsset::RemoveOpenStatus(int32_t fd)
{
    lock_guard<mutex> lock(openStatusMapMutex_);
    if (openStatusMap_ == nullptr) {
        return;
    }
    openStatusMap_->erase(fd);
}

int32_t RingtoneAsset::GetOpenStatus(int32_t fd)
{
    lock_guard<mutex> lock(openStatusMapMutex_);
    if (openStatusMap_ == nullptr) {
        return E_INVALID_VALUES;
    }
    if (openStatusMap_->find(fd) != openStatusMap_->end()) {
        return openStatusMap_->at(fd);
    } else {
        RINGTONE_ERR_LOG("can not find this fd: [%{public}d]", fd);
        return E_INVALID_VALUES;
    }
}

unordered_map<string, variant<int32_t, int64_t, string, double>> &RingtoneAsset::GetMemberMap()
{
    return member_;
}

variant<int32_t, int64_t, string, double> &RingtoneAsset::GetMemberValue(const string &name)
{
    return member_[name];
}

const string &RingtoneAsset::GetStrMember(const string &name) const
{
    return (member_.count(name) > 0) ? get<string>(member_.at(name)) : RINGTONE_DEFAULT_STR;
}

int32_t RingtoneAsset::GetInt32Member(const string &name) const
{
    return (member_.count(name) > 0) ? get<int32_t>(member_.at(name)) : RINGTONE_DEFAULT_INT32;
}

int64_t RingtoneAsset::GetInt64Member(const string &name) const
{
    return (member_.count(name) > 0) ? get<int64_t>(member_.at(name)) : RINGTONE_DEFAULT_INT64;
}
} // namespace Media
} // namespace OHOS
